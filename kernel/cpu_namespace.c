 
/*
 * Cpu Namespaces
 *
 * (C) 2014 Antonio Barbalace, SSRG Virginia Tech
 * (C) 2015 Antonio Barbalace and Marina Sadini, SSRG Virginia Tech
 */

#include <linux/cpu_namespace.h>
#include <linux/syscalls.h>
#include <linux/err.h>
//#include <linux/acct.h>
//#include <linux/slab.h>
#include <linux/proc_fs.h>

#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/popcorn_migration.h>

static DEFINE_MUTEX(cpu_caches_mutex);
static struct kmem_cache *cpu_ns_cachep;

static struct kmem_cache *create_cpu_cachep(int nr_ids)
{
	mutex_lock(&cpu_caches_mutex);
//TODO add code here
	mutex_unlock(&cpu_caches_mutex);
	return NULL;
}

static struct cpu_namespace *create_cpu_namespace(struct cpu_namespace *parent_cpu_ns)
{
	struct cpu_namespace *ns;
	unsigned int level = parent_cpu_ns->level +1;
	int i, err = -ENOMEM;

	ns = kmem_cache_zalloc(cpu_ns_cachep, GFP_KERNEL);
	if (ns == NULL)
		goto out;

        ns->cpu_cachep = create_cpu_cachep(level +1);
/*        if (ns->cpu_cachep == NULL)
            goto out_free;
*/

	kref_init(&ns->kref);
	ns->level = level;
	ns->parent = get_cpu_ns(parent_cpu_ns);
	ns->cpu_online_mask = ns->parent->cpu_online_mask;
	ns->nr_cpu_ids = ns->parent->nr_cpu_ids;
	ns->nr_cpus = ns->parent->nr_cpus;
	ns->cpumask_size = ns->parent->cpumask_size;	

/*        err = cpu_ns_prepare_proc(ns);
        if (err)
                goto out_put_parent_cpu_ns;
*/
        return ns;

out_put_parent_cpu_ns:
        put_cpu_ns(parent_cpu_ns);
out_free:
        kmem_cache_free(cpu_ns_cachep, ns);
out:
        return ERR_PTR(err);
}

static void destroy_cpu_namespace(struct cpu_namespace *ns)
{
	int i;
//TODO add code here
	kmem_cache_free(cpu_ns_cachep, ns);
}

struct cpu_namespace *copy_cpu_ns(unsigned long flags, struct cpu_namespace *old_ns)
{
	if (!(flags & CLONE_NEWCPU)) {
		return get_cpu_ns(old_ns);
	}
	if (flags & (CLONE_THREAD|CLONE_PARENT)) {
		return ERR_PTR(-EINVAL);
	}
	return create_cpu_namespace(old_ns);
}

void free_cpu_ns(struct kref *kref)
{
	struct cpu_namespace *ns, *parent;

	ns = container_of(kref, struct cpu_namespace, kref);

	parent = ns->parent;
	destroy_cpu_namespace(ns);

	if (parent != NULL)
		put_cpu_ns(parent);
}

void zap_cpu_ns_processes(struct cpu_namespace *cpu_ns)
{
// TODO add code here
}

static void *cpuns_get(struct task_struct *task)
{
	struct cpu_namespace *ns = NULL;
	struct nsproxy *nsproxy;

	rcu_read_lock();
        //nsproxy = get_pid_ns(task_active_pid_ns(task)); //from kernel/pid_namespace.c
	nsproxy = task_nsproxy(task);
	if (nsproxy)
		ns = get_cpu_ns(nsproxy->cpu_ns);
	rcu_read_unlock();

	return ns;
}

static void cpuns_put(void *ns)
{
	return put_cpu_ns(ns);
}

static int cpuns_install(struct nsproxy *nsproxy, void *ns)
{
	put_cpu_ns(nsproxy->cpu_ns);
	nsproxy->cpu_ns = get_cpu_ns(ns);
	return 0;
}

const struct proc_ns_operations cpuns_operations = {
	.name		= "cpu",
	.type		= CLONE_NEWCPU,
	.get		= cpuns_get,
	.put		= cpuns_put,
	.install	= cpuns_install,
};
struct cpu_namespace * popcorn_ns = 0;

