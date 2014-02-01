/**
 * Serve up processes to a remote client cpu
 *
 * DKatz
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
#include <linux/fs_struct.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/slab.h>
#include <linux/process_server.h>
#include <linux/mm.h>
#include <linux/io.h> // ioremap
#include <linux/mman.h> // MAP_ANONYMOUS
#include <linux/pcn_kmsg.h> // Messaging
#include <linux/pcn_perf.h> // performance measurement
#include <linux/string.h>

#include <asm/pgtable.h>
#include <asm/atomic.h>
#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h> // USER_DS
#include <asm/prctl.h> // prctl
#include <asm/proto.h> // do_arch_prctl
#include <asm/msr.h> // wrmsr_safe
#include <asm/mmu_context.h>

/**
 * General purpose configuration
 */
#define COPY_WHOLE_VM_WITH_MIGRATION 0

/**
 * Use the preprocessor to turn off printk.
 */
#define PROCESS_SERVER_VERBOSE 0
#if PROCESS_SERVER_VERBOSE
#define PSPRINTK(...) printk(__VA_ARGS__)
#else
#define PSPRINTK(...) ;
#endif

#define PROCESS_SERVER_INSTRUMENT_LOCK 0
#if PROCESS_SERVER_VERBOSE && PROCESS_SERVER_INSTRUMENT_LOCK
#define PS_SPIN_LOCK(x) PSPRINTK("Acquiring spin lock in %s at line %d\n",__func__,__LINE__); \
                       spin_lock(x); \
                       PSPRINTK("Done acquiring spin lock in %s at line %d\n",__func__,__LINE__)
#define PS_SPIN_UNLOCK(x) PSPRINTK("Releasing spin lock in %s at line %d\n",__func__,__LINE__); \
                          spin_unlock(x); \
                          PSPRINTK("Done releasing spin lock in %s at line %d\n",__func__,__LINE__)
#define PS_DOWN_READ(x) PSPRINTK("Acquiring read lock in %s at line %d\n",__func__,__LINE__); \
                        down_read(x); \
                        PSPRINTK("Done acquiring read lock in %s at line %d\n",__func__,__LINE__)
#define PS_UP_READ(x) PSPRINTK("Releasing read lock in %s at line %d\n",__func__,__LINE__); \
                      up_read(x); \
                      PSPRINTK("Done releasing read lock in %s at line %d\n",__func__,__LINE__)
#define PS_DOWN_WRITE(x) PSPRINTK("Acquiring write lock in %s at line %d\n",__func__,__LINE__); \
                         down_write(x); \
                         PSPRINTK("Done acquiring write lock in %s at line %d\n",__func__,__LINE__)
#define PS_UP_WRITE(x) PSPRINTK("Releasing read write in %s at line %d\n",__func__,__LINE__); \
                       up_write(x); \
                       PSPRINTK("Done releasing write lock in %s at line %d\n",__func__,__LINE__)


#else
#define PS_SPIN_LOCK(x) spin_lock(x)
#define PS_SPIN_UNLOCK(x) spin_unlock(x)
#define PS_DOWN_READ(x) down_read(x)
#define PS_UP_READ(x) up_read(x)
#define PS_DOWN_WRITE(x) down_write(x)
#define PS_UP_WRITE(x) up_write(x)
#endif

/**
 * Library data type definitions
 */
#define PROCESS_SERVER_DATA_TYPE_TEST 0
#define PROCESS_SERVER_VMA_DATA_TYPE 1
#define PROCESS_SERVER_PTE_DATA_TYPE 2
#define PROCESS_SERVER_CLONE_DATA_TYPE 3
#define PROCESS_SERVER_MAPPING_REQUEST_DATA_TYPE 4
#define PROCESS_SERVER_MUNMAP_REQUEST_DATA_TYPE 5
#define PROCESS_SERVER_MM_DATA_TYPE 6
#define PROCESS_SERVER_THREAD_COUNT_REQUEST_DATA_TYPE 7
#define PROCESS_SERVER_MPROTECT_DATA_TYPE 8

/**
 * Useful macros
 */
#define DO_UNTIL_SUCCESS(x) while(x != 0){}

/**
 * Perf
 */
#define MEASURE_PERF 1
#if MEASURE_PERF
#define PERF_INIT() perf_init()
#define PERF_MEASURE_START(x) perf_measure_start(x)
#define PERF_MEASURE_STOP(x,y,z)  perf_measure_stop(x,y,z)

pcn_perf_context_t perf_count_remote_thread_members;
pcn_perf_context_t perf_process_back_migration;
pcn_perf_context_t perf_process_mapping_request;
pcn_perf_context_t perf_process_mapping_request_search_active_mm;
pcn_perf_context_t perf_process_mapping_request_search_saved_mm;
pcn_perf_context_t perf_process_mapping_request_do_lookup;
pcn_perf_context_t perf_process_mapping_request_transmit;
pcn_perf_context_t perf_process_mapping_response;
pcn_perf_context_t perf_process_tgroup_closed_item;
pcn_perf_context_t perf_process_exec_item;
pcn_perf_context_t perf_process_exit_item;
pcn_perf_context_t perf_process_mprotect_item;
pcn_perf_context_t perf_process_munmap_request;
pcn_perf_context_t perf_process_munmap_response;
pcn_perf_context_t perf_process_server_try_handle_mm_fault;
pcn_perf_context_t perf_process_server_import_address_space;
pcn_perf_context_t perf_process_server_do_exit;
pcn_perf_context_t perf_process_server_do_munmap;
pcn_perf_context_t perf_process_server_do_migration;
pcn_perf_context_t perf_process_server_do_mprotect;
pcn_perf_context_t perf_process_server_notify_delegated_subprocess_starting;
pcn_perf_context_t perf_handle_thread_group_exit_notification;
pcn_perf_context_t perf_handle_remote_thread_count_response;
pcn_perf_context_t perf_handle_remote_thread_count_request;
pcn_perf_context_t perf_handle_munmap_response;
pcn_perf_context_t perf_handle_munmap_request;
pcn_perf_context_t perf_handle_mapping_response;
pcn_perf_context_t perf_handle_mapping_request;
pcn_perf_context_t perf_handle_pte_transfer;
pcn_perf_context_t perf_handle_vma_transfer;
pcn_perf_context_t perf_handle_exiting_process_notification;
pcn_perf_context_t perf_handle_process_pairing_request;
pcn_perf_context_t perf_handle_clone_request;
pcn_perf_context_t perf_handle_mprotect_response;
pcn_perf_context_t perf_handle_mprotect_request;
pcn_perf_context_t perf_pcn_kmsg_send;

/**
 *
 */
static void perf_init(void) {
   perf_init_context(&perf_count_remote_thread_members,
           "count_remote_thread_members");
   perf_init_context(&perf_process_back_migration,
           "process_back_migration");
   perf_init_context(&perf_process_mapping_request,
           "process_mapping_request");
   perf_init_context(&perf_process_mapping_request_search_active_mm,
           "process_mapping_request_search_active_mm");
   perf_init_context(&perf_process_mapping_request_search_saved_mm,
           "process_mapping_request_search_saved_mm");
   perf_init_context(&perf_process_mapping_request_do_lookup,
           "process_mapping_request_do_lookup");
   perf_init_context(&perf_process_mapping_request_transmit,
           "process_mapping_request_transmit");
   perf_init_context(&perf_process_mapping_response,
           "process_mapping_response");
   perf_init_context(&perf_process_tgroup_closed_item,
           "process_tgroup_closed_item");
   perf_init_context(&perf_process_exec_item,
           "process_exec_item");
   perf_init_context(&perf_process_exit_item,
           "process_exit_item");
   perf_init_context(&perf_process_mprotect_item,
           "process_mprotect_item");
   perf_init_context(&perf_process_munmap_request,
           "process_munmap_request");
   perf_init_context(&perf_process_munmap_response,
           "process_munmap_response");
   perf_init_context(&perf_process_server_try_handle_mm_fault,
           "process_server_try_handle_mm_fault");
   perf_init_context(&perf_process_server_import_address_space,
           "process_server_import_address_space");
   perf_init_context(&perf_process_server_do_exit,
           "process_server_do_exit");
   perf_init_context(&perf_process_server_do_munmap,
           "process_server_do_munmap");
   perf_init_context(&perf_process_server_do_migration,
           "process_server_do_migration");
   perf_init_context(&perf_process_server_do_mprotect,
           "process_server_do_mprotect");
   perf_init_context(&perf_process_server_notify_delegated_subprocess_starting,
           "process_server_notify_delegated_subprocess_starting");
   perf_init_context(&perf_handle_thread_group_exit_notification,
           "handle_thread_group_exit_notification");
   perf_init_context(&perf_handle_remote_thread_count_response,
           "handle_remote_thread_count_response");
   perf_init_context(&perf_handle_remote_thread_count_request,
           "handle_remote_thread_count_request");
   perf_init_context(&perf_handle_munmap_response,
           "handle_munmap_response");
   perf_init_context(&perf_handle_munmap_request,
           "handle_munmap_request");
   perf_init_context(&perf_handle_mapping_response,
           "handle_mapping_response");
   perf_init_context(&perf_handle_mapping_request,
           "handle_mapping_request");
   perf_init_context(&perf_handle_pte_transfer,
           "handle_pte_transfer");
   perf_init_context(&perf_handle_vma_transfer,
           "handle_vma_transfer");
   perf_init_context(&perf_handle_exiting_process_notification,
           "handle_exiting_process_notification");
   perf_init_context(&perf_handle_process_pairing_request,
           "handle_process_pairing_request");
   perf_init_context(&perf_handle_clone_request,
           "handle_clone_request");
   perf_init_context(&perf_handle_mprotect_request,
           "handle_mprotect_request");
   perf_init_context(&perf_handle_mprotect_response,
           "handle_mprotect_resonse");
   perf_init_context(&perf_pcn_kmsg_send,
           "pcn_kmsg_send");

}

#else
#define PERF_INIT() 
#define PERF_MEASURE_START(x) -1
#define PERF_MEASURE_STOP(x, y, z)
#endif

/**
 * Constants
 */
#define RETURN_DISPOSITION_EXIT 0
#define RETURN_DISPOSITION_MIGRATE 1

/**
 * Library
 */

/**
 * Some piping for linking data entries
 * and identifying data entry types.
 */
typedef struct _data_header {
    struct _data_header* next;
    struct _data_header* prev;
    int data_type;
} data_header_t;

/**
 * Hold data about a pte to vma mapping.
 */
typedef struct _pte_data {
    data_header_t header;
    int vma_id;
    int clone_request_id;
    int cpu;
    unsigned long vaddr;
    unsigned long paddr;
    unsigned long pfn;
} pte_data_t;

/**
 * Hold data about a vma to process
 * mapping.
 */
typedef struct _vma_data {
    data_header_t header;
    spinlock_t lock;
    unsigned long start;
    unsigned long end;
    int clone_request_id;
    int cpu;
    unsigned long flags;
    int vma_id;
    pgprot_t prot;
    unsigned long pgoff;
    pte_data_t* pte_list;
    int mmapping_in_progress;
    char path[256];
} vma_data_t;

/**
 *
 */
typedef struct _clone_data {
    data_header_t header;
    spinlock_t lock;
    int clone_request_id;
    int requesting_cpu;
    char exe_path[512];
    unsigned long clone_flags;
    unsigned long stack_start;
    unsigned long stack_ptr;
    unsigned long env_start;
    unsigned long env_end;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long heap_start;
    unsigned long heap_end;
    unsigned long data_start;
    unsigned long data_end;
    struct pt_regs regs;
    int placeholder_pid;
    int placeholder_tgid;
    int placeholder_cpu;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_sp0;
    unsigned long thread_sp;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int t_home_cpu;
    int t_home_id;
    int prio, static_prio, normal_prio; //from sched.c
	unsigned int rt_priority; //from sched.c
	int sched_class; //from sched.c but here we are using SCHED_NORMAL, SCHED_FIFO, etc.
    unsigned long previous_cpus;
    vma_data_t* vma_list;
    vma_data_t* pending_vma_list;
} clone_data_t;

/**
 * 
 */
typedef struct _mapping_request_data {
    data_header_t header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long address;
    unsigned long vaddr_mapping;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
    unsigned long paddr_mapping;
    size_t paddr_mapping_sz;
    pgprot_t prot;
    unsigned long vm_flags;
    int present;
    int complete;
    int from_saved_mm;
    int responses;
    int expected_responses;
    unsigned long pgoff;
    spinlock_t lock;
    char path[512];
} mapping_request_data_t;

/**
 *
 */
typedef struct _munmap_request_data {
    data_header_t header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
    int responses;
    int expected_responses;
    spinlock_t lock;
} munmap_request_data_t;

/**
 *
 */
typedef struct _remote_thread_count_request_data {
    data_header_t header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    int responses;
    int expected_responses;
    int count;
    spinlock_t lock;
} remote_thread_count_request_data_t;

/**
 *
 */
typedef struct _mm_data {
    data_header_t header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    struct mm_struct* mm;
} mm_data_t;

typedef struct _mprotect_data {
    data_header_t header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long start;
    int responses;
    int expected_responses;
    spinlock_t lock;
} mprotect_data_t;

/**
 * This message is sent to a remote cpu in order to 
 * ask it to spin up a process on behalf of the
 * requesting cpu.  Some of these fields may go
 * away in the near future.
 */
typedef struct _clone_request {
    struct pcn_kmsg_hdr header;
    int clone_request_id;
    unsigned long clone_flags;
    unsigned long stack_start;
    unsigned long stack_ptr;
    unsigned long env_start;
    unsigned long env_end;
    unsigned long arg_start;
    unsigned long arg_end;
    unsigned long heap_start;
    unsigned long heap_end;
    unsigned long data_start;
    unsigned long data_end;
    struct pt_regs regs;
    char exe_path[512];
    int placeholder_pid;
    int placeholder_tgid;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_sp0;
    unsigned long thread_sp;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int t_home_cpu;
    int t_home_id;
    int prio, static_prio, normal_prio; //from sched.c
	unsigned int rt_priority; //from sched.c
	int sched_class; //from sched.c but here we are using SCHED_NORMAL, SCHED_FIFO, etc.
    unsigned long previous_cpus;
} clone_request_t;

/**
 * This message is sent in response to a clone request.
 * Its purpose is to notify the requesting cpu that
 * the specified pid is executing on behalf of the
 * requesting cpu.
 */
typedef struct _create_process_pairing {
    struct pcn_kmsg_hdr header;
    int your_pid; // PID of cpu receiving this pairing request
    int my_pid;   // PID of cpu transmitting this pairing request
} create_process_pairing_t;

/**
 * This message informs the remote cpu of delegated
 * process death.  This occurs whether the process
 * is a placeholder or a delegate locally.
 */
struct _exiting_process {
    struct pcn_kmsg_hdr header;
    int t_home_cpu;             // 4
    int t_home_id;              // 4
    int my_pid;                 // 4
    int is_last_tgroup_member;  // 4+
                                // ---
                                // 16 -> 44 bytes of padding needed
    char pad[44];
} __attribute__((packed)) __attribute__((aligned(64)));  
typedef struct _exiting_process exiting_process_t;

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
} vma_transfer_t;

/**
 * Inform remote cpu of a pte to vma mapping.
 */
struct _pte_transfer {
    struct pcn_kmsg_hdr header;
    int vma_id;                  //  4
    int clone_request_id;        //  4
    unsigned long vaddr;         //  8
    unsigned long paddr;         //  8
    unsigned long pfn;           //  8+
                                 //  ---
                                 //  32 -> 28 bytes of padding needed
    char pad[28];
} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _pte_transfer pte_transfer_t;

/**
 *
 */
struct _mapping_request {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;          // 4
    unsigned long address;      // 8
                                // ---
                                // 20 -> 40 bytes of padding needed
    char pad[40];

} __attribute__((packed)) __attribute__((aligned(64)));

typedef struct _mapping_request mapping_request_t;

/*
 * type = PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION
 */
struct _thread_group_exited_notification {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
                                // ---
                                // 8 -> 52 bytes of padding needed
    char pad[52];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _thread_group_exited_notification thread_group_exited_notification_t;


/**
 *
 */
struct _mapping_response {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        
    int tgroup_home_id; 
    int requester_pid;
    unsigned long present;      
    int from_saved_mm;
    unsigned long address;      
    unsigned long vaddr_mapping;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
    unsigned long paddr_mapping;
    size_t paddr_mapping_sz;
    pgprot_t prot;              
    unsigned long vm_flags;     
    unsigned long pgoff;
    char path[512]; // save to last so we can cut
                    // off data when possible.
};
typedef struct _mapping_response mapping_response_t;

/**
 * This is a hack to eliminate the overhead of sending
 * an entire mapping_response_t when there is no mapping.
 * The overhead is due to the size of the message, which
 * requires the _long pcn_kmsg variant to be used.
 */
struct _nonpresent_mapping_response {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;            // 4
    unsigned long address;      // 8
                                // ---
                                // 20 -> 40 bytes of padding needed
    char pad[40];

} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _nonpresent_mapping_response nonpresent_mapping_response_t;

/**
 *
 */
struct _munmap_request {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;         // 4
    int tgroup_home_id;          // 4
    int requester_pid;           // 4
    unsigned long vaddr_start;   // 8
    unsigned long vaddr_size;    // 8
                                 // ---
                                 // 28 -> 32 bytes of padding needed
    char pad[32];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _munmap_request munmap_request_t;

/**
 *
 */
struct _munmap_response {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;          // 4
    unsigned long vaddr_start;  // 8
    unsigned long vaddr_size;   // 8+
                                // ---
                                // 28 -> 32 bytes of padding needed
    char pad[32];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _munmap_response munmap_response_t;

/**
 *
 */
struct _remote_thread_count_request {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;          // 4
                                // ---
                                // 12 -> 48 bytes of padding needed
    char pad[48];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _remote_thread_count_request remote_thread_count_request_t;

/**
 *
 */
struct _remote_thread_count_response {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;        // 4
    int count;                  // 4
                                // ---
                                // 16 -> 44 bytes of padding needed
    char pad[44];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _remote_thread_count_response remote_thread_count_response_t;

/**
 *
 */
struct _mprotect_request {
    struct pcn_kmsg_hdr header; 
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;          // 4
    unsigned long start;        // 8
    size_t len;                 // 4
    unsigned long prot;         // 8
                                // ---
                                // 32 -> 28 bytes of padding needed
    char pad[28];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _mprotect_request mprotect_request_t;

/**
 *
 */
struct _mprotect_response {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;        // 4
    int tgroup_home_id;         // 4
    int requester_pid;          // 4
    unsigned long start;        // 8
                                // ---
                                // 20 -> 40 bytes of padding needed
    char pad[40];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _mprotect_response mprotect_response_t;

/**
 *
 */
typedef struct _back_migration {
    struct pcn_kmsg_hdr header;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int t_home_cpu;
    int t_home_id;
    unsigned long previous_cpus;
    struct pt_regs regs;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
} back_migration_t;

/**
 *
 */
typedef struct _deconstruction_data {
    int clone_request_id;
    int vma_id;
    int dst_cpu;
} deconstruction_data_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    clone_data_t* clone_data;
} clone_exec_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    struct task_struct *task;
    pid_t pid;
    int t_home_cpu;
    int t_home_id;
    int is_last_tgroup_member;
    struct pt_regs regs;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_sp0;
    unsigned long thread_sp;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
} exit_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long address;
    int from_cpu;
} mapping_request_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    int from_saved_mm;
    unsigned long address;      
    unsigned long present;      
    unsigned long vaddr_mapping;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
    unsigned long paddr_mapping;
    size_t paddr_mapping_sz;
    pgprot_t prot;              
    unsigned long vm_flags;     
    char path[512];
    unsigned long pgoff;
    int from_cpu;
} mapping_response_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long address;
    int from_cpu;
} nonpresent_mapping_response_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
} tgroup_closed_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
    int from_cpu;
} munmap_request_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long vaddr_start;
    unsigned long vaddr_size;
} munmap_response_work_t;

/**
 * 
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    unsigned long start;
    size_t len;
    unsigned long prot;
    int from_cpu;
} mprotect_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    int count;
} remote_thread_count_response_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int requester_pid;
    int from_cpu;
} remote_thread_count_request_work_t;

/**
 *
 */
typedef struct {
    struct work_struct work;
    int tgroup_home_cpu;
    int tgroup_home_id;
    int t_home_cpu;
    int t_home_id;
    unsigned long previous_cpus;
    struct pt_regs regs;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
} back_migration_work_t;


/**
 * Prototypes
 */
static int handle_clone_request(struct pcn_kmsg_message* msg);
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,                                                                                                                   
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          struct task_struct* task);
static vma_data_t* find_vma_data(clone_data_t* clone_data, unsigned long addr_start);
static clone_data_t* find_clone_data(int cpu, int clone_request_id);
static void dump_mm(struct mm_struct* mm);
static void dump_task(struct task_struct* task,struct pt_regs* regs,unsigned long stack_ptr);
static void dump_thread(struct thread_struct* thread);
static void dump_regs(struct pt_regs* regs);
static void dump_stk(struct thread_struct* thread, unsigned long stack_ptr); 

/**
 * Prototypes from parts of the kernel that I modified or made available to external
 * modules.
 */
