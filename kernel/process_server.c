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
#include <linux/slab.h>
#include <linux/process_server.h>
#include <linux/mm.h>
#include <linux/io.h> // ioremap
#include <linux/mman.h> // MAP_ANONYMOUS

#include <asm/cacheflush.h>

/**
 * Use the preprocessor to turn off printk.
 */
#define PROCESS_SERVER_VERBOSE 1
#if PROCESS_SERVER_VERBOSE
#define PSPRINTK(...) printk(__VA_ARGS__)
#else
#define PSPRINTK(...) ;
#endif

/**
 * Size of the mcomm receive buffer.
 */
#define RCV_BUF_SZ 0x4000

/**
 * Message passing data type definitions
 */
#define MAX_MSG_LEN sizeof(clone_request_t) // should always be the largest msg defined.
#define PROCESS_SERVER_MSG_CLONE_REQUEST 1
#define PROCESS_SERVER_MSG_PROCESS_PAIRING_REQUEST 2
#define PROCESS_SERVER_MSG_PROCESS_EXITING 3
#define PROCESS_SERVER_VMA_TRANSFER 4
#define PROCESS_SERVER_PTE_TRANSFER 5

/**
 * Library data type definitions
 */
#define PROCESS_SERVER_DATA_TYPE_TEST 0
#define PROCESS_SERVER_VMA_DATA_TYPE 1
#define PROCESS_SERVER_PTE_DATA_TYPE 2
#define PROCESS_SERVER_CLONE_DATA_TYPE 3


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
    unsigned long start;
    unsigned long end;
    int clone_request_id;
    int cpu;
    unsigned long flags;
    int vma_id;
    pgprot_t prot;
    unsigned long pgoff;
    char path[256];
    pte_data_t* pte_list;
} vma_data_t;

/**
 *
 */
typedef struct _clone_data {
    data_header_t header;
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
    struct pt_regs regs;
    unsigned long stack_size;
    int placeholder_pid;
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
    vma_data_t* vma_list;
} clone_data_t;
/**
 * Message header.  All messages passed between
 * cpu's must have this header as the first
 * struct member.
 */
typedef struct _msg_header {
    int msg_type;
    int msg_len; // not including header
    int msg_id;
    int is_response; // Not sure if this will get used...
} msg_header_t;

/**
 * This message is sent to a remote cpu in order to 
 * ask it to spin up a process on behalf of the
 * requesting cpu.  Some of these fields may go
 * away in the near future.
 */
typedef struct _clone_request {
    msg_header_t header;
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
    struct pt_regs regs;
    unsigned long stack_size;
    char exe_path[512];
    int placeholder_pid;
    unsigned long thread_fs;
    unsigned long thread_gs;
    unsigned long thread_sp0;
    unsigned long thread_sp;
    unsigned long thread_usersp;
    unsigned short thread_es;
    unsigned short thread_ds;
    unsigned short thread_fsindex;
    unsigned short thread_gsindex;
} clone_request_t;

/**
 * This message is sent in response to a clone request.
 * Its purpose is to notify the requesting cpu that
 * the specified pid is executing on behalf of the
 * requesting cpu.
 */
typedef struct _create_process_pairing {
    msg_header_t header;
    int your_pid; // PID of cpu receiving this pairing request
    int my_pid;   // PID of cpu transmitting this pairing request
} create_process_pairing_t;

/**
 * This message informs the remote cpu of delegated
 * process death.  This occurs whether the process
 * is a placeholder or a delegate locally.
 */
typedef struct _exiting_process {
    msg_header_t header;
    int my_pid; // PID of process on cpu transmitting this exit notification.
} exiting_process_t;

/**
 * Inform remote cpu of a vma to process mapping.
 */
typedef struct _vma_transfer {
    msg_header_t header;
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
typedef struct _pte_transfer {
    msg_header_t header;
    int vma_id;
    unsigned long vaddr;
    unsigned long paddr;
    unsigned long pfn;
} pte_transfer_t;

/**
 * This message is sent when a signal is delivered
 * to a process that is either a placeholder for
 * a remotely executing process, or is the
 * remotely executing process.
 */
typedef struct _propagate_signal {
    msg_header_t header;
    int originating_cpu;
    int originating_dst_pid;
    int sig;
} propagate_signal_t;


/**
 * Prototypes
 */
static void handle_clone_request(clone_request_t* request,int source_cpu);
static int msg_tx(int dst, void* data, int data_len);
static void comms_handler(int source, char* data, int data_len);
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,                                                                                                                   
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          struct task_struct* task);
static int process_server(void* dummy);
static void dump_mm(struct mm_struct* mm);
static void dump_task(struct task_struct* task,struct pt_regs* regs);
static void dump_thread(struct thread_struct* thread);
static void dump_regs(struct pt_regs* regs);
static void dump_stk(unsigned long sp); 

