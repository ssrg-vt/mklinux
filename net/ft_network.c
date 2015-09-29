/*
 * ft_network.c
 *
 * Author: Marina
 */

#include <linux/ft_replication.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/net.h>
#include <net/sock.h>

#define FT_NET_VERBOSE 0
#if FT_NET_VERBOSE
#define FTPRINTK(...) printk(__VA_ARGS__)
#else
#define FTPRINTK(...) ;
#endif

struct send_fam_info{
	int size;
	__wsum csum;
	int ret;
};

struct rcv_fam_info_before{
        int size;
        int src_addr_size;
	void __user *ubuf;
	void *src_addr;
	int flags;
};

struct rcv_fam_info{
        int size;
	int flags;
        __wsum csum;
        int ret;
	//NOTE this must be the last field;
	char data;
};

static int after_syscall_rcv_family_primary(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int ret){

        struct rcv_fam_info *syscall_info;
	struct rcv_fam_info_before *store_info; 
	int data_size;
	char* where_to_copy;
	int err;

        FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);
	
	store_info= (struct rcv_fam_info_before*) current->useful;
        if(!store_info){
                printk("ERROR: %s current->useful (pid %d) is NULL\n", __func__, current->pid);
                return -EFAULT;
        }
      	else
		current->useful= NULL;


	/* in case without errors ret is the actual number of bytes copied.
	 * size is the maximum bytes allowed to copy.
	 */
	if(ret>0)
		data_size= ret;
	else	
		data_size= 0;

	syscall_info= kmalloc( sizeof(*syscall_info) + data_size+ 1, GFP_KERNEL);
	if(!syscall_info)
		return -ENOMEM;

	syscall_info->size= size;
	syscall_info->flags= flags;
	syscall_info->ret= ret;

	/* TODO a copy is not needed. It is sent just as a first test.
         * the data can be retrieved from the secondary from the packet forwarded to the stable buffer.
         */
	syscall_info->csum= 0;
	if(data_size){
			where_to_copy= &syscall_info->data;	
			syscall_info->csum= csum_and_copy_from_user(store_info->ubuf, where_to_copy, data_size, syscall_info->csum, &err);
			if(err){
				printk("ERROR: %s copy_from_user failed\n", __func__);
				goto out;
			}
			where_to_copy[data_size]='\0';
			FTPRINTK("%s: data %s size %d\n", __func__, where_to_copy, data_size);
	}
	
	/*TODO
         * NOTE: for tcp msg it is not important
         * but for udp msg the fields  msg_name/msg_namelen should be copied too.
         */

        /* NOTE: multiple threads could call rcv simultaneusly. On tcp_rcvmsg the socket is locked therefore they are serialized. 
	 * In here the lock already have been released, but the data was copied while holding it.
	 * In secondary replicas, if retriving data from the stable buffer, the same order of access to the stable buffer must be ensured.
	 */
	FTPRINTK("%s pid %d syscall_id %d sending size %d flags %d csum %d ret %d \n", __func__, current->pid, current->id_syscall, syscall_info->size, syscall_info->flags, syscall_info->csum, syscall_info->ret);
        //ft_send_syscall_info_from_work(current->ft_popcorn, &current->ft_pid, current->id_syscall, (char*) syscall_info, sizeof(*syscall_info)+ data_size);
	ft_send_syscall_info(current->ft_popcorn, &current->ft_pid, current->id_syscall, (char*) syscall_info, sizeof(*syscall_info)+ data_size);

out:
        kfree(syscall_info);
	kfree(store_info);
	
        return FT_SYSCALL_CONTINUE;
}

static int after_syscall_rcv_family_replicated_sock(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int ret){

        if(ft_is_primary_replica(current)){
                return after_syscall_rcv_family_primary(iocb, sock, msg, size, flags, ret);
        }

        if(ft_is_secondary_replica(current)){
                printk("ERROR: %s current (pid %d) is a secondary replica (it should have stop the syscall of the 'before' part\n", __func__, current->pid);
                return -EFAULT;
        }

        printk("ERROR: %s current (pid %d) is not primary or secondar replica \n", __func__, current->pid);
        return -EFAULT;

}


int ft_after_syscall_rcv_family(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int ret){

        if(ft_is_replicated(current)){
                return after_syscall_rcv_family_replicated_sock(iocb, sock, msg, size, flags, ret);
        }

        return FT_SYSCALL_CONTINUE;
}

static int before_syscall_rcv_family_primary(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int* ret){

	struct rcv_fam_info_before *store_info;

	FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);

	if(msg->msg_iovlen!=1){
                printk("ERROR %s iovlen is %d\n", __func__, (int) msg->msg_iovlen);
		return -EFAULT;
	}

	store_info= kmalloc(sizeof(*store_info), GFP_KERNEL);
	if(!store_info)
		return -ENOMEM;

	store_info->size= size;
	store_info->flags= flags;
	store_info->ubuf= msg->msg_iov->iov_base;

	store_info->src_addr_size= msg->msg_namelen;
	store_info->src_addr= msg->msg_name;
	
	if(current->useful!=NULL)
                printk("WARNING: %s going to use current->useful of pid %d but it is not NULL\n", __func__, current->pid);

        current->useful= (void*) store_info;

        return FT_SYSCALL_CONTINUE;

}