// I removed the 'static' modifier in mm/memory.c for do_wp_page so I could use it 
// here.
int do_wp_page(struct mm_struct *mm, struct vm_area_struct *vma,
               unsigned long address, pte_t *page_table, pmd_t *pmd,
               spinlock_t *ptl, pte_t orig_pte);
int do_mprotect(struct task_struct* task, unsigned long start, size_t len, unsigned long prot, int do_remote);

/**
 * Module variables
 */
static int _vma_id = 0;
static int _clone_request_id = 0;
static int _cpu = -1;
data_header_t* _saved_mm_head = NULL;             // Saved MM list
DEFINE_SPINLOCK(_saved_mm_head_lock);             // Lock for _saved_mm_head
data_header_t* _mapping_request_data_head = NULL; // Mapping request data head
DEFINE_SPINLOCK(_mapping_request_data_head_lock);  // Lock for above
data_header_t* _count_remote_tmembers_data_head = NULL;
DEFINE_SPINLOCK(_count_remote_tmembers_data_head_lock);
data_header_t* _munmap_data_head = NULL;
DEFINE_SPINLOCK(_munmap_data_head_lock);
data_header_t* _mprotect_data_head = NULL;
DEFINE_SPINLOCK(_mprotect_data_head_lock);
data_header_t* _data_head = NULL;                 // General purpose data store
DEFINE_SPINLOCK(_data_head_lock);                 // Lock for _data_head
DEFINE_SPINLOCK(_vma_id_lock);                    // Lock for _vma_id
DEFINE_SPINLOCK(_clone_request_id_lock);          // Lock for _clone_request_id
struct rw_semaphore _import_sem;
DEFINE_SPINLOCK(_remap_lock);


// Work Queues
static struct workqueue_struct *clone_wq;
static struct workqueue_struct *exit_wq;
static struct workqueue_struct *mapping_wq;

/**
 * General helper functions and debugging tools
 */


// TODO
static bool __user_addr (unsigned long x ) {
    return (x < PAGE_OFFSET);   
}

/**
 * A best effort at making a page writable
 */
static void mk_page_writable(struct mm_struct* mm,
                             struct vm_area_struct* vma,
                             unsigned long vaddr) {
    spinlock_t* ptl;
    pte_t *ptep, pte, entry;
     
         
    ptep = get_locked_pte(mm, vaddr, &ptl);
    if (!ptep)
        goto out;

    pte = *ptep;

    entry = pte_mkwrite(pte_mkdirty(pte));
    set_pte_at(mm, vaddr, ptep, entry);
    update_mmu_cache(vma, vaddr, ptep);

    arch_leave_lazy_mmu_mode();

//out_unlock:
    pte_unmap_unlock(pte, ptl);
out:
    return;
}

/**
 *
 */
static int is_page_writable(struct mm_struct* mm,
                            struct vm_area_struct* vma,
                            unsigned long addr) {
    spinlock_t* ptl;
    pte_t *ptep, pte;
    int ret = 0;

    ptep = get_locked_pte(mm,addr,&ptl);
    if(!ptep)
        goto out;

    pte = *ptep;

    ret = pte_write(pte);

    pte_unmap_unlock(pte, ptl);

out:
    return ret;
}

/**
 *
 */
static clone_data_t* get_current_clone_data(void) {
    clone_data_t* ret = NULL;

    if(!current->clone_data) {
        // Do costly lookup
        ret = find_clone_data(current->prev_cpu,
                                 current->clone_request_id);
        // Store it for easy access next time.
        current->clone_data = ret;
    } else {
        ret = (clone_data_t*)current->clone_data;
    }

    return ret;
}

/**
 *
 */
static int vm_search_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {
 
    unsigned long* resolved_addr = (unsigned long*)walk->private;

    if(NULL == pte || !pte_present(*pte)) {
        return 0;
    }

    *resolved_addr = (pte_val(*pte) & PHYSICAL_PAGE_MASK) | (start & (PAGE_SIZE-1));
    return 0;
}

static int get_physical_address(struct mm_struct* mm, 
                                unsigned long vaddr,
                                unsigned long* paddr) {
    unsigned long resolved = 0;
    struct mm_walk walk = {
        .pte_entry = vm_search_page_walk_pte_entry_callback,
        .private = &(resolved),
        .mm = mm
    };

    walk_page_range(vaddr & PAGE_MASK, (vaddr & PAGE_MASK) + PAGE_SIZE, &walk);
    if(resolved == 0) {
        return -1;
    }

    *paddr = resolved;

    return 0;
}

/**
 *
 */
static int is_vaddr_mapped(struct mm_struct* mm, unsigned long vaddr) {
    unsigned long resolved = 0;
    struct mm_walk walk = {
        .pte_entry = vm_search_page_walk_pte_entry_callback,
        .private = &(resolved),
        .mm = mm
    };
    /*struct vm_area_struct* vma = find_vma(mm,vaddr&PAGE_MASK);
    if(!vma || vma->vm_start > vaddr || vma->vm_end <= vaddr) {
        return 0;
    }*/

    walk_page_range(vaddr & PAGE_MASK, ( vaddr & PAGE_MASK ) + PAGE_SIZE, &walk);
    if(resolved != 0) {
        return 1;
    }
    return 0;

}

/**
 *
 */
int find_consecutive_physically_mapped_region(struct mm_struct* mm,
                                              struct vm_area_struct* vma,
                                              unsigned long vaddr,
                                              unsigned long* vaddr_mapping_start,
                                              unsigned long* paddr_mapping_start,
                                              size_t* paddr_mapping_sz) {
    unsigned long paddr_curr;
    unsigned long vaddr_curr = vaddr;
    unsigned long vaddr_next = vaddr;
    unsigned long paddr_next;
    unsigned long paddr_start;
    size_t sz = 0;

    
    // Initializes paddr_curr
    if(get_physical_address(mm,vaddr_curr,&paddr_curr) < 0) {
        return -1;
    }
    paddr_start = paddr_curr;
    *vaddr_mapping_start = vaddr_curr;
    *paddr_mapping_start = paddr_curr;
    
    sz = PAGE_SIZE;

    // seek up in memory
    // This stretches (sz) only while leaving
    // vaddr and paddr the samed
    while(1) {
        vaddr_next += PAGE_SIZE;
        
        // dont' go past the end of the vma
        if(vaddr_next >= vma->vm_end) {
            break;
        }

        if(get_physical_address(mm,vaddr_next,&paddr_next) < 0) {
            break;
        }

        if(paddr_next == paddr_curr + PAGE_SIZE) {
            sz += PAGE_SIZE;
            paddr_curr = paddr_next;
        } else {
            break;
        }
    }

    // seed down in memory
    // // This stretches sz, and the paddr and vaddr's
    vaddr_curr = vaddr;
    paddr_curr = paddr_start; 
    vaddr_next = vaddr_curr;
    while(1) {
        vaddr_next -= PAGE_SIZE;

        // don't go past the start of the vma
        if(vaddr_next < vma->vm_start) {
            break;
        }

        if(get_physical_address(mm,vaddr_next,&paddr_next) < 0) {
            break;
        }

        if(paddr_next == paddr_curr - PAGE_SIZE) {
            vaddr_curr = vaddr_next;
            paddr_curr = paddr_next;
            sz += PAGE_SIZE;
        } else {
            break;
        }
    }
   
    *vaddr_mapping_start = vaddr_curr;
    *paddr_mapping_start = paddr_curr;
    *paddr_mapping_sz = sz;

    PSPRINTK("%s: found consecutive area- vaddr{%lx}, paddr{%lx}, sz{%lx}\n",
                __func__,
                *vaddr_mapping_start,
                *paddr_mapping_start,
                *paddr_mapping_sz);

    return 0;
}

/**
 * Call remap_pfn_range on the parts of the specified virtual-physical
 * region that are not already mapped.
 *
 * Note: mm->mmap_sem must already be held by caller.
 */
int remap_pfn_range_remaining(struct mm_struct* mm,
                                  struct vm_area_struct* vma,
                                  unsigned long vaddr_start,
                                  unsigned long paddr_start,
                                  size_t sz,
                                  pgprot_t prot) {
    unsigned long vaddr_curr;
    unsigned long paddr_curr = paddr_start;
    unsigned long sz_curr = 0;
    int ret = 0;
    int err;

    for(vaddr_curr = vaddr_start; 
        vaddr_curr < vaddr_start + sz; 
        vaddr_curr+= PAGE_SIZE) {
        if( !is_vaddr_mapped(mm,vaddr_curr) ) {
            // not mapped - map it
            err = remap_pfn_range(vma,
                                  vaddr_curr,
                                  paddr_curr >> PAGE_SHIFT,
                                  PAGE_SIZE,
                                  prot);
            if( err != 0 ) ret = err;
        }
        paddr_curr += PAGE_SIZE;
    }

    return ret;
}


/**
 * Map, but only in areas that do not currently have mappings.
 * This should extend vmas that ara adjacent as necessary.
 * NOTE: current->enable_do_mmap_pgoff_hook must be disabled
 *       by client code before calling this.
 * NOTE: mm->mmap_sem must already be held by client code.
 * NOTE: entries in the per-mm list of vm_area_structs are
 *       ordered by starting address.  This is helpful, because
 *       I can exit my check early sometimes.
 */
unsigned long do_mmap_remaining(struct file *file, unsigned long addr,
                                unsigned long len, unsigned long prot,
                                unsigned long flags, unsigned long pgoff) {
    unsigned long ret = addr;
    unsigned long start = addr;
    unsigned long local_end = start;
    unsigned long end = addr + len;
    struct vm_area_struct* curr;

    // go through ALL vma's, looking for interference with this space.
    curr = current->mm->mmap;

    PSPRINTK("%s: processing {%lx,%lx}\n",__func__,addr,len);

    while(1) {

        if(start >= end) goto done;

        // We've reached the end of the list
        else if(curr == NULL) {
            // map through the end
            PSPRINTK("%s: curr == NULL - mapping {%lx,%lx}\n",
                    __func__,start,end-start);
            do_mmap(file, start, end - start, prot, flags, pgoff); 
            goto done;
        }

        // the VMA is fully above the region of interest
        else if(end <= curr->vm_start) {
                // mmap through local_end
            PSPRINTK("%s: VMA is fully above the region of interest - mapping {%lx,%lx}\n",
                    __func__,start,end-start);
            do_mmap(file, start, end - start, prot, flags, pgoff);
            goto done;
        }

        // the VMA fully encompases the region of interest
        else if(start >= curr->vm_start && end <= curr->vm_end) {
            // nothing to do
            PSPRINTK("%s: VMA fully encompases the region of interest\n",__func__);
            goto done;
        }

        // the VMA is fully below the region of interest
        else if(curr->vm_end <= start) {
            // move on to the next one
            PSPRINTK("%s: VMA is fully below region of interest\n",__func__);
        }

        // the VMA includes the start of the region of interest 
        // but not the end
        else if (start >= curr->vm_start && 
                 start < curr->vm_end &&
                 end > curr->vm_end) {
            // advance start (no mapping to do) 
            start = curr->vm_end;
            local_end = start;
            PSPRINTK("%s: VMA includes start but not end\n",__func__);
        }

        // the VMA includes the end of the region of interest
        // but not the start
        else if(start < curr->vm_start && 
                end <= curr->vm_end &&
                end > curr->vm_start) {
            local_end = curr->vm_start;
            
            // mmap through local_end
            PSPRINTK("%s: VMA includes end but not start - mapping {%lx,%lx}\n",
                    __func__,start, local_end - start);
            do_mmap(file, start, local_end - start, prot, flags, pgoff);

            // Then we're done
            goto done;
        }

        // the VMA is fully within the region of interest
        else if(start <= curr->vm_start && end >= curr->vm_end) {
            // advance local end
            local_end = curr->vm_start;

            // map the difference
            PSPRINTK("%s: VMS is fully within the region of interest - mapping {%lx,%lx}\n",
                    __func__,start, local_end - start);
            do_mmap(file, start, local_end - start, prot, flags, pgoff);

            // Then advance to the end of this vma
            start = curr->vm_end;
            local_end = start;
        }

        curr = curr->vm_next;

    }

done:
    
exit:
    PSPRINTK("%s: exiting\n",__func__);
    return ret;
}

/**
 *
 */
void dump_task(struct task_struct* task, struct pt_regs* regs, unsigned long stack_ptr) {
#if PROCESS_SERVER_VERBOSE
    if (!task) return;

    PSPRINTK("DUMP TASK\n");
    PSPRINTK("PID: %d\n",task->pid);
    PSPRINTK("State: %lx\n",task->state);
    PSPRINTK("Flags: %x\n",task->flags);
    PSPRINTK("Prio{%d},Static_Prio{%d},Normal_Prio{%d}\n",
            task->prio,task->static_prio,task->normal_prio);
    PSPRINTK("Represents_remote{%d}\n",task->represents_remote);
    PSPRINTK("Executing_for_remote{%d}\n",task->executing_for_remote);
    PSPRINTK("prev_pid{%d}\n",task->prev_pid);
    PSPRINTK("next_pid{%d}\n",task->next_pid);
    PSPRINTK("prev_cpu{%d}\n",task->prev_cpu);
    PSPRINTK("next_cpu{%d}\n",task->next_cpu);
    PSPRINTK("Clone_request_id{%d}\n",task->clone_request_id);
    dump_regs(regs);
    dump_thread(&task->thread);
    //dump_mm(task->mm);
    dump_stk(&task->thread,stack_ptr);
    PSPRINTK("TASK DUMP COMPLETE\n");
#endif
}

/**
 *
 */
static void dump_stk(struct thread_struct* thread, unsigned long stack_ptr) {
    if(!thread) return;
    PSPRINTK("DUMP STACK\n");
    if(thread->sp) {
        PSPRINTK("sp = %lx\n",thread->sp);
    }
    if(thread->usersp) {
        PSPRINTK("usersp = %lx\n",thread->usersp);
    }
    if(stack_ptr) {
        PSPRINTK("stack_ptr = %lx\n",stack_ptr);
    }
    PSPRINTK("STACK DUMP COMPLETE\n");
}

/**
 *
 */
static void dump_regs(struct pt_regs* regs) {
    unsigned long fs, gs;
    PSPRINTK("DUMP REGS\n");
    if(NULL != regs) {
        PSPRINTK("r15{%lx}\n",regs->r15);   
        PSPRINTK("r14{%lx}\n",regs->r14);
        PSPRINTK("r13{%lx}\n",regs->r13);
        PSPRINTK("r12{%lx}\n",regs->r12);
        PSPRINTK("r11{%lx}\n",regs->r11);
        PSPRINTK("r10{%lx}\n",regs->r10);
        PSPRINTK("r9{%lx}\n",regs->r9);
        PSPRINTK("r8{%lx}\n",regs->r8);
        PSPRINTK("bp{%lx}\n",regs->bp);
        PSPRINTK("bx{%lx}\n",regs->bx);
        PSPRINTK("ax{%lx}\n",regs->ax);
        PSPRINTK("cx{%lx}\n",regs->cx);
        PSPRINTK("dx{%lx}\n",regs->dx);
        PSPRINTK("di{%lx}\n",regs->di);
        PSPRINTK("orig_ax{%lx}\n",regs->orig_ax);
        PSPRINTK("ip{%lx}\n",regs->ip);
        PSPRINTK("cs{%lx}\n",regs->cs);
        PSPRINTK("flags{%lx}\n",regs->flags);
        PSPRINTK("sp{%lx}\n",regs->sp);
        PSPRINTK("ss{%lx}\n",regs->ss);
    }
    rdmsrl(MSR_FS_BASE, fs);
    rdmsrl(MSR_GS_BASE, gs);
    PSPRINTK("fs{%lx}\n",fs);
    PSPRINTK("gs{%lx}\n",gs);
    PSPRINTK("REGS DUMP COMPLETE\n");
}

/**
 *
 */
static void dump_thread(struct thread_struct* thread) {
    PSPRINTK("DUMP THREAD\n");
    PSPRINTK("sp0{%lx}, sp{%lx}\n",thread->sp0,thread->sp);
    PSPRINTK("usersp{%lx}\n",thread->usersp);
    PSPRINTK("es{%x}\n",thread->es);
    PSPRINTK("ds{%x}\n",thread->ds);
    PSPRINTK("fsindex{%x}\n",thread->fsindex);
    PSPRINTK("gsindex{%x}\n",thread->gsindex);
    PSPRINTK("gs{%lx}\n",thread->gs);
    PSPRINTK("THREAD DUMP COMPLETE\n");
}

static void dump_pte_data(pte_data_t* p) {
    PSPRINTK("PTE_DATA\n");
    PSPRINTK("vma_id{%x}\n",p->vma_id);
    PSPRINTK("clone_request_id{%x}\n",p->clone_request_id);
    PSPRINTK("cpu{%x}\n",p->cpu);
    PSPRINTK("vaddr{%lx}\n",p->vaddr);
    PSPRINTK("paddr{%lx}\n",p->paddr);
    PSPRINTK("pfn{%lx}\n",p->pfn);
}

static void dump_vma_data(vma_data_t* v) {
    pte_data_t* p;
    PSPRINTK("VMA_DATA\n");
    PSPRINTK("start{%lx}\n",v->start);
    PSPRINTK("end{%lx}\n",v->end);
    PSPRINTK("clone_request_id{%x}\n",v->clone_request_id);
    PSPRINTK("cpu{%x}\n",v->cpu);
    PSPRINTK("flags{%lx}\n",v->flags);
    PSPRINTK("vma_id{%x}\n",v->vma_id);
    PSPRINTK("path{%s}\n",v->path);

    p = v->pte_list;
    while(p) {
        dump_pte_data(p);
        p = (pte_data_t*)p->header.next;
    }
}

static void dump_clone_data(clone_data_t* r) {
    vma_data_t* v;
    PSPRINTK("CLONE REQUEST\n");
    PSPRINTK("clone_request_id{%x}\n",r->clone_request_id);
    PSPRINTK("clone_flags{%lx}\n",r->clone_flags);
    PSPRINTK("stack_start{%lx}\n",r->stack_start);
    PSPRINTK("stack_ptr{%lx}\n",r->stack_ptr);
    PSPRINTK("env_start{%lx}\n",r->env_start);
    PSPRINTK("env_end{%lx}\n",r->env_end);
    PSPRINTK("arg_start{%lx}\n",r->arg_start);
    PSPRINTK("arg_end{%lx}\n",r->arg_end);
    PSPRINTK("heap_start{%lx}\n",r->heap_start);
    PSPRINTK("heap_end{%lx}\n",r->heap_end);
    PSPRINTK("data_start{%lx}\n",r->data_start);
    PSPRINTK("data_end{%lx}\n",r->data_end);
    dump_regs(&r->regs);
    PSPRINTK("placeholder_pid{%x}\n",r->placeholder_pid);
    PSPRINTK("placeholder_tgid{%x}\n",r->placeholder_tgid);
    PSPRINTK("thread_fs{%lx}\n",r->thread_fs);
    PSPRINTK("thread_gs{%lx}\n",r->thread_gs);
    PSPRINTK("thread_sp0{%lx}\n",r->thread_sp0);
    PSPRINTK("thread_sp{%lx}\n",r->thread_sp);
    PSPRINTK("thread_usersp{%lx}\n",r->thread_usersp);

    v = r->vma_list;
    while(v) {
        dump_vma_data(v);
        v = (vma_data_t*)v->header.next;
    }
}

/**
 *
 */
