/*
 * Based on arch/arm/mm/fault.c
 *
 * Copyright (C) 1995  Linus Torvalds
 * Copyright (C) 1995-2004 Russell King
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

#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/perf_event.h>

#include <linux/process_server.h>
#include <linux/cpu_namespace.h>
#include <process_server_arch.h>

#include <asm/exception.h>
#include <asm/debug-monitors.h>
#include <asm/system_misc.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/compat.h>

#include <linux/delay.h>

static const char *fault_name(unsigned int esr);

extern void dump_instr(const char *lvl, struct pt_regs *regs);

/*
 * Dump out the page tables associated with 'addr' in mm 'mm'.
 */
void show_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;

	if (!mm)
		mm = &init_mm;

	pr_alert("pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	pr_alert("[%08lx] *pgd=%016llx", addr, pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd) || pgd_bad(*pgd))
			break;

		pud = pud_offset(pgd, addr);
		if (pud_none(*pud) || pud_bad(*pud))
			break;

		pmd = pmd_offset(pud, addr);
		printk(", *pmd=%016llx", pmd_val(*pmd));
		if (pmd_none(*pmd) || pmd_bad(*pmd))
			break;

		pte = pte_offset_map(pmd, addr);
		printk(", *pte=%016llx", pte_val(*pte));
		pte_unmap(pte);
	} while(0);

	printk("\n");
}

/*
 * The kernel tried to access some page that wasn't present.
 */
static void __do_kernel_fault(struct mm_struct *mm, unsigned long addr,
			      unsigned int esr, struct pt_regs *regs)
{
	/*
	 * Are we prepared to handle this kernel fault?
	 */
	if (fixup_exception(regs))
		return;

	/*
	 * No handler, we'll have to terminate things with extreme prejudice.
	 */
	bust_spinlocks(1);
	pr_alert("Unable to handle kernel %s at virtual address %08lx\n",
		 (addr < PAGE_SIZE) ? "NULL pointer dereference" :
		 "paging request", addr);

	show_pte(mm, addr);
	die("Oops", regs, esr);
	bust_spinlocks(0);
	do_exit(SIGKILL);
}

/*
 * Something tried to access memory that isn't in our memory map. User mode
 * accesses just cause a SIGSEGV
 */
static void __do_user_fault(struct task_struct *tsk, unsigned long addr,
			    unsigned int esr, unsigned int sig, int code,
			    struct pt_regs *regs)
{
	struct siginfo si;

	if (show_unhandled_signals && unhandled_signal(tsk, sig) &&
	    printk_ratelimit()) {
		pr_info("%s[%d]: unhandled %s (%d) at 0x%08lx, esr 0x%03x\n",
			tsk->comm, task_pid_nr(tsk), fault_name(esr), sig,
			addr, esr);
		show_pte(tsk->mm, addr);
		show_regs(regs);
	}

	tsk->thread.fault_address = addr;
	si.si_signo = sig;
	si.si_errno = 0;
	si.si_code = code;
	si.si_addr = (void __user *)addr;
	force_sig_info(sig, &si, tsk);
}

static void do_bad_area(unsigned long addr, unsigned int esr, struct pt_regs *regs)
{
	struct task_struct *tsk = current;
	struct mm_struct *mm = tsk->active_mm;

	/*
	 * If we are in kernel mode at this point, we have no context to
	 * handle this fault with.
	 */
	if (user_mode(regs))
		__do_user_fault(tsk, addr, esr, SIGSEGV, SEGV_MAPERR, regs);
	else
		__do_kernel_fault(mm, addr, esr, regs);
}

#define VM_FAULT_BADMAP		0x010000
#define VM_FAULT_BADACCESS	0x020000

#define ESR_WRITE		(1 << 6)
#define ESR_CM			(1 << 8)
#define ESR_LNX_EXEC		(1 << 24)

int access_error(unsigned long esr, struct vm_area_struct *vma)
{
	unsigned long mask = VM_READ | VM_WRITE | VM_EXEC;

	if (esr & ESR_LNX_EXEC) {
		mask = VM_EXEC;
	} else if ((esr & ESR_WRITE) && !(esr & ESR_CM)) {
		mask = VM_WRITE;
	}

	return vma->vm_flags & mask ? 0 : 1;
}

extern long my_pid;