static int before_syscall_rcv_family_secondary(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int* syscall_ret){

	struct sock *sk;
        struct rcv_fam_info *syscall_info_primary= NULL;
	int data_size;
	__user char* ubuf;
        __wsum my_csum;
        int err, ret= FT_SYSCALL_DROP;

        FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);

        syscall_info_primary= (struct rcv_fam_info *) ft_wait_for_syscall_info(&current->ft_pid, current->id_syscall);
        if(!syscall_info_primary){
                printk("ERROR: %s for pid %d no rcv info from primary\n", __func__, current->pid);
                return -EFAULT;
        }

        if(syscall_info_primary->ret > 0)
                data_size= syscall_info_primary->ret;
        else
                data_size= 0;

	if(syscall_info_primary->size != size){
                printk("ERROR: %s for pid %d size of rcv (syscall id %d) not matching between primary(%d) and secondary(%d)\n", __func__, current->pid, current->id_syscall, syscall_info_primary->size, (int) size);
		ret= -EFAULT;
   		goto out;
	 }

	if(syscall_info_primary->flags != flags){
                printk("ERROR: %s for pid %d flags of rcv (syscall id %d) not matching between primary(%d) and secondary(%d)\n", __func__, current->pid, current->id_syscall, syscall_info_primary->flags, flags);
		ret= -EFAULT;
        	goto out;
	}

	my_csum= 0;
        ubuf= msg->msg_iov->iov_base;
	if(data_size){
			sk= sock->sk;
		        if(!sk || !sk->ft_filter){
                		printk("ERROR: %s sock struct or sk->ft_filter is NULL\n", __func__);
                		ret= -EFAULT;
				goto out;
        		}
		
			err= remove_and_copy_from_stable_buffer(sk->ft_filter->stable_buffer, msg->msg_iov, data_size);
			if(err < 0){
				ret= err;
				goto out;
			}
			
			if(err != data_size){
				printk("ERROR: %s asked %d bytes from stable buffere but received %d\n", __func__, data_size, err);
				ret= -EFAULT;
				goto out;
			}

			char* app= kmalloc(data_size+1, GFP_KERNEL);
			if(!app){
				ret= -ENOMEM;
				goto out;
			}
                        my_csum= csum_and_copy_from_user(ubuf, app, data_size, my_csum, &err);
			if(err){
				printk("ERROR: %s copy_from_user failed\n", __func__);
                        	ret= -EFAULT;
				kfree(app);
			       	goto out;
			}
                       	app[data_size]='\0';
			FTPRINTK("%s: data %s size %d\n", __func__, app, data_size);
			kfree(app);
			
                
        }
	
	/*TODO
	 * NOTE: for tcp msg it is not important
	 * but for udp msg the fields msg_name/msg_namelen should be copied too.
	 */

	if(my_csum != syscall_info_primary->csum){
                printk("ERROR: %s for pid %d csum of send (syscall id %d) not matching between primary(%d) and secondary(%d)\n", __func__, current->pid, current->id_syscall, syscall_info_primary->csum, my_csum);
        	ret= -EFAULT;
	}

out:
        *syscall_ret= syscall_info_primary->ret;

        kfree(syscall_info_primary);

        return ret;
}

static int before_syscall_rcv_family_replicated_sock(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int* ret){

        /* Just a check to be sure that the sock that is using is replicated too...
         *
         */
        struct sock *sk= sock->sk;
        if(!sk || !sk->ft_filter){
                printk("ERROR: %s current is replicated (pid %d) but sock is not\n", __func__, current->pid);
                return -EFAULT;
        }

        if(ft_is_primary_replica(current)){
                return before_syscall_rcv_family_primary(iocb, sock, msg, size, flags, ret);
        }

        if(ft_is_secondary_replica(current)){
                return before_syscall_rcv_family_secondary(iocb, sock, msg, size, flags, ret);
        }

        printk("ERROR: %s current (pid %d) is not primary or secondar replica \n", __func__, current->pid);
        return -EFAULT;

}


int ft_before_syscall_rcv_family(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags, int* ret){

        if(ft_is_replicated(current)){
                return before_syscall_rcv_family_replicated_sock(iocb, sock, msg, size, flags, ret);
        }

        return FT_SYSCALL_CONTINUE;
}

static int after_syscall_send_family_primary(int ret){
	struct send_fam_info *syscall_info;
	
	FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);

	syscall_info= (struct send_fam_info*) current->useful;
	if(!syscall_info){
		printk("ERROR: %s current->useful (pid %d) is NULL\n", __func__, current->pid);
                return -EFAULT;
	}
	syscall_info->ret= ret;

	FTPRINTK("%s pid %d syscall_id %d sending size %d csum %d ret %d \n", __func__, current->pid, current->id_syscall, syscall_info->size, syscall_info->csum, syscall_info->ret);
	
	ft_send_syscall_info(current->ft_popcorn, &current->ft_pid, current->id_syscall, (char*) syscall_info, sizeof(*syscall_info));
	
	kfree(syscall_info);
	current->useful= NULL;
	
	return FT_SYSCALL_CONTINUE;
}