static remote_thread_count_request_data_t* find_remote_thread_count_data(int cpu, int id, int requester_pid) {
    data_header_t* curr = NULL;
    remote_thread_count_request_data_t* request = NULL;
    remote_thread_count_request_data_t* ret = NULL;

    PS_SPIN_LOCK(&_count_remote_tmembers_data_head_lock);

    curr = _count_remote_tmembers_data_head;
    while(curr) {
        request = (remote_thread_count_request_data_t*)curr;
        if(request->tgroup_home_cpu == cpu &&
           request->tgroup_home_id == id &&
           request->requester_pid == requester_pid) {
            ret = request;
            break;
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_count_remote_tmembers_data_head_lock);

    return ret;
}

/**
 *
 */
static munmap_request_data_t* find_munmap_request_data(int cpu, int id, int requester_pid, unsigned long address) {
    data_header_t* curr = NULL;
    munmap_request_data_t* request = NULL;
    munmap_request_data_t* ret = NULL;
    PS_SPIN_LOCK(&_munmap_data_head_lock);
    
    curr = _munmap_data_head;
    while(curr) {
        request = (munmap_request_data_t*)curr;
        if(request->tgroup_home_cpu == cpu && 
                request->tgroup_home_id == id &&
                request->requester_pid == requester_pid &&
                request->vaddr_start == address) {
            ret = request;
            break;
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_munmap_data_head_lock);

    return ret;

}

/**
 *
 */
static mprotect_data_t* find_mprotect_request_data(int cpu, int id, int requester_pid, unsigned long start) {
    data_header_t* curr = NULL;
    mprotect_data_t* request = NULL;
    mprotect_data_t* ret = NULL;
    PS_SPIN_LOCK(&_mprotect_data_head_lock);
    
    curr = _mprotect_data_head;
    while(curr) {
        request = (mprotect_data_t*)curr;
        if(request->tgroup_home_cpu == cpu && 
                request->tgroup_home_id == id &&
                request->requester_pid == requester_pid &&
                request->start == start) {
            ret = request;
            break;
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_mprotect_data_head_lock);

    return ret;

}

/**
 *
 */
static mapping_request_data_t* find_mapping_request_data(int cpu, int id, int pid, unsigned long address) {
    data_header_t* curr = NULL;
    mapping_request_data_t* request = NULL;
    mapping_request_data_t* ret = NULL;
    PS_SPIN_LOCK(&_mapping_request_data_head_lock);
    
    curr = _mapping_request_data_head;
    while(curr) {
        request = (mapping_request_data_t*)curr;
        if(request->tgroup_home_cpu == cpu && 
                request->tgroup_home_id == id &&
                request->requester_pid == pid &&
                request->address == address) {
            ret = request;
            break;
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_mapping_request_data_head_lock);

    return ret;
}

/**
 *
 */
static clone_data_t* find_clone_data(int cpu, int clone_request_id) {
    data_header_t* curr = NULL;
    clone_data_t* clone = NULL;
    clone_data_t* ret = NULL;
    PS_SPIN_LOCK(&_data_head_lock);
    
    curr = _data_head;
    while(curr) {
        if(curr->data_type == PROCESS_SERVER_CLONE_DATA_TYPE) {
            clone = (clone_data_t*)curr;
            if(clone->placeholder_cpu == cpu && clone->clone_request_id == clone_request_id) {
                ret = clone;
                break;
            }
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_data_head_lock);

    return ret;
}

static void destroy_clone_data(clone_data_t* data) {
    vma_data_t* vma_data;
    pte_data_t* pte_data;
    vma_data = data->vma_list;
    while(vma_data) {
        
        // Destroy this VMA's PTE's
        pte_data = vma_data->pte_list;
        while(pte_data) {

            // Remove pte from list
            vma_data->pte_list = (pte_data_t*)pte_data->header.next;
            if(vma_data->pte_list) {
                vma_data->pte_list->header.prev = NULL;
            }

            // Destroy pte
            kfree(pte_data);

            // Next is the new list head
            pte_data = vma_data->pte_list;
        }
        
        // Remove vma from list
        data->vma_list = (vma_data_t*)vma_data->header.next;
        if(data->vma_list) {
            data->vma_list->header.prev = NULL;
        }

        // Destroy vma
        kfree(vma_data);

        // Next is the new list head
        vma_data = data->vma_list;
    }

    // Destroy clone data
    kfree(data);
}

/**
 *
 */
static vma_data_t* find_vma_data(clone_data_t* clone_data, unsigned long addr_start) {

    vma_data_t* curr = clone_data->vma_list;
    vma_data_t* ret = NULL;

    while(curr) {
        
        if(curr->start == addr_start) {
            ret = curr;
            break;
        }

        curr = (vma_data_t*)curr->header.next;
    }

    return ret;
}

/**
 *
 */
static int dump_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {

    int nx;
    int rw;
    int user;
    int pwt;
    int pcd;
    int accessed;
    int dirty;

    if(NULL == pte || !pte_present(*pte)) {                                                                                                                             
        return 0;
    }

    nx       = pte_flags(*pte) & _PAGE_NX       ? 1 : 0;
    rw       = pte_flags(*pte) & _PAGE_RW       ? 1 : 0;
    user     = pte_flags(*pte) & _PAGE_USER     ? 1 : 0;
    pwt      = pte_flags(*pte) & _PAGE_PWT      ? 1 : 0;
    pcd      = pte_flags(*pte) & _PAGE_PCD      ? 1 : 0;
    accessed = pte_flags(*pte) & _PAGE_ACCESSED ? 1 : 0;
    dirty    = pte_flags(*pte) & _PAGE_DIRTY    ? 1 : 0;

    PSPRINTK("pte_entry start{%lx}, end{%lx}, phy{%lx}\n",
            start,
            end,
            (unsigned long)(pte_val(*pte) & PHYSICAL_PAGE_MASK) | (start & (PAGE_SIZE-1)));

    PSPRINTK("\tnx{%d}, ",nx);
    PSPRINTK("rw{%d}, ",rw);
    PSPRINTK("user{%d}, ",user);
    PSPRINTK("pwt{%d}, ",pwt);
    PSPRINTK("pcd{%d}, ",pcd);
    PSPRINTK("accessed{%d}, ",accessed);
    PSPRINTK("dirty{%d}\n",dirty);

    return 0;
}

/**
 * Print mm
 */
static void dump_mm(struct mm_struct* mm) {
    struct vm_area_struct * curr;
    char buf[256];
    struct mm_walk walk = {
        .pte_entry = dump_page_walk_pte_entry_callback,
        .mm = mm,
        .private = NULL
        };

    if(NULL == mm) {
        PSPRINTK("MM IS NULL!\n");
        return;
    }

    PS_DOWN_READ(&mm->mmap_sem);

    curr = mm->mmap;

    PSPRINTK("MM DUMP\n");
    PSPRINTK("Stack Growth{%lx}\n",mm->stack_vm);
    PSPRINTK("Code{%lx - %lx}\n",mm->start_code,mm->end_code);
    PSPRINTK("Brk{%lx - %lx}\n",mm->start_brk,mm->brk);
    PSPRINTK("Stack{%lx}\n",mm->start_stack);
    PSPRINTK("Arg{%lx - %lx}\n",mm->arg_start,mm->arg_end);
    PSPRINTK("Env{%lx - %lx}\n",mm->env_start,mm->env_end);

    while(curr) {
        if(!curr->vm_file) {
            PSPRINTK("Anonymous VM Entry: start{%lx}, end{%lx}, pgoff{%lx}, flags{%lx}\n",
                    curr->vm_start, 
                    curr->vm_end,
                    curr->vm_pgoff,
                    curr->vm_flags);
            // walk    
            walk_page_range(curr->vm_start,curr->vm_end,&walk);
        } else {
            PSPRINTK("Page VM Entry: start{%lx}, end{%lx}, pgoff{%lx}, path{%s}, flags{%lx}\n",
                    curr->vm_start,
                    curr->vm_end,
                    curr->vm_pgoff,
                    d_path(&curr->vm_file->f_path,buf, 256),
                    curr->vm_flags);
            walk_page_range(curr->vm_start,curr->vm_end,&walk);
        }
        curr = curr->vm_next;
    }

    PS_UP_READ(&mm->mmap_sem);
}

/**
 * Data library
 */

/**
 * Add data entry
 */
static void add_data_entry_to(void* entry, spinlock_t* lock, data_header_t** head) {
    data_header_t* hdr = (data_header_t*)entry;
    data_header_t* curr = NULL;

    if(!entry) {
        return;
    }

    // Always clear out the link information
    hdr->next = NULL;
    hdr->prev = NULL;

    PS_SPIN_LOCK(lock);
    
    if (!*head) {
        *head = hdr;
        hdr->next = NULL;
        hdr->prev = NULL;
    } else {
        curr = *head;
        while(curr->next != NULL) {
            if(curr == entry) {
                return;// It's already in the list!
            }
            curr = curr->next;
        }
        // Now curr should be the last entry.
        // Append the new entry to curr.
        curr->next = hdr;
        hdr->next = NULL;
        hdr->prev = curr;
    }

    PS_SPIN_UNLOCK(lock);
}

/**
 * Remove a data entry
 * Requires user to hold lock
 */
static void remove_data_entry_from(void* entry, data_header_t** head) {
    data_header_t* hdr = entry;

    if(!entry) {
        return;
    }

    if(*head == hdr) {
        *head = hdr->next;
    }

    if(hdr->next) {
        hdr->next->prev = hdr->prev;
    }

    if(hdr->prev) {
        hdr->prev->next = hdr->next;
    }

    hdr->prev = NULL;
    hdr->next = NULL;

}

/**
 * General purpose library
 * Add data entry
 */
static void add_data_entry(void* entry) {
    data_header_t* hdr = (data_header_t*)entry;
    data_header_t* curr = NULL;

    if(!entry) {
        return;
    }

    // Always clear out the link information
    hdr->next = NULL;
    hdr->prev = NULL;

    PS_SPIN_LOCK(&_data_head_lock);
    
    if (!_data_head) {
        _data_head = hdr;
        hdr->next = NULL;
        hdr->prev = NULL;
    } else {
        curr = _data_head;
        while(curr->next != NULL) {
            if(curr == entry) {
                return;// It's already in the list!
            }
            curr = curr->next;
        }
        // Now curr should be the last entry.
        // Append the new entry to curr.
        curr->next = hdr;
        hdr->next = NULL;
        hdr->prev = curr;
    }

    PS_SPIN_UNLOCK(&_data_head_lock);
}

/**
 * Remove a data entry
 * Requires user to hold _data_head_lock
 */
static void remove_data_entry(void* entry) {
    data_header_t* hdr = entry;

    if(!entry) {
        return;
    }

    if(_data_head == hdr) {
        _data_head = hdr->next;
    }

    if(hdr->next) {
        hdr->next->prev = hdr->prev;
    }

    if(hdr->prev) {
        hdr->prev->next = hdr->next;
    }

    hdr->prev = NULL;
    hdr->next = NULL;

}

/**
 * Print information about the list.
 */
static void dump_data_list(void) {
    data_header_t* curr = NULL;
    pte_data_t* pte_data = NULL;
    vma_data_t* vma_data = NULL;
    clone_data_t* clone_data = NULL;

    PS_SPIN_LOCK(&_data_head_lock);

    curr = _data_head;

    PSPRINTK("DATA LIST:\n");
    while(curr) {
        switch(curr->data_type) {
        case PROCESS_SERVER_VMA_DATA_TYPE:
            vma_data = (vma_data_t*)curr;
            PSPRINTK("VMA DATA: start{%lx}, end{%lx}, crid{%d}, vmaid{%d}, cpu{%d}, pgoff{%lx}\n",
                    vma_data->start,
                    vma_data->end,
                    vma_data->clone_request_id,
                    vma_data->vma_id, 
                    vma_data->cpu, 
                    vma_data->pgoff);
            break;
        case PROCESS_SERVER_PTE_DATA_TYPE:
            pte_data = (pte_data_t*)curr;
            PSPRINTK("PTE DATA: vaddr{%lx}, paddr{%lx}, vmaid{%d}, cpu{%d}\n",
                    pte_data->vaddr,
                    pte_data->paddr,
                    pte_data->vma_id,
                    pte_data->cpu);
            break;
        case PROCESS_SERVER_CLONE_DATA_TYPE:
            clone_data = (clone_data_t*)curr;
            PSPRINTK("CLONE DATA: flags{%lx}, stack_start{%lx}, heap_start{%lx}, heap_end{%lx}, ip{%lx}, crid{%d}\n",
                    clone_data->clone_flags,
                    clone_data->stack_start,
                    clone_data->heap_start,
                    clone_data->heap_end,
                    clone_data->regs.ip,
                    clone_data->clone_request_id);
            break;
        default:
            break;
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_data_head_lock);
}

/**
 * <MEASURE perf_count_remote_thread_members>
 */
static int count_remote_thread_members(int exclude_t_home_cpu,
                                       int exclude_t_home_id) {
    int tgroup_home_cpu = current->tgroup_home_cpu;
    int tgroup_home_id  = current->tgroup_home_id;
    remote_thread_count_request_data_t* data;
    remote_thread_count_request_t request;
    int i;
    int s;
    int ret = -1;
    int perf = -1;

    perf = PERF_MEASURE_START(&perf_count_remote_thread_members);

    PSPRINTK("%s: entered\n",__func__);

    data = kmalloc(sizeof(remote_thread_count_request_data_t),GFP_KERNEL);
    if(!data) goto exit;

    data->header.data_type = PROCESS_SERVER_THREAD_COUNT_REQUEST_DATA_TYPE;
    data->responses = 0;
    data->expected_responses = 0;
    data->tgroup_home_cpu = tgroup_home_cpu;
    data->tgroup_home_id = tgroup_home_id;
    data->requester_pid = current->pid;
    data->count = 0;
    spin_lock_init(&data->lock);

    add_data_entry_to(data,
                      &_count_remote_tmembers_data_head_lock,
                      &_count_remote_tmembers_data_head);

    request.header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST;
    request.header.prio = PCN_KMSG_PRIO_NORMAL;
    request.tgroup_home_cpu = current->tgroup_home_cpu;
    request.tgroup_home_id  = current->tgroup_home_id;
    request.requester_pid = data->requester_pid;
    for(i = 0; i < NR_CPUS; i++) {

        // Skip the current cpu
        if (i == _cpu) continue;

        // Send the request to this cpu.
        s = pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&request));
        if(!s) {
            // A successful send operation, increase the number
            // of expected responses.
            data->expected_responses++;
        }
    }

    PSPRINTK("%s: waiting on %d responses\n",__func__,data->expected_responses);

    // Wait for all cpus to respond.
    while(data->expected_responses != data->responses) {
        schedule();
    }

    // OK, all responses are in, we can proceed.
    ret = data->count;

    PSPRINTK("%s: found a total of %d remote threads in group\n",__func__,
            data->count);

    PS_SPIN_LOCK(&_count_remote_tmembers_data_head_lock);
    remove_data_entry_from(data,
                           &_count_remote_tmembers_data_head);
    PS_SPIN_UNLOCK(&_count_remote_tmembers_data_head_lock);

    kfree(data);

exit:
    PERF_MEASURE_STOP(&perf_count_remote_thread_members," ",perf);
    return ret;
}

/**
 *
 */
static int count_local_thread_members(int tgroup_home_cpu, int tgroup_home_id, int exclude_pid) {
    struct task_struct *task, *g;
    int count = 0;
    PSPRINTK("%s: entered\n",__func__);
    do_each_thread(g,task) {
        if(task->tgroup_home_id == tgroup_home_id &&
           task->tgroup_home_cpu == tgroup_home_cpu &&
           task->t_home_cpu == _cpu &&
           task->pid != exclude_pid &&
           task->exit_state != EXIT_ZOMBIE &&
           task->exit_state != EXIT_DEAD &&
           !(task->flags & PF_EXITING)) {

                count++;
            
        }
    } while_each_thread(g,task);
    PSPRINTK("%s: exited\n",__func__);

    return count;

}

/**
 *
 */
static int count_thread_members() {
     
    int count = 0;
    PSPRINTK("%s: entered\n",__func__);
    count += count_local_thread_members(current->tgroup_home_cpu, current->tgroup_home_id,current->pid);
    count += count_remote_thread_members(current->tgroup_home_cpu, current->tgroup_home_id);
    PSPRINTK("%s: exited\n",__func__);
    return count;
}


/*
 * Work exec
 *
 * <MEASURE perf_process_tgroup_closed_item>
 */

void process_tgroup_closed_item(struct work_struct* work) {

    tgroup_closed_work_t* w = (tgroup_closed_work_t*) work;
    data_header_t *curr, *next;
    mm_data_t* mm_data;
    struct task_struct *g, *task;
    int tgroup_closed = 0;
    int pass;
    int perf = -1;

    perf = PERF_MEASURE_START(&perf_process_tgroup_closed_item);

    PSPRINTK("%s: entered\n",__func__);
    PSPRINTK("%s: received group exit notification\n",__func__);

    PSPRINTK("%s: waiting for all members of this distributed thread group to finish\n",__func__);
    while(!tgroup_closed) {
        pass = 0;
        do_each_thread(g,task) {
            if(task->tgroup_home_cpu == w->tgroup_home_cpu &&
               task->tgroup_home_id  == w->tgroup_home_id) {
                
                // there are still living tasks within this distributed thread group
                // wait a bit
                schedule();
                pass = 1;
            }

        } while_each_thread(g,task);
        if(!pass) {
            tgroup_closed = 1;
        } else {
            PSPRINTK("%s: waiting for tgroup close out\n",__func__);
        }
    }

    PS_SPIN_LOCK(&_saved_mm_head_lock);
    
    // Remove all saved mm's for this thread group.
    curr = _saved_mm_head;
    while(curr) {
        next = curr->next;
        mm_data = (mm_data_t*)curr;
        if(mm_data->tgroup_home_cpu == w->tgroup_home_cpu &&
           mm_data->tgroup_home_id  == w->tgroup_home_id) {
            // We need to remove this data entry
            remove_data_entry_from(curr,&_saved_mm_head);

            PSPRINTK("%s: removing a mm for cpu{%d} id{%d}\n",
                    __func__,
                    w->tgroup_home_cpu,
                    w->tgroup_home_id);

            // Remove mm
            mmput(mm_data->mm);

            PSPRINTK("%s: mm removed\n",__func__);

            // Free up the data entry
            kfree(curr);
        }
        curr = next;
    }

    PS_SPIN_UNLOCK(&_saved_mm_head_lock);

    kfree(work);

    PERF_MEASURE_STOP(&perf_process_tgroup_closed_item," ",perf);
}

/**
 * 1 = handled
 * 0 = not handled
 */
static int break_cow(struct mm_struct *mm, struct vm_area_struct* vma, unsigned long address) {
    pgd_t *pgd = NULL;
    pud_t *pud = NULL;
    pmd_t *pmd = NULL;
    pte_t *ptep = NULL;
    pte_t pte;
    spinlock_t* ptl;

    PSPRINTK("%s: entered\n",__func__);

    // if it's not a cow mapping, return.
    if((vma->vm_flags & (VM_SHARED | VM_MAYWRITE)) != VM_MAYWRITE) {
        goto not_handled;
    }

    // if it's not writable in vm_flags, return.
    if(!(vma->vm_flags & VM_WRITE)) {
        goto not_handled;
    }

    PS_DOWN_WRITE(&mm->mmap_sem);


    pgd = pgd_offset(mm, address);
    if(!pgd_present(*pgd)) {
        goto not_handled_unlock;
    }

    pud = pud_offset(pgd,address);
    if(!pud_present(*pud)) {
        goto not_handled_unlock;
    }

    pmd = pmd_offset(pud,address);
    if(!pmd_present(*pmd)) {
        goto not_handled_unlock;
    }

    ptep = pte_offset_map(pmd,address);
    if(!ptep || !pte_present(*ptep)) {
        pte_unmap(ptep);
        goto not_handled_unlock;
    }

    pte = *ptep;

    if(pte_write(pte)) {
        goto not_handled_unlock;
    }
    
    // break the cow!
    ptl = pte_lockptr(mm,pmd);
    PS_SPIN_LOCK(ptl);
   
    PSPRINTK("%s: proceeding\n",__func__);
    do_wp_page(mm,vma,address,ptep,pmd,ptl,pte);


    // NOTE:
    // Do not call pte_unmap_unlock(ptep,ptl), since do_wp_page does that!
    
    goto handled;

not_handled_unlock:
    PS_UP_WRITE(&mm->mmap_sem);
not_handled:
    return 0;
handled:
    PS_UP_WRITE(&mm->mmap_sem);
    return 1;
}

/**
 *
 * <MEASURED perf_process_mapping_request>
 */
void process_mapping_request(struct work_struct* work) {
    mapping_request_work_t* w = (mapping_request_work_t*) work;
    mapping_response_t response;
    data_header_t* data_curr;
    mm_data_t* mm_data;
    struct task_struct* task = NULL;
    struct task_struct* g;
    struct vm_area_struct* vma = NULL;
    struct mm_struct* mm = NULL;
    unsigned long address = w->address;
    unsigned long resolved = 0;
    struct mm_walk walk = {
        .pte_entry = vm_search_page_walk_pte_entry_callback,
        .private = &(resolved)
    };
    char* plpath;
    char lpath[512];
    int try_count = 0;
    
    // for perf
    int used_saved_mm = 0;
    int found_vma = 1;
    int found_pte = 1;
    
    // Perf start
    int perf_send = -1;
    int perf = PERF_MEASURE_START(&perf_process_mapping_request);

    PSPRINTK("%s: entered\n",__func__);
    PSPRINTK("received mapping request address{%lx}, cpu{%d}, id{%d}\n",
            w->address,
            w->tgroup_home_cpu,
            w->tgroup_home_id);

    // First, search through existing processes
    do_each_thread(g,task) {
        if((task->tgroup_home_cpu == w->tgroup_home_cpu) &&
           (task->tgroup_home_id  == w->tgroup_home_id )) {
            PSPRINTK("mapping request found common thread group here\n");
            mm = task->mm;
            goto task_mm_search_exit;
        }
    } while_each_thread(g,task);
task_mm_search_exit:

    // Failing the process search, look through saved mm's.
    if(!mm) {
        PS_SPIN_LOCK(&_saved_mm_head_lock);
        data_curr = _saved_mm_head;
        while(data_curr) {

            mm_data = (mm_data_t*)data_curr;
            
            if((mm_data->tgroup_home_cpu == w->tgroup_home_cpu) &&
               (mm_data->tgroup_home_id  == w->tgroup_home_id)) {
                PSPRINTK("%s: Using saved mm to resolve mapping\n",__func__);
                mm = mm_data->mm;
                used_saved_mm = 1;
                break;
            }

            data_curr = data_curr->next;

        } // while

        PS_SPIN_UNLOCK(&_saved_mm_head_lock);
    }


    // OK, if mm was found, look up the mapping.
    if(mm) {

retry:
        try_count++;
        vma = find_vma(mm, address & PAGE_MASK);
        // Validate find_vma result
        if( (!vma) || 
            (vma->vm_start > (address & PAGE_MASK)) || 
            (vma->vm_end <= address) ) {
            PSPRINTK("find_vma turned up an invalid response, invalidating and continuing\n");
            if(!vma) {
                PSPRINTK("vma == NULL\n");
            } else {
                PSPRINTK("vma->vm_start=%lx, vma->vm_end=%lx\n",vma->vm_start,vma->vm_end);
            }
            vma = NULL;
        }

        walk.mm = mm;
        walk_page_range(address & PAGE_MASK, 
                (address & PAGE_MASK) + PAGE_SIZE, &walk);

        if(vma && resolved != 0) {
            unsigned long vaddr_mapping_boundary_start;
            unsigned long paddr_mapping_boundary_start;
            size_t paddr_mapping_boundary_sz;

            PSPRINTK("mapping found! %lx for vaddr %lx\n",resolved,
                    address & PAGE_MASK);

            // break the cow if necessary
            if(try_count < 2 && break_cow(mm, vma, address & PAGE_MASK)) {
                // cow broken, start over again.
                resolved = 0;
                vma = NULL;
                goto retry;
            }

            /*
             * Find the region of consecutive physical memory
             * in which the address resides within this vma.
             */
            find_consecutive_physically_mapped_region(mm,
                                                    vma,
                                                    address&PAGE_MASK,
                                                    &vaddr_mapping_boundary_start,
                                                    &paddr_mapping_boundary_start,
                                                    &paddr_mapping_boundary_sz);

            response.header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
            response.header.prio = PCN_KMSG_PRIO_NORMAL;
            response.tgroup_home_cpu = w->tgroup_home_cpu;
            response.tgroup_home_id = w->tgroup_home_id;
            response.requester_pid = w->requester_pid;
            response.address = address;
            response.present = 1;
            response.vaddr_mapping = vaddr_mapping_boundary_start;
            response.vaddr_start = vma->vm_start;
            response.vaddr_size = vma->vm_end - vma->vm_start;
            response.paddr_mapping = paddr_mapping_boundary_start;
            response.paddr_mapping_sz = paddr_mapping_boundary_sz;
            response.prot = vma->vm_page_prot;
            response.vm_flags = vma->vm_flags;
            if(vma->vm_file == NULL) {
                response.path[0] = '\0';
            } else {    
                plpath = d_path(&vma->vm_file->f_path,lpath,512);
                strcpy(response.path,plpath);
                response.pgoff = vma->vm_pgoff;
            }
            PSPRINTK("mapping prot = %lx, vm_flags = %lx\n",
                    response.prot,response.vm_flags);
        }
        
    }

    // Not found, respond accordingly
    if(resolved == 0) {
        found_vma = 0;
        found_pte = 0;
        PSPRINTK("Mapping not found\n");
        response.header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE;
        response.header.prio = PCN_KMSG_PRIO_NORMAL;
        response.tgroup_home_cpu = w->tgroup_home_cpu;
        response.tgroup_home_id = w->tgroup_home_id;
        response.requester_pid = w->requester_pid;
        response.address = address;
        response.paddr_mapping = 0;
        response.present = 0;
        response.vaddr_start = 0;
        response.vaddr_size = 0;
        response.path[0] = '\0';

        // Handle case where vma was present but no pte.
        if(vma) {
            PSPRINTK("But vma present\n");
            found_vma = 1;
            response.present = 1;
            response.vaddr_mapping = address & PAGE_MASK;
            response.vaddr_start = vma->vm_start;
            response.vaddr_size = vma->vm_end - vma->vm_start;
            response.prot = vma->vm_page_prot;
            response.vm_flags = vma->vm_flags;
             if(vma->vm_file == NULL) {
                 response.path[0] = '\0';
             } else {    
                 plpath = d_path(&vma->vm_file->f_path,lpath,512);
                 strcpy(response.path,plpath);
                 response.pgoff = vma->vm_pgoff;
             }
        }
    }

    // Send response
    if(response.present) {
        perf_send = PERF_MEASURE_START(&perf_pcn_kmsg_send);
        DO_UNTIL_SUCCESS(pcn_kmsg_send_long(w->from_cpu,
                            (struct pcn_kmsg_long_message*)(&response),
                            sizeof(mapping_response_t) - 
                            sizeof(struct pcn_kmsg_hdr) -   //
                            sizeof(response.path) +         // Chop off the end of the path
                            strlen(response.path) + 1));    // variable to save bandwidth.
        PERF_MEASURE_STOP(&perf_pcn_kmsg_send,"handle mapping request",perf_send);
    } else {
        // This is an optimization to get rid of the _long send 
        // which is a time sink.
        nonpresent_mapping_response_t nonpresent_response;
        nonpresent_response.header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_NONPRESENT;
        nonpresent_response.header.prio = PCN_KMSG_PRIO_NORMAL;
        nonpresent_response.tgroup_home_cpu = w->tgroup_home_cpu;
        nonpresent_response.tgroup_home_id  = w->tgroup_home_id;
        nonpresent_response.requester_pid = w->requester_pid;
        nonpresent_response.address = w->address;
        DO_UNTIL_SUCCESS(pcn_kmsg_send(w->from_cpu,(struct pcn_kmsg_message*)(&nonpresent_response)));

    }

    kfree(work);

    // Perf stop
    if(used_saved_mm && found_vma && found_pte) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "Saved MM + VMA + PTE",
                perf);
    } else if (used_saved_mm && found_vma && !found_pte) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "Saved MM + VMA + no PTE",
                perf);
    } else if (used_saved_mm && !found_vma) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "Saved MM + no VMA",
                perf);
    } else if (!used_saved_mm && found_vma && found_pte) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "VMA + PTE",
                perf);
    } else if (!used_saved_mm && found_vma && !found_pte) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "VMA + no PTE",
                perf);
    } else if (!used_saved_mm && !found_vma) {
        PERF_MEASURE_STOP(&perf_process_mapping_request,
                "no VMA",
                perf);
    } else {
        PERF_MEASURE_STOP(&perf_process_mapping_request,"ERR",perf);
    }

    return;
}

