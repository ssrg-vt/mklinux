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
#include <linux/pcn_kmsg.h> // Messaging

#include <asm/tlbflush.h>
#include <asm/cacheflush.h>
#include <asm/uaccess.h> // USER_DS
#include <asm/prctl.h> // prctl
#include <asm/proto.h> // do_arch_prctl
#include <asm/msr.h> // wrmsr_safe

/**
 * Use the preprocessor to turn off printk.
 */
#define PROCESS_SERVER_VERBOSE 0
#if PROCESS_SERVER_VERBOSE
#define PSPRINTK(...) printk(__VA_ARGS__)
#else
#define PSPRINTK(...) ;
#endif

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
    vma_data_t* vma_list;
    vma_data_t* pending_vma_list;
} clone_data_t;

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
typedef struct _exiting_process {
    struct pcn_kmsg_hdr header;
    int my_pid; // PID of process on cpu transmitting this exit notification.
} exiting_process_t;

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
typedef struct _deconstruction_data {
    int clone_request_id;
    int vma_id;
    int dst_cpu;
} deconstruction_data_t;

typedef struct {
    struct work_struct work;
    clone_data_t* clone_data;
} clone_exec_work_t;

typedef struct {
    struct work_struct work;
    pid_t pid;
} exit_work_t;

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
 * Module variables
 */
static struct task_struct* process_server_task = NULL;     // Remember the kthread task_struct
static int _vma_id = 0;
static int _clone_request_id = 0;
static int _cpu = -1;
data_header_t* _data_head = NULL;        // General purpose data store
DEFINE_SPINLOCK(_data_head_lock);        // Lock for _data_head
DEFINE_SPINLOCK(_vma_id_lock);           // Lock for _vma_id
DEFINE_SPINLOCK(_clone_request_id_lock); // Lock for _clone_request_id

// Exec list
static struct workqueue_struct *clone_wq;
static struct workqueue_struct *exit_wq;

/**
 * General helper functions and debugging tools
 */
clone_data_t* get_current_clone_data() {
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
    dump_mm(task->mm);
    dump_stk(&task->thread,stack_ptr);
    PSPRINTK("TASK DUMP COMPLETE\n");
#endif
}

/**
 *
 */