static int __do_page_fault(struct mm_struct *mm, unsigned long addr,
			   unsigned int mm_flags, unsigned long vm_flags,
			   struct task_struct *tsk, unsigned int esr,
				int retrying)
{
	struct vm_area_struct *vma;
	int fault, ret_reply;

	vma = find_vma(mm, addr);

	if (tsk->tgroup_distributed == 1) {
		printk("%s: pid %ld, addr %lx\n", __func__, tsk->pid, addr);
	}

	//Ajith - Multikernel changes taken from X86
	ret_reply = 0;
	// Nothing to do for a thread group that's not distributed.
	if(tsk->tgroup_distributed==1 && tsk->main==0 && (retrying == 0)) {
		printk("%s: call process_server_try_handle_mm_fault retrying %d\n", __func__, retrying);
		ret_reply = process_server_try_handle_mm_fault(tsk,mm,vma,addr,mm_flags, esr);

		if(ret_reply==0)
		{
			goto out_distr;
		}

		if(unlikely(ret_reply & (VM_FAULT_VMA| VM_FAULT_REPLICATION_PROTOCOL |\
			 VM_FAULT_ACCESS_ERROR| VM_FAULT_ERROR))){
			goto out_distr;
		}

		vma = find_vma(mm, addr);
	}

	fault = VM_FAULT_BADMAP;
	if (unlikely(!vma))
		goto out;
	if (unlikely(vma->vm_start > addr))
		goto check_stack;

	/*
	 * Ok, we have a good vm_area for this memory access, so we can handle
	 * it.
	 */
good_area:
	/*
	 * Check that the permissions on the VMA allow for the fault which
	 * occurred. If we encountered a write or exec fault, we must have
	 * appropriate permissions, otherwise we allow any permission.
	 */

	if (!(vma->vm_flags & vm_flags)) {
		fault = VM_FAULT_BADACCESS;
		goto out;
	}

	return ret_reply | handle_mm_fault(mm, vma, addr & PAGE_MASK, mm_flags);

check_stack:
	if (vma->vm_flags & VM_GROWSDOWN && !expand_stack(vma, addr))
		goto good_area;
out:
	return fault;

out_distr:
	return ret_reply;
}

static int __kprobes do_page_fault(unsigned long addr, unsigned int esr,
				   struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	struct vm_area_struct *vma;
	int fault, sig, code, retrying = 0, lock_aquired=0;
	unsigned long vm_flags = VM_READ | VM_WRITE | VM_EXEC;
	unsigned int mm_flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;

	//tsk = current;
	//Ajith - Multikernel changes taken from X86
	tsk = (current->surrogate == -1) ? current : 
		pid_task(find_get_pid(current->surrogate), PIDTYPE_PID);

	if(tsk->tgroup_distributed==1 && tsk->main==1){
		printk("ERROR: main is having a page fault\n");
	}

	mm  = tsk->mm;

	/* Enable interrupts if they were enabled in the parent context. */
	if (interrupts_enabled(regs))
		local_irq_enable();

	/*
	 * If we're in an interrupt or have no user context, we must not take
	 * the fault.
	 */
	if (in_atomic() || !mm)
		goto no_context;

	if (user_mode(regs))
		mm_flags |= FAULT_FLAG_USER;

	if (esr & ESR_LNX_EXEC) {
		vm_flags = VM_EXEC;
	} else if ((esr & ESR_WRITE) && !(esr & ESR_CM)) {
		vm_flags = VM_WRITE;
		mm_flags |= FAULT_FLAG_WRITE;
	}

	//Ajith - Mutikernel code taken from x86
	//Multikernel
	if(tsk->tgroup_distributed==1 && tsk->main==0){

		down_read(&mm->distribute_sem);
		lock_aquired= 1;
	}
	else
		lock_aquired= 0;

	/*
	 * As per x86, we may deadlock here. However, since the kernel only
	 * validly references user space from well defined areas of the code,
	 * we can bug out early if this is from code which shouldn't.
	 */
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
retry:
		down_read(&mm->mmap_sem);
	} else {
		/*
		 * The above down_read_trylock() might have succeeded in which
		 * case, we'll have missed the might_sleep() from down_read().
		 */
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) && !search_exception_tables(regs->pc))
			goto no_context;