/**
 * <MEASURE perf_process_mapping_response>
 */
void process_nonpresent_mapping_response(struct work_struct* work) {

    mapping_request_data_t* data;
    nonpresent_mapping_response_work_t* w = (nonpresent_mapping_response_work_t*) work;
    int perf = -1;

    perf = PERF_MEASURE_START(&perf_process_mapping_response);
   
    PSPRINTK("%s: entered\n",__func__);

    data = find_mapping_request_data(
                                     w->tgroup_home_cpu,
                                     w->tgroup_home_id,
                                     w->requester_pid,
                                     w->address);

    if(data == NULL) {
        printk("%s: ERROR null mapping request data\n",__func__);
        goto exit;
    }

    PSPRINTK("Nonpresent mapping response received for %lx from cpu %d\n",w->address,w->from_cpu);

    PS_SPIN_LOCK(&data->lock);
    data->responses++;
    PS_SPIN_UNLOCK(&data->lock);
exit:
    PERF_MEASURE_STOP(&perf_process_mapping_response,"no remote mapping for cpu",perf);
    kfree(work);
}

/**
 * <MEASURE perf_process_mapping_response>
 */
void process_mapping_response(struct work_struct* work) {
    mapping_request_data_t* data;
    mapping_response_work_t* w = (mapping_response_work_t*)work; 
    int do_data_free_here = 0;

    int perf = PERF_MEASURE_START(&perf_process_mapping_response);

    PSPRINTK("%s: entered\n",__func__);

    data = find_mapping_request_data(
                                     w->tgroup_home_cpu,
                                     w->tgroup_home_id,
                                     w->requester_pid,
                                     w->address);


    PSPRINTK("received mapping response: addr{%lx},requester{%d},sender{%d}\n",
             w->address,
             w->requester_pid,
             w->from_cpu);

    if(data == NULL) {
        printk("%s: ERROR data not found\n",__func__);
        kfree(work);
        PERF_MEASURE_STOP(&perf_process_mapping_response,
                "early exit",
                perf);
        return;
    }

    PS_SPIN_LOCK(&data->lock);

    // If this data entry is completely filled out,
    // there is no reason to go through any more of
    // this logic.  We do still need to account for
    // the response though, which is done after the
    // out label.
    if(data->complete) {
        goto out;
    }

    if(w->present) {
        PSPRINTK("received positive search result from cpu %d\n",
                w->from_cpu);
        
        // Enforce precedence rules.  Responses from saved mm's
        // are always ignored when a response from a live thread
        // can satisfy the mapping request.  The purpose of this
        // is to ensure that the mapping is not stale, since
        // mmap() operations will not effect saved mm's.  Saved
        // mm's are only useful for cases where a thread mmap()ed
        // some space then died before any other thread was able
        // to acquire the new mapping.
        //
        // A note on a case where multiple cpu's have mappings, but
        // of different sizes.  It is possible that two cpu's have
        // partially overlapping mappings.  This is possible because
        // new mapping's are merged when their regions are contiguous.  
        // This is not a problem here because if part of a mapping is 
        // accessed that is not part of an existig mapping, that new 
        // part will be merged in the resulting mapping request.
        //
        // Also, prefer responses that provide values for paddr.
        if(data->present == 1) {

            // another cpu already responded.
            if(!data->from_saved_mm && w->from_saved_mm
                    // but we need to add an exception in case a physical address
                    // is mapped into the saved mm, but not in the unsaved mm...
                    && !(w->paddr_mapping && !data->paddr_mapping)) {
                PSPRINTK("%s: prevented mapping resolver from importing stale mapping\n",__func__);
                goto out;
            }

            // Ensure that we keep physical mappings around.
            if(data->paddr_mapping && !w->paddr_mapping) {
                PSPRINTK("%s: prevented mapping resolver from downgrading from mapping with paddr to one without\n",__func__);
                goto out;
            }
        }
       
        data->from_saved_mm = w->from_saved_mm;
        data->vaddr_mapping = w->vaddr_mapping;
        data->vaddr_start = w->vaddr_start;
        data->vaddr_size = w->vaddr_size;
        data->paddr_mapping = w->paddr_mapping;
        data->paddr_mapping_sz = w->paddr_mapping_sz;
        data->prot = w->prot;
        data->vm_flags = w->vm_flags;
        data->present = 1;
        strcpy(data->path,w->path);
        data->pgoff = w->pgoff;

        // Determine if we can stop looking for a mapping
        if(data->paddr_mapping) {
            data->complete = 1;
        }

    } else {
        PSPRINTK("received negative search result from cpu %d\n",
                w->from_cpu);
    }



out:
    // Account for this cpu's response.
    data->responses++;

    PS_SPIN_UNLOCK(&data->lock);

    kfree(work);
    
    PERF_MEASURE_STOP(&perf_process_mapping_response," ",perf);

}

unsigned long long perf_aa, perf_bb, perf_cc, perf_dd, perf_ee;

/**
 * <MEASURE perf_process_exit_item>
 */
void process_exit_item(struct work_struct* work) {
    exit_work_t* w = (exit_work_t*) work;
    pid_t pid = w->pid;
    struct task_struct *task = w->task;

    int perf = PERF_MEASURE_START(&perf_process_exit_item);

    if(unlikely(!task)) {
        printk("%s: ERROR - empty task\n",__func__);
        kfree(work);
        PERF_MEASURE_STOP(&perf_process_exit_item,"ERROR",perf);
        return;
    }

    if(unlikely(task->pid != pid)) {
        printk("%s: ERROR - wrong task picked\n",__func__);
        kfree(work);
        PERF_MEASURE_STOP(&perf_process_exit_item,"ERROR",perf);
        return;
    }
    
    PSPRINTK("%s: process to kill %ld\n", __func__, (long)pid);
    PSPRINTK("%s: for_each_process Found task to kill, killing\n", __func__);
    PSPRINTK("%s: killing task - is_last_tgroup_member{%d}\n",
            __func__,
            w->is_last_tgroup_member);

    // Now we're executing locally, so update our records
    //if(task->t_home_cpu == _cpu && task->t_home_id == task->pid)
    //    task->represents_remote = 0;

    // Set the return disposition
    task->return_disposition = RETURN_DISPOSITION_EXIT;

    wake_up_process(task);

    kfree(work);

    PERF_MEASURE_STOP(&perf_process_exit_item," ",perf);
}

/**
 * <MEASURED perf_process_exec_item>
 */
void process_exec_item(struct work_struct* work) {
    clone_exec_work_t* w = (clone_exec_work_t*)work;
    clone_data_t* c = w->clone_data;
    struct subprocess_info* sub_info;
    char* argv[] = {c->exe_path,NULL};
    static char *envp[] = { 
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
    };
    int perf = -1;
perf_aa = native_read_tsc();
    sub_info = call_usermodehelper_setup( c->exe_path /*argv[0]*/, 
            argv, envp, 
            GFP_KERNEL );

    perf = PERF_MEASURE_START(&perf_process_exec_item);

    PSPRINTK("process_exec_item: %s\n",c->exe_path);

    if (sub_info == NULL) return;

    PSPRINTK("sub_info guard passed\n");

    /*
     * This information is passed into kmod in order to
     * act as closure information for when the process
     * is spun up.  Once that occurs, this cpu must
     * notify the requesting cpu of the local pid of the
     * delegate process so that it can maintain its records.
     * That information will be used to maintain the link
     * between the placeholder process on the requesting cpu
     * and the delegate process on the executing cpu.
     */
    sub_info->delegated = 1;
    sub_info->remote_pid = c->placeholder_pid;
    sub_info->remote_cpu = c->requesting_cpu;
    sub_info->clone_request_id = c->clone_request_id;
    memcpy(&sub_info->remote_regs, &c->regs, sizeof(struct pt_regs) );
    
    dump_regs(&sub_info->remote_regs);

    /*
     * Spin up the new process.
     */
    call_usermodehelper_exec(sub_info, UMH_NO_WAIT);
perf_bb = native_read_tsc();
    kfree(work);

    PERF_MEASURE_STOP(&perf_process_exec_item," ",perf);
}

/**
 * <MEASURE perf_process_munmap_request>
 */
