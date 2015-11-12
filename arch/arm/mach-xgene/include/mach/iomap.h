/*
 * linux/arch/arm/mach-xgene/include/mach/iomap.h
 *
 * Copyright (c) 2012, Applied Micro Circuits Corporation
 * Author: Tanmay Inamdar <tinamdar@apm.com>
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
#ifndef __MACH_IOMAP_H
#define __MACH_IOMAP_H

#define IO_PHYS_TO_VIRT(x)   (((x) & 0xfffff) | (((x) & 0x03000000) >> 4) | 0xf8000000)

#endif
