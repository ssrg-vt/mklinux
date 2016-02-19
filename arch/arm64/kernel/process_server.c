/*
 * File:
 * 	process_server.c
 *
 * Description:
 * 	Dummy file to support helper functionality 
 * of the process server
 *
 * Created on:
 * 	Sep 19, 2014
 *
 * Author:
 * 	Ajithchandra Saya, SSRG, VirginiaTech
 *
 */

/* File includes */
#include <linux/sched.h>
#include <linux/cpu_namespace.h>
#include <linux/popcorn_cpuinfo.h>
#include <linux/process_server.h>
#include <process_server_arch.h>

#include <asm/pgtable.h>
#include <asm/pgtable-hwdef.h>
#include <asm/pgtable-3level-types.h>

#include <uapi/asm/ptrace.h>

/* External function declarations */
extern unsigned long read_old_rsp(void);
extern struct task_struct* do_fork_for_main_kernel_thread(unsigned long clone_flags,
		unsigned long stack_start, struct pt_regs *regs, unsigned long stack_size,
		int __user *parent_tidptr, int __user *child_tidptr);

/*
 * Function:
 *		save_thread_info
 *
 * Description:
 *		this function saves the architecture specific info of the task
 *		to the field_arch structure passed
 *
 * Input:
 *	task,	pointer to the task structure of the task of which the
 *			architecture specific info needs to be saved
 *
 *	regs,	pointer to the pt_regs field of the task
 *
 * Output:
 *	arch,	pointer to the field_arch structure type where the
 *			architecture specific information of the task has to be
 *			saved
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int save_thread_info(struct task_struct *task, struct pt_regs *regs, field_arch *arch)
{
	unsigned long val = 0;

	arch->migration_pc = task_pt_regs(task)->user_regs.pc;
	arch->regs.sp = task_pt_regs(task)->user_regs.sp;
	arch->old_rsp = task_pt_regs(task)->user_regs.sp;
	arch->thread_fs = task->thread.tp_value;

	/*Ajith - for het migration */
	if(task->migration_pc != 0){
		arch->migration_pc = task->migration_pc;
		PSPRINTK("IN %s:%d migration PC = %lx\n", __func__, __LINE__, arch->migration_pc);
	}

	//dump_processor_regs(&arch->regs);

	return 0;
}

/*
 * Function:
 *		restore_thread_info
 *
 * Description:
 *		this function restores the architecture specific info of the
 *		task from the field_arch structure passed
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			architecture specific info needs to be restored
 *
 * 	arch,	pointer to the field_arch structure type from which the
 *			architecture specific information of the task has to be
 *			restored
 *
 * Output:
 *	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int restore_thread_info(struct task_struct *task, field_arch *arch)
{
	task_pt_regs(task)->user_regs.pc = arch->regs.ip;
	task_pt_regs(task)->user_regs.pstate = PSR_MODE_EL0t;
	task_pt_regs(task)->user_regs.sp = arch->old_rsp;

	task_pt_regs(task)->regs[29] = arch->bp;

	task->thread.tp_value = arch->thread_fs;
	//dump_processor_regs(&arch->regs);
	
	PSPRINTK("IP value during restore %lx %lx %ld\n", arch->regs.ip, arch->old_rsp, task->thread.tp_value);

	if(arch->migration_pc != 0)
		task_pt_regs(task)->user_regs.pc = arch->migration_pc;

	return 0;
}

/*
 * Function:
 *		update_thread_info
 *
 * Description:
 *		this function updates the task's thread structure to
 *		the latest register values.
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			thread structure needs to be updated
 *
 * Output:
 * 	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int update_thread_info(struct task_struct *task)
{
	return 0;
}

/*
 * Function:
 *		initialize_thread_retval
 *
 * Description:
 *		this function sets the return value of the task
 *		to the value specified in the argument val
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			return value needs to be set
 * 	val,	the return value to be set
 *
 * Output:
 * 	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int initialize_thread_retval(struct task_struct *task, int val)
{
	return 0;
}

/*
 * Function:
 *		create_thread
 *
 * Description:
 *		this function creates an empty thread and returns its task
 *		structure
 *
 * Input:
 * 	flags,	the clone flags to be used to create the new thread
 *
 * Output:
 * 	none
 *
 * Return value:
 * 	on success,	returns pointer to newly created task's structure,
 *	on failure, returns NULL
 */
struct task_struct* create_thread(int flags)
{
	struct task_struct *task = NULL;
	struct pt_regs regs;

