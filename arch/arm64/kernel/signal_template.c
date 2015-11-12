/*
 * Based on arch/arm/kernel/signal.c
 *
 * Copyright (C) 1995-2009 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

static int restore_sigframe(struct pt_regs *regs,
			    struct rt_sigframe __user *sf)
{
	sigset_t set;
	int i, err;
	struct aux_context __user *aux =
		(struct aux_context __user *)sf->uc.uc_mcontext.__reserved;

	err = __copy_from_user(&set, &sf->uc.uc_sigmask, sizeof(set));
	if (err == 0)
		set_current_blocked(&set);

	for (i = 0; i < 31; i++)
		__get_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i],
				 err);
	__get_user_error(regs->sp, &sf->uc.uc_mcontext.sp, err);
	__get_user_error(regs->pc, &sf->uc.uc_mcontext.pc, err);
	__get_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);

	/*
	 * Avoid sys_rt_sigreturn() restarting.
	 */
	regs->syscallno = ~0UL;

	err |= !valid_user_regs(&regs->user_regs);

	if (err == 0)
		err |= restore_fpsimd_context(&aux->fpsimd);

	return err;
}

asmlinkage long sys_rt_sigreturn(struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	/*
	 * Since we stacked the signal on a 128-bit boundary, then 'sp' should
	 * be word aligned here.
	 */
	if (regs->sp & 15)
		goto badframe;

	frame = (struct rt_sigframe __user *)regs->sp;

	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;

	if (restore_sigframe(regs, frame))
		goto badframe;

	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	return regs->regs[0];

badframe:
	if (show_unhandled_signals)
		pr_info_ratelimited("%s[%d]: bad frame in %s: pc=%08llx sp=%08llx\n",
				    current->comm, task_pid_nr(current), __func__,
				    regs->pc, regs->sp);
	force_sig(SIGSEGV, current);
	return 0;
}

static int setup_sigframe(struct rt_sigframe __user *sf,
			  struct pt_regs *regs, sigset_t *set)
{
	int i, err = 0;
	struct aux_context __user *aux =
		(struct aux_context __user *)sf->uc.uc_mcontext.__reserved;

	/* set up the stack frame for unwinding */
	__put_user_error(regs->regs[29], &sf->fp, err);
	__put_user_error(regs->regs[30], &sf->lr, err);

	for (i = 0; i < 31; i++)
		__put_user_error(regs->regs[i], &sf->uc.uc_mcontext.regs[i],
				 err);
	__put_user_error(regs->sp, &sf->uc.uc_mcontext.sp, err);
	__put_user_error(regs->pc, &sf->uc.uc_mcontext.pc, err);
	__put_user_error(regs->pstate, &sf->uc.uc_mcontext.pstate, err);

	__put_user_error(current->thread.fault_address, &sf->uc.uc_mcontext.fault_address, err);

	err |= __copy_to_user(&sf->uc.uc_sigmask, set, sizeof(*set));

	if (err == 0)
		err |= preserve_fpsimd_context(&aux->fpsimd);

	/* set the "end" magic */
	__put_user_error(0, &aux->end.magic, err);
	__put_user_error(0, &aux->end.size, err);

	return err;
}

static struct rt_sigframe __user *get_sigframe(struct k_sigaction *ka,
					       struct pt_regs *regs)
{
	unsigned long sp, sp_top;
	struct rt_sigframe __user *frame;

	sp = sp_top = regs->sp;

	/*
	 * This is the X/Open sanctioned signal stack switching.
	 */
	if ((ka->sa.sa_flags & SA_ONSTACK) && !sas_ss_flags(sp))
		sp = sp_top = current->sas_ss_sp + current->sas_ss_size;

	sp = (sp - sizeof(struct rt_sigframe)) & ~15;
	frame = (struct rt_sigframe __user *)sp;

	/*
	 * Check that we can actually write to the signal frame.
	 */
	if (!access_ok(VERIFY_WRITE, frame, sp_top - sp))
		frame = NULL;

	return frame;
}

static void setup_return(struct pt_regs *regs, struct k_sigaction *ka,
			 void __user *frame, int usig)
{
	__sigrestore_t sigtramp;

	regs->regs[0] = usig;
	regs->sp = (unsigned long)frame;
	regs->regs[29] = regs->sp + offsetof(struct rt_sigframe, fp);
	regs->pc = (unsigned long)ka->sa.sa_handler;

	if (ka->sa.sa_flags & SA_RESTORER)
		sigtramp = ka->sa.sa_restorer;
	else
#ifdef ilp32
		sigtramp = VDSO_SYMBOL(current->mm->context.vdso, sigtramp_ilp32);
#else
		sigtramp = VDSO_SYMBOL(current->mm->context.vdso, sigtramp);
#endif

	regs->regs[30] = (unsigned long)sigtramp;
}

static int setup_rt_frame(int usig, struct k_sigaction *ka, siginfo_t *info,
			  sigset_t *set, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;
	int err = 0;

	frame = get_sigframe(ka, regs);
	if (!frame)
		return 1;

	__put_user_error(0, &frame->uc.uc_flags, err);
	__put_user_error(0, &frame->uc.uc_link, err);

	err |= __save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= setup_sigframe(frame, regs, set);
	if (err == 0) {
		setup_return(regs, ka, frame, usig);
		if (ka->sa.sa_flags & SA_SIGINFO) {
			err |= copy_siginfo_to_user(&frame->info, info);
			regs->regs[1] = (unsigned long)&frame->info;
			regs->regs[2] = (unsigned long)&frame->uc;
		}
	}

	return err;
}