void process_munmap_request(struct work_struct* work) {
    munmap_request_work_t* w = (munmap_request_work_t*)work;
    munmap_response_t response;
    struct task_struct *task, *g;
    data_header_t *curr;
    mm_data_t* mm_data;

    int perf = PERF_MEASURE_START(&perf_process_munmap_request);

    PSPRINTK("%s: entered\n",__func__);

    // munmap the specified region in the specified thread group
    do_each_thread(g,task) {

        // Look for the thread group
        if(task->tgroup_home_cpu == w->tgroup_home_cpu &&
           task->tgroup_home_id  == w->tgroup_home_id) {

            // Thread group has been found, perform munmap operation on this
            // task.
            PS_DOWN_WRITE(&task->mm->mmap_sem);
            current->enable_distributed_munmap = 0;
            do_munmap(task->mm, w->vaddr_start, w->vaddr_size);
            current->enable_distributed_munmap = 1;
            PS_UP_WRITE(&task->mm->mmap_sem);
            
            goto done; // thread grouping - threads all share a common mm.

        }
    } while_each_thread(g,task);
done:

    // munmap the specified region in any saved mm's as well.
    // This keeps old mappings saved in the mm of dead thread
    // group members from being resolved accidentally after
    // being munmap()ped, as that would cause security/coherency
    // problems.
    PS_SPIN_LOCK(&_saved_mm_head_lock);

    curr = _saved_mm_head;
    while(curr) {
        mm_data = (mm_data_t*)curr;
        if(mm_data->tgroup_home_cpu == w->tgroup_home_cpu &&
           mm_data->tgroup_home_id  == w->tgroup_home_id) {
            
            // Entry found, perform munmap on this saved mm.
            PS_DOWN_WRITE(&mm_data->mm->mmap_sem);
            current->enable_distributed_munmap = 0;
            do_munmap(mm_data->mm, w->vaddr_start, w->vaddr_size);
            current->enable_distributed_munmap = 1;
            PS_UP_WRITE(&mm_data->mm->mmap_sem);

        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_saved_mm_head_lock);


    // Construct response
    response.header.type = PCN_KMSG_TYPE_PROC_SRV_MUNMAP_RESPONSE;
    response.header.prio = PCN_KMSG_PRIO_NORMAL;
    response.tgroup_home_cpu = w->tgroup_home_cpu;
    response.tgroup_home_id = w->tgroup_home_id;
    response.requester_pid = w->requester_pid;
    response.vaddr_start = w->vaddr_start;
    response.vaddr_size = w->vaddr_size;
    
    // Send response
    DO_UNTIL_SUCCESS(pcn_kmsg_send(w->from_cpu,
                        (struct pcn_kmsg_message*)(&response)));

    kfree(work);
    
    PERF_MEASURE_STOP(&perf_process_munmap_request," ",perf);
}

/**
 * <MEASURE perf_process_munmap_response>
 */
void process_munmap_response(struct work_struct* work) {
    munmap_response_work_t* w = (munmap_response_work_t*)work;
    munmap_request_data_t* data;
   
    int perf = PERF_MEASURE_START(&perf_process_munmap_response);

    data = find_munmap_request_data(
                                   w->tgroup_home_cpu,
                                   w->tgroup_home_id,
                                   w->requester_pid,
                                   w->vaddr_start);

    if(data == NULL) {
        PSPRINTK("unable to find munmap data\n");
        kfree(work);
        PERF_MEASURE_STOP(&perf_process_munmap_response,"ERROR",perf);
        return;
    }

    // Register this response.
    PS_SPIN_LOCK(&data->lock);
    data->responses++;
    PS_SPIN_UNLOCK(&data->lock);

    kfree(work);

    PERF_MEASURE_STOP(&perf_process_munmap_response," ",perf);

}

/**
 * <MEASRURE perf_process_mprotect_item>
 */
void process_mprotect_item(struct work_struct* work) {
    mprotect_response_t response;
    mprotect_work_t* w = (mprotect_work_t*)work;
    int tgroup_home_cpu = w->tgroup_home_cpu;
    int tgroup_home_id  = w->tgroup_home_id;
    unsigned long start = w->start;
    size_t len = w->len;
    unsigned long prot = w->prot;
    struct task_struct* task, *g;

    int perf = PERF_MEASURE_START(&perf_process_mprotect_item);
    
    // Find the task
    do_each_thread(g,task) {
        if(task->tgroup_home_cpu == tgroup_home_cpu &&
           task->tgroup_home_id  == tgroup_home_id) {
            
            // do_mprotect
            do_mprotect(task,start,len,prot,0);

            // then quit
            goto done;

        }
    } while_each_thread(g,task);
done:

    // Construct response
    response.header.type = PCN_KMSG_TYPE_PROC_SRV_MPROTECT_RESPONSE;
    response.header.prio = PCN_KMSG_PRIO_NORMAL;
    response.tgroup_home_cpu = tgroup_home_cpu;
    response.tgroup_home_id = tgroup_home_id;
    response.requester_pid = w->requester_pid;
    response.start = start;
    
    // Send response
    DO_UNTIL_SUCCESS(pcn_kmsg_send(w->from_cpu,
                        (struct pcn_kmsg_message*)(&response)));

    kfree(work);

    PERF_MEASURE_STOP(&perf_process_mprotect_item," ",perf);
}

void process_remote_thread_count_response(struct work_struct* work) {
    remote_thread_count_response_work_t* w = (remote_thread_count_response_work_t*) work;
    remote_thread_count_request_data_t* data;
    
    data = find_remote_thread_count_data(w->tgroup_home_cpu,
                                         w->tgroup_home_id,
                                         w->requester_pid);

    PSPRINTK("%s: entered - cpu{%d}, id{%d}, count{%d}\n",
            __func__,
            w->tgroup_home_cpu,
            w->tgroup_home_id,
            w->count);

    if(data == NULL) {
        PSPRINTK("unable to find remote thread count data\n");
        return;
    }

    // Register this response.
    PS_SPIN_LOCK(&data->lock);
    data->count += w->count;
    data->responses++;
    PS_SPIN_UNLOCK(&data->lock);

    kfree(work);
}

void process_remote_thread_count_request(struct work_struct* work) {
    remote_thread_count_request_work_t* w = (remote_thread_count_request_work_t*)work;
    remote_thread_count_response_t response;

    PSPRINTK("%s: entered - cpu{%d}, id{%d}\n",
            __func__,
            w->tgroup_home_cpu,
            w->tgroup_home_id);


    // Finish constructing response
    response.header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE;
    response.header.prio = PCN_KMSG_PRIO_NORMAL;
    response.tgroup_home_cpu = w->tgroup_home_cpu;
    response.tgroup_home_id = w->tgroup_home_id;
    response.requester_pid = w->requester_pid;
    response.count = count_local_thread_members(w->tgroup_home_cpu,w->tgroup_home_id,-1);

    PSPRINTK("%s: responding to thread count request with %d\n",__func__,
            response.count);

    // Send response
    DO_UNTIL_SUCCESS(pcn_kmsg_send(w->from_cpu,
                            (struct pcn_kmsg_message*)(&response)));

    kfree(work);

    return;
}

/**
 * <MEASURE perf_process_back_migration>
 */
void process_back_migration(struct work_struct* work) {
    back_migration_work_t* w = (back_migration_work_t*)work;
    struct task_struct* task, *g;
    int found = 0;
    int perf = -1;

    perf = PERF_MEASURE_START(&perf_process_back_migration);

    PSPRINTK("%s\n",__func__);

    // Find the task
    do_each_thread(g,task) {
        if(task->tgroup_home_id  == w->tgroup_home_id &&
           task->tgroup_home_cpu == w->tgroup_home_cpu &&
           task->t_home_id       == w->t_home_id &&
           task->t_home_cpu      == w->t_home_cpu) {
            found = 1;
            goto search_exit;
        }
    } while_each_thread(g,task);
search_exit:
    if(!found) {
        goto exit;
    }

    struct pt_regs* regs = task_pt_regs(task);

    // Now, transplant the state into the shadow process
    memcpy(regs, &w->regs, sizeof(struct pt_regs));
    task->previous_cpus = w->previous_cpus;
    task->thread.fs = w->thread_fs;
    task->thread.gs = w->thread_gs;
    task->thread.usersp = w->thread_usersp;
    task->thread.es = w->thread_es;
    task->thread.ds = w->thread_ds;
    task->thread.fsindex = w->thread_fsindex;
    task->thread.gsindex = w->thread_gsindex;

    // Update local state
    task->represents_remote = 0;
    task->executing_for_remote = 1;
    task->t_distributed = 1;

    // Set the return disposition
    task->return_disposition = RETURN_DISPOSITION_MIGRATE;

    // Release the task
    wake_up_process(task);
    
exit:
    kfree(work);

    PERF_MEASURE_STOP(&perf_process_back_migration," ",perf);
}

/**
 * Request implementations
 */

/**
 * <MEASURE perf_handle_thread_group_exit_notification>
 */
static int handle_thread_group_exited_notification(struct pcn_kmsg_message* inc_msg) {
    thread_group_exited_notification_t* msg = (thread_group_exited_notification_t*) inc_msg;
    tgroup_closed_work_t* exit_work;

    int perf = PERF_MEASURE_START(&perf_handle_thread_group_exit_notification);

    // Spin up bottom half to process this event
    exit_work = kmalloc(sizeof(tgroup_closed_work_t),GFP_ATOMIC);
    if(exit_work) {
        INIT_WORK( (struct work_struct*)exit_work, process_tgroup_closed_item);
        exit_work->tgroup_home_cpu = msg->tgroup_home_cpu;
        exit_work->tgroup_home_id  = msg->tgroup_home_id;
        queue_work(exit_wq, (struct work_struct*)exit_work);
    }

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_thread_group_exit_notification," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_remote_thread_count_response>
 */
static int handle_remote_thread_count_response(struct pcn_kmsg_message* inc_msg) {
    remote_thread_count_response_t* msg = (remote_thread_count_response_t*) inc_msg;
    remote_thread_count_response_work_t* work;

    int perf = PERF_MEASURE_START(&perf_handle_remote_thread_count_response);

    work = kmalloc( sizeof(remote_thread_count_response_work_t), GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_remote_thread_count_response);
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->requester_pid   = msg->requester_pid;
        work->count           = msg->count;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_remote_thread_count_response," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_remote_thread_count_request>
 */
static int handle_remote_thread_count_request(struct pcn_kmsg_message* inc_msg) {
    remote_thread_count_request_t* msg = (remote_thread_count_request_t*)inc_msg;
    remote_thread_count_request_work_t* work;

    int perf = PERF_MEASURE_START(&perf_handle_remote_thread_count_request);
    
    work = kmalloc(sizeof(remote_thread_count_request_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_remote_thread_count_request );
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->requester_pid = msg->requester_pid;
        work->from_cpu = msg->header.from_cpu;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    PERF_MEASURE_STOP(&perf_handle_remote_thread_count_request," ",perf);

    pcn_kmsg_free_msg(inc_msg);
    
    return 0;
}

/**
 * <MEASURE perf_handle_munmap_response>
 */
static int handle_munmap_response(struct pcn_kmsg_message* inc_msg) {
    munmap_response_t* msg = (munmap_response_t*)inc_msg;
    munmap_response_work_t* work;
   
    int perf = PERF_MEASURE_START(&perf_handle_munmap_response);

    work = kmalloc(sizeof(munmap_response_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_munmap_response );
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->requester_pid = msg->requester_pid;
        work->vaddr_start = msg->vaddr_start;
        work->vaddr_size  = msg->vaddr_size;
        queue_work(mapping_wq, (struct work_struct*)work);
    }
  
    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_munmap_response," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_munmap_request>
 */
static int handle_munmap_request(struct pcn_kmsg_message* inc_msg) {
    munmap_request_t* msg = (munmap_request_t*)inc_msg;
    munmap_request_work_t* work;
    
    int perf = PERF_MEASURE_START(&perf_handle_munmap_request);

    work = kmalloc(sizeof(munmap_request_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_munmap_request  );
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id =  msg->tgroup_home_id;
        work->requester_pid = msg->requester_pid;
        work->from_cpu = msg->header.from_cpu;
        work->vaddr_start = msg->vaddr_start;
        work->vaddr_size = msg->vaddr_size;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_munmap_request," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_mprotect_response>
 */
static int handle_mprotect_response(struct pcn_kmsg_message* inc_msg) {
    mprotect_response_t* msg = (mprotect_response_t*)inc_msg;
    mprotect_data_t* data;
  
    int perf = PERF_MEASURE_START(&perf_handle_mprotect_response);

    data = find_mprotect_request_data(
                                   msg->tgroup_home_cpu,
                                   msg->tgroup_home_id,
                                   msg->requester_pid,
                                   msg->start);

    if(data == NULL) {
        PSPRINTK("unable to find mprotect data\n");
        pcn_kmsg_free_msg(inc_msg);
        PERF_MEASURE_STOP(&perf_handle_mprotect_response,"ERROR",perf);
        return -1;
    }

    // Register this response.
    PS_SPIN_LOCK(&data->lock);
    data->responses++;
    PS_SPIN_UNLOCK(&data->lock);

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_mprotect_response," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_mprotect_request>
 */
static int handle_mprotect_request(struct pcn_kmsg_message* inc_msg) {
    mprotect_request_t* msg = (mprotect_request_t*)inc_msg;
    mprotect_work_t* work;
    unsigned long start = msg->start;
    size_t len = msg->len;
    unsigned long prot = msg->prot;
    int tgroup_home_cpu = msg->tgroup_home_cpu;
    int tgroup_home_id = msg->tgroup_home_id;


    int perf = PERF_MEASURE_START(&perf_handle_mprotect_request);

    // Schedule work
    work = kmalloc(sizeof(mprotect_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_mprotect_item  );
        work->tgroup_home_id = tgroup_home_id;
        work->tgroup_home_cpu = tgroup_home_cpu;
        work->requester_pid = msg->requester_pid;
        work->start = start;
        work->len = len;
        work->prot = prot;
        work->from_cpu = msg->header.from_cpu;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_mprotect_request," ",perf);

    return 0;
}

/**
 *
 */
static int handle_nonpresent_mapping_response(struct pcn_kmsg_message* inc_msg) {
    nonpresent_mapping_response_t* msg = (nonpresent_mapping_response_t*)inc_msg;
    nonpresent_mapping_response_work_t* work;

    work = kmalloc(sizeof(nonpresent_mapping_response_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_nonpresent_mapping_response);
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->requester_pid = msg->requester_pid;
        work->address = msg->address;
        work->from_cpu = msg->header.from_cpu;
        queue_work(mapping_wq, (struct work_struct*)work);
    } else {
        printk("%s: ERROR: Unable to malloc work\n",__func__);
    }
    pcn_kmsg_free_msg(inc_msg);

    return 0;
}

/**
 *  <MEASURE perf_handle_mapping_response>
 */
static int handle_mapping_response(struct pcn_kmsg_message* inc_msg) {
    mapping_response_t* msg = (mapping_response_t*)inc_msg;
    mapping_response_work_t* work;

    int perf = PERF_MEASURE_START(&perf_handle_mapping_response);

    work = kmalloc(sizeof(mapping_response_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_mapping_response );
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->requester_pid = msg->requester_pid;
        work->from_saved_mm = msg->from_saved_mm;
        work->address = msg->address;
        work->present = msg->present;
        work->vaddr_mapping = msg->vaddr_mapping;
        work->vaddr_start = msg->vaddr_start;
        work->vaddr_size = msg->vaddr_size;
        work->paddr_mapping = msg->paddr_mapping;
        work->paddr_mapping_sz = msg->paddr_mapping_sz;
        work->prot = msg->prot;
        work->vm_flags = msg->vm_flags;
        memcpy(&work->path,&msg->path,sizeof(work->path));
        work->pgoff = msg->pgoff;
        work->from_cpu = msg->header.from_cpu;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    pcn_kmsg_free_msg(inc_msg);
    
    PERF_MEASURE_STOP(&perf_handle_mapping_response," ",perf);

    return 0;
}

/**
 * <MEASRE perf_handle_mapping_request>
 */
static int handle_mapping_request(struct pcn_kmsg_message* inc_msg) {
    mapping_request_t* msg = (mapping_request_t*)inc_msg;
    mapping_request_work_t* work;

    int perf = PERF_MEASURE_START(&perf_handle_mapping_request);

    work = kmalloc(sizeof(mapping_request_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_mapping_request );
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->address = msg->address;
        work->requester_pid = msg->requester_pid;
        work->from_cpu = msg->header.from_cpu;
        queue_work(mapping_wq, (struct work_struct*)work);
    }

    // Clean up incoming message
    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_mapping_request," ",perf);

    return 0;
}

/**
 * <MEASURE perf_handle_pte_transfer>
 */
static int handle_pte_transfer(struct pcn_kmsg_message* inc_msg) {
    pte_transfer_t* msg = (pte_transfer_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    data_header_t* curr = NULL;
    vma_data_t* vma = NULL;
    pte_data_t* pte_data;
    
    int perf = PERF_MEASURE_START(&perf_handle_pte_transfer);

    pte_data = kmalloc(sizeof(pte_data_t),GFP_ATOMIC);
    
    PSPRINTK("%s: entered\n",__func__);
    if(!pte_data) {
        PSPRINTK("Failed to allocate pte_data_t\n");
        PERF_MEASURE_STOP(&perf_handle_pte_transfer,"kmalloc failure",perf);
        return 0;
    }

    PSPRINTK("pte transfer: src{%d}, vaddr{%lx}, paddr{%lx}, vma_id{%d}, pfn{%lx}\n",
            source_cpu,
            msg->vaddr, msg->paddr, msg->vma_id, msg->pfn);

    pte_data->header.data_type = PROCESS_SERVER_PTE_DATA_TYPE;
    pte_data->header.next = NULL;
    pte_data->header.prev = NULL;

    // Copy data into new data item.
    pte_data->cpu = source_cpu;
    pte_data->vma_id = msg->vma_id;
    pte_data->vaddr = msg->vaddr;
    pte_data->paddr = msg->paddr;
    pte_data->pfn = msg->pfn;
    pte_data->clone_request_id = msg->clone_request_id;

    // Look through data store for matching vma_data_t entries.
    PS_SPIN_LOCK(&_data_head_lock);

    curr = _data_head;
    while(curr) {
        if(curr->data_type == PROCESS_SERVER_VMA_DATA_TYPE) {
            vma = (vma_data_t*)curr;
            if(vma->cpu == pte_data->cpu &&
               vma->vma_id == pte_data->vma_id &&
               vma->clone_request_id == pte_data->clone_request_id) {
                // Add to vma data
                PS_SPIN_LOCK(&vma->lock);
                if(vma->pte_list) {
                    pte_data->header.next = (data_header_t*)vma->pte_list;
                    vma->pte_list->header.prev = (data_header_t*)pte_data;
                    vma->pte_list = pte_data;
                } else {
                    vma->pte_list = pte_data;
                }
                PSPRINTK("PTE added to vma\n");
                PS_SPIN_UNLOCK(&vma->lock);
                break;
            }
        }
        curr = curr->next;
    }

    PS_SPIN_UNLOCK(&_data_head_lock);

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_pte_transfer," ",perf);
    
    return 0;
}

/**
 * <MEASURE perf_handle_vma_transfer>
 */
static int handle_vma_transfer(struct pcn_kmsg_message* inc_msg) {
    vma_transfer_t* msg = (vma_transfer_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    vma_data_t* vma_data;
    
    int perf = PERF_MEASURE_START(&perf_handle_vma_transfer);
    
    vma_data = kmalloc(sizeof(vma_data_t),GFP_ATOMIC);
    
    PSPRINTK("%s: entered\n",__func__);
    PSPRINTK("handle_vma_transfer %d\n",msg->vma_id);
    
    if(!vma_data) {
        PSPRINTK("Failed to allocate vma_data_t\n");
        PERF_MEASURE_STOP(&perf_handle_vma_transfer,"kmalloc failure",perf);
        return 0;
    }

    vma_data->header.data_type = PROCESS_SERVER_VMA_DATA_TYPE;

    // Copy data into new data item.
    vma_data->cpu = source_cpu;
    vma_data->start = msg->start;
    vma_data->end = msg->end;
    vma_data->clone_request_id = msg->clone_request_id;
    vma_data->flags = msg->flags;
    vma_data->prot = msg->prot;
    vma_data->vma_id = msg->vma_id;
    vma_data->pgoff = msg->pgoff;
    vma_data->pte_list = NULL;
    vma_data->lock = __SPIN_LOCK_UNLOCKED(&vma_data->lock);
    strcpy(vma_data->path,msg->path);

    add_data_entry(vma_data); 
   
    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_vma_transfer," ",perf);

    return 0;
}

/**
 * Handler function for when either a remote placeholder or a remote delegate process dies,
 * and its local counterpart must be killed to reflect that.
 *
 * <MEASURE perf_handle_exiting_process_notification>
 */
static int handle_exiting_process_notification(struct pcn_kmsg_message* inc_msg) {
    exiting_process_t* msg = (exiting_process_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    struct task_struct *task, *g;
    exit_work_t* exit_work;

    int perf = PERF_MEASURE_START(&perf_handle_exiting_process_notification);

    PSPRINTK("%s: cpu: %d msg: (pid: %d from_cpu: %d [%d])\n", 
	   __func__, smp_processor_id(), msg->my_pid,  inc_msg->hdr.from_cpu, source_cpu);
    
    do_each_thread(g,task) {
        if(task->t_home_id == msg->t_home_id &&
           task->t_home_cpu == msg->t_home_cpu) {

            PSPRINTK("kmkprocsrv: killing local task pid{%d}\n",task->pid);


            // Now we're executing locally, so update our records
            // Should I be doing this here, or in the bottom-half handler?
            task->represents_remote = 0;
            
            exit_work = kmalloc(sizeof(exit_work_t),GFP_ATOMIC);
            if(exit_work) {
                INIT_WORK( (struct work_struct*)exit_work, process_exit_item);
                exit_work->task = task;
                exit_work->pid = task->pid;
                exit_work->t_home_id = task->t_home_id;
                exit_work->t_home_cpu = task->t_home_cpu;
                exit_work->is_last_tgroup_member = msg->is_last_tgroup_member;
                queue_work(exit_wq, (struct work_struct*)exit_work);
            }

          
            goto done; // No need to continue;
        }
    } while_each_thread(g,task);

done:

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_exiting_process_notification," ",perf);

    return 0;
}

/**
 * Handler function for when another processor informs the current cpu
 * of a pid pairing.
 *
 * <MEASURE perf_handle_process_pairing_request>
 */
static int handle_process_pairing_request(struct pcn_kmsg_message* inc_msg) {
    create_process_pairing_t* msg = (create_process_pairing_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    struct task_struct *task, *g;

    int perf = PERF_MEASURE_START(&perf_handle_process_pairing_request);

    PSPRINTK("%s entered\n",__func__);

    if(msg == NULL) {
        PSPRINTK("%s msg == null - ERROR\n",__func__);
        PERF_MEASURE_STOP(&perf_handle_process_pairing_request,"ERROR, msg == null",perf);
        return 0;
    }

    PSPRINTK("%s: remote_pid{%d}, local_pid{%d}, remote_cpu{%d}\n",
            __func__,
            msg->my_pid,
            msg->your_pid,
            source_cpu);
    /*
     * Go through all the processes looking for the one with the right pid.
     * Once that task is found, do the bookkeeping necessary to remember
     * the remote cpu and pid information.
     */
    do_each_thread(g,task) {

        if(task->pid == msg->your_pid && task->represents_remote ) {
            task->next_cpu = source_cpu;
            task->next_pid = msg->my_pid;
            task->executing_for_remote = 0;
 
            PSPRINTK("kmkprocsrv: Added paring at request remote_pid{%d}, local_pid{%d}, remote_cpu{%d}",
                    task->next_pid,
                    task->pid,
                    task->next_cpu);

            goto done; // No need to continue;
        }
    } while_each_thread(g,task);

done:

    pcn_kmsg_free_msg(inc_msg);

    PERF_MEASURE_STOP(&perf_handle_process_pairing_request," ",perf);

    return 0;
}

/**
 * Handle clone requests. 
 */
static int handle_clone_request(struct pcn_kmsg_message* inc_msg) {
    clone_request_t* request = (clone_request_t*)inc_msg;
    clone_exec_work_t* clone_work = NULL;
    unsigned int source_cpu = request->header.from_cpu;
    clone_data_t* clone_data;
    data_header_t* curr;
    data_header_t* next;
    vma_data_t* vma;

    int perf = PERF_MEASURE_START(&perf_handle_clone_request);

perf_cc = native_read_tsc();
    PSPRINTK("%s: entered\n",__func__);
    
    /*
     * Remember this request
     */
    clone_data = kmalloc(sizeof(clone_data_t),GFP_ATOMIC);
    clone_data->header.data_type = PROCESS_SERVER_CLONE_DATA_TYPE;

    clone_data->clone_request_id = request->clone_request_id;
    clone_data->requesting_cpu = source_cpu;
    clone_data->clone_flags = request->clone_flags;
    clone_data->stack_start = request->stack_start;
    clone_data->stack_ptr = request->stack_ptr;
    clone_data->arg_start = request->arg_start;
    clone_data->arg_end = request->arg_end;
    clone_data->env_start = request->env_start;
    clone_data->env_end = request->env_end;
    clone_data->heap_start = request->heap_start;
    clone_data->heap_end = request->heap_end;
    clone_data->data_start = request->data_start;
    clone_data->data_end = request->data_end;
    memcpy(&clone_data->regs, &request->regs, sizeof(struct pt_regs) );
    memcpy(&clone_data->exe_path, &request->exe_path, sizeof(request->exe_path));
    clone_data->placeholder_pid = request->placeholder_pid;
    clone_data->placeholder_tgid = request->placeholder_tgid;
    clone_data->placeholder_cpu = source_cpu;
    clone_data->thread_fs = request->thread_fs;
    clone_data->thread_gs = request->thread_gs;
    clone_data->thread_sp0 = request->thread_sp0;
    clone_data->thread_sp = request->thread_sp;
    clone_data->thread_usersp = request->thread_usersp;
    clone_data->thread_es = request->thread_es;
    clone_data->thread_ds = request->thread_ds;
    clone_data->thread_fsindex = request->thread_fsindex;
    clone_data->thread_gsindex = request->thread_gsindex;
    clone_data->vma_list = NULL;
    clone_data->tgroup_home_cpu = request->tgroup_home_cpu;
    clone_data->tgroup_home_id = request->tgroup_home_id;
    clone_data->t_home_cpu = request->t_home_cpu;
    clone_data->t_home_id = request->t_home_id;
    clone_data->previous_cpus = request->previous_cpus;
    clone_data->prio = request->prio;
    clone_data->static_prio = request->static_prio;
    clone_data->normal_prio = request->normal_prio;
    clone_data->rt_priority = request->rt_priority;
    clone_data->sched_class = request->sched_class;
    clone_data->lock = __SPIN_LOCK_UNLOCKED(&clone_data->lock);

    /*
     * Pull in vma data
     */
    PS_SPIN_LOCK(&_data_head_lock);

    curr = _data_head;
    while(curr) {
        next = curr->next;

        if(curr->data_type == PROCESS_SERVER_VMA_DATA_TYPE) {
            vma = (vma_data_t*)curr;
            if(vma->clone_request_id == clone_data->clone_request_id &&
               vma->cpu == source_cpu ) {

                // Remove the data entry from the general data store
                remove_data_entry(vma);

                // Place data entry in this clone request's vma list
                PS_SPIN_LOCK(&clone_data->lock);
                if(clone_data->vma_list) {
                    clone_data->vma_list->header.prev = (data_header_t*)vma;
                    vma->header.next = (data_header_t*)clone_data->vma_list;
                } 
                clone_data->vma_list = vma;
                PS_SPIN_UNLOCK(&clone_data->lock);
            }
        }

        curr = next;
    }

    PS_SPIN_UNLOCK(&_data_head_lock);

    add_data_entry(clone_data);

perf_dd = native_read_tsc();
    clone_work = kmalloc(sizeof(clone_exec_work_t),GFP_ATOMIC);
    if(clone_work) {
        INIT_WORK( (struct work_struct*)clone_work, process_exec_item);
        clone_work->clone_data = clone_data;
        queue_work(clone_wq, (struct work_struct*)clone_work);
    }

    pcn_kmsg_free_msg(inc_msg);
perf_ee = native_read_tsc();
    PERF_MEASURE_STOP(&perf_handle_clone_request," ",perf);
    return 0;
}

/**
 *
 */
static int handle_back_migration(struct pcn_kmsg_message* inc_msg) {
    back_migration_t* msg = (back_migration_t*)inc_msg;
    back_migration_work_t* work;

    work = kmalloc(sizeof(back_migration_work_t),GFP_ATOMIC);
    if(work) {
        INIT_WORK( (struct work_struct*)work, process_back_migration);
        work->tgroup_home_cpu = msg->tgroup_home_cpu;
        work->tgroup_home_id  = msg->tgroup_home_id;
        work->t_home_cpu      = msg->t_home_cpu;
        work->t_home_id       = msg->t_home_id;
        work->previous_cpus   = msg->previous_cpus;
        work->thread_fs       = msg->thread_fs;
        work->thread_gs       = msg->thread_gs;
        work->thread_usersp   = msg->thread_usersp;
        work->thread_es       = msg->thread_es;
        work->thread_ds       = msg->thread_ds;
        work->thread_fsindex  = msg->thread_fsindex;
        work->thread_gsindex  = msg->thread_gsindex;
        memcpy(&work->regs, &msg->regs, sizeof(struct pt_regs));
        queue_work(clone_wq, (struct work_struct*)work);
    }

    pcn_kmsg_free_msg(inc_msg);

    return 0;
}


/**
 * Find the mm_struct for a given distributed thread.  If one does not exist,
 * then return NULL.
 */
static struct mm_struct* find_thread_mm(
        int tgroup_home_cpu, 
        int tgroup_home_id, 
        mm_data_t **used_saved_mm,
        struct task_struct** task_out) {

    struct task_struct *task, *g;
    struct mm_struct * mm = NULL;
    data_header_t* data_curr;
    mm_data_t* mm_data;

    *used_saved_mm = NULL;

    // First, look through all active processes.
    do_each_thread(g,task) {
        if(task->tgroup_home_cpu == tgroup_home_cpu &&
           task->tgroup_home_id  == tgroup_home_id) {
            mm = task->mm;
            *task_out = task;
            *used_saved_mm = 0;
            goto out;
        }
    } while_each_thread(g,task);

    // Failing that, look through saved mm's.
    PS_SPIN_LOCK(&_saved_mm_head_lock);
    data_curr = _saved_mm_head;
    while(data_curr) {

        mm_data = (mm_data_t*)data_curr;
    
        if((mm_data->tgroup_home_cpu == tgroup_home_cpu) &&
           (mm_data->tgroup_home_id  == tgroup_home_id)) {
            mm = mm_data->mm;
            *used_saved_mm = mm_data;
            break;
        }

        data_curr = data_curr->next;

    } // while

    PS_SPIN_UNLOCK(&_saved_mm_head_lock);


out:
    return mm;
}


/**
 *
 * Public API
 */

//statistics
static unsigned long long perf_a, perf_b, perf_c, perf_d, perf_e;

/**
 * If this is a delegated process, look up any records that may
 * exist of the remote placeholder processes page information,
 * and map those pages.
 *
 * Assumes current->mm->mmap_sem is already held.
 *
 * <MEASURED perf_process_server_import_address_space>
 */
int process_server_import_address_space(unsigned long* ip, 
        unsigned long* sp, 
        struct pt_regs* regs) {
    pte_data_t* pte_curr = NULL;
    vma_data_t* vma_curr = NULL;
    clone_data_t* clone_data = NULL;
    unsigned long err = 0;
    struct file* f;
    struct vm_area_struct* vma;
    int munmap_ret = 0;
    int mmap_flags = 0;
    int vmas_installed = 0;
    int ptes_installed = 0;
    struct mm_struct* thread_mm = NULL;
    struct task_struct* thread_task = NULL;
    mm_data_t* used_saved_mm = NULL;
    int perf = -1;

    perf_a = native_read_tsc();
    
    PSPRINTK("import address space\n");
    
    // Verify that we're a delegated task.
    if (!current->executing_for_remote) {
        PSPRINTK("ERROR - not executing for remote\n");
        return -1;
    }

    perf = PERF_MEASURE_START(&perf_process_server_import_address_space);

    clone_data = find_clone_data(current->prev_cpu,current->clone_request_id);
    if(!clone_data) {
        PERF_MEASURE_STOP(&perf_process_server_import_address_space,"Clone data missing, early exit",perf);
        return -1;
    }

    perf_b = native_read_tsc();    
    
    // Search for existing thread members to share an mm with.
    // Immediately set tgroup_home_<foo> under lock to keep 
    // from allowing multiple tgroups to be created when there 
    // are multiple tasks being migrated at the same time in
    // the same thread group.
    PS_DOWN_WRITE(&_import_sem);

    thread_mm = find_thread_mm(clone_data->tgroup_home_cpu,
                               clone_data->tgroup_home_id,
                               &used_saved_mm,
                               &thread_task);


    current->prev_cpu = clone_data->placeholder_cpu;
    current->prev_pid = clone_data->placeholder_pid;
    current->tgroup_home_cpu = clone_data->tgroup_home_cpu;
    current->tgroup_home_id = clone_data->tgroup_home_id;
    current->t_home_cpu = clone_data->t_home_cpu;
    current->t_home_id = clone_data->t_home_id;
    current->previous_cpus = clone_data->previous_cpus; // This has already
                                                        // been updated by the
                                                        // sending cpu.
                                                        //
    current->tgroup_distributed = 1;
    current->t_distributed = 1;

    PSPRINTK("%s: previous_cpus{%lx}\n",__func__,current->previous_cpus);
    PSPRINTK("%s: t_home_cpu{%d}\n",__func__,current->t_home_cpu);
    PSPRINTK("%s: t_home_id{%d}\n",__func__,current->t_home_id);
  
    if(!thread_mm) {
        
        PS_DOWN_WRITE(&current->mm->mmap_sem);

        // Gut existing mappings
        current->enable_distributed_munmap = 0;
        vma = current->mm->mmap;
        while(vma) {
            PSPRINTK("Unmapping vma at %lx\n",vma->vm_start);
            munmap_ret = do_munmap(current->mm, vma->vm_start, vma->vm_end - vma->vm_start);
            vma = current->mm->mmap;
        }
        current->enable_distributed_munmap = 1;

        // Clean out cache and tlb
        flush_tlb_mm(current->mm);
        flush_cache_mm(current->mm);
        PS_UP_WRITE(&current->mm->mmap_sem);
        
        // import exe_file
        f = filp_open(clone_data->exe_path,O_RDONLY | O_LARGEFILE, 0);
        if(f) {
            get_file(f);
            current->mm->exe_file = f;
            filp_close(f,NULL);
        }

        perf_c = native_read_tsc();    

        // Import address space
        vma_curr = clone_data->vma_list;

        while(vma_curr) {
            PSPRINTK("do_mmap() at %lx\n",vma_curr->start);
            if(vma_curr->path[0] != '\0') {
                mmap_flags = MAP_FIXED|MAP_PRIVATE;
                f = filp_open(vma_curr->path,
                                O_RDONLY | O_LARGEFILE,
                                0);
                if(f) {
                    PS_DOWN_WRITE(&current->mm->mmap_sem);
                    vma_curr->mmapping_in_progress = 1;
                    current->enable_do_mmap_pgoff_hook = 0;
                    err = do_mmap(f, 
                            vma_curr->start, 
                            vma_curr->end - vma_curr->start,
                            PROT_READ|PROT_WRITE|PROT_EXEC, 
                            mmap_flags, 
                            vma_curr->pgoff << PAGE_SHIFT);
                    vmas_installed++;
                    vma_curr->mmapping_in_progress = 0;
                    current->enable_do_mmap_pgoff_hook = 1;
                    PS_UP_WRITE(&current->mm->mmap_sem);
                    filp_close(f,NULL);
                    if(err != vma_curr->start) {
                        PSPRINTK("Fault - do_mmap failed to map %lx with error %lx\n",
                                vma_curr->start,err);
                    }
                }
            } else {
                mmap_flags = MAP_UNINITIALIZED|MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE;
                PS_DOWN_WRITE(&current->mm->mmap_sem);
                current->enable_do_mmap_pgoff_hook = 0;
                err = do_mmap(NULL, 
                    vma_curr->start, 
                    vma_curr->end - vma_curr->start,
                    PROT_READ|PROT_WRITE/*|PROT_EXEC*/, 
                    mmap_flags, 
                    0);
                current->enable_do_mmap_pgoff_hook = 1;
                vmas_installed++;
                //PSPRINTK("mmap error for %lx = %lx\n",vma_curr->start,err);
                PS_UP_WRITE(&current->mm->mmap_sem);
                if(err != vma_curr->start) {
                    PSPRINTK("Fault - do_mmap failed to map %lx with error %lx\n",
                            vma_curr->start,err);
                }
            }
           
            if(err > 0) {
                // mmap_region succeeded
                vma = find_vma(current->mm, vma_curr->start);
                PSPRINTK("vma mmapped, pulling in pte's\n");
                if(vma && (vma->vm_start <= vma_curr->start) && (vma->vm_end > vma_curr->start)) {
                    pte_curr = vma_curr->pte_list;
                    if(pte_curr == NULL) {
                        PSPRINTK("vma->pte_curr == null\n");
                    }
                    while(pte_curr) {
                        // MAP it
                        int domapping = 1;
                        pte_t* remap_pte = NULL;
                        pmd_t* remap_pmd = NULL;
                        pud_t* remap_pud = NULL;
                        pgd_t* remap_pgd = NULL;

                        PS_DOWN_WRITE(&current->mm->mmap_sem);
                        //PS_SPIN_LOCK(&_remap_lock);
                        remap_pgd = pgd_offset(current->mm, pte_curr->vaddr);
                        if(pgd_present(*remap_pgd)) {
                            remap_pud = pud_offset(remap_pgd,pte_curr->vaddr); 
                            if(pud_present(*remap_pud)) {
                                remap_pmd = pmd_offset(remap_pud,pte_curr->vaddr);
                                if(pmd_present(*remap_pmd)) {
                                    remap_pte = pte_offset_map(remap_pmd,pte_curr->vaddr);
                                    if(remap_pte && pte_none(*remap_pte)) {
                                        // skip the mapping, it already exists!
                                        domapping = 0;
                                    }
                                }
                            }
                        }

                        if(domapping) {
                            err = remap_pfn_range(vma,
                                                    pte_curr->vaddr,
                                                    pte_curr->paddr >> PAGE_SHIFT,
                                                    PAGE_SIZE,
                                                    vma->vm_page_prot);
                        }
                        ptes_installed++;
                        PS_UP_WRITE(&current->mm->mmap_sem);
                        //PS_SPIN_UNLOCK(&_remap_lock);
                        
                        pte_curr = (pte_data_t*)pte_curr->header.next;
                    }
                }
            }
            vma_curr = (vma_data_t*)vma_curr->header.next;
        }
    } else {
        struct mm_struct* oldmm;
        // use_existing_mm.  Flush cache, to ensure
        // that the current processes cache entries
        // are never used.  Then, take ownership of
        // the mm.
        oldmm = current->mm;
        if(oldmm == thread_mm) {
            PS_DOWN_WRITE(&oldmm->mmap_sem);
        } else {
            PS_DOWN_WRITE(&oldmm->mmap_sem);
            PS_DOWN_WRITE(&thread_mm->mmap_sem);
        }

        flush_tlb_mm(current->mm);
        flush_cache_mm(current->mm);
        mm_update_next_owner(current->mm);
        mmput(current->mm);
        current->mm = thread_mm;
        current->active_mm = current->mm;
        percpu_write(cpu_tlbstate.active_mm, thread_mm);

        if(NULL == used_saved_mm) {
            // Did not use a saved MM.  Saved MM's have artificially
            // incremented mm_users fields to keep them from being
            // destroyed when the last tgroup member exits.  So we can
            // just use the current value of mm_users.  Since in this case
            // we are not using a saved mm, we must increment mm_users.
            atomic_inc(&current->mm->mm_users);
        } else {
            // Used a saved MM.  Must delete the saved mm entry.
            // It is safe to do so now, since we have ingested
            // its mm at this point.
            PS_SPIN_LOCK(&_saved_mm_head_lock);
            remove_data_entry_from(used_saved_mm,&_saved_mm_head);
            PS_SPIN_UNLOCK(&_saved_mm_head_lock);
            kfree(used_saved_mm);
        }

        if(oldmm == thread_mm) {
            PS_UP_WRITE(&oldmm->mmap_sem);
        } else {
            PS_UP_WRITE(&oldmm->mmap_sem);
            PS_UP_WRITE(&thread_mm->mmap_sem);
        }

        // Transplant thread group information
        // if there are other thread group members
        // on this cpu.
        if(thread_task) {

            write_lock_irq(&tasklist_lock);
            PS_SPIN_LOCK(&thread_task->sighand->siglock);

            // Copy grouping info
            current->group_leader = thread_task->group_leader;
            current->tgid = thread_task->tgid;
            current->real_parent = thread_task->real_parent;
            current->parent_exec_id = thread_task->parent_exec_id;

            // Unhash sibling 
            list_del_init(&current->sibling);
            INIT_LIST_HEAD(&current->sibling);

             // Remove from tasks list, since this is not group leader.
             // We know that by virtue of the fact that we found another
             // thread group member.
            list_del_rcu(&current->tasks);

            // Signal related stuff
            current->signal = thread_task->signal;     
            atomic_inc(&thread_task->signal->live);
            atomic_inc(&thread_task->signal->sigcnt);
            thread_task->signal->nr_threads++;
            current->exit_signal = -1;
            
            // Sighand related stuff
            current->sighand = thread_task->sighand;   
            atomic_inc(&thread_task->sighand->count);

            // Rehash thread_group
            list_del_rcu(&current->thread_group);
            list_add_tail_rcu(&current->thread_group,
                              &current->group_leader->thread_group);

            // Reduce process count
             __this_cpu_dec(process_counts);

            PS_SPIN_UNLOCK(&thread_task->sighand->siglock);
            write_unlock_irq(&tasklist_lock);
           
            // copy fs
            // TODO: This should probably only happen when CLONE_FS is used...
            current->fs = thread_task->fs;
            PS_SPIN_LOCK(&current->fs->lock);
            current->fs->users++;
            PS_SPIN_UNLOCK(&current->fs->lock);

            // copy files
            // TODO: This should probably only happen when CLONE_FILES is used...
            current->files = thread_task->files;
            atomic_inc(&current->files->count);
        }
    }


    perf_d = native_read_tsc();

    // install memory information
    current->mm->start_stack = clone_data->stack_start;
    current->mm->start_brk = clone_data->heap_start;
    current->mm->brk = clone_data->heap_end;
    current->mm->env_start = clone_data->env_start;
    current->mm->env_end = clone_data->env_end;
    current->mm->arg_start = clone_data->arg_start;
    current->mm->arg_end = clone_data->arg_end;
    current->mm->start_data = clone_data->data_start;
    current->mm->end_data = clone_data->data_end;

    // install thread information
    // TODO: Move to arch
    current->thread.es = clone_data->thread_es;
    current->thread.ds = clone_data->thread_ds;
    current->thread.usersp = clone_data->thread_usersp;
   

    // Set output variables.
    *sp = clone_data->thread_usersp;
    *ip = clone_data->regs.ip;
    
    // adjust registers as necessary
    memcpy(regs,&clone_data->regs,sizeof(struct pt_regs)); 
    regs->ax = 0; // Fake success for the "sched_setaffinity" syscall
                  // that this process just "returned from"

/*    printk("%s: usermodehelper prio: %d static: %d normal: %d rt: %u class: %d rt_prio %d\n",
__func__,
    		current->prio, current->static_prio, current->normal_prio, current->rt_priority,
		current->policy, rt_prio (current->prio));
  */  
    current->prio = clone_data->prio;
    current->static_prio = clone_data->static_prio;
    current->normal_prio = clone_data->normal_prio;
    current->rt_priority = clone_data->rt_priority;
    current->policy = clone_data->sched_class;
/*    switch (clone_data->sched_class) {
    case SCHED_RR:
    case SCHED_FIFO:
    	current->sched_class = &rt_sched_class;
    	break;
    case SCHED_NORMAL:
    	current->sched_class = &fair_sched_class;
    	break;
    case SCHED_IDLE:
    	current->sched_class = &idle_sched_class;
    	break;
    }
*/  /*  printk("%s: clone_data prio: %d static: %d normal: %d rt: %u class: %d rt_prio %d\n",
__func__,
    		current->prio, current->static_prio, current->normal_prio, current->rt_priority,
		current->policy, rt_prio (current->prio));
*/
    // We assume that an exec is going on
    // and the current process is the one is executing
    // (a switch will occur if it is not the one that must execute)
    {
    unsigned long fs, gs;
    unsigned int fsindex, gsindex;
    savesegment(fs, fsindex);
    savesegment(gs, gsindex);
   
    rdmsrl(MSR_GS_BASE, gs);
    rdmsrl(MSR_FS_BASE, fs);
    
    if (clone_data->thread_fs && __user_addr(clone_data->thread_fs)) { // we update only if the 
                                                                       // address of the base fs is different 
                                                                       // from 0 and not represent a kernel address 
                                                                       // (we are migrating only the virtual address 
                                                                       // space of the process)
        current->thread.fs = clone_data->thread_fs;
        current->thread.fsindex = clone_data->thread_fsindex;

        if (unlikely(fsindex | current->thread.fsindex)) {
	        loadsegment(fs, current->thread.fsindex);
        }
        else { 
	        loadsegment(fs, 0);
        }

        if (current->thread.fs) {
	        wrmsrl(MSR_FS_BASE, current->thread.fs);  
        }
    }
    else { 
        loadsegment(fs, 0);
    }
       
    if (clone_data->thread_gs && __user_addr(clone_data->thread_gs)) {
        current->thread.gs = clone_data->thread_gs;    
        current->thread.gsindex = clone_data->thread_gsindex;
      
        if (unlikely(gsindex | current->thread.gsindex)) {
	        loadsegment(gs, current->thread.gsindex);
        }
        else {
	        load_gs_index(0);
        }

        if (current->thread.gs) {
	        wrmsrl(MSR_GS_BASE, current->thread.gs);
        }
    }
    else {
        load_gs_index(0);
    }
    
    }
       
    // Save off clone data, replacing any that may
    // already exist.
    if(current->clone_data) {
        PS_SPIN_LOCK(&_data_head_lock);
        remove_data_entry(current->clone_data);
        PS_SPIN_UNLOCK(&_data_head_lock);
        destroy_clone_data(current->clone_data);
    }
    current->clone_data = clone_data;

    PS_UP_WRITE(&_import_sem);

    dump_task(current,NULL,0);

    PERF_MEASURE_STOP(&perf_process_server_import_address_space, " ",perf);


    perf_e = native_read_tsc();
    printk("%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
            __func__,
            perf_aa, perf_bb, perf_cc, perf_dd, perf_ee,
            perf_a, perf_b, perf_c, perf_d, perf_e);

    return 0;
}


/**
 * Notify of the fact that either a delegate or placeholder has died locally.  
 * In this case, the remote cpu housing its counterpart must be notified, so
 * that it can kill that counterpart.
 *
 * <MEASURE perf_process_server_do_exit>
 */
int process_server_do_exit(void) {

    exiting_process_t msg;
    int tx_ret = -1;
    int is_last_thread_in_local_group = 1;
    int is_last_thread_in_group;
    struct task_struct *task, *g;
    mm_data_t* mm_data = NULL;
    int i;
    thread_group_exited_notification_t exit_notification;
    clone_data_t* clone_data;
    int perf = -1;

    // Select only relevant tasks to operate on
    if(!(current->t_distributed || current->tgroup_distributed)/* || 
            !current->enable_distributed_exit*/) {
        return -1;
    }

/*     printk("%s: CHANGED? prio: %d static: %d normal: %d rt: %u class: %d rt_prio %d\n",
		__func__,
                 current->prio, current->static_prio, current->normal_prio, current->rt_priority,
                 current->policy, rt_prio (current->prio));
*/
    perf = PERF_MEASURE_START(&perf_process_server_do_exit);

    PSPRINTK("%s - pid{%d}, prev_cpu{%d}, prev_pid{%d}\n",__func__,
            current->pid,
            current->prev_cpu,
            current->prev_pid);
    
    // Determine if this is the last _active_ thread in the 
    // local group.  We have to count shadow tasks because
    // otherwise we risk missing tasks when they are exiting
    // and migrating back.
    do_each_thread(g,task) {
        if(task->tgid == current->tgid &&           // <--- narrow search to current thread group only 
                task->pid != current->pid &&        // <--- don't include current in the search
                task->exit_state != EXIT_ZOMBIE &&  // <-,
                task->exit_state != EXIT_DEAD &&    // <-|- check to see if it's in a runnable state
                !(task->flags & PF_EXITING)) {      // <-'
            is_last_thread_in_local_group = 0;
            goto finished_membership_search;
        }
    } while_each_thread(g,task);
finished_membership_search:

    // Count the number of threads in this distributed thread group
    // this will be useful for determining what to do with the mm.
    if(!is_last_thread_in_local_group) {
        // Not the last local thread, which means we're not the
        // last in the distributed thread group either.
        is_last_thread_in_group = 0;
    } else if (!(task->t_home_cpu == _cpu && task->t_home_id == task->pid)) {
        // OPTIMIZATION: only bother to count threads if we are not home base for
        // this thread.
        is_last_thread_in_group = 0;
    } else {
        // Last local thread, which means we MIGHT be the last
        // in the distributed thread group, but we have to check.
        int count = count_thread_members();
        if (count == 0) {
            // Distributed thread count yielded no thread group members
            // so the current <exiting> task is the last group member.
            is_last_thread_in_group = 1;
        } else {
            // There are more thread group members.
            is_last_thread_in_group = 0;
        }
    }
    
    // Find the clone data, we are going to destroy this very soon.
    clone_data = find_clone_data(current->prev_cpu, current->clone_request_id);

    // Build the message that is going to migrate this task back 
    // from whence it came.
    msg.header.type = PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    msg.my_pid = current->pid;
    msg.t_home_id = current->t_home_id;
    msg.t_home_cpu = current->t_home_cpu;
    msg.is_last_tgroup_member = is_last_thread_in_group;

    if(current->executing_for_remote) {
        int i;
        // this task is dying. If this is a migrated task, the shadow will soon
        // take over, so do not mark this as executing for remote
        current->executing_for_remote = 0;

        // Migrate back - you just had an out of body experience, you will wake in
        //                a familiar place (a place you've been before), but unfortunately, 
        //                your life is over.
        //                Note: comments like this must == I am tired.
        for(i = 0; i < NR_CPUS; i++) {
            if(i != _cpu && test_bit(i,&current->previous_cpus)) {
                pcn_kmsg_send(i, 
                            (struct pcn_kmsg_message*)&msg);
            }
        }
    } 



    // If this was the last thread in the local work, we take one of two 
    // courses of action, either we:
    //
    // 1) determine that this is the last thread globally, and issue a 
    //    notification to that effect.
    //
    //    or.
    //
    // 2) we determine that this is NOT the last thread globally, in which
    //    case we save the mm to use to resolve mappings with.
    if(is_last_thread_in_local_group) {
        // Check to see if this is the last member of the distributed
        // thread group.
        if(is_last_thread_in_group) {

            PSPRINTK("%s: This is the last thread member!\n",__func__);

            // Notify all cpus
            exit_notification.header.type = PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION;
            exit_notification.header.prio = PCN_KMSG_PRIO_NORMAL;
            exit_notification.tgroup_home_cpu = current->tgroup_home_cpu;
            exit_notification.tgroup_home_id = current->tgroup_home_id;
            for(i = 0; i < NR_CPUS; i++) {
                if(i == _cpu) continue; // Don't bother notifying myself... I already know.
                pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&exit_notification));
            }

        } else {
            // This is NOT the last distributed thread group member.  Grab
            // a reference to the mm, and increase the number of users to keep 
            // it from being destroyed
            PSPRINTK("%s: This is not the last thread member, saving mm\n",
                    __func__);
            atomic_inc(&current->mm->mm_users);

            // Remember the mm
            mm_data = kmalloc(sizeof(mm_data_t),GFP_KERNEL);
            mm_data->header.data_type = PROCESS_SERVER_MM_DATA_TYPE;
            mm_data->mm = current->mm;
            mm_data->tgroup_home_cpu = current->tgroup_home_cpu;
            mm_data->tgroup_home_id  = current->tgroup_home_id;

            // Add the data entry
            add_data_entry_to(mm_data,
                              &_saved_mm_head_lock,
                              &_saved_mm_head);

        }

    } else {
        PSPRINTK("%s: This is not the last local thread member\n",__func__);
    }

    // We know that this task is exiting, and we will never have to work
    // with it again, so remove its clone_data from the linked list, and
    // nuke it.
    if(clone_data) {
        PS_SPIN_LOCK(&_data_head_lock);
        remove_data_entry(clone_data);
        PS_SPIN_UNLOCK(&_data_head_lock);
        destroy_clone_data(clone_data);
    }

    PERF_MEASURE_STOP(&perf_process_server_do_exit," ",perf);

    return 0;
}

/**
 * Create a pairing between a newly created delegate process and the
 * remote placeholder process.  This function creates the local
 * pairing first, then sends a message to the originating cpu
 * so that it can do the same.
 */
int process_server_notify_delegated_subprocess_starting(pid_t pid, pid_t remote_pid, int remote_cpu) {

    create_process_pairing_t msg;
    int tx_ret = -1;

    int perf = PERF_MEASURE_START(&perf_process_server_notify_delegated_subprocess_starting);

    PSPRINTK("kmkprocsrv: notify_subprocess_starting: pid{%d}, remote_pid{%d}, remote_cpu{%d}\n",pid,remote_pid,remote_cpu);
    
    // Notify remote cpu of pairing between current task and remote
    // representative task.
    msg.header.type = PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    msg.your_pid = remote_pid; 
    msg.my_pid = pid;
    
    DO_UNTIL_SUCCESS(pcn_kmsg_send_long(remote_cpu, 
                        (struct pcn_kmsg_long_message*)&msg, 
                        sizeof(msg) - sizeof(msg.header)));

    PERF_MEASURE_STOP(&perf_process_server_notify_delegated_subprocess_starting,
            " ",
            perf);

    return 0;

}

/**
 * If the current process is distributed, we want to make sure that all members
 * of this distributed thread group carry out the same munmap operation.  Furthermore,
 * we want to make sure they do so _before_ this syscall returns.  So, synchronously
 * command every cpu to carry out the munmap for the specified thread group.
 *
 * <MEASURE perf_process_server_do_munmap>
 */
int process_server_do_munmap(struct mm_struct* mm, 
            struct vm_area_struct *vma,
            unsigned long start, 
            unsigned long len) {

    munmap_request_data_t* data;
    munmap_request_t request;
    int i;
    int s;
    int perf = -1;

     // Nothing to do for a thread group that's not distributed.
    if(!current->tgroup_distributed || !current->enable_distributed_munmap) {
        goto exit;
    } 

    perf = PERF_MEASURE_START(&perf_process_server_do_munmap);

    data = kmalloc(sizeof(munmap_request_data_t),GFP_KERNEL);
    if(!data) goto exit;

    data->header.data_type = PROCESS_SERVER_MUNMAP_REQUEST_DATA_TYPE;
    data->vaddr_start = start;
    data->vaddr_size = len;
    data->responses = 0;
    data->expected_responses = 0;
    data->tgroup_home_cpu = current->tgroup_home_cpu;
    data->tgroup_home_id = current->tgroup_home_id;
    data->requester_pid = current->pid;
    spin_lock_init(&data->lock);

    add_data_entry_to(data,
                      &_munmap_data_head_lock,
                      &_munmap_data_head);

    request.header.type = PCN_KMSG_TYPE_PROC_SRV_MUNMAP_REQUEST;
    request.header.prio = PCN_KMSG_PRIO_NORMAL;
    request.vaddr_start = start;
    request.vaddr_size  = len;
    request.tgroup_home_cpu = current->tgroup_home_cpu;
    request.tgroup_home_id  = current->tgroup_home_id;
    request.requester_pid = current->pid;
    for(i = 0; i < NR_CPUS; i++) {

        // Skip the current cpu
        if (i == _cpu) continue;

        // Send the request to this cpu.
        s = pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&request));
        if(!s) {
            // A successful send operation, increase the number
            // of expected responses.
            data->expected_responses++;
        }
    }

    // Wait for all cpus to respond.
    while(data->expected_responses != data->responses) {
        schedule();
    }

    // OK, all responses are in, we can proceed.

    PS_SPIN_LOCK(&_munmap_data_head_lock);
    remove_data_entry_from(data,
                           &_munmap_data_head);
    PS_SPIN_UNLOCK(&_munmap_data_head_lock);

    kfree(data);

exit:

    PERF_MEASURE_STOP(&perf_process_server_do_munmap,"Exit success",perf);

    return 0;
}

/**
 * 
 */
void process_server_do_mprotect(struct task_struct* task,
                                unsigned long start,
                                size_t len,
                                unsigned long prot) {
    mprotect_data_t* data;
    mprotect_request_t request;
    int i;
    int s;
    int perf = -1;

     // Nothing to do for a thread group that's not distributed.
    if(!current->tgroup_distributed) {
        goto exit;
    }

    PSPRINTK("%s entered\n",__func__);

    perf = PERF_MEASURE_START(&perf_process_server_do_mprotect);

    data = kmalloc(sizeof(mprotect_data_t),GFP_KERNEL);
    if(!data) goto exit;

    data->header.data_type = PROCESS_SERVER_MPROTECT_DATA_TYPE;
    data->responses = 0;
    data->expected_responses = 0;
    data->tgroup_home_cpu = task->tgroup_home_cpu;
    data->tgroup_home_id = task->tgroup_home_id;
    data->requester_pid = task->pid;
    data->start = start;
    spin_lock_init(&data->lock);

    add_data_entry_to(data,
                      &_mprotect_data_head_lock,
                      &_mprotect_data_head);

    request.header.type = PCN_KMSG_TYPE_PROC_SRV_MPROTECT_REQUEST;
    request.header.prio = PCN_KMSG_PRIO_NORMAL;
    request.start = start;
    request.len  = len;
    request.prot = prot;
    request.tgroup_home_cpu = task->tgroup_home_cpu;
    request.tgroup_home_id  = task->tgroup_home_id;
    request.requester_pid = task->pid;

    PSPRINTK("Sending mprotect request to all other kernels... ");

    for(i = 0; i < NR_CPUS; i++) {

        // Skip the current cpu
        if (i == _cpu) continue;

        // Send the request to this cpu.
        s = pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&request));
        if(!s) {
            // A successful send operation, increase the number
            // of expected responses.
            data->expected_responses++;
        }
    }

    PSPRINTK("done\nWaiting for responses... ");

    // Wait for all cpus to respond.
    while(data->expected_responses != data->responses) {
        schedule();
    }

    PSPRINTK("done\n");

    // OK, all responses are in, we can proceed.

    PS_SPIN_LOCK(&_mprotect_data_head_lock);
    remove_data_entry_from(data,
                           &_mprotect_data_head);
    PS_SPIN_UNLOCK(&_mprotect_data_head_lock);

    kfree(data);

exit:

    PERF_MEASURE_STOP(&perf_process_server_do_mprotect," ",perf);

}

/**
 *
 */
unsigned long process_server_do_mmap_pgoff(struct file *file, unsigned long addr,
                                           unsigned long len, unsigned long prot,
                                           unsigned long flags, unsigned long pgoff) {

    int aggregate_start, aggregate_end;

    // Nothing to do for a thread group that's not distributed.
    // Also, skip if the mmap hook is turned off.  This should
    // only happen when memory mapping within process_server
    // fault handler and import address space.
    if(!current->tgroup_distributed || !current->enable_do_mmap_pgoff_hook) {
        goto not_handled_no_perf;
    }

    // Do a distributed munmap on the entire range of addresses that
    // are about to be remapped.  This will ensure that the range
    // is cleared out remotely, as well as locally (handled by the
    // do_mmap_pgoff implementation) to keep from having differing
    // vm address spaces on different cpus.
    process_server_do_munmap(current->mm,
                             /*struct vm_area_struct *vma*/NULL,
                             addr,
                             len);

not_handled_no_perf:
    return 0;
}

/**
 * Fault hook
 * 0 = not handled
 * 1 = handled
 *
 * <MEASURED perf_process_server_try_handle_mm_fault>
 */
int process_server_try_handle_mm_fault(struct mm_struct *mm, 
                                       struct vm_area_struct *vma,
                                       unsigned long address, 
                                       unsigned int flags, 
                                       struct vm_area_struct **vma_out,
                                       unsigned long error_code) {

    mapping_request_data_t *data;
    unsigned long err = 0;
    int ret = 0;
    mapping_request_t request;
    int i;
    int s;
    struct file* f;
    unsigned long prot = 0;
    int started_outside_vma = 0;
    char path[512];
    char* ppath;
    int do_data_free_here = 0;

    // for perf
    int pte_provided = 0;
    int is_anonymous = 0;
    int vma_not_found = 1;
    int adjusted_permissions = 0;
    int is_new_vma = 0;
    int perf = -1;
    int perf_send = -1;

    // Nothing to do for a thread group that's not distributed.
    if(!current->tgroup_distributed) {
        goto not_handled_no_perf;
    }

    perf = PERF_MEASURE_START(&perf_process_server_try_handle_mm_fault);

    PSPRINTK("Fault caught on address{%lx}, cpu{%d}, id{%d}, pid{%d}, tgid{%d}, error_code{%lx}\n",
            address,
            current->tgroup_home_cpu,
            current->tgroup_home_id,
            current->pid,
            current->tgid,
            error_code);

    if(is_vaddr_mapped(mm,address)) {
        PSPRINTK("exiting mk fault handler because vaddr %lx is already mapped- cpu{%d}, id{%d}\n",
                address,current->tgroup_home_cpu,current->tgroup_home_id);

        dump_regs(task_pt_regs(current)); 

        // should this thing be writable?  if so, set it and exit
        // This is a security hole, and is VERY bad.
        // It will also probably cause problems for genuine COW mappings..
        if(vma->vm_flags & VM_WRITE && 
                !is_page_writable(mm, vma, address & PAGE_MASK)) {
            PSPRINTK("Touching up write setting\n");
            mk_page_writable(mm,vma,address & PAGE_MASK);
            adjusted_permissions = 1;
            ret = 1;
        } else {
            //dump_mm(mm);
        }

        goto not_handled;
    }
    
    if(vma) {
        if(vma->vm_file) {
            ppath = d_path(&vma->vm_file->f_path,
                        path,512);
        } else {
            path[0] = '\0';
        }

        PSPRINTK("working with provided vma: start{%lx}, end{%lx}, path{%s}\n",vma->vm_start,vma->vm_end,path);
    }

    // The vma that's passed in might not always be correct.  find_vma fails by returning the wrong
    // vma when the vma is not present.  How ugly...
    if(vma && (vma->vm_start >= address || vma->vm_end <= address)) {
        started_outside_vma = 1;
        PSPRINTK("set vma = NULL, since the vma does not hold the faulting address, for whatever reason...\n");
        vma = NULL;
    } else if (vma) {
        PSPRINTK("vma found and valid\n");
    } else {
        PSPRINTK("no vma present\n");
    }

    data = kmalloc(sizeof(mapping_request_data_t),GFP_KERNEL); 
   
retry:

    // Set up data entry to share with response handler.
    // This data entry will be modified by the response handler,
    // and we will check it periodically to see if our request
    // has been responded to by all active cpus.
    data->header.data_type = PROCESS_SERVER_MAPPING_REQUEST_DATA_TYPE;
    data->address = address;
    data->present = 0;
    data->complete = 0;
    spin_lock_init(&data->lock);
    data->responses = 0;
    data->expected_responses = 0;
    data->paddr_mapping = 0;
    data->paddr_mapping_sz = 0;
    data->tgroup_home_cpu = current->tgroup_home_cpu;
    data->tgroup_home_id = current->tgroup_home_id;
    data->requester_pid = current->pid;

    // Make data entry visible to handler.
    add_data_entry_to(data,
                      &_mapping_request_data_head_lock,
                      &_mapping_request_data_head);

    // Send out requests, tracking the number of successful
    // send operations.  That number is the number of requests
    // we will expect back.
    request.header.type = PCN_KMSG_TYPE_PROC_SRV_MAPPING_REQUEST;
    request.header.prio = PCN_KMSG_PRIO_NORMAL;
    request.address = address;
    request.tgroup_home_cpu = current->tgroup_home_cpu;
    request.tgroup_home_id  = current->tgroup_home_id;
    request.requester_pid = current->pid;
    for(i = 0; i < NR_CPUS; i++) {

        // Skip the current cpu
        if(i == _cpu) continue; 
    
        // Send the request to this cpu.
        perf_send = PERF_MEASURE_START(&perf_pcn_kmsg_send);
        s = pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&request));
        PERF_MEASURE_STOP(&perf_pcn_kmsg_send,"fault handler",perf_send);
        if(!s) {
            // A successful send operation, increase the number
            // of expected responses.
            data->expected_responses++;
        }
    }

    // Wait for all cpus to respond, or a mapping that is complete
    // with a physical mapping.  Mapping results that do not include
    // a physical mapping cause this to wait until all mapping responses
    // have arrived from remote cpus.
    while(1) {
        int done = 0;
        PS_SPIN_LOCK(&data->lock);
        if(data->expected_responses == data->responses || data->complete)
            done = 1;
        PS_SPIN_UNLOCK(&data->lock);
        if (done) break;
        schedule();
    }

    // All cpus have now responded.
    // TODO Do another query here to check to see if we need
    //      to retry.  If another cpu has completed a mapping
    //      simultaneous with this mapping, we need to catch
    //      that.  Not implementing right now because if performance
    //      concerns.
    // Upon fail,
    // goto retry;

    // Handle successful response.
    if(data->present) {
        PSPRINTK("Mapping communicated: vaddr_start{%lx}, vaddr_mapping{%lx},vaddr_size{%lx},paddr{%lx},prot{%lx},vm_flags{%lx},path{%s},pgoff{%lx}\n",
                data->vaddr_start,
                data->vaddr_mapping, 
                data->vaddr_size,
                data->paddr_mapping, 
                data->prot, 
                data->vm_flags,
                data->path,
                data->pgoff);
        vma_not_found = 0;

        // Figure out how to protect this region.
        prot |= (data->vm_flags & VM_READ)?  PROT_READ  : 0;
        prot |= (data->vm_flags & VM_WRITE)? PROT_WRITE : 0;
        prot |= (data->vm_flags & VM_EXEC)?  PROT_EXEC  : 0;

        // If there was not previously a vma, create one.
        if(!vma || vma->vm_start != data->vaddr_start || vma->vm_end != (data->vaddr_start + data->vaddr_size)) {
            PSPRINTK("vma not present\n");
            is_new_vma = 1;
            if(data->path[0] == '\0') {       
                PSPRINTK("mapping anonymous\n");
                is_anonymous = 1;
                PS_DOWN_WRITE(&current->mm->mmap_sem);
                current->enable_distributed_munmap = 0;
                current->enable_do_mmap_pgoff_hook = 0;
                // mmap parts that are missing, while leaving the existing
                // parts untouched.
                err = do_mmap_remaining(NULL,
                        data->vaddr_start,
                        data->vaddr_size,
                        prot,
                        MAP_FIXED|
                        MAP_ANONYMOUS|
                        ((data->vm_flags & VM_SHARED)?MAP_SHARED:MAP_PRIVATE),
                        0);
                current->enable_distributed_munmap = 1;
                current->enable_do_mmap_pgoff_hook = 1;
                PS_UP_WRITE(&current->mm->mmap_sem);
            } else {
                PSPRINTK("opening file to map\n");
                is_anonymous = 0;

                // Temporary, check to see if the path is /dev/null (deleted), it should just
                // be /dev/null in that case.  TODO: Add logic to detect and remove the 
                // " (deleted)" from any path here.  This is important, because anonymous mappings
                // are sometimes, depending on how glibc is compiled, mapped instead to the /dev/zero
                // file, and without this check, the filp_open call will fail because the "(deleted)"
                // string at the end of the path results in the file not being found.
                if( !strncmp( "/dev/zero (deleted)", data->path, strlen("/dev/zero (deleted)")+1 )) {
                    data->path[9] = '\0';
                }

                f = filp_open(data->path, (data->vm_flags & VM_SHARED)? O_RDWR:O_RDONLY, 0);
                if(f) {
                    PSPRINTK("mapping file %s, %lx, %lx, %lx\n",data->path,
                            data->vaddr_start, 
                            data->vaddr_size,
                            (unsigned long)f);
                    PS_DOWN_WRITE(&current->mm->mmap_sem);
                    current->enable_distributed_munmap = 0;
                    current->enable_do_mmap_pgoff_hook = 0;
                    // mmap parts that are missing, while leaving the existing
                    // parts untouched.
                    err = do_mmap_remaining(f,
                            data->vaddr_start,
                            data->vaddr_size,
                            prot,
                            MAP_FIXED |
                            ((data->vm_flags & VM_DENYWRITE)?MAP_DENYWRITE:0) |
                            ((data->vm_flags & VM_EXECUTABLE)?MAP_EXECUTABLE:0) |
                            ((data->vm_flags & VM_SHARED)?MAP_SHARED:MAP_PRIVATE),
                            data->pgoff << PAGE_SHIFT);
                    current->enable_distributed_munmap = 1;
                    current->enable_do_mmap_pgoff_hook = 1;
                    PS_UP_WRITE(&current->mm->mmap_sem);
                    filp_close(f,NULL);
                }
            }
            if(err != data->vaddr_start) {
                PSPRINTK("ERROR: Failed to do_mmap %lx\n",err);
                goto exit_remove_data;
            }
            
            vma = find_vma(current->mm, data->vaddr_mapping);

            // Validate find_vma result
            if(vma->vm_start > data->vaddr_mapping || 
               vma->vm_end <= data->vaddr_mapping) {
                PSPRINTK("invalid find_vma result, invalidating\n");
                vma = NULL;
            } else {
                PSPRINTK("mapping successful\n");
            }
        } else {
            PSPRINTK("vma is present, using existing\n");
        }

        // We should have a vma now, so map physical memory into it.
        if(vma && data->paddr_mapping) { 
            pte_provided = 1;

            // PCD - Page Cache Disable
            // PWT - Page Write-Through (as opposed to Page Write-Back)

            PSPRINTK("About to map new physical pages - vaddr{%lx}, paddr{%lx}, vm_flags{%lx}, prot{%lx}\n",
                    data->vaddr_mapping,
                    data->paddr_mapping, 
                    vma->vm_flags, 
                    vma->vm_page_prot);
            // This grabs the lock
            if(break_cow(current->mm,vma,address)) {
                vma = find_vma(current->mm,address&PAGE_MASK);
            }

            PS_DOWN_WRITE(&current->mm->mmap_sem);
            err = remap_pfn_range_remaining(current->mm,
                                            vma,
                                            data->vaddr_mapping,
                                            data->paddr_mapping,
                                            data->paddr_mapping_sz,
                                            vm_get_page_prot(vma->vm_flags));
            PS_UP_WRITE(&current->mm->mmap_sem);

            // If this VMA specifies VM_WRITE, make the mapping writable.
            // this function does not do the flag check.  This is safe,
            // since COW mappings should be broken by the time this code
            // is invoked.
            if(vma->vm_flags & VM_WRITE) {
                mk_page_writable(mm, vma, data->vaddr_mapping);
            }

            // Check remap_pfn_range success
            if(err) {
                PSPRINTK("ERROR: Failed to remap_pfn_range %d\n",err);
            } else {
                PSPRINTK("remap_pfn_range succeeded\n");
                ret = 1;
            }
        } 

        if(vma) {
            *vma_out = vma;
        }
    }

exit_remove_data:

    // Wait for all cpus to respond.
    // We waited previously for this, but let's do it again.
    // The above wait is optimized to exit once a response is
    // received that has a physical mapping.  If we get such
    // a response, we know we can go ahead with the rest of the
    // mapping process, but we have to now finish taking
    // in responses and clean up.
    while(1) {
        int done = 0;
        PS_SPIN_LOCK(&data->lock);
        if(data->expected_responses == data->responses)
            done = 1;
        PS_SPIN_UNLOCK(&data->lock);
        if (done) break;
        schedule();
    }

    PSPRINTK("%s: doing data free\n",__func__);
    PS_SPIN_LOCK(&_mapping_request_data_head_lock);
    remove_data_entry_from(data,
                          &_mapping_request_data_head);
    PS_SPIN_UNLOCK(&_mapping_request_data_head_lock);

    kfree(data);

    PSPRINTK("exiting fault handler\n");

not_handled:

    if (adjusted_permissions) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,"Adjusted Permissions",perf);
    } else if (is_new_vma && is_anonymous && pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "New Anonymous VMA + PTE",
                perf);
    } else if (is_new_vma && is_anonymous && !pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "New Anonymous VMA + No PTE",
                perf);
    } else if (is_new_vma && !is_anonymous && pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "New File Backed VMA + PTE",
                perf);
    } else if (is_new_vma && !is_anonymous && !pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "New File Backed VMA + No PTE",
                perf);
    } else if (!is_new_vma && is_anonymous && pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "Existing Anonymous VMA + PTE",
                perf);
    } else if (!is_new_vma && is_anonymous && !pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "Existing Anonymous VMA + No PTE",
                perf);
    } else if (!is_new_vma && !is_anonymous && pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "Existing File Backed VMA + PTE",
                perf);
    } else if (!is_new_vma && !is_anonymous && !pte_provided) {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,
                "Existing File Backed VMA + No PTE",
                perf);
    } else {
        PERF_MEASURE_STOP(&perf_process_server_try_handle_mm_fault,"test",perf);
    }

    return ret;

