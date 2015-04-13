/*
 * popcorn_user_dsm.h
 * Author: Marina Sadini, SSRG Virginia Tech
 */


#include <linux/types.h>
#include <linux/pcn_kmsg.h>
#include <linux/sched.h>
#include <linux/popcorn_macro.h>

#ifndef POPCORN_USER_DSM_H_
#define POPCORN_USER_DSM_H_

#define REPLICATION_STATUS_VALID 3
#define REPLICATION_STATUS_WRITTEN 1
#define REPLICATION_STATUS_INVALID 2
#define REPLICATION_STATUS_NOT_REPLICATED 0

#define CHECKSUM 0
#define POPCORN_DSM_VERBOSE 0

#if POPCORN_DSM_VERBOSE
#define DSMPRINTK(...) printk(__VA_ARGS__)
#else
#define DSMPRINTK(...) ;
#endif

#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

#define MAPPING_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		unsigned long address;\
		int is_write; \
		int is_fetch;\
		int vma_operation_index;\
		long last_write;

struct _mapping_fields {
	MAPPING_FIELDS
};

struct page_fault_mapping_request{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MAPPING_FIELDS
		};

#define MAPPING_FIELDS_PAD ((sizeof(struct _mapping_fields)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _mapping_fields)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MAPPING_FIELDS_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct page_fault_mapping {
	struct page_fault_mapping* next;
	struct page_fault_mapping* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	unsigned long address;
	int vma_present;
	unsigned long vaddr_start;
	unsigned long vaddr_size;
	unsigned long pgoff;
	char path[512];
	pgprot_t prot;
	unsigned long vm_flags;
	int is_write;
	int is_fetch;
	int owner;
	int address_present;
	long last_write;
	int owners [MAX_KERNEL_IDS];
	struct mapping_response* data;
	int arrived_response;
	struct task_struct* waiting;
	int futex_owner;

};

struct ack_answers {
	struct ack_answers* next;
	struct ack_answers* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	unsigned long address;
	int response_arrived;
	struct task_struct * waiting;

} ;

#define INVALID_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		unsigned long address; \
		long last_write;\
		int vma_operation_index;

struct _invalid_fields {
	INVALID_FIELDS
};

struct invalid_request{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			INVALID_FIELDS
		};

#define INVALID_FIELDS_PAD ((sizeof(struct _invalid_fields)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _invalid_fields)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[INVALID_FIELDS_PAD];
	}__attribute__((packed));

} __attribute__((packed)) ;

struct page_fault_mapping_request_work{
	struct delayed_work work;
	struct page_fault_mapping_request* request;
	unsigned long address;
	int tgroup_home_cpu;
	int tgroup_home_id;
};

struct invalid_request_work{
	struct delayed_work work;
	struct invalid_request* request;
};

#define ACK_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		unsigned long address; \
		int ack;\
		int writing; \
		unsigned long long time_stamp;

struct _ack {
	ACK_FIELDS
};

struct ack_invalid{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			ACK_FIELDS
		};

#define ACK_PAD ((sizeof(struct _ack)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _ack)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[ACK_PAD];
	}__attribute__((packed));

}__attribute__((packed));

/*Important: char data must be the last field*/
#define MAPPING_RESPONSE_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;  \
		unsigned long address; \
		__wsum checksum; \
		long last_write;\
		int owner;\
		int vma_present; \
		unsigned long vaddr_start;\
		unsigned long vaddr_size;\
		pgprot_t prot; \
		unsigned long vm_flags; \
		unsigned long pgoff;\
		char path[512];\
		unsigned int data_size;\
		int diff;\
		int futex_owner; \
		char data; \

struct _mapping_response {
	MAPPING_RESPONSE_FIELDS
};

struct mapping_response{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MAPPING_RESPONSE_FIELDS
		};
#define MAPPING_RESPONSE_FIELDS_PAD ((sizeof(struct _mapping_response)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _mapping_response)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MAPPING_RESPONSE_FIELDS_PAD];
	}__attribute__((packed));

}__attribute__((packed)) ;

#define MAPPING_RESPONSE_VOID_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;  \
		unsigned long address; \
		int vma_present; \
		unsigned long vaddr_start;\
		unsigned long vaddr_size;\
		unsigned long vm_flags; \
		unsigned long pgoff;\
		char path[512];\
		pgprot_t prot; \
		int fetching_read; \
		int fetching_write;\
		int owner;\
		__wsum checksum; \
		int futex_owner; \

struct _mapping_response_void {
	MAPPING_RESPONSE_VOID_FIELDS
};

struct mapping_response_void{
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MAPPING_RESPONSE_VOID_FIELDS
		};
#define MAPPING_RESPONSE_VOID_FIELDS_PAD ((sizeof(struct _mapping_response_void)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _mapping_response_void)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MAPPING_RESPONSE_VOID_FIELDS_PAD];
	}__attribute__((packed));

}__attribute__((packed));

int popcorn_update_page(struct task_struct * tsk,struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address, unsigned long page_fault_flags);
int popcorn_try_handle_mm_fault(struct task_struct *tsk,struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, unsigned long page_fault_flags,unsigned long error_code);
void popcorn_clean_page(struct page* page);
void popcorn_change_not_present_pte_for_mprotect(pte_t *pte, pte_t oldpte, pgprot_t newprot, struct mm_struct *mm, unsigned long addr);
int popcorn_change_present_pte_for_mprotect(pte_t oldpte, pgprot_t newprot);

#endif /* POPCORN_USER_DSM_H_ */
