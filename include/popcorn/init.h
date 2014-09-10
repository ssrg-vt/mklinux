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
extern unsigned long kernel_start_addr;

extern void popcorn_init(void);

extern int _init_RemoteCPUMask(void);


extern struct list_head rlist_head;

extern struct list_head pfn_list_head;

extern int _init_RemotePFN(void);


#endif /* __POPCORN_INIT_H */