int read_notify_cpu_ns(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len, i, idx;
	struct cpu_namespace *ns = current->nsproxy->cpu_ns;
	char cpumask_buffer[1024];
	
	memset(cpumask_buffer, 0,1024);
	
	if (ns->cpu_online_mask) 
		bitmap_scnprintf(cpumask_buffer, 1023, cpumask_bits(ns->cpu_online_mask), ns->_nr_cpumask_bits );
	else
		printk(KERN_ERR "%s: cpu_online_mask is zero\n",  __func__);

	p += sprintf(p, "task: %s(%p)\n"
			"cpu_ns %p level %d parent %p\n"
			"nr_cpus %d nr_cpu_ids %d nr_cpumask_bits %d cpumask_size %d\n"
			"cpumask %s\n",
			current->comm, current,
			ns, ns->level, ns->parent, ns->nr_cpus, ns->nr_cpu_ids,
			ns->_nr_cpumask_bits, ns->cpumask_size, cpumask_buffer);
	p += sprintf(p, "popcorn ns %p\n", popcorn_ns);
	
  	len = (p -page) - off;
	if (len < 0)
		len = 0;
	*eof = (len <= count) ? 1 : 0;
	*start = page + off;
	return len;
}

extern unsigned int offset_cpus; //from kernel/smp.c
/*
 * This function should be called every time a new kernel will join the popcorn
 * namespace. Note that if there are applications using the popcorn namespace
 * it is not possible to modify the namespace. force will force to update the 
 * namespace data (not currently implemented).
 */
int build_popcorn_ns( int force)
{
	int cnr_cpus =0;
	int cnr_cpu_ids =0;
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	struct cpumask *pcpum =0;
	unsigned long * summary, * tmp;
	int size, offset, error;

	/* calculate the minimum size of the bitmask */
	cnr_cpu_ids += cpumask_weight(cpu_online_mask); // current kernel
	int cpuid =-1;
	list_for_each(iter, &rlist_head) { // other kernels  
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		pcpum = &(objPtr->_data.cpumask); //&(objPtr->_data._cpumask);
		cnr_cpu_ids += bitmap_weight(cpumask_bits(pcpum),
			(objPtr->_data.cpumask_size * BITS_PER_BYTE));//cpumask_weight(pcpum);
	}    

	/* create the mask */
	size = max(cpumask_size(), (BITS_TO_LONGS(cnr_cpu_ids) * sizeof(long)));
	summary = kmalloc(size, GFP_KERNEL);
	tmp = kmalloc(size, GFP_KERNEL);
	if ( !summary || !tmp ) {
		printk(KERN_ERR "%s: kmalloc returned zero allocating summary (%p) or tmp (%p)\n",
			__func__, summary, tmp);
		return -ENOMEM;
	}
	//printk(KERN_ERR"%s: cnr_cpu_ids: %d size:%d summary %p tmp %p\n", __func__, cnr_cpu_ids, size, summary, tmp);
	/* current kernel */
	bitmap_zero(summary, size * BITS_PER_BYTE);
	bitmap_copy(summary, cpumask_bits(cpu_online_mask), nr_cpu_ids);
	bitmap_shift_left(summary, summary, offset_cpus, cnr_cpu_ids);
	/* other kernels  */
	cpuid =-1;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		cpuid = objPtr->_data._processor;
		pcpum = &(objPtr->_data.cpumask);//&(objPtr->_data._cpumask);
		offset = objPtr->_data.cpumask_offset;
		bitmap_zero(tmp, size * BITS_PER_BYTE);
		bitmap_copy(tmp, cpumask_bits(pcpum), (objPtr->_data.cpumask_size *BITS_PER_BYTE));
		bitmap_shift_left(tmp, tmp, offset, cnr_cpu_ids);
		bitmap_or(summary, summary, tmp, cnr_cpu_ids);
	}
    
	/* create the namespace */
	if (!popcorn_ns) {
		struct cpu_namespace * tmp_ns = create_cpu_namespace(&init_cpu_ns);
		if (IS_ERR(tmp_ns)) {
			return -ENODEV;
			//TODO release list lock
		}
		tmp_ns->cpu_online_mask = 0;
	
		//TODO lock the namespace
		popcorn_ns = tmp_ns;
	}
	if (popcorn_ns->cpu_online_mask) {
		if (popcorn_ns->cpu_online_mask != cpumask_bits(cpu_online_mask))
			kfree(popcorn_ns->cpu_online_mask);
	else
		printk(KERN_ERR "%s: Popcorn was associated with cpu_online_mask\n", __func__);
	}
	popcorn_ns->cpu_online_mask= (struct cpumask *)summary;
    
	popcorn_ns->nr_cpus = cnr_cpu_ids; // the followings are intentional
	popcorn_ns->nr_cpu_ids = cnr_cpu_ids;
	popcorn_ns->_nr_cpumask_bits= cnr_cpu_ids;
	popcorn_ns->cpumask_size = BITS_TO_LONGS (cnr_cpu_ids) * sizeof(long);
    
	if (!popcorn_ns->parent) {
		popcorn_ns->parent = &init_cpu_ns;
		popcorn_ns->level = 1;   
	}
    
	//TODO unlock popcorn namespace
	//TODO unlock kernel list
	popcorn_create_thread_pool();
	return 0;
}

