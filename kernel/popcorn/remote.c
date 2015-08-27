
/* Functions to add,find and remove an entry from the mapping list (head:_mapping_head , lock:_mapping_head_lock)
 */

#include "popcorn.h"

#if FOR_2_KERNELS
mapping_answers_for_2_kernels_t* _mapping_head = NULL;
#else
mapping_answers_t* _mapping_head = NULL;
#endif

#if FOR_2_KERNELS
ack_answers_for_2_kernels_t* _ack_head = NULL;
#else
ack_answers_t* _ack_head = NULL;
#endif

DEFINE_RAW_SPINLOCK(_mapping_head_lock);
DEFINE_RAW_SPINLOCK(_ack_head_lock);

#if FOR_2_KERNELS

static void add_mapping_entry(mapping_answers_for_2_kernels_t* entry) {

	mapping_answers_for_2_kernels_t* curr;

#else

static void add_mapping_entry(mapping_answers_t* entry) {

		mapping_answers_t* curr;

#endif
		unsigned long flags;

		if (!entry) {
			return;
		}

		raw_spin_lock_irqsave(&_mapping_head_lock, flags);

		if (!_mapping_head) {
			_mapping_head = entry;
			entry->next = NULL;
			entry->prev = NULL;
		} else {
			curr = _mapping_head;
			while (curr->next != NULL) {
				curr = curr->next;
			}
			// Now curr should be the last entry.
			// Append the new entry to curr.
			curr->next = entry;
			entry->next = NULL;
			entry->prev = curr;
		}

		raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

	}

#if FOR_2_KERNELS

mapping_answers_for_2_kernels_t* find_mapping_entry(int cpu, int id, unsigned long address) {
	mapping_answers_for_2_kernels_t* curr = NULL;
	mapping_answers_for_2_kernels_t* ret = NULL;
#else
mapping_answers_t* find_mapping_entry(int cpu, int id, unsigned long address) {
	mapping_answers_t* curr = NULL;
	mapping_answers_t* ret = NULL;
#endif
	unsigned long flags;

	raw_spin_lock_irqsave(&_mapping_head_lock, flags);

	curr = _mapping_head;
	while (curr) {

		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
				&& curr->address == address) {
			ret = curr;
			break;
		}

		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

	return ret;
}

#if FOR_2_KERNELS
void remove_mapping_entry(mapping_answers_for_2_kernels_t* entry) {
#else
void remove_mapping_entry(mapping_answers_t* entry) {
#endif
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_mapping_head_lock, flags);

	if (_mapping_head == entry) {
		_mapping_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

}

/* Functions to add,find and remove an entry from the ack list (head:_ack_head , lock:_ack_head_lock)
 */
#if FOR_2_KERNELS
static void add_ack_entry(ack_answers_for_2_kernels_t* entry) {
	ack_answers_for_2_kernels_t* curr;
#else
static void add_ack_entry(ack_answers_t* entry) {
	ack_answers_t* curr;
#endif
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	if (!_ack_head) {
		_ack_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _ack_head;
		while (curr->next != NULL) {
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);
}

#if FOR_2_KERNELS
ack_answers_for_2_kernels_t* find_ack_entry(int cpu, int id, unsigned long address) {
	ack_answers_for_2_kernels_t* curr = NULL;
	ack_answers_for_2_kernels_t* ret = NULL;
#else

ack_answers_t* find_ack_entry(int cpu, int id, unsigned long address) {
	ack_answers_t* curr = NULL;
	ack_answers_t* ret = NULL;
#endif
	unsigned long flags;

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	curr = _ack_head;
	while (curr) {

		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
				&& curr->address == address) {
			ret = curr;
			break;
		}

		curr = curr->next;
	}
	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);
	return ret;
}

#if FOR_2_KERNELS
static void remove_ack_entry(ack_answers_for_2_kernels_t* entry) {
#else
static void remove_ack_entry(ack_answers_t* entry) {
#endif

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	if (_ack_head == entry) {
		_ack_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);

}

#if FOR_2_KERNELS
//static
int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, unsigned long page_fault_flags,
		pmd_t* pmd, pte_t* pte,
		spinlock_t* ptl, struct page* page, int _cpu) {
	pte_t value_pte;
	int ret=0,i;
	int sent;

	data_request_for_2_kernels_t* read_message;
	mapping_answers_for_2_kernels_t* reading_page;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	void *vto;
	void *vfrom;

	if(strcmp(current->comm,"IS") == 0){
		trace_printk("s\n");
	}

#if STATISTICS
	read++;
#endif

	PSMINPRINTK("Read for address %lu pid %d\n", address,current->pid);

	page->reading= 1;

	//message to ask for a copy
	read_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
							       GFP_ATOMIC);
	if (read_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}

	read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	read_message->address = address;
	read_message->tgroup_home_cpu = tgroup_home_cpu;
	read_message->tgroup_home_id = tgroup_home_id;
	read_message->is_fetch= 0;
	read_message->is_write= 0;
	read_message->last_write= page->last_write;
	read_message->vma_operation_index= current->mm->vma_operation_index;

	//object to held responses
	reading_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
								  GFP_ATOMIC);
	if (reading_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_read_message;
	}

	reading_page->tgroup_home_cpu= tgroup_home_cpu;
	reading_page->tgroup_home_id= tgroup_home_id;
	reading_page->address = address;
	reading_page->address_present= 0;
	reading_page->data= NULL;
	reading_page->is_fetch= 0;
	reading_page->is_write= 0;
	reading_page->last_write= page->last_write;
	reading_page->owner= 0;

	reading_page->vma_present = 0;
	reading_page->vaddr_start = 0;
	reading_page->vaddr_size = 0;
	reading_page->pgoff = 0;
	memset(reading_page->path,0,sizeof(char)*512);
	memset(&(reading_page->prot),0,sizeof(pgprot_t));
	reading_page->vm_flags = 0;
	reading_page->waiting = current;

	// Make data entry visible to handler.
	add_mapping_entry(reading_page);

	PSPRINTK("Sending a read message for address %lu \n ", address);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/
	sent= 0;
	reading_page->arrived_response=0;

#ifndef SUPPORT_FOR_CLUSTERING
	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		// Skip the current cpu
		if(i == _cpu) continue;

#else
		// the list does not include the current processor group descirptor (TODO)
		/*
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		*/

		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;
#endif
			if (page->other_owners[i] == 1){
				if(strcmp(current->comm,"IS") == 0){
					trace_printk("se\n");
				}
			}

			if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (read_message),
							sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr)) == -1)) {
				// Message delivered
				sent++;
				if(sent>1)
					printk("ERROR: using protocol optimized for 2 kernels but sending a read to more than one kernel");
			}
		}

		if(sent){
			while (reading_page->arrived_response == 0) {
				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (reading_page->arrived_response == 0)
					schedule();
				set_task_state(current, TASK_RUNNING);
			}

			if(strcmp(current->comm,"IS") == 0) {
				trace_printk("r\n");
			}
		}
		else{
			printk("ERROR: impossible to send read message, no destination kernel\n");
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			goto exit_reading_page;
		}

		if(strcmp(current->comm,"IS") == 0){
			trace_printk("ls\n");
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/
		if(strcmp(current->comm,"IS") == 0){
			trace_printk("l\n");
		}

		vma = find_vma(mm, address);
		if (unlikely(
					!vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("ERROR: vma not valid during read for write\n");
			ret = VM_FAULT_VMA;
			goto exit_reading_page;
		}

		if(reading_page->address_present==1){
			if (reading_page->data->address != address) {
				printk("ERROR: trying to copy wrong address!");
				pcn_kmsg_free_msg(reading_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}

			if (reading_page->last_write != (page->last_write+1)) {
				printk("ERROR: new copy received during a read but my last write is %lu and received last write is %lu\n",
						page->last_write,reading_page->last_write);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
			else
				page->last_write= reading_page->last_write;

			if(reading_page->owner==1){
				printk("ERROR: owneship sent with read request\n");
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}

			// Ported to Linux 3.12
			//vto = kmap_atomic(page, KM_USER0);
			vto = kmap_atomic(page);
			vfrom = &(reading_page->data->data);

#if	DIFF_PAGE
			if(reading_page->data->data_size==PAGE_SIZE){
				copy_user_page(vto, vfrom, address, page);
			}
			else{

				if(reading_page->data->diff==1)
					WKdm_decompress_and_diff(vfrom,vto);
				else
				{
					kunmap_atomic(vto, KM_USER0);
					pcn_kmsg_free_msg(reading_page->data);
					printk(
							"ERROR: received data not diff in write address %lu\n",address);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_reading_page;
				}
			}

#else
			copy_user_page(vto, vfrom, address, page);
#endif

			// Ported to Linux 3.12
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);

#if !DIFF_PAGE
#if CHECKSUM
			// Ported to Linux 3.12
			//vto= kmap_atomic(page, KM_USER0);
			vto= kmap_atomic(page);
			__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
			// Ported to Linux 3.12
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);
			__wsum check2= csum_partial(&(reading_page->data->data), PAGE_SIZE, 0);
			if(check1!=check2) {
				printk("ERROR: page just copied is not matching, address %lu\n",address);
				pcn_kmsg_free_msg(reading_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
			if(check1!=reading_page->data->checksum) {
				printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
				pcn_kmsg_free_msg(reading_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
#endif
#endif
			pcn_kmsg_free_msg(reading_page->data);

			page->status = REPLICATION_STATUS_VALID;
			page->owner = reading_page->owner;

#if STATISTICS
			if(page->last_write> most_written_page)
				most_written_page= page->last_write;
#endif

			flush_cache_page(vma, address, pte_pfn(*pte));
			//now the page can be written
			value_pte = *pte;
			value_pte = pte_clear_flags(value_pte, _PAGE_RW);
			value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);

			value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

			ptep_clear_flush(vma, address, pte);

			set_pte_at_notify(mm, address, pte, value_pte);

			update_mmu_cache(vma, address, pte);

			//flush_tlb_page(vma, address);

			flush_tlb_fix_spurious_fault(vma, address);
			PSPRINTK("Out read %i address %lu \n ", read, address);
		}
		else{
			printk("ERROR: no copy received for a read\n");
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			remove_mapping_entry(reading_page);
			kfree(reading_page);
			kfree(read_message);
			goto exit;

		}
exit_reading_page:
		remove_mapping_entry(reading_page);
		kfree(reading_page);

exit_read_message:
		kfree(read_message);

exit:
		page->reading = 0;
		if(strcmp(current->comm,"IS") == 0){
			trace_printk("e\n");
		}
		return ret;
	}

#else

/* Read on a REPLICATED page => ask a copy of the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *0, write succeeded;
 * */
int do_remote_read(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page, int _cpu) {
#if STATISTICS
		int attemps_read;
#endif
		data_request_t* read_message;
		mapping_answers_t* reading_page;
		int i;
		void *vto;
		void *vfrom;
		pte_t value_pte;
		int ret;
		unsigned long flags;

#if STATISTICS
		read++;
		attemps_read=0;
#endif

		page->reading = 1;

		//message to ask a copy of the page
		read_message = (data_request_t*) kmalloc(sizeof(data_request_t),
				GFP_ATOMIC);
		if (read_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}

		read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
		read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		read_message->address = address;
		read_message->tgroup_home_cpu = tgroup_home_cpu;
		read_message->tgroup_home_id = tgroup_home_id;
		read_message->read_for_write = 0;
		read_message->flags = page_fault_flags;
		read_message->vma_operation_index= current->mm->vma_operation_index;

		//object to held responses
		reading_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
				GFP_ATOMIC);
		if (reading_page == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_read_message;
		}

		reading_page->address = address;
		reading_page->address_present = REPLICATION_STATUS_INVALID;
		reading_page->vma_present = 0;
		reading_page->data = NULL;
		reading_page->responses = 0;
		reading_page->expected_responses = 0;
		reading_page->last_invalid = -1;
		reading_page->last_write = 0;
		reading_page->tgroup_home_cpu = tgroup_home_cpu;
		reading_page->tgroup_home_id = tgroup_home_id;
		reading_page->waiting = current;
		raw_spin_lock_init(&(reading_page->lock));

		// Add to appropriate list.
		add_mapping_entry(reading_page);

retry_read:

#if STATISTICS
		attemps_read++;
#endif

		PSPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);
		PSMINPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);

		if (page->owner == _cpu) {
			printk("ERROR: asking a page to myself for read address %lu\n",
					address);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/

		/* Try to ask the page to the owner first.
		 * Likely it has the most updated version
		 */
		if (pcn_kmsg_send(page->owner, (struct pcn_kmsg_message*) (read_message))
				!= -1) {

			reading_page->expected_responses = 1;

			while (reading_page->responses == 0) {
				//DEFINE_WAIT(wait);
				//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (reading_page->responses == 0)
					schedule();
				set_task_state(current, TASK_RUNNING);
				//finish_wait(&request_wait, &wait);
			}
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/
		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

			printk("ERROR: vma not valid during read\n");
			ret = VM_FAULT_VMA;
			goto exit_reading_page;
		}

		/*If the owner has not a valid copy, or an invalid arrived in the meanwhile,
		 *ask to everybody in the system
		 */
		if (reading_page->address_present == REPLICATION_STATUS_INVALID
				|| reading_page->last_invalid >= reading_page->last_write) {

#if STATISTICS
			attemps_read++;
#endif
			PSPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);

			reading_page->address_present = REPLICATION_STATUS_INVALID;
			if (reading_page->data != NULL) {
				pcn_kmsg_free_msg(reading_page->data);
				reading_page->data = NULL;
			}
			reading_page->responses = 0;
			reading_page->expected_responses = 0;
			reading_page->vma_present = 0;

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			/*PTE UNLOCKED*/

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
					if (page->other_owners[i] == 1) {
						if (!(pcn_kmsg_send(i,
										(struct pcn_kmsg_message*) (read_message)) == -1)) {
							// Message delivered
							reading_page->expected_responses++;
						}
					}
				}

				while (!(reading_page->responses == reading_page->expected_responses)) {
					//DEFINE_WAIT(wait);
					//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
					set_task_state(current, TASK_UNINTERRUPTIBLE);
					if (!(reading_page->responses == reading_page->expected_responses))
						schedule();
					//finish_wait(&request_wait, &wait);
					set_task_state(current, TASK_RUNNING);
				}

				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/
				vma = find_vma(mm, address);
				if (unlikely(
							!vma || address >= vma->vm_end || address < vma->vm_start)) {

					printk("ERROR: vma not valid during read\n");
					ret = VM_FAULT_VMA;
					goto exit_reading_page;
				}
			}

			if (reading_page->last_invalid >= reading_page->last_write) {

				reading_page->address_present = REPLICATION_STATUS_INVALID;
				if (reading_page->data != NULL) {
					pcn_kmsg_free_msg(reading_page->data);
					reading_page->data = NULL;
				}
				reading_page->responses = 0;
				reading_page->expected_responses = 0;
				reading_page->vma_present = 0;

				goto retry_read;
			}

			raw_spin_lock_irqsave(&(reading_page->lock), flags);
			raw_spin_unlock_irqrestore(&(reading_page->lock), flags);

			remove_mapping_entry(reading_page);

			if (reading_page->address_present == REPLICATION_STATUS_INVALID) {
				//aaaaaaaaaaaaaaaaaaa not valid copy in the system!!!
				printk("ERROR: NO VALID COPY IN THE SYSTEM\n");
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;

			}

			PSPRINTK("Out read %i address %lu iter %i \n", read, address, attemps_read);

			if (reading_page->data->address != address) {
				printk("ERROR: trying to copy wrong address!");
				pcn_kmsg_free_msg(reading_page->data);
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}

			// Ported to Linux 3.12
			//vto = kmap_atomic(page, KM_USER0);
			vto = kmap_atomic(page);
			vfrom = reading_page->data->data;
			copy_user_page(vto, vfrom, address, page);
			// Ported to Linux 3.12
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);

