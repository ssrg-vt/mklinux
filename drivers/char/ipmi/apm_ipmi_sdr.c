/*
 * APM IPMI Virtual BMC Driver
 *
 * Copyright (c) 2013, Applied Micro Circuits Corporation
 * Author: Van Duc Uy <uvan@apm.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * @file apm_ipmi_sdr.c
 *
 * This file do intialize SDR Repository.
 */


#include <linux/kernel.h> /* For printk. */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include<linux/kernel.h>
#include<linux/kthread.h>
#include<linux/sched.h>
#include<linux/delay.h>

#include "apm_ipmi_sdr.h"
#include "apm_sensor_adm1032.h"
#include "apm_sensor_fan.h"
#include "apm_sensor_storm.h"
#include "apm_sensor_dimm.h"
#include "apm_sensor_vrm.h"


SDR_ENTRY sdr_repository[MAX_SDR_RECORD];
SENSOR_DATA *sensor[MAX_SDR_RECORD];
int current_sensor_count;


#ifdef SENSOR_SCANNING_THREAD_ENABLE
struct task_struct *periodic_task;
static bool g_sensor_scan_request = false;
#endif

int
add_sdr_entry(FULL_SENSOR_RECORD *sdr, SENSOR_DATA *sensor_data)
{
	if( current_sensor_count + 1 > MAX_SDR_RECORD )
		return -1 ;
	
	sdr_repository[current_sensor_count].record_ptr = ( unsigned char * )sdr;
	sdr_repository[current_sensor_count].record_id = current_sensor_count;
	((FULL_SENSOR_RECORD *)(sdr_repository[current_sensor_count].record_ptr))->key.sensor_number = current_sensor_count;
	((FULL_SENSOR_RECORD *)(sdr_repository[current_sensor_count].record_ptr))->header.record_id[0] = current_sensor_count;
	sensor_data->sensor_id = current_sensor_count;
	sensor[current_sensor_count] = sensor_data;
	current_sensor_count++;
	
	return 0;	
}

#ifdef SENSOR_SCANNING_THREAD_ENABLE
int 
is_sensor_scan_requested()
{
	return g_sensor_scan_request;
}

void 
set_sensor_scan_request(int param)
{
	g_sensor_scan_request = param;
}

int 
sensor_scanning_thread(void *param)
{
	int i;
	while(true) {
		if( is_sensor_scan_requested()) {
			/*printk("Sensor scanning...\n");*/
			for (i=0; i< current_sensor_count; i++)	{
				if (sensor[i]->sensor_scanning != NULL) {
					sensor[i]->sensor_scanning((void*)sensor[i]);	
				}
			}
			set_sensor_scan_request(false);
		}
		
		msleep(100);
	}
	return 0;
}
#endif

void
ipmi_sdr_repository_initialize()
{
	current_sensor_count = 0;

	storm_sdr_init();
	dimm_sdr_init();
	fan_sdr_init();
	adm1032_sdr_init();
	
	vrm_sdr_init();

	//adm1032_get_device_id();
	//
	//
#ifdef SENSOR_SCANNING_THREAD_ENABLE
	periodic_task = kthread_create(&sensor_scanning_thread,NULL,"Sensor Scanning");
	periodic_task = kthread_run(&sensor_scanning_thread,NULL,"Sensor Scanning");
	printk("Periodic Sensor Scanning Started\n");
#endif	
#if 0 
	{
	int i;
	for (i=0;i<current_sensor_count;i++) {

		printk("SDR[%d]\n",i);
		printk("\trecord_type:0x%x\n",
			((FULL_SENSOR_RECORD *)(sdr_repository[i].record_ptr))->header.record_type);
		printk("\trecord_size:%d\n",
			((FULL_SENSOR_RECORD *)(sdr_repository[i].record_ptr))->header.record_len);

		printk("\tsensor_type:%d\n",
			((FULL_SENSOR_RECORD*)sdr_repository[i].record_ptr)->body.sensor_type);
		printk("\tsensor_number:%d\n",
			((FULL_SENSOR_RECORD*)sdr_repository[i].record_ptr)->key.sensor_number);
		printk("\tSensor_id:0x%x\n",sensor[i]->sensor_id);
	}
	}
#endif

}

void
ipmi_sdr_repository_cleanup(void)
{
#ifdef SENSOR_SCANNING_THREAD_ENABLE	
	kthread_stop(periodic_task);
#endif	
}
