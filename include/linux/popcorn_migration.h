/*
 * popcorn_migration.h
 *
 * Author: Marina Sadini, SSRG Virginia Tech
 */

#ifndef POPCORN_MIGRATION_H_
#define POPCORN_MIGRATION_H_

#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/pcn_kmsg.h>
#include <linux/popcorn_macro.h>

#define NOT_REPLICATED_VMA_MANAGEMENT 1

#define NR_THREAD_POOL 1

#define CLONE_SUCCESS 0
#define CLONE_FAIL 1

#define MIGRATE_FPU 0

#define EXIT_ALIVE 0
#define EXIT_THREAD 1
#define EXIT_PROCESS 2
#define EXIT_FLUSHING 3
#define EXIT_NOT_ACTIVE 4

#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

#define POPCORN_NEW_THREAD_VERBOSE 0
#if POPCORN_NEW_THREAD_VERBOSE
#define NEWTHREADPRINTK(...) printk(__VA_ARGS__)
#else
#define NEWTHREADPRINTK(...) ;
#endif

#define NEW_KERNEL_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id;

struct _new_kernel {
	NEW_KERNEL_FIELDS
};

struct new_kernel {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			NEW_KERNEL_FIELDS
		};

#define NEW_KERNEL_PAD ((sizeof(struct _new_kernel)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _new_kernel)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[NEW_KERNEL_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct new_kernel_work {
	struct work_struct work;
	struct new_kernel *request;
};

#define NEW_KERNEL_ANSWER_FIELDS int tgroup_home_cpu;\
		int tgroup_home_id; \
		int my_set[MAX_KERNEL_IDS];\
		int vma_operation_index;

struct _new_kernel_answer {
	NEW_KERNEL_ANSWER_FIELDS
};

struct new_kernel_answer {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			NEW_KERNEL_ANSWER_FIELDS
		};

#define NEW_KERNEL_ANSWER_PAD ((sizeof(struct _new_kernel_answer)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _new_kernel_answer)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[NEW_KERNEL_ANSWER_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct migration_memory {
	struct migration_memory* next;
	struct migration_memory* prev;

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
	struct vma_operation* message_push_operation;
	struct thread_pool* thread_pool;
	atomic_t pending_migration;
	atomic_t pending_back_migration;

};

void add_migration_memory_entry(struct migration_memory* entry);

int add_migration_memory_entry_with_check(struct migration_memory* entry);

struct migration_memory* find_migration_memory_memory_entry(int cpu, int id);

struct mm_struct* find_mm_from_memory_mapping_entry(int cpu, int id);

struct migration_memory* find_and_remove_migration_memory_entry(int cpu, int id);

void remove_migration_memory_entry(struct migration_memory* entry);

struct new_kernel_work_answer {
	struct work_struct work;
	struct new_kernel_answer *answer;
	struct migration_memory *memory;
};

#define THREAD_PAIRING_FIELD int your_pid; \
		int my_pid;

struct _thread_pairing {
	THREAD_PAIRING_FIELD
};

struct thread_pairing {

	struct pcn_kmsg_hdr header;

	union {

		struct {
			THREAD_PAIRING_FIELD
		};

#define THREAD_PAIRING_PAD ((sizeof(struct _thread_pairing)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _thread_pairing)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[THREAD_PAIRING_PAD];

	}__attribute__((packed));

}__attribute__((packed));

#define ARCH_DEP_MIG_FIELDS struct pt_regs regs;\
		unsigned long thread_usersp;\
		unsigned long old_rsp;\
		unsigned short thread_es;\
		unsigned short thread_ds;\
		unsigned long thread_fs;\
		unsigned short thread_fsindex;\
		unsigned long thread_gs;\
		unsigned short thread_gsindex;
/*#ifdef MIGRATE_FPU		unsigned int  task_flags;\
  	    	unsigned char task_fpu_counter;\
		unsigned char thread_has_fpu;\
		union thread_xstate fpu_state;\
#endif	*/

typedef struct _arch_dep_mig_fields{
	ARCH_DEP_MIG_FIELDS
}arch_dep_mig_fields_t;

#define MIGRATION_FIELDS unsigned long stack_start; \
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
		arch_dep_mig_fields_t arch_dep_fields;\
		int tgroup_home_cpu;\
		int tgroup_home_id;\
		int origin_pid;\
		sigset_t remote_blocked, remote_real_blocked;\
		sigset_t remote_saved_sigmask;\
		struct sigpending remote_pending;\
		unsigned long sas_ss_sp;\
		size_t sas_ss_size;\
		struct k_sigaction action[_NSIG];

struct _migration_request {
	MIGRATION_FIELDS
};

struct migration_request {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			MIGRATION_FIELDS
		};
#define	MIGRATION_STRUCT_PAD ((sizeof(struct _migration_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _migration_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[MIGRATION_STRUCT_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct migration_work {
	struct delayed_work work;
	struct migration_request *request;
	int retry_process;
};