#endif
	}

	fault = __do_page_fault(mm, addr, mm_flags, vm_flags, tsk, esr, retrying);

	//Ajith - Return if page is remotely handled
	if(fault == 0)
		goto out;


	/*
	 * If we need to retry but a fatal signal is pending, handle the
	 * signal first. We do not need to release the mmap_sem because it
	 * would already be released in __lock_page_or_retry in mm/filemap.c.
	 */
	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		goto out_distr;

	/*
	 * Major/minor page fault accounting is only done on the initial
	 * attempt. If we go through a retry, it is extremely likely that the
	 * page will be found in page cache at that point.
	 */

	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, addr);
	if (mm_flags & FAULT_FLAG_ALLOW_RETRY) {
		if (fault & VM_FAULT_MAJOR) {
			tsk->maj_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1, regs,
				      addr);
		} else {
			tsk->min_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1, regs,
				      addr);
		}
		if (fault & VM_FAULT_RETRY) {
			/*
			 * Clear FAULT_FLAG_ALLOW_RETRY to avoid any risk of
			 * starvation.
			 */
			mm_flags &= ~FAULT_FLAG_ALLOW_RETRY;

			if(tsk->tgroup_distributed==1)
				retrying = 1;

			vma = find_vma(mm, addr);

			if((tsk->tgroup_distributed == 1 && tsk->main==0) && (fault & VM_CONTINUE_WITH_CHECK))
			{
				fault = process_server_update_page(tsk,mm,vma,addr,mm_flags, 1);
			}

			goto retry;
		}
	}


        if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP |
                              VM_FAULT_BADACCESS | VM_FAULT_REPLICATION_PROTOCOL |
                                VM_FAULT_VMA | VM_FAULT_ACCESS_ERROR)))) {

                vma = find_vma(mm, addr);
                if((tsk->tgroup_distributed == 1 && tsk->main==0) && (fault & VM_CONTINUE_WITH_CHECK))
                {
			fault = process_server_update_page(tsk,mm,vma,addr,mm_flags, 0);
                }
        }

	up_read(&mm->mmap_sem);

	/*
	 * Handle the "normal" case first - VM_FAULT_MAJOR / VM_FAULT_MINOR
	 */
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP |
			      VM_FAULT_BADACCESS | VM_FAULT_REPLICATION_PROTOCOL |
				VM_FAULT_VMA | VM_FAULT_ACCESS_ERROR))))
		goto out_distr;

	/*
	 * If we are in kernel mode at this point, we have no context to
	 * handle this fault with.
	 */
	if (!user_mode(regs))
		goto no_context;

	if (fault & VM_FAULT_OOM) {
		/*
		 * We ran out of memory, call the OOM killer, and return to
		 * userspace (which will retry the fault, or kill us if we got
		 * oom-killed).
		 */
		pagefault_out_of_memory();
		goto out_distr;
	}

	if (fault & VM_FAULT_SIGBUS) {
		/*
		 * We had some memory, but were unable to successfully fix up
		 * this page fault.
		 */
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		/*
		 * Something tried to access memory that isn't in our memory
		 * map.
		 */
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}

	__do_user_fault(tsk, addr, esr, sig, code, regs);
	goto out_distr;

no_context:
	__do_kernel_fault(mm, addr, esr, regs);
	goto out_distr;

out:
	up_read(&mm->mmap_sem);

out_distr:
	if((tsk->tgroup_distributed == 1 && tsk->main==0) && lock_aquired){
		up_read(&mm->distribute_sem);
	}

    /*if(my_pid == tsk->prev_pid)
    {
	    printk("In %s:%d\n",__func__, __LINE__);
            dump_instr(KERN_INFO, task_pt_regs(tsk));
    }*/

	return 0;
}

/*
 * First Level Translation Fault Handler
 *
 * We enter here because the first level page table doesn't contain a valid
 * entry for the address.
 *
 * If the address is in kernel space (>= TASK_SIZE), then we are probably
 * faulting in the vmalloc() area.
 *
 * If the init_task's first level page tables contains the relevant entry, we
 * copy the it to this task.  If not, we send the process a signal, fixup the
 * exception, or oops the kernel.
 *
 * NOTE! We MUST NOT take any locks for this case. We may be in an interrupt
 * or a critical region, and should only copy the information from the master
 * page table, nothing more.
 */