static int after_syscall_send_family_replicated_sock(int ret){

	if(ft_is_primary_replica(current)){
                return after_syscall_send_family_primary(ret);
        }

        if(ft_is_secondary_replica(current)){
                printk("ERROR: %s current (pid %d) is a secondary replica (it should have stop the syscall of the 'before' part\n", __func__, current->pid);
        	return -EFAULT;
        }

	printk("ERROR: %s current (pid %d) is not primary or secondar replica \n", __func__, current->pid);
	return -EFAULT;

}


int ft_after_syscall_send_family(int ret){

	if(ft_is_replicated(current)){
		return after_syscall_send_family_replicated_sock(ret);
	}
	
	return FT_SYSCALL_CONTINUE;
}

static int before_syscall_send_family_secondary(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int* ret){

        struct send_fam_info *sycall_info_primary= NULL;
	struct iovec *iov;
	int iovlen, i, err;
	__wsum my_csum;

	FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);

	sycall_info_primary= (struct send_fam_info *) ft_wait_for_syscall_info(&current->ft_pid, current->id_syscall);
	if(!sycall_info_primary){
		printk("ERROR: %s for pid %d no send info from primary\n", __func__, current->pid);
		return -EFAULT;
	}

	if(sycall_info_primary->size != size){
		printk("ERROR: %s for pid %d size of send (syscall id %d) not matching between primary(%d) and secondary(%d)\n", __func__, current->pid, current->id_syscall, sycall_info_primary->size, (int) size);
		goto out;
	}
	
	my_csum= 0;
	iovlen = msg->msg_iovlen;
        iov = msg->msg_iov;

	err= insert_in_send_buffer_and_csum(sock->sk->ft_filter->send_buffer, iov, iovlen, size, &my_csum);
	if(err){
		printk("ERROR %s Impossible to insert in send buffer\n", __func__);
		goto out;
	}

	if(my_csum != sycall_info_primary->csum){
		printk("ERROR: %s for pid %d csum of send (syscall id %d) not matching between primary(%d) and secondary(%d)\n", __func__, current->pid, current->id_syscall, sycall_info_primary->csum, my_csum);
	}

out:
	*ret= sycall_info_primary->ret;

	kfree(sycall_info_primary);

	return FT_SYSCALL_DROP;
}

static int before_syscall_send_family_primary(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size){
	struct send_fam_info *syscall_info;
	 struct iovec *iov;
        int iovlen, i, err;
	
	FTPRINTK("%s started for pid %d syscall_id %d\n", __func__, current->pid, current->id_syscall);

	syscall_info= kmalloc(sizeof(*syscall_info), GFP_KERNEL);
	if(!syscall_info)
		return -ENOMEM;

	//calculate hash
	syscall_info->csum= 0;
        iovlen = msg->msg_iovlen;
        iov = msg->msg_iov;

        /* The data is in user space, so I copy it in kernel and after I perform the checksum.
         * Is it really necessary to copy it?! NOT SURE. HOPEFULLY NOT! 
         */
        for(i=0; i< iovlen; i++){
                char* app= kmalloc(iov[i].iov_len +1, GFP_KERNEL);
                syscall_info->csum= csum_and_copy_from_user(iov[i].iov_base, (void*)app, iov[i].iov_len, syscall_info->csum, &err);
        	if(err){
			printk("ERROR: %s copy_from_user failed\n", __func__);
                        kfree(app);
                        goto out;
		}
		app[iov[i].iov_len]='\0';
		printk("%s: data %s\n",__func__,app);
	        kfree(app);
        }

	syscall_info->size= size;
		
	if(current->useful!=NULL)
		printk("WARNING: %s going to use current->useful of pid %d but it is not NULL\n", __func__, current->pid);

	current->useful= (void*) syscall_info;

out:
	return FT_SYSCALL_CONTINUE;
}

static int before_syscall_send_family_replicated_sock(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int* ret){

	/* Just a check to be sure that the sock that is using is replicated too...
	 *
	 */
	struct sock *sk= sock->sk;
	if(!sk || !sk->ft_filter){
		printk("ERROR: %s current is replicated (pid %d) but sock is not\n", __func__, current->pid);
		return -EFAULT;
	}

	if(ft_is_primary_replica(current)){
                return before_syscall_send_family_primary(iocb, sock, msg, size);
        }

        if(ft_is_secondary_replica(current)){
                return before_syscall_send_family_secondary(iocb, sock, msg, size, ret);
        }

	printk("ERROR: %s current (pid %d) is not primary or secondar replica \n", __func__, current->pid);
	return -EFAULT;

}


int ft_before_syscall_send_family(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int* ret){

	if(ft_is_replicated(current)){
		return before_syscall_send_family_replicated_sock(iocb, sock, msg, size, ret);
	}
	
	return FT_SYSCALL_CONTINUE;
}

