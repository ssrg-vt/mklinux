/*
 * Header for Obtaining Remote PID
 *
 * Akshay
 */

#ifndef __REMOTE_PROC_PID_H__
#define __REMOTE_PROC_PID_H__

#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/types.h>

#include <asm/ptrace.h>


//function borrowed from remote_proc_pid_file.c
extern int do_remote_task_stat(struct seq_file *m,
		struct proc_remote_pid_info *task, char *buf,size_t count);


//functions borrowed from base.c
extern const struct inode_operations proc_def_inode_operations;
extern int proc_setattr(struct dentry *dentry, struct iattr *attr);


// functions to manage remote pid objects (Hooks)
int remote_proc_pid_readdir(struct file *filp, void *dirent, filldir_t filldir,loff_t offset);
struct dentry *remote_proc_pid_lookup(struct inode *dir,
				   struct dentry *dentry, pid_t tgid);


#endif /* __REMOTE_PROC_PID_H__ */
