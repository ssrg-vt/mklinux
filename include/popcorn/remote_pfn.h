/*
 * remote_pfn.h
 *
 *  Created on: Sep 28, 2013
 *      Author: akshay
 */

#ifndef REMOTE_PFN_H_
#define REMOTE_PFN_H_

//list for pfn range of each kernels
struct _pfn_range_list
{
	unsigned long start_pfn_addr;
	unsigned long end_pfn_addr;
	int kernel_number;
	struct list_head pfn_list_member;
};
typedef struct _pfn_range_list _pfn_range_list_t;


#endif /* REMOTE_PFN_H_ */
