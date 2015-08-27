
#include "popcorn.h"

#if FOR_2_KERNELS

void process_mapping_request_for_2_kernels(struct work_struct* work, int _cpu,
					   struct workqueue_struct *message_request_wq){

      request_work_t* request_work = (request_work_t*) work;
      data_request_for_2_kernels_t* request = request_work->request;
      memory_t * memory;
      struct mm_struct* mm = NULL;
      struct vm_area_struct* vma = NULL;
      data_void_response_for_2_kernels_t* void_response;
      int owner= 0;
      char* plpath;
      char lpath[512];
      int from_cpu = request->header.from_cpu;
      unsigned long address = request->address & PAGE_MASK;
      pgd_t* pgd;
      pud_t* pud;
      pmd_t* pmd;
      pte_t* pte;
      pte_t entry;
      spinlock_t* ptl;
      request_work_t* delay;
      struct page* page, *old_page;
      data_response_for_2_kernels_t* response;
      mapping_answers_for_2_kernels_t* fetched_data;
      int lock =0;
      void *vfrom;

      trace_printk("s\n");
#if STATISTICS
	      request_data++;
#endif

	      PSMINPRINTK("Request for address %lu is fetch %i is write %i\n", request->address,((request->is_fetch==1)?1:0),((request->is_write==1)?1:0));
	      PSPRINTK("Request %i address %lu is fetch %i is write %i\n", request_data, request->address,((request->is_fetch==1)?1:0),((request->is_write==1)?1:0));

	      memory = find_memory_entry(request->tgroup_home_cpu,
			      request->tgroup_home_id);
	      if (memory != NULL) {
		      if(memory->setting_up==1){
			      owner=1;
			      goto out;
		      }
		      mm = memory->mm;
	      } else {
		      owner=1;
		      goto out;
	      }

	      down_read(&mm->mmap_sem);

	      //check the vma era first
	      if(mm->vma_operation_index < request->vma_operation_index){
		      printk("different era request mm->vma_operation_index %d request->vma_operation_index %d\n",mm->vma_operation_index,request->vma_operation_index);
		      delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);

		      if (delay) {
			      delay->request = request;
			      INIT_DELAYED_WORK( (struct delayed_work*)delay,
					      process_mapping_request_for_2_kernels);
			      queue_delayed_work(message_request_wq,
					      (struct delayed_work*) delay, 10);
		      }

		      up_read(&mm->mmap_sem);
		      kfree(work);
		      trace_printk("e\n");
		      return;
	      }

	      // check if there is a valid vma
	      vma = find_vma(mm, address);
	      if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		      vma = NULL;
		      if(_cpu == request->tgroup_home_cpu){
			      printk(KERN_ALERT"%s: vma NULL in xeon address{%lu} \n",__func__,address);
			      up_read(&mm->mmap_sem);
			      goto out;
		      }
	      } else {

		      if (unlikely(is_vm_hugetlb_page(vma))
				      || unlikely(transparent_hugepage_enabled(vma))) {
			      printk("ERROR: Request for HUGE PAGE vma\n");
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      PSPRINTK(
				      "Find vma from %s start %lu end %lu\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"), vma->vm_start, vma->vm_end);

	      }

		  PSPRINTK("In %s:%d vma_flags = %lx\n", __func__, __LINE__, vma->vm_flags);	

		  if(vma->vm_flags & VM_FETCH_LOCAL)
		  {
			  PSPRINTK("%s:%d - VM_FETCH_LOCAL flag set - Going to void response\n", __func__, __LINE__);
			  goto out;
		  }

		  /*if((vma->vm_flags & VM_EXEC) &&(address >= mm->start_code) && (address <= mm->end_code))
		  {
			  printk("%s:%d going to void response\n", __func__, __LINE__);
			  goto out;
		  }*/

	      if(_cpu!=request->tgroup_home_cpu){

		      pgd = pgd_offset(mm, address);
		      if (!pgd || pgd_none(*pgd)) {
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      pud = pud_offset(pgd, address);
		      if (!pud || pud_none(*pud)) {
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      pmd = pmd_offset(pud, address);

		      if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
			      up_read(&mm->mmap_sem);
			      goto out;
		      }
	      }
	      else{

		      pgd = pgd_offset(mm, address);

		      pud = pud_alloc(mm, pgd, address);
		      if (!pud){
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      pmd = pmd_alloc(mm, pud, address);
		      if (!pmd){
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      if (pmd_none(*pmd) && __pte_alloc(mm, vma, pmd, address)){
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      if (unlikely(pmd_trans_huge(*pmd))) {
			      printk("ERROR: request for huge page\n");
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

	      }

       pte = pte_offset_map_lock(mm, pmd, address, &ptl);
       /*PTE LOCKED*/

       entry = *pte;
       lock= 1;


       if (pte == NULL || pte_none(pte_clear_flags(entry, _PAGE_UNUSED1))) {

	       PSPRINTK("pte not mapped \n");

	       if( !pte_none(entry) ){

		       if(_cpu!=request->tgroup_home_cpu || request->is_fetch==1){
			       printk("ERROR: incorrect request for marked page\n");
			       goto out;
		       }
		       else{
			       PSPRINTK("request for a marked page\n");
		       }
	       }

	       if ((_cpu==request->tgroup_home_cpu) || memory->alive != 0) {

		       fetched_data = find_mapping_entry(
				       request->tgroup_home_cpu, request->tgroup_home_id, address);

		       //case concurrent fetch
		       if (fetched_data != NULL) {

fetch:				PSPRINTK("concurrent request\n");

				/*Whit marked pages only two scenarios can happenn:
				 * Or I am the main and I an locally fetching=> delay this fetch
				 * Or I am not the main, but the main already answer to my fetch (otherwise it will not answer to me the page)
				 * so wait that the answer arrive before consuming the fetch.
				 * */
				if (fetched_data->is_fetch != 1)
					printk(
							"ERROR: find a mapping_answers_for_2_kernels_t not mapped and not fetch\n");

				delay = (request_work_t*)kmalloc(sizeof(request_work_t),
						GFP_ATOMIC);

				if (delay!=NULL) {
					delay->request = request;
					INIT_DELAYED_WORK(
							(struct delayed_work*)delay,
							process_mapping_request_for_2_kernels);
					queue_delayed_work(message_request_wq,
							(struct delayed_work*) delay, 10);
				}

				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				kfree(work);
				trace_printk("e\n");
				return;

		       }

		       else{
			       //mark the pte if main
			       if(_cpu==request->tgroup_home_cpu){

				       PSPRINTK(KERN_ALERT"%s: marking a pte for address %lu \n",__func__,address);

				       entry = pte_set_flags(entry, _PAGE_UNUSED1);

				       ptep_clear_flush(vma, address, pte);

				       set_pte_at_notify(mm, address, pte, entry);
				       //in x86 does nothing
				       update_mmu_cache(vma, address, pte);

			       }
		       }
	       }
	       //pte not present
	       owner= 1;
	       goto out;

       }

       page = pte_page(entry);
       if (page != vm_normal_page(vma, address, entry)) {
	       PSPRINTK("Page different from vm_normal_page in request page\n");
       }
       old_page = NULL;

       if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {

	       PSPRINTK("Page not replicated\n");

	       /*There is the possibility that this request arrived while I am fetching, after that I installed the page
		* but before calling the update page....
		* */
	       if (memory->alive != 0) {
		       fetched_data = find_mapping_entry(
				       request->tgroup_home_cpu, request->tgroup_home_id, address);

		       if(fetched_data!=NULL){
			       goto fetch;
		       }
	       }

	       //the request must be for a fetch
	       if(request->is_fetch==0)
		       printk("ERROR received a request not fetch for a not replicated page\n");

	       if (vma->vm_flags & VM_WRITE) {

		       //if the page is writable but the pte has not the write flag set, it is a cow page
		       if (!pte_write(entry)) {

retry_cow:
			       PSPRINTK("COW page at %lu \n", address);

			       int ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);

			       if (ret & VM_FAULT_ERROR) {
				       if (ret & VM_FAULT_OOM){
					       printk("ERROR: %s VM_FAULT_OOM\n",__func__);
					       up_read(&mm->mmap_sem);
					       goto out;
				       }

				       if (ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
					       printk("ERROR: %s EHWPOISON\n",__func__);
					       up_read(&mm->mmap_sem);
					       goto out;
				       }

				       if (ret & VM_FAULT_SIGBUS){
					       printk("ERROR: %s EFAULT\n",__func__);
					       up_read(&mm->mmap_sem);
					       goto out;
				       }

				       printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
				       up_read(&mm->mmap_sem);
				       goto out;
			       }

			       spin_lock(ptl);
			       /*PTE LOCKED*/
			       lock = 1;

			       entry = *pte;

			       if(!pte_write(entry)){
				       printk("WARNING: page not writable after cow\n");
				       goto retry_cow;
			       }

			       page = pte_page(entry);

		       }

		       page->replicated = 1;

		       flush_cache_page(vma, address, pte_pfn(*pte));
		       entry = mk_pte(page, vma->vm_page_prot);

		       if(request->is_write==0){
			       //case fetch for read
			       page->status = REPLICATION_STATUS_VALID;
			       entry = pte_clear_flags(entry, _PAGE_RW);
			       entry = pte_set_flags(entry, _PAGE_PRESENT);
			       owner= 0;
			       page->owner= 1;
		       }
		       else{
			       //case fetch for write
			       page->status = REPLICATION_STATUS_INVALID;
			       entry = pte_clear_flags(entry, _PAGE_PRESENT);
			       owner= 1;
			       page->owner= 0;
		       }

		       page->last_write= 1;

		       entry = pte_set_flags(entry, _PAGE_USER);
		       entry = pte_set_flags(entry, _PAGE_ACCESSED);

		       ptep_clear_flush(vma, address, pte);

		       set_pte_at_notify(mm, address, pte, entry);

		       //in x86 does nothing
		       update_mmu_cache(vma, address, pte);

		       if (old_page != NULL){
			       page_remove_rmap(old_page);
		       }
	       } else {
		       //read only vma
		       page->replicated=0;
		       page->status= REPLICATION_STATUS_NOT_REPLICATED;

		       if(request->is_write==1){
			       printk("ERROR: received a write in a read-only not replicated page\n");
		       }
		       page->owner= 1;
		       owner= 0;
	       }

	       page->other_owners[_cpu]=1;
	       page->other_owners[from_cpu]=1;

	       goto resolved;
       }
       else{
	       //replicated page case
	       PSPRINTK("Page replicated...\n");

	       if(request->is_fetch==1){
		       printk("ERROR: received a fetch request in a replicated status\n");
	       }

	       if(page->writing==1){

		       PSPRINTK("Page currently in writing \n");


		       if(request->is_write==0){
			       PSPRINTK("Concurrent read request\n");
		       }
		       else{

			       PSPRINTK("Concurrent write request\n");
		       }
		       delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);

		       if (delay!=NULL) {
			       delay->request = request;
			       INIT_DELAYED_WORK( (struct delayed_work*)delay,
					       process_mapping_request_for_2_kernels);
			       queue_delayed_work(message_request_wq,
					       (struct delayed_work*) delay, 10);
		       }

		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       kfree(work);
		       trace_printk("e\n");
		       return;

	       }

	       if(page->reading==1){

		       printk("ERROR: page in reading but received a request\n");
		       goto out;
	       }

	       //invalid page case
	       if (page->status == REPLICATION_STATUS_INVALID) {

		       printk("ERROR: received a request in invalid status without reading or writing\n");
		       goto out;
	       }

	       //valid page case
	       if (page->status == REPLICATION_STATUS_VALID) {

		       PSPRINTK("Page requested valid\n");

		       if(page->owner!=1)
			       printk("ERROR: request in a not owner valid page\n");
		       else{
			       if(request->is_write){
				       if(page->last_write!= request->last_write)
					       printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				       page->status= REPLICATION_STATUS_INVALID;
				       page->owner= 0;
				       owner= 1;
				       entry = *pte;
				       entry = pte_clear_flags(entry, _PAGE_PRESENT);
				       entry = pte_set_flags(entry, _PAGE_ACCESSED);

				       ptep_clear_flush(vma, address, pte);

				       set_pte_at_notify(mm, address, pte, entry);

				       update_mmu_cache(vma, address, pte);
			       }
			       else{
				       printk("ERROR: received a read request in valid status\n");
			       }
		       }

		       goto out;
	       }

	       if (page->status == REPLICATION_STATUS_WRITTEN) {

		       PSPRINTK("Page requested in written status\n");

		       if(page->owner!=1)
			       printk("ERROR: page in written status without ownership\n");
		       else{
			       if(request->is_write==1){

				       if(page->last_write!= (request->last_write+1))
					       printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				       page->status= REPLICATION_STATUS_INVALID;
				       page->owner= 0;
				       owner= 1;
				       entry = *pte;
				       entry = pte_clear_flags(entry, _PAGE_PRESENT);
				       entry = pte_set_flags(entry, _PAGE_ACCESSED);

				       ptep_clear_flush(vma, address, pte);

				       set_pte_at_notify(mm, address, pte, entry);

				       update_mmu_cache(vma, address, pte);
			       }
			       else{

				       if(page->last_write!= (request->last_write+1))
					       printk("ERROR: received an read for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				       page->status = REPLICATION_STATUS_VALID;
				       page->owner= 1;
				       owner= 0;
				       entry = *pte;
				       entry = pte_set_flags(entry, _PAGE_PRESENT);
				       entry = pte_set_flags(entry, _PAGE_ACCESSED);
				       entry = pte_clear_flags(entry, _PAGE_RW);

				       ptep_clear_flush(vma, address, pte);

				       set_pte_at_notify(mm, address, pte, entry);

				       update_mmu_cache(vma, address, pte);
			       }
		       }

#if DIFF_PAGE
		       goto resolved_diff;
#else
		       goto resolved;
#endif
	       }

       }

resolved:

       PSPRINTK(
		       "Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

       PSPRINTK(
		       "Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

#if DIFF_PAGE

       char *app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
       if (app == NULL) {
	       printk("Impossible to kmalloc app.\n");
	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
	       pcn_kmsg_free_msg(request);
	       kfree(work);
	       return;
       }

       // Ported to Linux 3.12 
       //vfrom = kmap_atomic(page, KM_USER0);
       vfrom = kmap_atomic(page);

       unsigned int compressed_byte= WKdm_compress(vfrom,app);

       if(compressed_byte<((PAGE_SIZE/10)*9)){

#if STATISTICS
	       compressed_page_sent++;
#endif
	       // Ported to Linux 3.12 
	       //kunmap_atomic(vfrom, KM_USER0);
	       kunmap_atomic(vfrom);
	       response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+compressed_byte, GFP_ATOMIC);
	       if (response == NULL) {
		       printk("Impossible to kmalloc in process mapping request.\n");
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       pcn_kmsg_free_msg(request);
		       kfree(work);
		       kfree(app);
		       return;
	       }
	       memcpy(&(response->data),app,compressed_byte);
	       response->data_size= compressed_byte;
	       kfree(app);
       }
       else{

#if STATISTICS
	       not_compressed_page++;
#endif

	       response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
	       if (response == NULL) {
		       printk("Impossible to kmalloc in process mapping request.\n");
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       pcn_kmsg_free_msg(request);
		       kfree(work);
		       kfree(app);
		       return;
	       }
	       void* vto = &(response->data);
	       copy_page(vto, vfrom);
	       kunmap_atomic(vfrom, KM_USER0);
	       response->data_size= PAGE_SIZE;
	       kfree(app);
       }

       response->diff=0;

#else
       response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
       if (response == NULL) {
	       printk("Impossible to kmalloc in process mapping request.\n");
	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
	       pcn_kmsg_free_msg(request);
	       kfree(work);
	       return;
       }

       void* vto = &(response->data);
       // Ported to Linux 3.12 
       //vfrom = kmap_atomic(page, KM_USER0);
       vfrom = kmap_atomic(page);

#if READ_PAGE
       int ct=0;
       unsigned long _buff[16];

       if(address == PAGE_ADDR){
	       for(ct=0;ct<8;ct++){
		       _buff[ct]=(unsigned long) *(((unsigned long *)vfrom) + ct);
	       }
       }
#endif

       //printk("Copying page (address) : 0x%lx\n", address);
       copy_page(vto, vfrom);
       
       // Ported to Linux 3.12 
       //kunmap_atomic(vfrom, KM_USER0);
       kunmap_atomic(vfrom);

#if PRINT_PAGE
	unsigned char *print_ptr = vto;
	int i = 0;
	printk("\n================================================");
	printk("\nPAGE COPIED:\n");
	/* Print the needed code section */
	print_ptr = vto;
	for(i = 0; i < PAGE_SIZE;  i++) {
		if((i%32) == 0){
			printk("\n");
		}
               	printk("%02x ", *print_ptr);
		print_ptr++;
	}
	printk("\n================================================\n");
#endif
       response->data_size= PAGE_SIZE;


#if READ_PAGE
       if(address == PAGE_ADDR){
	       for(ct=8;ct<16;ct++){
		       _buff[ct]=(unsigned long) *((unsigned long*)(&(response->data))+ct-8);
	       }
	       for(ct=0;ct<16;ct++){
		       printk(KERN_ALERT"{%lx} ",_buff[ct]);
	       }
       }
#endif

#if CHECKSUM
       // Ported to Linux 3.12
       // vfrom= kmap_atomic(page, KM_USER0);
       vfrom= kmap_atomic(page);
       __wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
       // Ported to Linux 3.12 
       //kunmap_atomic(vfrom, KM_USER0);
       kunmap_atomic(vfrom);
       __wsum check2= csum_partial(&(response->data), PAGE_SIZE, 0);
       if(check1!=check2)
	       printk("page just copied is not matching, address %lu\n",address);
#endif

#endif

       flush_cache_page(vma, address, pte_pfn(*pte));

       response->last_write = page->last_write;

       response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
       response->header.prio = PCN_KMSG_PRIO_NORMAL;
       response->tgroup_home_cpu = request->tgroup_home_cpu;
       response->tgroup_home_id = request->tgroup_home_id;
       response->address = request->address;
       response->owner= owner;

       response->futex_owner = (!page) ? 0 : page->futex_owner;//akshay

#if NOT_REPLICATED_VMA_MANAGEMENT
       if (_cpu == request->tgroup_home_cpu && vma != NULL)
	       //only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	       if (vma != NULL)
#endif
#endif
	       {

		       response->vma_present = 1;
		       response->vaddr_start = vma->vm_start;
		       response->vaddr_size = vma->vm_end - vma->vm_start;
		       response->prot = vma->vm_page_prot;
		       response->vm_flags = vma->vm_flags;
		       response->pgoff = vma->vm_pgoff;
		       if (vma->vm_file == NULL) {
			       response->path[0] = '\0';
		       } else {
			       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			       strcpy(response->path, plpath);
		       }
		       PSPRINTK("response->vma_present %d response->vaddr_start %lu response->vaddr_size %lu response->prot %lu response->vm_flags %lu response->pgoff %lu response->path %s response->futex_owner %d\n",
				       response->vma_present, response->vaddr_start , response->vaddr_size,response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);
	       }

	       else
		       response->vma_present = 0;

       spin_unlock(ptl);
       up_read(&mm->mmap_sem);

#if !DIFF_PAGE
#if CHECKSUM
       response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif
#endif

       trace_printk("m\n");
       // Send response
       pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		       sizeof(data_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr) + response->data_size);
       trace_printk("a\n");
       // Clean up incoming messages
       pcn_kmsg_free_msg(request);
       kfree(work);
       kfree(response);
       //end= native_read_tsc();
#if PRINT_PAGE
	unsigned char *print_ptr = vto;
	int i = 0;
	printk("\n=======================================================");
	printk("\nPAGE COPIED: (0x%x)\n", address);
	/* Print the needed code section */
	print_ptr = vto;
        
	for(i = 0; i < PAGE_SIZE;  i++) {
		if((i%32) == 0){
			printk("\n");
		}
		printk("%02x ", *print_ptr);
		print_ptr++;
	}
	printk("\n======================================================\n");        
#endif
       PSPRINTK("Handle request end\n");
       trace_printk("e\n");
       return;

#if DIFF_PAGE

resolved_diff:

       if(page->old_page_version==NULL){
	       printk("ERROR: no previous version of the page to calculate diff address %lu\n",address);
	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
	       pcn_kmsg_free_msg(request);
	       kfree(work);
	       return;
       }

       app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
       if (app == NULL) {
	       printk("Impossible to kmalloc app.\n");
	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
	       pcn_kmsg_free_msg(request);
	       kfree(work);
	       return;
       }

       // Ported to Linux 3.12
       //vfrom = kmap_atomic(page, KM_USER0);
       vfrom = kmap_atomic(page);

       compressed_byte= WKdm_diff_and_compress (page->old_page_version, vfrom, app);

       if(compressed_byte<((PAGE_SIZE/10)*9)){

#if STATISTICS
	       compressed_page_sent++;
#endif

	       // Ported to Linux 3.12 
	       //kunmap_atomic(vfrom, KM_USER0);
	       kunmap_atomic(vfrom);
	       response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+compressed_byte, GFP_ATOMIC);
	       if (response == NULL) {
		       printk("Impossible to kmalloc in process mapping request.\n");
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       pcn_kmsg_free_msg(request);
		       kfree(work);
		       kfree(app);
		       return;
	       }
	       memcpy(&(response->data),app,compressed_byte);
	       response->data_size= compressed_byte;
	       kfree(app);
       }
       else{

#if STATISTICS
	       not_compressed_page++;
	       not_compressed_diff_page++;
#endif

	       response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
	       if (response == NULL) {
		       printk("Impossible to kmalloc in process mapping request.\n");
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       pcn_kmsg_free_msg(request);
		       kfree(work);
		       kfree(app);
		       return;
	       }
	       void* vto = &(response->data);
	       copy_page(vto, vfrom);
	       kunmap_atomic(vfrom, KM_USER0);
	       response->data_size= PAGE_SIZE;
	       kfree(app);
       }

       response->diff=1;

       PSPRINTK(
		       "Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

       PSPRINTK(
		       "Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

       flush_cache_page(vma, address, pte_pfn(*pte));

       response->last_write = page->last_write;

       response->futex_owner = page->futex_owner;//akshay

       response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
       response->header.prio = PCN_KMSG_PRIO_NORMAL;
       response->tgroup_home_cpu = request->tgroup_home_cpu;
       response->tgroup_home_id = request->tgroup_home_id;
       response->address = request->address;
       response->owner= owner;

#if NOT_REPLICATED_VMA_MANAGEMENT
       if (_cpu == request->tgroup_home_cpu && vma != NULL)
	       //only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	       if (vma != NULL)
#endif
#endif
	       {

		       response->vma_present = 1;
		       response->vaddr_start = vma->vm_start;
		       response->vaddr_size = vma->vm_end - vma->vm_start;
		       response->prot = vma->vm_page_prot;
		       response->vm_flags = vma->vm_flags;
		       response->pgoff = vma->vm_pgoff;
		       if (vma->vm_file == NULL) {
			       response->path[0] = '\0';
		       } else {
			       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			       strcpy(response->path, plpath);
		       }
	       }

	       else
		       response->vma_present = 0;

       spin_unlock(ptl);
       up_read(&mm->mmap_sem);

       trace_printk("m\n");
       // Send response
       pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		       sizeof(data_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr) + response->data_size);
       trace_printk("a\n");
       // Clean up incoming messages
       pcn_kmsg_free_msg(request);
       kfree(work);
       kfree(response);
       //end= native_read_tsc();
       PSPRINTK("Handle request end\n");
       trace_printk("e\n");
       return;
#endif

out:

       PSPRINTK("sending void answer\n");

       void_response = (data_void_response_for_2_kernels_t*) kmalloc(
		       sizeof(data_void_response_for_2_kernels_t), GFP_ATOMIC);
       if (void_response == NULL) {
	       if(lock){
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
	       }
	       printk("Impossible to kmalloc in process mapping request.\n");
	       pcn_kmsg_free_msg(request);
	       kfree(work);
	       return;
       }

       void_response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID;
       void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
       void_response->tgroup_home_cpu = request->tgroup_home_cpu;
       void_response->tgroup_home_id = request->tgroup_home_id;
       void_response->address = request->address;
       void_response->owner=owner;

       void_response->futex_owner = 0;//TODO: page->futex_owner;//akshay


#if NOT_REPLICATED_VMA_MANAGEMENT
       if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
	       if (vma != NULL)
#endif
#endif
	       {
		       void_response->vma_present = 1;
		       void_response->vaddr_start = vma->vm_start;
		       void_response->vaddr_size = vma->vm_end - vma->vm_start;
		       void_response->prot = vma->vm_page_prot;
		       void_response->vm_flags = vma->vm_flags;
		       void_response->pgoff = vma->vm_pgoff;
		       if (vma->vm_file == NULL) {
			       void_response->path[0] = '\0';
		       } else {
			       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			       strcpy(void_response->path, plpath);
		       }
	       } else
		       void_response->vma_present = 0;

       if(lock){
	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
       }

       trace_printk("m\n");
       // Send response
       pcn_kmsg_send_long(from_cpu,
		       (struct pcn_kmsg_long_message*) (void_response),
		       sizeof(data_void_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr));
       trace_printk("a\n");
       // Clean up incoming messages
       pcn_kmsg_free_msg(request);
       kfree(void_response);
       kfree(work);
       //end= native_read_tsc();
       PSPRINTK("Handle request end\n");
       trace_printk("e\n");
}

#else

void process_mapping_request(struct work_struct* work) {

      request_work_t* request_work = (request_work_t*) work;
      data_request_t* request = request_work->request;

      int from_cpu = request->header.from_cpu;
      unsigned long address = request->address & PAGE_MASK;

      data_response_t* response;
      data_void_response_t* void_response;

      struct mm_struct* mm = NULL;
      struct vm_area_struct* vma = NULL;
      pgd_t* pgd;
      pud_t* pud;
      pmd_t* pmd;
      pte_t* pte;
      pte_t entry;
      spinlock_t* ptl;
      request_work_t* delay;
      struct page* page, *old_page;
      void* vto, *vfrom;
      int i;
      //int wake = 0;
      int fetching = 0;
      char* plpath;
      char lpath[512];
      int app[MAX_KERNEL_IDS];
      memory_t * memory;
      int owners[MAX_KERNEL_IDS];
      //unsigned long long start,end;

#if STATISTICS
	      request_data++;
#endif

	      PSPRINTK(
			      "Request %i address %lu from cpu %i\n", request_data, request->address, from_cpu);

	      //start= native_read_tsc();

	      memset(owners,0,MAX_KERNEL_IDS*sizeof(int));

	      memory = find_memory_entry(request->tgroup_home_cpu,
			      request->tgroup_home_id);
	      if (memory != NULL) {
		      if(memory->setting_up==1){
			      goto out;
		      }
		      mm = memory->mm;
	      } else {
		      goto out;
	      }

	      down_read(&mm->mmap_sem);

	      //check the vma era first
	      if(mm->vma_operation_index < request->vma_operation_index){

		      delay = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

		      if (delay) {
			      delay->request = request;
			      INIT_DELAYED_WORK( (struct delayed_work*)delay,
					      process_mapping_request);
			      queue_delayed_work(message_request_wq,
					      (struct delayed_work*) delay, 10);
		      }

		      up_read(&mm->mmap_sem);
		      kfree(work);
		      return;
	      }

	      // check if there is a valid vma
	      vma = find_vma(mm, address);
	      if (!vma || address >= vma->vm_end || address < vma->vm_start) {
		      vma = NULL;
	      } else {

		      if (unlikely(is_vm_hugetlb_page(vma))
				      || unlikely(transparent_hugepage_enabled(vma))) {
			      printk("ERROR: Request for HUGE PAGE vma\n");
			      up_read(&mm->mmap_sem);
			      goto out;
		      }

		      PSPRINTK(
				      "Find vma from %s start %lu end %lu\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"), vma->vm_start, vma->vm_end);

	      }

	      pgd = pgd_offset(mm, address);
	      if (!pgd || pgd_none(*pgd)) {
		      up_read(&mm->mmap_sem);
		      goto out;
	      }

	      pud = pud_offset(pgd, address);
	      if (!pud || pud_none(*pud)) {
		      up_read(&mm->mmap_sem);
		      goto out;
	      }

	      pmd = pmd_offset(pud, address);

	      if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
		      up_read(&mm->mmap_sem);
		      goto out;
	      }

retry: pte = pte_offset_map_lock(mm, pmd, address, &ptl);
       /*PTE LOCKED*/

       entry = *pte;

       if (pte == NULL || pte_none(entry)) {

	       PSPRINTK("pte not mapped \n");

	       if (memory->alive != 0) {
		       //Check if I am concurrently fetching the page
		       mapping_answers_t* fetched_data = find_mapping_entry(
				       request->tgroup_home_cpu, request->tgroup_home_id, address);

		       if (fetched_data != NULL) {
			       unsigned long flags;
			       raw_spin_lock_irqsave(&(fetched_data->lock), flags);
			       fetched_data->fetching = 1;
			       fetched_data->owners[from_cpu] = 1;
			       memcpy(owners,fetched_data->owners,MAX_KERNEL_IDS*sizeof(int));
			       owners[_cpu]=1;
			       raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);
			       fetching = 1;
			       PSPRINTK("Concurrently fetching the same address\n");
		       }

	       }

	       spin_unlock(ptl);
	       up_read(&mm->mmap_sem);
	       goto out;

       }

       page = pte_page(entry);
       if (page != vm_normal_page(vma, address, entry)) {
	       PSPRINTK("Page different from vm_normal_page in request page\n");
       }
       old_page = NULL;

       /*If the page is not replicated and not read only I have to replicate it.
	*If nobody previously asked for the page I am the owner=> it is valid
	*If the page is the zero page, trying to access to page fields give error => check first if it is a zero page
	*/
       if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {

	       PSPRINTK("Page not replicated\n");

	       if (vma->vm_flags & VM_WRITE) {

		       //if the page is writable but the pte has not the write flag set, it is a cow page
		       if (!pte_write(entry)) {
			       /*
				* I unlock because alloc page may go to sleep
				*/
			       PSPRINTK("COW page at %lu \n", address);

			       spin_unlock(ptl);
			       /*PTE UNLOCKED*/

			       old_page = page;

			       if (unlikely(anon_vma_prepare(vma))) {
				       up_read(&mm->mmap_sem);
				       goto out;
			       }

			       if (is_zero_page(pte_pfn(entry))) {

				       page = alloc_zeroed_user_highpage_movable(vma, address);
				       if (!page) {
					       up_read(&mm->mmap_sem);
					       goto out;
				       }

			       } else {

				       page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
				       if (!page) {
					       up_read(&mm->mmap_sem);
					       goto out;
				       }

				       copy_user_highpage(page, old_page, address, vma);
			       }

			       __SetPageUptodate(page);

			       if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
				       page_cache_release(page);
				       up_read(&mm->mmap_sem);
				       goto out;

			       }

			       spin_lock(ptl);
			       /*PTE LOCKED*/

			       //if somebody changed the pte
			       if (unlikely(!pte_same(*pte, entry))) {

				       mem_cgroup_uncharge_page(page);
				       page_cache_release(page);
				       spin_unlock(ptl);
				       goto retry;

			       } else {
				       page_add_new_anon_rmap(page, vma, address);
#if STATISTICS
				       pages_allocated++;
#endif

			       }
		       }


		       page->replicated = 1;
		       page->status = REPLICATION_STATUS_VALID;
		       page->other_owners[from_cpu] = 1;
		       page->other_owners[_cpu] = 1;

		       flush_cache_page(vma, address, pte_pfn(*pte));

		       entry = mk_pte(page, vma->vm_page_prot);
		       //I need to catch the next write access
		       entry = pte_clear_flags(entry, _PAGE_RW);
		       entry = pte_set_flags(entry, _PAGE_PRESENT);
		       entry = pte_set_flags(entry, _PAGE_USER);
		       entry = pte_set_flags(entry, _PAGE_ACCESSED);

		       ptep_clear_flush(vma, address, pte);

		       set_pte_at_notify(mm, address, pte, entry);

		       //in x86 does nothing
		       update_mmu_cache(vma, address, pte);

		       /*according to the cpu this function flushes
			* or the single address on the tlb
			* or all the tlb
			*if SMP it flushes all the others tlb
			*/
		       //flush_tlb_page(vma, address);

		       //should be same as flush_tlb_page
		       //flush_tlb_fix_spurious_fault(vma, address);

		       if (old_page != NULL)
			       page_remove_rmap(old_page);

	       } else {

		       page->other_owners[from_cpu] = 1;
		       page->other_owners[_cpu] = 1;
	       }

	       memcpy(owners,page->other_owners,MAX_KERNEL_IDS*sizeof(int));
       }

       //page replicated
       else {
	       PSPRINTK("Page replicated...\n");

	       if (page->writing == 1) {
		       PSPRINTK("Page currently in writing \n");

		       //I cannot put this thread on sleep otherwise I cannot consume other messages => re-queue the work
		       delay = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

		       if (delay) {
			       delay->request = request;
			       INIT_DELAYED_WORK( (struct delayed_work*)delay,
					       process_mapping_request);
			       queue_delayed_work(message_request_wq,
					       (struct delayed_work*) delay, 10);
		       }

		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       kfree(work);
		       return;

	       }

	       /*if (page->writing == 1 && page->reading == 1) {
		 if (request->read_for_write == 0) {
		 printk("ERROR: Consuming normal fetch in read write\n");
		 }
		 if (page->status != REPLICATION_STATUS_INVALID) {
		 printk("ERROR: Answering in read write with a copy.\n");
		 }
		 (page->concurrent_fetch)++;
		 wake = 1;
		 PSPRINTK("Page in reading for write received a request \n");
		 }*/

	       //invalid page case
	       if (page->status == REPLICATION_STATUS_INVALID) {
		       page->other_owners[from_cpu] = 1;
		       memcpy(owners,page->other_owners,MAX_KERNEL_IDS*sizeof(int));
		       spin_unlock(ptl);
		       up_read(&mm->mmap_sem);
		       PSPRINTK("Request in status invalid\n");
		       goto out;
	       }

	       //valid page case
	       if (page->status == REPLICATION_STATUS_VALID) {
		       page->other_owners[from_cpu] = 1;
		       PSPRINTK("Page requested valid\n");
		       goto resolved;
	       }

	       //if it is written I need to change status to avoid to write local the next time
	       if (page->status == REPLICATION_STATUS_WRITTEN) {

		       PSPRINTK("Page requested in written status\n");
		       page->other_owners[from_cpu] = 1;

		       if (request->read_for_write == 1) {

			       (page->concurrent_fetch)++;
			       PSPRINTK("Page requested from a read for write \n");

		       }
		       page->need_fetch[from_cpu] = 1;

		       if (page->concurrent_writers != page->concurrent_fetch) {
			       spin_unlock(ptl);
			       up_read(&mm->mmap_sem);
			       pcn_kmsg_free_msg(request);
			       kfree(work);
			       PSPRINTK(
					       "Waiting, page->concurrent_writers!=page->concurrent_fetch\n");
			       return;
		       }

		       page->status = REPLICATION_STATUS_VALID;

		       entry = *pte;
		       entry = pte_set_flags(entry, _PAGE_PRESENT);
		       entry = pte_set_flags(entry, _PAGE_ACCESSED);
		       entry = pte_clear_flags(entry, _PAGE_RW);

		       ptep_clear_flush(vma, address, pte);

		       set_pte_at_notify(mm, address, pte, entry);

		       update_mmu_cache(vma, address, pte);
		       //flush_tlb_page(vma, address);

		       //flush_tlb_fix_spurious_fault(vma, address);

		       response = (data_response_t*) kmalloc(sizeof(data_response_t),
				       GFP_ATOMIC);
		       if (response == NULL) {
			       spin_unlock(ptl);
			       up_read(&mm->mmap_sem);
			       pcn_kmsg_free_msg(request);
			       kfree(work);
			       printk("Impossible to kmalloc in process mapping request\n");
			       return;
		       }

#if NOT_REPLICATED_VMA_MANAGEMENT
		       if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
			       if (vma != NULL)
#endif
#endif
			       {
				       response->vma_present = 1;
				       response->vaddr_start = vma->vm_start;
				       response->vaddr_size = vma->vm_end - vma->vm_start;
				       response->prot = vma->vm_page_prot;
				       response->vm_flags = vma->vm_flags;
				       response->pgoff = vma->vm_pgoff;
				       if (vma->vm_file == NULL) {
					       response->path[0] = '\0';
				       } else {
					       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
					       strcpy(response->path, plpath);
				       }
			       } else
				       response->vma_present = 0;

		       vto = response->data;
		       // Ported to Linux 3.12
		       //vfrom = kmap_atomic(page, KM_USER0);
		       vfrom = kmap_atomic(page);
		       copy_page(vto, vfrom);
		       // Ported to Linux 3.12
		       //kunmap_atomic(vfrom, KM_USER0);
		       kunmap_atomic(vfrom);

#if CHECKSUM
		       // Ported to Linux 3.12
		       //vfrom= kmap_atomic(page, KM_USER0);
		       vfrom= kmap_atomic(page);
		       __wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
		       // Ported to Linux 3.12
		       //kunmap_atomic(vfrom, KM_USER0);
		       kunmap_atomic(vfrom);
		       __wsum check2= csum_partial(&response->data, PAGE_SIZE, 0);
		       if(check1!=check2)
			       printk("page just copied is not matching, address %lu\n",address);
#endif

		       response->last_write = page->last_write;
		       for (i = 0; i < MAX_KERNEL_IDS; i++) {
			       response->owners[i] = page->other_owners[i];
		       }

		       response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
		       response->header.prio = PCN_KMSG_PRIO_NORMAL;
		       response->tgroup_home_cpu = request->tgroup_home_cpu;
		       response->tgroup_home_id = request->tgroup_home_id;
		       response->address = request->address;
		       response->address_present = REPLICATION_STATUS_WRITTEN;

#if CHECKSUM
		       response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif

		       page->concurrent_writers = 0;
		       page->concurrent_fetch = 0;
		       page->time_stamp = 0;

		       for (i = 0; i < MAX_KERNEL_IDS; i++)
			       if (page->need_fetch[i]) {
				       app[i] = 1;
				       page->need_fetch[i] = 0;
			       } else
				       app[i] = 0;

		       spin_unlock(ptl);

		       for (i = 0; i < MAX_KERNEL_IDS; i++)
			       if (app[i]) {
				       pcn_kmsg_send_long(i,
						       (struct pcn_kmsg_long_message*) (response),
						       sizeof(data_response_t)
						       - sizeof(struct pcn_kmsg_hdr));
			       }

		       up_read(&mm->mmap_sem);
		       kfree(response);
		       pcn_kmsg_free_msg(request);
		       kfree(work);
		       PSPRINTK("End request in written page \n");
		       return;

	       }
       }