int associate_to_popcorn_ns(struct task_struct * tsk)
{
	if (tsk->nsproxy->cpu_ns != popcorn_ns) {
		printk("%s assumes the namespace is popcorn but is not\n", __func__);
		return -ENODEV;
	}
	if (tsk->cpus_allowed_map && (tsk->cpus_allowed_map->ns != tsk->nsproxy->cpu_ns)) {
		printk(KERN_ERR"%s: WARN tsk->cpus_allowed_map->ns (%p) != tsk->nsproxy->cpu_ns (%p)\n",
			__func__, tsk->cpus_allowed_map->ns, tsk->nsproxy->cpu_ns);
	}
  
	if (tsk->cpus_allowed_map == NULL) {
		int size = CPUBITMAP_SIZE(popcorn_ns->nr_cpu_ids);
		struct cpubitmap * cbitm = kmalloc(size, GFP_ATOMIC);// here we should use  a cache instead of kmalloc
		if (!cbitm) {
			printk(KERN_ERR"%s: kmalloc allocation failed\n", __func__);
			return -ENOMEM;
		}
		cbitm->size = size-sizeof(struct cpubitmap);
		cbitm->ns = popcorn_ns;
		bitmap_copy (cbitm->bitmap, tsk->nsproxy->cpu_ns->cpu_online_mask, popcorn_ns->nr_cpu_ids);
		tsk->cpus_allowed_map = cbitm;
	}
	return 0;
}


/*
 * despite this is not the correct way to go, this is working in this way
 * every time we are writing something on this file (even NULL)
 * we are rebuilding a new popcorn namespace merging all the available kernels
 */
int write_notify_cpu_ns(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	int ret;
	get_task_struct(current);

#undef USE_OLD_IMPLEM
#ifdef USE_OLD_IMPLEM
	int cnr_cpu_ids, cnr_cpus;
	struct cpu_namespace *ns = current->nsproxy->cpu_ns;
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	struct cpumask tmp; 
	struct cpumask *pcpum =0;
	struct cpumask * summary;
	
	summary = kmalloc(cpumask_size(), GFP_KERNEL);
	if (!summary) {
		printk(KERN_ERR "%s: kmalloc returned 0\n", __func__);
		return -ENOMEM;
	}
	cnr_cpu_ids += cpumask_weight(cpu_online_mask);
	cpumask_copy(summary, cpu_online_mask);
      
	int cpuid =-1;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		cpuid = objPtr->_data._processor;
		pcpum = &(objPtr->_data._cpumask);
	
		cpumask_copy(&tmp, summary);
		cpumask_or (summary, &tmp, pcpum);
		cnr_cpu_ids += cpumask_weight(pcpum);
	}

	ns->cpu_online_mask = summary;
	ns->nr_cpus = NR_CPUS;
	ns->nr_cpu_ids = cnr_cpu_ids;
	
	if (current->cpus_allowed_map == NULL) {// in this case I have to convert allowed to global mask
		int size = CPUBITMAP_SIZE(ns->nr_cpu_ids);
		struct cpubitmap * cbitm = kmalloc(size, GFP_KERNEL);// TODO here we should use  a cache instead of mkalloc
		if (!cbitm) {
			printk(KERN_ERR"%s: kmalloc allocation failed\n", __func__);
			return -ENOMEM;
		}
		cbitm->size = size;
		cbitm->ns = ns;
		bitmap_zero(cbitm->bitmap, ns->nr_cpu_ids);
		bitmap_copy(cbitm->bitmap, cpumask_bits(&current->cpus_allowed), cpumask_size());
		bitmap_shift_left(cbitm->bitmap, cbitm->bitmap, offset_cpus, ns->nr_cpu_ids);
		current->cpus_allowed_map = cbitm;
	} 
	else {
	// TODO add the code here
	}
