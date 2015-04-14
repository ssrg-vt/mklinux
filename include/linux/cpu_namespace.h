/*
 * added for Popcorn Linux, 2014-2015 Antonio Barbalace, SSRG Virginia Tech
 */ 
 
#ifndef _LINUX_CPU_NS_H
#define _LINUX_CPU_NS_H

#include <linux/sched.h>
#include <linux/nsproxy.h>
#include <linux/kref.h>

#include <linux/popcorn_cpuinfo.h>

//holes are accepted
struct cpumap {
	atomic_t nr_free;
	void *page;
	//_remote_cpu_info_response_t * cpus;
	
	int start;
	int end;
};
#define CPUMAP_ENTRIES    8
//((PID_MAX_LIMIT + 8*PAGE_SIZE - 1)/PAGE_SIZE/8)

struct cpu_namespace;

struct cpubitmap
{
	struct list_head next_task;	// linked list of cpubitmaps of the same task
	struct task_struct * task;	// sched set affinity is per task
	struct list_head next_ns;	// linked list of cpubitmaps of the same namespace
	struct cpu_namespace * ns;	// ref namespace
	int size; 			// size of bitmap in bytes
	unsigned long bitmap[];		// bitmap (not included here)
};
#define CPUBITMAP_SIZE(cpus) (sizeof(struct cpubitmap) + (BITS_TO_LONGS(cpus) * sizeof(long)))

/*
 * Note that POSIX/libc/nptl does not support hotplug cpus (at least I am not 
 * aware of it) therefore if we have applications running in the Popcorn 
 * namespace we cannot update their namespace objects. Therefore we have to
 * create nested namespace that can be merged when applications end.
 */
struct cpu_namespace
{
	struct kref kref;  
	int nr_cpu_ids;
	int nr_cpus;
	int cpumask_size;		//in bytes
	int _nr_cpumask_bits;		//either nr_cpu_ids or nr_cpus

	struct cpumask * cpu_online_mask;	// for init, it is a copy of cpu_online_mask
						// otherwise it is variable length
	//void (*get_online_cpus)(void); // kernel/cpu.c
	//void (*put_online_cpus)(void); // kernel/cpu.c
  
	struct cpumap cpumap[CPUMAP_ENTRIES]; 	// TODO convert into a list or better data structure
	struct kmem_cache *cpu_cachep;	// TODO cache of elements to put in cpumap
    
	unsigned int level;
	struct cpu_namespace *parent;
    
/*#ifdef CONFIG_PROC_FS
    struct vfsmount *proc_mnt;
#endif
#ifdef CONFIG_BSD_PROCESS_ACCT
    struct bsd_acct_struct *bacct;
#endif
    */
};

extern struct cpu_namespace init_cpu_ns;
extern struct cpu_namespace * popcorn_ns;

//TODO the following must be added in menuconfig & friends
#define CONFIG_CPU_NS
#ifdef CONFIG_CPU_NS

/* 
 * Note that kref when kref is going to zero the kernel should not eliminate
 * the namespace. Until Popcorn is up, i.e. the kernels are inter-connected,
 * Popcorn OS is up and running and the namespace must exists.
 */
static inline struct cpu_namespace *get_cpu_ns(struct cpu_namespace *ns)
{
	if (ns != &init_cpu_ns)
		kref_get(&ns->kref);
	return ns;
}

extern struct cpu_namespace *copy_cpu_ns(unsigned long flags, struct cpu_namespace *ns);
extern void free_cpu_ns(struct kref *kref);

static inline void put_cpu_ns(struct cpu_namespace *cpu_ns)
{
	if (cpu_ns != &init_cpu_ns)
		kref_put(&cpu_ns->kref, free_cpu_ns);
}
#else /* !CONFIG_CPU_NS */
#include <linux/err.h>

static inline struct cpu_namespace *get_cpu_ns(struct cpu_namespace *ns)
{
        return ns;
}

static inline struct cpu_namespace *
copy_cpu_ns(unsigned long flags, struct cpu_namespace *ns)
{
        if (flags & CLONE_NEWPID)
                ns = ERR_PTR(-EINVAL);
        return ns;
}

static inline void put_cpu_ns(struct cpu_namespace *ns)
{
}

/*static inline void zap_cpu_ns_processes(struct cpu_namespace *ns)
{
        BUG();
}*/
#endif

int build_popcorn_ns( int force);
int associate_to_popcorn_ns(struct task_struct * tsk);

#endif /* _LINUX_CPU_NS_H */