static void dump_stk(struct thread_struct* thread, unsigned long stack_ptr) {
    int i;
    if(!thread) return;
    PSPRINTK("DUMP STACK\n");
    if(thread->sp) {
        PSPRINTK("sp = %lx\n",thread->sp);
        for(i = 0; i <= 8; i++) {
            PSPRINTK("stack peak %lx at %lx\n",*(unsigned long*)(thread->sp + i*8), thread->sp + i*8); 
        }
    }
    if(thread->usersp) {
        PSPRINTK("usersp = %lx\n",thread->usersp);
        for(i = 0; i <= 8; i++) {
            PSPRINTK("stack peak %lx at %lx\n",*(unsigned long*)(thread->usersp + i*8), thread->usersp + i*8);
        }
    }
    if(stack_ptr) {
        PSPRINTK("stack_ptr = %lx\n",stack_ptr);
        for(i = 0; i <= 8; i++) {
            PSPRINTK("stack peak %lx at %lx\n",*(unsigned long*)(stack_ptr + i*8), stack_ptr + i*8);
        }
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
    if(thread->fs) PSPRINTK("fs{%lx} - %lx\n",thread->fs,*((unsigned long*)thread->fs));
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
static clone_data_t* find_clone_data(int cpu, int clone_request_id) {
    data_header_t* curr = NULL;
    clone_data_t* clone = NULL;
    clone_data_t* ret = NULL;
    spin_lock(&_data_head_lock);
    
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

    spin_unlock(&_data_head_lock);

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

    down_read(&mm->mmap_sem);

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

    up_read(&mm->mmap_sem);
}

/**
 * Data library
 */


/**
 * Add data entry
 */
static void add_data_entry(void* entry) {
    data_header_t* hdr = (data_header_t*)entry;
    data_header_t* curr = NULL;

    if(!entry) {
        return;
    }

    spin_lock(&_data_head_lock);
    
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

    spin_unlock(&_data_head_lock);
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
static void dump_data_list() {
    data_header_t* curr = NULL;
    pte_data_t* pte_data = NULL;
    vma_data_t* vma_data = NULL;
    clone_data_t* clone_data = NULL;

    spin_lock(&_data_head_lock);

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

    spin_unlock(&_data_head_lock);
}



/*
 * Work exec
 */

/**
 *
 */
void process_exit_item(struct work_struct* work) {
    exit_work_t* w = (exit_work_t*) work;
    pid_t pid = w->pid;
    struct pid* spid;
    struct task_struct* task;
    
    PSPRINTK("%s: process to kill %ld SEARCHING\n", __func__, (long)pid);
    
    for_each_process(task) {
        if(task->pid == pid) {
            PSPRINTK("%s: for_each_process Found task to kill, killing\n", __func__);
            spid = task_pid(task);
            __set_task_state(task,TASK_INTERRUPTIBLE);
            kill_pid(spid,SIGKILL,1);
            goto happy_end;
        }
    }
  
    task = (pid) ? find_task_by_vpid(pid) : current;
    if (task) {
            PSPRINTK("%s: find_process_by_pid Found task to kill, killing\n", __func__);
            spid = task_pid(task);
            __set_task_state(task,TASK_INTERRUPTIBLE);
            kill_pid(spid,SIGKILL,1);
    }
    else
	    PSPRINTK("%s: process to kill %ld NOT FOUND\n", __func__, pid);

happy_end:
    kfree(work);
}

unsigned long long perf_aa, perf_bb, perf_cc, perf_dd, perf_ee;
/**
 *
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
perf_aa = native_read_tsc();
    sub_info = call_usermodehelper_setup( c->exe_path /*argv[0]*/, 
            argv, envp, 
            GFP_KERNEL );

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
}



/**
 * Request implementations
 */

/**
 *
 */
static int handle_pte_transfer(struct pcn_kmsg_message* inc_msg) {
    pte_transfer_t* msg = (pte_transfer_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    data_header_t* curr = NULL;
    vma_data_t* vma = NULL;
    pte_data_t* pte_data = kmalloc(sizeof(pte_data_t),GFP_ATOMIC);
    if(!pte_data) {
        PSPRINTK("Failed to allocate pte_data_t\n");
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
    spin_lock(&_data_head_lock);

    curr = _data_head;
    while(curr) {
        if(curr->data_type == PROCESS_SERVER_VMA_DATA_TYPE) {
            vma = (vma_data_t*)curr;
            if(vma->cpu == pte_data->cpu &&
               vma->vma_id == pte_data->vma_id &&
               vma->clone_request_id == pte_data->clone_request_id) {
                // Add to vma data
                spin_lock(&vma->lock);
                if(vma->pte_list) {
                    pte_data->header.next = (data_header_t*)vma->pte_list;
                    vma->pte_list->header.prev = (data_header_t*)pte_data;
                    vma->pte_list = pte_data;
                } else {
                    vma->pte_list = pte_data;
                }
                PSPRINTK("PTE added to vma\n");
                spin_unlock(&vma->lock);
                break;
            }
        }
        curr = curr->next;
    }

    spin_unlock(&_data_head_lock);

    pcn_kmsg_free_msg(inc_msg);
    
    return 0;
}

/**
 *
 */
static int handle_vma_transfer(struct pcn_kmsg_message* inc_msg) {
    vma_transfer_t* msg = (vma_transfer_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    vma_data_t* vma_data = kmalloc(sizeof(vma_data_t),GFP_ATOMIC);

    PSPRINTK("handle_vma_transfer %d\n",msg->vma_id);
    
    if(!vma_data) {
        PSPRINTK("Failed to allocate vma_data_t\n");
        return 0;
    }

    vma_data->header.data_type = PROCESS_SERVER_VMA_DATA_TYPE;
    vma_data->header.next = NULL;
    vma_data->header.prev = NULL;

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

    return 0;
}

/**
 * Handler function for when either a remote placeholder or a remote delegate process dies,
 * and its local counterpart must be killed to reflect that.
 */
static int handle_exiting_process_notification(struct pcn_kmsg_message* inc_msg) {
    exiting_process_t* msg = (exiting_process_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    struct task_struct* task;
    exit_work_t* exit_work;

    PSPRINTK("%s: cpu: %d msg: (pid: %d from_cpu: %d [%d])\n", 
	   __func__, smp_processor_id(), msg->my_pid,  inc_msg->hdr.from_cpu, source_cpu);
    
    for_each_process(task) {
        if(task->next_pid == msg->my_pid &&
           task->next_cpu == source_cpu) {

            PSPRINTK("kmkprocsrv: killing local task pid{%d}\n",task->pid);
            
            exit_work = kmalloc(sizeof(exit_work_t),GFP_ATOMIC);
            if(exit_work) {
                INIT_WORK( (struct work_struct*)exit_work, process_exit_item);
                exit_work->pid = task->pid;
                queue_work(exit_wq, (struct work_struct*)exit_work);
            }

            break; // No need to continue;
        }
    }

    pcn_kmsg_free_msg(inc_msg);

    return 0;
}

/**
 * Handler function for when another processor informs the current cpu
 * of a pid pairing.
 */
static int handle_process_pairing_request(struct pcn_kmsg_message* inc_msg) {
    create_process_pairing_t* msg = (create_process_pairing_t*)inc_msg;
    unsigned int source_cpu = msg->header.from_cpu;
    struct task_struct* task;
    if(msg == NULL) {
        return 0;
    }

    /*
     * Go through all the processes looking for the one with the right pid.
     * Once that task is found, do the bookkeeping necessary to remember
     * the remote cpu and pid information.
     */
    for_each_process(task) {
        if(task->pid == msg->your_pid && task->represents_remote) {
            task->next_cpu = source_cpu;
            task->next_pid = msg->my_pid;
            task->executing_for_remote = 0;
 
            PSPRINTK("kmkprocsrv: Added paring at request remote_pid{%d}, local_pid{%d}, remote_cpu{%d}",
                    task->next_pid,
                    task->pid,
                    task->next_cpu);

            break; // No need to continue;
        }
    }

    pcn_kmsg_free_msg(inc_msg);

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
    
    perf_cc = native_read_tsc();
    
    /*
     * Remember this request
     */
    clone_data = kmalloc(sizeof(clone_data_t),GFP_ATOMIC);
    clone_data->header.data_type = PROCESS_SERVER_CLONE_DATA_TYPE;
    clone_data->header.next = NULL;
    clone_data->header.prev = NULL;

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
    clone_data->lock = __SPIN_LOCK_UNLOCKED(&clone_data->lock);

    /*
     * Pull in vma data
     */
    spin_lock(&_data_head_lock);

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
                spin_lock(&clone_data->lock);
                if(clone_data->vma_list) {
                    clone_data->vma_list->header.prev = (data_header_t*)vma;
                    vma->header.next = (data_header_t*)clone_data->vma_list;
                } 
                clone_data->vma_list = vma;
                spin_unlock(&clone_data->lock);
            }
        }

        curr = next;
    }

    spin_unlock(&_data_head_lock);

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
    
    return 0;
}

/**
 * Message passing helper functions
 */

// TODO
static bool __user_addr (unsigned long x ) 
{
    return (x < PAGE_OFFSET);   
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

    perf_a = native_read_tsc();
    
    PSPRINTK("import address space\n");
    
    // Verify that we're a delegated task.
    if (!current->executing_for_remote) {
        PSPRINTK("ERROR - not executing for remote\n");
        return -1;
    }

    clone_data = find_clone_data(current->prev_cpu,current->clone_request_id);
    if(!clone_data) {
        return -1;
    }
perf_b = native_read_tsc();
    // Gut existing mappings
    
    down_write(&current->mm->mmap_sem);

    vma = current->mm->mmap;
    while(vma) {
        munmap_ret = do_munmap(current->mm, vma->vm_start, vma->vm_end - vma->vm_start);
        vma = current->mm->mmap;
    }

    // Clean out cache and tlb
    flush_tlb_mm(current->mm);
    flush_cache_mm(current->mm);

    up_write(&current->mm->mmap_sem);
    
    perf_c = native_read_tsc();    
    
    // import exe_file
    f = filp_open(clone_data->exe_path,O_RDONLY | O_LARGEFILE, 0);
    if(f) {
        get_file(f);
        current->mm->exe_file = f;
        filp_close(f,NULL);
    }
    
    // Import address space
    vma_curr = clone_data->vma_list;


    while(vma_curr) {
        PSPRINTK("do_mmap()\n");
        if(vma_curr->path[0] != '\0') {
            mmap_flags = /*MAP_UNINITIALIZED|*/MAP_FIXED|MAP_PRIVATE;
            f = filp_open(vma_curr->path,
                            O_RDONLY | O_LARGEFILE,
                            0);
            if(f) {
                down_write(&current->mm->mmap_sem);
                vma_curr->mmapping_in_progress = 1;
                err = do_mmap(f, 
                        vma_curr->start, 
                        vma_curr->end - vma_curr->start,
                        PROT_READ|PROT_WRITE|PROT_EXEC, 
                        mmap_flags, 
                        0);
                vmas_installed++;
                vma_curr->mmapping_in_progress = 0;
                up_write(&current->mm->mmap_sem);
                filp_close(f,NULL);
                if(err != vma_curr->start) {
                    PSPRINTK("Fault - do_mmap failed to map %lx with error %lx\n",
                            vma_curr->start,err);
                }
            }
        } else {
            mmap_flags = MAP_UNINITIALIZED|MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE;
            down_write(&current->mm->mmap_sem);
            err = do_mmap(NULL, 
                vma_curr->start, 
                vma_curr->end - vma_curr->start,
                PROT_READ|PROT_WRITE|PROT_EXEC, 
                mmap_flags, 
                0);
            vmas_installed++;
            //PSPRINTK("mmap error for %lx = %lx\n",vma_curr->start,err);
            up_write(&current->mm->mmap_sem);
            if(err != vma_curr->start) {
                PSPRINTK("Fault - do_mmap failed to map %lx with error %lx\n",
                        vma_curr->start,err);
            }
        }
       
        if(err > 0) {
            // mmap_region succeeded
            vma = find_vma(current->mm, vma_curr->start);
            PSPRINTK("vma mmapped, pulling in pte's\n");
            if(vma) {
                pte_curr = vma_curr->pte_list;
                if(pte_curr == NULL) {
                    PSPRINTK("vma->pte_curr == null\n");
                }
                while(pte_curr) {
                    // MAP it
                    err = remap_pfn_range(vma,
                            pte_curr->vaddr,
                            pte_curr->paddr >> PAGE_SHIFT,
                            PAGE_SIZE,
                            vma->vm_page_prot);
                    ptes_installed++;
                    pte_curr = (pte_data_t*)pte_curr->header.next;
                    if(err) {
                        PSPRINTK("Fault - remap_pfn_range failed to map %lx to %lx with error %lx\n",
                                pte_curr->paddr,pte_curr->vaddr,err);
                    }
                }
            }
        }
        vma_curr = (vma_data_t*)vma_curr->header.next;
    }

    printk("vmas_installed{%d}, ptes_installed{%d}\n",vmas_installed,ptes_installed);

    perf_d = native_read_tsc();

    // install memory information
    current->mm->start_stack = clone_data->stack_start;
    current->mm->start_brk = clone_data->heap_start;
    current->mm->brk = clone_data->heap_end;
    current->mm->env_start = clone_data->env_start;
    current->mm->env_end = clone_data->env_end;
    current->mm->arg_start = clone_data->arg_start;
    current->mm->arg_end = clone_data->arg_end;

    // install thread information
    // TODO: Move to arch
    current->thread.es = clone_data->thread_es;
    current->thread.ds = clone_data->thread_ds;
    current->thread.usersp = clone_data->thread_usersp;//clone_data->stack_ptr;
    

    // Set output variables.
    *sp = clone_data->thread_usersp;//clone_data->thread_sp;//clone_data->stack_ptr;
    *ip = clone_data->regs.ip;
    
    // adjust registers as necessary
    memcpy(regs,&clone_data->regs,sizeof(struct pt_regs)); 
    regs->ax = 0; // Fake success for the "sched_setaffinity" syscall
                  // that this process just "returned from"

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
    
    //dump_clone_data(clone_data);
    //dump_task(current,regs, clone_data->stack_ptr);
    perf_e = native_read_tsc();
    printk("%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
       __func__,
       perf_aa, perf_bb, perf_cc, perf_dd, perf_ee,
       perf_a, perf_b, perf_c, perf_d, perf_e);
   
    // Save off clone data
    current->clone_data = clone_data;

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
    clone_data_t* clone_data = find_clone_data(current->prev_cpu, current->clone_request_id);

    PSPRINTK("kmksrv: process_server_task_exit_notification - pid{%d}\n",pid);
    msg.header.type = PCN_KMSG_TYPE_PROC_SRV_EXIT_PROCESS;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    msg.my_pid = pid;

    if(current->pid == pid) {
        tx_ret = pcn_kmsg_send_long(current->prev_cpu, 
                    (struct pcn_kmsg_long_message*)&msg, 
                    sizeof(msg) - sizeof(msg.header));
    } else {
        for_each_process(task) {
            if(task->pid == pid) {
                tx_ret = pcn_kmsg_send_long(task->prev_cpu, 
                            (struct pcn_kmsg_long_message*)&msg, 
                            sizeof(msg) - sizeof(msg.header));
            }
        }
    }

    //dump_task(current,NULL,0);

    if(clone_data) {
        spin_lock(&_data_head_lock);
        remove_data_entry(clone_data);
        spin_unlock(&_data_head_lock);
        destroy_clone_data(clone_data);
    }

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
    msg.header.type = PCN_KMSG_TYPE_PROC_SRV_CREATE_PROCESS_PAIRING;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    msg.your_pid = remote_pid; 
    msg.my_pid = pid;
    
    tx_ret = pcn_kmsg_send_long(remote_cpu, 
                (struct pcn_kmsg_long_message*)&msg, 
                sizeof(msg) - sizeof(msg.header));

    return tx_ret;

}

/*
 * Thread group setup/teardown
 */
int join_thread_group_mcast() {

}

int quit_thread_group_mcast() {

}

/*
 * VMA Hook
 */

/**
 * 0 = not handled
 * 1 = handled
 */
int process_server_try_handle_mm_fault_no_vma(struct mm_struct *mm, unsigned long address,
                                              unsigned int flags) {
    clone_data_t* clone_data = NULL;
    vma_data_t* vma_data = NULL;
    if((!current->executing_for_remote) && (!current->represents_remote)) {
        goto not_handled; // Don't care
    }

    clone_data = get_current_clone_data();

    if(!clone_data) goto not_handled;

    // Check to see if we are possibly sharing VM with anybody.
    // If not, exit.
    if(!(clone_data->clone_flags & CLONE_VM)) {
        goto not_handled;
    }

    printk("%s: to be handled\n",__func__);
    return 0;

not_handled:
    return 0;
}

/**
 * 0 = not handled
 * 1 = handled
 */
int process_server_try_handle_mm_fault_vma(struct mm_struct *mm, struct vm_area_struct *vma,
                                       unsigned long address, unsigned int flags) {
    clone_data_t* clone_data = NULL;
    vma_data_t* vma_data = NULL;
    if((!current->executing_for_remote) && (!current->represents_remote)) {
        goto not_handled; // Don't care
    }

    clone_data = get_current_clone_data();

    if(!clone_data) goto not_handled;

    // Check to see if we are possibly sharing VM with anybody.
    // If not, exit.
    if(!(clone_data->clone_flags & CLONE_VM)) {
        goto not_handled;
    }

    printk("%s: to be handled\n",__func__);
    return 0;

not_handled:
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
    pcn_kmsg_send(dst_cpu, (struct pcn_kmsg_message *)&pte_xfer);

    return 0;
}

/**
 * Migrate the specified task <task> to cpu <cpu>
 * Currently, this function will put the specified task to 
 * sleep, and push its info over to the remote cpu.  The
 * remote cpu will then create a new process and import that
 * info into its new context.  
 *
 * TODO: Use a multicast channel for communicating data relevant
 *       to shadow processes.
 */
int process_server_do_migration(struct task_struct* task, int cpu) {
    struct pt_regs *regs = task_pt_regs(task);
    // TODO: THIS IS WRONG, task flags is not what I want here.
    unsigned long clone_flags = task->clone_flags;
    unsigned long stack_start = task->mm->start_stack;
    clone_request_t* request = kmalloc(sizeof(clone_request_t),GFP_KERNEL);
    int tx_ret = -1;
    int dst_cpu = cpu;
    char path[256] = {0};
    char* rpath = d_path(&task->mm->exe_file->f_path,
           path,256);
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

    PSPRINTK("process_server_do_migration\n");
    dump_regs(regs);

    // Execute locally if the scheduler decides to do so.
    if(dst_cpu == _cpu) {
        return PROCESS_SERVER_CLONE_FAIL;
    }

    // This will be a placeholder process for the remote
    // process that is subsequently going to be started.
    //  Block its execution.
    sigaddset(&task->pending.signal,SIGSTOP); 
    set_tsk_thread_flag(task,TIF_SIGPENDING); 
    __set_task_state(task,TASK_UNINTERRUPTIBLE);

    // Book keeping for placeholder process.
    task->represents_remote = 1;
    task->executing_for_remote = 0;

    // Pick an id for this remote process request
    spin_lock(&_clone_request_id_lock);
    lclone_request_id = _clone_request_id++;
    spin_unlock(&_clone_request_id_lock);

    down_read(&task->mm->mmap_sem);
    
    
    
    // VM Entries
    curr = task->mm->mmap;

    vma_xfer->header.type = PCN_KMSG_TYPE_PROC_SRV_VMA_TRANSFER;
    vma_xfer->header.prio = PCN_KMSG_PRIO_NORMAL;
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
        tx_ret = pcn_kmsg_send_long(dst_cpu, 
                    (struct pcn_kmsg_long_message*)vma_xfer, 
                    sizeof(vma_transfer_t) - sizeof(vma_xfer->header));
        if (tx_ret) {
            PSPRINTK("Unable to send vma transfer message, rc = %d\n", tx_ret);
        }

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

    up_read(&task->mm->mmap_sem);

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
// struct task_struct ---------------------------------------------------------    
    request->stack_ptr = stack_start;
    request->placeholder_pid = task->pid;
    request->placeholder_tgid = task->tgid;
// struct thread_struct -------------------------------------------------------
    // have a look at: copy_thread() arch/x86/kernel/process_64.c 
    // have a look at: struct thread_struct arch/x86/include/asm/processor.h
    {
      	unsigned long fs, gs, shadowgs;
	unsigned int fsindex, gsindex;
	unsigned int ds, cs, es;
	
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
    tx_ret = pcn_kmsg_send_long(dst_cpu, 
                (struct pcn_kmsg_long_message*)request, 
                sizeof(clone_request_t) - sizeof(request->header));

    PSPRINTK("kmkprocsrv: transmitted %d\n",tx_ret);

    kfree(request);
    kfree(vma_xfer);

    //dump_task(task,regs,request->stack_ptr);

    return PROCESS_SERVER_CLONE_SUCCESS;

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
     * Create a work queue so that we can do bottom side
     * processing on data that was brought in by the
     * communications module interrupt handlers.
     */
    clone_wq = create_workqueue("clone_wq");
    exit_wq  = create_workqueue("exit_wq");

    /*
     * Register to receive relevant incomming messages.
     */

    PSPRINTK("BEN: Registering process server callbacks!\n");

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

    return 0;
}

/**
 * Register process server init function as
 * module initialization function.
 */
late_initcall(process_server_init);




