/*
 * ft_time.c
 *
 * Author: Marina
 */

#include <linux/ft_replication.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/pcn_kmsg.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include <linux/sched.h>

#define FT_TIME_VERBOSE 0
#if FT_TIME_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct get_time_info{
	struct timeval tv;
        struct timezone tz;
};

static long ft_gettimeofday_primary(struct timeval __user * tv, struct timezone __user * tz){
	long ret= 0;
	struct get_time_info *get_time;

	FTPRINTK("%s called form pid %d\n", __func__, current->pid);
	
	get_time = kmalloc(sizeof(*get_time), GFP_KERNEL);
	if(!get_time){
        	return -ENOMEM;
        }

	if (likely(tv != NULL)) {
                do_gettimeofday(&get_time->tv);
                if (copy_to_user(tv, &get_time->tv, sizeof(struct timeval))){
                        ret= -EFAULT;
                        goto out;
                }
        }
        if (unlikely(tz != NULL)) {
                memcpy(&get_time->tz, &sys_tz, sizeof(struct timezone));
                if (copy_to_user(tz, &get_time->tz, sizeof(struct timezone))){
                        ret= -EFAULT;
                        goto out;
                }
        }

	ft_send_syscall_info_from_work(current->ft_popcorn, &current->ft_pid, current->id_syscall, (char*) get_time, sizeof(*get_time));

out: 
	if(get_time)
		kfree(get_time);

	return ret;

}

static long ft_gettimeofday_primary_after_secondary(struct timeval __user * tv, struct timezone __user * tz){
        long ret= 0;
	struct get_time_info *primary_info= NULL;

        FTPRINTK("%s called from pid %d\n", __func__, current->pid);

        primary_info= (struct get_time_info *)ft_get_pending_syscall_info(&current->ft_pid, current->id_syscall);

        if(!primary_info){
                return ft_gettimeofday_primary(tv, tz);
        }

	if (likely(tv != NULL)) {
                if (copy_to_user(tv, (void*) &primary_info->tv, sizeof(struct timeval))){
                        ret= -EFAULT;
                        goto out;
                }
        }

        if (unlikely(tz != NULL)) {
                if (copy_to_user(tz, (void*) &primary_info->tz, sizeof(struct timezone))){
                        ret= -EFAULT;
                        goto out;
                }
        }

out:
        kfree(primary_info);

        return 0;

}

static long ft_gettimeofday_secondary(struct timeval __user * tv, struct timezone __user * tz){
	long ret=0;
	struct get_time_info *primary_info= NULL;
	
	FTPRINTK("%s called from pid %d\n", __func__, current->pid);

	primary_info= (struct get_time_info *)ft_wait_for_syscall_info( &current->ft_pid, current->id_syscall);

	if(!primary_info){
		return ft_gettimeofday_primary(tv, tz);
	}

	if (likely(tv != NULL)) {
                if (copy_to_user(tv, (void*) &primary_info->tv, sizeof(struct timeval))){
                        ret= -EFAULT;
                        goto out;
                }
        }

        if (unlikely(tz != NULL)) {
                if (copy_to_user(tz, (void*) &primary_info->tz, sizeof(struct timezone))){
                        ret= -EFAULT;
                        goto out;
                }
        }

out:
	kfree(primary_info);
	
	return 0;
}

long ft_gettimeofday(struct timeval __user * tv, struct timezone __user * tz){
	
	if(ft_is_primary_replica(current)){
		return ft_gettimeofday_primary(tv, tz);	
	}
	else{
		if(ft_is_secondary_replica(current)){
			return ft_gettimeofday_secondary(tv, tz);
		}
		else{
			if(ft_is_primary_after_secondary_replica(current)){
				return ft_gettimeofday_primary_after_secondary(tv, tz);
			}
			else
				return -EFAULT;
		}

	}

        return 0;

}
