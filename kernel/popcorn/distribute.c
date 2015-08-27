
#include "popcorn.h"

void end_distribute_operation(int operation, long start_ret, unsigned long addr, int _cpu, wait_queue_head_t *request_distributed_vma_op) {
	int i;

	if (current->mm->distribute_unmap == 0) {
		return;
	}

	PSVMAPRINTK("Ending distributed vma operation %i pid %d\n", operation,current->pid);

	if (current->mm->distr_vma_op_counter <= 0
			|| (current->main == 0 && current->mm->distr_vma_op_counter > 2)
			|| (current->main == 1 && current->mm->distr_vma_op_counter > 3))
		printk("ERROR: exiting from a distributed vma operation with distr_vma_op_counter = %i\n",
				current->mm->distr_vma_op_counter);

	(current->mm->distr_vma_op_counter)--;

	if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		if (current->mm->was_not_pushed <= 0)
			printk("ERROR: exiting from a mapping operation with was_not_pushed = %i\n",
					current->mm->was_not_pushed);
		current->mm->was_not_pushed--;
	}

	memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
			current->tgroup_home_id);
	if (entry == NULL) {
		printk("ERROR: Cannot find message to send in exit operation\n");
	}

	if (start_ret == VMA_OP_SAVE) {

		/*if(operation!=VMA_OP_MAP ||operation!=VMA_OP_REMAP ||operation!=VMA_OP_BRK )
		  printk("ERROR: asking for saving address from operation %i",operation);
		 */
		if (_cpu != current->tgroup_home_cpu)
			printk("ERROR: asking for saving address from a client");

		//now I have the new address I can send the message
		if (entry->message_push_operation != NULL) {

			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (current->main == 0)
					printk("ERROR: server not main asked to save operation\n");

				entry->message_push_operation->addr = addr;
			} else {

				if (operation == VMA_OP_REMAP) {
					entry->message_push_operation->new_addr = addr;
				} else
					printk("ERROR: asking for saving address from a wrong operation\n");
			}

			up_write(&current->mm->mmap_sem);

			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (pcn_kmsg_send_long(entry->message_push_operation->from_cpu,
							(struct pcn_kmsg_long_message*) (entry->message_push_operation),
							sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr))
						== -1) {
					printk("ERROR: impossible to send operation to client in cpu %d\n",
							entry->message_push_operation->from_cpu);
				} else {
					PSVMAPRINTK("%s, operation %d sent to cpu %d \n",__func__,operation, entry->message_push_operation->from_cpu);
				}

			} else {
				PSVMAPRINTK("%s,sending operation %d to all \n",__func__,operation);
#ifndef SUPPORT_FOR_CLUSTERING
				for(i = 0; i < MAX_KERNEL_IDS; i++) {
					// Skip the current cpu
					if(i == _cpu) continue;

#else
					// the list does not include the current processor group descirptor (TODO)
					struct list_head *iter;
					_remote_cpu_info_list_t *objPtr;
					list_for_each(iter, &rlist_head) {
						objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						i = objPtr->_data._processor;
#endif
						if(entry->kernel_set[i]==1)
							pcn_kmsg_send_long(i,
									(struct pcn_kmsg_long_message*) (entry->message_push_operation),
									sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));

					}
				}

				down_write(&current->mm->mmap_sem);

				if (current->main == 0) {

					kfree(entry->message_push_operation);
					entry->message_push_operation = NULL;
				}

			} else {
				printk("ERROR: Cannot find message to send in exit operation\n");

			}
		}

		if (current->mm->distr_vma_op_counter == 0) {

			current->mm->thread_op = NULL;

			entry->my_lock = 0;

			if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
				PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
				current->mm->vma_operation_index++;
			}

			PSVMAPRINTK("Releasing distributed lock\n");
			up_write(&current->mm->distribute_sem);

			if(_cpu == current->tgroup_home_cpu && !(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
				up_read(&entry->kernel_set_sem);
			}

			wake_up(request_distributed_vma_op);

		} else
			if (current->mm->distr_vma_op_counter == 1
					&& _cpu == current->tgroup_home_cpu && current->main == 1) {

				if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
					PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
					current->mm->vma_operation_index++;
					up_read(&entry->kernel_set_sem);
				}

				PSVMAPRINTK("Releasing distributed lock\n");
				up_write(&current->mm->distribute_sem);

			}else{
				if (!(current->mm->distr_vma_op_counter == 1
							&& _cpu != current->tgroup_home_cpu && current->main == 1)) {

					//nested operation
					if(operation != VMA_OP_UNMAP)
						printk("ERROR exiting fom a nest operation that is %d",operation);

					//nested operation do not release the lock
					PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
					current->mm->vma_operation_index++;
				}

			}

		PSVMAPRINTK("operation index is %d\n", current->mm->vma_operation_index);
		if(strcmp(current->comm,"IS") == 0){
			trace_printk("e\n");
		}

	}