static int __kprobes do_translation_fault(unsigned long addr,
					  unsigned int esr,
					  struct pt_regs *regs)
{
	//if (addr < TASK_SIZE)
		return do_page_fault(addr, esr, regs);

	//do_bad_area(addr, esr, regs);
	//return 0;
}

/*
 * This abort handler always returns "fault".
 */
static int do_bad(unsigned long addr, unsigned int esr, struct pt_regs *regs)
{
	return 1;
}

static struct fault_info {
	int	(*fn)(unsigned long addr, unsigned int esr, struct pt_regs *regs);
	int	sig;
	int	code;
	const char *name;
} fault_info[] = {
	{ do_bad,		SIGBUS,  0,		"ttbr address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 1 address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 2 address size fault"	},
	{ do_bad,		SIGBUS,  0,		"level 3 address size fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"input address range fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"level 1 translation fault"	},
	{ do_translation_fault,	SIGSEGV, SEGV_MAPERR,	"level 2 translation fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_MAPERR,	"level 3 translation fault"	},
	{ do_bad,		SIGBUS,  0,		"reserved access flag fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 1 access flag fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 2 access flag fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 3 access flag fault"	},
	{ do_bad,		SIGBUS,  0,		"reserved permission fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 1 permission fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 2 permission fault"	},
	{ do_page_fault,	SIGSEGV, SEGV_ACCERR,	"level 3 permission fault"	},
	{ do_bad,		SIGBUS,  0,		"synchronous external abort"	},
	{ do_bad,		SIGBUS,  0,		"asynchronous external abort"	},
	{ do_bad,		SIGBUS,  0,		"unknown 18"			},
	{ do_bad,		SIGBUS,  0,		"unknown 19"			},
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous abort (translation table walk)" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error"	},
	{ do_bad,		SIGBUS,  0,		"asynchronous parity error"	},
	{ do_bad,		SIGBUS,  0,		"unknown 26"			},
	{ do_bad,		SIGBUS,  0,		"unknown 27"			},
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk" },
	{ do_bad,		SIGBUS,  0,		"synchronous parity error (translation table walk" },
	{ do_bad,		SIGBUS,  0,		"unknown 32"			},
	{ do_bad,		SIGBUS,  BUS_ADRALN,	"alignment fault"		},
	{ do_bad,		SIGBUS,  0,		"debug event"			},
	{ do_bad,		SIGBUS,  0,		"unknown 35"			},
	{ do_bad,		SIGBUS,  0,		"unknown 36"			},
	{ do_bad,		SIGBUS,  0,		"unknown 37"			},
	{ do_bad,		SIGBUS,  0,		"unknown 38"			},
	{ do_bad,		SIGBUS,  0,		"unknown 39"			},
	{ do_bad,		SIGBUS,  0,		"unknown 40"			},
	{ do_bad,		SIGBUS,  0,		"unknown 41"			},
	{ do_bad,		SIGBUS,  0,		"unknown 42"			},
	{ do_bad,		SIGBUS,  0,		"unknown 43"			},
	{ do_bad,		SIGBUS,  0,		"unknown 44"			},
	{ do_bad,		SIGBUS,  0,		"unknown 45"			},
	{ do_bad,		SIGBUS,  0,		"unknown 46"			},
	{ do_bad,		SIGBUS,  0,		"unknown 47"			},
	{ do_bad,		SIGBUS,  0,		"unknown 48"			},
	{ do_bad,		SIGBUS,  0,		"unknown 49"			},
	{ do_bad,		SIGBUS,  0,		"unknown 50"			},
	{ do_bad,		SIGBUS,  0,		"unknown 51"			},
	{ do_bad,		SIGBUS,  0,		"implementation fault (lockdown abort)" },
	{ do_bad,		SIGBUS,  0,		"unknown 53"			},
	{ do_bad,		SIGBUS,  0,		"unknown 54"			},
	{ do_bad,		SIGBUS,  0,		"unknown 55"			},
	{ do_bad,		SIGBUS,  0,		"unknown 56"			},
	{ do_bad,		SIGBUS,  0,		"unknown 57"			},
	{ do_bad,		SIGBUS,  0,		"implementation fault (coprocessor abort)" },
	{ do_bad,		SIGBUS,  0,		"unknown 59"			},
	{ do_bad,		SIGBUS,  0,		"unknown 60"			},
	{ do_bad,		SIGBUS,  0,		"unknown 61"			},
	{ do_bad,		SIGBUS,  0,		"unknown 62"			},
	{ do_bad,		SIGBUS,  0,		"unknown 63"			},
};

