// Copyright (c) 2013 - 2014, Akshay
// modified by Antonio Barbalace (c) 2014

#ifndef _LINUX_POPCORN_H
#define _LINUX_POPCORN_H

// TODO rlist_head should be renamed,
// TODO furthermore a R/W lock must be used to access the list
extern struct list_head rlist_head;

#define POPCORN_CPUMASK_SIZE 64
#define POPCORN_CPUMASK_BITS (POPCORN_CPUMASK_SIZE * BITS_PER_BYTE)

#if (POPCORN_CPUMASK_BITS < NR_CPUS)
  #error POPCORN_CPUMASK_BITS can not be smaller then NR_CPUS
#endif

struct _remote_cpu_info_data
{
// TODO the following must be added for the messaging layer
	unsigned int endpoint;

// TODO it must support different cpu type in an heterogeneous setting
        unsigned int _processor;
        char _vendor_id[16];
        int _cpu_family;
        unsigned int _model;
        char _model_name[64];
        int _stepping;
        unsigned long _microcode;
        unsigned _cpu_freq;
        int _cache_size;
        char _fpu[3];
        char _fpu_exception[3];
        int _cpuid_level;
	char _wp[3];
	char _flags[512];
        unsigned long _nbogomips;
	int _TLB_size;
	unsigned int _clflush_size;
	int _cache_alignment;
	unsigned int _bits_physical;
	unsigned int _bits_virtual;
	char _power_management[64];
#if 1
	int cpumask_offset;
	int cpumask_size;
	unsigned long cpumask[POPCORN_CPUMASK_SIZE];
#else
	struct cpumask _cpumask; 
#endif
};
typedef struct _remote_cpu_info_data _remote_cpu_info_data_t;

struct _remote_cpu_info_list
{
         _remote_cpu_info_data_t _data;
         struct list_head cpu_list_member;
};
typedef struct _remote_cpu_info_list _remote_cpu_info_list_t;

#endif
