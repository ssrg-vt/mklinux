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
#include <linux/uaccess.h>
#include <linux/cpu_namespace.h>
#include <linux/popcorn_cpuinfo.h>
#include <popcorn/process_server.h>
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
int save_thread_info(struct task_struct *task, struct pt_regs *regs,
		     field_arch *arch, void __user *uregs)
{
	int ret;

	dump_processor_regs(task_pt_regs(task));

	arch->migration_pc = task_pt_regs(task)->user_regs.pc;
	arch->regs.sp = task_pt_regs(task)->user_regs.sp;
	arch->old_rsp = task_pt_regs(task)->user_regs.sp;
	arch->thread_fs = task->thread.tp_value;

	arch->bp = task_pt_regs(task)->regs[29];

	/*Ajith - for het migration */
	if (task->migration_pc != 0){
		arch->migration_pc = task->migration_pc;
	}

	if (uregs != NULL) {
		ret = copy_from_user(&arch->regs_x86, uregs, sizeof(struct popcorn_regset_x86_64));
		if (ret = -EFAULT) {
			printk("%s: error while copying registers\n", __func__);
		}
	}

	// printk("IN %s:%d migration PC = %lx\n", __func__, __LINE__, arch->migration_pc);

	printk("%s: pc %lx sp %lx bp %lx\n", __func__, arch->migration_pc, arch->old_rsp, arch->bp);

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
	int i;

	task_pt_regs(task)->user_regs.pc = arch->regs.ip;
	task_pt_regs(task)->user_regs.pstate = PSR_MODE_EL0t;
	task_pt_regs(task)->user_regs.sp = arch->old_rsp;

	task->thread.tp_value = arch->thread_fs;

	for (i = 0; i < 31; i++)
		task_pt_regs(task)->regs[i] =  arch->regs_aarch.x[i];
	
	task_pt_regs(task)->regs[29] = arch->bp;
	task_pt_regs(task)->regs[30] = arch->ra;

	if(arch->migration_pc != 0)
		task_pt_regs(task)->user_regs.pc = arch->migration_pc;

	printk("%s: pc %lx sp %lx bp %lx ra %lx\n", __func__, arch->migration_pc, arch->old_rsp, arch->bp, arch->ra);

	dump_processor_regs(task_pt_regs(task));

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
	printk("%s\n", __func__);

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

        memset(&regs, 0, sizeof(struct pt_regs));

	current->flags &= ~PF_KTHREAD;
        task = do_fork_for_main_kernel_thread(flags, 0, &regs, 0, NULL, NULL);
	current->flags |= PF_KTHREAD;
		
	if (task != NULL) {
		//printk("%s [-]: task = 0x%lx\n", __func__, task);
	} else {
		printk("%s [-]: do_fork failed, task = 0x%lx, &task = 0x%lx\n", __func__,
		       (unsigned long)task, (unsigned long)&task);
	}
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
int dump_processor_regs(struct pt_regs *regs)
{
	int i;

	if (regs == NULL) {
		printk(KERN_ERR"process_server: invalid params to dump_processor_regs()");
		return 0;
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

	return 0;
}

unsigned long futex_atomic_add(unsigned long ptr, unsigned long val)
{
	atomic64_t v;
	unsigned long result;
	v.counter = ptr;
	
	result = atomic64_add_return(val, &v);
	return (result - val);
}

extern void update_popcorn_migrate(int a);
void suggest_migration(int suggestion) 
{
	update_popcorn_migrate(suggestion);
}