resolved:

       response = (data_response_t*) kmalloc(sizeof(data_response_t), GFP_ATOMIC);
       if (response == NULL) {
	       printk("Impossible to kmalloc in process mapping request.\n");
	       return;
       }

       PSPRINTK(
		       "Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

       PSPRINTK(
		       "Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

       flush_cache_page(vma, address, pte_pfn(*pte));

       vto = response->data;
       // Ported to Linux 3.12
       //vfrom = kmap_atomic(page, KM_USER0);
       vfrom = kmap_atomic(page);
       copy_page(vto, vfrom);
       // Ported to Linux 3.12
       //kunmap_atomic(vfrom, KM_USER0);
       kunmap_atomic(vfrom);

#if CHECKSUM
       // Ported to Linux 3.12
       //vfrom= kmap_atomic(page, KM_USER0);
       vfrom= kmap_atomic(page);
       __wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
       // Ported to Linux 3.12
       //kunmap_atomic(vfrom, KM_USER0);
       kunmap_atomic(vfrom);
       __wsum check2= csum_partial(&response->data, PAGE_SIZE, 0);
       if(check1!=check2)
	       printk("page just copied is not matching, address %lu\n",address);
#endif

       response->last_write = page->last_write;

       for (i = 0; i < MAX_KERNEL_IDS; i++) {
	       response->owners[i] = page->other_owners[i];
       }

       response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
       response->header.prio = PCN_KMSG_PRIO_NORMAL;
       response->tgroup_home_cpu = request->tgroup_home_cpu;
       response->tgroup_home_id = request->tgroup_home_id;
       response->address = request->address;
       response->address_present = REPLICATION_STATUS_VALID;

#if NOT_REPLICATED_VMA_MANAGEMENT
       if (_cpu == request->tgroup_home_cpu && vma != NULL)
	       //only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	       if (vma != NULL)
#endif
#endif
	       {

		       response->vma_present = 1;
		       response->vaddr_start = vma->vm_start;
		       response->vaddr_size = vma->vm_end - vma->vm_start;
		       response->prot = vma->vm_page_prot;
		       response->vm_flags = vma->vm_flags;
		       response->pgoff = vma->vm_pgoff;
		       if (vma->vm_file == NULL) {
			       response->path[0] = '\0';
		       } else {
			       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			       strcpy(response->path, plpath);
		       }
	       }

	       else
		       response->vma_present = 0;

       spin_unlock(ptl);
       up_read(&mm->mmap_sem);

#if CHECKSUM
       response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif

       // Send response
       pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		       sizeof(data_response_t) - sizeof(struct pcn_kmsg_hdr));

       // Clean up incoming messages
       pcn_kmsg_free_msg(request);
       kfree(work);
       kfree(response);
       //end= native_read_tsc();
       PSPRINTK("Handle request end\n");
       return;

out:

       PSPRINTK("There are no copies of the page...\n");

       void_response = (data_void_response_t*) kmalloc(
		       sizeof(data_void_response_t), GFP_ATOMIC);
       if (void_response == NULL) {
	       printk("Impossible to kmalloc in process mapping request.\n");
	       return;
       }

       void_response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID;
       void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
       void_response->tgroup_home_cpu = request->tgroup_home_cpu;
       void_response->tgroup_home_id = request->tgroup_home_id;
       void_response->address = request->address;
       void_response->address_present = REPLICATION_STATUS_INVALID;
       memcpy(void_response->owners,owners,MAX_KERNEL_IDS*sizeof(int));

       if (fetching)
	       void_response->fetching = 1;
       else
	       void_response->fetching = 0;

#if NOT_REPLICATED_VMA_MANAGEMENT
       if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
	       if (vma != NULL)
#endif
#endif
	       {
		       void_response->vma_present = 1;
		       void_response->vaddr_start = vma->vm_start;
		       void_response->vaddr_size = vma->vm_end - vma->vm_start;
		       void_response->prot = vma->vm_page_prot;
		       void_response->vm_flags = vma->vm_flags;
		       void_response->pgoff = vma->vm_pgoff;
		       if (vma->vm_file == NULL) {
			       void_response->path[0] = '\0';
		       } else {
			       plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			       strcpy(void_response->path, plpath);
		       }
	       } else
		       void_response->vma_present = 0;

       //if (wake) {
       //	wake_up(&fetch_write_wait);
       //}

       // Send response
       pcn_kmsg_send_long(from_cpu,
		       (struct pcn_kmsg_long_message*) (void_response),
		       sizeof(data_void_response_t) - sizeof(struct pcn_kmsg_hdr));

       // Clean up incoming messages
       pcn_kmsg_free_msg(request);
       kfree(void_response);
       kfree(work);
       //end= native_read_tsc();
       PSPRINTK("Handle request end\n");
}
#endif
