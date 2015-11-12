/*
 *
 * Copyright (c) 2012, Applied Micro Circuits Corporation
 * Author: Vinayak Kale <vkale@apm.com>
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
 */
#ifndef __MSLIM_IOF_MAP_H__
#define __MSLIM_IOF_MAP_H__

/* 
 * This API returns 42b IOF AXI address for a given MSLIM DDR Physical address.  
 * Returns 0 on error.
 */
extern unsigned long long mslim_pa_to_iof_axi(unsigned long pa);


/* 
 * This API returns 32b MSLIM DDR Physical address for a given 42b IOF AXI address.
 * Returns 0 on error.
 */
extern unsigned long mslim_iof_axi_to_pa(unsigned long long iof_axi_addr);

#endif /* __MSLIM_IOF_MAP_H__ */