#if CHECKSUM
			// Ported to Linux 3.12
			//vto= kmap_atomic(page, KM_USER0);
			vto= kmap_atomic(page);
			__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
			// Ported to Linux 3.12
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);
			__wsum check2= csum_partial(&(reading_page->data->data), PAGE_SIZE, 0);
			if(check1!=check2) {
				printk("ERROR: page just copied is not matching, address %lu\n",address);
				pcn_kmsg_free_msg(reading_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
			if(check1!=reading_page->data->checksum) {
				printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
				pcn_kmsg_free_msg(reading_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_reading_page;
			}
#endif

			pcn_kmsg_free_msg(reading_page->data);
			page->last_write = reading_page->last_write;

#if STATISTICS
			if(page->last_write> most_written_page)
				most_written_page= page->last_write;
			if(attemps_read > most_long_read)
				most_long_read= attemps_read;
#endif

			page->status = REPLICATION_STATUS_VALID;

			value_pte = *pte;
			//we need to catch write access
			value_pte = pte_clear_flags(value_pte, _PAGE_RW);
			//value_pte= pte_clear_flags(value_pte,_PAGE_DIRTY);
			value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
			//value_pte= pte_set_flags(value_pte,_PAGE_USER);
			value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

			ptep_clear_flush(vma, address, pte);

			set_pte_at_notify(mm, address, pte, value_pte);

			update_mmu_cache(vma, address, pte);

			//flush_tlb_page(vma, address);
			//flush_tlb_range(vma,vma->vm_start,vma->vm_end);

			//flush_tlb_fix_spurious_fault(vma, address);

			flush_cache_page(vma, address, pte_pfn(*pte));

			ret = 0;

exit_reading_page:

			remove_mapping_entry(reading_page);
			kfree(reading_page);

exit_read_message:

			kfree(read_message);

exit:

	page->reading = 0;
	return ret;
}
#endif

#if FOR_2_KERNELS
//static
int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page,int invalid, int _cpu) {

	int  i;
	int ret= 0;
	int sent;
	pte_t value_pte;

	ack_answers_for_2_kernels_t* answers;
	invalid_data_for_2_kernels_t* invalid_message;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	data_request_for_2_kernels_t* write_message;
	mapping_answers_for_2_kernels_t* writing_page;

	void *vto;
	void *vfrom;

	if(strcmp(current->comm,"IS") == 0){
		trace_printk("s\n");
	}
	page->writing = 1;

#if STATISTICS
	write++;
#endif

	PSPRINTK("Write %i address %lu pid %d\n", write, address,current->pid);
	PSMINPRINTK("Write for address %lu owner %d pid %d\n", address,page->owner==1?1:0,current->pid);

	if(page->owner==1){
		//in this case I send and invalid message
		if(invalid){
			printk("ERROR: I am the owner of the page and it is invalid when going to write\n");
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit;
		}
		//object to store the acks (nacks) sent by other kernels
		answers = (ack_answers_for_2_kernels_t*) kmalloc(sizeof(ack_answers_for_2_kernels_t),
								 GFP_ATOMIC);
		if (answers == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}
		answers->tgroup_home_cpu = tgroup_home_cpu;
		answers->tgroup_home_id = tgroup_home_id;
		answers->address = address;
		answers->waiting = current;

		//message to invalidate the other copies
		invalid_message = (invalid_data_for_2_kernels_t*) kmalloc(sizeof(invalid_data_for_2_kernels_t),
									  GFP_ATOMIC);
		if (invalid_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_answers;
		}
		invalid_message->header.type = PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA;
		invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		invalid_message->tgroup_home_cpu = tgroup_home_cpu;
		invalid_message->tgroup_home_id = tgroup_home_id;
		invalid_message->address = address;
		invalid_message->vma_operation_index= current->mm->vma_operation_index;

		// Insert the object in the appropriate list.
		add_ack_entry(answers);

		invalid_message->last_write = page->last_write;

		answers->response_arrived= 0;

		sent= 0;

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/
#ifndef SUPPORT_FOR_CLUSTERING
		for(i = 0; i < MAX_KERNEL_IDS; i++) {
			// Skip the current cpu
			if(i == _cpu) continue;

#else
			// the list does not include the current processor group descirptor (TODO)
			/*
			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			*/
			list_for_each(iter, &rlist_head) {
				objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
				i = objPtr->_data._processor;
#endif
				if (page->other_owners[i] == 1) {
					if(strcmp(current->comm,"IS") == 0){
						trace_printk("se\n");
					}

					if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (invalid_message),sizeof(invalid_data_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
								== -1)) {
						// Message delivered
						sent++;
						if(sent>1)
							printk("ERROR: using protocol optimized for 2 kernels but sending an invalid to more than one kernel");
					}
				}
			}

			if(sent){
				while (answers->response_arrived==0) {
					set_task_state(current, TASK_UNINTERRUPTIBLE);
					if (answers->response_arrived==0)
						schedule();
					set_task_state(current, TASK_RUNNING);
				}
				if(strcmp(current->comm,"IS") == 0){
					trace_printk("r\n");
				}
			}
			else
				printk("Impossible to send invalid, no destination kernel\n");

			if(strcmp(current->comm,"IS") == 0){
				trace_printk("ls\n");
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);

			if(strcmp(current->comm,"IS") == 0){
				trace_printk("l\n");
			}
			/*PTE LOCKED*/
			vma = find_vma(mm, address);
			if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {
				printk("ERROR: vma not valid after waiting for ack to invalid\n");
				ret = VM_FAULT_VMA;
				goto exit_invalid;
			}

			PSPRINTK("Received ack to invalid %i address %lu \n", write, address);

exit_invalid:
			kfree(invalid_message);
			remove_ack_entry(answers);
exit_answers:
			kfree(answers);
			if(ret!=0)
				goto exit;
		} else {
			//in this case I send a mapping request with write flag set

			//message to ask for a copy
			write_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
										GFP_ATOMIC);
			if (write_message == NULL) {
				ret = VM_FAULT_OOM;
				goto exit;
			}

			write_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
			write_message->header.prio = PCN_KMSG_PRIO_NORMAL;
			write_message->address = address;
			write_message->tgroup_home_cpu = tgroup_home_cpu;
			write_message->tgroup_home_id = tgroup_home_id;
			write_message->is_fetch= 0;
			write_message->is_write= 1;
			write_message->last_write= page->last_write;
			write_message->vma_operation_index= current->mm->vma_operation_index;

			//object to held responses
			writing_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
										  GFP_ATOMIC);
			if (writing_page == NULL) {
				ret = VM_FAULT_OOM;
				goto exit_write_message;

			}

			writing_page->tgroup_home_cpu= tgroup_home_cpu;
			writing_page->tgroup_home_id= tgroup_home_id;
			writing_page->address = address;
			writing_page->address_present= 0;
			writing_page->data= NULL;
			writing_page->is_fetch= 0;
			writing_page->is_write= 1;
			writing_page->last_write= page->last_write;
			writing_page->owner= 0;

			writing_page->vma_present = 0;
			writing_page->vaddr_start = 0;
			writing_page->vaddr_size = 0;
			writing_page->pgoff = 0;
			memset(writing_page->path,0,sizeof(char)*512);
			memset(&(writing_page->prot),0,sizeof(pgprot_t));
			writing_page->vm_flags = 0;
			writing_page->waiting = current;

			// Make data entry visible to handler.
			add_mapping_entry(writing_page);

			PSPRINTK(
					"Sending a write message for address %lu \n ", address);


			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			/*PTE UNLOCKED*/
			sent= 0;
			writing_page->arrived_response=0;

#ifndef SUPPORT_FOR_CLUSTERING
			for(i = 0; i < MAX_KERNEL_IDS; i++) {
				// Skip the current cpu
				if(i == _cpu) continue;

#else
				// the list does not include the current processor group descirptor (TODO)
				/*
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				*/
				list_for_each(iter, &rlist_head) {
					objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
#endif
					if (page->other_owners[i] == 1) {
						if(strcmp(current->comm,"IS") == 0){
							trace_printk("se\n");
						}
						if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (write_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
									== -1)) {
							// Message delivered
							sent++;
							if(sent>1)
								printk("ERROR: using protocol optimized for 2 kernels but sending a write to more than one kernel");
						}
					}
				}

				if(sent){
					while (writing_page->arrived_response == 0) {
						set_task_state(current, TASK_UNINTERRUPTIBLE);
						if (writing_page->arrived_response == 0)
							schedule();
						set_task_state(current, TASK_RUNNING);
					}

					if(strcmp(current->comm,"IS") == 0){
						trace_printk("r\n");
					}
				} else {
					printk("ERROR: impossible to send write message, no destination kernel\n");
					ret= VM_FAULT_REPLICATION_PROTOCOL;
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					goto exit_writing_page;
				}

				if(strcmp(current->comm,"IS") == 0){
					trace_printk("ls\n");
				}

				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/

				if(strcmp(current->comm,"IS") == 0){
					trace_printk("l\n");
				}

				vma = find_vma(mm, address);
				if (unlikely(
							!vma || address >= vma->vm_end || address < vma->vm_start)) {

					printk("ERROR: vma not valid during read for write\n");
					ret = VM_FAULT_VMA;
					goto exit_writing_page;
				}

				if(writing_page->owner!=1){
					printk("ERROR: received answer to write without ownership\n");
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_writing_page;
				}

				if(writing_page->address_present==1){
					if (writing_page->data->address != address) {
						printk("ERROR: trying to copy wrong address!");
						pcn_kmsg_free_msg(writing_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_writing_page;
					}
					//in this case I also received the new copy
					if (writing_page->last_write != (page->last_write+1)) {
						pcn_kmsg_free_msg(writing_page->data);
						printk(
								"ERROR: new copy received during a write but my last write is %lu and received last write is %lu\n",page->last_write,writing_page->last_write);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_writing_page;
					}
					else
						page->last_write= writing_page->last_write;

					// Ported to Linux 3.12 
					//vto = kmap_atomic(page, KM_USER0);
					vto = kmap_atomic(page);
					vfrom = &(writing_page->data->data);

#if	DIFF_PAGE
					if(writing_page->data->data_size==PAGE_SIZE){
						copy_user_page(vto, vfrom, address, page);
					}
					else{
						if(writing_page->data->diff==1)
							WKdm_decompress_and_diff(vfrom,vto);
						else
						{
							kunmap_atomic(vto, KM_USER0);
							pcn_kmsg_free_msg(writing_page->data);
							printk(
									"ERROR: received data not diff in write address %lu\n",address);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_writing_page;
						}
					}

#else
					copy_user_page(vto, vfrom, address, page);
#endif
					// Ported to Linux 3.12 
					//kunmap_atomic(vto, KM_USER0);
					kunmap_atomic(vto);
#if !DIFF_PAGE
#if CHECKSUM
					// Ported to Linux 3.12 
					//vto= kmap_atomic(page, KM_USER0);
					vto= kmap_atomic(page);
					__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
					// Ported to Linux 3.12 
					//kunmap_atomic(vto, KM_USER0);
					kunmap_atomic(vto);
					__wsum check2= csum_partial(&(writing_page->data->data), PAGE_SIZE, 0);
					if(check1!=check2) {
						printk("ERROR: page just copied is not matching, address %lu\n",address);
						pcn_kmsg_free_msg(writing_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_writing_page;
					}
					if(check1!=writing_page->data->checksum) {
						printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
						pcn_kmsg_free_msg(writing_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_writing_page;
					}
#endif
#endif
					pcn_kmsg_free_msg(writing_page->data);

exit_writing_page:

					remove_mapping_entry(writing_page);
					kfree(writing_page);

exit_write_message:

					kfree(write_message);

					if(ret!=0)
						goto exit;
				}
				else{
					remove_mapping_entry(writing_page);
					kfree(writing_page);
					kfree(write_message);

					if(invalid){
						printk("ERROR: writing an invalid page but not received a copy\n");
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit;
					}
				}
			}
			page->status = REPLICATION_STATUS_WRITTEN;
			page->owner = 1;
			(page->last_write)++;

#if STATISTICS
			if(page->last_write> most_written_page)
				most_written_page= page->last_write;
#endif

#if	DIFF_PAGE
			if(page->old_page_version==NULL){
				page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
						GFP_ATOMIC);
				if(page->old_page_version==NULL){
					printk("ERROR: impossible to kmalloc old diff page\n");
					goto exit;
				}
			}

			void *vto;
			void *vfrom;
			vto = page->old_page_version;
			// Ported to Linux 3.12 
			// vfrom = kmap_atomic(page, KM_USER0);
			vfrom = kmap_atomic(page);
			copy_user_page(vto, vfrom, address, page);
			// Ported to Linux 3.12 
			//kunmap_atomic(vfrom, KM_USER0);
			kunmap_atomic(vfrom);
#endif

			flush_cache_page(vma, address, pte_pfn(*pte));

			//now the page can be written
			value_pte = *pte;
			value_pte = pte_set_flags(value_pte, _PAGE_RW);
			value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
			//value_pte=pte_set_flags(value_pte,_PAGE_USER);
			value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);
			//value_pte=pte_set_flags(value_pte,_PAGE_DIRTY);
			ptep_clear_flush(vma, address, pte);

			set_pte_at_notify(mm, address, pte, value_pte);

			update_mmu_cache(vma, address, pte);

			//flush_tlb_page(vma, address);

			flush_tlb_fix_spurious_fault(vma, address);

			PSPRINTK("Out write %i address %lu last write is %lu \n ", write, address,page->last_write);

exit:
			page->writing = 0;

			if(strcmp(current->comm,"IS") == 0){
				trace_printk("e\n");
			}

			return ret;
		}

