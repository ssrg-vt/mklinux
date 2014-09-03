/**
 * Migration service + replication of virtual address space
 *
 * Marina
 */

#include <linux/mcomm.h> // IPC
#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/smp.h>
#include <linux/sched.h>
#include <linux/threads.h> // NR_CPUS
#include <linux/kmod.h>
#include <linux/path.h>
#include <linux/mount.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/process_server.h>
#include <linux/mm.h>
#include <linux/io.h> // ioremap
#include <linux/mman.h> // MAP_ANONYMOUS
#include <linux/pcn_kmsg.h> // Messaging
#include <linux/highmem.h> //Replication
#include <linux/memcontrol.h>
#include <linux/pagemap.h>
#include <linux/mmu_notifier.h>
#include <linux/swap.h>

#include <asm/traps.h>			/* dotraplinkage, ...		*/
#include <asm/pgalloc.h>		/* pgd_*(), ...			*/
#include <asm/kmemcheck.h>		/* kmemcheck_*(), ...		*/
#include <asm/fixmap.h>			/* VSYSCALL_START		*/

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h> // USER_DS
#include <asm/prctl.h> // prctl
#include <asm/proto.h> // do_arch_prctl
#include <asm/msr.h> // wrmsr_safe
#include <asm/page.h>//Replication
#include <linux/rmap.h>
#include <linux/memcontrol.h>
#include <asm/atomic.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <net/checksum.h>
#include <linux/fsnotify.h>
#include <linux/unistd.h>
#include <asm/mmu_context.h>
#include <linux/tsacct_kern.h>
#include <asm/uaccess.h>
#include <linux/popcorn_cpuinfo.h>
#include <asm/i387.h>
#include <linux/cpu_namespace.h>
#include "WKdm.h"

/*akshay*/
#include <linux/futex.h>
#define  NSIG 32

#include<linux/signal.h>
#include <linux/fcntl.h>
#include "futex_remote.h"
/*akshay*/
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

/**
 * Use the preprocessor to turn off printk.
 */
#define PROCESS_SERVER_VERBOSE 0
#define PROCESS_SERVER_VMA_VERBOSE 0
#define PROCESS_SERVER_NEW_THREAD_VERBOSE 0
#define PROCESS_SERVER_MINIMAL_PGF_VERBOSE 0

#define READ_PAGE 0
#define PAGE_ADDR 0

#define CHECKSUM 0
#define STATISTICS 0
#define TIMING 0

#if PROCESS_SERVER_VERBOSE
#define PSPRINTK(...) printk(__VA_ARGS__)
#undef STATISTICS
#define STATISTICS 1
#else
#define PSPRINTK(...) ;
#endif

#if PROCESS_SERVER_VMA_VERBOSE
#define PSVMAPRINTK(...) printk(__VA_ARGS__)
#else
#define PSVMAPRINTK(...) ;
#endif

#if PROCESS_SERVER_NEW_THREAD_VERBOSE
#define PSNEWTHREADPRINTK(...) printk(__VA_ARGS__)
#else
#define PSNEWTHREADPRINTK(...) ;
#endif

#if PROCESS_SERVER_MINIMAL_PGF_VERBOSE
#define PSMINPRINTK(...) printk(__VA_ARGS__)
#else
#define PSMINPRINTK(...) ;
#endif


#if PARTIAL_VMA_MANAGEMENT
#undef NOT_REPLICATED_VMA_MANAGEMENT
#define NOT_REPLICATED_VMA_MANAGEMENT 0
#endif

#define NR_THREAD_PULL 1

static void dump_regs(struct pt_regs* regs);

typedef struct _fetching_struct {
	struct _fetching_struct* next;
	struct _fetching_struct* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	unsigned long vaddr;
	int status;
	int owners[MAX_KERNEL_IDS];
	int owner;
	long last_invalid;

} fetching_t;



#if FOR_2_KERNELS
typedef struct ack_answers_for_2_kernels {
	struct ack_answers_for_2_kernels* next;
	struct ack_answers_for_2_kernels* prev;

	//data_header_t header;
	int tgroup_home_cpu;
	int tgroup_home_id;
	unsigned long address;
	int response_arrived;
	struct task_struct * waiting;

} ack_answers_for_2_kernels_t;
#else
typedef struct ack_answers {
	struct ack_answers* next;
	struct ack_answers* prev;

	//data_header_t header;
	int tgroup_home_cpu;
	int tgroup_home_id;
	unsigned long address;
	int responses;
	int expected_responses;
	int nack;
	int concurrent;
	int writers[MAX_KERNEL_IDS];
	int owner;
	unsigned long long time_stamp;
	raw_spinlock_t lock;
	struct task_struct * waiting;

} ack_answers_t;
#endif

#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

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

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_OPERATION_FIELDS
		};

#define VMA_OPERATION_PAD ((sizeof(struct _vma_operation)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_operation)):(PCN_KMSG_PAYLOAD_SIZE))

		char pad[VMA_OPERATION_PAD];
	}__attribute__((packed));

}__attribute__((packed)) vma_operation_t;

typedef struct _data_header {
	struct _data_header* next;
	struct _data_header* prev;
} data_header_t;

typedef struct {
	data_header_t head;
	struct task_struct * thread;
} shadow_thread_t;

struct _memory_struct;

typedef struct {
	data_header_t head;
	struct task_struct * main;
	shadow_thread_t* threads;
	raw_spinlock_t spinlock;
	struct _memory_struct* memory;
} thread_pull_t;

typedef struct _memory_struct {
	struct _memory_struct* next;
	struct _memory_struct* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	struct mm_struct* mm;
	int alive;
	struct task_struct * main;

	int operation;
	unsigned long addr;
	unsigned long new_addr;
	size_t len;
	unsigned long new_len;
	unsigned long prot;
	unsigned long pgoff;
	unsigned long flags;
	char path[512];

	struct task_struct* waiting_for_main;
	struct task_struct* waiting_for_op;
	int arrived_op;
	int my_lock;
	int kernel_set[MAX_KERNEL_IDS];
	int exp_answ;
	int answers;
	int setting_up;
	raw_spinlock_t lock_for_answer;
	struct rw_semaphore kernel_set_sem;
	vma_operation_t* message_push_operation;
	thread_pull_t* thread_pull;
	atomic_t pending_migration;

} memory_t;

typedef struct count_answers {
	struct count_answers* next;
	struct count_answers* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	int responses;
	int expected_responses;
	int count;
	raw_spinlock_t lock;
	struct task_struct * waiting;
} count_answers_t;

#define ACK_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		unsigned long address; \
		int ack;\
		int writing; \
		unsigned long long time_stamp;

struct _ack {
	ACK_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			ACK_FIELDS
		};

#define ACK_PAD ((sizeof(struct _ack)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _ack)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[ACK_PAD];
	}__attribute__((packed));

}__attribute__((packed)) ack_t;

//int my_set[NR_CPUS]; saif changed
#define NEW_KERNEL_ANSWER_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		int my_set[MAX_KERNEL_IDS];\
		int vma_operation_index;

struct _new_kernel_answer {
	NEW_KERNEL_ANSWER_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			NEW_KERNEL_ANSWER_FIELDS
		};

#define NEW_KERNEL_ANSWER_PAD ((sizeof(struct _new_kernel_answer)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _new_kernel_answer)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[NEW_KERNEL_ANSWER_PAD];
	}__attribute__((packed));

}__attribute__((packed)) new_kernel_answer_t;


#define BACK_MIGRATION_FIELDS unsigned int personality;\
		unsigned long def_flags;\
		pid_t placeholder_pid;\
		pid_t placeholder_tgid;\
		int back;\
		int prev_pid;\
		struct pt_regs regs;\
		unsigned long thread_usersp;\
		unsigned long old_rsp;\
		unsigned short thread_es;\
		unsigned short thread_ds;\
		unsigned long thread_fs;\
		unsigned short thread_fsindex;\
		unsigned long thread_gs;\
		unsigned short thread_gsindex;\
		int tgroup_home_cpu;\
		int tgroup_home_id;\
		int origin_pid;\
		sigset_t remote_blocked, remote_real_blocked;\
		sigset_t remote_saved_sigmask;\
		struct sigpending remote_pending;\
		unsigned long sas_ss_sp;\
		size_t sas_ss_size;\
		struct k_sigaction action[_NSIG];
/*#ifdef MIGRATE_FPU		unsigned int  task_flags;\
  	    	unsigned char task_fpu_counter;\
		unsigned char thread_has_fpu;\
		union thread_xstate fpu_state;\
#endif	*/


struct _back_migration_request {
		BACK_MIGRATION_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			BACK_MIGRATION_FIELDS
		};
#define	BACK_MIGRATION_STRUCT_PAD ((sizeof(struct _back_migration_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _back_migration_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[BACK_MIGRATION_STRUCT_PAD];
	}__attribute__((packed));

}__attribute__((packed)) back_migration_request_t;


#define CLONE_FIELDS unsigned long stack_start; \
		unsigned long env_start;\
		unsigned long env_end;\
		unsigned long arg_start;\
		unsigned long arg_end;\
		unsigned long start_brk;\
		unsigned long brk;\
		unsigned long start_code ;\
		unsigned long end_code;\
		unsigned long start_data;\
		unsigned long end_data ;\
		unsigned int personality;\
		unsigned long def_flags;\
		char exe_path[512];\
		pid_t placeholder_pid;\
		pid_t placeholder_tgid;\
		int back;\
		int prev_pid;\
		struct pt_regs regs;\
		unsigned long thread_usersp;\
		unsigned long old_rsp;\
		unsigned short thread_es;\
		unsigned short thread_ds;\
		unsigned long thread_fs;\
		unsigned short thread_fsindex;\
		unsigned long thread_gs;\
		unsigned short thread_gsindex;\
		int tgroup_home_cpu;\
		int tgroup_home_id;\
        int origin_pid;\
        sigset_t remote_blocked, remote_real_blocked;\
        sigset_t remote_saved_sigmask;\
        struct sigpending remote_pending;\
        unsigned long sas_ss_sp;\
        size_t sas_ss_size;\
        struct k_sigaction action[_NSIG];
/*#ifdef MIGRATE_FPU		unsigned int  task_flags;\
  	    	unsigned char task_fpu_counter;\
		unsigned char thread_has_fpu;\
		union thread_xstate fpu_state;\
#endif	*/	



struct _clone_request {
	CLONE_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			CLONE_FIELDS
		};
#define	CLONE_STRUCT_PAD ((sizeof(struct _clone_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _clone_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[CLONE_STRUCT_PAD];
	}__attribute__((packed));

}__attribute__((packed)) clone_request_t;



/**
 * This message is sent in response to a clone request.
 * Its purpose is to notify the requesting cpu that
 * the specified pid is executing on behalf of the
 * requesting cpu.
 */
#define PROCESS_PAIRING_FIELD int your_pid; \
		int my_pid;

struct _create_process_pairing{
	PROCESS_PAIRING_FIELD
};

typedef struct {

	struct pcn_kmsg_hdr header;

	union{

		struct{
			PROCESS_PAIRING_FIELD
		};

#define PROCESS_PAIRING_PAD ((sizeof(struct _create_process_pairing)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _create_process_pairing)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[PROCESS_PAIRING_PAD];

	}__attribute__((packed)) ;

}__attribute__((packed))create_process_pairing_t;

#define COUNT_REQUEST_FIELD int tgroup_home_cpu; \
		int tgroup_home_id;

struct _remote_thread_count_request{
	COUNT_REQUEST_FIELD
};

typedef struct {
	struct pcn_kmsg_hdr header;

	union{

		struct{
			COUNT_REQUEST_FIELD
		};

#define COUNT_REQUEST_PAD ((sizeof(struct _remote_thread_count_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _remote_thread_count_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[COUNT_REQUEST_PAD];
	}__attribute__((packed));

}__attribute__((packed)) remote_thread_count_request_t;

#define COUNT_RESPONSE_FIELD int tgroup_home_cpu; \
		int tgroup_home_id; \
		int count;

struct _remote_thread_count_response{
	COUNT_RESPONSE_FIELD
};

typedef struct {
	struct pcn_kmsg_hdr header;

	union{

		struct{
			COUNT_RESPONSE_FIELD
		};

#define COUNT_RESPONSE_PAD ((sizeof(struct _remote_thread_count_response)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _remote_thread_count_response)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[COUNT_RESPONSE_PAD];
	}__attribute__((packed));

}__attribute__((packed)) remote_thread_count_response_t;

/**
 * This message informs the remote cpu of delegated
 * process death.  This occurs whether the process
 * is a placeholder or a delegate locally.
 */
#define EXITING_PROCESS_FIELDS  pid_t my_pid; \
		pid_t prev_pid;\
		int is_last_tgroup_member; \
		int group_exit;\
		long code;\
		struct pt_regs regs;\
		unsigned long thread_fs;\
		unsigned long thread_gs;\
		unsigned long thread_usersp;\
		unsigned long old_rsp;\
		unsigned short thread_es;\
		unsigned short thread_ds;\
		unsigned short thread_fsindex;\
		unsigned short thread_gsindex;\
/*#ifdef MIGRATE_FPU		unsigned int  task_flags; \
		unsigned char task_fpu_counter;\
		unsigned char thread_has_fpu;\
		union thread_xstate fpu_state;\
#endif*/

struct _exiting_process {
	EXITING_PROCESS_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			EXITING_PROCESS_FIELDS
		};
#define EXITING_PROCES_PAD ((sizeof(struct _exiting_process)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _exiting_process)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[EXITING_PROCES_PAD];
	}__attribute__((packed));

} __attribute__((packed)) exiting_process_t;

#define EXIT_GROUP_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;

struct _thread_group_exited_notification {
	EXIT_GROUP_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;

	union{

		struct{
			EXIT_GROUP_FIELDS
		};

#define EXITING_GROUP_PAD ((sizeof(struct _thread_group_exited_notification)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _thread_group_exited_notification)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[EXITING_GROUP_PAD];

	}__attribute__((packed));


}__attribute__((packed)) thread_group_exited_notification_t;

typedef struct{
	struct pcn_kmsg_hdr header;
	char pad[PCN_KMSG_PAYLOAD_SIZE];

}__attribute__((packed)) create_thread_pull_t;
/**
 * Inform remote cpu of a vma to process mapping.
 */
typedef struct _vma_transfer {
	struct pcn_kmsg_hdr header;
	int vma_id;
	int clone_request_id;
	unsigned long start;
	unsigned long end;
	pgprot_t prot;
	unsigned long flags;
	unsigned long pgoff;
	char path[256];
}__attribute__((packed)) vma_transfer_t;

#define NEW_KERNEL_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id;

struct _new_kernel {
	NEW_KERNEL_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			NEW_KERNEL_FIELDS
		};

#define NEW_KERNEL_PAD ((sizeof(struct _new_kernel)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _new_kernel)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[NEW_KERNEL_PAD];
	}__attribute__((packed));

}__attribute__((packed)) new_kernel_t;

#if FOR_2_KERNELS

#define MAPPING_FIELDS_FOR_2_KERNELS int tgroup_home_cpu; \
		int tgroup_home_id; \
		unsigned long address;\
		int is_write; \
		int is_fetch;\
		int vma_operation_index;\
		long last_write;

struct _mapping_for_2_kernels {
	MAPPING_FIELDS_FOR_2_KERNELS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MAPPING_FIELDS_FOR_2_KERNELS
		};

#define MAPPING_FIELDS_FOR_2_KERNELS_PAD ((sizeof(struct _mapping_for_2_kernels)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _mapping_for_2_kernels)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MAPPING_FIELDS_FOR_2_KERNELS_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_request_for_2_kernels_t;

#else

#define MAPPING_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		unsigned long address; \
		int read_for_write; \
		unsigned int flags;\
		int vma_operation_index;

struct _mapping {
	MAPPING_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MAPPING_FIELDS
		};

#define MAPPING_PAD ((sizeof(struct _mapping)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _mapping)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MAPPING_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_request_t;

#endif

#if FOR_2_KERNELS

#define INVALID_FIELDS_FOR_2_KERNELS int tgroup_home_cpu;\
		int tgroup_home_id; \
		unsigned long address; \
		long last_write;\
		int vma_operation_index;

struct _invalid_for_2_kernels {
	INVALID_FIELDS_FOR_2_KERNELS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			INVALID_FIELDS_FOR_2_KERNELS
		};

#define INVALID_PAD ((sizeof(struct _invalid_for_2_kernels)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _invalid_for_2_kernels)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[INVALID_PAD];
	}__attribute__((packed));

} __attribute__((packed)) invalid_data_for_2_kernels_t;

#define DATA_RESPONSE_FIELDS_FOR_2_KERNELS int tgroup_home_cpu; \
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

struct _data_response_for_2_kernels {
	DATA_RESPONSE_FIELDS_FOR_2_KERNELS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			DATA_RESPONSE_FIELDS_FOR_2_KERNELS
		};
#define DATA_RESPONSE_FIELDS_FOR_2_KERNELS_PAD ((sizeof(struct _data_response_for_2_kernels)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _data_response_for_2_kernels)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[DATA_RESPONSE_FIELDS_FOR_2_KERNELS_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_response_for_2_kernels_t;

#define DATA_VOID_RESPONSE_FIELDS_FOR_2_KERNELS int tgroup_home_cpu; \
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

struct _data_void_response_for_2_kernels {
	DATA_VOID_RESPONSE_FIELDS_FOR_2_KERNELS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			DATA_VOID_RESPONSE_FIELDS_FOR_2_KERNELS
		};
#define DATA_VOID_RESPONSE_FIELDS_FOR_2_KERNELS_PAD ((sizeof(struct _data_void_response_for_2_kernels)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _data_void_response_for_2_kernels)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[DATA_VOID_RESPONSE_FIELDS_FOR_2_KERNELS_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_void_response_for_2_kernels_t;

typedef struct mapping_answers_2_kernels {
	struct mapping_answers_2_kernels* next;
	struct mapping_answers_2_kernels* prev;

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
	data_response_for_2_kernels_t* data;
	int arrived_response;
	struct task_struct* waiting;
	int futex_owner;
#if TIMING
	unsigned long long start;
#endif

} mapping_answers_for_2_kernels_t;

#else

#define INVALID_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		unsigned long address; \
		long last_write;\
		unsigned long long time_stamp;\
		int vma_operation_index;

struct _invalid {
	INVALID_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			INVALID_FIELDS
		};

#define INVALID_PAD ((sizeof(struct _invalid)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _invalid)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[INVALID_PAD];
	}__attribute__((packed));

} __attribute__((packed)) invalid_data_t;

#define DATA_RESPONSE_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;  \
		unsigned long address; \
		int address_present;\
		char data[PAGE_SIZE]; \
		__wsum checksum; \
		long last_write;\
		int owners[MAX_KERNEL_IDS];\
		int vma_present; \
		unsigned long vaddr_start;\
		unsigned long vaddr_size;\
		pgprot_t prot; \
		unsigned long vm_flags; \
		unsigned long pgoff;\
		char path[512];\

struct _data_response {
	DATA_RESPONSE_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			DATA_RESPONSE_FIELDS
		};
#define DATA_RESPONSE_PAD ((sizeof(struct _data_response)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _data_response)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[DATA_RESPONSE_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_response_t;

#define DATA_VOID_RESPONSE_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;  \
		unsigned long address; \
		int vma_present; \
		unsigned long vaddr_start;\
		unsigned long vaddr_size;\
		unsigned long vm_flags; \
		unsigned long pgoff;\
		char path[512];\
		pgprot_t prot; \
		int address_present; \
		int fetching; \
		int owners[MAX_KERNEL_IDS];\
		__wsum checksum; \

struct _data_void_response {
	DATA_VOID_RESPONSE_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			DATA_VOID_RESPONSE_FIELDS
		};
#define DATA_VOID_RESPONSE_PAD ((sizeof(struct _data_void_response)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _data_void_response)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[DATA_VOID_RESPONSE_PAD];
	}__attribute__((packed));

}__attribute__((packed)) data_void_response_t;

typedef struct mapping_answers {
	struct mapping_answers* next;
	struct mapping_answers* prev;

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
	int address_present;
	long last_invalid;
	long last_write;
	int fetching;
	int responses;
	int expected_responses;
	int owners[MAX_KERNEL_IDS];
	int owner;
	data_response_t* data;
	raw_spinlock_t lock;
	struct task_struct* waiting;
#if TIMING
	unsigned long long start;
#endif
} mapping_answers_t;

#endif


#define VMA_ACK_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		int vma_operation_index;\
		unsigned long addr;

struct _vma_ack {
	VMA_ACK_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_ACK_FIELDS
		};

#define VMA_ACK_PAD ((sizeof(struct _vma_ack)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_ack)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[VMA_ACK_PAD];

	}__attribute__((packed));

} __attribute__((packed)) vma_ack_t;

/*typedef struct _vma_ack{
 struct pcn_kmsg_hdr header;
 int tgroup_home_cpu; //4
 int tgroup_home_id; //4
 int vma_operation_index;
 char pad[PCN_KMSG_PAYLOAD_SIZE -12];
 }__attribute__((packed)) __attribute__((aligned(64))) vma_ack_t;
 */

#define UNMAP_FIELDS  int tgroup_home_cpu; \
		int tgroup_home_id;\
		unsigned long start;\
		size_t len;

struct _unmap_message {
	UNMAP_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			UNMAP_FIELDS
		};
#define UNMAP_FIELDS_PAD ((sizeof(struct _unmap_message)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _unmap_message)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[UNMAP_FIELDS_PAD];
	}__attribute__((packed));

} __attribute__((packed)) unmap_message_t;

#define VMA_LOCK_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id; \
		int from_cpu;\
		int vma_operation_index;

struct _vma_lock {
	VMA_LOCK_FIELDS
};

typedef struct {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			VMA_LOCK_FIELDS
		};
#define VMA_LOCK_PAD ((sizeof(struct _vma_lock)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _vma_lock)):(PCN_KMSG_PAYLOAD_SIZE))

		char pad[VMA_LOCK_PAD];
	};

} vma_lock_t;

typedef struct vma_op_answers {
	struct vma_op_answers* next;
	struct vma_op_answers* prev;

	//data_header_t header;
	int tgroup_home_cpu;
	int tgroup_home_id;
	int responses;
	int expected_responses;
	int vma_operation_index;
	unsigned long address;
	struct task_struct *waiting;
	raw_spinlock_t lock;

} vma_op_answers_t;

typedef struct {
	struct delayed_work work;
	clone_request_t* request;
} clone_work_t;

typedef struct{
	struct work_struct work;
	back_migration_request_t* back_mig_request;
#if TIMING
	unsigned long long start;
#endif
}back_mig_work_t;

typedef struct {
	struct delayed_work work;
#if FOR_2_KERNELS
	data_request_for_2_kernels_t* request;
#else
	data_request_t* request;
#endif
	unsigned long address;
	int tgroup_home_cpu;
	int tgroup_home_id;
} request_work_t;

typedef struct {
	struct work_struct work;
	thread_group_exited_notification_t* request;
} exit_group_work_t;

typedef struct {
	struct work_struct work;
	ack_t* response;
} ack_work_t;

/*typedef struct {
	struct work_struct work;
	data_response_t* response;
} response_work_t;*/

typedef struct {
	struct delayed_work work;
#if FOR_2_KERNELS
	invalid_data_for_2_kernels_t* request;
#else
	invalid_data_t* request;
#endif
} invalid_work_t;

typedef struct {
	struct work_struct work;
	new_kernel_t* request;
} new_kernel_work_t;

typedef struct {
	struct work_struct work;
	new_kernel_answer_t* answer;
	memory_t* memory;
} new_kernel_work_answer_t;

typedef struct {
	struct work_struct work;
	exiting_process_t* request;
} exit_work_t;

typedef struct {
	struct work_struct work;
	remote_thread_count_request_t* request;
}count_work_t;

typedef struct {
	struct work_struct work;
	vma_operation_t* operation;
	memory_t* memory;
	int fake;
} vma_op_work_t;

typedef struct {
	struct work_struct work;
	unmap_message_t* unmap;
	int fake;
	memory_t* memory;
} vma_unmap_work_t;

typedef struct {
	struct work_struct work;
	vma_lock_t* lock;
	memory_t* memory;
} vma_lock_work_t;

typedef struct info_page_walk {
	struct vm_area_struct* vma;
} info_page_walk_t;

/**
 * Module variables
 */

static int _cpu = -1;

data_header_t* _data_head = NULL; // General purpose data store
fetching_t* _fetching_head = NULL;

#if FOR_2_KERNELS
mapping_answers_for_2_kernels_t* _mapping_head = NULL;
#else
mapping_answers_t* _mapping_head = NULL;
#endif

#if FOR_2_KERNELS
ack_answers_for_2_kernels_t* _ack_head = NULL;
#else
ack_answers_t* _ack_head = NULL;
#endif

memory_t* _memory_head = NULL;
count_answers_t* _count_head = NULL;
vma_op_answers_t* _vma_ack_head = NULL;
thread_pull_t* thread_pull_head = NULL;

DEFINE_RAW_SPINLOCK(_data_head_lock);
DEFINE_RAW_SPINLOCK(_fetching_head_lock);
DEFINE_RAW_SPINLOCK(_mapping_head_lock);
DEFINE_RAW_SPINLOCK(_ack_head_lock);
DEFINE_SPINLOCK(_memory_head_lock);
DEFINE_RAW_SPINLOCK(_count_head_lock);
DEFINE_RAW_SPINLOCK(_vma_ack_head_lock);
DEFINE_RAW_SPINLOCK(thread_pull_head_lock);

static struct workqueue_struct *clone_wq;
static struct workqueue_struct *exit_wq;
static struct workqueue_struct *exit_group_wq;
static struct workqueue_struct *message_request_wq;
static struct workqueue_struct *invalid_message_wq;
static struct workqueue_struct *vma_op_wq;
static struct workqueue_struct *vma_lock_wq;
static struct workqueue_struct *new_kernel_wq;

//wait lists
DECLARE_WAIT_QUEUE_HEAD( read_write_wait);
DECLARE_WAIT_QUEUE_HEAD( request_distributed_vma_op);

/* For timing measurements
 * */
#if TIMING
typedef struct {
	unsigned long long min;
	unsigned long long max;
	unsigned long long tot;
	unsigned long count;
	spinlock_t spinlock;
} time_values_t;

#define FRL 0 //fetch read local
#define FWL 1 //fetch write local
#define FRR 2 //fetch read remote
#define FWR 3 //fetch write remote
#define VW 4 // write on a valid copy
#define VR 5 // read on a valid copy (in multithread)
#define MW 6 // write on a mofified copy (in multithread)
#define MR 7 // read on a mofified copy (in multithread)
#define IW 8 // write on a invalid copy (only 2 kernels)
#define IR 9 // read invalid copy
#define NRR 10 //read on a not replicated copy (in multithread)
#define NRW 11 // write on a not replicated copy (in multithread)
#define NR_TYPES 12

#define FIRST_MIG 0
#define FIRST_MIG_WITH_FORK 1
#define NORMAL_MIG 2
#define BACK_MIG 3
#define FIRST_MIG_R 4
#define NORMAL_MIG_R 5
#define BACK_MIG_R 6
#define NR_MIG 7

time_values_t times[NR_TYPES];
time_values_t migration_times[NR_MIG];

static void update_time(unsigned long long time_elapsed, int type){

	if(type<0 || type>=NR_TYPES)
		return;

	spin_lock(&(times[type].spinlock));
	times[type].tot+=time_elapsed;
	times[type].count++;
	if(time_elapsed>times[type].max)
		times[type].max=time_elapsed;
	if(times[type].min==0)
		times[type].min=time_elapsed;
	else
		if(time_elapsed<times[type].min)
			times[type].min=time_elapsed;
	spin_unlock(&(times[type].spinlock));

}

static void update_time_migration(unsigned long long time_elapsed, int type){


	if(type<0 || type>=NR_MIG)
		return;

	spin_lock(&(migration_times[type].spinlock));
	migration_times[type].tot+=time_elapsed;
	migration_times[type].count++;
	if(time_elapsed>migration_times[type].max)
		migration_times[type].max=time_elapsed;
	if(migration_times[type].min==0)
		migration_times[type].min=time_elapsed;
	else
		if(time_elapsed<migration_times[type].min)
			migration_times[type].min=time_elapsed;
	spin_unlock(&(migration_times[type].spinlock));

}


static void print_time(){
	int i;
	printk("\nPage fault times:\n");
	/*printk(" #define FRL 0 //fetch read local\n"
"#define FWL 1 //fetch write local\n"
"#define FRR 2 //fetch read remote\n"
"#define FWR 3 //fetch write remote\n"
"#define VW 4 // write on a valid copy\n"
"#define VR 5 // read on a valid copy (in multithread)\n"
"#define MW 6 // write on a mofified copy (in multithread)\n"
"#define MR 7 // read on a mofified copy (in multithread) \n"
"#define IW 8 // write on a invalid copy (only 2 kernels)\n"
"#define IR 9 // read invalid copy\n"
"#define NRR 10 //read on a not replicated copy (in multithread)\n"
"# NRW 11 // write on a not replicated copy (in multithread)\n\n");*/
	for(i=0;i<NR_TYPES;i++){
		spin_lock(&(times[i].spinlock));
		unsigned long long avg=0;
		if(times[i].count!=0)
			avg= times[i].tot/times[i].count;
		printk("Type %d avg %lu max %lu min %lu count %lu tot %lu\n", i, avg, times[i].max, times[i].min, times[i].count, times[i].tot);
		times[i].max=0; times[i].min=0;times[i].tot=0;times[i].count=0;
		spin_unlock(&(times[i].spinlock));
	}
}


static void print_migration_time(){
	int i;
	printk("\nMigration times:\n");
	/*printk(" #define FIRST_MIG 0"
	 * "#define NORMAL_MIG 1"
	 * "#define BACK_MIG 2"
	 * "#define NR_MIG 3"
	 * "\n\n");*/
	for(i=0;i<NR_MIG;i++){
		spin_lock(&(migration_times[i].spinlock));
		unsigned long long avg=0;
		if(migration_times[i].count!=0)
			avg= migration_times[i].tot/migration_times[i].count;
		printk("Type %d avg %lu max %lu min %lu count %lu tot %lu\n", i, avg, migration_times[i].max, migration_times[i].min, migration_times[i].count, migration_times[i].tot);
		migration_times[i].max=0; migration_times[i].min=0;migration_times[i].tot=0;migration_times[i].count=0;
		spin_unlock(&(migration_times[i].spinlock));
	}
}


#endif


void push_data(data_header_t** phead, raw_spinlock_t* spinlock,
		data_header_t* entry) {
	unsigned long flags;
	data_header_t* head= *phead;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(spinlock, flags);

	entry->prev = NULL;

	if (!head) {
		entry->next = NULL;
		*phead = entry;
	} else {
		entry->next = head;
		head->prev = entry;
		*phead = entry;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);
}

data_header_t* pop_data(data_header_t** phead, raw_spinlock_t* spinlock) {
	data_header_t* ret = NULL;
	data_header_t* head= *phead;
	unsigned long flags;

	raw_spin_lock_irqsave(spinlock, flags);

	if (head) {
		ret = head;
		if (head->next)
			head->next->prev = NULL;
		*phead = head->next;
		ret->next = NULL;
		ret->prev = NULL;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}

int count_data(data_header_t** phead, raw_spinlock_t* spinlock) {
	int ret = 0;
	unsigned long flags;
	data_header_t* head= *phead;
	data_header_t* curr;

	raw_spin_lock_irqsave(spinlock, flags);

	curr = head;
	while (curr) {
		ret++;
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(spinlock, flags);

	return ret;
}

/* Functions to add,find and remove an entry from the fetching list (head:_fetching_head , lock:_fetching_head_lock)
 */

int add_fetching_entry(fetching_t* entry) {
	fetching_t* curr;
	fetching_t* prev;
	unsigned long flags;

	if (!entry) {
		return -1;
	}

	raw_spin_lock_irqsave(&_fetching_head_lock, flags);

	if (!_fetching_head) {
		_fetching_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _fetching_head;
		prev = NULL;
		do {
			if ((curr->tgroup_home_cpu == entry->tgroup_home_cpu)
					&& (curr->tgroup_home_id == entry->tgroup_home_id)
					&& (curr->vaddr == entry->vaddr)) {
				// It's already in the list!
				raw_spin_unlock_irqrestore(&_fetching_head_lock, flags);
				return -1;
			}
			prev = curr;
			curr = curr->next;

		} while (curr != NULL);
		// Now curr should be the last entry.
		// Append the new entry to curr.
		prev->next = entry;
		entry->next = NULL;
		entry->prev = prev;
	}

	raw_spin_unlock_irqrestore(&_fetching_head_lock, flags);

	return 1;

}

fetching_t* find_fetching_entry(int cpu, int id, unsigned long address) {
	fetching_t* curr = NULL;
	fetching_t* ret = NULL;
	unsigned long flags;

	raw_spin_lock_irqsave(&_fetching_head_lock, flags);

	curr = _fetching_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
				&& curr->vaddr == address) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	raw_spin_unlock_irqrestore(&_fetching_head_lock, flags);

	return ret;
}

void remove_fetching_entry(fetching_t* entry) {
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_fetching_head_lock, flags);

	if (_fetching_head == entry) {
		_fetching_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_fetching_head_lock, flags);

}

/* Functions to add,find and remove an entry from the mapping list (head:_mapping_head , lock:_mapping_head_lock)
 */
#if FOR_2_KERNELS

void add_mapping_entry(mapping_answers_for_2_kernels_t* entry) {

	mapping_answers_for_2_kernels_t* curr;

#else

void add_mapping_entry(mapping_answers_t* entry) {

	mapping_answers_t* curr;

#endif
		unsigned long flags;

		if (!entry) {
			return;
		}

		raw_spin_lock_irqsave(&_mapping_head_lock, flags);

		if (!_mapping_head) {
			_mapping_head = entry;
			entry->next = NULL;
			entry->prev = NULL;
		} else {
			curr = _mapping_head;
			while (curr->next != NULL) {
				if (curr == entry) {
					// It's already in the list!
					raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);
					return;
				}
				curr = curr->next;
			}
			// Now curr should be the last entry.
			// Append the new entry to curr.
			curr->next = entry;
			entry->next = NULL;
			entry->prev = curr;
		}

		raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

}

#if FOR_2_KERNELS

mapping_answers_for_2_kernels_t* find_mapping_entry(int cpu, int id, unsigned long address) {

		mapping_answers_for_2_kernels_t* curr = NULL;
		mapping_answers_for_2_kernels_t* ret = NULL;
#else
		mapping_answers_t* find_mapping_entry(int cpu, int id, unsigned long address) {

			mapping_answers_t* curr = NULL;
			mapping_answers_t* ret = NULL;
#endif
			unsigned long flags;

			raw_spin_lock_irqsave(&_mapping_head_lock, flags);

			curr = _mapping_head;
			while (curr) {

				if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
						&& curr->address == address) {
					ret = curr;
					break;
				}

				curr = curr->next;
			}

			raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

			return ret;
		}

#if FOR_2_KERNELS

void remove_mapping_entry(mapping_answers_for_2_kernels_t* entry) {

#else
			void remove_mapping_entry(mapping_answers_t* entry) {

#endif

				unsigned long flags;

				if (!entry) {
					return;
				}

				raw_spin_lock_irqsave(&_mapping_head_lock, flags);

				if (_mapping_head == entry) {
					_mapping_head = entry->next;
				}

				if (entry->next) {
					entry->next->prev = entry->prev;
				}

				if (entry->prev) {
					entry->prev->next = entry->next;
				}

				entry->prev = NULL;
				entry->next = NULL;

				raw_spin_unlock_irqrestore(&_mapping_head_lock, flags);

			}

			/* Functions to add,find and remove an entry from the ack list (head:_ack_head , lock:_ack_head_lock)
			 */

#if FOR_2_KERNELS
			void add_ack_entry(ack_answers_for_2_kernels_t* entry) {
				ack_answers_for_2_kernels_t* curr;
#else
				void add_ack_entry(ack_answers_t* entry) {
					ack_answers_t* curr;
#endif
					unsigned long flags;

					if (!entry) {
						return;
					}

					raw_spin_lock_irqsave(&_ack_head_lock, flags);

					if (!_ack_head) {
						_ack_head = entry;
						entry->next = NULL;
						entry->prev = NULL;
					} else {
						curr = _ack_head;
						while (curr->next != NULL) {
							if (curr == entry) {
								// It's already in the list!
								raw_spin_unlock_irqrestore(&_ack_head_lock, flags);
								return;
							}
							curr = curr->next;
						}
						// Now curr should be the last entry.
						// Append the new entry to curr.
						curr->next = entry;
						entry->next = NULL;
						entry->prev = curr;
					}

					raw_spin_unlock_irqrestore(&_ack_head_lock, flags);
				}

#if FOR_2_KERNELS
				ack_answers_for_2_kernels_t* find_ack_entry(int cpu, int id, unsigned long address) {
					ack_answers_for_2_kernels_t* curr = NULL;
					ack_answers_for_2_kernels_t* ret = NULL;
#else

					ack_answers_t* find_ack_entry(int cpu, int id, unsigned long address) {
						ack_answers_t* curr = NULL;
						ack_answers_t* ret = NULL;
#endif

						unsigned long flags;

						raw_spin_lock_irqsave(&_ack_head_lock, flags);

						curr = _ack_head;
						while (curr) {

							if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id
									&& curr->address == address) {
								ret = curr;
								break;
							}

							curr = curr->next;
						}

						raw_spin_unlock_irqrestore(&_ack_head_lock, flags);

						return ret;
					}

#if FOR_2_KERNELS

void remove_ack_entry(ack_answers_for_2_kernels_t* entry) {

#else

void remove_ack_entry(ack_answers_t* entry) {

#endif

	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_ack_head_lock, flags);

	if (_ack_head == entry) {
		_ack_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	raw_spin_unlock_irqrestore(&_ack_head_lock, flags);

}

/* Functions to add,find and remove an entry from the memory list (head:_memory_head , lock:_memory_head_lock)
 */

void add_memory_entry(memory_t* entry) {
	memory_t* curr;

	if (!entry) {
		return;
	}

	spin_lock(&_memory_head_lock);

	if (!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		while (curr->next != NULL) {
			if (curr == entry) {
				// It's already in the list!
				spin_unlock(&_memory_head_lock);
				return;
			}
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	spin_unlock(&_memory_head_lock);
}

int add_memory_entry_with_check(memory_t* entry) {
	memory_t* curr;

	if (!entry) {
		return -1;
	}

	spin_lock(&_memory_head_lock);

	if (!_memory_head) {
		_memory_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _memory_head;
		while (curr->next != NULL) {
			if (curr == entry
					|| (curr->tgroup_home_cpu == entry->tgroup_home_cpu
							&& curr->tgroup_home_id == entry->tgroup_home_id)) {
				// It's already in the list!
				spin_unlock(&_memory_head_lock);
				return -1;
			}
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	spin_unlock(&_memory_head_lock);

	return 0;
}

memory_t* find_memory_entry(int cpu, int id) {
	memory_t* curr = NULL;
	memory_t* ret = NULL;

	spin_lock(&_memory_head_lock);

	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	spin_unlock(&_memory_head_lock);

	return ret;
}

struct mm_struct* find_dead_mapping(int cpu, int id) {
	memory_t* curr = NULL;
	struct mm_struct* ret = NULL;

	spin_lock(&_memory_head_lock);

	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr->mm;
			break;
		}
		curr = curr->next;
	}

	spin_unlock(&_memory_head_lock);

	return ret;
}

memory_t* find_and_remove_memory_entry(int cpu, int id) {
	memory_t* curr = NULL;
	memory_t* ret = NULL;

	spin_lock(&_memory_head_lock);

	curr = _memory_head;
	while (curr) {
		if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			ret = curr;
			break;
		}
		curr = curr->next;
	}

	if (ret) {
		if (_memory_head == ret) {
			_memory_head = ret->next;
		}

		if (ret->next) {
			ret->next->prev = ret->prev;
		}

		if (ret->prev) {
			ret->prev->next = ret->next;
		}

		ret->prev = NULL;
		ret->next = NULL;
	}

	spin_unlock(&_memory_head_lock);

	return ret;
}

void remove_memory_entry(memory_t* entry) {

	if (!entry) {
		return;
	}

	spin_lock(&_memory_head_lock);

	if (_memory_head == entry) {
		_memory_head = entry->next;
	}

	if (entry->next) {
		entry->next->prev = entry->prev;
	}

	if (entry->prev) {
		entry->prev->next = entry->next;
	}

	entry->prev = NULL;
	entry->next = NULL;

	spin_unlock(&_memory_head_lock);

}

/* Functions to add,find and remove an entry from the count list (head:_count_head , lock:_count_head_lock)
 */

 void add_count_entry(count_answers_t* entry) {
	count_answers_t* curr;
	unsigned long flags;

	if (!entry) {
		return;
	}

	raw_spin_lock_irqsave(&_count_head_lock, flags);

	if (!_count_head) {
		_count_head = entry;
		entry->next = NULL;
		entry->prev = NULL;
	} else {
		curr = _count_head;
		while (curr->next != NULL) {
			if (curr == entry) {
				// It's already in the list!
				raw_spin_unlock_irqrestore(&_count_head_lock, flags);
				return;
			}
			curr = curr->next;
		}
		// Now curr should be the last entry.
		// Append the new entry to curr.
		curr->next = entry;
		entry->next = NULL;
		entry->prev = curr;
	}

	raw_spin_unlock_irqrestore(&_count_head_lock, flags);
 }

 count_answers_t* find_count_entry(int cpu, int id) {
	 count_answers_t* curr = NULL;
	 count_answers_t* ret = NULL;
	 unsigned long flags;

	 raw_spin_lock_irqsave(&_count_head_lock, flags);

	 curr = _count_head;
	 while (curr) {
		 if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			 ret = curr;
			 break;
		 }
		 curr = curr->next;
	 }

	 raw_spin_unlock_irqrestore(&_count_head_lock, flags);

	 return ret;
 }

 void remove_count_entry(count_answers_t* entry) {

	 unsigned long flags;

	 if (!entry) {
		 return;
	 }

	 raw_spin_lock_irqsave(&_count_head_lock, flags);

	 if (_count_head == entry) {
		 _count_head = entry->next;
	 }

	 if (entry->next) {
		 entry->next->prev = entry->prev;
	 }

	 if (entry->prev) {
		 entry->prev->next = entry->next;
	 }

	 entry->prev = NULL;
	 entry->next = NULL;

	 raw_spin_unlock_irqrestore(&_count_head_lock, flags);
 }

 void add_vma_ack_entry(vma_op_answers_t* entry) {
	 vma_op_answers_t* curr;
	 unsigned long flags;

	 if (!entry) {
		 return;
	 }

	 raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	 if (!_vma_ack_head) {
		 _vma_ack_head = entry;
		 entry->next = NULL;
		 entry->prev = NULL;
	 } else {
		 curr = _vma_ack_head;
		 while (curr->next != NULL) {
			 if (curr == entry) {
				 // It's already in the list!
				 raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);
				 return;
			 }
			 curr = curr->next;
		 }
		 // Now curr should be the last entry.
		 // Append the new entry to curr.
		 curr->next = entry;
		 entry->next = NULL;
		 entry->prev = curr;
	 }

	 raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);
 }

 vma_op_answers_t* find_vma_ack_entry(int cpu, int id) {
	 vma_op_answers_t* curr = NULL;
	 vma_op_answers_t* ret = NULL;

	 unsigned long flags;
	 raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	 curr = _vma_ack_head;
	 while (curr) {

		 if (curr->tgroup_home_cpu == cpu && curr->tgroup_home_id == id) {
			 ret = curr;
			 break;
		 }

		 curr = curr->next;
	 }

	 raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);

	 return ret;
 }

 void remove_vma_ack_entry(vma_op_answers_t* entry) {
	 unsigned long flags;

	 if (!entry) {
		 return;
	 }

	 raw_spin_lock_irqsave(&_vma_ack_head_lock, flags);

	 if (_vma_ack_head == entry) {
		 _vma_ack_head = entry->next;
	 }

	 if (entry->next) {
		 entry->next->prev = entry->prev;
	 }

	 if (entry->prev) {
		 entry->prev->next = entry->next;
	 }

	 entry->prev = NULL;
	 entry->next = NULL;

	 raw_spin_unlock_irqrestore(&_vma_ack_head_lock, flags);

 }

#if STATISTICS
unsigned long long perf_aa, perf_bb, perf_cc, perf_dd, perf_ee;
static int page_fault_mio,fetch,local_fetch,write,concurrent_write,most_long_write,most_written_page,read,most_long_read,invalid,ack,answer_request,answer_request_void,request_data,pages_allocated,compressed_page_sent,not_compressed_page,not_compressed_diff_page;
#endif

extern unsigned long read_old_rsp(void);
extern int exec_mmap(struct mm_struct *mm);
extern struct task_struct* do_fork_for_main_kernel_thread(unsigned long clone_flags,
		unsigned long stack_start,
		struct pt_regs *regs,
		unsigned long stack_size,
		int __user *parent_tidptr,
		int __user *child_tidptr);

int count_remote_thread_members(int tgroup_home_cpu, int tgroup_home_id,memory_t* mm_data) {

	count_answers_t* data;
	remote_thread_count_request_t* request;
	int i, s;
	int ret = -1;
	unsigned long flags;

	data = (count_answers_t*) kmalloc(sizeof(count_answers_t), GFP_ATOMIC);
	if (!data)
		return -1;

	data->responses = 0;
	data->tgroup_home_cpu = tgroup_home_cpu;
	data->tgroup_home_id = tgroup_home_id;
	data->count = 0;
	data->waiting = current;
	raw_spin_lock_init(&(data->lock));

	add_count_entry(data);

	request= (remote_thread_count_request_t*) kmalloc(sizeof(remote_thread_count_request_t),GFP_ATOMIC);
	if(request==NULL)
		return -1;

	request->header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;
	request->tgroup_home_cpu = tgroup_home_cpu;
	request->tgroup_home_id = tgroup_home_id;

	data->expected_responses = 0;

	down_read(&mm_data->kernel_set_sem);

	//printk("%s before sending data->expected_responses %d data->responses %d\n",__func__,data->expected_responses,data->responses);
#ifndef SUPPORT_FOR_CLUSTERING
	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		// Skip the current cpu
		if(i == _cpu) continue;

#else
		// the list does not include the current processor group descirptor (TODO)
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;
#endif
if(mm_data->kernel_set[i]==1){
	// Send the request to this cpu.
	//s = pcn_kmsg_send(i, (struct pcn_kmsg_message*) (&request));
	s = pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (request),sizeof(remote_thread_count_request_t)- sizeof(struct pcn_kmsg_hdr));
	if (s!=-1) {
		// A successful send operation, increase the number
		// of expected responses.
		data->expected_responses++;
	}
}
		}

		up_read(&mm_data->kernel_set_sem);
		//printk("%s going to sleep data->expected_responses %d data->responses %d\n",__func__,data->expected_responses,data->responses);
		while (data->expected_responses != data->responses) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (data->expected_responses != data->responses)
				schedule();

			set_task_state(current, TASK_RUNNING);
		}

		//printk("%s waked up data->expected_responses%d data->responses%d\n",__func__,data->expected_responses,data->responses);
		raw_spin_lock_irqsave(&(data->lock), flags);
		raw_spin_unlock_irqrestore(&(data->lock), flags);
		// OK, all responses are in, we can proceed.
		//printk("%s data->count is %d",__func__,data->count);
		ret = data->count;
		remove_count_entry(data);
		kfree(data);
		kfree(request);
		return ret;
	}

	void process_vma_op(struct work_struct* work);

	static void process_new_kernel_answer(struct work_struct* work){
		new_kernel_work_answer_t* my_work= (new_kernel_work_answer_t*)work;
		new_kernel_answer_t* answer= my_work->answer;
		memory_t* memory= my_work->memory;

		if(answer->header.from_cpu==answer->tgroup_home_cpu){
			down_write(&memory->mm->mmap_sem);
			//	printk("%s answer->vma_operation_index %d NR_CPU %d\n",__func__,answer->vma_operation_index,MAX_KERNEL_IDS);
			memory->mm->vma_operation_index= answer->vma_operation_index;
			up_write(&memory->mm->mmap_sem);
		}

		down_write(&memory->kernel_set_sem);
		int i;
		for(i=0;i<MAX_KERNEL_IDS;i++){
			if(answer->my_set[i]==1)
				memory->kernel_set[i]= 1;
		}
		memory->answers++;


		if(memory->answers >= memory->exp_answ)
			wake_up_process(memory->main);

		up_write(&memory->kernel_set_sem);

		pcn_kmsg_free_msg(answer);
		kfree(work);

	}

	static int handle_new_kernel_answer(struct pcn_kmsg_message* inc_msg){
		new_kernel_answer_t* answer= (new_kernel_answer_t*)inc_msg;
		memory_t* memory= find_memory_entry(answer->tgroup_home_cpu,
				answer->tgroup_home_id);

		PSNEWTHREADPRINTK("received new kernel answer\n");
		//printk("%s: %d\n",__func__,answer->vma_operation_index);
		if(memory!=NULL){
			new_kernel_work_answer_t* work= (new_kernel_work_answer_t*)kmalloc(sizeof(new_kernel_work_answer_t), GFP_ATOMIC);
			if(work!=NULL){
				work->answer = answer;
				work->memory= memory;
				INIT_WORK( (struct work_struct*)work, process_new_kernel_answer);
				queue_work(new_kernel_wq, (struct work_struct*) work);
			}
			else
				pcn_kmsg_free_msg(inc_msg);
		}
		else{
			printk("ERROR: received an answer new kernel but memory not present\n");
			pcn_kmsg_free_msg(inc_msg);
		}

		return 1;
	}

	void process_new_kernel(struct work_struct* work){
		new_kernel_work_t* new_kernel_work= (new_kernel_work_t*) work;
		memory_t* memory;

		PSNEWTHREADPRINTK("received new kernel request\n");

		new_kernel_answer_t* answer= (new_kernel_answer_t*) kmalloc(sizeof(new_kernel_answer_t), GFP_ATOMIC);

		if(answer!=NULL){
			memory = find_memory_entry(new_kernel_work->request->tgroup_home_cpu,
					new_kernel_work->request->tgroup_home_id);
			if (memory != NULL) {

				down_write(&memory->kernel_set_sem);
				memory->kernel_set[new_kernel_work->request->header.from_cpu]= 1;
				memcpy(answer->my_set,memory->kernel_set,MAX_KERNEL_IDS*sizeof(int));
				up_write(&memory->kernel_set_sem);

				if(_cpu==new_kernel_work->request->tgroup_home_cpu){
					down_read(&memory->mm->mmap_sem);
					answer->vma_operation_index= memory->mm->vma_operation_index;
					//printk("%s answer->vma_operation_index %d \n",__func__,answer->vma_operation_index);
					up_read(&memory->mm->mmap_sem);
				}

			}
			else{
				memset(answer->my_set,0,MAX_KERNEL_IDS*sizeof(int));
			}

			answer->tgroup_home_cpu= new_kernel_work->request->tgroup_home_cpu;
			answer->tgroup_home_id= new_kernel_work->request->tgroup_home_id;
			answer->header.type= PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL_ANSWER;
			answer->header.prio= PCN_KMSG_PRIO_NORMAL;
			//printk("just before send %s answer->vma_operation_index %d NR_CPU %d\n",__func__,answer->vma_operation_index,MAX_KERNEL_IDS);
			pcn_kmsg_send_long(new_kernel_work->request->header.from_cpu,
					(struct pcn_kmsg_long_message*) answer,
					sizeof(new_kernel_answer_t) - sizeof(struct pcn_kmsg_hdr));
			//int ret=pcn_kmsg_send(new_kernel_work->request->header.from_cpu, (struct pcn_kmsg_long_message*) answer);
			//printk("%s send long ret is %d sizeof new_kernel_answer_t is %d size of header is %d\n",__func__,ret,sizeof(new_kernel_answer_t),sizeof(struct pcn_kmsg_hdr));
			kfree(answer);

		}

		pcn_kmsg_free_msg(new_kernel_work->request);
		kfree(work);

	}

	static int handle_new_kernel(struct pcn_kmsg_message* inc_msg) {
		new_kernel_t* new_kernel= (new_kernel_t*)inc_msg;
		new_kernel_work_t* request_work;

		request_work = (new_kernel_work_t*) kmalloc(sizeof(new_kernel_work_t), GFP_ATOMIC);

		if (request_work) {
			request_work->request = new_kernel;
			INIT_WORK( (struct work_struct*)request_work, process_new_kernel);
			queue_work(new_kernel_wq, (struct work_struct*) request_work);
		}

		return 1;

	}

	static int create_kernel_thread_for_distributed_process(void *data);

	static void update_thread_pull(struct work_struct* work) {

		int i, count;

		count = count_data((data_header_t**)&thread_pull_head, &thread_pull_head_lock);

		for (i = 0; i < NR_THREAD_PULL - count; i++) {

			printk("%s creating thread pull %d \n", __func__, i);

			kernel_thread(create_kernel_thread_for_distributed_process, NULL, SIGCHLD);

		}

		kfree(work);

	}

	static void _create_thread_pull(struct work_struct* work){

	 	int i;

	        for(i=0;i<NR_THREAD_PULL;i++){

		        printk("%s creating thread pull %d \n",__func__,i);

	        	kernel_thread(create_kernel_thread_for_distributed_process, NULL, SIGCHLD);

	        }

		create_thread_pull_t* msg= (create_thread_pull_t*) kmalloc(sizeof(create_thread_pull_t),GFP_ATOMIC);
		if(!msg){
			printk("%s Impossible to kmalloc",__func__);
			return;
		}

		msg->header.type= PCN_KMSG_TYPE_PROC_SRV_CREATE_THREAD_PULL;
		msg->header.prio= PCN_KMSG_PRIO_NORMAL;

	#ifndef SUPPORT_FOR_CLUSTERING
		for(i = 0; i < MAX_KERNEL_IDS; i++) {
			if(i == _cpu) continue;
	#else
			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head) {
				objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
				i = objPtr->_data._processor;
	#endif
		pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) (msg),
				sizeof(create_thread_pull_t)-sizeof(struct pcn_kmsg_hdr));

		}

		kfree(msg);
		kfree(work);

	}

	void create_thread_pull(void){

		static int only_one=0;

		if(only_one==0){
			struct work_struct* work = kmalloc(sizeof(struct work_struct),GFP_ATOMIC);

	        	if (work) {
	       			INIT_WORK( work, _create_thread_pull);
	        		queue_work(clone_wq, work);
	       		}

			only_one++;
		}

	}

	static int handle_thread_pull_creation(struct pcn_kmsg_message* inc_msg){

		create_thread_pull();
	        pcn_kmsg_free_msg(inc_msg);
	        return 0;
	}

	/* return type:
	 * 0 normal;
	 * 1 flush pending operation
	 * */
	static int exit_distributed_process(memory_t* mm_data, int flush,thread_pull_t * my_thread_pull) {
		struct task_struct *g;
		unsigned long flags;
		int is_last_thread_in_local_group = 1;
		int count = 0, i, status;
		thread_group_exited_notification_t* exit_notification;

		lock_task_sighand(current, &flags);
		g = current;
		while_each_thread(current, g)
		{
			if (g->main == 0 && g->distributed_exit == EXIT_ALIVE) {
				is_last_thread_in_local_group = 0;
				goto find;
			}
		};
		find: status = current->distributed_exit;
		current->distributed_exit = EXIT_ALIVE;
		unlock_task_sighand(current, &flags);

		if (mm_data->alive == 0 && !is_last_thread_in_local_group && atomic_read(&(mm_data->pending_migration))==0) {
			printk("ERROR: mm_data->alive is 0 but there are alive threads\n");
			return 0;
		}

		if (mm_data->alive == 0  && atomic_read(&(mm_data->pending_migration))==0) {
			

			if (status == EXIT_THREAD) {
				printk("ERROR: alive is 0 but status is exit thread\n");
				return flush;
			}

			if (status == EXIT_PROCESS) {

				if (flush == 0) {
					//this is needed to flush the list of pending operation before die

#if NOT_REPLICATED_VMA_MANAGEMENT
					vma_op_work_t* work = kmalloc(sizeof(vma_op_work_t),
							GFP_ATOMIC);

					if (work) {
						work->fake = 1;
						work->memory = mm_data;
						mm_data->arrived_op = 0;
						INIT_WORK( (struct work_struct*)work, process_vma_op);
						queue_work(vma_op_wq, (struct work_struct*) work);
					}
#else
#if PARTIAL_VMA_MANAGEMENT

					vma_unmap_work_t* work = kmalloc(sizeof(vma_unmap_work_t),
							GFP_ATOMIC);

					if (work) {
						work->fake = 1;
						work->memory= mm_data;
						mm_data->arrived_op= 0;
						INIT_WORK( (struct work_struct*)work, process_vma_op);
						queue_work(vma_op_wq, (struct work_struct*) work);
					}
#endif
#endif
					//printk("flush set to 1 here");
					return 1;
				}

			}

			if (flush == 1 && mm_data->arrived_op == 0) {
				if (status == EXIT_FLUSHING)
					printk("ERROR: status exit flush but arrived op is 0\n");

				return 1;
			} else {

				if(atomic_read(&(mm_data->pending_migration))!=0)
					printk("ERROR pending migration when cleaning memory\n");

				shadow_thread_t* my_shadow= NULL;

				my_shadow = (shadow_thread_t*) pop_data( (data_header_t **)&(my_thread_pull->threads),
						&(my_thread_pull->spinlock));

				while(my_shadow){
					my_shadow->thread->distributed_exit= EXIT_THREAD;
					wake_up_process(my_shadow->thread);
					kfree(my_shadow);
					my_shadow = (shadow_thread_t*) pop_data( (data_header_t **)&(my_thread_pull->threads),
							&(my_thread_pull->spinlock));
				}

				remove_memory_entry(mm_data);
				mmput(mm_data->mm);
				kfree(mm_data);

				//printk("main exit\n");
#if STATISTICS
				printk("page_fault %i fetch %i local_fetch %i write %i read %i most_long_read %i invalid %i ack %i answer_request %i answer_request_void %i request_data %i most_written_page %i concurrent_writes %i most long write %i pages_allocated %i compressed_page_sent %i not_compressed_page %i not_compressed_diff_page %i\n",
						page_fault_mio,fetch,local_fetch,write,read,most_long_read,invalid,ack,answer_request,answer_request_void, request_data,most_written_page, concurrent_write,most_long_write, pages_allocated,compressed_page_sent,not_compressed_page,not_compressed_diff_page);


#endif

#if TIMING
				print_time();
				print_migration_time();
#endif

#if STATISTICS

#endif

				struct work_struct* work = kmalloc(sizeof(struct work_struct),
						GFP_ATOMIC);
				if (work) {
					INIT_WORK( work, update_thread_pull);
					queue_work(clone_wq, work);
				}

				do_exit(0);

				return 0;
			}

		}

		else {
			/* If I am the last thread of my process in this kernel:
			 * - or I am the last thread of the process on all the system => send a group exit to all kernels and erase the mapping saved
			 * - or there are other alive threads in the system => do not erase the saved mapping
			 */
			if (is_last_thread_in_local_group) {


				PSPRINTK(
						"%s: This is the last thread of process (id %d, cpu %d) in the kernel!\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);

				//mm_data->alive = 0;

				count = count_remote_thread_members(current->tgroup_home_cpu,
						current->tgroup_home_id,mm_data);

				/* Ok this is complicated.
				 * If count is zero=> all the threads of my process went through this exit function (all task->distributed_exit==1 or
				 * there are no more tasks of this process around).
				 * Dying tasks that did not see count==0 saved a copy of the mapping. Someone should notice their kernels that now they can erase it.
				 * I can be the one, however more threads can be concurrently in this exit function on different kernels =>
				 * each one of them can see the count==0 => more than one "erase mapping message" can be sent.
				 * If count==0 I check if I already receive a "erase mapping message" and avoid to send another one.
				 * This check does not guarantee that more than one "erase mapping message" cannot be sent (in some executions it is inevitable) =>
				 * just be sure to not call more than one mmput one the same mapping!!!
				 */
				if (count == 0) {

					mm_data->alive = 0;

					if (status != EXIT_PROCESS) {

						PSPRINTK(
								"%s: This is the last thread of process (id %d, cpu %d) in the system, "
								"sending an erase mapping message!\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);
						//printk(		"%s: This is the last thread of process (id %d, cpu %d) in the system, "
						//		"sending an erase mapping message!\n", __func__, current->tgroup_home_id, current->tgroup_home_cpu);

						exit_notification= (thread_group_exited_notification_t*) kmalloc(sizeof(thread_group_exited_notification_t),GFP_ATOMIC);
						exit_notification->header.type =
								PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION;
						exit_notification->header.prio = PCN_KMSG_PRIO_NORMAL;
						exit_notification->tgroup_home_cpu =
								current->tgroup_home_cpu;
						exit_notification->tgroup_home_id = current->tgroup_home_id;

#ifndef SUPPORT_FOR_CLUSTERING
						for(i = 0; i < MAX_KERNEL_IDS; i++) {
							// Skip the current cpu
							if(i == _cpu) continue;

#else
							// the list does not include the current processor group descirptor (TODO)
							struct list_head *iter;
							_remote_cpu_info_list_t *objPtr;
							list_for_each(iter, &rlist_head) {
								objPtr =
										list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
								i = objPtr->_data._processor;
#endif
//pcn_kmsg_send(i,
								//		(struct pcn_kmsg_message*) (&exit_notification));
								pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*)(exit_notification),sizeof(thread_group_exited_notification_t)- sizeof(struct pcn_kmsg_hdr));

							}

							kfree(exit_notification);

						}

						if (flush == 0) {

							//this is needed to flush the list of pending operation before die

#if NOT_REPLICATED_VMA_MANAGEMENT
							vma_op_work_t* work = kmalloc(sizeof(vma_op_work_t),
									GFP_ATOMIC);

							if (work) {
								work->fake = 1;
								work->memory = mm_data;
								mm_data->arrived_op = 0;
								INIT_WORK( (struct work_struct*)work, process_vma_op);
								queue_work(vma_op_wq, (struct work_struct*) work);
							}
#else
#if PARTIAL_VMA_MANAGEMENT

							vma_unmap_work_t* work = kmalloc(sizeof(vma_unmap_work_t),
									GFP_ATOMIC);

							if (work) {
								work->fake = 1;
								work->memory= mm_data;
								mm_data->arrived_op = 0;
								INIT_WORK( (struct work_struct*)work, process_vma_op);
								queue_work(vma_op_wq, (struct work_struct*) work);
							}
#endif											
#endif												
							//printk("flush set to 1 there\n");
							return 1;

						} else {

							printk(
									"ERROR: flush is 1 during first exit (alive set to 0 now)\n");
							return 1;
						}

					}else
					{
						/*case i am the last thread but count is not zero
						* check if there are concurrent migration to be sure if I can put mm_data->alive = 0;
						*/
						if(atomic_read(&(mm_data->pending_migration))==0)
							mm_data->alive = 0;
					}

				}

				if ((!is_last_thread_in_local_group || count != 0)
						&& status == EXIT_PROCESS) {
					printk(
							"ERROR: received an exit process but is_last_thread_in_local_group id %d and count is %d\n ",
							is_last_thread_in_local_group, count);
				}

				return 0;
			}
		}

		static void create_new_threads(thread_pull_t * my_thread_pull,
				int *spare_threads) {
			int count;

			count = count_data((data_header_t**) &(my_thread_pull->threads), &(my_thread_pull->spinlock));

			if (count == 0) {

				while (count < *spare_threads) {
					count++;

					shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(
							sizeof(shadow_thread_t), GFP_ATOMIC);
					if (shadow) {

						struct pt_regs regs;

						//devo creare registri lato utente????
						memset(&regs, 0, sizeof(regs));

						//regs.si = (unsigned long) fn;
						//regs.di = (unsigned long) shadow;

		#ifdef CONFIG_X86_32
						regs.ds = __USER_DS;
						regs.es = __USER_DS;
						regs.fs = __KERNEL_PERCPU;
						regs.gs = __KERNEL_STACK_CANARY;
		#else
						regs.ss = __KERNEL_DS;
		#endif

						regs.orig_ax = -1;
						//regs.ip = (unsigned long) kernel_thread_helper;
						regs.cs = __KERNEL_CS | get_kernel_rpl();
						regs.flags = X86_EFLAGS_IF | 0x2;

						/* Ok, create the new process.. */
						shadow->thread =
								do_fork_for_main_kernel_thread(
										CLONE_THREAD | CLONE_SIGHAND | CLONE_VM
												| CLONE_UNTRACED, 0, &regs, 0, NULL,
										NULL);
						if (!IS_ERR(shadow->thread)) {
							printk("%s new shadow created\n",__func__);
							push_data((data_header_t**)&(my_thread_pull->threads),
									&(my_thread_pull->spinlock), (data_header_t*) shadow);
						} else {
							printk("ERROR not able to create shadow\n");
							kfree(shadow);
						}
					} else
						printk("ERROR impossible to kmalloc in %s\n", __func__);
				}

				*spare_threads = *spare_threads * 2;
			}

		}

		static void main_for_distributed_kernel_thread(memory_t* mm_data,
				thread_pull_t * my_thread_pull) {
			struct file* f;
			unsigned long ret = 0;
			int flush = 0;
			int count;
			int spare_threads = 2;

			while (1) {
				again:

				create_new_threads(my_thread_pull, &spare_threads);

				while (current->distributed_exit != EXIT_ALIVE) {
					flush = exit_distributed_process(mm_data, flush, my_thread_pull);
				}

				while (mm_data->operation != VMA_OP_NOP
						&& mm_data->mm->thread_op == current) {

					switch (mm_data->operation) {

					case VMA_OP_UNMAP:
						down_write(&mm_data->mm->mmap_sem);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 0;
						ret = do_munmap(mm_data->mm, mm_data->addr, mm_data->len);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 1;
						up_write(&mm_data->mm->mmap_sem);
						break;
					case VMA_OP_PROTECT:
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 0;
						ret = kernel_mprotect(mm_data->addr, mm_data->len,
								mm_data->prot);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 1;
						break;
					case VMA_OP_REMAP:
						down_write(&mm_data->mm->mmap_sem);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 0;
						ret = do_mremap(mm_data->addr, mm_data->len, mm_data->new_len,
								mm_data->flags, mm_data->new_addr);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 1;
						up_write(&mm_data->mm->mmap_sem);
						break;
					case VMA_OP_BRK:
						ret = -1;
						down_write(&mm_data->mm->mmap_sem);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 0;
						ret = do_brk(mm_data->addr, mm_data->len);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 1;
						up_write(&mm_data->mm->mmap_sem);

						break;
					case VMA_OP_MAP:

						ret = -1;
						f = NULL;
						if (mm_data->path[0] != '\0') {

							f = filp_open(mm_data->path, O_RDONLY | O_LARGEFILE, 0);
							if (IS_ERR(f)) {
								printk("ERROR: cannot open file to map\n");
								break;
							}

						}

						down_write(&mm_data->mm->mmap_sem);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 0;
						ret = do_mmap_pgoff(f, mm_data->addr, mm_data->len,
								mm_data->prot, mm_data->flags, mm_data->pgoff);
						if (current->tgroup_home_cpu != _cpu)
							mm_data->mm->distribute_unmap = 1;
						up_write(&mm_data->mm->mmap_sem);

						if (mm_data->path[0] != '\0') {

							filp_close(f, NULL);

						}

						break;
					default:
						break;
					}
					mm_data->addr = ret;
					mm_data->operation = VMA_OP_NOP;

					wake_up_process(mm_data->waiting_for_main);
				}

				__set_task_state(current, TASK_UNINTERRUPTIBLE);

				count = count_data((data_header_t**)&(my_thread_pull->threads), &my_thread_pull->spinlock);

				if (count == 0 || current->distributed_exit != EXIT_ALIVE
						|| (mm_data->operation != VMA_OP_NOP
								&& mm_data->mm->thread_op == current)) {
					__set_task_state(current, TASK_RUNNING);
					goto again;
				}

				schedule();

			}

		}


		static int create_kernel_thread_for_distributed_process_from_user_one(
				void *data) {

			memory_t* entry = (memory_t*) data;
			thread_pull_t* my_thread_pull;
			int i;

			current->main = 1;
			entry->main= current;

			if (!popcorn_ns) {
				if ((build_popcorn_ns(0)))
					printk("%s: build_popcorn returned error\n", __func__);
			}

			my_thread_pull = (thread_pull_t*) kmalloc(sizeof(thread_pull_t),
					GFP_ATOMIC);
			if (!my_thread_pull) {
				printk("ERROR kmalloc thread pull\n");
				return -1;
			}

			raw_spin_lock_init(&(my_thread_pull->spinlock));
			my_thread_pull->main = current;
			my_thread_pull->memory = entry;
			my_thread_pull->threads= NULL;

			entry->thread_pull= my_thread_pull;

			//int count= count_data((data_header_t**)&(thread_pull_head), &thread_pull_head_lock);
			//printk("WARNING count is %d in %s\n",count,__func__);
			for (i = 0; i < NR_CPUS; i++) {
				shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(
						sizeof(shadow_thread_t), GFP_ATOMIC);
				if (shadow) {

					struct pt_regs regs;

					//devo creare registri lato utente????
					memset(&regs, 0, sizeof(regs));

					//regs.si = (unsigned long) fn;
					//regs.di = (unsigned long) shadow;

		#ifdef CONFIG_X86_32
					regs.ds = __USER_DS;
					regs.es = __USER_DS;
					regs.fs = __KERNEL_PERCPU;
					regs.gs = __KERNEL_STACK_CANARY;
		#else
					regs.ss = __KERNEL_DS;
		#endif

					regs.orig_ax = -1;
					//regs.ip = (unsigned long) kernel_thread_helper;
					regs.cs = __KERNEL_CS | get_kernel_rpl();
					regs.flags = X86_EFLAGS_IF | 0x2;

					/* Ok, create the new process.. */
					shadow->thread = do_fork_for_main_kernel_thread(
							CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | CLONE_UNTRACED, 0,
							&regs, 0, NULL, NULL);
					if (!IS_ERR(shadow->thread)) {
						//printk("%s new shadow created\n",__func__);
						push_data((data_header_t**)&(my_thread_pull->threads), &(my_thread_pull->spinlock),
								(data_header_t*)shadow);
					} else {
						printk("ERROR not able to create shadow\n");
						kfree(shadow);
					}
				}
			}

			main_for_distributed_kernel_thread(entry,my_thread_pull);

			/* if here something went wrong....
			 */

			printk("ERROR: exited from main_for_distributed_kernel_thread\n");

			return 0;
		}


			static int handle_mapping_response_void(struct pcn_kmsg_message* inc_msg) {

#if FOR_2_KERNELS

				data_void_response_for_2_kernels_t* response;
				mapping_answers_for_2_kernels_t* fetched_data;

				response = (data_void_response_for_2_kernels_t*) inc_msg;
				fetched_data = find_mapping_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

#if STATISTICS
				answer_request_void++;
#endif

				PSPRINTK("answer_request_void %i address %lu from cpu %i. This is a void response.\n", answer_request_void, response->address, inc_msg->hdr.from_cpu);

				PSMINPRINTK("answer_request_void address %lu from cpu %i. This is a void response.\n", response->address, inc_msg->hdr.from_cpu);

				if (fetched_data == NULL) {
					PSPRINTK("data not found in local list\n");
					pcn_kmsg_free_msg(inc_msg);
					return -1;

				}

				if (response->owner == 1) {
					PSPRINTK("Response with ownership\n");
					fetched_data->owner = 1;
				}

				if (response->vma_present == 1) {

#if NOT_REPLICATED_VMA_MANAGEMENT
					if (response->header.from_cpu != response->tgroup_home_cpu)
						printk(
								"ERROR: a kernel that is not the server is sending the mapping\n");
#endif
					if (fetched_data->vma_present == 0) {
						PSPRINTK("Set vma\n");
						fetched_data->vma_present = 1;
						fetched_data->vaddr_start = response->vaddr_start;
						fetched_data->vaddr_size = response->vaddr_size;
						fetched_data->prot = response->prot;
						fetched_data->pgoff = response->pgoff;
						fetched_data->vm_flags = response->vm_flags;
						strcpy(fetched_data->path, response->path);
					}
#if NOT_REPLICATED_VMA_MANAGEMENT
					else
						printk("ERROR: received more than one mapping\n");
#endif
				}

				if(fetched_data->arrived_response!=0)
					printk("ERROR: received more than one answer, arrived_response is %d \n",fetched_data->arrived_response);

				fetched_data->arrived_response++;

	fetched_data->futex_owner = response->futex_owner;

				wake_up_process(fetched_data->waiting);

				pcn_kmsg_free_msg(inc_msg);

				return 1;

#else

				data_void_response_t* response;
				mapping_answers_t* fetched_data;
				unsigned long flags;
				struct task_struct * to_wake = NULL;
				int i;

				response = (data_void_response_t*) inc_msg;
				fetched_data = find_mapping_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

#if STATISTICS
				answer_request_void++;
#endif

				PSPRINTK(
						"answer_request_void %i address %lu from cpu %i. This is a void response.\n", answer_request_void, response->address, inc_msg->hdr.from_cpu);

				PSMINPRINTK("received answer address %lu void from cpu %i\n", response->address, inc_msg->hdr.from_cpu);

				if (fetched_data == NULL) {
					PSPRINTK("data not found in local list\n");
					pcn_kmsg_free_msg(inc_msg);
					return -1;

				}

				raw_spin_lock_irqsave(&(fetched_data->lock), flags);

				if (response->fetching == 1) {
					PSPRINTK("Response with fetching flag set\n");
					fetched_data->fetching = 1;
					fetched_data->owners[response->header.from_cpu] = 1;
				}

				if (response->vma_present == 1) {

#if NOT_REPLICATED_VMA_MANAGEMENT
					if (response->header.from_cpu != response->tgroup_home_cpu)
						printk(
								"ERROR: a kernel that is not the server is sending the mapping\n");
#endif
					if (fetched_data->vma_present == 0) {
						PSPRINTK("Set vma\n");
						fetched_data->vma_present = 1;
						fetched_data->vaddr_start = response->vaddr_start;
						fetched_data->vaddr_size = response->vaddr_size;
						fetched_data->prot = response->prot;
						fetched_data->pgoff = response->pgoff;
						fetched_data->vm_flags = response->vm_flags;
						strcpy(fetched_data->path, response->path);
					}
#if NOT_REPLICATED_VMA_MANAGEMENT
					else
						printk("ERROR: received more than one mapping\n");
#endif
				}

				out:

				for (i = 0; i < MAX_KERNEL_IDS; i++) {
					fetched_data->owners[i] = fetched_data->owners[i] | response->owners[i];
				}

				fetched_data->responses++;

				if (fetched_data->responses >= fetched_data->expected_responses)
					to_wake = fetched_data->waiting;

				raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);

				if (to_wake != NULL)
					wake_up_process(to_wake);

				pcn_kmsg_free_msg(inc_msg);

				return 1;

#endif

			}

			static int handle_mapping_response(struct pcn_kmsg_message* inc_msg) {

#if FOR_2_KERNELS
				data_response_for_2_kernels_t* response;
				mapping_answers_for_2_kernels_t* fetched_data;
				int set = 0;

				response = (data_response_for_2_kernels_t*) inc_msg;
				fetched_data = find_mapping_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

				//printk("sizeof(data_response_for_2_kernels_t) %d PAGE_SIZE %d response->data_size %d \n",sizeof(data_response_for_2_kernels_t),PAGE_SIZE,response->data_size);
#if STATISTICS
				answer_request++;
#endif

				PSPRINTK("Answer_request %i address %lu from cpu %i \n", answer_request, response->address, inc_msg->hdr.from_cpu);
				PSMINPRINTK("Received answer for address %lu last write %d from cpu %i\n", response->address, response->last_write,inc_msg->hdr.from_cpu);

				if (fetched_data == NULL) {
					PSPRINTK("data not found in local list\n");
					pcn_kmsg_free_msg(inc_msg);
					return -1;

				}

#if CHECKSUM
				__wsum check= csum_partial(&response->data, PAGE_SIZE, 0);
				if(check!=response->checksum)
					printk("Checksum sent: %i checksum computed %i\n",response->checksum,check);
#endif

				if (response->vma_present == 1) {

#if NOT_REPLICATED_VMA_MANAGEMENT
					if (response->header.from_cpu != response->tgroup_home_cpu)
						printk(
								"ERROR: a kernel that is not the server is sending the mapping\n");
#endif
 PSPRINTK("response->vma_pesent %d reresponse->vaddr_start %lu response->vaddr_size %lu response->prot %lu response->vm_flags %lu response->pgoff %lu response->path %s response->fowner %d\n",
response->vma_present, response->vaddr_start , response->vaddr_size,response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);

					if (fetched_data->vma_present == 0) {
						PSPRINTK("Set vma\n");
						fetched_data->vma_present = 1;
						fetched_data->vaddr_start = response->vaddr_start;
						fetched_data->vaddr_size = response->vaddr_size;
						fetched_data->prot = response->prot;
						fetched_data->pgoff = response->pgoff;
						fetched_data->vm_flags = response->vm_flags;
						strcpy(fetched_data->path, response->path);
					}
#if NOT_REPLICATED_VMA_MANAGEMENT
					else
						printk("ERROR: received more than one mapping\n");
#endif
				}

				if (response->owner == 1) {
					PSPRINTK("Response with ownership\n");
					fetched_data->owner = 1;
				}


				if (fetched_data->address_present == 1) {
					printk("ERROR: received more than one answer with a copy of the page\n");

				} else  {
					fetched_data->address_present= 1;
					fetched_data->data = response;
					fetched_data->last_write = response->last_write;
					set = 1;
				}

				if(fetched_data->arrived_response!=0)
					printk("ERROR: received more than one answer, arrived_response is %d \n",fetched_data->arrived_response);

				fetched_data->owners[inc_msg->hdr.from_cpu] = 1;

				fetched_data->arrived_response++;

		fetched_data->futex_owner = response->futex_owner;

				wake_up_process(fetched_data->waiting);

				if (set == 0)
					pcn_kmsg_free_msg(inc_msg);

#else
				data_response_t* response;
				mapping_answers_t* fetched_data;
				int i;
				int set = 0;
				unsigned long flags;
				struct task_struct* to_wake = NULL;

				response = (data_response_t*) inc_msg;
				fetched_data = find_mapping_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

#if STATISTICS
answer_request++;
#endif

PSPRINTK(
		"Answer_request %i address %lu from cpu %i data present?%i\n", answer_request, response->address, inc_msg->hdr.from_cpu, response->address_present);

if (fetched_data == NULL) {
	PSPRINTK("data not found in local list\n");
	pcn_kmsg_free_msg(inc_msg);
	return -1;

}

raw_spin_lock_irqsave(&(fetched_data->lock), flags);

#if CHECKSUM
__wsum check= csum_partial(&response->data, PAGE_SIZE, 0);
if(check!=response->checksum)
	printk("Checksum sent: %i checksum computed %i\n",response->checksum,check);
#endif

if (response->vma_present == 1) {

#if NOT_REPLICATED_VMA_MANAGEMENT
	if (response->header.from_cpu != response->tgroup_home_cpu)
		printk(
				"ERROR: a kernel that is not the server is sending the mapping\n");
#endif
	if (fetched_data->vma_present == 0) {
		PSPRINTK("Set vma\n");
		fetched_data->vma_present = 1;
		fetched_data->vaddr_start = response->vaddr_start;
		fetched_data->vaddr_size = response->vaddr_size;
		fetched_data->prot = response->prot;
		fetched_data->pgoff = response->pgoff;
		fetched_data->vm_flags = response->vm_flags;
		strcpy(fetched_data->path, response->path);
	}
#if NOT_REPLICATED_VMA_MANAGEMENT
	else
		printk("ERROR: received more than one mapping\n");
#endif
}

if (response->address_present == REPLICATION_STATUS_VALID
		|| response->address_present == REPLICATION_STATUS_WRITTEN) {


	PSMINPRINTK("received answer address %lu last write %i from cpu %i\n", response->address, response->last_write,inc_msg->hdr.from_cpu);

	if (fetched_data->address_present == REPLICATION_STATUS_INVALID) {
		PSPRINTK(
				"Copy page, last write on this copy is: %lu\n", fetched_data->last_write);
		fetched_data->address_present = response->address_present;
		fetched_data->data = response;
		fetched_data->last_write = response->last_write;
		set = 1;

	} else if (response->last_write > fetched_data->last_write) {
		PSPRINTK(
				"Substituting copy page, last write on this copy is: %lu\n", fetched_data->last_write);
		pcn_kmsg_free_msg(fetched_data->data);
		fetched_data->data = response;
		fetched_data->last_write = response->last_write;
		set = 1;
	}
}

for (i = 0; i < MAX_KERNEL_IDS; i++) {
	fetched_data->owners[i] = fetched_data->owners[i] | response->owners[i];
}

fetched_data->responses++;

if (fetched_data->responses >= fetched_data->expected_responses)
	to_wake = fetched_data->waiting;

raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);

if (to_wake != NULL)
	wake_up_process(to_wake);

if (set == 0)
	pcn_kmsg_free_msg(inc_msg);
#endif
return 1;
			}

			static int handle_ack(struct pcn_kmsg_message* inc_msg) {

#if FOR_2_KERNELS
				ack_t* response;
				ack_answers_for_2_kernels_t* fetched_data;

				response = (ack_t*) inc_msg;
				fetched_data = find_ack_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

#if STATISTICS
				ack++;
#endif
				PSPRINTK(
						"Answer_invalid %i address %lu from cpu %i \n", ack, response->address, inc_msg->hdr.from_cpu);

				if (fetched_data == NULL) {
					goto out;
				}

				fetched_data->response_arrived++;

				if(fetched_data->response_arrived>1)
					printk("ERROR: received more than one ack\n");

				wake_up_process(fetched_data->waiting);

				out: pcn_kmsg_free_msg(inc_msg);
#else

				ack_t* response;
				ack_answers_t* fetched_data;
				unsigned long flags;
				struct task_struct* to_wake = NULL;

				response = (ack_t*) inc_msg;
				fetched_data = find_ack_entry(response->tgroup_home_cpu,
						response->tgroup_home_id, response->address);

#if STATISTICS
ack++;
#endif
PSPRINTK(
		"Answer_invalid %i address %lu from cpu %i ack?%i concurrent?%i\n", ack, response->address, inc_msg->hdr.from_cpu, response->ack, (response->writing==0)?0:1);

if (fetched_data == NULL) {
	goto out;
}

raw_spin_lock_irqsave(&(fetched_data->lock), flags);

if (response->writing == 1) {
	(fetched_data->concurrent)++;
	fetched_data->writers[inc_msg->hdr.from_cpu] = 1;
	if (response->time_stamp < fetched_data->time_stamp) {
		fetched_data->time_stamp = response->time_stamp;
		fetched_data->owner = inc_msg->hdr.from_cpu;
	} else if (response->time_stamp == fetched_data->time_stamp)
		if (inc_msg->hdr.from_cpu < fetched_data->owner) {
			fetched_data->time_stamp = response->time_stamp;
			fetched_data->owner = inc_msg->hdr.from_cpu;
		}

}
if (response->ack == 0) {
	fetched_data->nack++;
}

fetched_data->responses++;

if (fetched_data->responses >= fetched_data->expected_responses)
	to_wake = fetched_data->waiting;

raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);

if (to_wake != NULL)
	wake_up_process(to_wake);

out: pcn_kmsg_free_msg(inc_msg);
#endif
return 0;
			}

#if FOR_2_KERNELS
			void process_invalid_request_for_2_kernels(struct work_struct* work){
				invalid_work_t* work_request = (invalid_work_t*) work;
				invalid_data_for_2_kernels_t* data = work_request->request;
				ack_t* response;
				memory_t* memory = NULL;
				struct mm_struct* mm = NULL;
				struct vm_area_struct* vma;
				unsigned long address = data->address & PAGE_MASK;
				int from_cpu = data->header.from_cpu;
				pgd_t* pgd;
				pud_t* pud;
				pmd_t* pmd;
				pte_t* pte;
				pte_t entry;
				struct page* page;
				spinlock_t *ptl;
				int lock = 0;

				//unsigned long long start,end;

				invalid_work_t *delay;

#if STATISTICS
invalid++;
#endif

PSPRINTK("Invalid %i address %lu from cpu %i\n", invalid, data->address, from_cpu);

PSMINPRINTK("Invalid for address %lu from cpu %i\n",data->address, from_cpu);

//start= native_read_tsc();

response= (ack_t*) kmalloc(sizeof(ack_t), GFP_ATOMIC);
if(response==NULL){
	pcn_kmsg_free_msg(data);
	kfree(work);
	return;
}
response->writing = 0;

memory = find_memory_entry(data->tgroup_home_cpu, data->tgroup_home_id);
if (memory != NULL) {
	if(memory->setting_up==1){
		goto out;
	}
	mm = memory->mm;
} else {
	goto out;
}

down_read(&mm->mmap_sem);

//check the vma era first
if(mm->vma_operation_index < data->vma_operation_index){

	printk("different era invalid\n");
	delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

	if (delay!=NULL) {
		delay->request = data;
		INIT_DELAYED_WORK( (struct delayed_work*)delay,
				process_invalid_request_for_2_kernels);
		queue_delayed_work(invalid_message_wq,
				(struct delayed_work*) delay, 10);
	}

	up_read(&mm->mmap_sem);
	kfree(work);
	return;
}

// check if there is a valid vma
vma = find_vma(mm, address);
if (!vma || address >= vma->vm_end || address < vma->vm_start) {
	vma = NULL;
} else {

	if (unlikely(is_vm_hugetlb_page(vma))
			|| unlikely(transparent_hugepage_enabled(vma))) {
		printk("Request for HUGE PAGE vma\n");
		up_read(&mm->mmap_sem);
		goto out;
	}

}

pgd = pgd_offset(mm, address);
if (!pgd || pgd_none(*pgd)) {
	up_read(&mm->mmap_sem);
	goto out;
}
pud = pud_offset(pgd, address);
if (!pud || pud_none(*pud)) {
	up_read(&mm->mmap_sem);
	goto out;
}
pmd = pmd_offset(pud, address);
if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
	up_read(&mm->mmap_sem);
	goto out;
}

pte = pte_offset_map_lock(mm, pmd, address, &ptl);

/*PTE LOCKED*/

lock = 1;

//case pte not yet installed
if (pte == NULL || pte_none(pte_clear_flags(*pte,_PAGE_UNUSED1)) ) {

	PSPRINTK("pte not yet mapped \n");

	//If I receive an invalid while it is not mapped, I must be fetching the page.
	//Otherwise it is an error.
	//Delay the invalid while I install the page.

	//Check if I am concurrently fetching the page
	mapping_answers_for_2_kernels_t* fetched_data = find_mapping_entry(
			data->tgroup_home_cpu, data->tgroup_home_id, address);

	if (fetched_data != NULL) {
		PSPRINTK("Concurrently fetching the same address\n");

		if(fetched_data->is_fetch!=1)
			printk("ERROR: invalid received for a not mapped pte that has a mapping_answer not in fetching\n");

		delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

		if (delay!=NULL) {
			delay->request = data;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					process_invalid_request_for_2_kernels);
			queue_delayed_work(invalid_message_wq,
					(struct delayed_work*) delay, 10);
		}
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		kfree(work);
		return;
	}
	else
		printk("ERROR: received an invalid for a not mapped pte not in fetching status\n");

	goto out;

} else {

	//the "standard" page fault releases the pte lock after that it installs the page
	//so before that I lock the pte again there is a moment in which is not null
	//but still fetching
	if (memory->alive != 0) {
		mapping_answers_for_2_kernels_t* fetched_data = find_mapping_entry(
				data->tgroup_home_cpu, data->tgroup_home_id, address);

		if(fetched_data!=NULL && fetched_data->is_fetch==1){

			printk("OCCHIO...beccato....\n");

			delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = data;
				INIT_DELAYED_WORK( (struct delayed_work*)delay,
						process_invalid_request_for_2_kernels);
				queue_delayed_work(invalid_message_wq,
						(struct delayed_work*) delay, 10);
			}
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;
		}
	}

	page = pte_page(*pte);
	if (page != vm_normal_page(vma, address, *pte)) {
		PSPRINTK("page different from vm_normal_page in request page\n");
	}
	if (page->replicated == 0 || page->status==REPLICATION_STATUS_NOT_REPLICATED) {
		printk("ERROR: Invalid message in not replicated page.\n");
		goto out;
	}

	if (page->status == REPLICATION_STATUS_WRITTEN) {
		printk("ERROR: invalid message in a written page\n");
		goto out;
	}

	if(page->reading==1){
		/*If I am reading my current status must be invalid and the one of the other kernel must be written.
		 *After that he sees my request of page, it mights want to write again and it sends me an invalid.
		 *So this request must be delayed.
		 */
		//printk("OCCHIO... page reading when received invalid\n");

		if(page->status!=REPLICATION_STATUS_INVALID || page->last_write!=(data->last_write-1))
			printk("Incorrect invalid received while reading address %lu, my status is %d, page last write %lu, invalid for version %lu",
					address,page->status,page->last_write,data->last_write);

		delay = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

		if (delay!=NULL) {
			delay->request = data;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					process_invalid_request_for_2_kernels);
			queue_delayed_work(invalid_message_wq,
					(struct delayed_work*) delay, 10);
		}
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		kfree(work);
		return;
	}

	if(page->writing==1){
		/*Concurrent write.
		 *To be correct I must be or in valid or invalid state and not owner.
		 *The kernel with the ownership always wins.
		 */
		response->writing=1;
		if(page->owner==1 || page->status==REPLICATION_STATUS_WRITTEN)
			printk("Incorrect invalid received while writing address %lu, my status is %d, page last write %lu, invalid for version %lu page owner %d",
					address,page->status,page->last_write,data->last_write,page->owner);

		//printk("OCCHIO...received invalid while writing\n");
	}

	if(page->last_write!= data->last_write)
		printk("ERROR: received an invalid for copy %lu but my copy is %lu\n",data->last_write,page->last_write);

	page->status = REPLICATION_STATUS_INVALID;
	page->owner = 0;

	flush_cache_page(vma, address, pte_pfn(*pte));

	entry = *pte;
	//the page is invalid so as not present
	entry = pte_clear_flags(entry, _PAGE_PRESENT);
	entry = pte_set_flags(entry, _PAGE_ACCESSED);

	ptep_clear_flush(vma, address, pte);

	set_pte_at_notify(mm, address, pte, entry);

	update_mmu_cache(vma, address, pte);
	//flush_tlb_page(vma, address);
	//flush_tlb_fix_spurious_fault(vma, address);

}

out: if (lock) {
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
}

response->header.type = PCN_KMSG_TYPE_PROC_SRV_ACK_DATA;
response->header.prio = PCN_KMSG_PRIO_NORMAL;
response->tgroup_home_cpu = data->tgroup_home_cpu;
response->tgroup_home_id = data->tgroup_home_id;
response->address = data->address;
response->ack = 1;
//pcn_kmsg_send(from_cpu, (struct pcn_kmsg_message*) (response));
pcn_kmsg_send_long(from_cpu,(struct pcn_kmsg_long_message*) (response),sizeof(ack_t)-sizeof(struct pcn_kmsg_hdr));
kfree(response);
pcn_kmsg_free_msg(data);
kfree(work);

			}

#else

			void process_invalid_request(struct work_struct* work) {
				invalid_work_t* work_request = (invalid_work_t*) work;
				invalid_data_t* data = work_request->request;
				ack_t* response;
				memory_t* memory = NULL;
				struct mm_struct* mm = NULL;
				struct vm_area_struct* vma;
				unsigned long address = data->address & PAGE_MASK;
				int from_cpu = data->header.from_cpu;
				pgd_t* pgd;
				pud_t* pud;
				pmd_t* pmd;
				pte_t* pte;
				pte_t entry;
				struct page* page;
				spinlock_t *ptl;
				int ack = 1;
				int lock = 0;
				char lpath[512];
				//unsigned long long start,end;

				invalid_work_t *delay;

#if STATISTICS
invalid++;
#endif

PSPRINTK("Invalid %i address %lu from cpu %i\n", invalid, data->address, from_cpu);

PSMINPRINTK("Invalid %i address %lu from cpu %i\n", invalid, data->address, from_cpu);

response = (ack_t*) kmalloc(sizeof(ack_t),GPF_ATOMIC);
if(response==NULL){
	pcn_kmsg_free_msg(data);
	kfree(work);
	return;
}
response->writing = 0;
//start= native_read_tsc();

memory = find_memory_entry(data->tgroup_home_cpu, data->tgroup_home_id);
if (memory != NULL) {
	if(memory->setting_up==1){
		goto out;
	}

	mm = memory->mm;
} else {
	goto out;
}

down_read(&mm->mmap_sem);

//check the vma era first
if(mm->vma_operation_index < data->vma_operation_index){

	delay = kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

	if (delay) {
		delay->request = data;
		INIT_DELAYED_WORK( (struct delayed_work*)delay,
				process_invalid_request);
		queue_delayed_work(invalid_message_wq,
				(struct delayed_work*) delay, 10);
	}

	up_read(&mm->mmap_sem);
	kfree(work);
	return;
}

// check if there is a valid vma
vma = find_vma(mm, address);
if (!vma || address >= vma->vm_end || address < vma->vm_start) {
	vma = NULL;
} else {

	if (unlikely(is_vm_hugetlb_page(vma))
			|| unlikely(transparent_hugepage_enabled(vma))) {
		printk("Request for HUGE PAGE vma\n");
		up_read(&mm->mmap_sem);
		goto out;
	}

}

pgd = pgd_offset(mm, address);
if (!pgd || pgd_none(*pgd)) {
	up_read(&mm->mmap_sem);
	goto out;
}
pud = pud_offset(pgd, address);
if (!pud || pud_none(*pud)) {
	up_read(&mm->mmap_sem);
	goto out;
}
pmd = pmd_offset(pud, address);
if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
	up_read(&mm->mmap_sem);
	goto out;
}

pte = pte_offset_map_lock(mm, pmd, address, &ptl);

/*PTE LOCKED*/

lock = 1;

//case pte not yet installed
if (pte == NULL || pte_none(*pte)) {

	PSPRINTK("pte not mapped \n");

	if (memory->alive != 0) {
		//Check if I am concurrently fetching the page
		mapping_answers_t* fetched_data = find_mapping_entry(
				data->tgroup_home_cpu, data->tgroup_home_id, address);

		if (fetched_data != NULL) {
			PSPRINTK("Concurrently fetching the same address\n");
			unsigned long flags;
			raw_spin_lock_irqsave(&(fetched_data->lock), flags);
			if (data->last_write > fetched_data->last_invalid) {
				fetched_data->last_invalid = data->last_write;
				fetched_data->owner = from_cpu;
			}
			fetched_data->owners[from_cpu] = 1;
			raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);

		}

	}

	goto out;
} else {

	page = pte_page(*pte);
	if (page != vm_normal_page(vma, address, *pte)) {
		PSPRINTK("page different from vm_normal_page in request page\n");
	}
	if (!(page->replicated == 1)) {
		printk("Invalid message in not replicated page.\n");
		goto out;
	}

	/* During concurrent writes there is the possibility that after one write succeed
	 * it receives the invalid of the concurrent ones.
	 * Only in this case a written status can receive an invalid message.
	 * The answer must be a nack.
	 */

	if (page->status == REPLICATION_STATUS_WRITTEN) {
		ack = 0;
		response->writing = 1;
		response->time_stamp = page->time_stamp;
		PSPRINTK("Invalid message in written page\n");
	}

	/* If I am writing too the write corresponding to this invalidation
	 * is concurrent with my write.
	 * One must be aborted and retry.
	 */
	if (page->writing == 1) {

		/* To choose which concurrent write can continue, a time stamp comparison method is used.
		 * The write with the smaller time stamp wins. If the time stamps are equals, the write with the small cpu wins.
		 * There is no a global clock shared by kernels. However each clock is monotone.
		 * The time stamp of a write is chosen at the first retry, so eventually this time stamp will became the smaller.
		 */

		if (data->time_stamp < page->time_stamp) {
			ack = 1;
		} else if (data->time_stamp == page->time_stamp) {
			if (from_cpu < _cpu) {
				ack = 1;
			} else
				ack = 0;
		} else
			ack = 0;

		response->writing = 1;
		response->time_stamp = page->time_stamp;

	}

	if (ack == 1) {
		/* I have to invalidate the page and save the information of the last write.
		 */
		PSPRINTK("ack =1\n");
		page->status = REPLICATION_STATUS_INVALID;
		page->owner = from_cpu;

		if (page->reading == 1) {

			mapping_answers_t* fetched_data = find_mapping_entry(
					data->tgroup_home_cpu, data->tgroup_home_id, address);

			if (fetched_data != NULL) {
				unsigned long flags;
				raw_spin_lock_irqsave(&(fetched_data->lock), flags);
				if (data->last_write > fetched_data->last_invalid) {
					fetched_data->last_invalid = data->last_write;
				}
				raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);
			} else {
				printk(
						"ERROR: Received invalid message, no concurrent read but flag's page is reading\n");
			}
		}

		flush_cache_page(vma, address, pte_pfn(*pte));

		entry = *pte;
		//the page is invalid so as not present
		entry = pte_clear_flags(entry, _PAGE_PRESENT);
		entry = pte_set_flags(entry, _PAGE_ACCESSED);

		ptep_clear_flush(vma, address, pte);
		set_pte_at_notify(mm, address, pte, entry);

		update_mmu_cache(vma, address, pte);
		//flush_tlb_page(vma, address);
		//	flush_tlb_fix_spurious_fault(vma, address);

	}

}

out: if (lock) {
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
}

response->header.type = PCN_KMSG_TYPE_PROC_SRV_ACK_DATA;
response->header.prio = PCN_KMSG_PRIO_NORMAL;
response->tgroup_home_cpu = data->tgroup_home_cpu;
response->tgroup_home_id = data->tgroup_home_id;
response->address = data->address;
response->ack = ack;
pcn_kmsg_send(from_cpu, (struct pcn_kmsg_message*) (response));

kfree(response);
pcn_kmsg_free_msg(data);
kfree(work);

			}
#endif

			static int handle_invalid_request(struct pcn_kmsg_message* inc_msg) {

#if FOR_2_KERNELS
				invalid_work_t* request_work;
				invalid_data_for_2_kernels_t* data = (invalid_data_for_2_kernels_t*) inc_msg;

				request_work = (invalid_work_t*)kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

				if (request_work!=NULL) {
					request_work->request = data;
					INIT_WORK( (struct work_struct*)request_work, process_invalid_request_for_2_kernels);
					queue_work(invalid_message_wq, (struct work_struct*) request_work);
				}
#else
	invalid_work_t* request_work;
	invalid_data_t* data = (invalid_data_t*) inc_msg;

	request_work = (invalid_work_t*) kmalloc(sizeof(invalid_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = data;
		INIT_WORK( (struct work_struct*)request_work, process_invalid_request);
		queue_work(invalid_message_wq, (struct work_struct*) request_work);
	}
#endif
return 1;

			}



extern int do_wp_page_for_popcorn(struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long address, pte_t *page_table, pmd_t *pmd,
		spinlock_t *ptl, pte_t orig_pte);


#if FOR_2_KERNELS

			void process_mapping_request_for_2_kernels(struct work_struct* work) {

				request_work_t* request_work = (request_work_t*) work;
				data_request_for_2_kernels_t* request = request_work->request;
				memory_t * memory;
				struct mm_struct* mm = NULL;
				struct vm_area_struct* vma = NULL;
				data_void_response_for_2_kernels_t* void_response;
				int owner= 0;
				char* plpath;
				char lpath[512];
				int from_cpu = request->header.from_cpu;
				unsigned long address = request->address & PAGE_MASK;
				pgd_t* pgd;
				pud_t* pud;
				pmd_t* pmd;
				pte_t* pte;
				pte_t entry;
				spinlock_t* ptl;
				request_work_t* delay;
				struct page* page, *old_page;
				data_response_for_2_kernels_t* response;
				mapping_answers_for_2_kernels_t* fetched_data;
				int lock =0;
				void *vfrom;


#if STATISTICS
request_data++;
#endif

	PSMINPRINTK("Request for address %lu is fetch %i is write %i\n", request->address,((request->is_fetch==1)?1:0),((request->is_write==1)?1:0));
	PSPRINTK("Request %i address %lu is fetch %i is write %i\n", request_data, request->address,((request->is_fetch==1)?1:0),((request->is_write==1)?1:0));

memory = find_memory_entry(request->tgroup_home_cpu,
		request->tgroup_home_id);
if (memory != NULL) {
	if(memory->setting_up==1){
		owner=1;
		goto out;
	}
	mm = memory->mm;
} else {
	owner=1;
	goto out;
}

down_read(&mm->mmap_sem);

//check the vma era first
if(mm->vma_operation_index < request->vma_operation_index){
	printk("different era request mm->vma_operation_index %d request->vma_operation_index %d\n",mm->vma_operation_index,request->vma_operation_index);
	delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);

	if (delay) {
		delay->request = request;
		INIT_DELAYED_WORK( (struct delayed_work*)delay,
				process_mapping_request_for_2_kernels);
		queue_delayed_work(message_request_wq,
				(struct delayed_work*) delay, 10);
	}

	up_read(&mm->mmap_sem);
	kfree(work);
	return;
}

// check if there is a valid vma
vma = find_vma(mm, address);
if (!vma || address >= vma->vm_end || address < vma->vm_start) {
	vma = NULL;
		if(_cpu == request->tgroup_home_cpu){
			printk(KERN_ALERT"%s:OCCHIO vma NULL in xeon address{%lu} \n",__func__,address);
			up_read(&mm->mmap_sem);
			goto out;
		}
} else {

	if (unlikely(is_vm_hugetlb_page(vma))
			|| unlikely(transparent_hugepage_enabled(vma))) {
		printk("ERROR: Request for HUGE PAGE vma\n");
		up_read(&mm->mmap_sem);
		goto out;
	}

	PSPRINTK(
			"Find vma from %s start %lu end %lu\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"), vma->vm_start, vma->vm_end);

}


if(_cpu!=request->tgroup_home_cpu){

	pgd = pgd_offset(mm, address);
	if (!pgd || pgd_none(*pgd)) {
		up_read(&mm->mmap_sem);
		goto out;
	}

	pud = pud_offset(pgd, address);
	if (!pud || pud_none(*pud)) {
		up_read(&mm->mmap_sem);
		goto out;
	}

	pmd = pmd_offset(pud, address);

	if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
		up_read(&mm->mmap_sem);
		goto out;
	}
}
else{

	pgd = pgd_offset(mm, address);

	pud = pud_alloc(mm, pgd, address);
	if (!pud){
		up_read(&mm->mmap_sem);
		goto out;
	}

	pmd = pmd_alloc(mm, pud, address);
	if (!pmd){
		up_read(&mm->mmap_sem);
		goto out;
	}

	if (pmd_none(*pmd) && __pte_alloc(mm, vma, pmd, address)){
		up_read(&mm->mmap_sem);
		goto out;
	}

	if (unlikely(pmd_trans_huge(*pmd))) {
		printk("ERROR: request for huge page\n");
		up_read(&mm->mmap_sem);
		goto out;
	}

}

retry: pte = pte_offset_map_lock(mm, pmd, address, &ptl);
/*PTE LOCKED*/

entry = *pte;
lock= 1;


if (pte == NULL || pte_none(pte_clear_flags(entry, _PAGE_UNUSED1))) {

	PSPRINTK("pte not mapped \n");

	if( !pte_none(entry) ){

		if(_cpu!=request->tgroup_home_cpu || request->is_fetch==1){
			printk("ERROR: incorrect request for marked page\n");
			goto out;
		}
		else{
			PSPRINTK("request for a marked page\n");
		}
	}

	if ((_cpu==request->tgroup_home_cpu) || memory->alive != 0) {

		fetched_data = find_mapping_entry(
				request->tgroup_home_cpu, request->tgroup_home_id, address);

		//case concurrent fetch
		if (fetched_data != NULL) {

			fetch:				PSPRINTK("concurrent request\n");

			/*Whit marked pages only two scenarios can happenn:
			 * Or I am the main and I an locally fetching=> delay this fetch
			 * Or I am not the main, but the main already answer to my fetch (otherwise it will not answer to me the page)
			 * so wait that the answer arrive before consuming the fetch.
			 * */
			if (fetched_data->is_fetch != 1)
				printk(
						"ERROR: find a mapping_answers_for_2_kernels_t not mapped and not fetch\n");

			delay = (request_work_t*)kmalloc(sizeof(request_work_t),
					GFP_ATOMIC);

			if (delay!=NULL) {
				delay->request = request;
				INIT_DELAYED_WORK(
						(struct delayed_work*)delay,
						process_mapping_request_for_2_kernels);
				queue_delayed_work(message_request_wq,
						(struct delayed_work*) delay, 10);
			}

			spin_unlock(ptl);
			up_read(&mm->mmap_sem);
			kfree(work);
			return;

		}

		else{
			//mark the pte if main
			if(_cpu==request->tgroup_home_cpu){

				PSPRINTK(KERN_ALERT"%s: marking a pte for address %lu \n",__func__,address);

				entry = pte_set_flags(entry, _PAGE_UNUSED1);

				ptep_clear_flush(vma, address, pte);

				set_pte_at_notify(mm, address, pte, entry);
				//in x86 does nothing
				update_mmu_cache(vma, address, pte);

			}
		}
	}
	//pte not present
	owner= 1;
	goto out;

}

page = pte_page(entry);
if (page != vm_normal_page(vma, address, entry)) {
	PSPRINTK("Page different from vm_normal_page in request page\n");
}
old_page = NULL;

if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {

	PSPRINTK("Page not replicated\n");

	/*There is the possibility that this request arrived while I am fetching, after that I installed the page
	 * but before calling the update page....
	 * */
	if (memory->alive != 0) {
		fetched_data = find_mapping_entry(
				request->tgroup_home_cpu, request->tgroup_home_id, address);

		if(fetched_data!=NULL){
			printk("OCCHIO...beccato....\n");
			goto fetch;
		}
	}

	//the request must be for a fetch
	if(request->is_fetch==0)
		printk("ERROR received a request not fetch for a not replicated page\n");

	if (vma->vm_flags & VM_WRITE) {

		//if the page is writable but the pte has not the write flag set, it is a cow page
		if (!pte_write(entry)) {

retry_cow:
			PSPRINTK("COW page at %lu \n", address);

			int ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);

			if (ret & VM_FAULT_ERROR) {
				if (ret & VM_FAULT_OOM){
					printk("ERROR: %s VM_FAULT_OOM\n",__func__);
					up_read(&mm->mmap_sem);
					goto out;
				}

				if (ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
					printk("ERROR: %s EHWPOISON\n",__func__);
					up_read(&mm->mmap_sem);
					goto out;
				}

				if (ret & VM_FAULT_SIGBUS){
					printk("ERROR: %s EFAULT\n",__func__);
					up_read(&mm->mmap_sem);
					goto out;
				}

				printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
				up_read(&mm->mmap_sem);
				goto out;
			}

			spin_lock(ptl);
			/*PTE LOCKED*/
			lock = 1;

			entry = *pte;

			if(!pte_write(entry)){
				printk("WARNING: page not writable after cow\n");
				goto retry_cow;
			}

			page = pte_page(entry);

		}
		//printk(KERN_ALERT"UUUCHIO \n");

		page->replicated = 1;

		flush_cache_page(vma, address, pte_pfn(*pte));
		entry = mk_pte(page, vma->vm_page_prot);

		if(request->is_write==0){
			//case fetch for read
			page->status = REPLICATION_STATUS_VALID;
			entry = pte_clear_flags(entry, _PAGE_RW);
			entry = pte_set_flags(entry, _PAGE_PRESENT);
			owner= 0;
			page->owner= 1;
		}
		else{
			//case fetch for write
			page->status = REPLICATION_STATUS_INVALID;
			entry = pte_clear_flags(entry, _PAGE_PRESENT);
			owner= 1;
			page->owner= 0;
		}

		page->last_write= 1;

		entry = pte_set_flags(entry, _PAGE_USER);
		entry = pte_set_flags(entry, _PAGE_ACCESSED);

		ptep_clear_flush(vma, address, pte);

		set_pte_at_notify(mm, address, pte, entry);
		//printk(KERN_ALERT"ZZZZCHIO smp{%d} comm{%s} tick(%lx} for address{%d}\n",smp_processor_id(),current->comm,native_read_tsc(),address);

		//in x86 does nothing
		update_mmu_cache(vma, address, pte);

		//printk(KERN_ALERT"???????HIO smp{%d} comm{%s} tick(%lx} for address{%d}\n",smp_processor_id(),current->comm,native_read_tsc(),address);
	if (old_page != NULL){
			page_remove_rmap(old_page);
	}
	} else {
		//read only vma
		page->replicated=0;
		page->status= REPLICATION_STATUS_NOT_REPLICATED;

		if(request->is_write==1){
			printk("ERROR: received a write in a read-only not replicated page\n");
		}
		page->owner= 1;
		owner= 0;
	}

	page->other_owners[_cpu]=1;
	page->other_owners[from_cpu]=1;

	goto resolved;
}
else{
	//replicated page case
	PSPRINTK("Page replicated...\n");

	if(request->is_fetch==1){
		printk("ERROR: received a fetch request in a replicated status\n");
	}

	if(page->writing==1){

		PSPRINTK("Page currently in writing \n");


		if(request->is_write==0){
			PSPRINTK("Concurrent read request\n");
		}
		else{

			PSPRINTK("Concurrent write request\n");
		}
		delay = (request_work_t*)kmalloc(sizeof(request_work_t), GFP_ATOMIC);

		if (delay!=NULL) {
			delay->request = request;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					process_mapping_request_for_2_kernels);
			queue_delayed_work(message_request_wq,
					(struct delayed_work*) delay, 10);
		}

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		kfree(work);
		return;

	}

	if(page->reading==1){

		printk("ERROR: page in reading but received a request\n");
		goto out;
	}

	//invalid page case
	if (page->status == REPLICATION_STATUS_INVALID) {

		printk("ERROR: received a request in invalid status without reading or writing\n");
		goto out;
	}

	//valid page case
	if (page->status == REPLICATION_STATUS_VALID) {

		PSPRINTK("Page requested valid\n");

		if(page->owner!=1)
			printk("ERROR: request in a not owner valid page\n");
		else{
			if(request->is_write){
				if(page->last_write!= request->last_write)
					printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				page->status= REPLICATION_STATUS_INVALID;
				page->owner= 0;
				owner= 1;
				entry = *pte;
				entry = pte_clear_flags(entry, _PAGE_PRESENT);
				entry = pte_set_flags(entry, _PAGE_ACCESSED);

				ptep_clear_flush(vma, address, pte);

				set_pte_at_notify(mm, address, pte, entry);

				update_mmu_cache(vma, address, pte);
			}
			else{
				printk("ERROR: received a read request in valid status\n");
			}
		}

		goto out;
	}

	if (page->status == REPLICATION_STATUS_WRITTEN) {

		PSPRINTK("Page requested in written status\n");

		if(page->owner!=1)
			printk("ERROR: page in written status without ownership\n");
		else{
			if(request->is_write==1){

				if(page->last_write!= (request->last_write+1))
					printk("ERROR: received a write for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				page->status= REPLICATION_STATUS_INVALID;
				page->owner= 0;
				owner= 1;
				entry = *pte;
				entry = pte_clear_flags(entry, _PAGE_PRESENT);
				entry = pte_set_flags(entry, _PAGE_ACCESSED);

				ptep_clear_flush(vma, address, pte);

				set_pte_at_notify(mm, address, pte, entry);

				update_mmu_cache(vma, address, pte);
			}
			else{

				if(page->last_write!= (request->last_write+1))
					printk("ERROR: received an read for copy %lu but my copy is %lu\n",request->last_write,page->last_write);

				page->status = REPLICATION_STATUS_VALID;
				page->owner= 1;
				owner= 0;
				entry = *pte;
				entry = pte_set_flags(entry, _PAGE_PRESENT);
				entry = pte_set_flags(entry, _PAGE_ACCESSED);
				entry = pte_clear_flags(entry, _PAGE_RW);

				ptep_clear_flush(vma, address, pte);

				set_pte_at_notify(mm, address, pte, entry);

				update_mmu_cache(vma, address, pte);
			}
		}

#if DIFF_PAGE
		goto resolved_diff;
#else
		goto resolved;
#endif
	}

}

resolved:

PSPRINTK(
		"Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

PSPRINTK(
		"Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

#if DIFF_PAGE

char *app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
if (app == NULL) {
	printk("Impossible to kmalloc app.\n");
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	pcn_kmsg_free_msg(request);
	kfree(work);
	return;
}

vfrom = kmap_atomic(page, KM_USER0);

unsigned int compressed_byte= WKdm_compress(vfrom,app);

if(compressed_byte<((PAGE_SIZE/10)*9)){

#if STATISTICS
	compressed_page_sent++;
#endif

	kunmap_atomic(vfrom, KM_USER0);
	response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+compressed_byte, GFP_ATOMIC);
	if (response == NULL) {
		printk("Impossible to kmalloc in process mapping request.\n");
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg(request);
		kfree(work);
		kfree(app);
		return;
	}
	memcpy(&(response->data),app,compressed_byte);
	response->data_size= compressed_byte;
	kfree(app);
}
else{

#if STATISTICS
	not_compressed_page++;
#endif

	response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
	if (response == NULL) {
		printk("Impossible to kmalloc in process mapping request.\n");
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg(request);
		kfree(work);
		kfree(app);
		return;
	}
	void* vto = &(response->data);
	copy_page(vto, vfrom);
	kunmap_atomic(vfrom, KM_USER0);
	response->data_size= PAGE_SIZE;
	kfree(app);
}

response->diff=0;

#else
response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
if (response == NULL) {
	printk("Impossible to kmalloc in process mapping request.\n");
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	pcn_kmsg_free_msg(request);
	kfree(work);
	return;
}

void* vto = &(response->data);
vfrom = kmap_atomic(page, KM_USER0);

#if READ_PAGE
int ct=0;
unsigned long _buff[16];

if(address == PAGE_ADDR){
	for(ct=0;ct<8;ct++){
		_buff[ct]=(unsigned long) *(((unsigned long *)vfrom) + ct);
	}
}
#endif

copy_page(vto, vfrom);
kunmap_atomic(vfrom, KM_USER0);

response->data_size= PAGE_SIZE;


#if READ_PAGE
if(address == PAGE_ADDR){
	for(ct=8;ct<16;ct++){
		_buff[ct]=(unsigned long) *((unsigned long*)(&(response->data))+ct-8);
	}
	for(ct=0;ct<16;ct++){
		printk(KERN_ALERT"{%lx} ",_buff[ct]);
	}
}
#endif

#if CHECKSUM
vfrom= kmap_atomic(page, KM_USER0);
__wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
kunmap_atomic(vfrom, KM_USER0);
__wsum check2= csum_partial(&(response->data), PAGE_SIZE, 0);
if(check1!=check2)
	printk("page just copied is not matching, address %lu\n",address);
#endif

#endif

flush_cache_page(vma, address, pte_pfn(*pte));

response->last_write = page->last_write;

response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
response->header.prio = PCN_KMSG_PRIO_NORMAL;
response->tgroup_home_cpu = request->tgroup_home_cpu;
response->tgroup_home_id = request->tgroup_home_id;
response->address = request->address;
response->owner= owner;

		response->futex_owner = (!page) ? 0 : page->futex_owner;//akshay

#if NOT_REPLICATED_VMA_MANAGEMENT
if (_cpu == request->tgroup_home_cpu && vma != NULL)
	//only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	if (vma != NULL)
#endif
#endif
	{

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}
			 PSPRINTK("response->vma_present %d response->vaddr_start %lu response->vaddr_size %lu response->prot %lu response->vm_flags %lu response->pgoff %lu response->path %s response->futex_owner %d\n",
response->vma_present, response->vaddr_start , response->vaddr_size,response->prot, response->vm_flags , response->pgoff, response->path,response->futex_owner);
	}

	else
		response->vma_present = 0;

spin_unlock(ptl);
up_read(&mm->mmap_sem);

#if !DIFF_PAGE
#if CHECKSUM
response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif
#endif

//printk("sizeof(data_response_for_2_kernels_t) %d PAGE_SIZE %d response->data_size %d \n",sizeof(data_response_for_2_kernels_t),PAGE_SIZE,response->data_size);


// Send response
pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		sizeof(data_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr) + response->data_size);

// Clean up incoming messages
pcn_kmsg_free_msg(request);
kfree(work);
kfree(response);
//end= native_read_tsc();
PSPRINTK("Handle request end\n");
return;

#if	DIFF_PAGE

resolved_diff:

if(page->old_page_version==NULL){
	printk("ERROR: no previous version of the page to calculate diff address %lu\n",address);
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	pcn_kmsg_free_msg(request);
	kfree(work);
	return;
}

app= kmalloc(sizeof(char)*PAGE_SIZE*2, GFP_ATOMIC);
if (app == NULL) {
	printk("Impossible to kmalloc app.\n");
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	pcn_kmsg_free_msg(request);
	kfree(work);
	return;
		}

vfrom = kmap_atomic(page, KM_USER0);

compressed_byte= WKdm_diff_and_compress (page->old_page_version, vfrom, app);

if(compressed_byte<((PAGE_SIZE/10)*9)){

#if STATISTICS
	compressed_page_sent++;
#endif

	kunmap_atomic(vfrom, KM_USER0);
	response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+compressed_byte, GFP_ATOMIC);
	if (response == NULL) {
		printk("Impossible to kmalloc in process mapping request.\n");
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg(request);
		kfree(work);
		kfree(app);
		return;
	}
	memcpy(&(response->data),app,compressed_byte);
	response->data_size= compressed_byte;
	kfree(app);
}
else{

#if STATISTICS
	not_compressed_page++;
	not_compressed_diff_page++;
#endif

	response = (data_response_for_2_kernels_t*) kmalloc(sizeof(data_response_for_2_kernels_t)+PAGE_SIZE, GFP_ATOMIC);
	if (response == NULL) {
		printk("Impossible to kmalloc in process mapping request.\n");
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		pcn_kmsg_free_msg(request);
		kfree(work);
		kfree(app);
		return;
	}
	void* vto = &(response->data);
	copy_page(vto, vfrom);
	kunmap_atomic(vfrom, KM_USER0);
	response->data_size= PAGE_SIZE;
	kfree(app);
}

response->diff=1;

PSPRINTK(
		"Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

PSPRINTK(
		"Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

flush_cache_page(vma, address, pte_pfn(*pte));

response->last_write = page->last_write;

		response->futex_owner = page->futex_owner;//akshay

response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
response->header.prio = PCN_KMSG_PRIO_NORMAL;
response->tgroup_home_cpu = request->tgroup_home_cpu;
response->tgroup_home_id = request->tgroup_home_id;
response->address = request->address;
response->owner= owner;

#if NOT_REPLICATED_VMA_MANAGEMENT
if (_cpu == request->tgroup_home_cpu && vma != NULL)
	//only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	if (vma != NULL)
#endif
#endif
	{

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}
	}

	else
		response->vma_present = 0;

spin_unlock(ptl);
up_read(&mm->mmap_sem);

// Send response
pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		sizeof(data_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr) + response->data_size);

// Clean up incoming messages
pcn_kmsg_free_msg(request);
kfree(work);
kfree(response);
//end= native_read_tsc();
PSPRINTK("Handle request end\n");
return;
#endif

out:

PSPRINTK("sending void answer\n");

void_response = (data_void_response_for_2_kernels_t*) kmalloc(
		sizeof(data_void_response_for_2_kernels_t), GFP_ATOMIC);
if (void_response == NULL) {
	if(lock){
		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
	}
	printk("Impossible to kmalloc in process mapping request.\n");
	pcn_kmsg_free_msg(request);
	kfree(work);
	return;
}

void_response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID;
void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
void_response->tgroup_home_cpu = request->tgroup_home_cpu;
void_response->tgroup_home_id = request->tgroup_home_id;
void_response->address = request->address;
void_response->owner=owner;

	void_response->futex_owner = 0;//TODO: page->futex_owner;//akshay


#if NOT_REPLICATED_VMA_MANAGEMENT
if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
	if (vma != NULL)
#endif
#endif
	{
		void_response->vma_present = 1;
		void_response->vaddr_start = vma->vm_start;
		void_response->vaddr_size = vma->vm_end - vma->vm_start;
		void_response->prot = vma->vm_page_prot;
		void_response->vm_flags = vma->vm_flags;
		void_response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			void_response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(void_response->path, plpath);
		}
	} else
		void_response->vma_present = 0;

if(lock){
	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
}
// Send response
pcn_kmsg_send_long(from_cpu,
		(struct pcn_kmsg_long_message*) (void_response),
		sizeof(data_void_response_for_2_kernels_t) - sizeof(struct pcn_kmsg_hdr));

// Clean up incoming messages
pcn_kmsg_free_msg(request);
kfree(void_response);
kfree(work);
//end= native_read_tsc();
PSPRINTK("Handle request end\n");
			}

#else

			void process_mapping_request(struct work_struct* work) {

				request_work_t* request_work = (request_work_t*) work;
				data_request_t* request = request_work->request;

				int from_cpu = request->header.from_cpu;
				unsigned long address = request->address & PAGE_MASK;

				data_response_t* response;
				data_void_response_t* void_response;

				struct mm_struct* mm = NULL;
				struct vm_area_struct* vma = NULL;
				pgd_t* pgd;
				pud_t* pud;
				pmd_t* pmd;
				pte_t* pte;
				pte_t entry;
				spinlock_t* ptl;
				request_work_t* delay;
				struct page* page, *old_page;
				void* vto, *vfrom;
				int i;
				//int wake = 0;
				int fetching = 0;
				char* plpath;
				char lpath[512];
				int app[MAX_KERNEL_IDS];
				memory_t * memory;
				int owners[MAX_KERNEL_IDS];
				//unsigned long long start,end;

#if STATISTICS
				request_data++;
#endif

				PSPRINTK(
						"Request %i address %lu from cpu %i\n", request_data, request->address, from_cpu);

				//start= native_read_tsc();

				memset(owners,0,MAX_KERNEL_IDS*sizeof(int));

				memory = find_memory_entry(request->tgroup_home_cpu,
						request->tgroup_home_id);
				if (memory != NULL) {
					if(memory->setting_up==1){
						goto out;
					}
					mm = memory->mm;
				} else {
					goto out;
				}

				down_read(&mm->mmap_sem);

				//check the vma era first
				if(mm->vma_operation_index < request->vma_operation_index){

					delay = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

					if (delay) {
						delay->request = request;
						INIT_DELAYED_WORK( (struct delayed_work*)delay,
								process_mapping_request);
						queue_delayed_work(message_request_wq,
								(struct delayed_work*) delay, 10);
					}

					up_read(&mm->mmap_sem);
					kfree(work);
					return;
				}

				// check if there is a valid vma
				vma = find_vma(mm, address);
				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					vma = NULL;
				} else {

					if (unlikely(is_vm_hugetlb_page(vma))
							|| unlikely(transparent_hugepage_enabled(vma))) {
						printk("ERROR: Request for HUGE PAGE vma\n");
						up_read(&mm->mmap_sem);
						goto out;
					}

					PSPRINTK(
							"Find vma from %s start %lu end %lu\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"), vma->vm_start, vma->vm_end);

				}

				pgd = pgd_offset(mm, address);
				if (!pgd || pgd_none(*pgd)) {
					up_read(&mm->mmap_sem);
					goto out;
				}

				pud = pud_offset(pgd, address);
				if (!pud || pud_none(*pud)) {
					up_read(&mm->mmap_sem);
					goto out;
				}

				pmd = pmd_offset(pud, address);

				if (!pmd || pmd_none(*pmd) || pmd_trans_huge(*pmd)) {
					up_read(&mm->mmap_sem);
					goto out;
				}

				retry: pte = pte_offset_map_lock(mm, pmd, address, &ptl);
				/*PTE LOCKED*/

				entry = *pte;

				if (pte == NULL || pte_none(entry)) {

					PSPRINTK("pte not mapped \n");

					if (memory->alive != 0) {
						//Check if I am concurrently fetching the page
						mapping_answers_t* fetched_data = find_mapping_entry(
								request->tgroup_home_cpu, request->tgroup_home_id, address);

						if (fetched_data != NULL) {
							unsigned long flags;
							raw_spin_lock_irqsave(&(fetched_data->lock), flags);
							fetched_data->fetching = 1;
							fetched_data->owners[from_cpu] = 1;
							memcpy(owners,fetched_data->owners,MAX_KERNEL_IDS*sizeof(int));
							owners[_cpu]=1;
							raw_spin_unlock_irqrestore(&(fetched_data->lock), flags);
							fetching = 1;
							PSPRINTK("Concurrently fetching the same address\n");
						}

					}

					spin_unlock(ptl);
					up_read(&mm->mmap_sem);
					goto out;

				}

				page = pte_page(entry);
				if (page != vm_normal_page(vma, address, entry)) {
					PSPRINTK("Page different from vm_normal_page in request page\n");
				}
				old_page = NULL;

				/*If the page is not replicated and not read only I have to replicate it.
				 *If nobody previously asked for the page I am the owner=> it is valid
				 *If the page is the zero page, trying to access to page fields give error => check first if it is a zero page
				 */
				if (is_zero_page(pte_pfn(entry)) || !(page->replicated == 1)) {

					PSPRINTK("Page not replicated\n");

					if (vma->vm_flags & VM_WRITE) {

						//if the page is writable but the pte has not the write flag set, it is a cow page
						if (!pte_write(entry)) {
							/*
							 * I unlock because alloc page may go to sleep
							 */
							PSPRINTK("COW page at %lu \n", address);

							spin_unlock(ptl);
							/*PTE UNLOCKED*/

							old_page = page;

							if (unlikely(anon_vma_prepare(vma))) {
								up_read(&mm->mmap_sem);
								goto out;
							}

							if (is_zero_page(pte_pfn(entry))) {

								page = alloc_zeroed_user_highpage_movable(vma, address);
								if (!page) {
									up_read(&mm->mmap_sem);
									goto out;
								}

							} else {

								page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
								if (!page) {
									up_read(&mm->mmap_sem);
									goto out;
								}

								copy_user_highpage(page, old_page, address, vma);
							}

							__SetPageUptodate(page);

							if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
								page_cache_release(page);
								up_read(&mm->mmap_sem);
								goto out;

							}

							spin_lock(ptl);
							/*PTE LOCKED*/

							//if somebody changed the pte
							if (unlikely(!pte_same(*pte, entry))) {

								mem_cgroup_uncharge_page(page);
								page_cache_release(page);
								spin_unlock(ptl);
								goto retry;

							} else {
								page_add_new_anon_rmap(page, vma, address);
#if STATISTICS
								pages_allocated++;
#endif

							}
						}


						page->replicated = 1;
						page->status = REPLICATION_STATUS_VALID;
						page->other_owners[from_cpu] = 1;
						page->other_owners[_cpu] = 1;

						flush_cache_page(vma, address, pte_pfn(*pte));

						entry = mk_pte(page, vma->vm_page_prot);
						//I need to catch the next write access
						entry = pte_clear_flags(entry, _PAGE_RW);
						entry = pte_set_flags(entry, _PAGE_PRESENT);
						entry = pte_set_flags(entry, _PAGE_USER);
						entry = pte_set_flags(entry, _PAGE_ACCESSED);

						ptep_clear_flush(vma, address, pte);

						set_pte_at_notify(mm, address, pte, entry);

						//in x86 does nothing
						update_mmu_cache(vma, address, pte);

						/*according to the cpu this function flushes
						 * or the single address on the tlb
						 * or all the tlb
						 *if SMP it flushes all the others tlb
						 */
						//flush_tlb_page(vma, address);

						//should be same as flush_tlb_page
						//flush_tlb_fix_spurious_fault(vma, address);

						if (old_page != NULL)
							page_remove_rmap(old_page);

					} else {

						page->other_owners[from_cpu] = 1;
						page->other_owners[_cpu] = 1;
					}

					memcpy(owners,page->other_owners,MAX_KERNEL_IDS*sizeof(int));
				}

				//page replicated
				else {
					PSPRINTK("Page replicated...\n");

					if (page->writing == 1) {
						PSPRINTK("Page currently in writing \n");

						//I cannot put this thread on sleep otherwise I cannot consume other messages => re-queue the work
						delay = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

						if (delay) {
							delay->request = request;
							INIT_DELAYED_WORK( (struct delayed_work*)delay,
									process_mapping_request);
							queue_delayed_work(message_request_wq,
									(struct delayed_work*) delay, 10);
						}

						spin_unlock(ptl);
						up_read(&mm->mmap_sem);
						kfree(work);
						return;

					}

					/*if (page->writing == 1 && page->reading == 1) {
		 if (request->read_for_write == 0) {
		 printk("ERROR: Consuming normal fetch in read write\n");
		 }
		 if (page->status != REPLICATION_STATUS_INVALID) {
		 printk("ERROR: Answering in read write with a copy.\n");
		 }
		 (page->concurrent_fetch)++;
		 wake = 1;
		 PSPRINTK("Page in reading for write received a request \n");
		 }*/

					//invalid page case
					if (page->status == REPLICATION_STATUS_INVALID) {
						page->other_owners[from_cpu] = 1;
						memcpy(owners,page->other_owners,MAX_KERNEL_IDS*sizeof(int));
						spin_unlock(ptl);
						up_read(&mm->mmap_sem);
						PSPRINTK("Request in status invalid\n");
						goto out;
					}

					//valid page case
					if (page->status == REPLICATION_STATUS_VALID) {
						page->other_owners[from_cpu] = 1;
						PSPRINTK("Page requested valid\n");
						goto resolved;
					}

					//if it is written I need to change status to avoid to write local the next time
					if (page->status == REPLICATION_STATUS_WRITTEN) {

						PSPRINTK("Page requested in written status\n");
						page->other_owners[from_cpu] = 1;

						if (request->read_for_write == 1) {

							(page->concurrent_fetch)++;
							PSPRINTK("Page requested from a read for write \n");

						}
						page->need_fetch[from_cpu] = 1;

						if (page->concurrent_writers != page->concurrent_fetch) {
							spin_unlock(ptl);
							up_read(&mm->mmap_sem);
							pcn_kmsg_free_msg(request);
							kfree(work);
							PSPRINTK(
									"Waiting, page->concurrent_writers!=page->concurrent_fetch\n");
							return;
						}

						page->status = REPLICATION_STATUS_VALID;

						entry = *pte;
						entry = pte_set_flags(entry, _PAGE_PRESENT);
						entry = pte_set_flags(entry, _PAGE_ACCESSED);
						entry = pte_clear_flags(entry, _PAGE_RW);

						ptep_clear_flush(vma, address, pte);

						set_pte_at_notify(mm, address, pte, entry);

						update_mmu_cache(vma, address, pte);
						//flush_tlb_page(vma, address);

						//flush_tlb_fix_spurious_fault(vma, address);

						response = (data_response_t*) kmalloc(sizeof(data_response_t),
								GFP_ATOMIC);
						if (response == NULL) {
							spin_unlock(ptl);
							up_read(&mm->mmap_sem);
							pcn_kmsg_free_msg(request);
							kfree(work);
							printk("Impossible to kmalloc in process mapping request\n");
							return;
						}

#if NOT_REPLICATED_VMA_MANAGEMENT
						if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
							if (vma != NULL)
#endif
#endif
							{
								response->vma_present = 1;
								response->vaddr_start = vma->vm_start;
								response->vaddr_size = vma->vm_end - vma->vm_start;
								response->prot = vma->vm_page_prot;
								response->vm_flags = vma->vm_flags;
								response->pgoff = vma->vm_pgoff;
								if (vma->vm_file == NULL) {
									response->path[0] = '\0';
								} else {
									plpath = d_path(&vma->vm_file->f_path, lpath, 512);
									strcpy(response->path, plpath);
								}
							} else
								response->vma_present = 0;

						vto = response->data;
						vfrom = kmap_atomic(page, KM_USER0);
						copy_page(vto, vfrom);
						kunmap_atomic(vfrom, KM_USER0);

#if CHECKSUM
						vfrom= kmap_atomic(page, KM_USER0);
						__wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
						kunmap_atomic(vfrom, KM_USER0);
						__wsum check2= csum_partial(&response->data, PAGE_SIZE, 0);
						if(check1!=check2)
							printk("page just copied is not matching, address %lu\n",address);
#endif

						response->last_write = page->last_write;
						for (i = 0; i < MAX_KERNEL_IDS; i++) {
							response->owners[i] = page->other_owners[i];
						}

						response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
						response->header.prio = PCN_KMSG_PRIO_NORMAL;
						response->tgroup_home_cpu = request->tgroup_home_cpu;
						response->tgroup_home_id = request->tgroup_home_id;
						response->address = request->address;
						response->address_present = REPLICATION_STATUS_WRITTEN;

#if CHECKSUM
response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif

page->concurrent_writers = 0;
page->concurrent_fetch = 0;
page->time_stamp = 0;

for (i = 0; i < MAX_KERNEL_IDS; i++)
	if (page->need_fetch[i]) {
		app[i] = 1;
		page->need_fetch[i] = 0;
	} else
		app[i] = 0;

spin_unlock(ptl);

for (i = 0; i < MAX_KERNEL_IDS; i++)
	if (app[i]) {
		pcn_kmsg_send_long(i,
				(struct pcn_kmsg_long_message*) (response),
				sizeof(data_response_t)
				- sizeof(struct pcn_kmsg_hdr));
	}

up_read(&mm->mmap_sem);
kfree(response);
pcn_kmsg_free_msg(request);
kfree(work);
PSPRINTK("End request in written page \n");
return;

					}
				}

				resolved:

				response = (data_response_t*) kmalloc(sizeof(data_response_t), GFP_ATOMIC);
				if (response == NULL) {
					printk("Impossible to kmalloc in process mapping request.\n");
					return;
				}

				PSPRINTK(
						"Resolved Copy from %s\n", ((vma->vm_file!=NULL)?d_path(&vma->vm_file->f_path,lpath,512):"no file"));

				PSPRINTK(
						"Page read only?%i Page shared?%i \n", (vma->vm_flags & VM_WRITE)?0:1, (vma->vm_flags & VM_SHARED)?1:0);

				flush_cache_page(vma, address, pte_pfn(*pte));

				vto = response->data;
				vfrom = kmap_atomic(page, KM_USER0);
				copy_page(vto, vfrom);
				kunmap_atomic(vfrom, KM_USER0);

#if CHECKSUM
				vfrom= kmap_atomic(page, KM_USER0);
				__wsum check1= csum_partial(vfrom, PAGE_SIZE, 0);
				kunmap_atomic(vfrom, KM_USER0);
				__wsum check2= csum_partial(&response->data, PAGE_SIZE, 0);
				if(check1!=check2)
					printk("page just copied is not matching, address %lu\n",address);
#endif

				response->last_write = page->last_write;

				for (i = 0; i < MAX_KERNEL_IDS; i++) {
					response->owners[i] = page->other_owners[i];
				}

				response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
				response->header.prio = PCN_KMSG_PRIO_NORMAL;
				response->tgroup_home_cpu = request->tgroup_home_cpu;
				response->tgroup_home_id = request->tgroup_home_id;
				response->address = request->address;
				response->address_present = REPLICATION_STATUS_VALID;

#if NOT_REPLICATED_VMA_MANAGEMENT
if (_cpu == request->tgroup_home_cpu && vma != NULL)
	//only the vmas SERVER sends the vma
#else
#if PARTIAL_VMA_MANAGEMENT
	if (vma != NULL)
#endif
#endif
	{

		response->vma_present = 1;
		response->vaddr_start = vma->vm_start;
		response->vaddr_size = vma->vm_end - vma->vm_start;
		response->prot = vma->vm_page_prot;
		response->vm_flags = vma->vm_flags;
		response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(response->path, plpath);
		}
	}

	else
		response->vma_present = 0;

spin_unlock(ptl);
up_read(&mm->mmap_sem);

#if CHECKSUM
response->checksum= csum_partial(&response->data, PAGE_SIZE, 0);
#endif

// Send response
pcn_kmsg_send_long(from_cpu, (struct pcn_kmsg_long_message*) (response),
		sizeof(data_response_t) - sizeof(struct pcn_kmsg_hdr));

// Clean up incoming messages
pcn_kmsg_free_msg(request);
kfree(work);
kfree(response);
//end= native_read_tsc();
PSPRINTK("Handle request end\n");
return;

out:

PSPRINTK("There are no copies of the page...\n");

void_response = (data_void_response_t*) kmalloc(
		sizeof(data_void_response_t), GFP_ATOMIC);
if (void_response == NULL) {
	printk("Impossible to kmalloc in process mapping request.\n");
	return;
}

void_response->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID;
void_response->header.prio = PCN_KMSG_PRIO_NORMAL;
void_response->tgroup_home_cpu = request->tgroup_home_cpu;
void_response->tgroup_home_id = request->tgroup_home_id;
void_response->address = request->address;
void_response->address_present = REPLICATION_STATUS_INVALID;
memcpy(void_response->owners,owners,MAX_KERNEL_IDS*sizeof(int));

if (fetching)
	void_response->fetching = 1;
else
	void_response->fetching = 0;

#if NOT_REPLICATED_VMA_MANAGEMENT
if (_cpu == request->tgroup_home_cpu && vma != NULL)
#else
#if PARTIAL_VMA_MANAGEMENT
	if (vma != NULL)
#endif
#endif
	{
		void_response->vma_present = 1;
		void_response->vaddr_start = vma->vm_start;
		void_response->vaddr_size = vma->vm_end - vma->vm_start;
		void_response->prot = vma->vm_page_prot;
		void_response->vm_flags = vma->vm_flags;
		void_response->pgoff = vma->vm_pgoff;
		if (vma->vm_file == NULL) {
			void_response->path[0] = '\0';
		} else {
			plpath = d_path(&vma->vm_file->f_path, lpath, 512);
			strcpy(void_response->path, plpath);
		}
	} else
		void_response->vma_present = 0;

//if (wake) {
//	wake_up(&fetch_write_wait);
//}

// Send response
pcn_kmsg_send_long(from_cpu,
		(struct pcn_kmsg_long_message*) (void_response),
		sizeof(data_void_response_t) - sizeof(struct pcn_kmsg_hdr));

// Clean up incoming messages
pcn_kmsg_free_msg(request);
kfree(void_response);
kfree(work);
//end= native_read_tsc();
PSPRINTK("Handle request end\n");
			}
#endif

			static int handle_mapping_request(struct pcn_kmsg_message* inc_msg) {

				request_work_t* request_work;

#if FOR_2_KERNELS
				data_request_for_2_kernels_t* request = (data_request_for_2_kernels_t*) inc_msg;


				request_work = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

				if (request_work) {
					request_work->request = request;
					INIT_WORK( (struct work_struct*)request_work, process_mapping_request_for_2_kernels);
					queue_work(message_request_wq, (struct work_struct*) request_work);
				}

#else
	data_request_t* request = (data_request_t*) inc_msg;

	request_work = kmalloc(sizeof(request_work_t), GFP_ATOMIC);

	if (request_work) {
		request_work->request = request;
		INIT_WORK( (struct work_struct*)request_work, process_mapping_request);
		queue_work(message_request_wq, (struct work_struct*) request_work);
	}

#endif

return 1;

			}

			/*
 static int remove_invalid_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {
 struct info_page_walk* info = (struct info_page_walk*)walk->private;
 struct page *page;
 struct mm_struct* mm= walk->mm;
 pgd_t* pgd;
 pud_t* pud;
 pmd_t* pmd;
 pte_t* find_pte;
 pte_t entry;
 spinlock_t * ptl;
 unsigned long addr;
 int rss[NR_MM_COUNTERS];
 int i;

 if(NULL == pte ||pte_none(*pte)||is_zero_page(pte_pfn(*pte))) {
 return 0;
 }

 page= pte_page(*pte);

 if(page->replicated==0){
 return 0;
 }

 if(page->status==REPLICATION_STATUS_INVALID){

 //addr= pte_val(*pte);
 addr=start;
 pgd= pgd_offset(mm,addr);
 pud= pud_offset(pgd, addr);
 pmd= pmd_offset(pud, addr);

 find_pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
 if(pte!=find_pte)
 printk("ptes don't match when removing invalid\n");
 entry=*find_pte;

 if (PageAnon(page))
 rss[MM_ANONPAGES]--;
 else {
 if (pte_dirty(entry))
 set_page_dirty(page);
 if (pte_young(entry) &&
 likely(!VM_SequentialReadHint(info->vma)))
 mark_page_accessed(page);

 rss[MM_FILEPAGES]--;
 }
 page_remove_rmap(page);
 free_page_and_swap_cache(page);
 //__free_pages(page, 0);
 //add_mm_rss_vec(mm, rss);


 for (i = 0; i < NR_MM_COUNTERS; i++)
 if (rss[i])
 add_mm_counter(mm, i, rss[i]);
 pte_clear(mm,addr,find_pte);
 update_mmu_cache(info->vma, addr, pte);
 flush_tlb_page(info->vma, addr);
 pte_unmap_unlock(find_pte, ptl);
 return 1;

 }
 if(page->status==REPLICATION_STATUS_INVALID){
 //addr= pte_val(*pte);
 addr=start;
 pgd= pgd_offset(mm,addr);
 pud= pud_offset( pgd, addr);
 pmd= pmd_offset( pud, addr);

 find_pte = pte_offset_map_lock(mm, pmd, addr, &ptl);
 if(pte!=find_pte)
 printk("ptes don't match when removing invalid\n");
 entry=*find_pte;
 entry= pte_set_flags(entry,_PAGE_RW);
 set_pte_at_notify(mm, addr, find_pte, entry);
 update_mmu_cache(info->vma, addr, pte);
 flush_tlb_page(info->vma, addr);
 pte_unmap_unlock(find_pte, ptl);
 return 1;
 }
 return 0;
 }
 void clean_mm_from_invalid_replica(struct mm_struct *mm){
 struct vm_area_struct* curr;
 info_page_walk_t info;
 struct mm_walk walk = {
 .pte_entry = remove_invalid_page_walk_pte_entry_callback,
 .mm = mm,
 .private = &info,
 };

 down_read(&mm->mmap_sem);

 curr = mm->mmap;
 while(curr) {
 info.vma= curr;
 walk_page_range(curr->vm_start,curr->vm_end,&walk);
 curr = curr->vm_next;
 }

 up_read(&mm->mmap_sem);

 }
									 */
			void process_exit_group_notification(struct work_struct* work) {
				exit_group_work_t* request_exit = (exit_group_work_t*) work;
				thread_group_exited_notification_t* msg = request_exit->request;
				unsigned long flags;

				memory_t* mm_data = find_memory_entry(msg->tgroup_home_cpu,
						msg->tgroup_home_id);
				if (mm_data) {
					while (mm_data->main == NULL)
						schedule();

					lock_task_sighand(mm_data->main, &flags);
					mm_data->main->distributed_exit = EXIT_PROCESS;
					unlock_task_sighand(mm_data->main, &flags);

					wake_up_process(mm_data->main);
				}

				pcn_kmsg_free_msg(msg);
				kfree(work);

			}

			void process_exiting_process_notification(struct work_struct* work) {
				exit_work_t* request_work = (exit_work_t*) work;
				exiting_process_t* msg = request_work->request;

				unsigned int source_cpu = msg->header.from_cpu;
				struct task_struct *task;

				task = pid_task(find_get_pid(msg->prev_pid), PIDTYPE_PID);

				if (task && task->next_pid == msg->my_pid && task->next_cpu == source_cpu
						&& task->represents_remote == 1) {

					// set regs
					memcpy(task_pt_regs(task), &msg->regs, sizeof(struct pt_regs));

					// set thread info
					task->thread.fs = msg->thread_fs;
					task->thread.gs = msg->thread_gs;
					task->thread.usersp = msg->old_rsp;
					task->thread.es = msg->thread_es;
					task->thread.ds = msg->thread_ds;
					task->thread.fsindex = msg->thread_fsindex;
					task->thread.gsindex = msg->thread_gsindex;
					task->group_exit = msg->group_exit;
					task->distributed_exit_code = msg->code;
					//printk("%s half way\n",__func__);
#if MIGRATE_FPU
					if (msg->task_flags & PF_USED_MATH)
						//set_used_math();
						set_stopped_child_used_math(task);

					task->fpu_counter = msg->task_fpu_counter;

					//    if (__thread_has_fpu(current)) {
					if (!fpu_allocated(&task->thread.fpu)){
						fpu_alloc(&task->thread.fpu);
						fpu_finit(&task->thread.fpu);
					}

					struct fpu temp; temp.state = &msg->fpu_state;
					fpu_copy(&task->thread.fpu, &temp);

					//    }

					PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d]\n",
							__func__, task->flags, (int)task->fpu_counter,
							(int)task->thread.has_fpu, (int)__thread_has_fpu(task), (int)fpu_allocated(&task->thread.fpu));

					//FPU migration code --- id the following optional?
					if (tsk_used_math(task) && task->fpu_counter >5) //fpu.preload
						__math_state_restore(task);

#endif
					wake_up_process(task);

				} else
					printk("ERROR: task not found. Impossible to kill shadow.");

				pcn_kmsg_free_msg(msg);
				kfree(work);

			}

			static int handle_thread_group_exited_notification(
					struct pcn_kmsg_message* inc_msg) {

				//printk("%s, entered\n",__func__);
				exit_group_work_t* request_work;
				thread_group_exited_notification_t* request =
						(thread_group_exited_notification_t*) inc_msg;

				request_work = kmalloc(sizeof(exit_group_work_t), GFP_ATOMIC);

				if (request_work) {
					request_work->request = request;
					INIT_WORK( (struct work_struct*)request_work,
							process_exit_group_notification);
					queue_work(exit_group_wq, (struct work_struct*) request_work);
				}

				return 1;
			}

			static int handle_exiting_process_notification(struct pcn_kmsg_message* inc_msg) {
				exit_work_t* request_work;
				exiting_process_t* request = (exiting_process_t*) inc_msg;

				//printk("%s, entered\n",__func__);
				request_work = kmalloc(sizeof(exit_work_t), GFP_ATOMIC);

				if (request_work) {
					request_work->request = request;
					INIT_WORK( (struct work_struct*)request_work,
							process_exiting_process_notification);
					queue_work(exit_wq, (struct work_struct*) request_work);
				}

				return 1;

			}

			/**
			 * Handler function for when another processor informs the current cpu
			 * of a pid pairing.
			 */
			static int handle_process_pairing_request(struct pcn_kmsg_message* inc_msg) {
				create_process_pairing_t* msg = (create_process_pairing_t*) inc_msg;
				unsigned int source_cpu = msg->header.from_cpu;
				struct task_struct* task;

				if (inc_msg == NULL) {
					return -1;
				}

				if (msg == NULL) {
					pcn_kmsg_free_msg(inc_msg);
					return -1;
				}

				task = find_task_by_vpid(msg->your_pid);
				if (task == NULL || task->represents_remote == 0) {
					return -1;
				}
				task->next_cpu = source_cpu;
				task->next_pid = msg->my_pid;
				task->executing_for_remote = 0;
				pcn_kmsg_free_msg(inc_msg);

				return 1;
			}

			static int handle_remote_thread_count_response(struct pcn_kmsg_message* inc_msg) {
				remote_thread_count_response_t* msg= (remote_thread_count_response_t*) inc_msg;
				count_answers_t* data = find_count_entry(msg->tgroup_home_cpu,
						msg->tgroup_home_id);
				unsigned long flags;
				struct task_struct* to_wake = NULL;

				PSPRINTK("%s: entered - cpu{%d}, id{%d}, count{%d}\n", __func__, msg->tgroup_home_cpu, msg->tgroup_home_id, msg->count);
				//printk("%s: entered - cpu{%d}, id{%d}, count{%d}\n", __func__, msg->tgroup_home_cpu, msg->tgroup_home_id, msg->count);

				if (data == NULL) {
					PSPRINTK("unable to find remote thread count data\n");
					pcn_kmsg_free_msg(inc_msg);
					return -1;
				}

				raw_spin_lock_irqsave(&(data->lock), flags);

				// Register this response.
				data->responses++;
				data->count += msg->count;

				if (data->responses >= data->expected_responses)
					to_wake = data->waiting;

				raw_spin_unlock_irqrestore(&(data->lock), flags);

				if (to_wake != NULL)
					wake_up_process(to_wake);

				pcn_kmsg_free_msg(inc_msg);

				return 0;
			}

			void process_count_request(struct work_struct* work) {
				count_work_t* request_work = (count_work_t*) work;
				remote_thread_count_request_t* msg = request_work->request;
				remote_thread_count_response_t* response;
				struct task_struct *tgroup_iterator;

				PSPRINTK("%s: entered - cpu{%d}, id{%d}\n", __func__, msg->tgroup_home_cpu, msg->tgroup_home_id);

				response= (remote_thread_count_response_t*) kmalloc(sizeof(remote_thread_count_response_t),GFP_ATOMIC);
				if(!response)
					return;
				response->count = 0;

				/* This is needed to know if the requesting kernel has to save the mapping or send the group dead message.
				 * If there is at least one alive thread of the process in the system the mapping must be saved.
				 * I count how many threads there are but actually I can stop when I know that there is one.
				 * If there are no more threads in the system, a group dead message should be sent by at least one kernel.
				 * I do not need to take the sighand lock (used to set task->distributed_exit=1) because:
				 * --count remote thread is called AFTER set task->distributed_exit=1
				 * --if I am here the last thread of the process in the requesting kernel already set his flag distributed_exit to 1
				 * --two things can happend if the last thread of the process is in this kernel and it is dying too:
				 * --1. set its flag before I check it => I send 0 => the other kernel will send the message
				 * --2. set its flag after I check it => I send 1 => I will send the message
				 * Is important to not take the lock so everything can be done in the messaging layer without fork another kthread.
				 */

				memory_t* memory = find_memory_entry(msg->tgroup_home_cpu,
						msg->tgroup_home_id);
				if (memory != NULL) {
					while (memory->main == NULL)
						schedule();
					tgroup_iterator = memory->main;
					while_each_thread(memory->main, tgroup_iterator)
					{
						if (tgroup_iterator->distributed_exit == EXIT_ALIVE
								&& tgroup_iterator->main != 1) {
							response->count++;
							goto out;
						}
					};
				}

				// Finish constructing response
				out: response->header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE;
				response->header.prio = PCN_KMSG_PRIO_NORMAL;
				response->tgroup_home_cpu = msg->tgroup_home_cpu;
				response->tgroup_home_id = msg->tgroup_home_id;
				PSPRINTK("%s: responding to thread count request with %d\n", __func__, response->count);
				// Send response
				pcn_kmsg_send_long(msg->header.from_cpu, (struct pcn_kmsg_long_message*) (response),sizeof(remote_thread_count_response_t)-sizeof(struct pcn_kmsg_hdr));
				pcn_kmsg_free_msg(msg);
				kfree(response);
				kfree(request_work);


			}

			static int handle_remote_thread_count_request(struct pcn_kmsg_message* inc_msg) {
				count_work_t* request_work;
				remote_thread_count_request_t* request =(remote_thread_count_request_t*) inc_msg;
				request_work = kmalloc(sizeof(count_work_t), GFP_ATOMIC);

				if (request_work) {
					request_work->request = request;
					INIT_WORK( (struct work_struct*)request_work,
							process_count_request);
					queue_work(exit_wq, (struct work_struct*) request_work);
				}

				return 1;

			}

			void process_back_migration(struct work_struct* work) {
				back_mig_work_t* info_work = (back_mig_work_t*) work;
				back_migration_request_t* request = info_work->back_mig_request;

				struct task_struct * task = pid_task(find_get_pid(request->prev_pid), PIDTYPE_PID);

				if (task!=NULL && (task->next_pid == request->placeholder_pid) && (task->next_cpu == request->header.from_cpu)
						&& (task->represents_remote == 1)) {

					memcpy(task_pt_regs(task), &request->regs, sizeof(struct pt_regs));
					task_pt_regs(task)->ax = 0;
					task->thread.fs = request->thread_fs;
					task->thread.gs = request->thread_gs;
					task->thread.usersp = request->old_rsp;
					task->thread.es = request->thread_es;
					task->thread.ds = request->thread_ds;
					task->thread.fsindex = request->thread_fsindex;
					task->thread.gsindex = request->thread_gsindex;
					task->prev_cpu = request->header.from_cpu;
					task->prev_pid = request->placeholder_pid;
					task->personality = request->personality;

				//	task->origin_pid = request->origin_pid;
				//	sigorsets(&task->blocked,&task->blocked,&request->remote_blocked) ;
				///	sigorsets(&task->real_blocked,&task->real_blocked,&request->remote_real_blocked);
				//	sigorsets(&task->saved_sigmask,&task->saved_sigmask,&request->remote_saved_sigmask);
				//	task->pending = request->remote_pending;
				///	task->sas_ss_sp = request->sas_ss_sp;
				//	task->sas_ss_size = request->sas_ss_size;

			//		int cnt = 0;
				//	for (cnt = 0; cnt < _NSIG; cnt++)
				//		task->sighand->action[cnt] = request->action[cnt];

#if MIGRATE_FPU
					//FPU migration code --- server
					/* PF_USED_MATH is set if the task used the FPU before
					 * fpu_counter is incremented every time you go in __switch_to while owning the FPU
					 * has_fpu is true if the task is the owner of the FPU, thus the FPU contains its data
					 * fpu.preload (see arch/x86/include/asm.i387.h:switch_fpu_prepare()) is a heuristic
					 */
					if (request->task_flags & PF_USED_MATH)
						//set_used_math();
						set_stopped_child_used_math(task);

					task->fpu_counter = request->task_fpu_counter;

					//    if (__thread_has_fpu(current)) {
					if (!fpu_allocated(&task->thread.fpu)){
						fpu_alloc(&task->thread.fpu);
						fpu_finit(&task->thread.fpu);
					}

					struct fpu temp; temp.state = &app->fpu_state;
					fpu_copy(&task->thread.fpu, &temp);

					//    }

					PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d]\n",
							__func__, task->flags, (int)task->fpu_counter,
							(int)task->thread.has_fpu, (int)__thread_has_fpu(task), (int)fpu_allocated(&task->thread.fpu));

					//FPU migration code --- is the following optional?
					if (tsk_used_math(task) && task->fpu_counter >5) //fpu.preload
						__math_state_restore(task); //it uses current. Does it restore fpu in current registers???

#endif					//Why thread_has_fpu is not copied?

					task->executing_for_remote = 1;
					task->represents_remote = 0;

					wake_up_process(task);

#if TIMING
					unsigned long long stop= native_read_tsc();
					unsigned long long elapsed_time =stop-info_work->start;
					update_time_migration(elapsed_time,BACK_MIG_R);
#endif

				} else{

					printk("ERROR: task not found. Impossible to re-run shadow.");

				}

				pcn_kmsg_free_msg(request);
				kfree(work);
			}

			static int handle_back_migration(struct pcn_kmsg_message* inc_msg){

				back_migration_request_t* request= (back_migration_request_t*) inc_msg;


/*				back_mig_work_t* work;
#if TIMING
				unsigned long long start= native_read_tsc();
#endif

				work = (back_mig_work_t*) kmalloc(sizeof(back_mig_work_t), GFP_ATOMIC);
				if (work) {
					INIT_WORK( (struct work_struct*)work, process_back_migration);
					work->back_mig_request = request;
#if TIMING
					work->start= start;
#endif
					queue_work(clone_wq, (struct work_struct*) work);
				}
*/





//temporary code to check if the back migration can be faster
//
				struct task_struct * task = pid_task(find_get_pid(request->prev_pid), PIDTYPE_PID);

                                if (task!=NULL && (task->next_pid == request->placeholder_pid) && (task->next_cpu == request->header.from_cpu)
                                                && (task->represents_remote == 1)) {

                                        memcpy(task_pt_regs(task), &request->regs, sizeof(struct pt_regs));
                                        task_pt_regs(task)->ax = 0;
                                        task->thread.fs = request->thread_fs;
                                        task->thread.gs = request->thread_gs;
                                        task->thread.usersp = request->old_rsp;
                                        task->thread.es = request->thread_es;
                                        task->thread.ds = request->thread_ds;
                                        task->thread.fsindex = request->thread_fsindex;
                                        task->thread.gsindex = request->thread_gsindex;
                                        task->prev_cpu = request->header.from_cpu;
                                        task->prev_pid = request->placeholder_pid;
                                        task->personality = request->personality;
					
					task->executing_for_remote = 1;
                                        task->represents_remote = 0;

                                        wake_up_process(task);

#if TIMING
                                        unsigned long long stop= native_read_tsc();
                                        unsigned long long elapsed_time =stop-info_work->start;
                                        update_time_migration(elapsed_time,BACK_MIG_R);
#endif

                                } else{

                                        printk("ERROR: task not found. Impossible to re-run shadow.");

                                }

                                pcn_kmsg_free_msg(request);

				return 0;

			}


			/**
			 * Notify of the fact that either a delegate or placeholder has died locally.
			 * In this case, the remote cpu housing its counterpart must be notified, so
			 * that it can kill that counterpart.
			 */
			int process_server_task_exit_notification(struct task_struct *tsk, long code) {
				int tx_ret = -1;
				int count = 0;

				memory_t* entry = NULL;
				unsigned long flags;

				PSPRINTK(
						"MORTEEEEEE-Process_server_task_exit_notification - pid{%d}\n", tsk->pid);
				//	dump_stack();
				//printk("%s, entered pid %d\n",__func__,tsk->pid);

				if(tsk->distributed_exit==EXIT_ALIVE){

					entry = find_memory_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id);
					if (entry) {

						while (entry->main == NULL)
							schedule();

					} else {
						printk("ERROR: Mapping disappeared, cannot wake up main thread...\n");
						return -1;
					}

					lock_task_sighand(tsk, &flags);

					tsk->distributed_exit = EXIT_THREAD;
					if (entry->main->distributed_exit == EXIT_ALIVE)
						entry->main->distributed_exit = EXIT_THREAD;

					unlock_task_sighand(tsk, &flags);

					/* If I am executing on behalf of a thread on another kernel,
					 * notify the shadow of that thread that I am dying.
					 */
					if (tsk->executing_for_remote) {

						exiting_process_t* msg = (exiting_process_t*) kmalloc(
								sizeof(exiting_process_t), GFP_ATOMIC);

						if (msg != NULL) {
							msg->header.type = PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS;
							msg->header.prio = PCN_KMSG_PRIO_NORMAL;
							msg->my_pid = tsk->pid;
							msg->prev_pid = tsk->prev_pid;
							memcpy(&msg->regs, task_pt_regs(tsk), sizeof(struct pt_regs));
							//msg->regs.ip = (unsigned long) msg->regs.ip -2;
							msg->thread_fs = tsk->thread.fs;
							msg->thread_gs = tsk->thread.gs;
							msg->old_rsp = read_old_rsp();
							msg->thread_usersp = tsk->thread.usersp;
							msg->thread_es = tsk->thread.es;
							msg->thread_ds = tsk->thread.ds;
							msg->thread_fsindex = tsk->thread.fsindex;
							msg->thread_gsindex = tsk->thread.gsindex;
							if (tsk->group_exit == 1)
								msg->group_exit = 1;
							else
								msg->group_exit = 0;
							msg->code = code;

							msg->is_last_tgroup_member = (count == 1 ? 1 : 0);

			#if MIGRATE_FPU
							//FPU migration code --- initiator
							PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d] %d:%d %x\n",
									__func__, tsk->flags, (int)tsk->fpu_counter, (int)tsk->thread.has_fpu,
									(int)__thread_has_fpu(tsk), (int)fpu_allocated(&tsk->thread.fpu),
									(int)use_xsave(), (int)use_fxsr(), (int) PF_USED_MATH);

							msg->task_flags = tsk->flags;
							msg->task_fpu_counter = tsk->fpu_counter;
							msg->thread_has_fpu = tsk->thread.has_fpu;

							//    if (__thread_has_fpu(task)) {

							if (!fpu_allocated(&tsk->thread.fpu)) {
								fpu_alloc(&tsk->thread.fpu);
								fpu_finit(&tsk->thread.fpu);
							}

							fpu_save_init(&tsk->thread.fpu);

							struct fpu temp; temp.state = &msg->fpu_state;

							fpu_copy(&temp,&tsk->thread.fpu);

							//    }

			#endif

							//printk("message exit to shadow sent\n");
							tx_ret = pcn_kmsg_send_long(tsk->prev_cpu,
									(struct pcn_kmsg_long_message*) msg,
									sizeof(exiting_process_t) - sizeof(struct pcn_kmsg_hdr));
							kfree(msg);
						}
					}

					wake_up_process(entry->main);
				}
				return tx_ret;
			}


			/**
			 * Create a pairing between a newly created delegate process and the
			 * remote placeholder process.  This function creates the local
			 * pairing first, then sends a message to the originating cpu
			 * so that it can do the same.
			 */
			int process_server_notify_delegated_subprocess_starting(pid_t pid,
					pid_t remote_pid, int remote_cpu) {

				create_process_pairing_t* msg;
				int tx_ret = -1;

				msg= (create_process_pairing_t*) kmalloc(sizeof(create_process_pairing_t),GFP_ATOMIC);
				if(!msg)
					return -1;
				// Notify remote cpu of pairing between current task and remote
				// representative task.
				msg->header.type = PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING;
				msg->header.prio = PCN_KMSG_PRIO_NORMAL;
				msg->your_pid = remote_pid;
				msg->my_pid = pid;

				tx_ret = pcn_kmsg_send_long(remote_cpu, (struct pcn_kmsg_long_message *) msg, sizeof(create_process_pairing_t)-sizeof(struct pcn_kmsg_hdr));
				kfree(msg);

				return tx_ret;

			}

			/* No other kernels had the page during the remote fetch => a local fetch has been performed.
			 * If during the local fetch a thread in another kernel asks for this page, I would not set the page as replicated.
			 * This function check if the page sould be set as replicated.
			 *
			 * the mm->mmap_sem semaphore is already held in read
			 * return types:
			 * VM_FAULT_OOM, problem allocating memory.
			 * VM_FAULT_VMA, error vma management.
			 * VM_FAULT_REPLICATION_PROTOCOL, replication protocol error.
			 * 0, updated;
			 */
			int process_server_update_page(struct task_struct * tsk, struct mm_struct *mm,
					struct vm_area_struct *vma, unsigned long address_not_page, unsigned long page_fault_flags) {

				unsigned long address;

				pgd_t* pgd;
				pud_t* pud;
				pmd_t* pmd;
				pte_t* pte;
				pte_t entry;
				spinlock_t* ptl = NULL;
				struct page* page, *old_page;
				int ret = 0;

#if FOR_2_KERNELS
				mapping_answers_for_2_kernels_t* fetched_data;

				address = address_not_page & PAGE_MASK;

				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					printk("ERROR: updating a page without valid vma\n");
					ret = VM_FAULT_VMA;
					goto out_not_data;
				}

				if (unlikely(is_vm_hugetlb_page(vma))
						|| unlikely(transparent_hugepage_enabled(vma))) {
					printk("ERROR: Installed a vma with HUGEPAGE\n");
					ret = VM_FAULT_VMA;
					goto out_not_data;
				}

				fetched_data = find_mapping_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id,
						address);

				if (fetched_data != NULL) {

					if(fetched_data->is_fetch!=1 ){
						printk("ERROR: data structure is not for fetch\n");
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto out_not_locked;
					}

					pgd = pgd_offset(mm, address);
					pud = pud_offset(pgd, address);
					if (!pud) {
						printk(
								"ERROR: no pud while trying to update a page locally fetched \n");
						ret = VM_FAULT_VMA;
						goto out_not_locked;
					}
					pmd = pmd_offset(pud, address);
					if (!pmd) {
						printk(
								"ERROR: no pmd while trying to update a page locally fetched \n");
						ret = VM_FAULT_VMA;
						goto out_not_locked;
					}

					retry:			pte = pte_offset_map_lock(mm, pmd, address, &ptl);
					entry= *pte;

					page = pte_page(entry);

					//I replicate only if it is a writable page
					if (vma->vm_flags & VM_WRITE) {

						if (!pte_write(entry)) {
retry_cow:
							PSPRINTK("COW page at %lu \n", address);

							int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, entry);

							if (cow_ret & VM_FAULT_ERROR) {
								if (cow_ret & VM_FAULT_OOM){
									printk("ERROR: %s VM_FAULT_OOM\n",__func__);
									ret = VM_FAULT_OOM;
									goto out_not_locked;
								}

								if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
									printk("ERROR: %s EHWPOISON\n",__func__);
									ret = VM_FAULT_OOM;
									goto out_not_locked;
								}

								if (cow_ret & VM_FAULT_SIGBUS){
									printk("ERROR: %s EFAULT\n",__func__);
									ret = VM_FAULT_OOM;
									goto out_not_locked;
								}

								printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
								ret = VM_FAULT_OOM;
								goto out_not_locked;
							}

							spin_lock(ptl);
							/*PTE LOCKED*/

							entry = *pte;

							if(!pte_write(entry)){
								printk("WARNING: page not writable after cow\n");
								goto retry_cow;
							}

							page = pte_page(entry);
						}

						page->replicated = 0;
						page->owner= 1;
						page->other_owners[_cpu] = 1;

					} else {

						page->replicated = 0;
						page->other_owners[_cpu] = 1;
						page->owner= 1;
					}


				} else {
					printk("ERROR: impossible to find data to update\n");
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto out_not_data;
				}
#else
				int status;
				mapping_answers_t* fetched_data;

				address = address_not_page & PAGE_MASK;

				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					printk("ERROR: updating a page without valid vma\n");
					ret = VM_FAULT_VMA;
					goto out_not_data;
				}

				if (unlikely(is_vm_hugetlb_page(vma))
						|| unlikely(transparent_hugepage_enabled(vma))) {
					printk("ERROR: Installed a vma with HUGEPAGE\n");
					ret = VM_FAULT_VMA;
					goto out_not_data;
				}

				fetched_data = find_mapping_entry(tsk->tgroup_home_cpu, tsk->tgroup_home_id,
						address);

				if (fetched_data != NULL) {

					pgd = pgd_offset(mm, address);
					pud = pud_offset(pgd, address);
					if (!pud) {
						printk(
								"ERROR: no pud while trying to update a page locally fetched \n");
						ret = VM_FAULT_VMA;
						goto out_not_locked;
					}
					pmd = pmd_offset(pud, address);
					if (!pmd) {
						printk(
								"ERROR: no pmd while trying to update a page locally fetched \n");
						ret = VM_FAULT_VMA;
						goto out_not_locked;
					}

					pte = pte_offset_map_lock(mm, pmd, address, &ptl);
					retry: entry = *pte;

					if (fetched_data->last_invalid >= fetched_data->last_write) {

						PSPRINTK("Page will be installed as invalid\n");

						status = REPLICATION_STATUS_INVALID;

					} else if (fetched_data->fetching
							== 1|| fetched_data->address_present!=REPLICATION_STATUS_INVALID)status= REPLICATION_STATUS_VALID;
					else
						status= REPLICATION_STATUS_NOT_REPLICATED;

					page = pte_page(entry);

					if (status != REPLICATION_STATUS_NOT_REPLICATED) {

						//I replicate only if it is a writable page
						if (vma->vm_flags & VM_WRITE) {

							old_page = NULL;

							/* If the page is writable but the pte has not the write flag set, it is a cow page =>
							 * I should create a new page to held the replication
							 */
							if (!pte_write(entry)) {
								/*
								 * I unlock because alloc page may go to sleep
								 */
								spin_unlock(ptl);

								old_page = page;

								if (unlikely(anon_vma_prepare(vma))) {
									ret = VM_FAULT_OOM;
									goto out_not_locked;
								}

								if (is_zero_page(pte_pfn(entry))) {

									page = alloc_zeroed_user_highpage_movable(vma, address);
									if (!page) {
										ret = VM_FAULT_OOM;
										goto out_not_locked;
									}

								} else {

									page =
											alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
									if (!page) {
										ret = VM_FAULT_OOM;
										goto out_not_locked;
									}

									copy_user_highpage(page, old_page, address, vma);
								}

								__SetPageUptodate(page);
								if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
									page_cache_release(page);
									goto out_not_locked;

								}

								spin_lock(ptl);
								//if somebody changed the pte
								if (unlikely(!pte_same(*pte, entry))) {

									mem_cgroup_uncharge_page(page);
									page_cache_release(page);
									goto retry;

								} else {
									page_add_new_anon_rmap(page, vma, address);
									if (fetched_data->last_invalid
											>= fetched_data->last_write) {
										status = REPLICATION_STATUS_INVALID;
										PSPRINTK("Page will be installed as invalid\n");
									} else
										status = REPLICATION_STATUS_VALID;

								}
							}

#if STATISTICS
							pages_allocated++;
#endif
							//process_server_clean_page(page);

							entry = mk_pte(page, vma->vm_page_prot);

							page->replicated = 1;

							if (status == REPLICATION_STATUS_VALID)
								page->status = REPLICATION_STATUS_VALID;
							else
								page->status = REPLICATION_STATUS_INVALID;
							page->owner = fetched_data->owner;

							for (i = 0; i < MAX_KERNEL_IDS; i++) {
								page->other_owners[i] = fetched_data->owners[i];
							}

							page->other_owners[_cpu] = 1;

							flush_cache_page(vma, address, pte_pfn(entry));

							if (status == REPLICATION_STATUS_VALID) {
								/*I need to catch the next write access=> no write permission
								 * if this page fault is caused by a write, clear the flag allows to trigger another page fault
								 */
								entry = pte_clear_flags(entry, _PAGE_RW);
								entry = pte_set_flags(entry, _PAGE_PRESENT);
							} else
								entry = pte_clear_flags(entry, _PAGE_PRESENT);

							entry = pte_set_flags(entry, _PAGE_USER);
							entry = pte_set_flags(entry, _PAGE_ACCESSED);

							ptep_clear_flush(vma, address, pte);

							set_pte_at_notify(mm, address, pte, entry);

							update_mmu_cache(vma, address, pte);
							//flush_tlb_page(vma, address);

							//flush_tlb_fix_spurious_fault(vma, address);

							if (old_page != NULL)
								page_remove_rmap(old_page);

						} else {
							page->replicated = 0;
							for (i = 0; i < MAX_KERNEL_IDS; i++) {
								page->other_owners[i] = fetched_data->owners[i];
							}
							page->other_owners[_cpu] = 1;
						}

					} else {

						page->replicated = 0;
						for (i = 0; i < MAX_KERNEL_IDS; i++) {
							page->other_owners[i] = fetched_data->owners[i];
						}
						page->other_owners[_cpu] = 1;
					}

				} else {
					printk("ERROR: impossible to find data to update\n");
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto out_not_data;
				}

#endif

				spin_unlock(ptl);

				out_not_locked:
#if TIMING
				if(ret==0){
					unsigned long long stop= native_read_tsc();
					unsigned long long time_elapsed= stop-fetched_data->start;

					if(page_fault_flags & FAULT_FLAG_WRITE){
						update_time(time_elapsed,FWL);
					}
					else{
						update_time(time_elapsed,FRL);
					}
				}else
					printk("WARNING: after updating page ret is %d when updating time\n",ret);


#endif
				remove_mapping_entry(fetched_data);
				kfree(fetched_data);
				out_not_data:

				wake_up(&read_write_wait);
				return ret;
			}

			void process_server_clean_page(struct page* page) {

				if (page == NULL) {
					return;
				}

				page->replicated = 0;
				page->status = REPLICATION_STATUS_NOT_REPLICATED;
				page->owner = 0;
				memset(page->other_owners, 0, MAX_KERNEL_IDS*sizeof(int));
				page->writing = 0;
				page->reading = 0;

#if !FOR_2_KERNELS
page->time_stamp = 0;
page->concurrent_writers = 0;
page->concurrent_fetch = 0;
memset(page->need_fetch, 0, MAX_KERNEL_IDS*sizeof(int));
page->last_write = 0;
#endif

#if DIFF_PAGE
page->old_page_version= NULL;
#endif

			}

#if FOR_2_KERNELS

			static int do_remote_read_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
					struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
					unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
					struct page* page) {

				pte_t value_pte;
				int ret=0,i;

#if STATISTICS
				read++;
#endif

				PSMINPRINTK("Read for address %lu pid %d\n", address,current->pid);

				page->reading= 1;

				//message to ask for a copy
				data_request_for_2_kernels_t* read_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
						GFP_ATOMIC);
				if (read_message == NULL) {
					ret = VM_FAULT_OOM;
					goto exit;
				}

				read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
				read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
				read_message->address = address;
				read_message->tgroup_home_cpu = tgroup_home_cpu;
				read_message->tgroup_home_id = tgroup_home_id;
				read_message->is_fetch= 0;
				read_message->is_write= 0;
				read_message->last_write= page->last_write;
				read_message->vma_operation_index= current->mm->vma_operation_index;

				//object to held responses
				mapping_answers_for_2_kernels_t* reading_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
						GFP_ATOMIC);
				if (reading_page == NULL) {
					ret = VM_FAULT_OOM;
					goto exit_read_message;

				}

				reading_page->tgroup_home_cpu= tgroup_home_cpu;
				reading_page->tgroup_home_id= tgroup_home_id;
				reading_page->address = address;
				reading_page->address_present= 0;
				reading_page->data= NULL;
				reading_page->is_fetch= 0;
				reading_page->is_write= 0;
				reading_page->last_write= page->last_write;
				reading_page->owner= 0;

				reading_page->vma_present = 0;
				reading_page->vaddr_start = 0;
				reading_page->vaddr_size = 0;
				reading_page->pgoff = 0;
				memset(reading_page->path,0,sizeof(char)*512);
				memset(&(reading_page->prot),0,sizeof(pgprot_t));
				reading_page->vm_flags = 0;
				reading_page->waiting = current;

				// Make data entry visible to handler.
				add_mapping_entry(reading_page);

				PSPRINTK(
						"Sending a read message for address %lu \n ", address);

				spin_unlock(ptl);
				up_read(&mm->mmap_sem);
				/*PTE UNLOCKED*/
				int sent= 0;
				reading_page->arrived_response=0;

#ifndef SUPPORT_FOR_CLUSTERING
				for(i = 0; i < MAX_KERNEL_IDS; i++) {
					// Skip the current cpu
					if(i == _cpu) continue;

#else
					// the list does not include the current processor group descirptor (TODO)
					struct list_head *iter;
					_remote_cpu_info_list_t *objPtr;
					list_for_each(iter, &rlist_head) {
						objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						i = objPtr->_data._processor;
#endif
						if (page->other_owners[i] == 1) {

							if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (read_message), sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
									== -1)) {
								// Message delivered
								sent++;
								if(sent>1)
									printk("ERROR: using protocol optimized for 2 kernels but sending a read to more than one kernel");
							}
						}
					}

					if(sent){

						while (reading_page->arrived_response == 0) {

							set_task_state(current, TASK_UNINTERRUPTIBLE);
							if (reading_page->arrived_response == 0)
								schedule();
							set_task_state(current, TASK_RUNNING);
						}


					}
					else{
						printk("ERROR: impossible to send read message, no destination kernel\n");
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						down_read(&mm->mmap_sem);
						spin_lock(ptl);
						goto exit_reading_page;
					}

					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					/*PTE LOCKED*/

					vma = find_vma(mm, address);
					if (unlikely(
							!vma || address >= vma->vm_end || address < vma->vm_start)) {

						printk("ERROR: vma not valid during read for write\n");
						ret = VM_FAULT_VMA;
						goto exit_reading_page;
					}


					if(reading_page->address_present==1){

						if (reading_page->data->address != address) {
							printk("ERROR: trying to copy wrong address!");
							pcn_kmsg_free_msg(reading_page->data);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_reading_page;
						}

						if (reading_page->last_write != (page->last_write+1)) {

							printk(
									"ERROR: new copy received during a read but my last write is %lu and received last write is %lu\n",page->last_write,reading_page->last_write);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_reading_page;
						}
						else
							page->last_write= reading_page->last_write;

						if(reading_page->owner==1){
							printk("ERROR: owneship sent with read request\n");
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_reading_page;
						}

						void *vto;
						void *vfrom;
						vto = kmap_atomic(page, KM_USER0);
						vfrom = &(reading_page->data->data);

#if	DIFF_PAGE
						if(reading_page->data->data_size==PAGE_SIZE){
							copy_user_page(vto, vfrom, address, page);
						}
						else{

							if(reading_page->data->diff==1)
								WKdm_decompress_and_diff(vfrom,vto);
							else
							{
								kunmap_atomic(vto, KM_USER0);
								pcn_kmsg_free_msg(reading_page->data);
								printk(
										"ERROR: received data not diff in write address %lu\n",address);
								ret = VM_FAULT_REPLICATION_PROTOCOL;
								goto exit_reading_page;
							}
						}

#else
						copy_user_page(vto, vfrom, address, page);
#endif

						kunmap_atomic(vto, KM_USER0);

#if !DIFF_PAGE
#if CHECKSUM
						vto= kmap_atomic(page, KM_USER0);
						__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
						kunmap_atomic(vto, KM_USER0);
						__wsum check2= csum_partial(&(reading_page->data->data), PAGE_SIZE, 0);
						if(check1!=check2) {
							printk("ERROR: page just copied is not matching, address %lu\n",address);
							pcn_kmsg_free_msg(reading_page->data);
							ret= VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_reading_page;
						}
						if(check1!=reading_page->data->checksum) {
							printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
							pcn_kmsg_free_msg(reading_page->data);
							ret= VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_reading_page;
						}
#endif
#endif

						pcn_kmsg_free_msg(reading_page->data);

						page->status = REPLICATION_STATUS_VALID;
						page->owner = reading_page->owner;

#if STATISTICS
						if(page->last_write> most_written_page)
							most_written_page= page->last_write;
#endif

						flush_cache_page(vma, address, pte_pfn(*pte));
						//now the page can be written
						value_pte = *pte;
						value_pte = pte_clear_flags(value_pte, _PAGE_RW);
						value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);

						value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

						ptep_clear_flush(vma, address, pte);

						set_pte_at_notify(mm, address, pte, value_pte);

						update_mmu_cache(vma, address, pte);

						//flush_tlb_page(vma, address);

						flush_tlb_fix_spurious_fault(vma, address);


						PSPRINTK("Out read %i address %lu \n ", read, address);


					}
					else{

						printk("ERROR: no copy received for a read\n");
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						remove_mapping_entry(reading_page);
						kfree(reading_page);
						kfree(read_message);
						goto exit;

					}


					exit_reading_page:


					remove_mapping_entry(reading_page);
					kfree(reading_page);

					exit_read_message:

					kfree(read_message);

					exit:

					page->reading = 0;

					return ret;
				}

#else
				/* Read on a REPLICATED page => ask a copy of the page at address "address" on the
				 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
				 *
				 * down_read(&mm->mmap_sem) must be held.
				 * pte lock must be held.
				 *
				 *return types:
				 *VM_FAULT_OOM, problem allocating memory.
				 *VM_FAULT_VMA, error vma management.
				 *VM_FAULT_REPLICATION_PROTOCOL, general error.
				 *0, write succeeded;
				 * */
				static int do_remote_read(int tgroup_home_cpu, int tgroup_home_id,
						struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
						unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
						struct page* page) {
#if STATISTICS
					int attemps_read;
#endif
					data_request_t* read_message;
					mapping_answers_t* reading_page;
					int i;
					void *vto;
					void *vfrom;
					pte_t value_pte;
					int ret;
					unsigned long flags;

#if STATISTICS
read++;
attemps_read=0;
#endif

page->reading = 1;

//message to ask a copy of the page
read_message = (data_request_t*) kmalloc(sizeof(data_request_t),
		GFP_ATOMIC);
if (read_message == NULL) {
	ret = VM_FAULT_OOM;
	goto exit;
}

read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
read_message->address = address;
read_message->tgroup_home_cpu = tgroup_home_cpu;
read_message->tgroup_home_id = tgroup_home_id;
read_message->read_for_write = 0;
read_message->flags = page_fault_flags;
read_message->vma_operation_index= current->mm->vma_operation_index;

//object to held responses
reading_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
		GFP_ATOMIC);
if (reading_page == NULL) {
	ret = VM_FAULT_OOM;
	goto exit_read_message;
}

reading_page->address = address;
reading_page->address_present = REPLICATION_STATUS_INVALID;
reading_page->vma_present = 0;
reading_page->data = NULL;
reading_page->responses = 0;
reading_page->expected_responses = 0;
reading_page->last_invalid = -1;
reading_page->last_write = 0;
reading_page->tgroup_home_cpu = tgroup_home_cpu;
reading_page->tgroup_home_id = tgroup_home_id;
reading_page->waiting = current;
raw_spin_lock_init(&(reading_page->lock));

// Add to appropriate list.
add_mapping_entry(reading_page);

retry_read:

#if STATISTICS
attemps_read++;
#endif

PSPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);
PSMINPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);

if (page->owner == _cpu) {
	printk("ERROR: asking a page to myself for read address %lu\n",
			address);
	ret = VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_reading_page;
}

spin_unlock(ptl);
up_read(&mm->mmap_sem);
/*PTE UNLOCKED*/

/* Try to ask the page to the owner first.
 * Likely it has the most updated version
 */
if (pcn_kmsg_send(page->owner, (struct pcn_kmsg_message*) (read_message))
		!= -1) {

	reading_page->expected_responses = 1;

	while (reading_page->responses == 0) {
		//DEFINE_WAIT(wait);
		//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (reading_page->responses == 0)
			schedule();
		set_task_state(current, TASK_RUNNING);
		//finish_wait(&request_wait, &wait);
	}

}

down_read(&mm->mmap_sem);
spin_lock(ptl);
/*PTE LOCKED*/
vma = find_vma(mm, address);
if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

	printk("ERROR: vma not valid during read\n");
	ret = VM_FAULT_VMA;
	goto exit_reading_page;
}
/*If the owner has not a valid copy, or an invalid arrived in the meanwhile,
 *ask to everybody in the system
 */
if (reading_page->address_present == REPLICATION_STATUS_INVALID
		|| reading_page->last_invalid >= reading_page->last_write) {

#if STATISTICS
	attemps_read++;
#endif
	PSPRINTK("Read %i address %lu iter %i \n", read, address, attemps_read);

	reading_page->address_present = REPLICATION_STATUS_INVALID;
	if (reading_page->data != NULL) {
		pcn_kmsg_free_msg(reading_page->data);
		reading_page->data = NULL;
	}
	reading_page->responses = 0;
	reading_page->expected_responses = 0;
	reading_page->vma_present = 0;

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

#ifndef SUPPORT_FOR_CLUSTERING
for(i = 0; i < MAX_KERNEL_IDS; i++) {
	// Skip the current cpu
	if(i == _cpu) continue;
#else
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
#endif
if (page->other_owners[i] == 1) {

	if (!(pcn_kmsg_send(i,
			(struct pcn_kmsg_message*) (read_message)) == -1)) {
		// Message delivered
		reading_page->expected_responses++;
	}
}
	}

	while (!(reading_page->responses == reading_page->expected_responses)) {
		//DEFINE_WAIT(wait);
		//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (!(reading_page->responses == reading_page->expected_responses))
			schedule();
		//finish_wait(&request_wait, &wait);
		set_task_state(current, TASK_RUNNING);
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/
	vma = find_vma(mm, address);
	if (unlikely(
			!vma || address >= vma->vm_end || address < vma->vm_start)) {

		printk("ERROR: vma not valid during read\n");
		ret = VM_FAULT_VMA;
		goto exit_reading_page;
	}

}

if (reading_page->last_invalid >= reading_page->last_write) {

	reading_page->address_present = REPLICATION_STATUS_INVALID;
	if (reading_page->data != NULL) {
		pcn_kmsg_free_msg(reading_page->data);
		reading_page->data = NULL;
	}
	reading_page->responses = 0;
	reading_page->expected_responses = 0;
	reading_page->vma_present = 0;

	goto retry_read;
}

raw_spin_lock_irqsave(&(reading_page->lock), flags);
raw_spin_unlock_irqrestore(&(reading_page->lock), flags);

remove_mapping_entry(reading_page);

if (reading_page->address_present == REPLICATION_STATUS_INVALID) {
	//aaaaaaaaaaaaaaaaaaa not valid copy in the system!!!
	printk("ERROR: NO VALID COPY IN THE SYSTEM\n");
	ret = VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_reading_page;

}

PSPRINTK("Out read %i address %lu iter %i \n", read, address, attemps_read);

if (reading_page->data->address != address) {
	printk("ERROR: trying to copy wrong address!");
	pcn_kmsg_free_msg(reading_page->data);
	ret = VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_reading_page;
}

vto = kmap_atomic(page, KM_USER0);
vfrom = reading_page->data->data;
copy_user_page(vto, vfrom, address, page);
kunmap_atomic(vto, KM_USER0);

#if CHECKSUM
vto= kmap_atomic(page, KM_USER0);
__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
kunmap_atomic(vto, KM_USER0);
__wsum check2= csum_partial(&(reading_page->data->data), PAGE_SIZE, 0);
if(check1!=check2) {
	printk("ERROR: page just copied is not matching, address %lu\n",address);
	pcn_kmsg_free_msg(reading_page->data);
	ret= VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_reading_page;
}
if(check1!=reading_page->data->checksum) {
	printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
	pcn_kmsg_free_msg(reading_page->data);
	ret= VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_reading_page;
}
#endif

pcn_kmsg_free_msg(reading_page->data);

page->last_write = reading_page->last_write;

#if STATISTICS
if(page->last_write> most_written_page)
	most_written_page= page->last_write;
if(attemps_read > most_long_read)
	most_long_read= attemps_read;
#endif

page->status = REPLICATION_STATUS_VALID;

value_pte = *pte;
//we need to catch write access
value_pte = pte_clear_flags(value_pte, _PAGE_RW);
//value_pte= pte_clear_flags(value_pte,_PAGE_DIRTY);
value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
//value_pte= pte_set_flags(value_pte,_PAGE_USER);
value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

ptep_clear_flush(vma, address, pte);

set_pte_at_notify(mm, address, pte, value_pte);

update_mmu_cache(vma, address, pte);

//flush_tlb_page(vma, address);
//flush_tlb_range(vma,vma->vm_start,vma->vm_end);

//flush_tlb_fix_spurious_fault(vma, address);

flush_cache_page(vma, address, pte_pfn(*pte));

ret = 0;

exit_reading_page:

remove_mapping_entry(reading_page);
kfree(reading_page);

exit_read_message:

kfree(read_message);

exit:

page->reading = 0;
return ret;
}
#endif

#if FOR_2_KERNELS
static int do_remote_write_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
		struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
		unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
		struct page* page,int invalid) {

	int  i;
	int ret= 0;
	pte_t value_pte;

	page->writing = 1;

#if STATISTICS
write++;
#endif

PSPRINTK("Write %i address %lu pid %d\n", write, address,current->pid);
PSMINPRINTK("Write for address %lu owner %d pid %d\n", address,page->owner==1?1:0,current->pid);

if(page->owner==1){
	//in this case I send and invalid message

	if(invalid){
		printk("ERROR: I am the owner of the page and it is invalid when going to write\n");
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		goto exit;
	}
	//object to store the acks (nacks) sent by other kernels
	ack_answers_for_2_kernels_t* answers = (ack_answers_for_2_kernels_t*) kmalloc(sizeof(ack_answers_for_2_kernels_t), GFP_ATOMIC);
	if (answers == NULL) {
		ret = VM_FAULT_OOM;
		goto exit;
	}
	answers->tgroup_home_cpu = tgroup_home_cpu;
	answers->tgroup_home_id = tgroup_home_id;
	answers->address = address;
	answers->waiting = current;

	//message to invalidate the other copies
	invalid_data_for_2_kernels_t* invalid_message = (invalid_data_for_2_kernels_t*) kmalloc(sizeof(invalid_data_for_2_kernels_t),
			GFP_ATOMIC);
	if (invalid_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_answers;
	}
	invalid_message->header.type = PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA;
	invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	invalid_message->tgroup_home_cpu = tgroup_home_cpu;
	invalid_message->tgroup_home_id = tgroup_home_id;
	invalid_message->address = address;
	invalid_message->vma_operation_index= current->mm->vma_operation_index;

	// Insert the object in the appropriate list.
	add_ack_entry(answers);

	invalid_message->last_write = page->last_write;

	answers->response_arrived= 0;

	int sent= 0;

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/


#ifndef SUPPORT_FOR_CLUSTERING
	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		// Skip the current cpu
		if(i == _cpu) continue;

#else
		// the list does not include the current processor group descirptor (TODO)
		struct list_head *iter;
		_remote_cpu_info_list_t *objPtr;
		list_for_each(iter, &rlist_head) {
			objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
			i = objPtr->_data._processor;
#endif
if (page->other_owners[i] == 1) {

	if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (invalid_message),sizeof(invalid_data_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
			== -1)) {
		// Message delivered
		sent++;
		if(sent>1)
			printk("ERROR: using protocol optimized for 2 kernels but sending an invalid to more than one kernel");
	}
}
		}

		if(sent){
			while (answers->response_arrived==0) {

				set_task_state(current, TASK_UNINTERRUPTIBLE);
				if (answers->response_arrived==0)
					schedule();

				set_task_state(current, TASK_RUNNING);
			}
		}
		else
			printk("Impossible to send invalid, no destination kernel\n");

		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		/*PTE LOCKED*/

		vma = find_vma(mm, address);
		if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

			printk("ERROR: vma not valid after waiting for ack to invalid\n");
			ret = VM_FAULT_VMA;
			goto exit_invalid;
		}

		PSPRINTK("Received ack to invalid %i address %lu \n", write, address);

		exit_invalid:
		kfree(invalid_message);
		remove_ack_entry(answers);
		exit_answers:
		kfree(answers);
		if(ret!=0)
			goto exit;
	}
	else{
		//in this case I send a mapping request with write flag set

		//message to ask for a copy
		data_request_for_2_kernels_t* write_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
				GFP_ATOMIC);
		if (write_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}

		write_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
		write_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		write_message->address = address;
		write_message->tgroup_home_cpu = tgroup_home_cpu;
		write_message->tgroup_home_id = tgroup_home_id;
		write_message->is_fetch= 0;
		write_message->is_write= 1;
		write_message->last_write= page->last_write;
		write_message->vma_operation_index= current->mm->vma_operation_index;

		//object to held responses
		mapping_answers_for_2_kernels_t* writing_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
				GFP_ATOMIC);
		if (writing_page == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_write_message;

		}

		writing_page->tgroup_home_cpu= tgroup_home_cpu;
		writing_page->tgroup_home_id= tgroup_home_id;
		writing_page->address = address;
		writing_page->address_present= 0;
		writing_page->data= NULL;
		writing_page->is_fetch= 0;
		writing_page->is_write= 1;
		writing_page->last_write= page->last_write;
		writing_page->owner= 0;

		writing_page->vma_present = 0;
		writing_page->vaddr_start = 0;
		writing_page->vaddr_size = 0;
		writing_page->pgoff = 0;
		memset(writing_page->path,0,sizeof(char)*512);
		memset(&(writing_page->prot),0,sizeof(pgprot_t));
		writing_page->vm_flags = 0;
		writing_page->waiting = current;

		// Make data entry visible to handler.
		add_mapping_entry(writing_page);

		PSPRINTK(
				"Sending a write message for address %lu \n ", address);


		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/
		int sent= 0;
		writing_page->arrived_response=0;

#ifndef SUPPORT_FOR_CLUSTERING
		for(i = 0; i < MAX_KERNEL_IDS; i++) {
			// Skip the current cpu
			if(i == _cpu) continue;

#else
			// the list does not include the current processor group descirptor (TODO)
			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head) {
				objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
				i = objPtr->_data._processor;
#endif
if (page->other_owners[i] == 1) {

	if (!(pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (write_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr))
			== -1)) {
		// Message delivered
		sent++;
		if(sent>1)
			printk("ERROR: using protocol optimized for 2 kernels but sending a write to more than one kernel");
	}
}
			}

			if(sent){

				while (writing_page->arrived_response == 0) {

					set_task_state(current, TASK_UNINTERRUPTIBLE);
					if (writing_page->arrived_response == 0)
						schedule();
					set_task_state(current, TASK_RUNNING);
				}


			}
			else{
				printk("ERROR: impossible to send write message, no destination kernel\n");
				ret= VM_FAULT_REPLICATION_PROTOCOL;
				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				goto exit_writing_page;
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			/*PTE LOCKED*/

			vma = find_vma(mm, address);
			if (unlikely(
					!vma || address >= vma->vm_end || address < vma->vm_start)) {

				printk("ERROR: vma not valid during read for write\n");
				ret = VM_FAULT_VMA;
				goto exit_writing_page;
			}

			if(writing_page->owner!=1){
				printk("ERROR: received answer to write without ownership\n");
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_writing_page;
			}

			if(writing_page->address_present==1){

				if (writing_page->data->address != address) {
					printk("ERROR: trying to copy wrong address!");
					pcn_kmsg_free_msg(writing_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_writing_page;
				}
				//in this case I also received the new copy
				if (writing_page->last_write != (page->last_write+1)) {
					pcn_kmsg_free_msg(writing_page->data);
					printk(
							"ERROR: new copy received during a write but my last write is %lu and received last write is %lu\n",page->last_write,writing_page->last_write);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_writing_page;
				}
				else
					page->last_write= writing_page->last_write;


				void *vto;
				void *vfrom;
				vto = kmap_atomic(page, KM_USER0);
				vfrom = &(writing_page->data->data);


#if	DIFF_PAGE
if(writing_page->data->data_size==PAGE_SIZE){
	copy_user_page(vto, vfrom, address, page);
}
else{
	if(writing_page->data->diff==1)
		WKdm_decompress_and_diff(vfrom,vto);
	else
	{
		kunmap_atomic(vto, KM_USER0);
		pcn_kmsg_free_msg(writing_page->data);
		printk(
				"ERROR: received data not diff in write address %lu\n",address);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_writing_page;
	}
}

#else

copy_user_page(vto, vfrom, address, page);
#endif

kunmap_atomic(vto, KM_USER0);

#if !DIFF_PAGE
#if CHECKSUM
vto= kmap_atomic(page, KM_USER0);
__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
kunmap_atomic(vto, KM_USER0);
__wsum check2= csum_partial(&(writing_page->data->data), PAGE_SIZE, 0);
if(check1!=check2) {
	printk("ERROR: page just copied is not matching, address %lu\n",address);
	pcn_kmsg_free_msg(writing_page->data);
	ret= VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_writing_page;
}
if(check1!=writing_page->data->checksum) {
	printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
	pcn_kmsg_free_msg(writing_page->data);
	ret= VM_FAULT_REPLICATION_PROTOCOL;
	goto exit_writing_page;
}
#endif
#endif

pcn_kmsg_free_msg(writing_page->data);

exit_writing_page:

remove_mapping_entry(writing_page);
kfree(writing_page);

exit_write_message:

kfree(write_message);

if(ret!=0)
	goto exit;
			}
			else{

				remove_mapping_entry(writing_page);
				kfree(writing_page);
				kfree(write_message);

				if(invalid){
					printk("ERROR: writing an invalid page but not received a copy\n");
					ret= VM_FAULT_REPLICATION_PROTOCOL;
					goto exit;
				}
			}

		}

		page->status = REPLICATION_STATUS_WRITTEN;
		page->owner = 1;
		(page->last_write)++;

#if STATISTICS
		if(page->last_write> most_written_page)
			most_written_page= page->last_write;
#endif

#if	DIFF_PAGE
		if(page->old_page_version==NULL){
			page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
					GFP_ATOMIC);
			if(page->old_page_version==NULL){
				printk("ERROR: impossible to kmalloc old diff page\n");
				goto exit;
			}
		}

		void *vto;
		void *vfrom;
		vto = page->old_page_version;
		vfrom = kmap_atomic(page, KM_USER0);
		copy_user_page(vto, vfrom, address, page);
		kunmap_atomic(vfrom, KM_USER0);
#endif

flush_cache_page(vma, address, pte_pfn(*pte));

//now the page can be written
value_pte = *pte;
value_pte = pte_set_flags(value_pte, _PAGE_RW);
value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
//value_pte=pte_set_flags(value_pte,_PAGE_USER);
value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);
//value_pte=pte_set_flags(value_pte,_PAGE_DIRTY);
ptep_clear_flush(vma, address, pte);

set_pte_at_notify(mm, address, pte, value_pte);

update_mmu_cache(vma, address, pte);

//flush_tlb_page(vma, address);

flush_tlb_fix_spurious_fault(vma, address);


PSPRINTK("Out write %i address %lu last write is %lu \n ", write, address,page->last_write);

exit:

page->writing = 0;

return ret;

	}
#else
	/* Write on a REPLICATED page => coordinate with other kernels to write on the page at address "address" on the
	 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
	 *
	 * down_read(&mm->mmap_sem) must be held.
	 * pte lock must be held.
	 *
	 *return types:
	 *VM_FAULT_OOM, problem allocating memory.
	 *VM_FAULT_VMA, error vma management.
	 *VM_FAULT_REPLICATION_PROTOCOL, general error.
	 *0, write succeeded;
	 * */
	static int do_remote_write(int tgroup_home_cpu, int tgroup_home_id,
			struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
			unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, spinlock_t* ptl,
			struct page* page) {

		int attemps_write, i;
		ack_answers_t* answers;
		invalid_data_t* invalid_message;
		data_request_t* read_message;
		mapping_answers_t* reading_page;
		unsigned long flags;
		int ret;
		void *vto;
		void *vfrom;
		pte_t value_pte;

		attemps_write = 1;

		page->writing = 1;

		/* Each write has a unique time stamp associated.
		 * This time stamp will not change until the status will be set to written.
		 */
		page->time_stamp = native_read_tsc();

		page->concurrent_writers = 0;
		page->concurrent_fetch = 0;

		//object to store the acks (nacks) sent by other kernels
		answers = (ack_answers_t*) kmalloc(sizeof(ack_answers_t), GFP_ATOMIC);
		if (answers == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}
		answers->tgroup_home_cpu = tgroup_home_cpu;
		answers->tgroup_home_id = tgroup_home_id;
		answers->address = address;
		answers->waiting = current;
		raw_spin_lock_init(&(answers->lock));

		//message to invalidate the other copies
		invalid_message = (invalid_data_t*) kmalloc(sizeof(invalid_data_t),
				GFP_ATOMIC);
		if (invalid_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_answers;
		}
		invalid_message->header.type = PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA;
		invalid_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		invalid_message->tgroup_home_cpu = tgroup_home_cpu;
		invalid_message->tgroup_home_id = tgroup_home_id;
		invalid_message->address = address;
		invalid_message->time_stamp = page->time_stamp;
		invalid_message->vma_operation_index= current->mm->vma_operation_index;

		// Insert the object in the appropriate list.
		add_ack_entry(answers);

#if STATISTICS
write++;
#endif

retry_write:

PSPRINTK(
		"Write %i address %lu attempts %i\n", write, address, attemps_write);

/* If this is not the fist attempt to write, the page has been written by a concurrent thread in another kernel.
 * I need to remote-read the page before trying to write again.
 */
if (attemps_write != 1) {

	page->reading = 1;

	//message to ask for a copy
	read_message = (data_request_t*) kmalloc(sizeof(data_request_t),
			GFP_ATOMIC);
	if (read_message == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_invalid;
	}

	read_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
	read_message->header.prio = PCN_KMSG_PRIO_NORMAL;
	read_message->address = address;
	read_message->tgroup_home_cpu = tgroup_home_cpu;
	read_message->tgroup_home_id = tgroup_home_id;
	read_message->read_for_write = 1;
	read_message->vma_operation_index= current->mm->vma_operation_index;

	//object to held responses
	reading_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
			GFP_ATOMIC);
	if (reading_page == NULL) {
		ret = VM_FAULT_OOM;
		goto exit_read_message;

	}
	reading_page->address = address;
	reading_page->address_present = REPLICATION_STATUS_INVALID;
	reading_page->vma_present = 0;
	reading_page->responses = 0;
	reading_page->expected_responses = 0;
	reading_page->tgroup_home_cpu = tgroup_home_cpu;
	reading_page->tgroup_home_id = tgroup_home_id;
	reading_page->waiting = current;
	raw_spin_lock_init(&(reading_page->lock));

	// Make data entry visible to handler.
	add_mapping_entry(reading_page);

	PSPRINTK(
			"Read for write %i attempt %i address %lu \n ", write, attemps_write, address);

	if (answers->owner == _cpu) {
		printk("ERROR: asking a copy of a page for a write to myself.\n");
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}

	spin_unlock(ptl);
	up_read(&mm->mmap_sem);
	/*PTE UNLOCKED*/

	// Wait for owner to respond.
	if (pcn_kmsg_send(answers->owner,
			(struct pcn_kmsg_message*) (read_message)) != -1) {

		reading_page->expected_responses = 1;

		while (reading_page->responses == 0) {
			//DEFINE_WAIT(wait);
			//prepare_to_wait(&request_wait, &wait, TASK_UNINTERRUPTIBLE);
			set_task_state(current, TASK_UNINTERRUPTIBLE);
			if (reading_page->responses == 0)
				schedule();
			//finish_wait(&request_wait, &wait);
			set_task_state(current, TASK_RUNNING);
		}

	} else {
		printk("ERROR: owner not reachable.\n");
		down_read(&mm->mmap_sem);
		spin_lock(ptl);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	vma = find_vma(mm, address);
	if (unlikely(
			!vma || address >= vma->vm_end || address < vma->vm_start)) {

		printk("ERROR: vma not valid during read for write\n");
		ret = VM_FAULT_VMA;
		goto exit_reading_page;
	}

	raw_spin_lock_irqsave(&(reading_page->lock), flags);
	raw_spin_unlock_irqrestore(&(reading_page->lock), flags);

	if (reading_page->address_present == REPLICATION_STATUS_INVALID) {
		//aaaaaaaaaaaaaaaaaaa not valid copy in the system!!!
		printk(
				"ERROR: NO VALID COPY IN THE SYSTEM WHEN READING FOR WRITE\n");
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}

	if (reading_page->data->address != address) {
		printk("ERROR: trying to copy wrong address!");
		pcn_kmsg_free_msg(reading_page->data);
		ret = VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}
	vto = kmap_atomic(page, KM_USER0);
	vfrom = reading_page->data->data;
	copy_user_page(vto, vfrom, address, page);
	kunmap_atomic(vto, KM_USER0);

#if CHECKSUM
	vto= kmap_atomic(page, KM_USER0);
	__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
	kunmap_atomic(vto, KM_USER0);
	__wsum check2= csum_partial(reading_page->data->data, PAGE_SIZE, 0);
	if(check1!=check2) {
		printk("ERROR: page just copied is not matching, address %lu\n",address);
		pcn_kmsg_free_msg(reading_page->data);
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}
	if(check1!=reading_page->data->checksum) {
		printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
		pcn_kmsg_free_msg(reading_page->data);
		ret= VM_FAULT_REPLICATION_PROTOCOL;
		goto exit_reading_page;
	}
#endif

	pcn_kmsg_free_msg(reading_page->data);
	flush_cache_page(vma, address, pte_pfn(*pte));

	page->status = REPLICATION_STATUS_VALID;
	page->last_write = reading_page->last_write;

	value_pte = *pte;
	//we need to catch write access
	value_pte = pte_clear_flags(value_pte, _PAGE_RW);
	//value_pte= pte_clear_flags(value_pte,_PAGE_DIRTY);
	value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
	//value_pte= pte_set_flags(value_pte,_PAGE_USER);
	value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);

	ptep_clear_flush(vma, address, pte);

	set_pte_at_notify(mm, address, pte, value_pte);

	update_mmu_cache(vma, address, pte);

	//flush_tlb_page(vma, address);
	//flush_tlb_fix_spurious_fault(vma, address);

	remove_mapping_entry(reading_page);
	kfree(reading_page);
	kfree(read_message);

	page->reading = 0;

	//flush_cache_page(vma, address, pte_pfn(*pte));
	PSPRINTK(
			"Out read for write %i attempt %i address %lu \n ", write, attemps_write, address);
}

invalid_message->last_write = page->last_write;

answers->nack = 0;
answers->responses = 0;
answers->expected_responses = 0;
answers->concurrent = 0;
answers->owner = _cpu;
answers->time_stamp = page->time_stamp;

//send to the other copies the invalidation message
spin_unlock(ptl);
up_read(&mm->mmap_sem);
/*PTE UNLOCKED*/

PSMINPRINTK("writing %i address %lu iter %i \n", write, address,attemps_write);

#ifndef SUPPORT_FOR_CLUSTERING
for(i = 0; i < MAX_KERNEL_IDS; i++) {
	// Skip the current cpu
	if(i == _cpu) continue;

#else
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
#endif
if (page->other_owners[i] == 1) {

	if (!(pcn_kmsg_send(i, (struct pcn_kmsg_message*) (invalid_message))
			== -1)) {
		// Message delivered
		answers->expected_responses++;
	}
}
	}

	//wait for all the answers (ack or nack) to arrive
	while (!(answers->responses == answers->expected_responses)) {
		//DEFINE_WAIT(wait);
		//	prepare_to_wait(&ack_wait, &wait, TASK_UNINTERRUPTIBLE);
		set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (!(answers->responses == answers->expected_responses))
			schedule();
		//finish_wait(&ack_wait, &wait);
		set_task_state(current, TASK_RUNNING);
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	vma = find_vma(mm, address);
	if (unlikely(!vma || address >= vma->vm_end || address < vma->vm_start)) {

		printk("ERROR: vma not valid after waiting for ack to invalid\n");
		ret = VM_FAULT_VMA;
		goto exit_invalid;
	}

	/* If somebody was concurrently writing with success on other kernels, retry.
	 */
	PSPRINTK("Concurrent_writers %i \n", page->concurrent_writers);

	if (answers->nack != 0) {

#if STATISTICS
		concurrent_write++;
#endif

		attemps_write++;
		goto retry_write;

	} else {

		PSPRINTK(
				"Received all acks to write %i address %lu attempts %i\n", write, address, attemps_write);

		raw_spin_lock_irqsave(&(answers->lock), flags);
		raw_spin_unlock_irqrestore(&(answers->lock), flags);

		//change status to written
		page->status = REPLICATION_STATUS_WRITTEN;
		page->owner = _cpu;
		(page->last_write)++;

#if STATISTICS
		if(page->last_write> most_written_page)
			most_written_page= page->last_write;
		if(attemps_write >most_long_write)
			most_long_write= attemps_write;
#endif

		memset(page->need_fetch, 0, MAX_KERNEL_IDS*sizeof(int));
		page->concurrent_fetch = 0;
		page->concurrent_writers = answers->concurrent;

		flush_cache_page(vma, address, pte_pfn(*pte));

		//now the page can be written
		value_pte = *pte;
		value_pte = pte_set_flags(value_pte, _PAGE_RW);
		value_pte = pte_set_flags(value_pte, _PAGE_PRESENT);
		//value_pte=pte_set_flags(value_pte,_PAGE_USER);
		value_pte = pte_set_flags(value_pte, _PAGE_ACCESSED);
		//value_pte=pte_set_flags(value_pte,_PAGE_DIRTY);
		ptep_clear_flush(vma, address, pte);

		set_pte_at_notify(mm, address, pte, value_pte);

		update_mmu_cache(vma, address, pte);

		//flush_tlb_page(vma, address);

		flush_tlb_fix_spurious_fault(vma, address);

		ret = 0;
		goto exit_invalid;
	}

	exit_reading_page:

	remove_mapping_entry(reading_page);
	kfree(reading_page);

	exit_read_message:

	kfree(read_message);
	page->reading = 0;

	exit_invalid:

	kfree(invalid_message);
	remove_ack_entry(answers);

	exit_answers:

	kfree(answers);

	exit:

	page->writing = 0;

	return ret;
}

#endif

static unsigned long map_difference(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff) {
	unsigned long ret = addr;
	unsigned long start = addr;
	unsigned long local_end = start;
	unsigned long end = addr + len;
	struct vm_area_struct* curr;
	unsigned long error;

	// go through ALL vma's, looking for interference with this space.
	curr = current->mm->mmap;

	while (1) {

		if (start >= end)
			goto done;

		// We've reached the end of the list
		else if (curr == NULL) {
			// map through the end
			error = do_mmap(file, start, end - start, prot, flags, pgoff);
			if (error != start) {
				ret = VM_FAULT_VMA;
			}
			goto done;
		}

		// the VMA is fully above the region of interest
		else if (end <= curr->vm_start) {
			// mmap through local_end
			error = do_mmap(file, start, end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;
			goto done;
		}

		// the VMA fully encompases the region of interest
		else if (start >= curr->vm_start && end <= curr->vm_end) {
			// nothing to do
			goto done;
		}

		// the VMA is fully below the region of interest
		else if (curr->vm_end <= start) {
			// move on to the next one

		}

		// the VMA includes the start of the region of interest
		// but not the end
		else if (start >= curr->vm_start && start < curr->vm_end
				&& end > curr->vm_end) {
			// advance start (no mapping to do)
			start = curr->vm_end;
			local_end = start;

		}

		// the VMA includes the end of the region of interest
		// but not the start
		else if (start < curr->vm_start && end <= curr->vm_end
				&& end > curr->vm_start) {
			local_end = curr->vm_start;

			// mmap through local_end
			error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;

			// Then we're done
			goto done;
		}

		// the VMA is fully within the region of interest
		else if (start <= curr->vm_start && end >= curr->vm_end) {
			// advance local end
			local_end = curr->vm_start;

			// map the difference
			error = do_mmap(file, start, local_end - start, prot, flags, pgoff);
			if (error != start)
				ret = VM_FAULT_VMA;

			// Then advance to the end of this vma
			start = curr->vm_end;
			local_end = start;
		}

		curr = curr->vm_next;

	}

	done:

	return ret;
}

#if FOR_2_KERNELS
static int do_mapping_for_distributed_process(mapping_answers_for_2_kernels_t* fetching_page,
		struct mm_struct* mm, unsigned long address, spinlock_t* ptl) {
#else
	static int do_mapping_for_distributed_process(mapping_answers_t* fetching_page,
			struct mm_struct* mm, unsigned long address, spinlock_t* ptl) {
#endif

		struct vm_area_struct* vma;
		unsigned long prot = 0;
		unsigned long err, ret;

		prot |= (fetching_page->vm_flags & VM_READ) ? PROT_READ : 0;
		prot |= (fetching_page->vm_flags & VM_WRITE) ? PROT_WRITE : 0;
		prot |= (fetching_page->vm_flags & VM_EXEC) ? PROT_EXEC : 0;

		if (fetching_page->vma_present == 1) {

			if (fetching_page->path[0] == '\0') {
			
			        vma = find_vma(mm, address);
                                if (!vma || address >= vma->vm_end || address < vma->vm_start) {
                                        vma = NULL;
                                }

                                if (!vma || (vma->vm_start != fetching_page->vaddr_start)
                                                || (vma->vm_end != (fetching_page->vaddr_start + fetching_page->vaddr_size))) {
				

				spin_unlock(ptl);
				/*PTE UNLOCKED*/

				/* Note: during a page fault the distribute lock is held in read =>
				 * distributed vma operations cannot happen in the same time
				 */
				up_read(&mm->mmap_sem);
				down_write(&mm->mmap_sem);

				/* when I release the down write on mmap_sem, another thread of my process
				 * could install the same vma that I am trying to install
				 * (only fetch of same addresses are prevent, not fetch of different addresses on the same vma)
				 * take the newest vma.
				 * */
				vma = find_vma(mm, address);
				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					vma = NULL;
				}

				/* All vma operations are distributed, except for mmap =>
				 * When I receive a vma, the only difference can be on the size (start, end) of the vma.
				 */
				if (!vma || (vma->vm_start != fetching_page->vaddr_start)
						|| (vma->vm_end
								!= (fetching_page->vaddr_start
										+ fetching_page->vaddr_size))) {
					PSPRINTK(
							"Mapping anonimous vma start %lu end %lu \n", fetching_page->vaddr_start, (fetching_page->vaddr_start + fetching_page->vaddr_size));
#if NOT_REPLICATED_VMA_MANAGEMENT

					/*Note:
					 * This mapping is caused because when a thread migrates it does not have any vma
					 * so during fetch vma can be pushed.
					 * This mapping has the precedence over "normal" vma operations because is a page fault
					 * */

					current->mm->distribute_unmap = 0;
#else
#if PARTIAL_VMA_MANAGEMENT
					current->mm->distribute_unmap = 0;

#endif
#endif
					/*map_difference should map in such a way that no unmap operations (the only nested operation that mmap can call) are nested called.
					 * This is important both to not unmap pages that should not be unmapped
					 * but also because otherwise the vma protocol will deadlock!
					 */
					err = map_difference(NULL, fetching_page->vaddr_start,
							fetching_page->vaddr_size, prot,
							MAP_FIXED | MAP_ANONYMOUS
							| ((fetching_page->vm_flags & VM_SHARED) ?
									MAP_SHARED : MAP_PRIVATE)
									| ((fetching_page->vm_flags & VM_HUGETLB) ?
											MAP_HUGETLB : 0)
											| ((fetching_page->vm_flags & VM_GROWSDOWN) ?
													MAP_GROWSDOWN : 0), 0);

#if NOT_REPLICATED_VMA_MANAGEMENT

					current->mm->distribute_unmap = 1;
#else
#if PARTIAL_VMA_MANAGEMENT
					current->mm->distribute_unmap = 1;

#endif
#endif
					if (err != fetching_page->vaddr_start) {
						up_write(&mm->mmap_sem);
						down_read(&mm->mmap_sem);
						spin_lock(ptl);
						/*PTE LOCKED*/
						printk(
								"ERROR: error mapping anonimous vma while fetching address %lu \n",
								address);
						ret = VM_FAULT_VMA;
						return ret;
					}

				}

				up_write(&mm->mmap_sem);
				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/
				}

			} else {

				vma = find_vma(mm, address);
                                if (!vma || address >= vma->vm_end || address < vma->vm_start) {
                                        vma = NULL;
                                }

                                 if (!vma || (vma->vm_start != fetching_page->vaddr_start)
                                                || (vma->vm_end != (fetching_page->vaddr_start + fetching_page->vaddr_size))) {
 
				spin_unlock(ptl);
				/*PTE UNLOCKED*/

				up_read(&mm->mmap_sem);

				struct file* f;

				f = filp_open(fetching_page->path, O_RDONLY | O_LARGEFILE, 0);

				down_write(&mm->mmap_sem);

				if (!IS_ERR(f)) {


					//check if other threads already installed the vma
					vma = find_vma(mm, address);
					if (!vma || address >= vma->vm_end || address < vma->vm_start) {
						vma = NULL;
					}

					if (!vma || (vma->vm_start != fetching_page->vaddr_start)
							|| (vma->vm_end
									!= (fetching_page->vaddr_start
											+ fetching_page->vaddr_size))) {

						PSPRINTK(
								"Mapping file vma start %lu end %lu\n", fetching_page->vaddr_start, (fetching_page->vaddr_start + fetching_page->vaddr_size));

#if NOT_REPLICATED_VMA_MANAGEMENT
						/*Note:
						 * This mapping is caused because when a thread migrates it does not have any vma
						 * so during fetch vma can be pushed.
						 * This mapping has the precedence over "normal" vma operations because is a page fault
						 * */

						current->mm->distribute_unmap = 0;
#else
#if PARTIAL_VMA_MANAGEMENT

						current->mm->distribute_unmap = 0;

#endif
#endif
						/*map_difference should map in such a way that no unmap operations (the only nested operation that mmap can call) are nested called.
						 * This is important both to not unmap pages that should not be unmapped
						 * but also because otherwise the vma protocol will deadlock!
						 */
						err =
								map_difference(f, fetching_page->vaddr_start,
										fetching_page->vaddr_size, prot,
										MAP_FIXED
										| ((fetching_page->vm_flags
												& VM_DENYWRITE) ?
														MAP_DENYWRITE : 0)
														| ((fetching_page->vm_flags
																& VM_EXECUTABLE) ?
																		MAP_EXECUTABLE : 0)
																		| ((fetching_page->vm_flags
																				& VM_SHARED) ?
																						MAP_SHARED : MAP_PRIVATE)
																						| ((fetching_page->vm_flags
																								& VM_HUGETLB) ?
																										MAP_HUGETLB : 0),
																										fetching_page->pgoff << PAGE_SHIFT);

#if NOT_REPLICATED_VMA_MANAGEMENT

						current->mm->distribute_unmap = 1;
#else
#if PARTIAL_VMA_MANAGEMENT

						current->mm->distribute_unmap = 1;

#endif
#endif
						if (err != fetching_page->vaddr_start) {
							up_write(&mm->mmap_sem);
							down_read(&mm->mmap_sem);
							spin_lock(ptl);
							/*PTE LOCKED*/
							printk(
									"ERROR: error mapping file vma while fetching address %lu \n",
									address);
							ret = VM_FAULT_VMA;
							return ret;
						}
					}

				} else {
					up_write(&mm->mmap_sem);
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					/*PTE LOCKED*/
					printk("ERROR: error while opening file %s \n",
							fetching_page->path);
					ret = VM_FAULT_VMA;
					return ret;
				}

				up_write(&mm->mmap_sem);

				filp_close(f, NULL);

				down_read(&mm->mmap_sem);
				spin_lock(ptl);
				/*PTE LOCKED*/
				}

			}

			return 0;
		}
		return 0;
	}

#if FOR_2_KERNELS

	static int do_remote_fetch_for_2_kernels(int tgroup_home_cpu, int tgroup_home_id,
			struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
			unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
			spinlock_t* ptl) {

		mapping_answers_for_2_kernels_t* fetching_page;
		data_request_for_2_kernels_t* fetch_message;
		int ret= 0,i,reachable,other_cpu=-1;

		PSMINPRINTK("Fetch for address %lu write %i pid %d is local?%d\n", address,((page_fault_flags & FAULT_FLAG_WRITE)?1:0),current->pid,pte_none(value_pte));

#if STATISTICS
		fetch++;
#endif

		fetching_page = (mapping_answers_for_2_kernels_t*) kmalloc(sizeof(mapping_answers_for_2_kernels_t),
				GFP_ATOMIC);
		if (fetching_page == NULL) {
			ret = VM_FAULT_OOM;
			goto exit;
		}

		fetching_page->tgroup_home_cpu = tgroup_home_cpu;
		fetching_page->tgroup_home_id = tgroup_home_id;
		fetching_page->address = address;

		fetching_page->vma_present = 0;
		fetching_page->vaddr_start = 0;
		fetching_page->vaddr_size = 0;
		fetching_page->pgoff = 0;
		memset(fetching_page->path,0,sizeof(char)*512);
		memset(&(fetching_page->prot),0,sizeof(pgprot_t));
		fetching_page->vm_flags = 0;

		if(page_fault_flags & FAULT_FLAG_WRITE)
			fetching_page->is_write= 1;
		else
			fetching_page->is_write= 0;

		fetching_page->is_fetch= 1;
		fetching_page->owner= 0;
		fetching_page->address_present= 0;
		fetching_page->last_write= 0;
		fetching_page->data= NULL;
	fetching_page->futex_owner = -1;//akshay

		fetching_page->waiting = current;

		add_mapping_entry(fetching_page);

		if(_cpu==tgroup_home_cpu){
			if(pte_none(value_pte)){
				//not marked pte

#if STATISTICS
local_fetch++;
#endif
PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lu\n", local_fetch, address);
ret = VM_CONTINUE_WITH_CHECK;
goto exit;
			}
		}

		fetch_message = (data_request_for_2_kernels_t*) kmalloc(sizeof(data_request_for_2_kernels_t),
				GFP_ATOMIC);
		if (fetch_message == NULL) {
			ret = VM_FAULT_OOM;
			goto exit_fetching_page;
		}

		fetch_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
		fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
		fetch_message->address = address;
		fetch_message->tgroup_home_cpu = tgroup_home_cpu;
		fetch_message->tgroup_home_id = tgroup_home_id;
		fetch_message->is_write = fetching_page->is_write;
		fetch_message->is_fetch= 1;
		fetch_message->vma_operation_index= current->mm->vma_operation_index;

		PSPRINTK("Fetch %i address %lu\n", fetch, address);

		spin_unlock(ptl);
		up_read(&mm->mmap_sem);
		/*PTE UNLOCKED*/

		fetching_page->arrived_response= 0;
		reachable= 0;

		memory_t* memory= find_memory_entry(current->tgroup_home_cpu,
				current->tgroup_home_id);

		down_read(&memory->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
		for(i = 0; i < MAX_KERNEL_IDS; i++) {
			// Skip the current cpu
			if(i == _cpu) continue;

#else
			// the list does not include the current processor group descirptor (TODO)
			struct list_head *iter;
			_remote_cpu_info_list_t *objPtr;
			list_for_each(iter, &rlist_head) {
				objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
				i = objPtr->_data._processor;

#endif
if(memory->kernel_set[i]==1)
	if ((ret=pcn_kmsg_send_long(i, (struct pcn_kmsg_long_message*) (fetch_message),sizeof(data_request_for_2_kernels_t)-sizeof(struct pcn_kmsg_hdr)))
			!= -1) {
		// Message delivered
		reachable++;
		other_cpu= i;
		if(reachable>1)
			printk("ERROR: using optimized algorithm for 2 kernels with more than two kernels\n");
	}
			}

			up_read(&memory->kernel_set_sem);

			if(reachable>0){
				while (fetching_page->arrived_response==0) {

					set_task_state(current, TASK_UNINTERRUPTIBLE);

					if (fetching_page->arrived_response==0) {
						schedule();
					}

					set_task_state(current, TASK_RUNNING);
				}
			}else
				printk("OCCHIO... qui c'e'roba non richiesta.... ret is %i",ret);

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			/*PTE LOCKED*/

			PSPRINTK("Out wait fetch %i address %lu \n", fetch, address);

#if NOT_REPLICATED_VMA_MANAGEMENT
			//only the client has to update the vma
			if(tgroup_home_cpu!=_cpu)
#endif

			{
				ret = do_mapping_for_distributed_process(fetching_page, mm, address, ptl);
				if (ret != 0)
					goto exit_fetch_message;

				vma = find_vma(mm, address);
				if (!vma || address >= vma->vm_end || address < vma->vm_start) {
					vma = NULL;
				} else if (unlikely(is_vm_hugetlb_page(vma))
						|| unlikely(transparent_hugepage_enabled(vma))) {
					printk("ERROR: Installed a vma with HUGEPAGE\n");
					ret = VM_FAULT_VMA;
					goto exit_fetch_message;
				}

				if (vma == NULL) {
			//PSPRINTK
			dump_stack();
			printk(KERN_ALERT"%s: ERROR: no vma for address %lu in the system {%d} \n",__func__, address,current->pid);
					ret = VM_FAULT_VMA;
					goto exit_fetch_message;
				}

			}

			if(_cpu==tgroup_home_cpu && fetching_page->address_present == 0){
				printk("ERROR: No response for a marked page\n");
				ret = VM_FAULT_REPLICATION_PROTOCOL;
				goto exit_fetch_message;
			}

			if (fetching_page->address_present == 1) {

				struct page* page;
				spin_unlock(ptl);
				/*PTE UNLOCKED*/

				if (unlikely(anon_vma_prepare(vma))) {
					spin_lock(ptl);
					/*PTE LOCKED*/
					ret = VM_FAULT_OOM;
					goto exit_fetch_message;
				}

				page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
				if (!page) {
					spin_lock(ptl);
					/*PTE LOCKED*/
					ret = VM_FAULT_OOM;
					goto exit_fetch_message;
				}

				__SetPageUptodate(page);

				if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
					page_cache_release(page);
					spin_lock(ptl);
					/*PTE LOCKED*/
					ret = VM_FAULT_OOM;
					goto exit_fetch_message;
				}

#if STATISTICS
				pages_allocated++;
#endif
				spin_lock(ptl);
				/*PTE LOCKED*/

				int status;

				//if nobody changed the pte
				if (likely(pte_same(*pte, value_pte))) {

					if(fetching_page->is_write){ //if I am doing a write

						status= REPLICATION_STATUS_WRITTEN;
						if(fetching_page->owner==0){
							printk("ERROR: copy of a page sent to a write fetch request without ownership\n");
							pcn_kmsg_free_msg(fetching_page->data);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_fetch_message;
						}

					}
					else{

						status= REPLICATION_STATUS_VALID;
						if(fetching_page->owner==1){
							printk("ERROR: copy of a page sent to a read fetch request with ownership\n");
							pcn_kmsg_free_msg(fetching_page->data);
							ret = VM_FAULT_REPLICATION_PROTOCOL;
							goto exit_fetch_message;
						}
					}

					void *vto;
					void *vfrom;

					if (fetching_page->data->address != address) {
						printk("ERROR: trying to copy wrong address!");
						pcn_kmsg_free_msg(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}


#if DIFF_PAGE

					if(fetching_page->data->diff==1){
						printk("ERROR: answered to a fetch with diff data\n");
						pcn_kmsg_free_msg(fetching_page->data);
						ret = VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}

					vto = kmap_atomic(page, KM_USER0);
					vfrom = &(fetching_page->data->data);

					if(fetching_page->data->data_size==PAGE_SIZE)
						copy_user_page(vto, vfrom, address, page);
					else{
						WKdm_decompress(vfrom,vto);
					}

					kunmap_atomic(vto, KM_USER0);

					if(status==REPLICATION_STATUS_WRITTEN){
						if(page->old_page_version==NULL){
							page->old_page_version= kmalloc(sizeof(char)*PAGE_SIZE,
									GFP_ATOMIC);
							if(page->old_page_version==NULL){
								printk("ERROR: impossible to kmalloc old diff page\n");
								pcn_kmsg_free_msg(fetching_page->data);
								ret = VM_FAULT_REPLICATION_PROTOCOL;
								goto exit_fetch_message;
							}
						}

						vto = page->old_page_version;
						vfrom = kmap_atomic(page, KM_USER0);
						memcpy(vto, vfrom, PAGE_SIZE);
						kunmap_atomic(vto, KM_USER0);
					}
#else
					vto = kmap_atomic(page, KM_USER0);
					vfrom = &(fetching_page->data->data);
					copy_user_page(vto, vfrom, address, page);
					kunmap_atomic(vto, KM_USER0);

#if READ_PAGE
int ct=0;
if(address == PAGE_ADDR)
	for(ct=0;ct<8;ct++){
	printk(KERN_ALERT"{%lx} ",(unsigned long) *(((unsigned long *)vfrom)+ct));
	}
}
#endif

#if CHECKSUM
					vto= kmap_atomic(page, KM_USER0);
					__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
					kunmap_atomic(vto, KM_USER0);
					__wsum check2= csum_partial(&(fetching_page->data->data), PAGE_SIZE, 0);


					if(check1!=check2) {
						printk("ERROR: page just copied is not matching, address %lu\n",address);
						pcn_kmsg_free_msg(fetching_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
					if(check1!=fetching_page->data->checksum) {
						printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
						pcn_kmsg_free_msg(fetching_page->data);
						ret= VM_FAULT_REPLICATION_PROTOCOL;
						goto exit_fetch_message;
					}
#endif

#endif

					pcn_kmsg_free_msg(fetching_page->data);

					pte_t entry = mk_pte(page, vma->vm_page_prot);

					//if the page is read only no need to keep replicas coherent
					if (vma->vm_flags & VM_WRITE) {

						page->replicated = 1;

						if(fetching_page->is_write){
							page->last_write = fetching_page->last_write+1;
						}
						else
							page->last_write = fetching_page->last_write;

#if STATISTICS
if(page->last_write> most_written_page)
	most_written_page= page->last_write;
#endif
page->owner = fetching_page->owner;

page->status = status;

if (status == REPLICATION_STATUS_VALID) {
	entry = pte_clear_flags(entry, _PAGE_RW);
} else {
	entry = pte_set_flags(entry, _PAGE_RW);
}

					} else {
						if(fetching_page->is_write)
							printk("ERROR: trying to write a read only page\n");

						if(fetching_page->owner==1)
							printk("ERROR: received ownership with a copy of a read only page\n");

						page->replicated = 0;
						page->owner= 0;
						page->status= REPLICATION_STATUS_NOT_REPLICATED;

					}

					entry = pte_set_flags(entry, _PAGE_PRESENT);
					page->other_owners[_cpu]=1;
					page->other_owners[other_cpu]=1;
					page->futex_owner = fetching_page->futex_owner;//akshay

					flush_cache_page(vma, address, pte_pfn(*pte));

					entry = pte_set_flags(entry, _PAGE_USER);
					entry = pte_set_flags(entry, _PAGE_ACCESSED);


					ptep_clear_flush(vma, address, pte);



					page_add_new_anon_rmap(page, vma, address);
					set_pte_at_notify(mm, address, pte, entry);

					update_mmu_cache(vma, address, pte);


				} else {
					printk("OCCHIO... pte changed while fetching\n");
					status = REPLICATION_STATUS_INVALID;
					mem_cgroup_uncharge_page(page);
					page_cache_release(page);
					pcn_kmsg_free_msg(fetching_page->data);

				}

				PSPRINTK("End fetching address %lu \n", address);
				ret= 0;
				goto exit_fetch_message;

			}

			//copy not present on the other kernel
			else {

#if STATISTICS
				local_fetch++;
#endif
				PSPRINTK("Copy not present in the other kernel, local fetch %d of address %lu\n", local_fetch, address);
				PSMINPRINTK("Local fetch for address %lu\n",address);
				kfree(fetch_message);
				ret = VM_CONTINUE_WITH_CHECK;
				goto exit;
			}

			exit_fetch_message:

			kfree(fetch_message);

			exit_fetching_page:

			remove_mapping_entry(fetching_page);
			kfree(fetching_page);

			exit:
			return ret;
		}

#else
		/* Fetch a page from the system => ask other kernels if they have a copy of the page at address "address" on the
		 * virtual mapping of the process identified by "tgroup_home_cpu" and "tgroup_home_id".
		 *
		 * down_read(&mm->mmap_sem) must be held.
		 * pte lock must be held.
		 *
		 *return types:
		 *VM_FAULT_OOM, problem allocating memory.
		 *VM_FAULT_VMA, error vma management.
		 *VM_FAULT_REPLICATION_PROTOCOL, general error.
		 *VM_CONTINUE_WITH_CHECK, fetch the page locally.
		 *0, remotely fetched;
		 *-1, invalidated while fetching;
		 * */
		static int do_remote_fetch(int tgroup_home_cpu, int tgroup_home_id,
				struct mm_struct *mm, struct vm_area_struct *vma, unsigned long address,
				unsigned long page_fault_flags, pmd_t* pmd, pte_t* pte, pte_t value_pte,
				spinlock_t* ptl) {

			mapping_answers_t* fetching_page;
			data_request_t* fetch_message;
			int i;
			unsigned long flags;
			int ret = 0;
			char lpath[512];

			/* I need to keep the information that this address is currently on a fetch phase.
			 * Store the info in an appropriate list.
			 * This allows the handlers of invalidation and request to maintain an updated status for the future page.
			 * Plus the answers to my fetch will update this object.
			 * Plus it will prevent multiple fetch of the same address.
			 */

			fetching_page = (mapping_answers_t*) kmalloc(sizeof(mapping_answers_t),
					GFP_ATOMIC);
			if (fetching_page == NULL) {
				ret = VM_FAULT_OOM;
				goto exit;
			}
			fetching_page->address = address;
			fetching_page->tgroup_home_cpu = tgroup_home_cpu;
			fetching_page->tgroup_home_id = tgroup_home_id;
			fetching_page->address_present = REPLICATION_STATUS_INVALID;
			fetching_page->data = NULL;
			fetching_page->fetching = 0;
			fetching_page->last_invalid = -1;
			fetching_page->last_write = 0;
			fetching_page->owner = -1;
			memset(fetching_page->owners, 0, sizeof(int) * MAX_KERNEL_IDS);
			fetching_page->vma_present = 0;
			fetching_page->vaddr_start = 0;
			fetching_page->vaddr_size = 0;
			fetching_page->vm_flags = 0;
			fetching_page->pgoff = 0;
			memset(fetching_page->path,0,sizeof(char)*512);
			memset(&(fetching_page->prot),0,sizeof(pgprot_t));
			raw_spin_lock_init(&(fetching_page->lock));
			fetching_page->responses = 0;
			fetching_page->waiting = current;

			// Insert the object in the appropriate list.
			add_mapping_entry(fetching_page);

			//create the message to broadcast to other kernels
			fetch_message = (data_request_t*) kmalloc(sizeof(data_request_t),
					GFP_ATOMIC);
			if (fetch_message == NULL) {
				ret = VM_FAULT_OOM;
				goto exit_fetching_page;
			}

			fetch_message->header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
			fetch_message->header.prio = PCN_KMSG_PRIO_NORMAL;
			fetch_message->address = address;
			fetch_message->tgroup_home_cpu = tgroup_home_cpu;
			fetch_message->tgroup_home_id = tgroup_home_id;
			fetch_message->read_for_write = 0;
			fetch_message->vma_operation_index= current->mm->vma_operation_index;

#if STATISTICS
fetch++;
#endif
PSPRINTK("Fetch %i address %lu \n", fetch, address);
PSMINPRINTK("Fetch %i address %lu \n", fetch, address);

spin_unlock(ptl);
up_read(&mm->mmap_sem);
/*PTE UNLOCKED*/

//send to all cpus
fetching_page->expected_responses = 0;

memory_t* memory= find_memory_entry(current->tgroup_home_cpu,
		current->tgroup_home_id);

down_read(&memory->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
for(i = 0; i < MAX_KERNEL_IDS; i++) {
	// Skip the current cpu
	if(i == _cpu) continue;

#else
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
#endif
if(memory->kernel_set[i]==1)
	if (pcn_kmsg_send(i, (struct pcn_kmsg_message*) (fetch_message))
			!= -1) {
		// Message delivered
		fetching_page->expected_responses++;
	}
	}

	up_read(&memory->kernel_set_sem);

	//wait while all the reachable cpus send back an answer
	while (fetching_page->expected_responses != fetching_page->responses) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if (fetching_page->expected_responses != fetching_page->responses) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	down_read(&mm->mmap_sem);
	spin_lock(ptl);
	/*PTE LOCKED*/

	PSPRINTK("Out wait fetch %i address %lu \n", fetch, address);

	raw_spin_lock_irqsave(&(fetching_page->lock), flags);
	raw_spin_unlock_irqrestore(&(fetching_page->lock), flags);

#if NOT_REPLICATED_VMA_MANAGEMENT
	//only the client has to update the vma
	if(tgroup_home_cpu!=_cpu)
#endif

	{
		ret = do_mapping_for_distributed_process(fetching_page, mm, address, ptl);
		if (ret != 0)
			goto exit_fetch_message;

		vma = find_vma(mm, address);
		if (!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		} else if (unlikely(is_vm_hugetlb_page(vma))
				|| unlikely(transparent_hugepage_enabled(vma))) {
			printk("ERROR: Installed a vma with HUGEPAGE\n");
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}

		if (vma == NULL) {
			PSPRINTK("ERROR: no vma for address %lu in the system\n", address);
			ret = VM_FAULT_VMA;
			goto exit_fetch_message;
		}
	}
	/*Check if someone sent a copy of the page.
	 *If there not exist valid copies, a copy is locally fetched.
	 */
	if (fetching_page->address_present != REPLICATION_STATUS_INVALID) {
		struct page* page;
		spin_unlock(ptl);
		/*PTE UNLOCKED*/

		if (unlikely(anon_vma_prepare(vma))) {
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

		page = alloc_page_vma(GFP_HIGHUSER_MOVABLE, vma, address);
		if (!page) {
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

		__SetPageUptodate(page);

		if (mem_cgroup_newpage_charge(page, mm, GFP_ATOMIC)) {
			page_cache_release(page);
			spin_lock(ptl);
			/*PTE LOCKED*/
			ret = VM_FAULT_OOM;
			goto exit_fetch_message;
		}

		//process_server_clean_page(page);
#if STATISTICS
		pages_allocated++;
#endif
		spin_lock(ptl);
		/*PTE LOCKED*/

		int status;
		//if nobody changed the pte
		if (likely(pte_same(*pte, value_pte))) {

			/*If an invalid message arrived for the oldest copy that I received,
			 *the copy should be discarded and not installed.
			 */
			if (fetching_page->last_invalid >= fetching_page->last_write) {
				status = REPLICATION_STATUS_INVALID;
				PSPRINTK("Page will be installed as invalid\n");
			} else
				status = REPLICATION_STATUS_VALID;

			void *vto;
			void *vfrom;
			//copy into the page the copy received
			if (status == REPLICATION_STATUS_VALID) {

				if (fetching_page->data->address != address) {
					printk("ERROR: trying to copy wrong address!");
					pcn_kmsg_free_msg(fetching_page->data);
					ret = VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
				vto = kmap_atomic(page, KM_USER0);
				vfrom = fetching_page->data->data;
				copy_user_page(vto, vfrom, address, page);
				kunmap_atomic(vto, KM_USER0);

#if CHECKSUM
				vto= kmap_atomic(page, KM_USER0);
				__wsum check1= csum_partial(vto, PAGE_SIZE, 0);
				kunmap_atomic(vto, KM_USER0);
				__wsum check2= csum_partial(fetching_page->data->data, PAGE_SIZE, 0);
				if(check1!=check2) {
					printk("ERROR: page just copied is not matching, address %lu\n",address);
					pcn_kmsg_free_msg(fetching_page->data);
					ret= VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
				if(check1!=fetching_page->data->checksum) {
					printk("ERROR: page just copied is not matching the one sent, address %lu\n",address);
					pcn_kmsg_free_msg(fetching_page->data);
					ret= VM_FAULT_REPLICATION_PROTOCOL;
					goto exit_fetch_message;
				}
#endif

			}

			pcn_kmsg_free_msg(fetching_page->data);

			pte_t entry = mk_pte(page, vma->vm_page_prot);

			//if the page is read only no need to keep replicas coherent
			if (vma->vm_flags & VM_WRITE) {

				page->replicated = 1;

				page->last_write = fetching_page->last_write;

#if STATISTICS
				if(page->last_write> most_written_page)
					most_written_page= page->last_write;
#endif

				memcpy(page->other_owners, fetching_page->owners,
						sizeof(int) * MAX_KERNEL_IDS);
				page->other_owners[_cpu] = 1;
				page->owner = fetching_page->owner;

				if (status == REPLICATION_STATUS_VALID) {
					page->status = REPLICATION_STATUS_VALID;
					entry = pte_set_flags(entry, _PAGE_PRESENT);
					entry = pte_clear_flags(entry, _PAGE_RW);
				} else {
					entry = pte_clear_flags(entry, _PAGE_PRESENT);
					page->status = REPLICATION_STATUS_INVALID;
				}

			} else {
				page->replicated = 0;
				memcpy(page->other_owners, fetching_page->owners,
						sizeof(int) * MAX_KERNEL_IDS);
				page->other_owners[_cpu] = 1;
				entry = pte_set_flags(entry, _PAGE_PRESENT);
			}

			flush_cache_page(vma, address, pte_pfn(*pte));

			entry = pte_set_flags(entry, _PAGE_USER);
			entry = pte_set_flags(entry, _PAGE_ACCESSED);

			ptep_clear_flush(vma, address, pte);

			page_add_new_anon_rmap(page, vma, address);

			set_pte_at_notify(mm, address, pte, entry);

			update_mmu_cache(vma, address, pte);

			//flush_tlb_page(vma, address);

			//flush_tlb_fix_spurious_fault(vma, address);

		} else {
			status = REPLICATION_STATUS_INVALID;
			mem_cgroup_uncharge_page(page);
			page_cache_release(page);
			pcn_kmsg_free_msg(fetching_page->data);
		}

		PSPRINTK("End fetching address %lu \n", address);
		if (status == REPLICATION_STATUS_INVALID)
			ret = -1;
		else
			ret = 0;
		goto exit_fetch_message;

	}

	//copies not present on other kernels
	else {
		//I am the only using it => no need of replication until someone asks for it
#if STATISTICS
		local_fetch++;
#endif
		PSPRINTK(
				"Copy not present in the system, local fetch %d of address %lu\n", local_fetch, address);

		kfree(fetch_message);
		ret = VM_CONTINUE_WITH_CHECK;
		goto exit;
	}

	exit_fetch_message:

	kfree(fetch_message);

	exit_fetching_page:

	remove_mapping_entry(fetching_page);
	kfree(fetching_page);

	exit:

	return ret;

}

#endif

extern int access_error(unsigned long error_code, struct vm_area_struct *vma);

/**
 * down_read(&mm->mmap_sem) already held
 *
 * return types:
 * VM_FAULT_OOM, problem allocating memory.
 * VM_FAULT_VMA, error vma management.
 * VM_FAULT_ACCESS_ERROR, access error;
 * VM_FAULT_REPLICATION_PROTOCOL, replication protocol error.
 * VM_CONTINUE_WITH_CHECK, fetch the page locally.
 * VM_CONTINUE, normal page_fault;
 * 0, remotely fetched;
 */
int process_server_try_handle_mm_fault(struct task_struct *tsk,
		struct mm_struct *mm, struct vm_area_struct *vma,
		unsigned long page_faul_address, unsigned long page_fault_flags,
		unsigned long error_code) {

	pgd_t* pgd;
	pud_t* pud;
	pmd_t* pmd;
	pte_t* pte;
	pte_t value_pte;
	spinlock_t *ptl;

	struct page* page;

	unsigned long address;

	int tgroup_home_cpu = tsk->tgroup_home_cpu;
	int tgroup_home_id = tsk->tgroup_home_id;
	int ret;

#if TIMING
unsigned long long start= native_read_tsc();
#endif

address = page_faul_address & PAGE_MASK;

#if STATISTICS
page_fault_mio++;
#endif
PSPRINTK(
		"Page fault %i address %lu in page %lu task pid %d t_group_cpu %d t_group_id %d \n", page_fault_mio, page_faul_address, address, tsk->pid, tgroup_home_cpu, tgroup_home_id);
PSMINPRINTK(
                "Page fault for address %lu in page %lu task pid %d t_group_cpu %d t_group_id %d \n", page_faul_address, address, tsk->pid, tgroup_home_cpu, tgroup_home_id);

	if(page_fault_flags & FAULT_FLAG_WRITE){
		PSPRINTK(KERN_ALERT"write\n");
	}
	else{
		PSPRINTK(KERN_ALERT"read\n");
	}

if (address == 0) {
	printk("ERROR: accessing page at address 0 pid %i\n",tsk->pid);
	return VM_FAULT_ACCESS_ERROR | VM_FAULT_VMA;
}


if (vma && (address < vma->vm_end && address >= vma->vm_start)
		&& (unlikely(is_vm_hugetlb_page(vma))
				|| transparent_hugepage_enabled(vma))) {
	printk("ERROR: page fault for huge page\n");
	return VM_CONTINUE;
}

pgd = pgd_offset(mm, address);

pud = pud_alloc(mm, pgd, address);
if (!pud)
	return VM_FAULT_OOM;

pmd = pmd_alloc(mm, pud, address);
if (!pmd)
	return VM_FAULT_OOM;

if (pmd_none(*pmd) && __pte_alloc(mm, vma, pmd, address))
	return VM_FAULT_OOM;

if (unlikely(pmd_trans_huge(*pmd))) {
	printk("ERROR: page fault for huge page\n");
	return VM_CONTINUE;
}

pte = pte_offset_map_lock(mm, pmd, address, &ptl);
/*PTE LOCKED*/

value_pte = *pte;

/*case pte UNMAPPED
 * --Remote fetch--
 */
#if FOR_2_KERNELS
start: if (pte == NULL || pte_none(pte_clear_flags(value_pte, _PAGE_UNUSED1))) {

	if(pte==NULL)
		printk("OCCHIO... pte NULL\n");

#else
	start: if (pte == NULL || pte_none(value_pte)) {
#endif


		/* Check if other threads of my process are already
		 * fetching the same address on this kernel.
		 */
		if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address) != NULL) {

			//wait while the fetch is ended
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);

			while (find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address)
					!= NULL) {

				DEFINE_WAIT(wait);
				prepare_to_wait(&read_write_wait, &wait, TASK_UNINTERRUPTIBLE);

				if (find_mapping_entry(tgroup_home_cpu, tgroup_home_id,
						address)!=NULL) {
					schedule();
				}

				finish_wait(&read_write_wait, &wait);
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			value_pte = *pte;

			vma = find_vma(mm, address);
			if (unlikely(
					!vma || address >= vma->vm_end
					|| address < vma->vm_start)) {

				printk(
						"ERROR: vma not valid after waiting for another thread to fetch\n");
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}

			goto start;
		}

		if (!vma || address >= vma->vm_end || address < vma->vm_start) {
			vma = NULL;
		}

#if FOR_2_KERNELS
		ret = do_remote_fetch_for_2_kernels(tsk->tgroup_home_cpu, tsk->tgroup_home_id, mm,
				vma, address, page_fault_flags, pmd, pte, value_pte, ptl);

#else

		ret = do_remote_fetch(tsk->tgroup_home_cpu, tsk->tgroup_home_id, mm,
				vma, address, page_fault_flags, pmd, pte, value_pte, ptl);

		//if it is a write and I did not have errors, avoid doing another page fault
		if (ret == -1 || ((page_fault_flags & FAULT_FLAG_WRITE) && ret == 0)) {

			wake_up(&read_write_wait);
			value_pte = *pte;

			vma = find_vma(mm, address);
			if (unlikely(
					!vma || address >= vma->vm_end
					|| address < vma->vm_start)) {

				printk(
						"ERROR: vma not valid after fetching it without errors\n");
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}
#if TIMING
			unsigned long long stop= native_read_tsc();
			unsigned long long time_elapsed= stop-start;

			update_time(time_elapsed,FWR);
#endif

			goto start;
		}

#endif

		spin_unlock(ptl);
		wake_up(&read_write_wait);

#if TIMING
		if(ret==0){//case without check
			unsigned long long stop= native_read_tsc();
			unsigned long long time_elapsed= stop-start;

			if(page_fault_flags & FAULT_FLAG_WRITE){
				update_time(time_elapsed,FWR);
			}
			else{
				update_time(time_elapsed,FRR);
			}
		}
		else
			if(ret==VM_CONTINUE_WITH_CHECK){
#if FOR_2_KERNELS
				mapping_answers_for_2_kernels_t* fetched_data= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
				if(fetched_data!=NULL)
					fetched_data->start= start;
				else
					printk("WARNING: after fetch is not possible to find fetched data while trying to store timing\n");
#else
				mapping_answers_t* fetched_data= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
				if(fetched_data!=NULL)
					fetched_data->start= start;
				else
					printk("WARNING: after fetch is not possible to find fetched data while trying to store timing\n");

#endif
			}
			else
				printk("WARNING: ret from fetch is %d when trying to store timing\n");

#endif
		return ret;

	}

	/*case pte MAPPED
	 */
	else {

#if FOR_2_KERNELS
		/* There can be an unluckily case in which I am still fetching...
		 */
		mapping_answers_for_2_kernels_t* fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
		if (fetch != NULL && fetch->is_fetch==1) {

			//wait while the fetch is ended
			spin_unlock(ptl);
			up_read(&mm->mmap_sem);

			fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
			while (fetch != NULL && fetch->is_fetch==1) {

				DEFINE_WAIT(wait);
				prepare_to_wait(&read_write_wait, &wait, TASK_UNINTERRUPTIBLE);

				fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
				if (fetch != NULL && fetch->is_fetch==1) {
					schedule();
				}

				finish_wait(&read_write_wait, &wait);

				fetch= find_mapping_entry(tgroup_home_cpu, tgroup_home_id, address);
			}

			down_read(&mm->mmap_sem);
			spin_lock(ptl);
			value_pte = *pte;

			vma = find_vma(mm, address);
			if (unlikely(
					!vma || address >= vma->vm_end
					|| address < vma->vm_start)) {

				printk(
						"ERROR: vma not valid after waiting for another thread to fetch\n");
				spin_unlock(ptl);
				return VM_FAULT_VMA;
			}

			goto start;
		}
#endif
		/* The pte is mapped so the vma should be valid.
		 * Check if the access is within the limit.
		 */
		if (unlikely(
				!vma || address >= vma->vm_end || address < vma->vm_start)) {
			printk("ERROR: no vma for address %lu in the system\n", address);
			spin_unlock(ptl);
			return VM_FAULT_VMA;
		}

		/*
		 * Check if the permission are ok.
		 */
		if (unlikely(access_error(error_code, vma))) {
			spin_unlock(ptl);
			return VM_FAULT_ACCESS_ERROR;
		}

		page = pte_page(value_pte);
		if (page != vm_normal_page(vma, address, value_pte)) {
			PSPRINTK("page different from vm_normal_page\n");
		}
		/* case page NOT REPLICATED
		 */
		if (page->replicated == 0) {
			PSPRINTK("Page not replicated address %lu \n", address);

			//check if it a cow page...
			if ((vma->vm_flags & VM_WRITE) && !pte_write(value_pte)) {

retry_cow:
				PSPRINTK("COW page at %lu \n", address);

				int cow_ret= do_wp_page_for_popcorn(mm, vma,address, pte,pmd,ptl, value_pte);

				if (cow_ret & VM_FAULT_ERROR) {
					if (cow_ret & VM_FAULT_OOM){
						printk("ERROR: %s VM_FAULT_OOM\n",__func__);
						return VM_FAULT_OOM;
					}

					if (cow_ret & (VM_FAULT_HWPOISON | VM_FAULT_HWPOISON_LARGE)){
						printk("ERROR: %s EHWPOISON\n",__func__);
						return VM_FAULT_OOM;
					}

					if (cow_ret & VM_FAULT_SIGBUS){
						printk("ERROR: %s EFAULT\n",__func__);
						return VM_FAULT_OOM;
					}

					printk("ERROR: %s bug from do_wp_page_for_popcorn\n",__func__);
					return VM_FAULT_OOM;
				}

				spin_lock(ptl);
				/*PTE LOCKED*/

				value_pte = *pte;

				if(!pte_write(value_pte)){
					printk("WARNING: page not writable after cow\n");
					goto retry_cow;
				}

				page = pte_page(value_pte);

				page->replicated = 0;
				page->status= REPLICATION_STATUS_NOT_REPLICATED;
				page->owner= 1;
				page->other_owners[_cpu] = 1;


			}

			spin_unlock(ptl);

#if TIMING

			unsigned long long stop= native_read_tsc();
			unsigned long long time_elapsed= stop-start;

			if(page_fault_flags & FAULT_FLAG_WRITE){
				update_time(time_elapsed,NRW);
			}
			else{
				update_time(time_elapsed,NRR);
			}

#endif

			return 0;
		}

		check:

		/* case REPLICATION_STATUS_VALID:
		 * the data of the page is up to date.
		 * reads can be performed locally.
		 * to write is needed to send an invalidation message to all the other copies.
		 * a write in REPLICATION_STATUS_VALID changes the status to REPLICATION_STATUS_WRITTEN
		 */
		if (page->status == REPLICATION_STATUS_VALID) {

			PSPRINTK("Page status valid address %lu \n", address);

			/*read case
			 */
			if (!(page_fault_flags & FAULT_FLAG_WRITE)) {
				spin_unlock(ptl);
#if TIMING
				unsigned long long stop= native_read_tsc();
				unsigned long long time_elapsed= stop-start;

				update_time(time_elapsed,VR);

#endif

				return 0;
			}

			/*write case
			 */
			else {

				/* If other threads of this process are writing or reading in this kernel, I wait.
				 *
				 * I wait for concurrent writes because after a write the status is updated to REPLICATION_STATUS_WRITTEN,
				 * so only the first write needs to send the invalidation messages.
				 *
				 * I wait for reads because if the invalidation message of the write is handled before the request of the read
				 * there could be the possibility that nobody answers to the read whit a copy.
				 *
				 */

				if (page->writing == 1 || page->reading == 1) {

					spin_unlock(ptl);
					up_read(&mm->mmap_sem);

					while (page->writing == 1 || page->reading == 1) {
						DEFINE_WAIT(wait);

						prepare_to_wait(&read_write_wait, &wait,
								TASK_UNINTERRUPTIBLE);
						if (page->writing == 1 || page->reading == 1)
							schedule();
						finish_wait(&read_write_wait, &wait);

					}
					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					value_pte = *pte;

					vma = find_vma(mm, address);
					if (unlikely(
							!vma || address >= vma->vm_end
							|| address < vma->vm_start)) {

						printk(
								"ERROR: vma not valid after waiting for another thread to fetch\n");
						spin_unlock(ptl);
						return VM_FAULT_VMA;
					}

					goto check;

				}

#if FOR_2_KERNELS
				ret = do_remote_write_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
						address, page_fault_flags, pmd, pte, ptl, page,0);
#else
				ret = do_remote_write(tgroup_home_cpu, tgroup_home_id, mm, vma,
						address, page_fault_flags, pmd, pte, ptl, page);
#endif
				spin_unlock(ptl);
				wake_up(&read_write_wait);
#if TIMING
				if(ret==0){
					unsigned long long stop= native_read_tsc();
					unsigned long long time_elapsed= stop-start;

					update_time(time_elapsed,VW);

				}else
					printk("WARNING not possible update time ret is %d in valid write\n",ret);
#endif
				return ret;
			}
		} else

			/* case REPLICATION_STATUS_WRITTEN
			 * both read and write can be performed on this page.
			 * */
			if (page->status == REPLICATION_STATUS_WRITTEN) {
				PSPRINTK("Page status written address %lu \n", address);
				spin_unlock(ptl);
#if TIMING
				unsigned long long stop= native_read_tsc();
				unsigned long long time_elapsed= stop-start;
				if(page_fault_flags & FAULT_FLAG_WRITE){
					update_time(time_elapsed,MW);
				}
				else
					update_time(time_elapsed,MR);
#endif

return 0;
			}

			else {

				if (!(page->status == REPLICATION_STATUS_INVALID)) {
					printk("ERROR: Page status not correct on address %lu \n",
							address);
					spin_unlock(ptl);
					return VM_FAULT_REPLICATION_PROTOCOL;
				}

				PSPRINTK("Page status invalid address %lu \n", address);

				/*If other threads are already reading or writing it wait,
				 * they will eventually read a valid copy
				 */
				if (page->writing == 1 || page->reading == 1) {

					spin_unlock(ptl);
					up_read(&mm->mmap_sem);

					while (page->writing == 1 || page->reading == 1) {
						DEFINE_WAIT(wait);
						prepare_to_wait(&read_write_wait, &wait,
								TASK_UNINTERRUPTIBLE);
						if (page->writing == 1 || page->reading == 1)
							schedule();
						finish_wait(&read_write_wait, &wait);
					}

					down_read(&mm->mmap_sem);
					spin_lock(ptl);
					value_pte = *pte;

					vma = find_vma(mm, address);
					if (unlikely(
							!vma || address >= vma->vm_end
							|| address < vma->vm_start)) {

						printk(
								"ERROR: vma not valid after waiting for another thread to fetch\n");
						spin_unlock(ptl);
						return VM_FAULT_VMA;
					}

					goto check;

				}

#if FOR_2_KERNELS
				if (page_fault_flags & FAULT_FLAG_WRITE)
					ret = do_remote_write_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
							address, page_fault_flags, pmd, pte, ptl, page,1);
				else
					ret = do_remote_read_for_2_kernels(tgroup_home_cpu, tgroup_home_id, mm, vma,
							address, page_fault_flags, pmd, pte, ptl, page);
#else
	/* case REPLICATION_STATUS_INVALID
	 * both read and write need to remote-read the page.
	 */

				ret = do_remote_read(tgroup_home_cpu, tgroup_home_id, mm, vma,
						address, page_fault_flags, pmd, pte, ptl, page);

				//if it is a write and I did not have errors, avoid doing another page fault
				if ((page_fault_flags & FAULT_FLAG_WRITE) && ret == 0) {

					value_pte = *pte;
					wake_up(&read_write_wait);

					goto check;
				}
#endif
spin_unlock(ptl);
wake_up(&read_write_wait);

#if TIMING

unsigned long long stop= native_read_tsc();
unsigned long long time_elapsed= stop-start;

if(page_fault_flags & FAULT_FLAG_WRITE){
	update_time(time_elapsed,IW);
}
else{
	update_time(time_elapsed,IR);
}

#endif
return ret;

			}
	}

}

int process_server_dup_task(struct task_struct* orig, struct task_struct* task) {
	unsigned long flags;

	task->executing_for_remote = 0;
	task->represents_remote = 0;
	task->distributed_exit = EXIT_ALIVE;
	task->tgroup_distributed = 0;
	task->prev_cpu = -1;
	task->next_cpu = -1;
	task->prev_pid = -1;
	task->next_pid = -1;
	task->tgroup_home_cpu = -1;
	task->tgroup_home_id = -1;
	task->main = 0;
	task->group_exit = -1;
	task->surrogate = -1;
        task->group_exit= -1;
        task->uaddr = 0;
        task->origin_pid = -1;
	// If the new task is not in the same thread group as the parent,
	// then we do not need to propagate the old thread info.
	if (orig->tgid != task->tgid) {
		return 1;
	}

	lock_task_sighand(orig, &flags);
	// This is important.  We want to make sure to keep an accurate record
	// of which cpu and thread group the new thread is a part of.
	if (orig->tgroup_distributed == 1) {
		task->tgroup_home_cpu = orig->tgroup_home_cpu;
		task->tgroup_home_id = orig->tgroup_home_id;
		task->tgroup_distributed = 1;
	}

	unlock_task_sighand(orig, &flags);

	return 1;

}


/*
 * Send a message to <dst_cpu> for migrating back a task <task>.
 * This is a back migration => <task> must already been migrated at least once in <dst_cpu>.
 * It returns -1 in error case.
 */
static int do_back_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs){

	unsigned long flags;
	int ret;
	back_migration_request_t* request;

	request= (back_migration_request_t*) kmalloc(sizeof(back_migration_request_t), GFP_ATOMIC);
	if(request==NULL)
		return -1;

//printk("%s entered dst{%d} \n",__func__,dst_cpu);
	request->header.type = PCN_KMSG_TYPE_PROC_SRV_BACK_MIG_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;

	request->back=1;
	request->prev_pid= task->prev_pid;

	request->personality = task->personality;

	/*mklinux_akshay*/
	request->origin_pid = task->origin_pid;
	request->remote_blocked = task->blocked;
	request->remote_real_blocked = task->real_blocked;
	request->remote_saved_sigmask = task->saved_sigmask;
	request->remote_pending = task->pending;
	request->sas_ss_sp = task->sas_ss_sp;
	request->sas_ss_size = task->sas_ss_size;
	int cnt = 0;
	for (cnt = 0; cnt < _NSIG; cnt++)
		request->action[cnt] = task->sighand->action[cnt];


{
	unsigned short fsindex, gsindex;
	unsigned short es, ds;
	unsigned long fs, gs;

	memcpy(&request->regs, regs, sizeof(struct pt_regs));
	request->thread_usersp = task->thread.usersp;

	request->old_rsp = read_old_rsp();
	request->thread_es = task->thread.es;
	savesegment(es, es);
	if ((current == task) && (es != request->thread_es)) {
		PSPRINTK("%s: es %x thread %x\n", __func__, es, request->thread_es);
	}
	request->thread_ds = task->thread.ds;
	savesegment(ds, ds);
	if (ds != request->thread_ds) {
		PSPRINTK("%s: ds %x thread %x\n", __func__, ds, request->thread_ds);
	}
	request->thread_fsindex = task->thread.fsindex;
	savesegment(fs, fsindex);
	if (fsindex != request->thread_fsindex) {
		PSPRINTK(
				"%s: fsindex %x thread %x\n", __func__, fsindex, request->thread_fsindex);
	}
	request->thread_gsindex = task->thread.gsindex;
	savesegment(gs, gsindex);
	if (gsindex != request->thread_gsindex) {
		PSPRINTK(
				"%s: gsindex %x thread %x\n", __func__, gsindex, request->thread_gsindex);
	}
	request->thread_fs = task->thread.fs;
	rdmsrl(MSR_FS_BASE, fs);
	if (fs != request->thread_fs) {
		PSPRINTK(
				"%s: fs %lx thread %lx\n", __func__, fs, request->thread_fs);
		request->thread_fs = fs;
	}

	request->thread_gs = task->thread.gs;
	rdmsrl(MSR_KERNEL_GS_BASE, gs);

	if (gs != request->thread_gs) {
		PSPRINTK(
				"%s: gs %lx thread %lx\n", __func__, gs, request->thread_gs);
		request->thread_gs = gs;
	}

#if MIGRATE_FPU
	//FPU migration code --- initiator
	PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d] %d:%d %x\n",
			__func__, task->flags, (int)task->fpu_counter, (int)task->thread.has_fpu,
			(int)__thread_has_fpu(task), (int)fpu_allocated(&task->thread.fpu),
			(int)use_xsave(), (int)use_fxsr(), (int) PF_USED_MATH);

	request->task_flags = task->flags;
	request->task_fpu_counter = task->fpu_counter;
	request->thread_has_fpu = task->thread.has_fpu;

	//    if (__thread_has_fpu(task)) {
	if (!fpu_allocated(&task->thread.fpu)){
		fpu_alloc(&task->thread.fpu);
		fpu_finit(&task->thread.fpu);
	}

	fpu_save_init(&task->thread.fpu);

	struct fpu temp; temp.state = &request->fpu_state;

	fpu_copy(&temp,&task->thread.fpu);


//    }
#endif

}



	lock_task_sighand(task, &flags);

	if (task->tgroup_distributed == 0) {
		unlock_task_sighand(task, &flags);
		printk("ERROR: back migrating thread of not tgroup_distributed process\n");
		kfree(request);
		return -1;
	}

	task->represents_remote = 1;
	task->next_cpu = task->prev_cpu;
	task->next_pid = task->prev_pid;
	task->executing_for_remote= 0;

	unlock_task_sighand(task, &flags);

	ret = pcn_kmsg_send_long(dst_cpu,(struct pcn_kmsg_long_message*) request,sizeof(clone_request_t) - sizeof(struct pcn_kmsg_hdr));

	kfree(request);

	return ret;
}
static void dump_regs(struct pt_regs* regs) {
    unsigned long fs, gs;
   printk(KERN_ALERT"DUMP REGS\n");
    if(NULL != regs) {
        printk(KERN_ALERT"r15{%lx}\n",regs->r15);
        printk(KERN_ALERT"r14{%lx}\n",regs->r14);
        printk(KERN_ALERT"r13{%lx}\n",regs->r13);
        printk(KERN_ALERT"r12{%lx}\n",regs->r12);
        printk(KERN_ALERT"r11{%lx}\n",regs->r11);
        printk(KERN_ALERT"r10{%lx}\n",regs->r10);
        printk(KERN_ALERT"r9{%lx}\n",regs->r9);
        printk(KERN_ALERT"r8{%lx}\n",regs->r8);
        printk(KERN_ALERT"bp{%lx}\n",regs->bp);
        printk(KERN_ALERT"bx{%lx}\n",regs->bx);
        printk(KERN_ALERT"ax{%lx}\n",regs->ax);
        printk(KERN_ALERT"cx{%lx}\n",regs->cx);
        printk(KERN_ALERT"dx{%lx}\n",regs->dx);
        printk(KERN_ALERT"di{%lx}\n",regs->di);
        printk(KERN_ALERT"orig_ax{%lx}\n",regs->orig_ax);
        printk(KERN_ALERT"ip{%lx}\n",regs->ip);
        printk(KERN_ALERT"cs{%lx}\n",regs->cs);
        printk(KERN_ALERT"flags{%lx}\n",regs->flags);
        printk(KERN_ALERT"sp{%lx}\n",regs->sp);
        printk(KERN_ALERT"ss{%lx}\n",regs->ss);
    }
    rdmsrl(MSR_FS_BASE, fs);
    rdmsrl(MSR_GS_BASE, gs);
    printk(KERN_ALERT"fs{%lx}\n",fs);
    printk(KERN_ALERT"gs{%lx}\n",gs);
    printk(KERN_ALERT"REGS DUMP COMPLETE\n");
}

/*
 * Send a message to <dst_cpu> for migrating a task <task>.
 * This function will ask the remote cpu to create a thread to host task.
 * It returns -1 in error case.
 */
static int do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs, int* is_first){

	clone_request_t* request;
	int tx_ret = -1;
	struct task_struct* tgroup_iterator = NULL;
	char path[256] = { 0 };
	char* rpath;
	memory_t* entry;
	int first= 0;
	unsigned long flags;

	request = kmalloc(sizeof(clone_request_t), GFP_ATOMIC);
	if (request == NULL) {
		return -1;
	}

	*is_first=0;
	// Build request
	request->header.type = PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST;
	request->header.prio = PCN_KMSG_PRIO_NORMAL;

	// struct mm_struct -----------------------------------------------------------
	rpath = d_path(&task->mm->exe_file->f_path, path, 256);
	strncpy(request->exe_path, rpath, 512);
	request->stack_start = task->mm->start_stack;
	request->start_brk = task->mm->start_brk;
	request->brk = task->mm->brk;
	request->env_start = task->mm->env_start;
	request->env_end = task->mm->env_end;
	request->arg_start = task->mm->arg_start;
	request->arg_end = task->mm->arg_end;
	request->start_code = task->mm->start_code;
	request->end_code = task->mm->end_code;
	request->start_data = task->mm->start_data;
	request->end_data = task->mm->end_data;
	request->def_flags = task->mm->def_flags;
	// struct task_struct ---------------------------------------------------------
	request->placeholder_pid = task->pid;
	request->placeholder_tgid = task->tgid;
    /*mklinux_akshay*/
    if (task->prev_pid == -1)
    	request->origin_pid = task->pid;
    else
    	request->origin_pid = task->origin_pid;
    request->remote_blocked = task->blocked;
    request->remote_real_blocked = task->real_blocked;
    request->remote_saved_sigmask = task->saved_sigmask;
    request->remote_pending = task->pending;
    request->sas_ss_sp = task->sas_ss_sp;
    request->sas_ss_size = task->sas_ss_size;
    int cnt = 0;
    for (cnt = 0; cnt < _NSIG; cnt++)
    	request->action[cnt] = task->sighand->action[cnt];

	request->back=0;

    /*mklinux_akshay*/
    if(task->prev_pid==-1)
    	task->origin_pid=task->pid;
    else
    	task->origin_pid=task->origin_pid;

	request->personality = task->personality;
	// struct thread_struct -------------------------------------------------------
	// have a look at: copy_thread() arch/x86/kernel/process_64.c
	// have a look at: struct thread_struct arch/x86/include/asm/processor.h
	{
		unsigned short fsindex, gsindex;
		unsigned short es, ds;
		unsigned long fs, gs;
		//printk("size of struct pt_regs id %d \n",sizeof(struct pt_regs));
		memcpy(&request->regs, regs, sizeof(struct pt_regs));
		request->thread_usersp = task->thread.usersp;

		request->old_rsp = read_old_rsp();
		request->thread_es = task->thread.es;
		savesegment(es, es);
		if ((current == task) && (es != request->thread_es)) {
			PSPRINTK("%s: es %x thread %x\n", __func__, es, request->thread_es);
		}
		request->thread_ds = task->thread.ds;
		savesegment(ds, ds);
		if (ds != request->thread_ds) {
			PSPRINTK("%s: ds %x thread %x\n", __func__, ds, request->thread_ds);
		}
		request->thread_fsindex = task->thread.fsindex;
		savesegment(fs, fsindex);
		if (fsindex != request->thread_fsindex) {
			PSPRINTK(
					"%s: fsindex %x thread %x\n", __func__, fsindex, request->thread_fsindex);
		}
		request->thread_gsindex = task->thread.gsindex;
		savesegment(gs, gsindex);
		if (gsindex != request->thread_gsindex) {
			PSPRINTK(
					"%s: gsindex %x thread %x\n", __func__, gsindex, request->thread_gsindex);
		}
		request->thread_fs = task->thread.fs;
		rdmsrl(MSR_FS_BASE, fs);
		if (fs != request->thread_fs) {
			PSPRINTK(
					"%s: fs %lx thread %lx\n", __func__, fs, request->thread_fs);
			request->thread_fs = fs;
		}

		request->thread_gs = task->thread.gs;
		rdmsrl(MSR_KERNEL_GS_BASE, gs);

		if (gs != request->thread_gs) {
			PSPRINTK(
					"%s: gs %lx thread %lx\n", __func__, gs, request->thread_gs);
			request->thread_gs = gs;
		}

#if MIGRATE_FPU
		//FPU migration code --- initiator
		PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d] %d:%d %x\n",
				__func__, task->flags, (int)task->fpu_counter, (int)task->thread.has_fpu,
				(int)__thread_has_fpu(task), (int)fpu_allocated(&task->thread.fpu),
				(int)use_xsave(), (int)use_fxsr(), (int) PF_USED_MATH);

		request->task_flags = task->flags;
		request->task_fpu_counter = task->fpu_counter;
		request->thread_has_fpu = task->thread.has_fpu;

		//    if (__thread_has_fpu(task)) {
		if (!fpu_allocated(&task->thread.fpu)){
			fpu_alloc(&task->thread.fpu);
			fpu_finit(&task->thread.fpu);
		}

		fpu_save_init(&task->thread.fpu);

		struct fpu temp; temp.state = &request->fpu_state;

		fpu_copy(&temp,&task->thread.fpu);


		//    }
#endif
	}

	/*I use siglock to coordinate the thread group.
	 *This process is becoming a distributed one if it was not already.
	 *The first migrating thread has to create the memory entry to handle page requests,
	 *and fork the main kernel thread of this process.
	 */
	lock_task_sighand(task, &flags);

	if (task->tgroup_distributed == 0) {

		task->tgroup_distributed = 1;
		task->tgroup_home_id = task->tgid;
		task->tgroup_home_cpu = _cpu;

		entry = (memory_t*) kmalloc(sizeof(memory_t), GFP_ATOMIC);
		if (!entry){
			unlock_task_sighand(task, &flags);
			printk("ERROR: Impossible allocate memory_t while migrating thread\n");
			return -1;
		}

		entry->mm = task->mm;
		atomic_inc(&entry->mm->mm_users);
		entry->tgroup_home_cpu = task->tgroup_home_cpu;
		entry->tgroup_home_id = task->tgroup_home_id;
		entry->next = NULL;
		entry->prev = NULL;
		entry->alive = 1;
		entry->main = NULL;
		atomic_set(&(entry->pending_migration),0);
		entry->operation = VMA_OP_NOP;
		entry->waiting_for_main = NULL;
		entry->waiting_for_op = NULL;
		entry->arrived_op = 0;
		entry->my_lock = 0;
		memset(entry->kernel_set,0,MAX_KERNEL_IDS*sizeof(int));
		entry->kernel_set[_cpu]= 1;
		init_rwsem(&entry->kernel_set_sem);
		entry->setting_up= 0;
		init_rwsem(&task->mm->distribute_sem);
		task->mm->distr_vma_op_counter = 0;
		task->mm->was_not_pushed = 0;
		task->mm->thread_op = NULL;
		task->mm->vma_operation_index = 0;
		task->mm->distribute_unmap = 1;

		add_memory_entry(entry);

		first=1;
		*is_first = 1;

		tgroup_iterator = task;
		while_each_thread(task, tgroup_iterator)
		{
			tgroup_iterator->tgroup_home_id = task->tgroup_home_id;
			tgroup_iterator->tgroup_home_cpu = task->tgroup_home_cpu;
			tgroup_iterator->tgroup_distributed = 1;
		};

	}

	task->represents_remote = 1;

	unlock_task_sighand(task, &flags);

	request->tgroup_home_cpu = task->tgroup_home_cpu;
	request->tgroup_home_id = task->tgroup_home_id;

	//dump_regs(&request->regs);
	tx_ret = pcn_kmsg_send_long(dst_cpu,
			(struct pcn_kmsg_long_message*) request,
			sizeof(clone_request_t) - sizeof(struct pcn_kmsg_hdr));

	if (first)
		kernel_thread(
				create_kernel_thread_for_distributed_process_from_user_one,
				entry, CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | SIGCHLD);

	kfree(request);

	return tx_ret;
}

/**
 * Migrate the specified task <task> to cpu <dst_cpu>
 * Currently, this function will put the specified task to
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then create a new thread and import that
 * info into its new context.
 *
 * It returns PROCESS_SERVER_CLONE_FAIL in case of error,
 * PROCESS_SERVER_CLONE_SUCCESS otherwise.
 */
int process_server_do_migration(struct task_struct* task, int dst_cpu,
		struct pt_regs * regs) {

	int first = 0;
	int back= 0;
	int ret= 0;

	//printk("%s : migrating pid %d tgid %d task->tgroup_home_id %d task->tgroup_home_cpu %d\n",__func__,current->pid,current->tgid,task->tgroup_home_id,task->tgroup_home_cpu);

#if TIMING
	unsigned long long start= native_read_tsc();
#endif


	/*	sched.c changed so this is not needed anymore
	 *
	 * #ifndef SUPPORT_FOR_CLUSTERING
	// Nothing to do if we're migrating to the current cpu
	if (dst_cpu == _cpu) {
		return PROCESS_SERVER_CLONE_FAIL;
	}
#else
    if (cpumask_test_cpu(dst_cpu, cpu_present_mask)) {
         printk(KERN_ERR"%s: called but task %p does not require inter-kernel migration"
                        "(cpu: %d present_mask)\n", __func__, task, dst_cpu);
         return -EBUSY;
     }
     // TODO seems like that David is using previous_cpus as a bitmask..
     // TODO well this must be upgraded to a cpumask, declared as usigned long in task_struct
     struct list_head *iter;
     _remote_cpu_info_list_t *objPtr;
     struct cpumask *pcpum =0;
     int cpuid=-1;
     list_for_each(iter, &rlist_head) {
         objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
         cpuid = objPtr->_data._processor;
         pcpum = &(objPtr->_data.cpumask);
         if (cpumask_test_cpu(dst_cpu, pcpum)) {
                 dst_cpu= cpuid;
         }
     }
#endif*/


	if(task->prev_cpu==dst_cpu){
		back= 1;
		ret= do_back_migration(task, dst_cpu, regs);
		if(ret==-1)
			return PROCESS_SERVER_CLONE_FAIL;
	}
	else{
		ret= do_migration(task, dst_cpu, regs,&first);
	}


#if TIMING
	if(ret!=-1){
		unsigned long long stop= native_read_tsc();
		unsigned long long elapsed_time =stop-start;

		if(first)
			update_time_migration(elapsed_time,FIRST_MIG_WITH_FORK);
		else
			if(back)
				update_time_migration(elapsed_time,BACK_MIG);
			else
				update_time_migration(elapsed_time,NORMAL_MIG);

	}
	else
		printk("WARNING in timing for migration ret is %d\n",ret);

#endif

if (ret != -1) {
	//	printk(KERN_ALERT"%s clone request sent ret{%d} \n", __func__,ret);

	__set_task_state(task, TASK_UNINTERRUPTIBLE);

	return PROCESS_SERVER_CLONE_SUCCESS;

} else

	return PROCESS_SERVER_CLONE_FAIL;

}

void process_vma_op(struct work_struct* work) {

#if	NOT_REPLICATED_VMA_MANAGEMENT

	vma_op_work_t* vma_work = (vma_op_work_t*) work;
	vma_operation_t* operation = vma_work->operation;
	memory_t* memory = vma_work->memory;

	//to coordinate with dead of process
	if (vma_work->fake == 1) {
		unsigned long flags;
		memory->arrived_op = 1;
		lock_task_sighand(memory->main, &flags);
		memory->main->distributed_exit = EXIT_FLUSHING;
		unlock_task_sighand(memory->main, &flags);
		wake_up_process(memory->main);
		kfree(work);
		return;
	}

	struct mm_struct* mm = memory->mm;

	PSVMAPRINTK("Received vma operation from cpu %d for tgroup_home_cpu %i tgroup_home_id %i operation %i\n", operation->header.from_cpu, operation->tgroup_home_cpu, operation->tgroup_home_id, operation->operation);

	down_write(&mm->mmap_sem);

	if (_cpu == operation->tgroup_home_cpu) {

		PSVMAPRINTK("SERVER\n");
		//SERVER

		//if another operation is on going, it will be serialized after.
		while (mm->distr_vma_op_counter > 0) {

			PSVMAPRINTK("%s, A distributed operation already started, going to sleep\n",__func__);
			up_write(&mm->mmap_sem);

			DEFINE_WAIT(wait);
			prepare_to_wait(&request_distributed_vma_op, &wait,
					TASK_UNINTERRUPTIBLE);

			if (mm->distr_vma_op_counter > 0) {
				schedule();
			}

			finish_wait(&request_distributed_vma_op, &wait);

			down_write(&mm->mmap_sem);

		}

		if (mm->distr_vma_op_counter != 0 || mm->was_not_pushed != 0) {
			up_write(&mm->mmap_sem);
			printk("ERROR: handling a new vma operation but mm->distr_vma_op_counter is %i and mm->was_not_pushed is %i \n",
					mm->distr_vma_op_counter, mm->was_not_pushed);
			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		PSVMAPRINTK("%s,SERVER: Starting operation %i for cpu %i\n",__func__, operation->operation, operation->header.from_cpu);

		mm->distr_vma_op_counter++;
		//the main kernel thread will execute the local operation
		while(memory->main==NULL)
			schedule();

		mm->thread_op = memory->main;

		if (operation->operation == VMA_OP_MAP
				|| operation->operation == VMA_OP_BRK) {
			mm->was_not_pushed++;
		}

		up_write(&mm->mmap_sem);

		//wake up the main thread to execute the operation locally
		memory->message_push_operation = operation;
		memory->addr = operation->addr;
		memory->len = operation->len;
		memory->prot = operation->prot;
		memory->new_addr = operation->new_addr;
		memory->new_len = operation->new_len;
		memory->flags = operation->flags;
		memory->pgoff = operation->pgoff;
		strcpy(memory->path, operation->path);
		memory->waiting_for_main = current;
		//This is the field check by the main thread
		//so it is the last one to be populated
		memory->operation = operation->operation;

		wake_up_process(memory->main);

		PSVMAPRINTK("%s,SERVER: woke up the main\n",__func__);

		while (memory->operation != VMA_OP_NOP) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if (memory->operation != VMA_OP_NOP) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);

		}

		down_write(&mm->mmap_sem);

		mm->distr_vma_op_counter--;
		if (mm->distr_vma_op_counter != 0)
			printk(	"ERROR: exiting from distributed operation but mm->distr_vma_op_counter is %i \n",
					mm->distr_vma_op_counter);
		if (operation->operation == VMA_OP_MAP
				|| operation->operation == VMA_OP_BRK) {
			mm->was_not_pushed--;
			if (mm->was_not_pushed != 0)
				printk("ERROR: exiting from distributed operation but mm->was_not_pushed is %i \n",
						mm->distr_vma_op_counter);
		}

		mm->thread_op = NULL;

		up_write(&mm->mmap_sem);

		wake_up(&request_distributed_vma_op);

		pcn_kmsg_free_msg(operation);
		kfree(work);
		PSVMAPRINTK("SERVER: vma_operation_index is %d\n",mm->vma_operation_index);
		PSVMAPRINTK("%s, SERVER: end requested operation\n",__func__);

		return ;

	} else {
		PSVMAPRINTK("%s, CLIENT: Starting operation %i of index %i\n ",__func__, operation->operation, operation->vma_operation_index);
		//CLIENT

		//NOTE: the current->mm->distribute_sem is already held

		//MMAP and BRK are not pushed in the system
		//if I receive one of them I must have initiate it
		if (operation->operation == VMA_OP_MAP
				|| operation->operation == VMA_OP_BRK) {

			if (memory->my_lock != 1) {
				printk("ERROR: wrong distributed lock aquisition\n");
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (operation->from_cpu != _cpu) {
				printk("ERROR: the server pushed me an operation %i of cpu %i\n",
						operation->operation, operation->from_cpu);
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (memory->waiting_for_op == NULL) {
				printk(	"ERROR:received a push operation started by me but nobody is waiting\n");
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			memory->addr = operation->addr;
			memory->arrived_op = 1;
			//mm->vma_operation_index = operation->vma_operation_index;
			PSVMAPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
			PSVMAPRINTK("%s, CLIENT: Operation %i started by a local thread pid %d\n ",__func__,operation->operation,memory->waiting_for_op->pid);
			up_write(&mm->mmap_sem);

			wake_up_process(memory->waiting_for_op);

			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;

		}

		//I could have started the operation...check!
		if (operation->from_cpu == _cpu) {

			if (memory->my_lock != 1) {
				printk("ERROR: wrong distributed lock aquisition\n");
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (memory->waiting_for_op == NULL) {
				printk(
						"ERROR:received a push operation started by me but nobody is waiting\n");
				up_write(&mm->mmap_sem);
				pcn_kmsg_free_msg(operation);
				kfree(work);
				return ;
			}

			if (operation->operation == VMA_OP_REMAP)
				memory->addr = operation->new_addr;

			memory->arrived_op = 1;
			//mm->vma_operation_index = operation->vma_operation_index;
			PSVMAPRINTK("%s, CLIENT: Operation %i started by a local thread pid %d\n ",__func__,operation->operation,memory->waiting_for_op->pid);
			PSVMAPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
			up_write(&mm->mmap_sem);

			wake_up_process(memory->waiting_for_op);

			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		PSVMAPRINTK("%s, CLIENT Pushed operation started by somebody else\n",__func__);

		if (operation->addr < 0) {
			printk("WARNING: %s, server sent me and error\n",__func__);
			pcn_kmsg_free_msg(operation);
			kfree(work);
			return ;
		}

		mm->distr_vma_op_counter++;
		struct task_struct *prev = mm->thread_op;

		while(memory->main==NULL)
			schedule();

		mm->thread_op = memory->main;
		up_write(&mm->mmap_sem);

		//wake up the main thread to execute the operation locally
		memory->addr = operation->addr;
		memory->len = operation->len;
		memory->prot = operation->prot;
		memory->new_addr = operation->new_addr;
		memory->new_len = operation->new_len;
		memory->flags = operation->flags;

		//the new_addr sent by the server is fixed
		if (operation->operation == VMA_OP_REMAP)
			memory->flags |= MREMAP_FIXED;

		memory->waiting_for_main = current;
		memory->operation = operation->operation;

		wake_up_process(memory->main);

		PSVMAPRINTK("%s,CLIENT: woke up the main\n",__func__);

		while (memory->operation != VMA_OP_NOP) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if (memory->operation != VMA_OP_NOP) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);

		}

		down_write(&mm->mmap_sem);
		memory->waiting_for_main = NULL;
		mm->thread_op = prev;
		mm->distr_vma_op_counter--;

		//mm->vma_operation_index = operation->vma_operation_index;
		PSVMAPRINTK("CLIENT: Incremeting vma_operation_index\n");
		mm->vma_operation_index++;

		if (memory->my_lock != 1) {
			PSVMAPRINTK("Released distributed lock\n");
			up_write(&mm->distribute_sem);
		}

		PSVMAPRINTK("CLIENT: vma_operation_index is %d\n",mm->vma_operation_index);
		PSVMAPRINTK("CLIENT: Ending distributed vma operation\n");
		up_write(&mm->mmap_sem);

		wake_up(&request_distributed_vma_op);

		pcn_kmsg_free_msg(operation);
		kfree(work);

		return ;

	}
#else

#if PARTIAL_VMA_MANAGEMENT
	vma_unmap_work_t* vma_work = (vma_unmap_work_t*) work;
	unmap_message_t* unmap = vma_work->unmap;

	memory_t* memory;

	//to coordinate with dead of process

	if (vma_work->fake == 1) {

		memory = vma_work->memory;
		unsigned long flags;
		memory->arrived_op = 1;
		lock_task_sighand(memory->main, &flags);
		memory->main->distributed_exit = EXIT_FLUSHING;
		unlock_task_sighand(memory->main, &flags);
		wake_up_process(memory->main);
		kfree(work);
		return ;
	}

	PSPRINTK("Staring unmap\n");
	memory = find_memory_entry(unmap->tgroup_home_cpu,
			unmap->tgroup_home_id);

	if (memory != NULL) {
		//the main should do it, but to be compliant I'm doing it here
		down_write(&memory->mm->mmap_sem);
		memory->mm->distribute_unmap = 0;
		do_munmap(memory->mm, unmap->start, unmap->len);
		memory->mm->distribute_unmap = 1;
		up_write(&memory->mm->mmap_sem);
	}

	vma_ack_t* ack = (vma_ack_t*) kmalloc(sizeof(vma_ack_t), GFP_ATOMIC);
	if (ack == NULL)
		return ;
	ack->tgroup_home_cpu = unmap->tgroup_home_cpu;
	ack->tgroup_home_id = unmap->tgroup_home_id;
	ack->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_ACK;
	ack->header.prio = PCN_KMSG_PRIO_NORMAL;

	pcn_kmsg_send_long(unmap->header.from_cpu,
			(struct pcn_kmsg_long_message*) (ack),
			sizeof(vma_ack_t) - sizeof(struct pcn_kmsg_hdr));

	PSPRINTK("Operation done\n");

	pcn_kmsg_free_msg(unmap);
	kfree(ack);
	kfree(work);
	return ;
#endif
#endif

}

void process_vma_lock(struct work_struct* work) {
	vma_lock_work_t* vma_lock_work = (vma_lock_work_t*) work;
	vma_lock_t* lock = vma_lock_work->lock;

	memory_t* entry = find_memory_entry(lock->tgroup_home_cpu,
			lock->tgroup_home_id);
	if (entry != NULL) {
		down_write(&entry->mm->distribute_sem);
		PSVMAPRINTK("Acquired distributed lock\n");
		if (lock->from_cpu == _cpu)
			entry->my_lock = 1;
	}

	vma_ack_t* ack_to_server = (vma_ack_t*) kmalloc(sizeof(vma_ack_t),
			GFP_ATOMIC);
	if (ack_to_server == NULL)
		return ;
	ack_to_server->tgroup_home_cpu = lock->tgroup_home_cpu;
	ack_to_server->tgroup_home_id = lock->tgroup_home_id;
	ack_to_server->vma_operation_index = lock->vma_operation_index;
	ack_to_server->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_ACK;
	ack_to_server->header.prio = PCN_KMSG_PRIO_NORMAL;

	pcn_kmsg_send_long(lock->tgroup_home_cpu,
			(struct pcn_kmsg_long_message*) (ack_to_server),
			sizeof(vma_ack_t) - sizeof(struct pcn_kmsg_hdr));

	kfree(ack_to_server);
	pcn_kmsg_free_msg(lock);
	kfree(work);

	return ;

}

static int handle_vma_lock(struct pcn_kmsg_message* inc_msg) {
	vma_lock_t* lock = (vma_lock_t*) inc_msg;
	vma_lock_work_t* work;

	work = kmalloc(sizeof(vma_lock_work_t), GFP_ATOMIC);

	if (work) {
		work->lock = lock;
		INIT_WORK( (struct work_struct*)work, process_vma_lock);
		queue_work(vma_lock_wq, (struct work_struct*) work);
	}

	else {
		pcn_kmsg_free_msg(lock);
	}
	return 1;

}

static int handle_vma_ack(struct pcn_kmsg_message* inc_msg) {
	vma_ack_t* ack = (vma_ack_t*) inc_msg;
	vma_op_answers_t* ack_holder;
	unsigned long flags;
	struct task_struct* task_to_wake_up = NULL;

	PSVMAPRINTK("Vma ack received from cpu %d\n", ack->header.from_cpu);
	ack_holder = find_vma_ack_entry(ack->tgroup_home_cpu, ack->tgroup_home_id);
	if (ack_holder) {

		raw_spin_lock_irqsave(&(ack_holder->lock), flags);

		ack_holder->responses++;

#if	NOT_REPLICATED_VMA_MANAGEMENT

		ack_holder->address = ack->addr;

		if (ack_holder->vma_operation_index == -1)
			ack_holder->vma_operation_index = ack->vma_operation_index;
		else if (ack_holder->vma_operation_index != ack->vma_operation_index)
			printk(
					"ERROR: receiving an ack vma for a different operation index\n");

#else

#if PARTIAL_VMA_MANAGEMENT
		//nothing to do
#endif

#endif
		if (ack_holder->responses >= ack_holder->expected_responses)
			task_to_wake_up = ack_holder->waiting;

		raw_spin_unlock_irqrestore(&(ack_holder->lock), flags);

		if (task_to_wake_up)
			wake_up_process(task_to_wake_up);

	}

	pcn_kmsg_free_msg(inc_msg);

	return 1;
}

static int handle_vma_op(struct pcn_kmsg_message* inc_msg) {

#if	NOT_REPLICATED_VMA_MANAGEMENT

	vma_operation_t* operation = (vma_operation_t*) inc_msg;
	vma_op_work_t* work;

	PSVMAPRINTK("Received an operation\n");

	memory_t* memory = find_memory_entry(operation->tgroup_home_cpu,
			operation->tgroup_home_id);
	if (memory != NULL) {

		work = kmalloc(sizeof(vma_op_work_t), GFP_ATOMIC);

		if (work) {
			work->fake = 0;
			work->memory = memory;
			work->operation = operation;
			INIT_WORK( (struct work_struct*)work, process_vma_op);
			queue_work(vma_op_wq, (struct work_struct*) work);
		}

	} else {
		if (operation->tgroup_home_cpu == _cpu)
			printk(
					"ERROR: received an operation that said that I am the server but no memory_t found\n");
		else {
			PSVMAPRINTK(
					"Received an operation for a distributed process not present here\n");
		}
		pcn_kmsg_free_msg(inc_msg);
	}

	return 1;

#else

#if PARTIAL_VMA_MANAGEMENT

	unmap_message_t* unmap = (unmap_message_t*) inc_msg;
	vma_unmap_work_t* work;

	PSPRINTK("Received an unmap from cpu %d\n", unmap->header.from_cpu);

	work = kmalloc(sizeof(vma_unmap_work_t), GFP_ATOMIC);

	if (work) {
		work->fake = 0;
		work->unmap = unmap;
		INIT_WORK( (struct work_struct*)work, process_vma_op);
		queue_work(vma_op_wq, (struct work_struct*) work);
	} else {
		pcn_kmsg_free_msg(inc_msg);
	}

	return 1;
#endif
#endif
}

void end_distribute_operation(int operation, long start_ret, unsigned long addr) {
	int i;

	if (current->mm->distribute_unmap == 0) {
		return;
	}

	PSVMAPRINTK("Ending distributed vma operation %i pid %d\n", operation,current->pid);

	if (current->mm->distr_vma_op_counter <= 0
			|| (current->main == 0 && current->mm->distr_vma_op_counter > 2)
			|| (current->main == 1 && current->mm->distr_vma_op_counter > 3))
		printk("ERROR: exiting from a distributed vma operation with distr_vma_op_counter = %i\n",
				current->mm->distr_vma_op_counter);

	(current->mm->distr_vma_op_counter)--;

	if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		if (current->mm->was_not_pushed <= 0)
			printk("ERROR: exiting from a mapping operation with was_not_pushed = %i\n",
					current->mm->was_not_pushed);
		current->mm->was_not_pushed--;
	}

	memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
			current->tgroup_home_id);
	if (entry == NULL) {
		printk("ERROR: Cannot find message to send in exit operation\n");
	}

	if (start_ret == VMA_OP_SAVE) {

		/*if(operation!=VMA_OP_MAP ||operation!=VMA_OP_REMAP ||operation!=VMA_OP_BRK )
		 printk("ERROR: asking for saving address from operation %i",operation);
		 */
		if (_cpu != current->tgroup_home_cpu)
			printk("ERROR: asking for saving address from a client");

		//now I have the new address I can send the message
		if (entry->message_push_operation != NULL) {

			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (current->main == 0)
					printk("ERROR: server not main asked to save operation\n");

				entry->message_push_operation->addr = addr;
			} else {

				if (operation == VMA_OP_REMAP) {
					entry->message_push_operation->new_addr = addr;
				} else
					printk("ERROR: asking for saving address from a wrong operation\n");
			}

			up_write(&current->mm->mmap_sem);

			if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
				if (pcn_kmsg_send_long(entry->message_push_operation->from_cpu,
						(struct pcn_kmsg_long_message*) (entry->message_push_operation),
						sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr))
						== -1) {
					printk("ERROR: impossible to send operation to client in cpu %d\n",
							entry->message_push_operation->from_cpu);
				} else {
					PSVMAPRINTK("%s, operation %d sent to cpu %d \n",__func__,operation, entry->message_push_operation->from_cpu);
				}

			} else {
 				PSVMAPRINTK("%s,sending operation %d to all \n",__func__,operation);
#ifndef SUPPORT_FOR_CLUSTERING
				for(i = 0; i < MAX_KERNEL_IDS; i++) {
					// Skip the current cpu
					if(i == _cpu) continue;

#else
				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
#endif
					if(entry->kernel_set[i]==1)
						pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*) (entry->message_push_operation),
							sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));

				}
			}

			down_write(&current->mm->mmap_sem);

			if (current->main == 0) {

				kfree(entry->message_push_operation);
				entry->message_push_operation = NULL;
			}

		} else {
			printk("ERROR: Cannot find message to send in exit operation\n");

		}
	}

	if (current->mm->distr_vma_op_counter == 0) {

		current->mm->thread_op = NULL;

		entry->my_lock = 0;

		if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
				PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
				current->mm->vma_operation_index++;
		}

		PSVMAPRINTK("Releasing distributed lock\n");
		up_write(&current->mm->distribute_sem);

		if(_cpu == current->tgroup_home_cpu && !(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
			up_read(&entry->kernel_set_sem);
		}

		wake_up(&request_distributed_vma_op);

	} else
		 if (current->mm->distr_vma_op_counter == 1
				&& _cpu == current->tgroup_home_cpu && current->main == 1) {

			if(!(operation == VMA_OP_MAP || operation == VMA_OP_BRK)){
				PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
				current->mm->vma_operation_index++;
				up_read(&entry->kernel_set_sem);
			}

			PSVMAPRINTK("Releasing distributed lock\n");
			up_write(&current->mm->distribute_sem);

		}else{
			if (!(current->mm->distr_vma_op_counter == 1
                                && _cpu != current->tgroup_home_cpu && current->main == 1)) {

				//nested operation
				if(operation != VMA_OP_UNMAP)
					printk("ERROR exiting fom a nest operation that is %d",operation);

				//nested operation do not release the lock
			 	PSVMAPRINTK("%s incrementing vma_operation_index\n",__func__);
                         	current->mm->vma_operation_index++;
			}

		}

	PSVMAPRINTK("operation index is %d\n", current->mm->vma_operation_index);
}

	/*I assume that down_write(&mm->mmap_sem) is held
	 *There are two different protocols:
	 *mmap and brk need to only contact the server,
	 *all other operations (remap, mprotect, unmap) need that the server pushes it in the system
	 */
	long start_distribute_operation(int operation, unsigned long addr, size_t len,
			unsigned long prot, unsigned long new_addr, unsigned long new_len,
			unsigned long flags, struct file *file, unsigned long pgoff) {

		long ret;
		int server;

		if (current->tgroup_home_cpu != _cpu)
			server = 0;
		else
			server = 1;

		//set default ret
		if (server)
			ret = VMA_OP_NOT_SAVE;
		else if (operation == VMA_OP_REMAP)
			ret = new_addr;
		else
			ret = addr;

		/*Operations can be nested-called.
		 * MMAP->UNMAP
		 * BR->UNMAP
		 * MPROT->/
		 * UNMAP->/
		 * MREMAP->UNMAP
		 * =>only UNMAP can be nested-called
		 *
		 * If this is an unmap nested-called by an operation pushed in the system,
		 * skip the distribution part.
		 *
		 * If this is an unmap nested-called by an operation not pushed in the system,
		 * and I am the server, push it in the system.
		 *
		 * If this is an unmap nested-called by an operation not pushed in the system,
		 * and I am NOT the server, it is an error. The server should have pushed that unmap
		 * before, if I am executing it again something is wrong.
		 */

		/*All the operation pushed by the server are executed as not distributed in clients*/
		if (current->mm->distribute_unmap == 0) {
			return ret;
		}

		PSVMAPRINTK("%s, Starting vma operation for pid %i tgroup_home_cpu %i tgroup_home_id %i main %d operation %i addr %lu len %lu end %lu\n",
				__func__, current->pid, current->tgroup_home_cpu, current->tgroup_home_id, current->main?1:0, operation, addr, len, addr+len);


		/*only server can have legal distributed nested operations*/
		if (current->mm->distr_vma_op_counter > 0
				&& current->mm->thread_op == current) {


			PSVMAPRINTK("%s, Recursive operation\n",__func__);

			if (server == 0
					|| (current->main == 0 && current->mm->distr_vma_op_counter > 1)
					|| (current->main == 0 && operation != VMA_OP_UNMAP)) {
				printk("ERROR: invalid nested vma operation %i\n", operation);
				return -EPERM;
			} else
				/*the main executes the operations for the clients
				 *distr_vma_op_counter is already increased when it start the operation*/
				if (current->main == 1) {

					PSVMAPRINTK("%s, I am the main, so it maybe not a real recursive operation...\n",__func__);

					if (current->mm->distr_vma_op_counter < 1
							|| current->mm->distr_vma_op_counter > 2
							|| (current->mm->distr_vma_op_counter == 2
									&& operation != VMA_OP_UNMAP)) {
						printk("ERROR: invalid nested vma operation in main server\n");
						return -EPERM;
					} else
						if (current->mm->distr_vma_op_counter == 2){

							PSVMAPRINTK("%s, Recursive operation for the main\n",__func__);
							/*in this case is a nested operation on main
							 * if the previous operation was a pushed operation
							 * do not distribute it again*/
							if (current->mm->was_not_pushed == 0) {
								current->mm->distr_vma_op_counter++;
								PSVMAPRINTK("%s, don't ditribute again, return!\n",__func__);
								return ret;
							} else
								goto start;
						}
						else
							goto start;
				} else
					if (current->mm->was_not_pushed == 0) {
						current->mm->distr_vma_op_counter++;
						PSVMAPRINTK("%s, don't ditribute again, return!\n",__func__);
						return ret;
					} else
						goto start;

		}

		/* I did not start an operation, but another thread maybe did...
		 * => no concurrent operations of the same process on the same kernel*/
		while (current->mm->distr_vma_op_counter > 0) {

			PSVMAPRINTK("%s Somebody already started a distributed operation (current->mm->thread_op->pid is %d). I am pid %d and I am going to sleep\n",
				__func__,current->mm->thread_op->pid,current->pid);

			up_write(&current->mm->mmap_sem);

			DEFINE_WAIT(wait);
			prepare_to_wait(&request_distributed_vma_op, &wait,
					TASK_UNINTERRUPTIBLE);

			if (current->mm->distr_vma_op_counter > 0) {
				schedule();
			}

			finish_wait(&request_distributed_vma_op, &wait);

			down_write(&current->mm->mmap_sem);

		}

		if (current->mm->distr_vma_op_counter != 0) {
			printk("ERROR: a new vma operation can be started, but distr_vma_op_counter is %i\n",
					current->mm->distr_vma_op_counter);
			return -EPERM;
		}

		start: current->mm->distr_vma_op_counter++;
		current->mm->thread_op = current;

		if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
			current->mm->was_not_pushed++;
		}

		if (server) {
			//SERVER

			if (current->main == 1 && !(current->mm->distr_vma_op_counter>2)) {

				/*I am the main thread=>
				 * a client asked me to do an operation.
				 */

				//(current->mm->vma_operation_index)++;
				int index = current->mm->vma_operation_index;

				PSVMAPRINTK("SERVER MAIN: starting operation %d, current index is %d \n", operation, index);

				up_write(&current->mm->mmap_sem);

				memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
						current->tgroup_home_id);
				if (entry == NULL || entry->message_push_operation == NULL) {
					printk("ERROR: Mapping disappeared or cannot find message to update \n");
					down_write(&current->mm->mmap_sem);
					ret = -ENOMEM;
					goto out;
				}

				/*First: send a message to everybody to acquire the lock to block page faults*/
				vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
						GFP_ATOMIC);
				if (lock_message == NULL) {
					down_write(&current->mm->mmap_sem);
					ret = -ENOMEM;
					goto out;
				}
				lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
				lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
				lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
				lock_message->tgroup_home_id = current->tgroup_home_id;
				lock_message->from_cpu = entry->message_push_operation->from_cpu;
				lock_message->vma_operation_index = index;

				vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
						sizeof(vma_op_answers_t), GFP_ATOMIC);
				if (acks == NULL) {
					kfree(lock_message);
					down_write(&current->mm->mmap_sem);
					ret = -ENOMEM;
					goto out;
				}
				acks->tgroup_home_cpu = current->tgroup_home_cpu;
				acks->tgroup_home_id = current->tgroup_home_id;
				acks->vma_operation_index = index;
				acks->waiting = current;
				acks->responses = 0;
				acks->expected_responses = 0;
				raw_spin_lock_init(&(acks->lock));

				add_vma_ack_entry(acks);

				int i, error;

				/*Partial replication: mmap and brk need to communicate only between server and one client
				 * */
				 if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {

					 error = pcn_kmsg_send_long(
							 entry->message_push_operation->from_cpu,
							 (struct pcn_kmsg_long_message*) (lock_message),
							 sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
					 if (error != -1) {
						 acks->expected_responses++;
					 }

				 } else {

					 down_read(&entry->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
					 for(i = 0; i < MAX_KERNEL_IDS; i++) {
						 // Skip the current cpu
						 if(i == _cpu) continue;

#else
						 // the list does not include the current processor group descirptor (TODO)
					 struct list_head *iter;
					 _remote_cpu_info_list_t *objPtr;
					 list_for_each(iter, &rlist_head) {
						 objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						 i = objPtr->_data._processor;
#endif
						if(entry->kernel_set[i]==1){
							if (current->mm->distr_vma_op_counter == 3
								&& i == entry->message_push_operation->from_cpu)
								continue;

								error = pcn_kmsg_send_long(i,
									(struct pcn_kmsg_long_message*) (lock_message),sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
								if (error != -1) {
									acks->expected_responses++;
								}
							}
						 }
				}

				 while (acks->expected_responses != acks->responses) {

					 set_task_state(current, TASK_UNINTERRUPTIBLE);

					 if (acks->expected_responses != acks->responses) {
						 schedule();
					 }

					 set_task_state(current, TASK_RUNNING);

				 }

				 PSVMAPRINTK("SERVER MAIN: Received all ack to lock\n");

				 unsigned long flags;
				 raw_spin_lock_irqsave(&(acks->lock), flags);
				 raw_spin_unlock_irqrestore(&(acks->lock), flags);

				 remove_vma_ack_entry(acks);

				 entry->message_push_operation->vma_operation_index = index;

				 /*I acquire the lock to block page faults too
				  *Important: this should happen before sending the push message or executing the operation*/
				 if (current->mm->distr_vma_op_counter == 2) {
					 down_write(&current->mm->distribute_sem);
					 PSVMAPRINTK("local distributed lock acquired\n");
				 }

				 /* Third: push the operation to everybody
				  * If the operation was a mmap,brk or remap without fixed parameters, I cannot let other kernels
				  * locally choose where to remap it =>
				  * I need to push what the server choose as parameter to the other an push the operation with
				  * a fixed flag.
				  * */
				 if (operation == VMA_OP_UNMAP || operation == VMA_OP_PROTECT
						 || ((operation == VMA_OP_REMAP) && (flags & MREMAP_FIXED))) {

					 PSVMAPRINTK("SERVER MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
#ifndef SUPPORT_FOR_CLUSTERING
					 for(i = 0; i < MAX_KERNEL_IDS; i++) {
						 // Skip the current cpu
						 if(i == _cpu) continue;
#else
					 // the list does not include the current processor group descirptor (TODO)
					struct list_head *iter;
					 _remote_cpu_info_list_t *objPtr;
					 list_for_each(iter, &rlist_head) {
						 objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						 i = objPtr->_data._processor;
#endif
						if(entry->kernel_set[i]==1)
							pcn_kmsg_send_long(i,
								(struct pcn_kmsg_long_message*) (entry->message_push_operation),sizeof(vma_operation_t)- sizeof(struct pcn_kmsg_hdr));

					 }

					 kfree(lock_message);
					 kfree(acks);

					 down_write(&current->mm->mmap_sem);

					 return ret;

				} else {
					 PSVMAPRINTK("SERVER MAIN: going to execute the operation locally %d\n",operation);
					 kfree(lock_message);
					 kfree(acks);

					 down_write(&current->mm->mmap_sem);

					 return VMA_OP_SAVE;

				 }

			 } else {
				//server not main
				 PSVMAPRINTK("SERVER NOT MAIN: Starting operation %d for pid %d current index is %d\n", operation, current->pid, current->mm->vma_operation_index);

				 switch (operation) {

						 case VMA_OP_MAP:
						 case VMA_OP_BRK:
							 //if I am the server, mmap and brk can be executed locally
							PSVMAPRINTK("%s pure local operation!\n",__func__);
							 //Note: the order in which locks are taken is important
							 up_write(&current->mm->mmap_sem);

							 down_write(&current->mm->distribute_sem);
							 PSVMAPRINTK("Distributed lock acquired\n");
							 down_write(&current->mm->mmap_sem);

							 //(current->mm->vma_operation_index)++;

							 return ret;

						 default:
							 break;

				}

				 //new push-operation
				PSVMAPRINTK("%s push operation!\n",__func__);

				 //(current->mm->vma_operation_index)++;
				 int index = current->mm->vma_operation_index;
				 PSVMAPRINTK("current index is %d\n", index);

				 /*Important: while I am waiting for the acks to the LOCK message
					  * mmap_sem have to be unlocked*/
				 up_write(&current->mm->mmap_sem);

				 /*First: send a message to everybody to acquire the lock to block page faults*/
				 vma_lock_t* lock_message = (vma_lock_t*) kmalloc(sizeof(vma_lock_t),
							 GFP_ATOMIC);
				 if (lock_message == NULL) {
					 down_write(&current->mm->mmap_sem);
					 ret = -ENOMEM;
					 goto out;
				 }
				 lock_message->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK;
				 lock_message->header.prio = PCN_KMSG_PRIO_NORMAL;
				 lock_message->tgroup_home_cpu = current->tgroup_home_cpu;
				 lock_message->tgroup_home_id = current->tgroup_home_id;
				 lock_message->from_cpu = _cpu;
				 lock_message->vma_operation_index = index;

				 vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
						 sizeof(vma_op_answers_t), GFP_ATOMIC);
				 if (acks == NULL) {
					 kfree(lock_message);
					 down_write(&current->mm->mmap_sem);
					 ret = -ENOMEM;
					 goto out;
				 }
				 acks->tgroup_home_cpu = current->tgroup_home_cpu;
				 acks->tgroup_home_id = current->tgroup_home_id;
				 acks->vma_operation_index = index;
				 acks->waiting = current;
				 acks->responses = 0;
				 acks->expected_responses = 0;
				 raw_spin_lock_init(&(acks->lock));
				 int i, error;

				 memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
						 current->tgroup_home_id);
				 if (entry==NULL) {
					 printk("ERROR: Mapping disappeared, cannot save message to update by exit_distribute_operation\n");
					 kfree(lock_message);
					 kfree(acks);
					 down_write(&current->mm->mmap_sem);
					 ret = -EPERM;
					 goto out;
				 }

				 add_vma_ack_entry(acks);

				 down_read(&entry->kernel_set_sem);

#ifndef SUPPORT_FOR_CLUSTERING
				for(i = 0; i < MAX_KERNEL_IDS; i++) {
					// Skip the current cpu
					if(i == _cpu) continue;
#else
				// the list does not include the current processor group descirptor (TODO)
				struct list_head *iter;
				_remote_cpu_info_list_t *objPtr;
				list_for_each(iter, &rlist_head) {
					objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
					i = objPtr->_data._processor;
#endif

					if(entry->kernel_set[i]==1){
						error = pcn_kmsg_send_long(i,
							(struct pcn_kmsg_long_message*) (lock_message),	sizeof(vma_lock_t) - sizeof(struct pcn_kmsg_hdr));
						if (error != -1) {
							acks->expected_responses++;
						}
					}
				}

				/*Second: wait that everybody acquire the lock, and acquire it locally too*/
				while (acks->expected_responses != acks->responses) {

					set_task_state(current, TASK_UNINTERRUPTIBLE);

					if (acks->expected_responses != acks->responses) {
						schedule();
					}

					set_task_state(current, TASK_RUNNING);

				}

				PSVMAPRINTK("SERVER NOT MAIN: Received all ack to lock\n");

				unsigned long flags;
				raw_spin_lock_irqsave(&(acks->lock), flags);
				raw_spin_unlock_irqrestore(&(acks->lock), flags);

				remove_vma_ack_entry(acks);

				vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
					sizeof(vma_operation_t), GFP_ATOMIC);
				if (operation_to_send == NULL) {
					down_write(&current->mm->mmap_sem);
					up_read(&entry->kernel_set_sem);
					kfree(lock_message);
					kfree(acks);
					ret = -ENOMEM;
					goto out;
				}

				operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
				operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;

				operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
				operation_to_send->tgroup_home_id = current->tgroup_home_id;
				operation_to_send->operation = operation;
				operation_to_send->addr = addr;
				operation_to_send->new_addr = new_addr;
				operation_to_send->len = len;
				operation_to_send->new_len = new_len;
				operation_to_send->prot = prot;
				operation_to_send->flags = flags;
				operation_to_send->vma_operation_index = index;
				operation_to_send->from_cpu = _cpu;

				/*I acquire the lock to block page faults too
				 *Important: this should happen before sending the push message or executing the operation*/
	 			if (current->mm->distr_vma_op_counter == 1) {
		 			down_write(&current->mm->distribute_sem);
		 			PSVMAPRINTK("Distributed lock acquired locally\n");
	 			}


				 /* Third: push the operation to everybody
	  			 * If the operation was a remap without fixed parameters, I cannot let other kernels
	 			 * locally choose where to remap it =>
				 * I need to push what the server choose as parameter to the other an push the operation with
				 * a fixed flag.
	 			 * */
	 			if (!(operation == VMA_OP_REMAP) || (flags & MREMAP_FIXED)) {

 					PSVMAPRINTK("SERVER NOT MAIN: sending done for operation, we can execute the operation in parallel! %d\n",operation);
#ifndef SUPPORT_FOR_CLUSTERING
		 			for(i = 0; i < MAX_KERNEL_IDS; i++) {
			 			// Skip the current cpu
			 			if(i == _cpu) continue;
#else
			 			// the list does not include the current processor group descirptor (TODO)
			 		struct list_head *iter;
			 		_remote_cpu_info_list_t * objPtr;
			 		list_for_each(iter, &rlist_head) {
						objPtr =list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
						i = objPtr->_data._processor;
#endif
						if(entry->kernel_set[i]==1)
							pcn_kmsg_send_long(i,(struct pcn_kmsg_long_message*) (operation_to_send),
								sizeof(vma_operation_t)	- sizeof(struct pcn_kmsg_hdr));

			 			}

			 			kfree(lock_message);
			 			kfree(operation_to_send);
			 			kfree(acks);

			 		down_write(&current->mm->mmap_sem);

			 		return ret;

		 		} else {
					PSVMAPRINTK("SERVER NOT MAIN: going to execute the operation locally %d\n",operation);
			 		entry->message_push_operation = operation_to_send;

			 		kfree(lock_message);
			 		kfree(acks);

			 		down_write(&current->mm->mmap_sem);

			 		return VMA_OP_SAVE;
		 		}

	 	}

	} else {
		//CLIENT
		PSVMAPRINTK("CLIENT: starting operation %i for pid %d current index is%d\n", operation, current->pid, current->mm->vma_operation_index);


		/*First: send the operation to the server*/
		vma_operation_t* operation_to_send = (vma_operation_t*) kmalloc(
			sizeof(vma_operation_t), GFP_ATOMIC);
		if (operation_to_send == NULL) {
			ret = -ENOMEM;
			goto out;
		}
		operation_to_send->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
		operation_to_send->header.prio = PCN_KMSG_PRIO_NORMAL;

		operation_to_send->tgroup_home_cpu = current->tgroup_home_cpu;
		operation_to_send->tgroup_home_id = current->tgroup_home_id;
		operation_to_send->operation = operation;
		operation_to_send->addr = addr;
		operation_to_send->new_addr = new_addr;
		operation_to_send->len = len;
		operation_to_send->new_len = new_len;
		operation_to_send->prot = prot;
		operation_to_send->flags = flags;
		operation_to_send->vma_operation_index = -1;
		operation_to_send->from_cpu = _cpu;
		operation_to_send->pgoff = pgoff;
		if (file != NULL) {
			char path[256] = { 0 };
			char* rpath;
			rpath = d_path(&file->f_path, path, 256);
			strcpy(operation_to_send->path, rpath);
		} else
			operation_to_send->path[0] = '\0';

		/*In this case the server will eventually send me the push operation.
	 	*Differently from a not-started-by-me push operation, it is not the main thread that has to execute it,
	 	*but this thread has.
	 	*/
	 	memory_t* entry = find_memory_entry(current->tgroup_home_cpu,
				 current->tgroup_home_id);
		if (entry) {

			if (entry->waiting_for_op != NULL) {
				printk("ERROR: Somebody is already waiting for an op\n");
				kfree(operation_to_send);
				ret = -EPERM;
				goto out;
			}

			entry->waiting_for_op = current;
			entry->arrived_op = 0;

		} else {
			printk("ERROR: Mapping disappeared, cannot wait for push op\n");
			kfree(operation_to_send);
			ret = -EPERM;
			goto out;
		}

		up_write(&current->mm->mmap_sem);

		int error;
		//send the operation to the server
		error = pcn_kmsg_send_long(current->tgroup_home_cpu,
				(struct pcn_kmsg_long_message*) (operation_to_send),
				sizeof(vma_operation_t) - sizeof(struct pcn_kmsg_hdr));
		if (error == -1) {
			printk("Impossible to contact the server");
			kfree(operation_to_send);
			down_write(&current->mm->mmap_sem);
			ret = -EPERM;
			goto out;
		}

		/*Second: the server will send me a LOCK message... another thread will handle it...*/

		/*Third: wait that the server push me the operation*/
		while (entry->arrived_op == 0) {

			set_task_state(current, TASK_UNINTERRUPTIBLE);

			if (entry->arrived_op == 0) {
				schedule();
			}

			set_task_state(current, TASK_RUNNING);
		}

		PSVMAPRINTK("My operation finally arrived pid %d vma operation %d\n",current->pid,current->mm->vma_operation_index);

		/*Note, the distributed lock already has been acquired*/
		down_write(&current->mm->mmap_sem);

		if (current->mm->thread_op != current) {
			printk(	"ERROR: waking up to locally execute a vma operation started by me, but thread_op s not me\n");
			kfree(operation_to_send);
			ret = -EPERM;
			goto out_dist_lock;
		}

		if (operation == VMA_OP_REMAP || operation == VMA_OP_MAP
			|| operation == VMA_OP_BRK) {
			ret = entry->addr;
			if (entry->addr < 0) {
				printk("Received error %lu from the server for operation %d\n", ret,operation);
			goto out_dist_lock;
			}
		}

		entry->waiting_for_op = NULL;

		kfree(operation_to_send);

		return ret;

	}

	 out_dist_lock:

	 up_write(&current->mm->distribute_sem);
	 PSVMAPRINTK("Released distributed lock from out_dist_lock\n");
	 PSVMAPRINTK("current index is %d in out_dist_lock\n", current->mm->vma_operation_index);

	 out: current->mm->distr_vma_op_counter--;
	 current->mm->thread_op = NULL;
	 if (operation == VMA_OP_MAP || operation == VMA_OP_BRK) {
		 current->mm->was_not_pushed--;
	 }

	 wake_up(&request_distributed_vma_op);
	 return ret;
 }

					 long process_server_do_unmap_start(struct mm_struct *mm, unsigned long start,
							 size_t len) {

#if PARTIAL_VMA_MANAGEMENT

						 unsigned long ret = 0;

						 if (current->mm->distribute_unmap == 0) {
							 return ret;
						 }

						 PSPRINTK("Asking unmap for pid %d\n", current->pid);
						 /* Other vma operations can be on-going.
						  * They release the down_write lock to
						  * send messages.
						  * Check if it is the case.
						  */
						 while (current->mm->distr_vma_op_counter > 0) {

							 up_write(&current->mm->mmap_sem);

							 DEFINE_WAIT(wait);
							 prepare_to_wait(&request_distributed_vma_op, &wait,
									 TASK_UNINTERRUPTIBLE);

							 if (current->mm->distr_vma_op_counter > 0) {
								 schedule();
							 }

							 finish_wait(&request_distributed_vma_op, &wait);

							 down_write(&current->mm->mmap_sem);

						 }

						 PSPRINTK("Unmap for pid %d\n", current->pid);

						 //it is my turn...
						 if (current->mm->distr_vma_op_counter != 0) {
							 printk("ERROR: unmapping started but distr_vma_op_counter is %i\n",
									 current->mm->distr_vma_op_counter);
							 return -1;
						 }

						 current->mm->distr_vma_op_counter++;

						 up_write(&current->mm->mmap_sem);

						 unmap_message_t* unmap_msg = (unmap_message_t*) kmalloc(
								 sizeof(unmap_message_t), GFP_ATOMIC);
						 if (unmap_msg == NULL) {
							 ret = -1;
							 goto out;
						 }
						 unmap_msg->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_OP;
						 unmap_msg->header.prio = PCN_KMSG_PRIO_NORMAL;
						 unmap_msg->tgroup_home_cpu = current->tgroup_home_cpu;
						 unmap_msg->tgroup_home_id = current->tgroup_home_id;
						 unmap_msg->start = start;
						 unmap_msg->len = len;

						 vma_op_answers_t* acks = (vma_op_answers_t*) kmalloc(
								 sizeof(vma_op_answers_t), GFP_ATOMIC);
						 if (acks == NULL) {
							 kfree(unmap_msg);
							 ret = -1;
							 goto out;
						 }
						 acks->tgroup_home_cpu = current->tgroup_home_cpu;
						 acks->tgroup_home_id = current->tgroup_home_id;
						 acks->waiting = current;
						 acks->responses = 0;
						 acks->expected_responses = 0;
						 raw_spin_lock_init(&(acks->lock));

						 add_vma_ack_entry(acks);

						 int i, error;

#ifndef SUPPORT_FOR_CLUSTERING
for(i = 0; i < MAX_KERNEL_IDS; i++) {
	// Skip the current cpu
	if(i == _cpu) continue;
#else
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
#endif
error = pcn_kmsg_send_long(i,
		(struct pcn_kmsg_long_message*) (unmap_msg),
		sizeof(unmap_message_t) - sizeof(struct pcn_kmsg_hdr));
if (error != -1) {
	acks->expected_responses++;
}
	}

	while (acks->responses != acks->expected_responses) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if (acks->responses != acks->expected_responses) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	unsigned long flags;
	raw_spin_lock_irqsave(&(acks->lock), flags);
	raw_spin_unlock_irqrestore(&(acks->lock), flags);

	remove_vma_ack_entry(acks);

	kfree(unmap_msg);
	kfree(acks);

	out: down_write(&current->mm->mmap_sem);
	current->mm->distr_vma_op_counter--;

	if (current->mm->distr_vma_op_counter != 0) {
		printk("ERROR: unmapping ending but distr_vma_op_counter is %i\n",
				current->mm->distr_vma_op_counter);
		return -1;
	}

	wake_up(&request_distributed_vma_op);

	return ret;

#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	return start_distribute_operation(VMA_OP_UNMAP, start, len, 0, 0, 0, 0,
			NULL, 0);
#endif
#endif

}

long process_server_do_unmap_end(struct mm_struct *mm, unsigned long start,
		size_t len, int start_ret) {

#if PARTIAL_VMA_MANAGEMENT
	wake_up(&request_distributed_vma_op);
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	end_distribute_operation(VMA_OP_UNMAP, start_ret, start);
	return 0;
#endif
#endif

}

long process_server_mprotect_start(unsigned long start, size_t len,
		unsigned long prot) {

#if PARTIAL_VMA_MANAGEMENT
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	return start_distribute_operation(VMA_OP_PROTECT, start, len, prot, 0, 0, 0,
			NULL, 0);
#endif
#endif

}

long process_server_mprotect_end(unsigned long start, size_t len,
		unsigned long prot, int start_ret) {

#if PARTIAL_VMA_MANAGEMENT
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	end_distribute_operation(VMA_OP_PROTECT, start_ret, start);
	return 0;
#endif
#endif

}
long process_server_do_mmap_pgoff_start(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff) {

#if PARTIAL_VMA_MANAGEMENT
	int ret;

	ret = process_server_do_unmap_start(current->mm, addr, len);
	if (ret != -1)
		return addr;
	else
		return -1;

#else
#if NOT_REPLICATED_VMA_MANAGEMENT
return start_distribute_operation(VMA_OP_MAP, addr, len, prot, 0, 0, flags,
		file, pgoff);
#endif
#endif

}

long process_server_do_mmap_pgoff_end(struct file *file, unsigned long addr,
		unsigned long len, unsigned long prot, unsigned long flags,
		unsigned long pgoff, unsigned long start_ret) {
#if PARTIAL_VMA_MANAGEMENT
	//nothing to do;
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	end_distribute_operation(VMA_OP_MAP, start_ret, addr);
	return 0;
#endif
#endif

}

long process_server_do_brk_start(unsigned long addr, unsigned long len) {

#if PARTIAL_VMA_MANAGEMENT
	int ret;

	ret = process_server_do_unmap_start(current->mm, addr, len);
	if (ret != -1)
		return addr;
	else
		return -1;

#else
#if NOT_REPLICATED_VMA_MANAGEMENT
return start_distribute_operation(VMA_OP_BRK, addr, len, 0, 0, 0, 0, NULL,
		0);
#endif
#endif

}

long process_server_do_brk_end(unsigned long addr, unsigned long len,
		unsigned long start_ret) {
#if PARTIAL_VMA_MANAGEMENT
	//nothing to do;
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	end_distribute_operation(VMA_OP_BRK, start_ret, addr);
	return 0;
#endif
#endif

}

long process_server_do_mremap_start(unsigned long addr, unsigned long old_len,
		unsigned long new_len, unsigned long flags, unsigned long new_addr) {

#if PARTIAL_VMA_MANAGEMENT
	//nothing to do;
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	return start_distribute_operation(VMA_OP_REMAP, addr, (size_t) old_len, 0,
			new_addr, new_len, flags, NULL, 0);
#endif
#endif

}

long process_server_do_mremap_end(unsigned long addr, unsigned long old_len,
		unsigned long new_len, unsigned long flags, unsigned long new_addr,
		unsigned long start_ret) {

#if PARTIAL_VMA_MANAGEMENT
	//nothing to do;
	return 0;
#else
#if NOT_REPLICATED_VMA_MANAGEMENT
	end_distribute_operation(VMA_OP_REMAP, start_ret, new_addr);
	return 0;
#endif
#endif

}

void sleep_shadow() {

	memory_t* memory = NULL;
//	printk("%s called\n", __func__);

	while (current->executing_for_remote == 0 && current->distributed_exit== EXIT_NOT_ACTIVE) {
		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if (current->executing_for_remote == 0 && current->distributed_exit== EXIT_NOT_ACTIVE) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	if(current->distributed_exit!= EXIT_NOT_ACTIVE){
		current->represents_remote = 0;
		do_exit(0);
	}

//	printk("%s shadow activated\n", __func__);

	current->distributed_exit= EXIT_ALIVE;
	current->represents_remote = 0;

	// Notify of PID/PID pairing.
	process_server_notify_delegated_subprocess_starting(current->pid,
			current->prev_pid, current->prev_cpu);

	//this force the task to wait that the main correctly set up the memory
	while (current->tgroup_distributed != 1) {
		//printk("%s waiting for main to set up me\n",__func__);
		msleep(1);
	}

//	printk("%s main set up me\n",__func__);

	memory = find_memory_entry(current->tgroup_home_cpu,
			current->tgroup_home_id);
	memory->alive = 1;

	{ // FS/GS update --- start
		unsigned int fsindex, gsindex;

		savesegment(fs, fsindex);
		if (unlikely(fsindex | current->thread.fsindex))
			loadsegment(fs, current->thread.fsindex);
		else
			loadsegment(fs, 0);

		if (current->thread.fs)
			checking_wrmsrl(MSR_FS_BASE, current->thread.fs);

		savesegment(gs, gsindex); //read the gs register in gsindex variable
		if (unlikely(gsindex | current->thread.gsindex))
			load_gs_index(current->thread.gsindex);
		else
			load_gs_index(0);

		if (current->thread.gs)
			checking_wrmsrl(MSR_KERNEL_GS_BASE, current->thread.gs);

	} // FS/GS update --- end

	atomic_dec(&(memory->pending_migration));

#if MIGRATE_FPU
	if (tsk_used_math(current) && current->fpu_counter >5) //fpu.preload
	__math_state_restore(current);
#endif

//	printk("%s ending...\n",__func__);
/*#if TIMING
	unsigned long long stop = native_read_tsc();
	unsigned long long elapsed_time = stop - clone_data->start;
	if (clone_data->first == 1)
		update_time_migration(elapsed_time, FIRST_MIG_R);
	else
		update_time_migration(elapsed_time, NORMAL_MIG_R);
#endif*/

}
int create_user_thread_for_distributed_process(clone_request_t* clone_data,
		thread_pull_t* my_thread_pull) {
	shadow_thread_t* my_shadow;
	struct task_struct* task;
	int ret;

	my_shadow = (shadow_thread_t*) pop_data((data_header_t**)&(my_thread_pull->threads),
			&(my_thread_pull->spinlock));

	if (my_shadow) {

	//	printk("%s found a shadow\n", __func__);

		task = my_shadow->thread;
		if (task == NULL) {
			printk("%s, ERROR task is NULL", __func__);
			return -1;
		}

		if (!popcorn_ns) {
			printk("ERROR: no popcorn_ns when forking migrating threads\n");
			return -1;
		}

		/* if we are already attached, let's skip the unlinking and linking */
		if (task->nsproxy->cpu_ns != popcorn_ns) {
			//i TODO temp fix or of all active cpus?! ---- TODO this must be fixed is not acceptable
			do_set_cpus_allowed(task, cpu_online_mask);
			put_cpu_ns(task->nsproxy->cpu_ns);
			task->nsproxy->cpu_ns = get_cpu_ns(popcorn_ns);
		}

		//associate the task with the namespace
		ret = associate_to_popcorn_ns(task);
		if (ret) {
			printk(KERN_ERR "%s: associate_to_popcorn_ns returned: %d\n", __func__,ret);
		}

		task->thread.usersp = clone_data->old_rsp;
		memcpy(task_pt_regs(task), &clone_data->regs, sizeof(struct pt_regs));
		task_pt_regs(task)->ax = 0;
		task_pt_regs(task)->sp = clone_data->old_rsp;

		// set thread info
		task->thread.es = clone_data->thread_es;
		task->thread.ds = clone_data->thread_ds;

		task->thread.fsindex = clone_data->thread_fsindex;
		task->thread.fs = clone_data->thread_fs;
		task->thread.gs = clone_data->thread_gs;
		task->thread.gsindex = clone_data->thread_gsindex;

		task->prev_cpu = clone_data->header.from_cpu;
		task->prev_pid = clone_data->placeholder_pid;

		task->personality = clone_data->personality;

	//	task->origin_pid = clone_data->origin_pid;
	//	sigorsets(&task->blocked,&task->blocked,&clone_data->remote_blocked) ;
	//	sigorsets(&task->real_blocked,&task->real_blocked,&clone_data->remote_real_blocked);
	//	sigorsets(&task->saved_sigmask,&task->saved_sigmask,&clone_data->remote_saved_sigmask);
	//	task->pending = clone_data->remote_pending;
	//	task->sas_ss_sp = clone_data->sas_ss_sp;
	//	task->sas_ss_size = clone_data->sas_ss_size;

	//	int cnt = 0;
	//	for (cnt = 0; cnt < _NSIG; cnt++)
	//		task->sighand->action[cnt] = clone_data->action[cnt];

#if MIGRATE_FPU
//FPU migration code --- server
		/* PF_USED_MATH is set if the task used the FPU before
		 * fpu_counter is incremented every time you go in __switch_to while owning the FPU
		 * has_fpu is true if the task is the owner of the FPU, thus the FPU contains its data
		 * fpu.preload (see arch/x86/include/asm.i387.h:switch_fpu_prepare()) is a heuristic
		 */
		if (clone_data->task_flags & PF_USED_MATH)
		//set_used_math();
		set_stopped_child_used_math(task);

		task->fpu_counter = clone_data->task_fpu_counter;

		//    if (__thread_has_fpu(current)) {
		if (!fpu_allocated(&task->thread.fpu)) {
			fpu_alloc(&task->thread.fpu);
			fpu_finit(&task->thread.fpu);
		}

		struct fpu temp; temp.state = &clone_data->fpu_state;
		fpu_copy(&task->thread.fpu, &temp);

		//    }

		PSPRINTK(KERN_ERR "%s: task flags %x fpu_counter %x has_fpu %x [%d:%d]\n",
				__func__, task->flags, (int)task->fpu_counter,
				(int)task->thread.has_fpu, (int)__thread_has_fpu(task), (int)fpu_allocated(&task->thread.fpu));

		//FPU migration code --- is the following optional?
		if (tsk_used_math(task) && task->fpu_counter >5)//fpu.preload
		__math_state_restore(task);
#endif
		//the task will be activated only when task->executing_for_remote==1
		task->executing_for_remote = 1;
		wake_up_process(task);

		kfree(my_shadow);
		pcn_kmsg_free_msg(clone_data);
		return 0;

	} else {
		//printk("%s no shadows found!!\n", __func__);
		wake_up_process(my_thread_pull->main);
		return -1;
	}
}

static int create_kernel_thread_for_distributed_process(void *data) {

	thread_pull_t* my_thread_pull;
	struct cred * new;
	struct mm_struct *mm;
	memory_t* entry;
	struct task_struct* tgroup_iterator = NULL;
	unsigned long flags;
	int i;

	//printk("%s entered \n", __func__);

	spin_lock_irq(&current->sighand->siglock);
	flush_signal_handlers(current, 1);
	spin_unlock_irq(&current->sighand->siglock);

	set_cpus_allowed_ptr(current, cpu_all_mask);

	set_user_nice(current, 0);

	new = prepare_kernel_cred(current);
	commit_creds(new);

	mm = mm_alloc();
	if (!mm) {
		printk("ERROR: Impossible allocate new mm_struct\n");
		return -1;
	}

	init_new_context(current, mm);

	arch_pick_mmap_layout(mm);

	exec_mmap(mm);
	set_fs(USER_DS);
	current->flags &= ~(PF_RANDOMIZE | PF_KTHREAD);
	flush_thread();
	flush_signal_handlers(current, 0);

	current->main = 1;

	if (!popcorn_ns) {
		if ((build_popcorn_ns(0)))
			printk("%s: build_popcorn returned error\n", __func__);
	}

	my_thread_pull = (thread_pull_t*) kmalloc(sizeof(thread_pull_t),
			GFP_ATOMIC);
	if (!my_thread_pull) {
		printk("ERROR kmalloc thread pull\n");
		return -1;
	}

	init_rwsem(&current->mm->distribute_sem);
	current->mm->distr_vma_op_counter = 0;
	current->mm->was_not_pushed = 0;
	current->mm->thread_op = NULL;
	current->mm->vma_operation_index = 0;
	current->mm->distribute_unmap = 1;

	my_thread_pull->memory= NULL;
	my_thread_pull->threads= NULL;
	raw_spin_lock_init(&(my_thread_pull->spinlock));
	my_thread_pull->main = current;

	push_data((data_header_t**)&(thread_pull_head), &thread_pull_head_lock, (data_header_t *)my_thread_pull);

	int count= count_data((data_header_t**)&(thread_pull_head), &thread_pull_head_lock);
	printk("WARNING count is %d \n",count);

	for (i = 0; i < NR_CPUS; i++) {
		shadow_thread_t* shadow = (shadow_thread_t*) kmalloc(
				sizeof(shadow_thread_t), GFP_ATOMIC);
		if (shadow) {

			struct pt_regs regs;

			//devo creare registri lato utente????
			memset(&regs, 0, sizeof(regs));

			//regs.si = (unsigned long) fn;
			//regs.di = (unsigned long) shadow;

#ifdef CONFIG_X86_32
			regs.ds = __USER_DS;
			regs.es = __USER_DS;
			regs.fs = __KERNEL_PERCPU;
			regs.gs = __KERNEL_STACK_CANARY;
#else
			regs.ss = __KERNEL_DS;
#endif

			regs.orig_ax = -1;
			//regs.ip = (unsigned long) kernel_thread_helper;
			regs.cs = __KERNEL_CS | get_kernel_rpl();
			regs.flags = X86_EFLAGS_IF | 0x2;

			/* Ok, create the new process.. */
			shadow->thread = do_fork_for_main_kernel_thread(
					CLONE_THREAD | CLONE_SIGHAND | CLONE_VM | CLONE_UNTRACED, 0,
					&regs, 0, NULL, NULL);
			if (!IS_ERR(shadow->thread)) {
			//	printk("%s new shadow created\n",__func__);
				push_data((data_header_t**)&(my_thread_pull->threads), &(my_thread_pull->spinlock),
						(data_header_t *)shadow);
			} else {
				printk("ERROR not able to create shadow\n");
				kfree(shadow);
			}
		}
	}

	while (my_thread_pull->memory == NULL) {
		__set_task_state(current, TASK_UNINTERRUPTIBLE);
		if (my_thread_pull->memory == NULL)
			schedule();
		__set_task_state(current, TASK_RUNNING);
	}

	entry = my_thread_pull->memory;

	entry->operation = VMA_OP_NOP;
	entry->waiting_for_main = NULL;
	entry->waiting_for_op = NULL;
	entry->arrived_op = 0;
	entry->my_lock = 0;

	memset(entry->kernel_set, 0, MAX_KERNEL_IDS * sizeof(int));
	entry->kernel_set[_cpu] = 1;
	init_rwsem(&entry->kernel_set_sem);

	new_kernel_t* new_kernel_msg = (new_kernel_t*) kmalloc(sizeof(new_kernel_t),
			GFP_ATOMIC);
	if (new_kernel_msg == NULL) {
		printk("ERROR: impossible to alloc new kernel message\n");
	}
	new_kernel_msg->tgroup_home_cpu = current->tgroup_home_cpu;
	new_kernel_msg->tgroup_home_id = current->tgroup_home_id;

	new_kernel_msg->header.type = PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL;
	new_kernel_msg->header.prio = PCN_KMSG_PRIO_NORMAL;

	entry->exp_answ = 0;
	entry->answers = 0;
	raw_spin_lock_init(&(entry->lock_for_answer));
	//inform all kernel that a new distributed process is present here
#ifndef SUPPORT_FOR_CLUSTERING
	for(i = 0; i < MAX_KERNEL_IDS; i++) {
		// Skip the current cpu
		if(i == _cpu) continue;

#else
	// the list does not include the current processor group descirptor (TODO)
	struct list_head *iter;
	_remote_cpu_info_list_t *objPtr;
	list_for_each(iter, &rlist_head) {
		objPtr = list_entry(iter, _remote_cpu_info_list_t, cpu_list_member);
		i = objPtr->_data._processor;
		//	printk("sending new kernel message to %d\n",i);
		//printk("cpu %d id %d\n",new_kernel_msg->tgroup_home_cpu,
		//new_kernel_msg->tgroup_home_id);
#endif
		//if (pcn_kmsg_send(i, (struct pcn_kmsg_message*) (new_kernel_msg))
		//		!= -1) {
		if (pcn_kmsg_send_long(i,
				(struct pcn_kmsg_long_message*) (new_kernel_msg),
				sizeof(new_kernel_t) - sizeof(struct pcn_kmsg_hdr)) != -1) {
			// Message delivered
			entry->exp_answ++;
		}
	}

	PSNEWTHREADPRINTK("sent %d new kernel messages\n", entry->exp_answ);

	while (entry->exp_answ != entry->answers) {

		set_task_state(current, TASK_UNINTERRUPTIBLE);

		if (entry->exp_answ != entry->answers) {
			schedule();
		}

		set_task_state(current, TASK_RUNNING);
	}

	PSNEWTHREADPRINTK("received all answers\n");
	kfree(new_kernel_msg);

	lock_task_sighand(current, &flags);

	tgroup_iterator = current;
	while_each_thread(current, tgroup_iterator)
	{
		tgroup_iterator->tgroup_home_id = current->tgroup_home_id;
		tgroup_iterator->tgroup_home_cpu = current->tgroup_home_cpu;
		tgroup_iterator->tgroup_distributed = 1;
	};
	
	unlock_task_sighand(current, &flags);
	
	//printk("woke up everybody\n");
	entry->alive = 1;
	entry->setting_up = 0;

	struct file* f;
        f = filp_open("/bin/test_thread_migration", O_RDONLY | O_LARGEFILE, 0);
	if(IS_ERR(f))
		printk("Impossible to open file /bin/test_thread_migration error is %d\n",PTR_ERR(f));
	else
		filp_close(f,NULL);

	main_for_distributed_kernel_thread(entry,my_thread_pull);

	printk("ERROR: exited from main_for_distributed_kernel_thread\n");

	return 0;

}

static int clone_remote_thread(clone_request_t* clone,int inc) {
	struct file* f;
	memory_t* memory = NULL;
	unsigned long flags;
	int ret;

	retry: memory = find_memory_entry(clone->tgroup_home_cpu,
			clone->tgroup_home_id);

	if (memory) {

	//	printk("%s memory_t found\n", __func__);
		if(inc)
			atomic_inc(&(memory->pending_migration));

		if (memory->thread_pull) {
			return create_user_thread_for_distributed_process(clone,
					memory->thread_pull);
		} else {
			//printk("%s thread pull not ready yet\n", __func__);
			return -1;
		}

	} else {

		//printk("%s trying to add a new memory_t\n", __func__);

		memory_t* entry = (memory_t*) kmalloc(sizeof(memory_t), GFP_ATOMIC);
		if (!entry) {
			printk("ERROR: Impossible allocate memory_t\n");
			return -1;
		}

		entry->tgroup_home_cpu = clone->tgroup_home_cpu;
		entry->tgroup_home_id = clone->tgroup_home_id;
		entry->setting_up = 1;
		atomic_set(&(entry->pending_migration),1);
		ret = add_memory_entry_with_check(entry);

		if (ret == 0) {
			//printk("%s fetching a thread pull \n", __func__);

			thread_pull_t* my_thread_pull = (thread_pull_t*) pop_data(
					(data_header_t**)&(thread_pull_head), &thread_pull_head_lock);
			if (my_thread_pull) {

				//printk("%s found a thread pull \n", __func__);

				entry->thread_pull = my_thread_pull;
				entry->main = my_thread_pull->main;
				entry->mm = my_thread_pull->main->mm;
				//printk("%s main kernel thread is %p\n",__func__,my_thread_pull->main);
				atomic_inc(&entry->mm->mm_users);
				//printk("%s exe file is %s \n",__func__,clone->exe_path);
				f = filp_open(clone->exe_path, O_RDONLY | O_LARGEFILE | O_EXCL,
						0);
				if (IS_ERR(f)) {
					printk("ERROR: error opening exe_path\n");
					return -1;
				}
				set_mm_exe_file(entry->mm, f);
				filp_close(f, NULL);
				entry->mm->start_stack = clone->stack_start;
				entry->mm->start_brk = clone->start_brk;
				entry->mm->brk = clone->brk;
				entry->mm->env_start = clone->env_start;
				entry->mm->env_end = clone->env_end;
				entry->mm->arg_start = clone->arg_start;
				entry->mm->arg_end = clone->arg_end;
				entry->mm->start_code = clone->start_code;
				entry->mm->end_code = clone->end_code;
				entry->mm->start_data = clone->start_data;
				entry->mm->end_data = clone->end_data;
				entry->mm->def_flags = clone->def_flags;

				int i, ch;
				const char *name;
				char tcomm[sizeof(my_thread_pull->main->comm)];

				name = clone->exe_path;

				for (i = 0; (ch = *(name++)) != '\0';) {
					if (ch == '/')
						i = 0;
					else if (i < (sizeof(tcomm) - 1))
						tcomm[i++] = ch;
				}
				tcomm[i] = '\0';
				set_task_comm(my_thread_pull->main, tcomm);
				//printk("%s just before locking sighand\n",__func__);
				lock_task_sighand(my_thread_pull->main, &flags);
				my_thread_pull->main->tgroup_home_cpu = clone->tgroup_home_cpu;
				my_thread_pull->main->tgroup_home_id = clone->tgroup_home_id;
				my_thread_pull->main->tgroup_distributed = 1;
				unlock_task_sighand(my_thread_pull->main, &flags);
				//printk("%s after sighand\n",__func__);
				//the main will be activated only when my_thread_pull->memory !=NULL
				my_thread_pull->memory = entry;
				wake_up_process(my_thread_pull->main);
				//printk("%s before calling create user thread\n",__func__);
				return create_user_thread_for_distributed_process(clone,
						my_thread_pull);

			} else {
				//printk("%s asking to create more thread pull \n", __func__);

				struct work_struct* work = kmalloc(sizeof(struct work_struct),
						GFP_ATOMIC);
				if (work) {
					INIT_WORK( work, update_thread_pull);
					queue_work(clone_wq, work);
				}

				return -1;
			}
		} else {
			//printk("%s thread pull already fetched \n", __func__);
			kfree(entry);
			goto retry;
		}
	}
}

void try_create_remote_thread(struct work_struct* work) {
	clone_work_t* clone_work = (clone_work_t*) work;
	clone_request_t* clone = clone_work->request;
	int ret;

	ret = clone_remote_thread(clone, 0);

	if (ret != 0) {
		printk("%s retry after\n", __func__);
		clone_work_t* delay = (clone_work_t*) kmalloc(sizeof(clone_work_t),
				GFP_ATOMIC);

		if (delay) {
			delay->request = clone;
			INIT_DELAYED_WORK( (struct delayed_work*)delay,
					try_create_remote_thread);
			queue_delayed_work(clone_wq, (struct delayed_work*) delay, 10);
		}

	} else {
		printk("%s success\n", __func__);
	}

	kfree(work);
}

static int handle_clone_request(struct pcn_kmsg_message* inc_msg) {
	clone_request_t* clone = (clone_request_t*) inc_msg;
	int ret;

	ret = clone_remote_thread(clone, 1);

	if (ret != 0) {
		printk("%s clone_work activated with try_create_remote_thread \n",
				__func__);

		clone_work_t* request_work = kmalloc(sizeof(clone_work_t), GFP_ATOMIC);

		if (request_work) {
			request_work->request = clone;
			INIT_WORK( (struct work_struct*)request_work,
					try_create_remote_thread);
			queue_work(clone_wq, (struct work_struct*) request_work);
		}
	}

	return 0;
}
extern int scif_get_nodeIDs(uint16_t *nodes, int len, uint16_t *self);
/**
 * process_server_init
 * Start the process loop in a new kthread.
 */
static int __init process_server_init(void) {

	printk("\n\nPopcorn version with user data replication\n\n");
	printk("Per me si va ne la città dolente,\n"
			"per me si va ne l'etterno dolore,\n"
			"per me si va tra la perduta gente.\n"
			"Giustizia mosse il mio alto fattore;\n"
			"fecemi la divina podestate,\n"
			"la somma sapïenza e 'l primo amore.\n"
			"Dinanzi a me non fuor cose create\n"
			"se non etterne, e io etterno duro.\n"
			"Lasciate ogne speranza, voi ch'intrate\n\n");


	/*#ifndef SUPPORT_FOR_CLUSTERING
     _cpu= smp_processor_id();
#else
     _cpu= cpumask_first(cpu_present_mask);
#endif
	 */

	uint16_t copy_cpu;
	if(scif_get_nodeIDs(NULL, 0, &copy_cpu)==-1)
		printk("ERROR process_server cannot initialize _cpu\n");

	else{
		_cpu= copy_cpu;
		printk("I am cpu %d\n",_cpu);
	}

	/*
	 * Create a work queue so that we can do bottom side
	 * processing on data that was brought in by the
	 * communications module interrupt handlers.
	 */
	clone_wq = create_workqueue("clone_wq");
	exit_wq  = create_workqueue("exit_wq");
	exit_group_wq = create_workqueue("exit_group_wq");
	message_request_wq = create_workqueue("request_wq");
	invalid_message_wq= create_workqueue("invalid_wq");
	vma_op_wq= create_workqueue("vma_op_wq");
	vma_lock_wq= create_workqueue("vma_lock_wq");
	new_kernel_wq= create_workqueue("new_kernel_wq");

#if STATISTICS
	page_fault_mio=0;

	fetch=0;
	local_fetch=0;
	write=0;
	concurrent_write= 0;
	most_long_write=0;
	read=0;

	invalid=0;
	ack=0;
	answer_request=0;
	answer_request_void=0;
	request_data=0;

	most_written_page= 0;
	most_long_read= 0;
	pages_allocated=0;

	compressed_page_sent=0;
	not_compressed_page=0;
	not_compressed_diff_page=0;

#endif

#if TIMING
int i=0;
for(i=0;i<NR_TYPES;i++){
	times[i].max=0;
	times[i].min=0;
	times[i].tot=0;
	times[i].count=0;
	spin_lock_init(&(times[i].spinlock));
}

for(i=0;i<NR_MIG;i++){
	migration_times[i].max=0;
	migration_times[i].min=0;
	migration_times[i].tot=0;
	migration_times[i].count=0;
	spin_lock_init(&(migration_times[i].spinlock));
}

#endif

/*
 * Register to receive relevant incomming messages.
 */
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS,
		 handle_exiting_process_notification);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING,
		 handle_process_pairing_request);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST,
		 handle_clone_request);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST,
		 handle_mapping_request);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE,
		 handle_mapping_response);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_VOID,
		 handle_mapping_response_void);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_INVALID_DATA,
		 handle_invalid_request);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_ACK_DATA,
		 handle_ack);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST,
		 handle_remote_thread_count_request);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE,
		 handle_remote_thread_count_response);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION,
		 handle_thread_group_exited_notification);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_OP,
		 handle_vma_op);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_ACK,
		 handle_vma_ack);
 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_LOCK,
		 handle_vma_lock);

 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL,
		 handle_new_kernel);

 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_NEW_KERNEL_ANSWER,
		 handle_new_kernel_answer);

 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_BACK_MIG_REQUEST,
		 handle_back_migration);

 pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_CREATE_THREAD_PULL,
                                 handle_thread_pull_creation);

 return 0;
}

/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall_popcorn(process_server_init);