/**
 * Module variables
 */
static struct task_struct* process_server_task = NULL;     // Remember the kthread task_struct
static comm_buffers* _comm_buf = NULL;
static int _initialized = 0;
static int _msg_id = 0;
static int _vma_id = 0;
static int _clone_request_id = 0;
static int _cpu = -1;
char _rcv_buf[RCV_BUF_SZ] = {0};
clone_data_t* _clone_data = NULL;
vma_data_t* _unclaimed_vma_data = NULL;
static scheduler_fn_t _scheduler_impl;
DEFINE_SPINLOCK(_msg_id_lock);             // Lock for _msg_id
DEFINE_SPINLOCK(_vma_id_lock);             // Lock for _vma_id
DEFINE_SPINLOCK(_clone_request_id_lock);   // Lock for _clone_request_id
DEFINE_SPINLOCK(_clone_data_lock);
DEFINE_SPINLOCK(_unclaimed_vma_data_lock); // Lock for _unclaimed_vma_data


/**
 * General helper functions and debugging tools
 */

/**
 * Copied directly from mm/mmap.c
 */
static void __vma_link_file(struct vm_area_struct *vma)
{   
    struct file *file;

    file = vma->vm_file;
    if (file) {
        struct address_space *mapping = file->f_mapping;

        if (vma->vm_flags & VM_DENYWRITE)
            atomic_dec(&file->f_path.dentry->d_inode->i_writecount);
        if (vma->vm_flags & VM_SHARED)
            mapping->i_mmap_writable++;

        flush_dcache_mmap_lock(mapping);
        if (unlikely(vma->vm_flags & VM_NONLINEAR))
            vma_nonlinear_insert(vma, &mapping->i_mmap_nonlinear);
        else
            vma_prio_tree_insert(vma, &mapping->i_mmap);                                                                                                                
        flush_dcache_mmap_unlock(mapping);
    }
}

/**
 *
 */
void dump_task(struct task_struct* task, struct pt_regs* regs) {
#if PROCESS_SERVER_VERBOSE
    if(NULL == task) return;

    PSPRINTK("DUMP TASK\n");
    PSPRINTK("PID: %d\n",task->pid);
    PSPRINTK("State: %lx\n",task->state);
    PSPRINTK("Flags: %lx\n",task->flags);
    PSPRINTK("Ptrace: %lx\n",task->ptrace);
#ifdef CONFIG_SMP
    PSPRINTK("ON_CPU: %d\n",task->on_cpu);
#endif
    PSPRINTK("Prio{%d},Static_Prio{%d},Normal_Prio{%d}\n",
            task->prio,task->static_prio,task->normal_prio);
    PSPRINTK("Represents_remote{%d}\n",task->represents_remote);
    PSPRINTK("Executing_for_remote{%d}\n",task->executing_for_remote);
    PSPRINTK("Remote_pid{%d}\n",task->remote_pid);
    PSPRINTK("Remote_cpu{%d}\n",task->remote_cpu);
    PSPRINTK("Clone_request_id{%d}\n",task->clone_request_id);
    if(regs) dump_regs(regs);
    dump_thread(&task->thread);
    dump_mm(task->mm);
    if(task->mm) dump_stk(task->thread.usersp);
#endif
}

/**
 *
 */
static void dump_stk(unsigned long sp) {
    int i;
    PSPRINTK("DUMP STACK\n");
    PSPRINTK("sp = %lx\n",sp);
    for(i = -16; i <= 16; i++) {
        PSPRINTK("stack peak %lx at %lx\n",*(unsigned long*)(sp + i*8), sp + i*8); 
    }
}

/**
 *
 */
