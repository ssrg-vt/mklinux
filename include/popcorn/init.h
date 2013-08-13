#ifndef __POPCORN_INIT_H
#define __POPCORN_INIT_H
/*
 * Boot parameters for allocating Kernel ID
 *
 * (C) Akshay Ravichandran <akshay87@vt.edu> 2012
 */


extern unsigned int Kernel_Id;
extern unsigned long *token_bucket;
extern unsigned long long bucket_phys_addr;

extern void popcorn_init(void);

extern int _init_RemoteCPUMask(void);

//extern struct _remote_cpu_info_list rlist;

extern struct list_head rlist_head;

#endif /* __POPCORN_INIT_H */



/*
{

unsigned long bucket_phys_addr=0;
struct boot_params *boot_params_va;
int i=0;

ssize_t bucket_size =sizeof(long)*max_nodes;


printk("POP_INIT:Called popcorn_init boot id--max_nodes :%d! %d\n",mklinux_boot,max_nodes);

if(!mklinux_boot)
{
token_bucket= kmalloc(bucket_size,
					  GFP_KERNEL);
if (!token_bucket) {
			printk("Failed to kmalloc token_bucket !\n");
			return -1;
		}
bucket_phys_addr = virt_to_phys(token_bucket);
memset(token_bucket, 0x0, bucket_size);
token_bucket[0]=1;
printk("POP_INIT:Setting boot_params...\n");
		boot_params_va = (struct boot_params *)
			(0xffffffff80000000 + orig_boot_params);

printk("POP_INIT:Boot params virtual address: 0x%p\n", boot_params_va);
Kernel_Id=1;

boot_params_va->kernel_id_param.shm_kernel_id_addr=bucket_phys_addr;
printk("POP_INIT:token_bucket Initial values; \n");
for(i=0;i<max_nodes;i++)
	{
	printk("%d\t",token_bucket[i]);
	}
}

else
{

	printk("POP_INIT:kernel bucket_phys_addr: 0x%lx\n",
			  (unsigned long) boot_params.kernel_id_param.shm_kernel_id_addr);
	bucket_phys_addr=boot_params.kernel_id_param.shm_kernel_id_addr;
	token_bucket=ioremap_cache(bucket_phys_addr,
			bucket_size);
	printk("POP_INIT:token_bucket addr: 0x%p\n", token_bucket);
	for(i=0;i<max_nodes;i++)
	{
		if(token_bucket[i]==0)
		{   token_bucket[i]=1;
			Kernel_Id=i+1;break;
		}
	}

	printk("POP_INIT:token_bucket  values; \n");
	for(i=0;i<max_nodes;i++)
		{
		printk("%d\t",token_bucket[i]);
		}


}
printk("POP_INIT:Kernel id is %d\n",Kernel_Id);
printk("POP_INIT:Virt add : 0x%p --- shm kernel id address: 0x%lx\n",token_bucket,bucket_phys_addr);

}
*/
