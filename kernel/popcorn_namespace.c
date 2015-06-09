 
/*
 * Popcorn Namespaces
 * 
 * Author: Marina
 */

#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/popcorn_namespace.h>
#include <linux/spinlock.h>
#include <linux/ft_replication.h>

static struct kmem_cache *popcorn_ns_cachep;
struct proc_dir_entry *res;
DEFINE_SPINLOCK(ft_lock); 

struct popcorn_namespace init_pop_ns = {
	.kref = {
		.refcount= ATOMIC_INIT(2),
	},
	.activate= 0,
	.root= 0,
	.replication_degree= 0,
};
EXPORT_SYMBOL_GPL(init_pop_ns);

int is_popcorn_namespace_active(struct popcorn_namespace* ns){
	if(ns){
		if(ns!=&init_pop_ns){
			return ns->activate==1;
		}
	}

	return 0;
}

static struct popcorn_namespace *create_popcorn_namespace(struct popcorn_namespace *parent_ns)
{
	struct popcorn_namespace *ns;
	int err = -ENOMEM;

	ns = kmem_cache_zalloc(popcorn_ns_cachep, GFP_KERNEL);
	if (ns == NULL)
		goto out;

	kref_init(&ns->kref);

        return ns;

out:
        return ERR_PTR(err);
}

struct popcorn_namespace* get_popcorn_ns(struct popcorn_namespace *ns){
	if(ns!= &init_pop_ns)
		kref_get(&ns->kref);
	return ns;
}

struct popcorn_namespace *copy_pop_ns(unsigned long flags, struct popcorn_namespace *old_ns)
{
	if (!(flags & CLONE_NEWPOPCORN)) {
		return get_popcorn_ns(old_ns);
	}
	if (flags & (CLONE_THREAD|CLONE_PARENT)) {
		return ERR_PTR(-EINVAL);
	}
	return create_popcorn_namespace(old_ns);
}

static void destroy_popcorn_namespace(struct popcorn_namespace *ns)
{
        kmem_cache_free(popcorn_ns_cachep, ns);
}

void free_popcorn_ns(struct kref *kref)
{
	struct popcorn_namespace *ns;

	ns = container_of(kref, struct popcorn_namespace, kref);

	destroy_popcorn_namespace(ns);

}

void put_pop_ns(struct popcorn_namespace *ns)
{
        if (ns != &init_pop_ns)
                kref_put(&ns->kref, free_popcorn_ns);
}

int read_notify_popcorn_ns(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	char *p = page;
	int len;
	struct popcorn_namespace *ns = current->nsproxy->pop_ns;

	if(ns){
		if(ns==&init_pop_ns){
			p += sprintf(p, "popcorn ns %p is init popcorn => no associated by default\n", ns);
		}
		else{
			if(ns->activate==1){
				p += sprintf(p, "popcorn ns is %p. It is associate root is pid %d and replication degree is %d\n", ns,ns->root,ns->replication_degree);
			}
			else{
				 p += sprintf(p, "popcorn ns is %p. It is not associate\n", ns);
			}
			
		}
	}
	else{
		 p += sprintf(p, "no popcorn ns pointer \n");
	}

  	len = (p -page) - off;
	if (len < 0)
		len = 0;
	*eof = (len <= count) ? 1 : 0;
	*start = page + off;
	return len;
}

int associate_to_popcorn_ns(struct task_struct * tsk, int replication_degree)
{
	struct popcorn_namespace* pop;

	pop= tsk->nsproxy->pop_ns;
	if(!pop){
		printk("%s: no popcorn_ns pointer\n",__func__);
		return -1;
	}
		
	if (tsk->nsproxy->pop_ns == &init_pop_ns) {
                printk("%s: tring to associate init popcorn namespace with popcorn from pid %d. Create new namespace before tring again.\n", __func__,tsk->pid);
                return -1;
        }
	
	if(pop->activate==1){
		printk("%s: tring to associate popcorn namespace from pid %d but already associated by pid %d\n", __func__,tsk->pid,pop->root);
                return -1;
	}

	spin_lock(&ft_lock);
	if(pop->activate==0){
		pop->root= tsk->pid;
		tsk->replica_type= ROOT_POT_HOT_REPLICA;
		pop->replication_degree= replication_degree;
		pop->activate=1;
	}
	spin_unlock(&ft_lock);

	printk("%s: associated popcorn namespace %p with root %d\n",__func__,pop,pop->root);
	return 0;
}


int write_notify_popcorn_ns(struct file *file, const char __user *buffer, unsigned long count, void *data)
{
	get_task_struct(current);

	//TODO write replication_degree on proc/popcorn_namespace? or counting runnign kernels?
	if((associate_to_popcorn_ns(current, 2))==-1) {
		printk("associate_to_popcorn_ns failed for pid %d\n", current->pid);
	}
	else{
		printk("task pid %d %s associated with popcorn\n",current->pid, current->comm);
	}
	 
	put_task_struct(current);
	return count;
}

static int register_popcorn_ns(void)
{
	printk("Inserting popcorn fd in proc\n");

	res = create_proc_entry("popcorn_namespace", S_IRUGO, NULL);
	if (!res) {
		printk("%s: create_proc_entry failed (%p)\n", __func__, res);
		return -ENOMEM;
	}
	res->read_proc = read_notify_popcorn_ns;
	res->write_proc = write_notify_popcorn_ns;
	
	return 0;
}

static __init int popcorn_namespaces_init(void)
{
	printk("Initializing popcorn_namespace\n");
	popcorn_ns_cachep = KMEM_CACHE(popcorn_namespace, SLAB_PANIC);
	if (!popcorn_ns_cachep) {
		printk("%s: popcorn_namespace initialization error.\n", __func__);
		return -ENOMEM;
	}
	
	register_popcorn_ns();
	
	return 0;
}

__initcall(popcorn_namespaces_init);