/*I assume that down_write(&mm->mmap_sem) is held
 *There are two different protocols:
 *mmap and brk need to only contact the server,
 *all other operations (remap, mprotect, unmap) need that the server pushes it in the system
 */
long start_distribute_operation(int operation, unsigned long addr, size_t len,
		unsigned long prot, unsigned long new_addr, unsigned long new_len,
		unsigned long flags, struct file *file, unsigned long pgoff,
		int _cpu, wait_queue_head_t *request_distributed_vma_op) {

	long ret;
	int server;

	if (current->tgroup_home_cpu != _cpu)
		server = 0;
	else
		server = 1;

	//set default ret
	if (server)
		ret = VMA_OP_NOT_SAVE;
	else if (operation == VMA_OP_REMAP)
		ret = new_addr;
	else
		ret = addr;

	/*Operations can be nested-called.
	 * MMAP->UNMAP
	 * BR->UNMAP
	 * MPROT->/
	 * UNMAP->/
	 * MREMAP->UNMAP
	 * =>only UNMAP can be nested-called
	 *
	 * If this is an unmap nested-called by an operation pushed in the system,
	 * skip the distribution part.
	 *
	 * If this is an unmap nested-called by an operation not pushed in the system,
	 * and I am the server, push it in the system.
	 *
	 * If this is an unmap nested-called by an operation not pushed in the system,
	 * and I am NOT the server, it is an error. The server should have pushed that unmap
	 * before, if I am executing it again something is wrong.
	 */

	/*All the operation pushed by the server are executed as not distributed in clients*/
	if (current->mm->distribute_unmap == 0) {
		return ret;
	}
	if(strcmp(current->comm,"IS") == 0){
		trace_printk("s\n");
	}

	PSVMAPRINTK("%s, Starting vma operation for pid %i tgroup_home_cpu %i tgroup_home_id %i main %d operation %i addr %lu len %lu end %lu\n",
			__func__, current->pid, current->tgroup_home_cpu, current->tgroup_home_id, current->main?1:0, operation, addr, len, addr+len);


	/*only server can have legal distributed nested operations*/
	if (current->mm->distr_vma_op_counter > 0
			&& current->mm->thread_op == current) {


		PSVMAPRINTK("%s, Recursive operation\n",__func__);

		if (server == 0
				|| (current->main == 0 && current->mm->distr_vma_op_counter > 1)
				|| (current->main == 0 && operation != VMA_OP_UNMAP)) {
			printk("ERROR: invalid nested vma operation %i\n", operation);
			return -EPERM;
		} else
			/*the main executes the operations for the clients
			 *distr_vma_op_counter is already increased when it start the operation*/
			if (current->main == 1) {

				PSVMAPRINTK("%s, I am the main, so it maybe not a real recursive operation...\n",__func__);

				if (current->mm->distr_vma_op_counter < 1
						|| current->mm->distr_vma_op_counter > 2
						|| (current->mm->distr_vma_op_counter == 2
							&& operation != VMA_OP_UNMAP)) {
					printk("ERROR: invalid nested vma operation in main server\n");
					return -EPERM;
				} else
					if (current->mm->distr_vma_op_counter == 2){

						PSVMAPRINTK("%s, Recursive operation for the main\n",__func__);
						/*in this case is a nested operation on main
						 * if the previous operation was a pushed operation
						 * do not distribute it again*/
						if (current->mm->was_not_pushed == 0) {
							current->mm->distr_vma_op_counter++;
							PSVMAPRINTK("%s, don't ditribute again, return!\n",__func__);
							return ret;
						} else
							goto start;
					}
					else
						goto start;
			} else
				if (current->mm->was_not_pushed == 0) {
					current->mm->distr_vma_op_counter++;
					PSVMAPRINTK("%s, don't ditribute again, return!\n",__func__);
					return ret;
				} else
					goto start;

	}

	/* I did not start an operation, but another thread maybe did...
	 * => no concurrent operations of the same process on the same kernel*/
	while (current->mm->distr_vma_op_counter > 0) {

		PSVMAPRINTK("%s Somebody already started a distributed operation (current->mm->thread_op->pid is %d). I am pid %d and I am going to sleep\n",
				__func__,current->mm->thread_op->pid,current->pid);

		up_write(&current->mm->mmap_sem);

		DEFINE_WAIT(wait);
		prepare_to_wait(request_distributed_vma_op, &wait,
				TASK_UNINTERRUPTIBLE);

		if (current->mm->distr_vma_op_counter > 0) {
			schedule();
		}

		finish_wait(request_distributed_vma_op, &wait);

		down_write(&current->mm->mmap_sem);

	}

	if (current->mm->distr_vma_op_counter != 0) {
		printk("ERROR: a new vma operation can be started, but distr_vma_op_counter is %i\n",
				current->mm->distr_vma_op_counter);
		return -EPERM;
	}

start: current->mm->distr_vma_op_counter++;
       current->mm->thread_op = current;

       if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
	       current->mm->was_not_pushed++;
       }

       if (server) {
	       //SERVER

	       if (current->main == 1 && !(current->mm->distr_vma_op_counter>2)) {

		       /*I am the main thread=>
			* a client asked me to do an operation.
			*/

		       //(current->mm->vma_operation_index)++;
		       int index = current->mm->vma_operation_index;

		       PSVMAPRINTK("SERVER MAIN: starting operation %d, current index is %d \n", operation, index);

		       up_write(&current->mm->mmap_sem);

		       memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
				       current->tgroup_home_id);
		       if (entry == NULL || entry->message_push_operation == NULL) {
			       printk("ERROR: Mapping disappeared or cannot find message to update \n");
			       down_write(&current->mm->mmap_sem);
			       ret = -ENOMEM;
			       goto out;
		       }

		       /*First: send a message to everybody to acquire the lock to block page faults*/
		       vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
				       GFP_ATOMIC);
		       if (lock_message == NULL) {
			       down_write(&current->mm->mmap_sem);
			       ret = -ENOMEM;
			       goto out;
		       }
		       lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
		       lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		       lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
		       lock_message->tgroup_home_id = current->tgroup_home_id;
		       lock_message->from_cpu = entry->message_push_operation->from_cpu;
		       lock_message->vma_operation_index = index;

		       vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
				       sizeof(vma_op_answers_t), GFP_ATOMIC);
		       if (acks == NULL) {
			       kfree(lock_message);
			       down_write(&current->mm->mmap_sem);
			       ret = -ENOMEM;
			       goto out;
		       }
		       acks->tgroup_home_cpu = current->tgroup_home_cpu;
		       acks->tgroup_home_id = current->tgroup_home_id;
		       acks->vma_operation_index = index;
		       acks->waiting = current;
		       acks->responses = 0;
		       acks->expected_responses = 0;
		       raw_spin_lock_init(&(acks->lock));

		       add_vma_ack_entry(acks);

		       int i, error;

		       /*Partial replication: mmap and brk need to communicate only between server and one client
			* */
		       if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {

			       error = pcn_kmsg_send_long(
					       entry->message_push_operation->from_cpu,
					       (struct pcn_kmsg_long_message*) (lock_message),
					       sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
			       if (error != -1) {
				       acks->expected_responses++;
			       }

		       } else {

			       down_read(&entry->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
			       for(i = 0; i < MAX_KERNEL_IDS; i++) {
				       // Skip the current cpu
				       if(i == _cpu) continue;

#else
				       // the list does not include the current processor group descirptor (TODO)
				       struct list_head *iter;
				       _remote_cpu_info_list_t *objPtr;
				       list_for_each(iter, &rlist_head) {
					       objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					       i = objPtr->_data._processor;
#endif
					       if(entry->kernel_set[i]==1){
						       if (current->mm->distr_vma_op_counter == 3
								       && i == entry->message_push_operation->from_cpu)
							       continue;

						       error = pcn_kmsg_send_long(i,
								       (struct pcn_kmsg_long_message*) (lock_message),sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
						       if (error != -1) {
							       acks->expected_responses++;
						       }
					       }
				       }
			       }

			       while (acks->expected_responses != acks->responses) {

				       set_task_state(current, TASK_UNINTERRUPTIBLE);

				       if (acks->expected_responses != acks->responses) {
					       schedule();
				       }

				       set_task_state(current, TASK_RUNNING);

			       }

			       PSVMAPRINTK("SERVER MAIN: Received all ack to lock\n");

			       unsigned long flags;
			       raw_spin_lock_irqsave(&(acks->lock), flags);
			       raw_spin_unlock_irqrestore(&(acks->lock), flags);

			       remove_vma_ack_entry(acks);

			       entry->message_push_operation->vma_operation_index = index;

			       /*I acquire the lock to block page faults too
				*Important: this should happen before sending the push message or executing the operation*/
			       if (current->mm->distr_vma_op_counter == 2) {
				       down_write(&current->mm->distribute_sem);
				       PSVMAPRINTK("local distributed lock acquired\n");
			       }

			       /* Third: push the operation to everybody
				* If the operation was a mmap,brk or remap without fixed parameters, I cannot let other kernels
				* locally choose where to remap it =>
				* I need to push what the server choose as parameter to the other an push the operation with
				* a fixed flag.
				* */
			       if (operation == VMA_OP_UNMAP || operation == VMA_OP_PROTECT
					       || ((operation == VMA_OP_REMAP) && (flags & MREMAP_FIXED))) {

				       PSVMAPRINTK("SERVER MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
#ifndef SUPPORT_FOR_CLUSTERING
				       for(i = 0; i < MAX_KERNEL_IDS; i++) {
					       // Skip the current cpu
					       if(i == _cpu) continue;
#else
					       // the list does not include the current processor group descirptor (TODO)
					       struct list_head *iter;
					       _remote_cpu_info_list_t *objPtr;
					       list_for_each(iter, &rlist_head) {
						       objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						       i = objPtr->_data._processor;
#endif
						       if(entry->kernel_set[i]==1)
							       pcn_kmsg_send_long(i,
									       (struct pcn_kmsg_long_message*) (entry->message_push_operation),sizeof(vma_operation_t)- sizeof(struct pcn_kmsg_hdr));

					       }

					       kfree(lock_message);
					       kfree(acks);

					       down_write(&current->mm->mmap_sem);

					       return ret;

				       } else {
					       PSVMAPRINTK("SERVER MAIN: going to execute the operation locally %d\n",operation);
					       kfree(lock_message);
					       kfree(acks);

					       down_write(&current->mm->mmap_sem);

					       return VMA_OP_SAVE;

				       }

			       } else {
				       //server not main
				       PSVMAPRINTK("SERVER NOT MAIN: Starting operation %d for pid %d current index is %d\n", operation, current->pid, current->mm->vma_operation_index);

				       switch (operation) {

					       case VMA_OP_MAP:
					       case VMA_OP_BRK:
						       //if I am the server, mmap and brk can be executed locally
						       PSVMAPRINTK("%s pure local operation!\n",__func__);
						       //Note: the order in which locks are taken is important
						       up_write(&current->mm->mmap_sem);

						       down_write(&current->mm->distribute_sem);
						       PSVMAPRINTK("Distributed lock acquired\n");
						       down_write(&current->mm->mmap_sem);

						       //(current->mm->vma_operation_index)++;

						       return ret;

					       default:
						       break;

				       }

				       //new push-operation
				       PSVMAPRINTK("%s push operation!\n",__func__);

				       //(current->mm->vma_operation_index)++;
				       int index = current->mm->vma_operation_index;
				       PSVMAPRINTK("current index is %d\n", index);

				       /*Important: while I am waiting for the acks to the LOCK message
					* mmap_sem have to be unlocked*/
				       up_write(&current->mm->mmap_sem);

				       /*First: send a message to everybody to acquire the lock to block page faults*/
				       vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
						       GFP_ATOMIC);
				       if (lock_message == NULL) {
					       down_write(&current->mm->mmap_sem);
					       ret = -ENOMEM;
					       goto out;
				       }
				       lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
				       lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
				       lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
				       lock_message->tgroup_home_id = current->tgroup_home_id;
				       lock_message->from_cpu = _cpu;
				       lock_message->vma_operation_index = index;

				       vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
						       sizeof(vma_op_answers_t), GFP_ATOMIC);
				       if (acks == NULL) {
					       kfree(lock_message);
					       down_write(&current->mm->mmap_sem);
					       ret = -ENOMEM;
					       goto out;
				       }
				       acks->tgroup_home_cpu = current->tgroup_home_cpu;
				       acks->tgroup_home_id = current->tgroup_home_id;
				       acks->vma_operation_index = index;
				       acks->waiting = current;
				       acks->responses = 0;
				       acks->expected_responses = 0;
				       raw_spin_lock_init(&(acks->lock));
				       int i, error;

				       memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
						       current->tgroup_home_id);
				       if (entry==NULL) {
					       printk("ERROR: Mapping disappeared, cannot save message to update by exit_distribute_operation\n");
					       kfree(lock_message);
					       kfree(acks);
					       down_write(&current->mm->mmap_sem);
					       ret = -EPERM;
					       goto out;
				       }

				       add_vma_ack_entry(acks);

				       down_read(&entry->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
				       for(i = 0; i < MAX_KERNEL_IDS; i++) {
					       // Skip the current cpu
					       if(i == _cpu) continue;
#else
					       // the list does not include the current processor group descirptor (TODO)
					       struct list_head *iter;
					       _remote_cpu_info_list_t *objPtr;
					       list_for_each(iter, &rlist_head) {
						       objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						       i = objPtr->_data._processor;
#endif

						       if(entry->kernel_set[i]==1){
							       error = pcn_kmsg_send_long(i,
									       (struct pcn_kmsg_long_message*) (lock_message),	sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
							       if (error != -1) {
								       acks->expected_responses++;
							       }
						       }
					       }

					       /*Second: wait that everybody acquire the lock, and acquire it locally too*/
					       while (acks->expected_responses != acks->responses) {

						       set_task_state(current, TASK_UNINTERRUPTIBLE);

						       if (acks->expected_responses != acks->responses) {
							       schedule();
						       }

						       set_task_state(current, TASK_RUNNING);

					       }

					       PSVMAPRINTK("SERVER NOT MAIN: Received all ack to lock\n");

					       unsigned long flags;
					       raw_spin_lock_irqsave(&(acks->lock), flags);
					       raw_spin_unlock_irqrestore(&(acks->lock), flags);

					       remove_vma_ack_entry(acks);

					       vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
							       sizeof(vma_operation_t), GFP_ATOMIC);
					       if (operation_to_send == NULL) {
						       down_write(&current->mm->mmap_sem);
						       up_read(&entry->kernel_set_sem);
						       kfree(lock_message);
						       kfree(acks);
						       ret = -ENOMEM;
						       goto out;
					       }

					       operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
					       operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;

					       operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
					       operation_to_send->tgroup_home_id = current->tgroup_home_id;
					       operation_to_send->operation = operation;
					       operation_to_send->addr = addr;
					       operation_to_send->new_addr = new_addr;
					       operation_to_send->len = len;
					       operation_to_send->new_len = new_len;
					       operation_to_send->prot = prot;
					       operation_to_send->flags = flags;
					       operation_to_send->vma_operation_index = index;
					       operation_to_send->from_cpu = _cpu;

					       /*I acquire the lock to block page faults too
						*Important: this should happen before sending the push message or executing the operation*/
					       if (current->mm->distr_vma_op_counter == 1) {
						       down_write(&current->mm->distribute_sem);
						       PSVMAPRINTK("Distributed lock acquired locally\n");
					       }


					       /* Third: push the operation to everybody
						* If the operation was a remap without fixed parameters, I cannot let other kernels
						* locally choose where to remap it =>
						* I need to push what the server choose as parameter to the other an push the operation with
						* a fixed flag.
						* */
					       if (!(operation == VMA_OP_REMAP) || (flags & MREMAP_FIXED)) {

						       PSVMAPRINTK("SERVER NOT MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
#ifndef SUPPORT_FOR_CLUSTERING
						       for(i = 0; i < MAX_KERNEL_IDS; i++) {
							       // Skip the current cpu
							       if(i == _cpu) continue;
#else
							       // the list does not include the current processor group descirptor (TODO)
							       struct list_head *iter;
							       _remote_cpu_info_list_t * objPtr;
							       list_for_each(iter, &rlist_head) {
								       objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
								       i = objPtr->_data._processor;
#endif
								       if(entry->kernel_set[i]==1)
									       pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) (operation_to_send),
											       sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));

							       }

							       kfree(lock_message);
							       kfree(operation_to_send);
							       kfree(acks);

							       down_write(&current->mm->mmap_sem);

							       return ret;

						       } else {
							       PSVMAPRINTK("SERVER NOT MAIN: going to execute the operation locally %d\n",operation);
							       entry->message_push_operation = operation_to_send;

							       kfree(lock_message);
							       kfree(acks);

							       down_write(&current->mm->mmap_sem);

							       return VMA_OP_SAVE;
						       }

					       }

				       } else {
					       //CLIENT
					       PSVMAPRINTK("CLIENT: starting operation %i for pid %d current index is%d\n", operation, current->pid, current->mm->vma_operation_index);


					       /*First: send the operation to the server*/
					       vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
							       sizeof(vma_operation_t), GFP_ATOMIC);
					       if (operation_to_send == NULL) {
						       ret = -ENOMEM;
						       goto out;
					       }
					       operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
					       operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;

					       operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
					       operation_to_send->tgroup_home_id = current->tgroup_home_id;
					       operation_to_send->operation = operation;
					       operation_to_send->addr = addr;
					       operation_to_send->new_addr = new_addr;
					       operation_to_send->len = len;
					       operation_to_send->new_len = new_len;
					       operation_to_send->prot = prot;
					       operation_to_send->flags = flags;
					       operation_to_send->vma_operation_index = -1;
					       operation_to_send->from_cpu = _cpu;
					       operation_to_send->pgoff = pgoff;
					       if (file != NULL) {
						       char path[256] = { 0 };
						       char* rpath;
						       rpath = d_path(&file->f_path, path, 256);
						       strcpy(operation_to_send->path, rpath);
					       } else
						       operation_to_send->path[0] = '\0';

					       /*In this case the server will eventually send me the push operation.
						*Differently from a not-started-by-me push operation, it is not the main thread that has to execute it,
						*but this thread has.
						*/
					       memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
							       current->tgroup_home_id);
					       if (entry) {

						       if (entry->waiting_for_op != NULL) {
							       printk("ERROR: Somebody is already waiting for an op\n");
							       kfree(operation_to_send);
							       ret = -EPERM;
							       goto out;
						       }

						       entry->waiting_for_op = current;
						       entry->arrived_op = 0;

					       } else {
						       printk("ERROR: Mapping disappeared, cannot wait for push op\n");
						       kfree(operation_to_send);
						       ret = -EPERM;
						       goto out;
					       }

					       up_write(&current->mm->mmap_sem);

					       int error;
					       //send the operation to the server
					       error = pcn_kmsg_send_long(current->tgroup_home_cpu,
							       (struct pcn_kmsg_long_message*) (operation_to_send),
							       sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr));
					       if (error == -1) {
						       printk("Impossible to contact the server");
						       kfree(operation_to_send);
						       down_write(&current->mm->mmap_sem);
						       ret = -EPERM;
						       goto out;
					       }

					       /*Second: the server will send me a LOCK message... another thread will handle it...*/

					       /*Third: wait that the server push me the operation*/
					       while (entry->arrived_op == 0) {

						       set_task_state(current, TASK_UNINTERRUPTIBLE);

						       if (entry->arrived_op == 0) {
							       schedule();
						       }

						       set_task_state(current, TASK_RUNNING);
					       }

					       PSVMAPRINTK("My operation finally arrived pid %d vma operation %d\n",current->pid,current->mm->vma_operation_index);

					       /*Note, the distributed lock already has been acquired*/
					       down_write(&current->mm->mmap_sem);

					       if (current->mm->thread_op != current) {
						       printk(	"ERROR: waking up to locally execute a vma operation started by me, but thread_op s not me\n");
						       kfree(operation_to_send);
						       ret = -EPERM;
						       goto out_dist_lock;
					       }

					       if (operation == VMA_OP_REMAP || operation == VMA_OP_MAP
							       || operation == VMA_OP_BRK) {
						       ret = entry->addr;
						       if (entry->addr < 0) {
							       printk("Received error %lu from the server for operation %d\n", ret,operation);
							       goto out_dist_lock;
						       }
					       }

					       entry->waiting_for_op = NULL;

					       kfree(operation_to_send);

					       return ret;

				       }

out_dist_lock:

				       up_write(&current->mm->distribute_sem);
				       PSVMAPRINTK("Released distributed lock from out_dist_lock\n");
				       PSVMAPRINTK("current index is %d in out_dist_lock\n", current->mm->vma_operation_index);

out: current->mm->distr_vma_op_counter--;
     current->mm->thread_op = NULL;
     if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
	     current->mm->was_not_pushed--;
     }

     wake_up(request_distributed_vma_op);
     return ret;
}