not_handled_no_perf:

    return 0;
}

/**
 * Page walk has encountered a pte while deconstructing
 * the client side processes address space.  Transfer it.
 */
static int deconstruction_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {
    deconstruction_data_t* decon_data = (deconstruction_data_t*)walk->private;
    int vma_id = decon_data->vma_id;
    int dst_cpu = decon_data->dst_cpu;
    int clone_request_id = decon_data->clone_request_id;
    pte_transfer_t pte_xfer;

    if(NULL == pte || !pte_present(*pte)) {
        return 0;
    }

    pte_xfer.header.type = PCN_KMSG_TYPE_PROC_SRV_PTE_TRANSFER;
    pte_xfer.header.prio = PCN_KMSG_PRIO_NORMAL;
    pte_xfer.paddr = (pte_val(*pte) & PHYSICAL_PAGE_MASK) | (start & (PAGE_SIZE-1));
    // NOTE: Found the above pte to paddr conversion here -
    // http://wbsun.blogspot.com/2010/12/convert-userspace-virtual-address-to.html
    pte_xfer.vaddr = start;
    pte_xfer.vma_id = vma_id;
    pte_xfer.clone_request_id = clone_request_id;
    pte_xfer.pfn = pte_pfn(*pte);
    PSPRINTK("Sending PTE\n"); 
    DO_UNTIL_SUCCESS(pcn_kmsg_send(dst_cpu, (struct pcn_kmsg_message *)&pte_xfer));

    return 0;
}