static void dump_regs(struct pt_regs* regs) {
    if(NULL == regs) return;
    PSPRINTK("DUMP REGS\n");
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

/**
 *
 */
static void dump_thread(struct thread_struct* thread) {
    PSPRINTK("DUMP THREAD\n");
    PSPRINTK("sp0{%lx}, sp{%lx}\n",thread->sp0,thread->sp);
    PSPRINTK("usersp{%lx}\n",thread->usersp);
    PSPRINTK("es{%lx}\n",thread->es);
    PSPRINTK("ds{%lx}\n",thread->ds);
    PSPRINTK("fsindex{%lx}\n",thread->fsindex);
    PSPRINTK("gsindex{%lx}\n",thread->gsindex);
    PSPRINTK("fs{%lx}\n",thread->fs);
    PSPRINTK("gs{%lx}\n",thread->gs);
}

/**
 *
 */
static int dump_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {

    if(NULL == pte || !pte_present(*pte)) {                                                                                                                             
        return 0;
    }

    PSPRINTK("pte_entry start{%lx}, end{%lx}, phy{%lx}\n",
            start,
            end,
            (unsigned long)(pte_val(*pte) & PHYSICAL_PAGE_MASK) | (start & (PAGE_SIZE-1)));
    return 0;
}

/**
 * Print mm
 */
static void dump_mm(struct mm_struct* mm) {

    struct vm_area_struct * curr;
    struct mm_walk walk = {
        .pte_entry = dump_page_walk_pte_entry_callback,
        .mm = mm,
        .private = NULL
        };
    char buf[256];

    if(NULL == mm) {
        PSPRINTK("MM IS NULL!\n");
        return;
    }

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
        }
        curr = curr->vm_next;
    }
}

/**
 * Data library
 */

/**
 * Add to the beginning of the list
 */
static void add_clone_to_clone_list(clone_data_t* clone) {
    PSPRINTK("add_clone_to_clone_list\n");
    spin_lock(&_clone_data_lock);
    
    if(_clone_data) {
        _clone_data->header.prev = clone;
    }
    clone->header.next = _clone_data;
    _clone_data = clone;

    spin_unlock(&_clone_data_lock);
}

static clone_data_t* find_clone_data(int clone_request_id, int cpu) {
    PSPRINTK("find_clone_data\n");
    clone_data_t* curr = _clone_data;
    clone_data_t* ret = NULL;
    while(curr) {
        if(curr->clone_request_id == clone_request_id &&
           curr->placeholder_cpu == cpu) {
            ret = curr;
            break;
        }
    }

    return ret;
}

/**
 * _clone_data_lock must be held 
 */
static void add_vma_to_clone_list(vma_data_t* vma, clone_data_t* d) {
    PSPRINTK("add_vma_to_clone_list\n");
    clone_data_t* curr = NULL;

    if(d->clone_request_id == vma->clone_request_id) {
        // Add to the front of this one
        if(d->vma_list) {
            d->vma_list->header.prev = vma;
        }
        vma->header.next = d->vma_list;
        d->vma_list = vma;
    }
}

/**
 * _unclaimed_vma_data_lock must be held
 */
static void add_vma_to_unclaimed_list(vma_data_t* vma) {
    PSPRINTK("add_vma_to_unclaimed_list\n");
    if(_unclaimed_vma_data) {
        _unclaimed_vma_data->header.prev = vma;
    }
    vma->header.next = _unclaimed_vma_data;
    _unclaimed_vma_data = vma;
}

/**
 * _unclaimed_vma_data_lock must be held
 */
static void remove_vma_from_unclaimed_list(vma_data_t* vma) {
    PSPRINTK("remove_vma_from_unclaimed_list\n");
    if(_unclaimed_vma_data == vma) {
        _unclaimed_vma_data = vma->header.next;
    }
    if(vma->header.next) {
        vma->header.next->prev = NULL;
    }
    vma->header.next = NULL;
    vma->header.prev = NULL;
}

/**
 *
 */
static void add_pte_to_vma_list(pte_data_t* pte) {
    PSPRINTK("add_pte_to_vma_list\n");
    vma_data_t* curr = NULL;
    spin_lock(&_unclaimed_vma_data_lock);
    
    curr = _unclaimed_vma_data;
    while(curr) {

        if(curr->vma_id == pte->vma_id &&
           curr->cpu == pte->cpu) {
            PSPRINTK("found vma\n");
            // add to the front of this one
            if(curr->pte_list) {
                PSPRINTK("list is occupied\n");
                curr->pte_list->header.prev = (data_header_t*)pte;
            }
            pte->header.next = curr->pte_list;
            curr->pte_list = pte;

            break;
        }

        curr = curr->header.next;
    }

    spin_unlock(&_unclaimed_vma_data_lock);
    PSPRINTK("Exiting add_pte_to_vma_list\n");
}

static void import_vma_data(clone_data_t* d) {
    PSPRINTK("import_vma_data\n");
    vma_data_t* curr = NULL;
    vma_data_t* next = NULL;
    spin_lock(&_clone_data_lock);
    spin_lock(&_unclaimed_vma_data_lock);

    curr = _unclaimed_vma_data;
    while(curr) {
        next = curr->header.next;
        if(curr->clone_request_id == d->clone_request_id &&
           curr->cpu == d->placeholder_cpu) {
            remove_vma_from_unclaimed_list(curr);
            add_vma_to_clone_list(curr,d);            
        }
        curr = next;
    }

    spin_unlock(&_unclaimed_vma_data_lock);
    spin_unlock(&_clone_data_lock);
}