static const char *fault_name(unsigned int esr)
{
	const struct fault_info *inf = fault_info + (esr & 63);
	return inf->name;
}

void dump_processor_regs_arm64(struct pt_regs* regs)
{
	int i;

	if (regs == NULL) {
		printk(KERN_ERR"process_server: invalid params to dump_processor_regs()");
		return;
	}

	dump_stack();

	printk(KERN_ALERT"DUMP REGS %s\n", __func__);

	if (NULL != regs) {
		printk(KERN_ALERT"sp: 0x%lx\n", regs->sp);
		printk(KERN_ALERT"pc: 0x%lx\n", regs->pc);
		printk(KERN_ALERT"pstate: 0x%lx\n", regs->pstate);

		for (i = 0; i < 31; i++) {
			printk(KERN_ALERT"regs[%d]: 0x%lx\n", i, regs->regs[i]);
		}
	}
}

/*
 * Dispatch a data abort to the relevant handler.
 */
asmlinkage void __exception do_mem_abort(unsigned long addr, unsigned int esr,
					 struct pt_regs *regs)
{
	const struct fault_info *inf = fault_info + (esr & 63);
	struct siginfo info;

	if (!inf->fn(addr, esr, regs))
		return;

	pr_alert("Unhandled fault: %s (0x%08x) at 0x%016lx\n",
		 inf->name, esr, addr);

	/* dump_stack(); */
	dump_processor_regs(regs);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("", regs, &info, esr);
}

void hook_fault_code(int nr,
				  int (*fn)(unsigned long, unsigned int, struct pt_regs *),
				  int sig, int code, const char *name)
{
	BUG_ON(nr < 0 || nr >= ARRAY_SIZE(fault_info));

	fault_info[nr].fn	= fn;
	fault_info[nr].sig	= sig;
	fault_info[nr].code	= code;
	fault_info[nr].name	= name;
}

/*
 * Handle stack alignment exceptions.
 */
asmlinkage void __exception do_sp_pc_abort(unsigned long addr,
					   unsigned int esr,
					   struct pt_regs *regs)
{
	struct siginfo info;

	printk(" In %s:%d\n", __func__, __LINE__);
	printk("SP: %lx PC: %lx\n", regs->user_regs.sp, regs->user_regs.pc);

	info.si_signo = SIGBUS;
	info.si_errno = 0;
	info.si_code  = BUS_ADRALN;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("", regs, &info, esr);
}

static struct fault_info debug_fault_info[] = {
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware breakpoint"	},
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware single-step"	},
	{ do_bad,	SIGTRAP,	TRAP_HWBKPT,	"hardware watchpoint"	},
	{ do_bad,	SIGBUS,		0,		"unknown 3"		},
	{ do_bad,	SIGTRAP,	TRAP_BRKPT,	"aarch32 BKPT"		},
	{ do_bad,	SIGTRAP,	0,		"aarch32 vector catch"	},
	{ do_bad,	SIGTRAP,	TRAP_BRKPT,	"aarch64 BRK"		},
	{ do_bad,	SIGBUS,		0,		"unknown 7"		},
};

void __init hook_debug_fault_code(int nr,
				  int (*fn)(unsigned long, unsigned int, struct pt_regs *),
				  int sig, int code, const char *name)
{
	BUG_ON(nr < 0 || nr >= ARRAY_SIZE(debug_fault_info));

	debug_fault_info[nr].fn		= fn;
	debug_fault_info[nr].sig	= sig;
	debug_fault_info[nr].code	= code;
	debug_fault_info[nr].name	= name;
}

asmlinkage int __exception do_debug_exception(unsigned long addr,
					      unsigned int esr,
					      struct pt_regs *regs)
{
	const struct fault_info *inf = debug_fault_info + DBG_ESR_EVT(esr);
	struct siginfo info;

	if (!inf->fn(addr, esr, regs))
		return 1;

	pr_alert("Unhandled debug exception: %s (0x%08x) at 0x%016lx\n",
		 inf->name, esr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm64_notify_die("", regs, &info, esr);

	return 0;
}
