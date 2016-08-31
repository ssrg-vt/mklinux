#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/seqlock.h>

#include <linux/bootmem.h>

#include <popcorn/init.h>
#include <popcorn/cpuinfo.h>

#if defined(CONFIG_ARM64)
#include <asm/memory.h>
#endif

static int krninfo_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m,
		"POPCORN KERNEL INFO:   \n"
		"Kernel Id    %8u(%d)\n"
		"Start PFN    %8lu \n"
		"End   PFN    %8lu \n",
		Kernel_Id, _cpu,
#if defined(CONFIG_X86)
		kernel_start_addr,
#elif defined(CONFIG_ARM64)
		(long unsigned)memstart_addr,
#endif
		(long unsigned)PFN_PHYS(max_low_pfn));

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