/**
 * Request implementations
 */

/**
 *
 */
static void handle_pte_transfer(pte_transfer_t* msg, int source_cpu) {
    
    pte_data_t* pte_data = kmalloc(sizeof(pte_data_t),GFP_KERNEL);
    if(!pte_data) {
        PSPRINTK("Failed to allocate pte_data_t\n");
        return;
    }
    pte_data->header.data_type = PROCESS_SERVER_PTE_DATA_TYPE;

    // Copy data into new data item.
    pte_data->cpu = source_cpu;
    pte_data->vma_id = msg->vma_id;
    pte_data->vaddr = msg->vaddr;
    pte_data->paddr = msg->paddr;
    pte_data->pfn = msg->pfn;

    add_pte_to_vma_list(pte_data);
    
}

/**
 *
 */
static void handle_vma_transfer(vma_transfer_t* msg, int source_cpu) {

    vma_data_t* vma_data = kmalloc(sizeof(vma_data_t),GFP_KERNEL);
    if(!vma_data) {
        PSPRINTK("Failed to allocate vma_data_t\n");
        return;
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
    strcpy(vma_data->path,msg->path);

    add_vma_to_unclaimed_list(vma_data);
    //add_data_entry(vma_data); 
    
}

/**
 * Handler function for when either a remote placeholder or a remote delegate process dies,
 * and its local counterpart must be killed to reflect that.
 */
static void handle_exiting_process_notification(exiting_process_t* msg, int source_cpu) {

    struct task_struct* task;
    PSPRINTK("kmkprocsrv: exiting process remote_pid{%d}, src_cpu{%d}\n",msg->my_pid,source_cpu);

    for_each_process(task) {
        if(task->remote_pid == msg->my_pid) {
            PSPRINTK("kmkprocsrv: killing local task pid{%d}\n",task->pid);
            __set_task_state(task,TASK_INTERRUPTIBLE);
            kill_pid(task_pid(task),SIGKILL,1);

            break; // No need to continue;
        }
    }
}

/**
 * Handler function for when another processor informs the current cpu
 * of a pid pairing.
 */
static void handle_process_pairing_request(create_process_pairing_t* msg, int source_cpu) {

    struct task_struct* task;
    if(msg == NULL) {
        return;
    }

    /*
     * Go through all the processes looking for the one with the right pid.
     * Once that task is found, do the bookkeeping necessary to remember
     * the remote cpu and pid information.
     */
    for_each_process(task) {
        if(task->pid == msg->your_pid && task->represents_remote) {
            task->remote_cpu = source_cpu;
            task->remote_pid = msg->my_pid;
            task->executing_for_remote = 0;
 
            PSPRINTK("kmkprocsrv: Added paring at request remote_pid{%d}, local_pid{%d}, remote_cpu{%d}",
                    task->remote_pid,
                    task->pid,
                    task->remote_cpu);

            break; // No need to continue;
        }
    }
}

/**
 * Handle clone requests. 
 */
static void handle_clone_request(clone_request_t* request, int source_cpu) {
    clone_data_t* clone_data;
    struct subprocess_info* sub_info;
    char* argv[] = {request->exe_path,NULL,NULL};
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
    };

    PSPRINTK("Handling clone request: %lx\n",request->clone_flags);

    sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC );
    if (sub_info == NULL) return;

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
    sub_info->remote_pid = request->placeholder_pid;
    sub_info->remote_cpu = source_cpu;
    sub_info->clone_request_id = request->clone_request_id;
    memcpy(&sub_info->remote_regs, &request->regs, sizeof(struct pt_regs) );

    PSPRINTK("subinfo eip{%lx}\n",sub_info->remote_regs.ip);

    /*
     * Remember this request
     */
    clone_data = kmalloc(sizeof(clone_data_t),GFP_KERNEL);
    clone_data->header.data_type = PROCESS_SERVER_CLONE_DATA_TYPE;
    clone_data->clone_request_id = request->clone_request_id;
    clone_data->clone_flags = request->clone_flags;
    clone_data->stack_start = request->stack_start;
    clone_data->stack_ptr = request->stack_ptr;
    clone_data->arg_start = request->arg_start;
    clone_data->arg_end = request->arg_end;
    clone_data->env_start = request->env_start;
    clone_data->env_end = request->env_end;
    clone_data->heap_start = request->heap_start;
    clone_data->heap_end = request->heap_end;
    memcpy(&clone_data->regs, &request->regs, sizeof(struct pt_regs) );
    clone_data->stack_size = request->stack_size;
    clone_data->placeholder_pid = request->placeholder_pid;
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
    
    import_vma_data(clone_data);
    add_clone_to_clone_list(clone_data);
    //add_data_entry(clone_data);

    /*
     * Spin up the new process.
     */
    call_usermodehelper_exec(sub_info, UMH_NO_WAIT);
}

