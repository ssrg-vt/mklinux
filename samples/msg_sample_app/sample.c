#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>

#include "kosif.h"
#include "dis_types.h"
#include "sci_types.h"
#include "sci_errno.h"
#include "probetypes.h"
#include "genif_query.h"
#include "id.h"

#include "genif.h"

#define NO_FLAGS 	0
#define SEG_SIZE 	20000
#define TARGET_NODE 	8

int poll_flag = 0;
int int_flag = 0;

int local_cbfunc (void *arg, sci_l_segment_handle_t local_segment_handle,
			unsigned32 reason, unsigned32 source_node, 
                       unsigned32 local_adapter_number)
{
	printk(" In %s: reason = %d\n", __func__, reason);
	return 0;
}

int callback_func(void *arg,
                        sci_r_segment_handle_t remote_segment_handle,
                        unsigned32 reason,
                        unsigned32 status)
{
	if(status == 0)
		poll_flag = 1;

	printk("Reason = %d, status = %d\n", reason, status);
	return 0;
}

signed32 interrupt_cb (unsigned32 local_adapter_number,
                       void *arg, unsigned32 interrupt_number)
{
	printk("Remote interrupt triggered\n");
	int_flag = 1;
	return 0;
}

int __init initialize()
{
	int status = 0, offset = 0;
	unsigned module_id = 0, local_segid = 0, remote_segid = 0, local_adapter_number = 0;
	sci_binding_t binding = NULL;
	sci_l_segment_handle_t local_segment_handle = NULL;
	sci_r_segment_handle_t remote_segment_handle = NULL;
	vkaddr_t *v_addr = NULL, *remote_vaddr = NULL;
	int target_node = 0, value = 0, local_intr_no = 0, remote_intr_no = 0;
	sci_map_handle_t map_handle = NULL;
	probe_status_t report;

	sci_l_interrupt_handle_t local_interrupt_handle = NULL;
	sci_r_interrupt_handle_t remote_interrupt_handle = NULL;

	
	/* Initilaize the adapter */
	status = sci_initialize (module_id);
	if(status == 0)
	{
		printk(" Error in sci_initialize: %d\n", status);
	}

	status = sci_bind (&binding);
	if(status != 0)
	{
		printk(" Error in sci_bind: %d\n", status);
	}

	/* Query node ID */
	status = sci_query_adapter_number(local_adapter_number, Q_ADAPTER_NODE_ID,
					NO_FLAGS, &value);
	if(status != 0)
        {
                printk(" Error in sci_create_segment: %d\n", status);
        }
	else
		printk(" adapter number = %d\n", value);


	target_node = TARGET_NODE;

	local_segid = (value << 8) + target_node;
	remote_segid = (target_node << 8) + value;

	printk(" Segid: Local - %d, remote - %d\n", local_segid, remote_segid);

	/* Create a local segment with above ID */

	status = sci_create_segment( binding, module_id, local_segid,
                   NO_FLAGS, SEG_SIZE, local_cbfunc, NULL, &local_segment_handle);
	if(status != 0)
	{
		printk(" Error in sci_create_segment: %d\n", status);
	}

	status = sci_export_segment(local_segment_handle, 
					local_adapter_number,
                   			NO_FLAGS);
	if(status != 0)
	{
		printk(" Error in sci_export_segment: %d\n", status);
	}

	v_addr = (vkaddr_t *) sci_local_kernel_virtual_address (local_segment_handle);
	if(v_addr != NULL)
	{
		printk(" local segment kernel virtual address is: %x\n", v_addr);
	}

	status = sci_set_local_segment_available (local_segment_handle,
                                 	local_adapter_number);
	if(status != 0)
	{
		printk(" Error in sci_set_local_segment_available: %d\n", status);
	}

	status = sci_probe_node(module_id,NO_FLAGS, target_node,
               local_adapter_number, &report);
        if(status != 0)
        {
                printk(" Error in sci_set_local_segment_available: %d\n", status);
        }
	else
		printk("probe status = %d\n", report);


	/* Create and initialize iterrupt */
	status = sci_allocate_interrupt_flag(binding, local_adapter_number,
                                  0, NO_FLAGS, interrupt_cb ,
                                  NULL, &local_interrupt_handle);
        if(status == 0)
        {
                printk(" Local interrupt cannot be created %d\n", status);
        }

	local_intr_no = sci_interrupt_number(local_interrupt_handle);
	printk("Local interrupt number = %d\n", local_intr_no);

 
	status = sci_is_local_segment_available(local_segment_handle,
						local_adapter_number);
	if(status == 0)
	{
		printk(" Local segment not available to connect to\n");
	}

        printk("writing to local memory %d\n", *v_addr);

	do{
		status = sci_connect_segment(binding, target_node, local_adapter_number,
                    module_id, remote_segid, NO_FLAGS, callback_func , NULL,
                    &remote_segment_handle);
		if(status != 0)
		{
			msleep(1000);
			printk(" Error in sci_connect_segment: %d\n", status);
		}
		msleep(1000);
	}
	while(poll_flag == 0);

	while(!poll_flag)
	{
		msleep(100);
	}

	status = sci_map_segment(remote_segment_handle, NO_FLAGS,
                offset, SEG_SIZE, &map_handle);
	if(status != 0)
	{
		printk(" Error in sci_map_segment: %d\n", status);
	}

 	remote_vaddr = sci_kernel_virtual_address_of_mapping(map_handle);
	if(remote_vaddr != NULL)
	{
		printk(" Remote virtual address: %x\n", remote_vaddr);
	}

	printk("Writing to remote address %d\n", remote_vaddr);
	//memcpy(remote_vaddr, v_addr, SEG_SIZE);
	*remote_vaddr = local_intr_no;

	printk("After writing to remote\n");
	printk("Remote memory value = %d\n", *remote_vaddr);

	while(*v_addr == 0)
	{
		msleep(100);
	}

	remote_intr_no = *v_addr;

	status = sci_connect_interrupt_flag(binding, target_node,
                                        local_adapter_number, remote_intr_no,
                                        NO_FLAGS, &remote_interrupt_handle);
    if(status != 0)
    {
           	printk("Unable to connect to remote interrupt: %d\n", status);
	goto cleanup;
    }

	//*v_addr = 4096;

	/*Waiting for dma transfer completion interrupt */ 
	while(int_flag == 0)
	{
		msleep(1000);
		printk("waiting on interrupt\n");
	}

    /* trigger the interrupt */
    status = sci_trigger_interrupt_flag(remote_interrupt_handle,
                                    NO_FLAGS);
    if(status != 0)
    {
            printk(" Error in sci_trigger_interrupt_flag: %d\n", status);
    }

    printk(" After write: Memory values %d %d\n", *v_addr, *remote_vaddr);

    /* Remove interrupt */
    status = sci_disconnect_interrupt_flag(&remote_interrupt_handle, NO_FLAGS);
    if(status != 0)
    {
            printk(" Error in sci_disconnect_interrupt_flag: %d\n", status);
    }

cleanup:
	/* Deintialize path */
	status = sci_unmap_segment(&map_handle,NO_FLAGS);
	if(status != 0)
	{
		printk(" Error in sci_unmap_segment: %d\n", status);
	}

	status = sci_disconnect_segment(&remote_segment_handle, NO_FLAGS);
	if(status != 0)
	{
		printk(" Error in sci_disconnect_segment: %d\n", status);
	}

	status = sci_remove_interrupt_flag(&local_interrupt_handle, NO_FLAGS);
        if(status != 0)
        {
                printk(" Error in sci_remove_interrupt_flag: %d\n", status);
        }

	status = sci_set_local_segment_unavailable (local_segment_handle,
						 local_adapter_number);
	if(status != 0)
	{
		printk(" Error in sci_set_local_segment_unavailable: %d\n", status);
	}

	status = sci_unexport_segment(local_segment_handle, local_adapter_number,
                     			NO_FLAGS);
	if(status != 0)
	{
		printk(" Error in sci_unexport_segment: %d\n", status);
	}

	status = sci_remove_segment(&local_segment_handle,NO_FLAGS);
	if(status != 0)
	{
		printk(" Error in sci_remove_segment: %d\n", status);
	}

	status = sci_unbind (&binding);
	if(status != 0)
	{
		printk(" Error in sci_unbind: %d\n", status);
	}

	sci_terminate(module_id);

	return 0;
}

void __exit uninitialize()
{
	return;
}


module_init(initialize);
module_exit(uninitialize);
MODULE_LICENSE("GPL");

