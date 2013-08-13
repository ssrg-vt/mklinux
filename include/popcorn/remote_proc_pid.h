
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/types.h>

#include <asm/ptrace.h>

int iterate_process();
int send_request_to_remote(int KernelId);
int remote_proc_pid_readdir(struct file *filp, void *dirent, filldir_t filldir,loff_t offset);

struct dentry *remote_proc_pid_lookup(struct inode *dir,
				   struct dentry *dentry, pid_t tgid);


extern const struct inode_operations proc_def_inode_operations;
int proc_setattr(struct dentry *dentry, struct iattr *attr);