/**
 * Message passing helper functions
 */

/**
 * Helper function to send a message to some destination
 * cpu.
 */
static int msg_tx(int dst, void* data, int data_len) {
    int ret = -1;
    msg_header_t* msg = (msg_header_t*)data;

    if(data_len <= 0) {
        return -1;
    }

    // Increase the message ID by one.
    spin_lock(&_msg_id_lock);
    msg->msg_id = _msg_id++;
    spin_unlock(&_msg_id_lock);

    ret = matrix_send_to( _comm_buf, dst, data, data_len );

    return ret;
}

/**
 * Routes commands to the appropriate handler.
 */
static void comms_handler(int source_cpu, char* data, int data_len) {
    msg_header_t* hdr = NULL;
    if (data == NULL) {
        goto error;
    }

    if (data_len <= 0) {
        goto error;
    }

    if (source_cpu < 0) {
        goto error;
    }

    // Grab the header to figure out the message type
    hdr = (msg_header_t*)data;

    PSPRINTK("kmkprocsrv: Routing message to handler msg_type{%d}\n",hdr->msg_type);

    // Clone Request
    if (hdr->msg_type == PROCESS_SERVER_MSG_CLONE_REQUEST) {
        clone_request_t* request = (clone_request_t*)data;
        // Check message integrity.
        if(request->header.msg_len == sizeof(clone_request_t) - sizeof(msg_header_t)) {
            // Handle this message.
            handle_clone_request(request,source_cpu);
        } 
    }

    // Process Pairing Request
    else if (hdr->msg_type == PROCESS_SERVER_MSG_PROCESS_PAIRING_REQUEST) {
        create_process_pairing_t* request = (create_process_pairing_t*)data;
        // Check message integrity.
        if(request->header.msg_len == sizeof(create_process_pairing_t) - sizeof(msg_header_t)) {
            // Handle this message.
            handle_process_pairing_request(request,source_cpu);
        }
    }

    // Process Exiting Notification
    else if (hdr->msg_type == PROCESS_SERVER_MSG_PROCESS_EXITING) {
        exiting_process_t* request = (exiting_process_t*)data;
        if(request->header.msg_len == sizeof(exiting_process_t) - sizeof(msg_header_t)) {
            // Handle this message
            handle_exiting_process_notification(request,source_cpu);
        }
    }

    else if (hdr->msg_type == PROCESS_SERVER_VMA_TRANSFER) {
        vma_transfer_t* msg = (vma_transfer_t*)data;
        if(msg->header.msg_len == sizeof(vma_transfer_t) - sizeof(msg_header_t)) {
            // Handle this message
            handle_vma_transfer(msg,source_cpu);
        }
    }

    else if (hdr->msg_type == PROCESS_SERVER_PTE_TRANSFER) {
        pte_transfer_t* msg = (pte_transfer_t*)data;
        if(msg->header.msg_len == sizeof(pte_transfer_t) - sizeof(msg_header_t)) {
            // Handle this message
            handle_pte_transfer(msg,source_cpu);
        }
    }

    else {
        PSPRINTK("error parsing message id{%d},len{%d},type{%d}\n",
                hdr->msg_id,
                hdr->msg_len,
                hdr->msg_type);
    }

error:
    return;
}


/**
 *
 * Public API
 */

/**
 * If this is a delegated process, look up any records that may
 * exist of the remote placeholder processes page information,
 * and map those pages.
 *
 * Assumes current->mm->mmap_sem is already held.
 */