#else
	/* the new code builds a popcorn namespace and we should remove the 
	 * ref count and destroy the current (if is not init) and attach to
	 * popcorn.
	 */
  
	/* if the namespace does not exist, create it */
	if (!popcorn_ns) {
		printk(KERN_ALERT "%s: Popcorn namespace pointer is null\n", __func__);
		if ((ret = build_popcorn_ns(0))) { 
			printk(KERN_ERR"%s: build_popcorn returned %d\n", __func__, ret);
			return count;
		}
	}
  
	/* if we are already attached, let's skip the unlinking and linking */
	if (current->nsproxy->cpu_ns != popcorn_ns) { 
		put_cpu_ns(current->nsproxy->cpu_ns);
		current->nsproxy->cpu_ns = get_cpu_ns(popcorn_ns);
		printk(KERN_ERR"%s: cpu_ns %p\n", __func__, current->nsproxy->cpu_ns);
	} else
		printk(KERN_ERR"%s: already attached to popcorn(%p) current = %p\n",
			__func__, popcorn_ns, current->nsproxy->cpu_ns);
		
	if ((ret = associate_to_popcorn_ns(current))) {
		printk(KERN_ERR "associate_to_popcorn_ns returned: %d\n", ret);
		return count;
	}
#endif /* !USE_OLD_IMPLEM */
 
	printk("task %p %s associated with popcorn (local nr_cpu_ids %d NR_CPUS %d cpumask_bits %d OFFSET %d\n",
	       current, current->comm, nr_cpu_ids, NR_CPUS, nr_cpumask_bits, offset_cpus);
	put_task_struct(current);
	return count;
}

struct proc_dir_entry *res;
int register_popcorn_ns(void)
{
	printk(KERN_ALERT "Inserting popcorn fd in proc\n");

	res = create_proc_entry("popcorn", S_IRUGO, NULL);
	if (!res) {
		printk(KERN_ALERT "%s: create_proc_entry failed (%p)\n", __func__, res);
		return -ENOMEM;
	}
	res->read_proc = read_notify_cpu_ns;
	res->write_proc = write_notify_cpu_ns;
	//res->proc_fops  = &ns_file_operations; // required by setns
}

extern const struct file_operations ns_file_operations;
int notify_cpu_ns(void)
{
	struct path old_path;
	int err = 0;
	struct dentry *dentry = old_path.dentry;
	struct inode *dir = dentry->d_inode, * inode;
	struct proc_inode *ei;

	err = kern_path("/proc/", LOOKUP_FOLLOW, &old_path);
	if (err)
		return err;
	
	inode = new_inode(dir->i_sb);
	if (!inode)
	  return err;
	
	ei = PROC_I(inode);
	inode->i_ino = get_next_ino();
	inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
	inode->i_op = dir->i_op; //&proc_def_inode_operations;
	ei = PROC_I(inode);

	inode->i_mode = S_IFREG|S_IRUSR;
	inode->i_fop  = &ns_file_operations;
	ei->ns_ops    = &cpuns_operations;
	ei->ns	      = &popcorn_ns;

	//d_set_d_op(dentry, &pid_dentry_operations); //this are already set in /proc
	d_add(dentry, inode);

// TODO add the real dentry entry! with the name of the file!
	
	return 0;
}

/*
 * The idea is to implement cpu_namespace similarly to net_namespace, i.e.
 * there will be a file somewhere and can be in /proc/popcorn or 
 * /var/run/cpuns/possible_configurations in which you can do setns and being 
 * part of the namespace. Currently implemented as /proc/popcorn.
 */
static __init int cpu_namespaces_init(void)
{
	printk(KERN_ALERT "Initializing cpu_namespace\n");
	cpu_ns_cachep = KMEM_CACHE(cpu_namespace, SLAB_PANIC);
	if (!cpu_ns_cachep) {
		printk(KERN_ERR "%s: cpu_namespace initialization error.\n", __func__);
		return -ENOMEM;
	}
	
	//notify_cpu_ns(); /// initial version
	register_popcorn_ns();
	
	return 0;
}

__initcall(cpu_namespaces_init);