/**
 * Propagate origin thread group to children, and initialize other 
 * task members.  If the parent was member of a remote thread group,
 * store the thread group info.
 *
 * Note: In addition to in this function, distribution information is
 * maintained when a thread or process is migrated.  That ensures
 * that all members of a thread group are kept up-to-date when
 * one member migrates.
 */
int process_server_dup_task(struct task_struct* orig, struct task_struct* task) {
    task->executing_for_remote = 0;
    task->represents_remote = 0;
    task->enable_do_mmap_pgoff_hook = 1;
    task->prev_cpu = -1;
    task->next_cpu = -1;
    task->prev_pid = -1;
    task->next_pid = -1;
    task->clone_data = NULL;

    task->t_home_cpu = _cpu;
    task->t_home_id  = task->pid;
    task->t_distributed = 0;
    task->previous_cpus = 0;
    task->return_disposition = RETURN_DISPOSITION_EXIT;

    // If this is pid 1 or 2, the parent cannot have been migrated
    // so it is safe to take on all local thread info.
    if(unlikely(orig->pid == 1 || orig->pid == 2)) {
        task->tgroup_home_cpu = _cpu;
        task->tgroup_home_id = orig->tgid;
        task->tgroup_distributed = 0;
        return 1;
    }
    // If the new task is not in the same thread group as the parent,
    // then we do not need to propagate the old thread info.
    if(orig->tgid != task->tgid) {
        task->tgroup_home_cpu = _cpu;
        task->tgroup_home_id = task->tgid;
        task->tgroup_distributed = 0;
        return 1;
    }

    // This is important.  We want to make sure to keep an accurate record
    // of which cpu and thread group the new thread is a part of.
    if(orig->executing_for_remote == 1 || orig->tgroup_home_cpu != _cpu) {
        task->tgroup_home_cpu = orig->tgroup_home_cpu;
        task->tgroup_home_id = orig->tgroup_home_cpu;
        task->tgroup_distributed = 1;
    } else {
        task->tgroup_home_cpu = _cpu;
        task->tgroup_home_id = orig->tgid;
        task->tgroup_distributed = 0;
    }

    return 1;

}