int process_server_import_address_space(unsigned long* ip, 
        unsigned long* sp, 
        struct pt_regs* regs) {

    int remote_cpu = current->remote_cpu;
    int clone_request_id = current->clone_request_id; 
    data_header_t* data_curr = NULL;
    pte_data_t* pte_curr = NULL;
    vma_data_t* vma_curr = NULL;
    clone_data_t* clone_data = NULL;
    unsigned long err = 0;
    struct file* f;
    int mmap_ret = 0;
    struct vm_area_struct* vma;
    int munmap_ret = 0;
    int mmap_flags = 0;

    // Verify that we're a delegated task.
    if (!current->executing_for_remote) {
        return -1;
    }

    // Gut existing mappings
    PSPRINTK("Removing existing mappings\n");
    vma = current->mm->mmap;
    while(vma) {
        PSPRINTK("Removing entry at %lx - ",vma->vm_start);
        munmap_ret = do_munmap(current->mm, vma->vm_start, vma->vm_end - vma->vm_start);
        PSPRINTK("%d\n",munmap_ret);
        vma = current->mm->mmap;
    }


    clone_data = find_clone_data(current->clone_request_id, current->remote_cpu);
    if(!clone_data) {
        return -1;
    }

    PSPRINTK("Have clone_data\n");

    // install memory information
    current->mm->start_stack = clone_data->stack_start;
    current->mm->start_brk = clone_data->heap_start;
    current->mm->brk = clone_data->heap_end;
    current->mm->env_start = clone_data->env_start;
    current->mm->env_end = clone_data->env_end;
    current->mm->arg_start = clone_data->arg_start;
    current->mm->arg_end = clone_data->arg_end;

    // install thread information
    current->thread.fs = clone_data->thread_fs;
    current->thread.gs = clone_data->thread_gs;
    current->thread.sp0 = clone_data->thread_sp0;
    current->thread.sp = clone_data->thread_sp;
    current->thread.usersp = clone_data->thread_usersp;
    current->thread.es = clone_data->thread_es;
    current->thread.ds = clone_data->thread_ds;
    current->thread.fsindex = clone_data->thread_fsindex;
    current->thread.gsindex = clone_data->thread_gsindex;

    PSPRINTK("clone_data usersp = %lx\n",current->thread.usersp);

    // Set output variables.
    *sp = clone_data->stack_ptr;
    *ip = clone_data->regs.ip;
    memcpy(regs,&clone_data->regs,sizeof(struct pt_regs)); 

    vma_curr = clone_data->vma_list;

    while(vma_curr) {            
        PSPRINTK("Processing vma_curr\n");
        // This is our vma.
        // Iterate through all pte's looking for vma_id and cpu matches.
        // We can start at the current place in the list, since VMA is transfered
        // before any pte's and all new entries are added to the end of the list.
        if(vma_curr->path[0] != '\0') {
            mmap_flags = MAP_FIXED|MAP_PRIVATE;
        } else {
            mmap_flags = MAP_UNINITIALIZED|MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE;
        }
        PSPRINTK("do_mmap()ing\n");
        err = do_mmap(NULL, 
                vma_curr->start, 
                vma_curr->end - vma_curr->start,
                PROT_READ|PROT_WRITE, 
                mmap_flags, 
                0);

        if(err > 0) {
            // mmap_region succeeded
            vma = find_vma(current->mm, vma_curr->start);
            if(vma) {
                if(vma_curr->path[0] != '\0') {
                    f = filp_open(vma_curr->path,
                            O_RDONLY | O_LARGEFILE,
                            0);
                    if(f) {
                        vma->vm_file = f;
                        vma->vm_pgoff = vma_curr->pgoff;
                        vma->vm_page_prot = vma_curr->prot;
                        vma->vm_flags = vma_curr->flags;
                        __vma_link_file(vma);
                    } else {
                        PSPRINTK("Failed to open vma file\n");
                    }
                }

                pte_curr = vma_curr->pte_list;

                while(pte_curr) {
                    PSPRINTK("Processing pte_curr\n");
                    remap_pfn_range(vma,
                                    pte_curr->vaddr,
                                    pte_curr->paddr >> PAGE_SHIFT,
                                    PAGE_SIZE,
                                    vma->vm_page_prot);

                    pte_curr = pte_curr->header.next;
                }
            }
        }
        vma_curr = vma_curr->header.next;
    }

    current->thread.usersp = clone_data->stack_ptr;
    current->thread.sp = clone_data->thread_sp;

    dump_task(current,regs);

    return 0;
}

/**
 * Notify of the fact that either a delegate or placeholder has died locally.  
 * In this case, the remote cpu housing its counterpart must be notified, so
 * that it can kill that counterpart.
 */
int process_server_task_exit_notification(pid_t pid) {

    exiting_process_t msg;
    int tx_ret = -1;
    struct task_struct* task;

    PSPRINTK("kmksrv: process_server_task_exit_notification - pid{%d}\n",pid);
    msg.header.msg_type = PROCESS_SERVER_MSG_PROCESS_EXITING;
    msg.header.msg_len = sizeof(exiting_process_t) - sizeof(msg_header_t);
    msg.my_pid = pid;

    if(current->pid == pid) {
        tx_ret = msg_tx(current->remote_cpu, &msg, sizeof(msg));
    } else {
        for_each_process(task) {
            if(task->pid == pid) {
                tx_ret = msg_tx(task->remote_cpu, &msg, sizeof(msg));
            }
        }
    }

    dump_task(current,NULL);


    return tx_ret;
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

    PSPRINTK("kmkprocsrv: notify_subprocess_starting: pid{%d}, remote_pid{%d}, remote_cpu{%d}\n",pid,remote_pid,remote_cpu);
    
    // Notify remote cpu of pairing between current task and remote
    // representative task.
    msg.header.msg_type = PROCESS_SERVER_MSG_PROCESS_PAIRING_REQUEST;
    msg.header.msg_len = sizeof(create_process_pairing_t) - sizeof(msg_header_t);
    msg.your_pid = remote_pid; 
    msg.my_pid = pid;
    
    tx_ret = msg_tx(remote_cpu, &msg, sizeof(msg));

    return tx_ret;

}