        //printk("%s [+]: flags = 0x%x\n", __func__, flags);
        memset(&regs, 0, sizeof(struct pt_regs));

	current->flags &= ~PF_KTHREAD;
        task = do_fork_for_main_kernel_thread(flags, 0, &regs, 0, NULL, NULL);
	current->flags |= PF_KTHREAD;
		
	if (task != NULL) {
		//printk("%s [-]: task = 0x%lx\n", __func__, task);
	} else {
		printk("%s [-]: do_fork failed, task = 0x%lx, &task = 0x%lx\n", task, &task);
	}
exit:
        return task;

}

#if MIGRATE_FPU

/*
 * Function:
 *		save_fpu_info
 *
 * Description:
 *		this function saves the FPU info of the task specified
 *		to the arch structure specified in the argument
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			FPU info needs to be saved
 *
 * Output:
 * 	arch,	pointer to the field_arch structure where the FPU info
 * 			needs to be saved
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int save_fpu_info(struct task_struct *task, field_arch *arch)
{
	return 0;
}

/*
 * Function:
 *		restore_fpu_info
 *
 * Description:
 *		this function restores the FPU info of the task specified
 *		from the arch structure specified in the argument
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			FPU info needs to be restored
 *
 * 	arch,	pointer to the field_arch struture from where the fpu info
 * 			needs to be restored
 *
 * Output:
 * 	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int restore_fpu_info(struct task_struct *task, field_arch *arch)
{
	return 0;
}

/*
 * Function:
 *		update_fpu_info
 *
 * Description:
 *		this function updates the FPU info of the task specified
 *
 * Input:
 * 	task,	pointer to the task structure of the task of which the
 * 			FPU info needs to be updated
 *
 * Output:
 * 	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int update_fpu_info(struct task_struct *task)
{
	return 0;
}

#endif

/*
 * Function:
 *		dump_processor_regs
 *
 * Description:
 *		this function prints the architecture specific registers specified
 *		in the input argument
 *
 * Input:
 * 	task,	pointer to the architecture specific registers
 *
 * Output:
 * 	none
 *
 * Return value:
 *	on success, returns 0
 * 	on failure, returns negative integer
 */
int dump_processor_regs(struct x86_pt_regs *regs)
{
        int ret = -1;
        unsigned long fs, gs;

        if(regs == NULL){
                printk(KERN_ERR"process_server: invalid params to dump_processor_regs()");
                goto exit;
        }
        printk(KERN_ALERT"DUMP REGS\n");

        if(NULL != regs) {
                printk(KERN_ALERT"r15{%lx}\n",regs->r15);
                printk(KERN_ALERT"r14{%lx}\n",regs->r14);
                printk(KERN_ALERT"r13{%lx}\n",regs->r13);
                printk(KERN_ALERT"r12{%lx}\n",regs->r12);
                printk(KERN_ALERT"r11{%lx}\n",regs->r11);
                printk(KERN_ALERT"r10{%lx}\n",regs->r10);
                printk(KERN_ALERT"r9{%lx}\n",regs->r9);
                printk(KERN_ALERT"r8{%lx}\n",regs->r8);
                printk(KERN_ALERT"bp{%lx}\n",regs->bp);
                printk(KERN_ALERT"bx{%lx}\n",regs->bx);
                printk(KERN_ALERT"ax{%lx}\n",regs->ax);
                printk(KERN_ALERT"cx{%lx}\n",regs->cx);
                printk(KERN_ALERT"dx{%lx}\n",regs->dx);
                printk(KERN_ALERT"di{%lx}\n",regs->di);
                printk(KERN_ALERT"orig_ax{%lx}\n",regs->orig_ax);
                printk(KERN_ALERT"ip{%lx}\n",regs->ip);
                printk(KERN_ALERT"cs{%lx}\n",regs->cs);
                printk(KERN_ALERT"flags{%lx}\n",regs->flags);
                printk(KERN_ALERT"sp{%lx}\n",regs->sp);
                printk(KERN_ALERT"ss{%lx}\n",regs->ss);
        }
        printk(KERN_ALERT"fs{%lx}\n",fs);
        printk(KERN_ALERT"gs{%lx}\n",gs);
        printk(KERN_ALERT"REGS DUMP COMPLETE\n");
        ret = 0;

exit:
        return ret;

}

unsigned long futex_atomic_add(unsigned long ptr, unsigned long val)
{
	atomic64_t v;
	unsigned long result;
	v.counter = ptr;
	
	result = atomic64_add_return(val, &v);
	return (result - val);
}

