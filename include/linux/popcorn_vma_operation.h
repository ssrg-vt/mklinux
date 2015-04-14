/*
 * popcorn_vma_operation.h
 *
 * Author: Marina Sadini, SSRG Virginia Tech
 */


#include <linux/popcorn_migration.h>
#include <linux/pcn_kmsg.h>

#ifndef POPCORN_VMA_OPERATION_H_
#define POPCORN_VMA_OPERATION_H_

#define POPCORN_VMA_VERBOSE 0

#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

#define VMA_OP_NOP 0
#define VMA_OP_UNMAP 1
#define VMA_OP_PROTECT 2
#define VMA_OP_REMAP 3
#define VMA_OP_MAP 4
#define VMA_OP_BRK 5

#define VMA_OP_SAVE -70
#define VMA_OP_NOT_SAVE -71

#if POPCORN_VMA_VERBOSE
#define VMAPRINTK(...) printk(__VA_ARGS__)
#else
#define VMAPRINTK(...) ;
#endif

#define VMA_LOCK_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		int from_cpu;\
		int vma_operation_index;

struct _vma_lock {
	VMA_LOCK_FIELDS
};

struct vma_lock {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_LOCK_FIELDS
		};
#define VMA_LOCK_PAD ((sizeof(struct _vma_lock)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_lock)):(PCN_KMSG_PAYLOAD_SIZE))

		char pad[VMA_LOCK_PAD];
	};

};

struct vma_lock_work{
	struct work_struct work;
	struct vma_lock* lock;
	struct migration_memory* memory;
};

#define VMA_OPERATION_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		int operation;\
		unsigned long addr;\
		unsigned long new_addr;\
		size_t len;\
		unsigned long new_len;\
		unsigned long prot;\
		unsigned long flags; \
		int from_cpu;\
		int vma_operation_index;\
		int pgoff;\
		char path[512];

struct _vma_operation {
	VMA_OPERATION_FIELDS
};

struct  vma_operation{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_OPERATION_FIELDS
		};

#define VMA_OPERATION_PAD ((sizeof(struct _vma_operation)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_operation)):(PCN_KMSG_PAYLOAD_SIZE))

		char pad[VMA_OPERATION_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct vma_op_work{
	struct work_struct work;
	struct vma_operation* operation;
	struct migration_memory* memory;
	int fake;
};

struct vma_op_answers {
	struct vma_op_answers* next;
	struct vma_op_answers* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	int responses;
	int expected_responses;
	int vma_operation_index;
	unsigned long address;
	struct task_struct *waiting;
	raw_spinlock_t lock;

};


#define VMA_ACK_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		int vma_operation_index;\
		unsigned long addr;

struct _vma_ack {
	VMA_ACK_FIELDS
};

struct vma_ack{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_ACK_FIELDS
		};

#define VMA_ACK_PAD ((sizeof(struct _vma_ack)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_ack)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[VMA_ACK_PAD];

	}__attribute__((packed));

} __attribute__((packed));

long popcorn_do_unmap_start(struct mm_struct *mm, unsigned long start, size_t len);
long popcorn_do_unmap_end(struct mm_struct *mm, unsigned long start, size_t len,int start_ret);

long popcorn_mprotect_start(unsigned long start, size_t len,unsigned long prot);
long popcorn_mprotect_end(unsigned long start, size_t len,unsigned long prot,int start_ret);

long popcorn_do_mmap_pgoff_start(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff);
long popcorn_do_mmap_pgoff_end(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot,
		unsigned long flags, unsigned long pgoff, unsigned long start_ret);

long popcorn_do_brk_start(unsigned long addr, unsigned long len);
long popcorn_do_brk_end(unsigned long addr, unsigned long len, unsigned long start_ret);

long popcorn_do_mremap_start(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr);
long popcorn_do_mremap_end(unsigned long addr,
		unsigned long old_len, unsigned long new_len,
		unsigned long flags, unsigned long new_addr,unsigned long start_ret);

#endif /* POPCORN_VMA_OPERATION_H_ */