/**
 * Page walk has encountered a pte while deconstructing
 * the client side processes address space.  Transfer it.
 */
static int deconstruction_page_walk_pte_entry_callback(pte_t *pte, unsigned long start, unsigned long end, struct mm_walk *walk) {
    int* vma_id_ptr = (int*)walk->private;
    int vma_id = *vma_id_ptr;
    int dst_cpu = 3;
    pte_transfer_t* pte_xfer = NULL;

    if(NULL == pte || !pte_present(*pte)) {
        return 0;
    }

    pte_xfer = kmalloc(sizeof(pte_transfer_t),GFP_KERNEL);

    pte_xfer->header.msg_type = PROCESS_SERVER_PTE_TRANSFER;
    pte_xfer->header.msg_len = sizeof(pte_transfer_t) - sizeof(msg_header_t);
    pte_xfer->paddr = (pte_val(*pte) & PHYSICAL_PAGE_MASK) | (start & (PAGE_SIZE-1));
    // NOTE: Found the above pte to paddr conversion here -
    // http://wbsun.blogspot.com/2010/12/convert-userspace-virtual-address-to.html
    pte_xfer->vaddr = start;
    pte_xfer->vma_id = vma_id;
    pte_xfer->pfn = pte_pfn(*pte);
    msg_tx(dst_cpu, pte_xfer, sizeof(pte_transfer_t));

    kfree(pte_xfer);

    return 0;
}

/**
 * Request delegation to another cpu.
 */
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          struct task_struct* task) {

    clone_request_t* request = kmalloc(sizeof(clone_request_t),GFP_KERNEL);
    int tx_ret = -1;
    int dst_cpu = 3;
    char path[256] = {0};
    char* rpath = d_path(&task->active_mm->exe_file->f_path,
           path,256);
    char lpath[256];
    char *plpath;
    struct vm_area_struct* curr = NULL;
    struct mm_walk walk = {
        .pte_entry = deconstruction_page_walk_pte_entry_callback,
        .mm = current->mm,
        .private = NULL
        };
    vma_transfer_t* vma_xfer = kmalloc(sizeof(vma_transfer_t),GFP_KERNEL);
    int lclone_request_id;

    // Set destination cpu
    if(_scheduler_impl) {
        dst_cpu = _scheduler_impl();
    }

    // Execute locally if the scheduler decides to do so.
    if(dst_cpu == _cpu) {
        return PROCESS_SERVER_CLONE_FAIL;
    } 

    // Pick an id for this remote process request
    spin_lock(&_clone_request_id_lock);
    lclone_request_id = _clone_request_id++;
    spin_unlock(&_clone_request_id_lock);

    PSPRINTK("kmkprocsrv: process_server_clone invoked\n");
    PSPRINTK("kmkprocsrv: mount - %s\n",task->
            active_mm->
            exe_file->
            f_path.
            mnt->
            mnt_mountpoint->
            d_iname);
    PSPRINTK("kmkprocsrv: file - %s\n",task->
            active_mm->
            exe_file->
            f_path.
            dentry->
            d_iname);

    PSPRINTK("kmkprocsrv: path - %s\n",rpath);

    // VM Entries
    curr = task->mm->mmap;

    vma_xfer->header.msg_type = PROCESS_SERVER_VMA_TRANSFER;
    vma_xfer->header.msg_len = sizeof(vma_transfer_t) - sizeof(msg_header_t);
    while(curr) {

        /*
         * re-initialize path.
         */
        if(curr->vm_file == NULL) {
            vma_xfer->path[0] = '\0';
        } else {
            plpath = d_path(&curr->vm_file->f_path,
                        lpath,256);
            strcpy(vma_xfer->path,plpath);
        }

        /*
         * Transfer the vma
         */
        spin_lock(&_vma_id_lock);
        vma_xfer->vma_id = _vma_id++;
        spin_unlock(&_vma_id_lock);
        vma_xfer->start = curr->vm_start;
        vma_xfer->end = curr->vm_end;
        vma_xfer->prot = curr->vm_page_prot;
        vma_xfer->clone_request_id = lclone_request_id;
        vma_xfer->flags = curr->vm_flags;
        vma_xfer->pgoff = curr->vm_pgoff;
        tx_ret = msg_tx(dst_cpu, vma_xfer, sizeof(vma_transfer_t));
        if(tx_ret <= 0) {
            PSPRINTK("Unable to send vma transfer message\n");
        }

        PSPRINTK("Anonymous VM Entry: start{%lx}, end{%lx}, pgoff{%lx}\n",
                curr->vm_start, 
                curr->vm_end,
                curr->vm_pgoff);
        walk.private = &vma_xfer->vma_id;
        walk_page_range(curr->vm_start,curr->vm_end,&walk);
    
        curr = curr->vm_next;
    }

    // Build request
    request->header.msg_type = PROCESS_SERVER_MSG_CLONE_REQUEST;
    request->header.msg_len = sizeof( clone_request_t ) - sizeof( msg_header_t );
    request->clone_flags = clone_flags;
    request->stack_start = task->mm->start_stack;
    request->stack_ptr = stack_start;
    request->heap_start = task->mm->start_brk;
    request->heap_end = task->mm->brk;
    request->env_start = task->mm->env_start;
    request->env_end = task->mm->env_end;
    request->arg_start = task->mm->arg_start;
    request->arg_end = task->mm->arg_end;
    request->clone_request_id = lclone_request_id;
    memcpy( &request->regs, regs, sizeof(struct pt_regs) );
    request->stack_size = stack_size;
    strncpy( request->exe_path, rpath, 512 );
    request->placeholder_pid = task->pid;
    request->thread_fs = task->thread.fs;
    request->thread_gs = task->thread.gs;
    request->thread_sp0 = task->thread.sp0;
    request->thread_sp = task->thread.sp;
    request->thread_usersp = task->thread.usersp;
    request->thread_es = task->thread.es;
    request->thread_ds = task->thread.ds;
    request->thread_fsindex = task->thread.fsindex;
    request->thread_gsindex = task->thread.gsindex;

    // Send request
    tx_ret = msg_tx(dst_cpu, request, sizeof(clone_request_t));

    PSPRINTK("kmkprocsrv: transmitted %d\n",tx_ret);

    kfree(request);
    kfree(vma_xfer);

    dump_task(task,regs);

    return PROCESS_SERVER_CLONE_SUCCESS;
}