#else
/* Write on a REPLICATED page => coordinate with other kernels to write on the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *0, write succeeded;
 * */
int do_remote_write(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page, int _cpu) {

	int attemps_write, i;
	ack_answers_t* answers;
	invalid_data_t* invalid_message;
	data_request_t* read_message;
	mapping_answers_t* reading_page;
	unsigned long flags;
	int ret;
	void *vto;
	void *vfrom;
	pte_t value_pte;

	attemps_write = 1;

	page->writing = 1;

	/* Each write has a unique time stamp associated.
	 * This time stamp will not change until the status will be set to written.
	 */
	page->time_stamp = native_read_tsc();

	page->concurrent_writers = 0;
	page->concurrent_fetch = 0;

	//object to store the acks (nacks) sent by other kernels
	answers = (ack_answers_t*) kmalloc(sizeof(ack_answers_t), GFP_ATOMIC);
	if (answers == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}
	answers->tgroup_home_cpu = tgroup_home_cpu;
	answers->tgroup_home_id = tgroup_home_id;
	answers->address = address;
	answers->waiting = current;
	raw_spin_lock_init(&(answers->lock));

	//message to invalidate the other copies
	invalid_message = (invalid_data_t*) kmalloc(sizeof(invalid_data_t),
			GFP_ATOMIC);
	if (invalid_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_answers;
	}
	invalid_message->header.type = PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA;
	invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	invalid_message->tgroup_home_cpu = tgroup_home_cpu;
	invalid_message->tgroup_home_id = tgroup_home_id;
	invalid_message->address = address;
	invalid_message->time_stamp = page->time_stamp;
	invalid_message->vma_operation_index= current->mm->vma_operation_index;

	// Insert the object in the appropriate list.
	add_ack_entry(answers);

#if STATISTICS
	write++;
#endif

retry_write:

	PSPRINTK("Write %i address %lu attempts %i\n", write, address, attemps_write);

	/* If this is not the fist attempt to write, the page has been written by a concurrent thread in another kernel.
	 * I need to remote-read the page before trying to write again.
	 */
	if (attemps_write != 1) {

		page->reading = 1;

		//message to ask for a copy
		read_message = (data_request_t*) kmalloc(sizeof(data_request_t),
				GFP_ATOMIC);
		if (read_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_invalid;
		}

		read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
		read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		read_message->address = address;
		read_message->tgroup_home_cpu = tgroup_home_cpu;
		read_message->tgroup_home_id = tgroup_home_id;
		read_message->read_for_write = 1;
		read_message->vma_operation_index= current->mm->vma_operation_index;

		//object to held responses
		reading_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
				GFP_ATOMIC);
		if (reading_page == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_read_message;

		}
		reading_page->address = address;
		reading_page->address_present = REPLICATION_STATUS_INVALID;
		reading_page->vma_present = 0;
		reading_page->responses = 0;
		reading_page->expected_responses = 0;
		reading_page->tgroup_home_cpu = tgroup_home_cpu;
		reading_page->tgroup_home_id = tgroup_home_id;
		reading_page->waiting = current;
		raw_spin_lock_init(&(reading_page->lock));

		// Make data entry visible to handler.
		add_mapping_entry(reading_page);

		PSPRINTK(
				"Read for write %i attempt %i address %lu \n ", write, attemps_write, address);

		if (answers->owner == _cpu) {
			printk("ERROR: asking a copy of a page for a write to myself.\n");
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/

		// Wait for owner to respond.
		if (pcn_kmsg_send(answers->owner,
					(struct pcn_kmsg_message*) (read_message)) != -1) {

			reading_page->expected_responses = 1;

			while (reading_page->responses == 0) {
				//DEFINE_WAIT(wait);
				//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (reading_page->responses == 0)
					schedule();
				//finish_wait(&request_wait, &wait);
				set_task_state(current, TASK_RUNNING);
			}

		} else {
			printk("ERROR: owner not reachable.\n");
			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/

		vma = find_vma(mm, address);
		if (unlikely(
					!vma || address >= vma->vm_end || address < vma->vm_start)) {

			printk("ERROR: vma not valid during read for write\n");
			ret = VM_FAULT_VMA;
			goto exit_reading_page;
		}

		raw_spin_lock_irqsave(&(reading_page->lock), flags);
		raw_spin_unlock_irqrestore(&(reading_page->lock), flags);

		if (reading_page->address_present == REPLICATION_STATUS_INVALID) {
			//aaaaaaaaaaaaaaaaaaa not valid copy in the system!!!
			printk(
					"ERROR: NO VALID COPY IN THE SYSTEM WHEN READING FOR WRITE\n");
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}

		if (reading_page->data->address != address) {
			printk("ERROR: trying to copy wrong address!");
			pcn_kmsg_free_msg(reading_page->data);
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
		// Ported to Linux 3.12 
		//vto = kmap_atomic(page, KM_USER0);
		vto = kmap_atomic(page);
		vfrom = reading_page->data->data;
		copy_user_page(vto, vfrom, address, page);
		// Ported to Linux 3.12 
		//kunmap_atomic(vto, KM_USER0);
		kunmap_atomic(vto);

#if CHECKSUM
		// Ported to Linux 3.12 
		//vto= kmap_atomic(page, KM_USER0);
		vto= kmap_atomic(page);
		__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
		// Ported to Linux 3.12 
		//kunmap_atomic(vto, KM_USER0);
		kunmap_atomic(vto);
		__wsum check2= csum_partial(reading_page->data->data, PAGE_SIZE, 0);
		if(check1!=check2) {
			printk("ERROR: page just copied is not matching, address %lu\n",address);
			pcn_kmsg_free_msg(reading_page->data);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
		if(check1!=reading_page->data->checksum) {
			printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
			pcn_kmsg_free_msg(reading_page->data);
			ret= VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_reading_page;
		}
#endif

		pcn_kmsg_free_msg(reading_page->data);
		flush_cache_page(vma, address, pte_pfn(*pte));

		page->status = REPLICATION_STATUS_VALID;
		page->last_write = reading_page->last_write;

		value_pte = *pte;
		//we need to catch write access
		value_pte = pte_clear_flags(value_pte, _PAGE_RW);
		//value_pte= pte_clear_flags(value_pte,_PAGE_DIRTY);
		value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
		//value_pte= pte_set_flags(value_pte,_PAGE_USER);
		value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

		ptep_clear_flush(vma, address, pte);

		set_pte_at_notify(mm, address, pte, value_pte);

		update_mmu_cache(vma, address, pte);

		//flush_tlb_page(vma, address);
		//flush_tlb_fix_spurious_fault(vma, address);

		remove_mapping_entry(reading_page);
		kfree(reading_page);
		kfree(read_message);

		page->reading = 0;

		//flush_cache_page(vma, address, pte_pfn(*pte));
		PSPRINTK(
				"Out read for write %i attempt %i address %lu \n ", write, attemps_write, address);
	}

	invalid_message->last_write = page->last_write;

	answers->nack = 0;
	answers->responses = 0;
	answers->expected_responses = 0;
	answers->concurrent = 0;
	answers->owner = _cpu;
	answers->time_stamp = page->time_stamp;

	//send to the other copies the invalidation message
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	PSMINPRINTK("writing %i address %lu iter %i \n", write, address,attemps_write);

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
			if (page->other_owners[i] == 1) {

				if (!(pcn_kmsg_send(i, (struct pcn_kmsg_message*) (invalid_message))
							== -1)) {
					// Message delivered
					answers->expected_responses++;
				}
			}
		}

		//wait for all the answers (ack or nack) to arrive
		while (!(answers->responses == answers->expected_responses)) {
			//DEFINE_WAIT(wait);
			//	prepare_to_wait(&ack_wait, &wait, TASK_UNINTERRUPTIBLE);
			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (!(answers->responses == answers->expected_responses))
				schedule();
			//finish_wait(&ack_wait, &wait);
			set_task_state(current, TASK_RUNNING);
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/

		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

			printk("ERROR: vma not valid after waiting for ack to invalid\n");
			ret = VM_FAULT_VMA;
			goto exit_invalid;
		}

		/* If somebody was concurrently writing with success on other kernels, retry.
		 */
		PSPRINTK("Concurrent_writers %i \n", page->concurrent_writers);

		if (answers->nack != 0) {

#if STATISTICS
			concurrent_write++;
#endif

			attemps_write++;
			goto retry_write;

		} else {

			PSPRINTK(
					"Received all acks to write %i address %lu attempts %i\n", write, address, attemps_write);

			raw_spin_lock_irqsave(&(answers->lock), flags);
			raw_spin_unlock_irqrestore(&(answers->lock), flags);

			//change status to written
			page->status = REPLICATION_STATUS_WRITTEN;
			page->owner = _cpu;
			(page->last_write)++;

#if STATISTICS
			if(page->last_write> most_written_page)
				most_written_page= page->last_write;
			if(attemps_write >most_long_write)
				most_long_write= attemps_write;
#endif

			memset(page->need_fetch, 0, MAX_KERNEL_IDS*sizeof(int));
			page->concurrent_fetch = 0;
			page->concurrent_writers = answers->concurrent;

			flush_cache_page(vma, address, pte_pfn(*pte));

			//now the page can be written
			value_pte = *pte;
			value_pte = pte_set_flags(value_pte, _PAGE_RW);
			value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
			//value_pte=pte_set_flags(value_pte,_PAGE_USER);
			value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);
			//value_pte=pte_set_flags(value_pte,_PAGE_DIRTY);
			ptep_clear_flush(vma, address, pte);

			set_pte_at_notify(mm, address, pte, value_pte);

			update_mmu_cache(vma, address, pte);

			//flush_tlb_page(vma, address);

			flush_tlb_fix_spurious_fault(vma, address);

			ret = 0;
			goto exit_invalid;
		}

exit_reading_page:

		remove_mapping_entry(reading_page);
		kfree(reading_page);

exit_read_message:

		kfree(read_message);
		page->reading = 0;

exit_invalid:

		kfree(invalid_message);
		remove_ack_entry(answers);

exit_answers:

		kfree(answers);

exit:

		page->writing = 0;

		return ret;
}

#endif

#if FOR_2_KERNELS
//static
int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl, int _cpu) {

	mapping_answers_for_2_kernels_t* fetching_page;
	data_request_for_2_kernels_t* fetch_message;
	int ret= 0,i,reachable,other_cpu=-1;

	memory_t* memory;

	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;

	int status;
	void *vto;
	void *vfrom;

	pte_t entry;

	if(strcmp(current->comm,"IS") == 0){
		trace_printk("s\n");
	}

	PSMINPRINTK("Fetch for address %lu write %i pid %d is local?%d\n", address,((page_fault_flags & FAULT_FLAG_WRITE)?1:0),current->pid,pte_none(value_pte));
#if STATISTICS
	fetch++;
#endif

	fetching_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
			GFP_ATOMIC);
	if (fetching_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}

	fetching_page->tgroup_home_cpu = tgroup_home_cpu;
	fetching_page->tgroup_home_id = tgroup_home_id;
	fetching_page->address = address;

	fetching_page->vma_present = 0;
	fetching_page->vaddr_start = 0;
	fetching_page->vaddr_size = 0;
	fetching_page->pgoff = 0;
	memset(fetching_page->path,0,sizeof(char)*512);
	memset(&(fetching_page->prot),0,sizeof(pgprot_t));
	fetching_page->vm_flags = 0;

	if(page_fault_flags & FAULT_FLAG_WRITE)
		fetching_page->is_write= 1;
	else
		fetching_page->is_write= 0;

	fetching_page->is_fetch= 1;
	fetching_page->owner= 0;
	fetching_page->address_present= 0;
	fetching_page->last_write= 0;
	fetching_page->data= NULL;
	fetching_page->futex_owner = -1;//akshay

	fetching_page->waiting = current;

	add_mapping_entry(fetching_page);

	if(_cpu==tgroup_home_cpu){
		if(pte_none(value_pte)){
			//not marked pte

#if STATISTICS
			local_fetch++;
#endif
			PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lu\n", local_fetch, address);
			ret = VM_CONTINUE_WITH_CHECK;
			goto exit;
		}
	}

	fetch_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
			GFP_ATOMIC);
	if (fetch_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_fetching_page;
	}

	fetch_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	fetch_message->address = address;
	fetch_message->tgroup_home_cpu = tgroup_home_cpu;
	fetch_message->tgroup_home_id = tgroup_home_id;
	fetch_message->is_write = fetching_page->is_write;
	fetch_message->is_fetch= 1;
	fetch_message->vma_operation_index= current->mm->vma_operation_index;

	PSPRINTK("Fetch %i address %lu\n", fetch, address);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	fetching_page->arrived_response= 0;
	reachable= 0;

	memory= find_memory_entry(current->tgroup_home_cpu, current->tgroup_home_id);

	down_read(&memory->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		// Skip the current cpu
		if(i == _cpu) continue;

#else
		// the list does not include the current processor group descirptor (TODO)
		/*
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		*/
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;

#endif
			if(memory->kernel_set[i]==1)
				if(strcmp(current->comm,"IS") == 0){
					trace_printk("se\n");
				}

			if ((ret=pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (fetch_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr)))
					!= -1) {
				// Message delivered
				reachable++;
				other_cpu= i;
				if(reachable>1)
					printk("ERROR: using optimized algorithm for 2 kernels with more than two kernels\n");
			}
		}

		up_read(&memory->kernel_set_sem);

		if(reachable>0){
			while (fetching_page->arrived_response==0) {

				set_task_state(current, TASK_UNINTERRUPTIBLE);

				if (fetching_page->arrived_response==0) {
					schedule();
				}

				set_task_state(current, TASK_RUNNING);
			}
			if(strcmp(current->comm,"IS") == 0){
				trace_printk("r\n");
			}

		}

		if(strcmp(current->comm,"IS") == 0){
			trace_printk("ls\n");
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/
		if(strcmp(current->comm,"IS") == 0){
			trace_printk("l\n");
		}

		PSPRINTK("Out wait fetch %i address %lu \n", fetch, address);

#if NOT_REPLICATED_VMA_MANAGEMENT
		//only the client has to update the vma
		if(tgroup_home_cpu!=_cpu)
#endif

		{
			ret = do_mapping_for_distributed_process(fetching_page, mm, address, ptl);
			if (ret != 0)
				goto exit_fetch_message;
				
			PSPRINTK("Mapping end\n");

			vma = find_vma(mm, address);
			if (!vma || address >= vma->vm_end || address < vma->vm_start) {
				vma = NULL;
			} else if (unlikely(is_vm_hugetlb_page(vma))
					|| unlikely(transparent_hugepage_enabled(vma))) {
				printk("ERROR: Installed a vma with HUGEPAGE\n");
				ret = VM_FAULT_VMA;
				goto exit_fetch_message;
			}

			if (vma == NULL) {
				//PSPRINTK
				dump_stack();
				printk(KERN_ALERT"%s: ERROR: no vma for address %lu in the system {%d} \n",__func__, address,current->pid);
				ret = VM_FAULT_VMA;
				goto exit_fetch_message;
			}

		}

		if(_cpu==tgroup_home_cpu && fetching_page->address_present == 0){
			printk("ERROR: No response for a marked page\n");
			ret = VM_FAULT_REPLICATION_PROTOCOL;
			goto exit_fetch_message;
		}

		if (fetching_page->address_present == 1) {

			struct page* page;
			spin_unlock(ptl);
			/*PTE UNLOCKED*/

			if (unlikely(anon_vma_prepare(vma))) {
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

			page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
			if (!page) {
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

			__SetPageUptodate(page);

			if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
				page_cache_release(page);
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

#if STATISTICS
			pages_allocated++;
#endif
			spin_lock(ptl);
			/*PTE LOCKED*/

			//if nobody changed the pte
			if (likely(pte_same(*pte, value_pte))) {

				if(fetching_page->is_write){ //if I am doing a write

					status= REPLICATION_STATUS_WRITTEN;
					if(fetching_page->owner==0){
						printk("ERROR: copy of a page sent to a write fetch request without ownership\n");
						pcn_kmsg_free_msg(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}

				}
				else{

					status= REPLICATION_STATUS_VALID;
					if(fetching_page->owner==1){
						printk("ERROR: copy of a page sent to a read fetch request with ownership\n");
						pcn_kmsg_free_msg(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
				}

				if (fetching_page->data->address != address) {
					printk("ERROR: trying to copy wrong address!");
					pcn_kmsg_free_msg(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}


#if DIFF_PAGE

				if(fetching_page->data->diff==1){
					printk("ERROR: answered to a fetch with diff data\n");
					pcn_kmsg_free_msg(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}

				// Ported to Linux 3.12 
				//vto = kmap_atomic(page, KM_USER0);
				vto = kmap_atomic(page);
				vfrom = &(fetching_page->data->data);

				if(fetching_page->data->data_size==PAGE_SIZE)
					copy_user_page(vto, vfrom, address, page);
				else{
					WKdm_decompress(vfrom,vto);
				}

				// Ported to Linux 3.12 
				//kunmap_atomic(vto, KM_USER0);
				kunmap_atomic(vto);

				if(status==REPLICATION_STATUS_WRITTEN){
					if(page->old_page_version==NULL){
						page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
								GFP_ATOMIC);
						if(page->old_page_version==NULL){
							printk("ERROR: impossible to kmalloc old diff page\n");
							pcn_kmsg_free_msg(fetching_page->data);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_fetch_message;
						}
					}

					vto = page->old_page_version;
					// Ported to Linux 3.12 
					//vfrom = kmap_atomic(page, KM_USER0);
					vfrom = kmap_atomic(page);
					memcpy(vto, vfrom, PAGE_SIZE);
					// Ported to Linux 3.12 
					//kunmap_atomic(vto, KM_USER0);
					kunmap_atomic(vto);
				}
#else
				// Ported to Linux 3.12 
				//vto = kmap_atomic(page, KM_USER0);
				vto = kmap_atomic(page);
				vfrom = &(fetching_page->data->data);
				copy_user_page(vto, vfrom, address, page);
				// Ported to Linux 3.12 
				//kunmap_atomic(vto, KM_USER0);
				kunmap_atomic(vto);
#if READ_PAGE
				int ct=0;
				if(address == PAGE_ADDR)
					for(ct=0;ct<8;ct++){
						printk(KERN_ALERT"{%lx} ",(unsigned long) *(((unsigned long *)vfrom)+ct));
					}
			}
#endif

#if CHECKSUM
			// Ported to Linux 3.12 
			//vto= kmap_atomic(page, KM_USER0);
			vto= kmap_atomic(page);
			__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
			// Ported to Linux 3.12 
			//kunmap_atomic(vto, KM_USER0);
			kunmap_atomic(vto);
			__wsum check2= csum_partial(&(fetching_page->data->data), PAGE_SIZE, 0);


			if(check1!=check2) {
				printk("ERROR: page just copied is not matching, address %lu\n",address);
				pcn_kmsg_free_msg(fetching_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}
			if(check1!=fetching_page->data->checksum) {
				printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
				pcn_kmsg_free_msg(fetching_page->data);
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}
#endif

#endif

			pcn_kmsg_free_msg(fetching_page->data);

			entry = mk_pte(page, vma->vm_page_prot);

			//if the page is read only no need to keep replicas coherent
			if (vma->vm_flags & VM_WRITE) {

				page->replicated = 1;

				if(fetching_page->is_write){
					page->last_write = fetching_page->last_write+1;
				}
				else
					page->last_write = fetching_page->last_write;

#if STATISTICS
				if(page->last_write> most_written_page)
					most_written_page= page->last_write;
#endif
				page->owner = fetching_page->owner;

				page->status = status;

				if (status == REPLICATION_STATUS_VALID) {
					entry = pte_clear_flags(entry, _PAGE_RW);
				} else {
					entry = pte_set_flags(entry, _PAGE_RW);
				}

			} else {
				if(fetching_page->is_write)
					printk("ERROR: trying to write a read only page\n");

				if(fetching_page->owner==1)
					printk("ERROR: received ownership with a copy of a read only page\n");

				page->replicated = 0;
				page->owner= 0;
				page->status= REPLICATION_STATUS_NOT_REPLICATED;

			}

			entry = pte_set_flags(entry, _PAGE_PRESENT);
			page->other_owners[_cpu]=1;
			page->other_owners[other_cpu]=1;
			page->futex_owner = fetching_page->futex_owner;//akshay

			flush_cache_page(vma, address, pte_pfn(*pte));

			entry = pte_set_flags(entry, _PAGE_USER);
			entry = pte_set_flags(entry, _PAGE_ACCESSED);


			ptep_clear_flush(vma, address, pte);



			page_add_new_anon_rmap(page, vma, address);
			set_pte_at_notify(mm, address, pte, entry);

			update_mmu_cache(vma, address, pte);
#if PRINT_PAGE
			unsigned char *print_ptr = vto;
			int i = 0;
			printk("\n====================================================");
			printk("\nPAGE WRITTEN:(0x%x)\n", address);
			for(i = 0; i < PAGE_SIZE;  i++) {
				if((i%32) == 0) {
					printk("\n");
				}
				printk("%02x ", *print_ptr);
				print_ptr++;
			}                       
			printk("\n====================================================\n");
#endif

		} else {
			printk("pte changed while fetching\n");
			status = REPLICATION_STATUS_INVALID;
			mem_cgroup_uncharge_page(page);
			page_cache_release(page);
			pcn_kmsg_free_msg(fetching_page->data);

		}

		PSPRINTK("End fetching address %lu \n", address);
		ret= 0;
		goto exit_fetch_message;

	}

	//copy not present on the other kernel
	else {

#if STATISTICS
		local_fetch++;
#endif
		PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lu\n", local_fetch, address);
		PSMINPRINTK("Local fetch for address %lu\n",address);
		kfree(fetch_message);
		ret = VM_CONTINUE_WITH_CHECK;
		goto exit;
	}

exit_fetch_message:

	kfree(fetch_message);

exit_fetching_page:

	remove_mapping_entry(fetching_page);
	kfree(fetching_page);

exit:
	if(strcmp(current->comm,"IS") == 0){
		trace_printk("e\n");
	}

	return ret;
}

#else
/* Fetch a page from the system => ask other kernels if they have a copy of the page at address "address" on the
 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
 *
 * down_read(&mm->mmap_sem) must be held.
 * pte lock must be held.
 *
 *return types:
 *VM_FAULT_OOM, problem allocating memory.
 *VM_FAULT_VMA, error vma management.
 *VM_FAULT_REPLICATION_PROTOCOL, general error.
 *VM_CONTINUE_WITH_CHECK, fetch the page locally.
 *0, remotely fetched;
 *-1, invalidated while fetching;
 * */
int do_remote_fetch(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
		spinlock_t* ptl, int _cpu) {

	mapping_answers_t* fetching_page;
	data_request_t* fetch_message;
	int i;
	unsigned long flags;
	int ret = 0;
	char lpath[512];

	/* I need to keep the information that this address is currently on a fetch phase.
	 * Store the info in an appropriate list.
	 * This allows the handlers of invalidation and request to maintain an updated status for the future page.
	 * Plus the answers to my fetch will update this object.
	 * Plus it will prevent multiple fetch of the same address.
	 */

	fetching_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
			GFP_ATOMIC);
	if (fetching_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}
	fetching_page->address = address;
	fetching_page->tgroup_home_cpu = tgroup_home_cpu;
	fetching_page->tgroup_home_id = tgroup_home_id;
	fetching_page->address_present = REPLICATION_STATUS_INVALID;
	fetching_page->data = NULL;
	fetching_page->fetching = 0;
	fetching_page->last_invalid = -1;
	fetching_page->last_write = 0;
	fetching_page->owner = -1;
	memset(fetching_page->owners, 0, sizeof(int) * MAX_KERNEL_IDS);
	fetching_page->vma_present = 0;
	fetching_page->vaddr_start = 0;
	fetching_page->vaddr_size = 0;
	fetching_page->vm_flags = 0;
	fetching_page->pgoff = 0;
	memset(fetching_page->path,0,sizeof(char)*512);
	memset(&(fetching_page->prot),0,sizeof(pgprot_t));
	raw_spin_lock_init(&(fetching_page->lock));
	fetching_page->responses = 0;
	fetching_page->waiting = current;

	// Insert the object in the appropriate list.
	add_mapping_entry(fetching_page);

	//create the message to broadcast to other kernels
	fetch_message = (data_request_t*) kmalloc(sizeof(data_request_t),
			GFP_ATOMIC);
	if (fetch_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_fetching_page;
	}

	fetch_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	fetch_message->address = address;
	fetch_message->tgroup_home_cpu = tgroup_home_cpu;
	fetch_message->tgroup_home_id = tgroup_home_id;
	fetch_message->read_for_write = 0;
	fetch_message->vma_operation_index= current->mm->vma_operation_index;

#if STATISTICS
	fetch++;
#endif
	PSPRINTK("Fetch %i address %lu \n", fetch, address);
	PSMINPRINTK("Fetch %i address %lu \n", fetch, address);

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	//send to all cpus
	fetching_page->expected_responses = 0;

	memory_t* memory= find_memory_entry(current->tgroup_home_cpu,
			current->tgroup_home_id);

	down_read(&memory->kernel_set_sem);

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
			if(memory->kernel_set[i]==1)
				if (pcn_kmsg_send(i, (struct pcn_kmsg_message*) (fetch_message))
						!= -1) {
					// Message delivered
					fetching_page->expected_responses++;
				}
		}

		up_read(&memory->kernel_set_sem);

		//wait while all the reachable cpus send back an answer
		while (fetching_page->expected_responses != fetching_page->responses) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if (fetching_page->expected_responses != fetching_page->responses) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);
		}

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/

		PSPRINTK("Out wait fetch %i address %lu \n", fetch, address);

		raw_spin_lock_irqsave(&(fetching_page->lock), flags);
		raw_spin_unlock_irqrestore(&(fetching_page->lock), flags);

#if NOT_REPLICATED_VMA_MANAGEMENT
		//only the client has to update the vma
		if(tgroup_home_cpu!=_cpu)
#endif

		{
			ret = do_mapping_for_distributed_process(fetching_page, mm, address, ptl);
			if (ret != 0)
				goto exit_fetch_message;

			vma = find_vma(mm, address);
			if (!vma || address >= vma->vm_end || address < vma->vm_start) {
				vma = NULL;
			} else if (unlikely(is_vm_hugetlb_page(vma))
					|| unlikely(transparent_hugepage_enabled(vma))) {
				printk("ERROR: Installed a vma with HUGEPAGE\n");
				ret = VM_FAULT_VMA;
				goto exit_fetch_message;
			}

			if (vma == NULL) {
				PSPRINTK("ERROR: no vma for address %lu in the system\n", address);
				ret = VM_FAULT_VMA;
				goto exit_fetch_message;
			}
		}
		/*Check if someone sent a copy of the page.
		 *If there not exist valid copies, a copy is locally fetched.
		 */
		if (fetching_page->address_present != REPLICATION_STATUS_INVALID) {
			struct page* page;
			spin_unlock(ptl);
			/*PTE UNLOCKED*/

			if (unlikely(anon_vma_prepare(vma))) {
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

			page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
			if (!page) {
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

			__SetPageUptodate(page);

			if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
				page_cache_release(page);
				spin_lock(ptl);
				/*PTE LOCKED*/
				ret = VM_FAULT_OOM;
				goto exit_fetch_message;
			}

			//process_server_clean_page(page);
#if STATISTICS
			pages_allocated++;
#endif
			spin_lock(ptl);
			/*PTE LOCKED*/

			int status;
			//if nobody changed the pte
			if (likely(pte_same(*pte, value_pte))) {

				/*If an invalid message arrived for the oldest copy that I received,
				 *the copy should be discarded and not installed.
				 */
				if (fetching_page->last_invalid >= fetching_page->last_write) {
					status = REPLICATION_STATUS_INVALID;
					PSPRINTK("Page will be installed as invalid\n");
				} else
					status = REPLICATION_STATUS_VALID;

				void *vto;
				void *vfrom;
				//copy into the page the copy received
				if (status == REPLICATION_STATUS_VALID) {

					if (fetching_page->data->address != address) {
						printk("ERROR: trying to copy wrong address!");
						pcn_kmsg_free_msg(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
					// Ported to Linux 3.12 
					//vto = kmap_atomic(page, KM_USER0);
					vto = kmap_atomic(page);
					vfrom = fetching_page->data->data;
					copy_user_page(vto, vfrom, address, page);
					// Ported to Linux 3.12 
					//kunmap_atomic(vto, KM_USER0);
					kunmap_atomic(vto);

#if CHECKSUM
					// Ported to Linux 3.12 
					//vto= kmap_atomic(page, KM_USER0);
					vto= kmap_atomic(page);
					__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
					// Ported to Linux 3.12 
					//kunmap_atomic(vto, KM_USER0);
					kunmap_atomic(vto);
					__wsum check2= csum_partial(fetching_page->data->data, PAGE_SIZE, 0);
					if(check1!=check2) {
						printk("ERROR: page just copied is not matching, address %lu\n",address);
						pcn_kmsg_free_msg(fetching_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
					if(check1!=fetching_page->data->checksum) {
						printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
						pcn_kmsg_free_msg(fetching_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
#endif

				}

				pcn_kmsg_free_msg(fetching_page->data);

				pte_t entry = mk_pte(page, vma->vm_page_prot);

				//if the page is read only no need to keep replicas coherent
				if (vma->vm_flags & VM_WRITE) {

					page->replicated = 1;

					page->last_write = fetching_page->last_write;

#if STATISTICS
					if(page->last_write> most_written_page)
						most_written_page= page->last_write;
#endif

					memcpy(page->other_owners, fetching_page->owners,
							sizeof(int) * MAX_KERNEL_IDS);
					page->other_owners[_cpu] = 1;
					page->owner = fetching_page->owner;

					if (status == REPLICATION_STATUS_VALID) {
						page->status = REPLICATION_STATUS_VALID;
						entry = pte_set_flags(entry, _PAGE_PRESENT);
						entry = pte_clear_flags(entry, _PAGE_RW);
					} else {
						entry = pte_clear_flags(entry, _PAGE_PRESENT);
						page->status = REPLICATION_STATUS_INVALID;
					}

				} else {
					page->replicated = 0;
					memcpy(page->other_owners, fetching_page->owners,
							sizeof(int) * MAX_KERNEL_IDS);
					page->other_owners[_cpu] = 1;
					entry = pte_set_flags(entry, _PAGE_PRESENT);
				}

				flush_cache_page(vma, address, pte_pfn(*pte));

				entry = pte_set_flags(entry, _PAGE_USER);
				entry = pte_set_flags(entry, _PAGE_ACCESSED);

				ptep_clear_flush(vma, address, pte);

				page_add_new_anon_rmap(page, vma, address);

				set_pte_at_notify(mm, address, pte, entry);

				update_mmu_cache(vma, address, pte);

				//flush_tlb_page(vma, address);

				//flush_tlb_fix_spurious_fault(vma, address);

			} else {
				status = REPLICATION_STATUS_INVALID;
				mem_cgroup_uncharge_page(page);
				page_cache_release(page);
				pcn_kmsg_free_msg(fetching_page->data);
			}

			PSPRINTK("End fetching address %lu \n", address);
			if (status == REPLICATION_STATUS_INVALID)
				ret = -1;
			else
				ret = 0;
			goto exit_fetch_message;

		}

		//copies not present on other kernels
		else {
			//I am the only using it => no need of replication until someone asks for it
#if STATISTICS
			local_fetch++;
#endif
			PSPRINTK(
					"Copy not present in the system, local fetch %d of address %lu\n", local_fetch, address);

			kfree(fetch_message);
			ret = VM_CONTINUE_WITH_CHECK;
			goto exit;
		}

exit_fetch_message:

		kfree(fetch_message);

exit_fetching_page:

		remove_mapping_entry(fetching_page);
		kfree(fetching_page);

exit:

		return ret;

}

#endif