/**
 * Migrate the specified task <task> to cpu <cpu>
 * Currently, this function will put the specified task to 
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then create a new process and import that
 * info into its new context.  
 *
 * <MEASURE perf_process_server_do_migration>
 *
 */
static int do_migration_to_new_cpu(struct task_struct* task, int cpu) {
    struct pt_regs *regs = task_pt_regs(task);
    // TODO: THIS IS WRONG, task flags is not what I want here.
    unsigned long clone_flags = task->clone_flags;
    unsigned long stack_start = task->mm->start_stack;
    clone_request_t* request = kmalloc(sizeof(clone_request_t),GFP_KERNEL);
    struct task_struct* tgroup_iterator = NULL;
    struct task_struct* g;
    int dst_cpu = cpu;
    char path[256] = {0};
    char* rpath = d_path(&task->mm->exe_file->f_path,path,256);
    char lpath[256];
    char *plpath;
    struct vm_area_struct* curr = NULL;
    struct mm_walk walk = {
        .pte_entry = deconstruction_page_walk_pte_entry_callback,
        .mm = task->mm,
        .private = NULL
        };
    vma_transfer_t* vma_xfer = kmalloc(sizeof(vma_transfer_t),GFP_KERNEL);
    int lclone_request_id;
    deconstruction_data_t decon_data;
    int perf = -1;

    PSPRINTK("process_server_do_migration\n");
    dump_regs(regs);

    // Nothing to do if we're migrating to the current cpu
    if(dst_cpu == _cpu) {
        return PROCESS_SERVER_CLONE_FAIL;
    }

    perf = PERF_MEASURE_START(&perf_process_server_do_migration);

    // This will be a placeholder process for the remote
    // process that is subsequently going to be started.
    // Block its execution.
    __set_task_state(task,TASK_UNINTERRUPTIBLE);

    // Book keeping for previous cpu bitmask.
    set_bit(smp_processor_id(),&task->previous_cpus);

    // Book keeping for placeholder process.
    task->represents_remote = 1;
    task->t_distributed = 1;

    // Book keeping for distributed threads.
    task->tgroup_distributed = 1;
    do_each_thread(g,tgroup_iterator) {
        if(tgroup_iterator != task) {
            if(tgroup_iterator->tgid == task->tgid) {
                tgroup_iterator->tgroup_distributed = 1;
                tgroup_iterator->tgroup_home_id = task->tgroup_home_id;
                tgroup_iterator->tgroup_home_cpu = task->tgroup_home_cpu;
            }
        }
    } while_each_thread(g,tgroup_iterator);

    // Pick an id for this remote process request
    PS_SPIN_LOCK(&_clone_request_id_lock);
    lclone_request_id = _clone_request_id++;
    PS_SPIN_UNLOCK(&_clone_request_id_lock);

#if COPY_WHOLE_VM_WITH_MIGRATION
    // We have to break every cow page before migrating if we're
    // about to move the whole thing.
restart_break_cow_all:
    curr = task->mm->mmap;
    while(curr) {
        unsigned long addr;
        int broken = 0;
        for(addr = curr->vm_start; addr < curr->vm_end; addr += PAGE_SIZE) {
            if(break_cow(task->mm,curr,addr)) 
                broken = 1;
        }
        if(broken) 
            goto restart_break_cow_all;
        curr = curr->vm_next;
    }
#endif

    PS_DOWN_READ(&task->mm->mmap_sem);
    
    // Transfer VM Entries
    curr = task->mm->mmap;

    vma_xfer->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_TRANSFER;
    vma_xfer->header.prio = PCN_KMSG_PRIO_NORMAL;

    while(curr) {
        unsigned long start_stack = task->mm->start_stack;

        // Transfer the stack only
#if !(COPY_WHOLE_VM_WITH_MIGRATION)
        if(!((start_stack <= curr->vm_end && start_stack >= curr->vm_start))) {
            curr = curr->vm_next;
            continue;
        }
#endif

        //
        // re-initialize path.
        //
        if(curr->vm_file == NULL) {
            vma_xfer->path[0] = '\0';
        } else {
            plpath = d_path(&curr->vm_file->f_path,
                        lpath,256);
            strcpy(vma_xfer->path,plpath);
        }

        //
        // Transfer the vma
        //
        PS_SPIN_LOCK(&_vma_id_lock);
        vma_xfer->vma_id = _vma_id++;
        PS_SPIN_UNLOCK(&_vma_id_lock);
        vma_xfer->start = curr->vm_start;
        vma_xfer->end = curr->vm_end;
        vma_xfer->prot = curr->vm_page_prot;
        vma_xfer->clone_request_id = lclone_request_id;
        vma_xfer->flags = curr->vm_flags;
        vma_xfer->pgoff = curr->vm_pgoff;
        DO_UNTIL_SUCCESS(pcn_kmsg_send_long(dst_cpu, 
                            (struct pcn_kmsg_long_message*)vma_xfer, 
                            sizeof(vma_transfer_t) - sizeof(vma_xfer->header)));

        PSPRINTK("Anonymous VM Entry: start{%lx}, end{%lx}, pgoff{%lx}\n",
                curr->vm_start, 
                curr->vm_end,
                curr->vm_pgoff);

        decon_data.clone_request_id = lclone_request_id;
        decon_data.vma_id = vma_xfer->vma_id;
        decon_data.dst_cpu = dst_cpu;

        walk.private = &decon_data;
        walk_page_range(curr->vm_start,curr->vm_end,&walk);
    
        curr = curr->vm_next;
    }

    PS_UP_READ(&task->mm->mmap_sem);

    // Build request
    request->header.type = PCN_KMSG_TYPE_PROC_SRV_CLONE_REQUEST;
    request->header.prio = PCN_KMSG_PRIO_NORMAL;
    request->clone_flags = clone_flags;
    request->clone_request_id = lclone_request_id;
    memcpy( &request->regs, regs, sizeof(struct pt_regs) );
    strncpy( request->exe_path, rpath, 512 );
// struct mm_struct -----------------------------------------------------------
    request->stack_start = task->mm->start_stack;
    request->heap_start = task->mm->start_brk;
    request->heap_end = task->mm->brk;
    request->env_start = task->mm->env_start;
    request->env_end = task->mm->env_end;
    request->arg_start = task->mm->arg_start;
    request->arg_end = task->mm->arg_end;
    request->data_start = task->mm->start_data;
    request->data_end = task->mm->end_data;
// struct task_struct ---------------------------------------------------------    
    request->stack_ptr = stack_start;
    request->placeholder_pid = task->pid;
    request->placeholder_tgid = task->tgid;
    request->tgroup_home_cpu = task->tgroup_home_cpu;
    request->tgroup_home_id = task->tgroup_home_id;
    request->t_home_cpu = task->t_home_cpu;
    request->t_home_id = task->t_home_id;
    request->previous_cpus = task->previous_cpus;
    request->prio = task->prio;
    request->static_prio = task->static_prio;
    request->normal_prio = task->normal_prio;
    request->rt_priority = task->rt_priority;
    request->sched_class = task->policy;
// struct thread_struct -------------------------------------------------------
    // have a look at: copy_thread() arch/x86/kernel/process_64.c 
    // have a look at: struct thread_struct arch/x86/include/asm/processor.h
    {
      	unsigned long fs, gs;
	unsigned int fsindex, gsindex;
	unsigned int ds, es;
	
	    if (current != task)
	      PSPRINTK("DAVEK current is different from task!\n");

    request->thread_sp0 = task->thread.sp0;
    request->thread_sp = task->thread.sp;
    
    //printk("%s: usersp percpu %lx thread %lx\n", __func__, percpu_read(old_rsp), task->thread.usersp);
    // if (percpu_read(old_rsp), task->thread.usersp) set to 0 otherwise copy
    request->thread_usersp = task->thread.usersp;
    
    request->thread_es = task->thread.es;
    savesegment(es, es);          
    if ((current == task) && (es != request->thread_es))
      PSPRINTK("%s: DAVEK: es %x thread %x\n", __func__, es, request->thread_es);
      
    request->thread_ds = task->thread.ds;
    savesegment(ds, ds);
    if (ds != request->thread_ds)
      PSPRINTK("%s: DAVEK: ds %x thread %x\n", __func__, ds, request->thread_ds);
      
    request->thread_fsindex = task->thread.fsindex;
    savesegment(fs, fsindex);
    if (fsindex != request->thread_fsindex)
      PSPRINTK("%s: DAVEK: fsindex %x thread %x\n", __func__, fsindex, request->thread_fsindex);
      
    request->thread_gsindex = task->thread.gsindex;
    savesegment(gs, gsindex);
    if (gsindex != request->thread_gsindex)
      PSPRINTK("%s: DAVEK: gsindex %x thread %x\n", __func__, gsindex, request->thread_gsindex);
    
    request->thread_fs = task->thread.fs;
    rdmsrl(MSR_FS_BASE, fs);
    if (fs != request->thread_fs) {
      request->thread_fs = fs;
      PSPRINTK("%s: DAVEK: fs %lx thread %lx\n", __func__, fs, request->thread_fs);
    }

    request->thread_gs = task->thread.gs;
    rdmsrl(MSR_GS_BASE, gs);
    if (gs != request->thread_gs) {
      request->thread_gs = gs;
      PSPRINTK("%s: DAVEK: gs %lx thread %lx\n", __func__, fs, request->thread_gs);
    }
    // ptrace, debug, dr7: struct perf_event *ptrace_bps[HBP_NUM]; unsigned long debugreg6; unsigned long ptrace_dr7;
    // Fault info: unsigned long cr2; unsigned long trap_no; unsigned long error_code;
    // floating point: struct fpu fpu; THIS IS NEEDED
    // IO permissions: unsigned long *io_bitmap_ptr; unsigned long iopl; unsigned io_bitmap_max;
    }

    // Send request
    DO_UNTIL_SUCCESS(pcn_kmsg_send_long(dst_cpu, 
                        (struct pcn_kmsg_long_message*)request, 
                        sizeof(clone_request_t) - sizeof(request->header)));

    kfree(request);
    kfree(vma_xfer);

    //dump_task(task,regs,request->stack_ptr);
    
    PERF_MEASURE_STOP(&perf_process_server_do_migration,"migration to new cpu",perf);

    return PROCESS_SERVER_CLONE_SUCCESS;

}

/**
 * <MEASURE perf_process_server_do_migration>
 */
static int do_migration_back_to_previous_cpu(struct task_struct* task, int cpu) {
    back_migration_t mig;
    struct pt_regs* regs = task_pt_regs(task);

    int perf = -1;

    perf = PERF_MEASURE_START(&perf_process_server_do_migration);

    // Set up response header
    mig.header.type = PCN_KMSG_TYPE_PROC_SRV_BACK_MIGRATION;
    mig.header.prio = PCN_KMSG_PRIO_NORMAL;

    // Make mark on the list of previous cpus
    set_bit(smp_processor_id(),&task->previous_cpus);

    // Knock this task out.
    __set_task_state(task,TASK_UNINTERRUPTIBLE);
    
    // Update local state
    task->executing_for_remote = 0;
    task->represents_remote = 1;
    task->t_distributed = 1; // This should already be the case
    task->return_disposition = RETURN_DISPOSITION_EXIT;
    
    // Build message
    mig.tgroup_home_cpu = task->tgroup_home_cpu;
    mig.tgroup_home_id  = task->tgroup_home_id;
    mig.t_home_cpu      = task->t_home_cpu;
    mig.t_home_id       = task->t_home_id;
    mig.previous_cpus   = task->previous_cpus;
    mig.thread_fs       = task->thread.fs;
    mig.thread_gs       = task->thread.gs;
    mig.thread_usersp   = task->thread.usersp;
    mig.thread_es       = task->thread.es;
    mig.thread_ds       = task->thread.ds;
    mig.thread_fsindex  = task->thread.fsindex;
    mig.thread_gsindex  = task->thread.gsindex;
    memcpy(&mig.regs, regs, sizeof(struct pt_regs));

    // Send migration request to destination.
    pcn_kmsg_send_long(cpu,
                       (struct pcn_kmsg_long_message*)&mig,
                       sizeof(back_migration_t) - sizeof(struct pcn_kmsg_hdr));

    PERF_MEASURE_STOP(&perf_process_server_do_migration,"back migration",perf);

    return PROCESS_SERVER_CLONE_SUCCESS;
}

/**
 * Migrate the specified task <task> to cpu <cpu>
 * This function will put the specified task to 
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then create a new process and import that
 * info into its new context.  
 *
 * Note: There are now two different migration implementations.
 *       One is for the case where we are migrating to a cpu that
 *       has never before hosted this thread.  The other is where
 *       we are migrating to a cpu that has hosted this thread
 *       before.  There's a lot of stuff that we do not need
 *       to do when the thread has been there before, and the
 *       messaging data requirements are much less for that case.
 *
 */
int process_server_do_migration(struct task_struct* task, int cpu) {
   
    int ret = 0;

    if(test_bit(cpu,&task->previous_cpus)) {
        ret = do_migration_back_to_previous_cpu(task,cpu);
    } else {
        ret = do_migration_to_new_cpu(task,cpu);
    }

    return ret;
}

/**
 *
 */
void process_server_do_return_disposition(void) {

    PSPRINTK("%s\n",__func__);

    switch(current->return_disposition) {
    case RETURN_DISPOSITION_MIGRATE:
        // Nothing to do, already back-imported the
        // state in process_back_migration.  This will
        // remain a stub until something needs to occur
        // here.
        PSPRINTK("%s: return disposition migrate\n",__func__);
        break;
    case RETURN_DISPOSITION_EXIT:
        PSPRINTK("%s: return disposition exit\n",__func__);
    default:
        do_exit(0);
        break;
    }

    return;
}

/**
 * process_server_init
 * Start the process loop in a new kthread.
 */
static int __init process_server_init(void) {

    /*
     * Cache some local information.
     */
    _cpu = smp_processor_id();

    /*
     * Init global semaphores
     */
    init_rwsem(&_import_sem);

    /*
     * Create work queues so that we can do bottom side
     * processing on data that was brought in by the
     * communications module interrupt handlers.
     */
    clone_wq   = create_workqueue("clone_wq");
    exit_wq    = create_workqueue("exit_wq");
    mapping_wq = create_workqueue("mapping_wq");

    /*
     * Register to receive relevant incomming messages.
     */
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_PTE_TRANSFER, 
            handle_pte_transfer);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_VMA_TRANSFER, 
            handle_vma_transfer);
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
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MAPPING_RESPONSE_NONPRESENT,
            handle_nonpresent_mapping_response);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MUNMAP_REQUEST,
            handle_munmap_request);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MUNMAP_RESPONSE,
            handle_munmap_response);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_REQUEST,
            handle_remote_thread_count_request);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_COUNT_RESPONSE,
            handle_remote_thread_count_response);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_THREAD_GROUP_EXITED_NOTIFICATION,
            handle_thread_group_exited_notification);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MPROTECT_REQUEST,
            handle_mprotect_request);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_MPROTECT_RESPONSE,
            handle_mprotect_response);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PROC_SRV_BACK_MIGRATION,
            handle_back_migration);
    PERF_INIT(); 
    return 0;
}

/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(process_server_init);