/**
 *
 */
int process_server_register_scheduler(scheduler_fn_t scheduler_impl) {
    _scheduler_impl = scheduler_impl;
}

/**
 * Kthread implementation
 */

/**
 * process_server
 * The kthread process loop.
 */
static int process_server(void* dummy) {

    int rcved = -1;
    int i = 0;
    unsigned long timeout = 1;  // Process loop wait duration

    // Initialize knowledge of local environment

    PSPRINTK("kmkprocsrv: Getting processor ID\r\n");

    _cpu = smp_processor_id();

    PSPRINTK("kmkprocsrv: Processor ID: %d\r\n",_cpu);
    PSPRINTK("kmkprocsrv: Number of cpus detected: %d\r\n",NR_CPUS);

    // Retrieve the communications buffers pointer from
    // the mcomm module.
    _comm_buf = matrix_get_buffers();
    if(NULL == _comm_buf) goto error_init_buffers;

    PSPRINTK("kmkprocsrv: Initialized local process server\r\n");

    _initialized = 1;

    while (1) {

        // Check for work to do.
        // Look for input from all CPUs
        for(i = 0; i < NR_CPUS; i++) {
            
            // Don't read from self.
            if( i == _cpu) continue;

            rcved = matrix_recv_from( _comm_buf, i, _rcv_buf, RCV_BUF_SZ);

            // If input was found on this CPU, handle it.
            if (rcved > 0) {

                PSPRINTK( "kmkprocsrv: Received data from cpu{%d}, len{%d}\n", i, rcved );
                PSPRINTK( _rcv_buf );

                comms_handler(i, _rcv_buf, rcved);

            }
        }

        // Sleep a while
		while ( schedule_timeout_interruptible( timeout*HZ / 0x10 ) );

    }


    // Should not reach

error_init_buffers:

    PSPRINTK("pmkprocsrv: Error, exiting\r\n");

    return 0;
}

/**
 * process_server_init
 * Start the process loop in a new kthread.
 */
static int __init process_server_init(void) {

    process_server_task = kthread_run(process_server, NULL, "kmkprocsrv"); 

    return 0;
}

/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(process_server_init);