#define BACK_MIGRATION_FIELDS unsigned int personality;\
		unsigned long def_flags;\
		pid_t placeholder_pid;\
		pid_t placeholder_tgid;\
		int back;\
		int prev_pid;\
		arch_dep_mig_fields_t arch_dep_fields;\
		int tgroup_home_cpu;\
		int tgroup_home_id;\
		int origin_pid;\
		sigset_t remote_blocked, remote_real_blocked;\
		sigset_t remote_saved_sigmask;\
		struct sigpending remote_pending;\
		unsigned long sas_ss_sp;\
		size_t sas_ss_size;\
		struct k_sigaction action[_NSIG];


struct _back_migration_request {
	BACK_MIGRATION_FIELDS
};

struct back_migration_request {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			BACK_MIGRATION_FIELDS
		};
#define	BACK_MIGRATION_STRUCT_PAD ((sizeof(struct _back_migration_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _back_migration_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[BACK_MIGRATION_STRUCT_PAD];
	}__attribute__((packed));

}__attribute__((packed));

#define EXITING_MIG_THREAD_FIELDS  pid_t my_pid; \
		pid_t prev_pid;\
		int is_last_tgroup_member; \
		int group_exit;\
		long code;\
		arch_dep_mig_fields_t arch_dep_fields;\

struct _exiting_mig_thread {
	EXITING_MIG_THREAD_FIELDS
};

struct exiting_mig_thread {
	struct pcn_kmsg_hdr header;
	union {
		struct {
			EXITING_MIG_THREAD_FIELDS
		};
#define EXITING_MIG_THREAD_PAD ((sizeof(struct _exiting_mig_thread)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _exiting_mig_thread)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[EXITING_MIG_THREAD_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct exiting_mig_thread_work {
	struct work_struct work;
	struct exiting_mig_thread* request;
};

#define EXIT_PROCESS_FIELDS int tgroup_home_cpu; \
		int tgroup_home_id;

struct _exit_process_notification {
	EXIT_PROCESS_FIELDS
};

struct exit_process_notification {
	struct pcn_kmsg_hdr header;

	union {

		struct {
			EXIT_PROCESS_FIELDS
		};

#define EXITING_GROUP_PAD ((sizeof(struct _exit_process_notification)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _exit_process_notification)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[EXITING_GROUP_PAD];

	}__attribute__((packed));

}__attribute__((packed));

struct exit_process_notification_work {
	struct work_struct work;
	struct exit_process_notification* request;
};


struct count_answers {
	struct count_answers* next;
	struct count_answers* prev;

	int tgroup_home_cpu;
	int tgroup_home_id;
	int responses;
	int expected_responses;
	int count;
	raw_spinlock_t lock;
	struct task_struct * waiting;
};

#define COUNT_REQUEST_FIELD int tgroup_home_cpu; \
		int tgroup_home_id;

struct _remote_thread_count_request {
	COUNT_REQUEST_FIELD
};

struct remote_thread_count_request {
	struct pcn_kmsg_hdr header;

	union {

		struct {
			COUNT_REQUEST_FIELD
		};

#define COUNT_REQUEST_PAD ((sizeof(struct _remote_thread_count_request)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _remote_thread_count_request)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[COUNT_REQUEST_PAD];
	}__attribute__((packed));

}__attribute__((packed));

#define COUNT_RESPONSE_FIELD int tgroup_home_cpu; \
		int tgroup_home_id; \
		int count;

struct _remote_thread_count_response {
	COUNT_RESPONSE_FIELD
};

struct remote_thread_count_response {
	struct pcn_kmsg_hdr header;

	union {

		struct {
			COUNT_RESPONSE_FIELD
		};

#define COUNT_RESPONSE_PAD ((sizeof(struct _remote_thread_count_response)>PCN_KMSG_PAYLOAD_SIZE)?PAD_LONG_MESSAGE(sizeof(struct _remote_thread_count_response)):(PCN_KMSG_PAYLOAD_SIZE))
		char pad[COUNT_RESPONSE_PAD];
	}__attribute__((packed));

}__attribute__((packed));

struct remote_thread_count_work {
	struct work_struct work;
	struct remote_thread_count_request* request;
};

struct create_thread_pool {
	struct pcn_kmsg_hdr header;
	char pad[PCN_KMSG_PAYLOAD_SIZE];
}__attribute__((packed));

typedef struct _data_header {
	struct _data_header* next;
	struct _data_header* prev;
} data_header_t;

struct shadow_thread {
	data_header_t head;
	struct task_struct *thread;
};

struct thread_pool {
	data_header_t head;
	struct task_struct *main;
	struct shadow_thread *threads;
	raw_spinlock_t spinlock;
	struct migration_memory *memory;
};

int popcorn_dup_task(struct task_struct* orig, struct task_struct* task);
int popcorn_thread_exit(struct task_struct *tsk,long code);
void popcorn_sleep_shadow(void);
int popcorn_do_migration(struct task_struct* task, int cpu, struct pt_regs* regs);
void popcorn_synchronize_migrations(int tgroup_home_cpu,int tgroup_home_id );
void popcorn_create_thread_pool(void);

#endif /* POPCORN_MIGRATION_H_ */
