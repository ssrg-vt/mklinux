#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>


static int krninfo_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m,
		"Popcorn Kernel info:       %8lu kB\n",
		0);

	return 0;
}

static int krninfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, krninfo_proc_show, NULL);
}

static const struct file_operations krninfo_proc_fops = {
	.open		= krninfo_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_krninfo_init(void)
{
	proc_create("krninfo", 0, NULL, &krninfo_proc_fops);
	return 0;
}
module_init(proc_krninfo_init);
