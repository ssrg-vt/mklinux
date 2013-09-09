/*******************************************************
 *
 * Popcorn performance measurement framework
 * DKatz
 *
 */

#include <linux/kernel.h>
#include <linux/pcn_perf.h>
//#include <linux/pcn_kmsg.h>
#include <linux/syscalls.h>
#include <linux/slab.h>


/**
 *
 */
struct _perf_start_message {
    struct pcn_kmsg_hdr header;
    char pad[60];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _perf_start_message perf_start_message_t;

/**
 *
 */
struct _perf_stop_message {
    struct pcn_kmsg_hdr header;
    char pad[60];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _perf_stop_message perf_stop_message_t;

/**
 *
 */
struct _perf_end_ack_message {
    struct pcn_kmsg_hdr header;
    char pad[60];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _perf_end_ack_message perf_end_ack_message_t;

/**
 *
 */
struct _perf_context_message {
    struct pcn_kmsg_hdr header;
    char name[512];
    int context_id;
    int home_cpu;
};
typedef struct _perf_context_message perf_context_message_t;

/**
 *
 */
struct _perf_entry_message {
    struct pcn_kmsg_hdr header;
    unsigned long long start;       
    unsigned long long end;         
    int in_progress;
    int context_id;
    char note[512];
} __attribute__((packed)) __attribute__((aligned(64)));
typedef struct _perf_entry_message perf_entry_message_t;

typedef struct _perf_end_response_data {
    int expected_responses;
    int responses;
} perf_end_response_data_t;

pcn_perf_context_t* _context_head;
int _cpu = -1;
int _context_id;
DEFINE_SPINLOCK(_context_id_lock);
DEFINE_SPINLOCK(_do_popcorn_perf_end_lock);
perf_end_response_data_t response_data;

/**
 *PCN_KMSG_TYPE_PCN_PERF_CONTEXT_MESSAGE,
     PCN_KMSG_TYPE_PCN_PERF_ENTRY_MESSAGE,
 */
static void send_context(pcn_perf_context_t* ctxt, int cpu) {
    perf_context_message_t cxt_message;
    perf_entry_message_t entry_message;
    pcn_perf_entry_t *curr;

    cxt_message.header.type = PCN_KMSG_TYPE_PCN_PERF_CONTEXT_MESSAGE;
    cxt_message.header.prio = PCN_KMSG_PRIO_NORMAL;
    entry_message.header.type = PCN_KMSG_TYPE_PCN_PERF_ENTRY_MESSAGE;
    entry_message.header.prio = PCN_KMSG_PRIO_NORMAL;

    // first send the context
    cxt_message.context_id = ctxt->context_id;
    cxt_message.home_cpu = _cpu;
    strcpy(cxt_message.name,ctxt->name);
    pcn_kmsg_send_long(cpu,
                        (struct pcn_kmsg_long_message*)(&cxt_message),
                        sizeof(perf_context_message_t) - sizeof(struct pcn_kmsg_hdr));

    // then send the entries
    curr = ctxt->entry_list;
    while(curr) {

        if(curr->start == 0 || curr->end == 0) continue; // skip unfinished entries

        entry_message.start = curr->start;
        entry_message.end   = curr->end;
        entry_message.in_progress = curr->in_progress;
        entry_message.context_id = curr->context_id;
        strcpy(entry_message.note,curr->note);
        pcn_kmsg_send_long(cpu,(struct pcn_kmsg_message*)(&entry_message),
                            sizeof(perf_entry_message_t) - sizeof(struct pcn_kmsg_hdr)); 

        curr = curr->next;
    }
    

}

/**
 *
 */
static int send_end_message(void) {
    perf_stop_message_t msg;
    int i;
    int s;
    int ret = 0;
    msg.header.type = PCN_KMSG_TYPE_PCN_PERF_END_MESSAGE;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    for(i = 0; i < NR_CPUS; i++) {
        if(i == _cpu) continue; // Don't bother notifying myself... I already know.
        s = pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&msg));
        if(!s) {
            ret ++;
        }
    }
    return ret;
}

/**
 *
 */
static void send_start_message(void) {
    perf_stop_message_t msg;
    int i;
    msg.header.type = PCN_KMSG_TYPE_PCN_PERF_START_MESSAGE;
    msg.header.prio = PCN_KMSG_PRIO_NORMAL;
    for(i = 0; i < NR_CPUS; i++) {
        if(i == _cpu) continue; // Don't bother notifying myself... I already know.
        pcn_kmsg_send(i,(struct pcn_kmsg_message*)(&msg));                                                                                       
    }
}

/**
 *
 */
static void unlink_context( pcn_perf_context_t* cxt ) {
    if(!cxt) {
        return;
    }

    if(_context_head == cxt) {
        _context_head = cxt->next;
    }

    if(cxt->next) {
        cxt->next->prev = cxt->prev;
    }

    if(cxt->prev) {
        cxt->prev->next = cxt->next;
    }

    cxt->prev = NULL;
    cxt->next = NULL;
}

/**
 *
 */
static void link_context( pcn_perf_context_t* cxt ) {
    pcn_perf_context_t* curr = NULL;
    cxt->next = NULL;
    cxt->prev = NULL;
    if(!_context_head) {
        _context_head = cxt;
    } else {
        curr = _context_head;
        while(curr->next != NULL) {
            if(curr == cxt) {
                return; // it's already in the list.
            }
            curr = curr->next;
        }
        curr->next = cxt;
        cxt->next = NULL;
        cxt->prev = curr;
    }
}

/**
 * Go through all measurement contexts, and clear
 * their data out.
 */
static void clear_data(void) {
    pcn_perf_context_t* curr = _context_head;
    pcn_perf_entry_t* entry = NULL;
    pcn_perf_entry_t* next_entry = NULL;
    while(curr) {
        entry = curr->entry_list;
        while(entry) {
            next_entry = entry->next;
            kfree(entry);
            entry = next_entry;
        }
        curr->entry_list = NULL;
        curr = curr->next;
    }
}

/**
 * Clear data
 * Active all measurement contexts
 */
static void do_popcorn_perf_start_impl(void) {
    pcn_perf_context_t* curr = _context_head;
    clear_data();
    while(curr) {
        curr->is_active = 1;
        curr = curr->next;
    }
}

/**
 *
 */
void do_popcorn_perf_start(void) {
    send_start_message();
    do_popcorn_perf_start_impl();
}

/**
 *
 */
static void dump_data(void) {
    pcn_perf_context_t* curr = _context_head;
    pcn_perf_entry_t* entry;
    while(curr) {
        entry = curr->entry_list;
        while(entry) {
            printk("pcn_perf: %s;%llu;%d;%d;%d;%d;%s\n",
                    curr->name,
                    entry->end - entry->start,
                    curr->home_cpu,
                    current->pid,
                    current->tgroup_home_cpu,
                    current->tgroup_home_id,
                    entry->note);
            entry = entry->next;
        }
        curr = curr->next;
    }
}

/**
 * Deactivate all measurement contexts
 * Display data
 */
static void do_popcorn_perf_end_impl(void) {
    pcn_perf_context_t* curr = _context_head;
    while(curr) {
        curr->is_active = 0;
        curr = curr->next;
    }
    dump_data();
    //clear_data();
}
/**
 *
 */
void do_popcorn_perf_end(void) {
  
    // Only one process can do this at a time
    spin_lock(&_do_popcorn_perf_end_lock);

    // Reset response expectations
    response_data.expected_responses = 0;
    response_data.responses = 0;

    // Send notification of the end of the perf measurement cycle,
    // and record the number of cpus this was successfully sent to.
    // Every cpu that receives this message will 
    // 1) send all its perf data, then
    // 2) send an ack to conclude
    response_data.expected_responses = send_end_message();

    // wait on all responses to come in
    while(response_data.responses != response_data.expected_responses) {
        schedule();
    }

    do_popcorn_perf_end_impl();

    spin_unlock(&_do_popcorn_perf_end_lock);
}

/**
 *
 */
void perf_init_context( pcn_perf_context_t * cxt, char * name ) {
    
    if(strlen(name) >= sizeof(cxt->name)) {
        printk("%s - error, name too large: %s\n",__func__,name);
        return;
    }

    spin_lock(&_context_id_lock);
    cxt->context_id = _context_id++;
    spin_unlock(&_context_id_lock);
    cxt->home_cpu = _cpu;
    strcpy(cxt->name,name);
    cxt->entry_list = NULL;
    cxt->is_active = 0;
    cxt->next = NULL;
    link_context(cxt);

}

/**
 *
 */
void perf_measure_start(pcn_perf_context_t *cxt) {
    pcn_perf_entry_t* entry;
    if(!cxt->is_active) return;
    
    entry = kmalloc(sizeof(pcn_perf_entry_t),GFP_ATOMIC);
    entry->next = NULL;
    entry->context_id = cxt->context_id;
    entry->start = 0;
    entry->end = 0;
    if(entry) {
        // add to front of context's entry list
        if(cxt->entry_list) {
            cxt->entry_list->prev = entry;
            entry->next = cxt->entry_list;
            cxt->entry_list = entry;
        } else {
            cxt->entry_list = entry;
        }

        // now take measurement
        cxt->entry_list->in_progress = 1;
        cxt->entry_list->start = native_read_tsc();
    }
}

/**
 *
 */
void perf_measure_stop(pcn_perf_context_t *cxt, char* note) {
    unsigned long long time = native_read_tsc();

    if(cxt->entry_list && cxt->entry_list->in_progress) {
        cxt->entry_list->end = time;
        cxt->entry_list->in_progress = 0;
        strcpy(cxt->entry_list->note,note);
    }
}

/**
 *
 */
static int handle_start_message(struct pcn_kmsg_message* inc_msg) {
    do_popcorn_perf_start_impl();
   pcn_kmsg_free_msg(inc_msg); 
   return 0;
}

/**
 *
 */
static int handle_end_message(struct pcn_kmsg_message* inc_msg) {
    pcn_perf_context_t* curr = _context_head;
    perf_end_ack_message_t ack;
    int cpu = inc_msg->hdr.from_cpu;
    while(curr) {
        curr->is_active = 0;
        send_context(curr,cpu);
        curr = curr->next;
    }
    do_popcorn_perf_end_impl();
    clear_data();

    // We're done, now acknowlege the completion of this transfer
    // to the requesting cpu.
    ack.header.type = PCN_KMSG_TYPE_PCN_PERF_END_ACK_MESSAGE;
    ack.header.prio = PCN_KMSG_PRIO_NORMAL;
    pcn_kmsg_send(cpu,(struct pcn_kmsg_message*)(&ack));

    pcn_kmsg_free_msg(inc_msg);
    return 0;
}

/**
 *
 */
static int handle_end_ack_message(struct pcn_kmsg_message* inc_msg) {
   response_data.responses++;
   pcn_kmsg_free_msg(inc_msg);
   return 0;
}

static int handle_context_transfer(struct pcn_kmsg_message* inc_msg) {
    pcn_perf_context_t* cxt = kmalloc(sizeof(pcn_perf_context_t),GFP_ATOMIC);
    perf_context_message_t* msg = (perf_context_message_t*)inc_msg;

    if(cxt) {
        cxt->context_id = msg->context_id;
        cxt->home_cpu = inc_msg->hdr.from_cpu;
        strcpy(cxt->name,msg->name );
        cxt->entry_list = NULL;
        cxt->is_active = 0;
        cxt->next = NULL;
        link_context(cxt);
    }

    pcn_kmsg_free_msg(inc_msg);
    return 0;
}

static int handle_entry_transfer(struct pcn_kmsg_message* inc_msg) {
    pcn_perf_entry_t* entry = kmalloc(sizeof(pcn_perf_entry_t),GFP_ATOMIC);
    perf_entry_message_t* msg = (perf_entry_message_t*)inc_msg;
    pcn_perf_context_t* cxt = NULL;
    pcn_perf_context_t* curr = _context_head;

    while(curr) {
        if(curr->home_cpu == msg->header.from_cpu &&
           curr->context_id == msg->context_id) {
            cxt = curr;
            break;
        }
        curr = curr->next;
    }

    if(cxt && entry) {
        entry->next = NULL;
        entry->start = msg->start;
        entry->end = msg->end;
        strcpy(entry->note,msg->note);
        if(cxt->entry_list) {
            cxt->entry_list->prev = entry;
            entry->next = cxt->entry_list;
            cxt->entry_list = entry;
        } else {
             cxt->entry_list = entry;
        }
    }

    pcn_kmsg_free_msg(inc_msg);
    return 0;
}

/**
 *
 */
static int __init pcn_perf_init(void) {

    _cpu = smp_processor_id();
    _context_id = 0;

    // register messaging handlers
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PCN_PERF_START_MESSAGE,
                        handle_start_message);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PCN_PERF_END_MESSAGE,                
                        handle_end_message);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PCN_PERF_CONTEXT_MESSAGE,
                        handle_context_transfer);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PCN_PERF_ENTRY_MESSAGE,
                        handle_entry_transfer);
    pcn_kmsg_register_callback(PCN_KMSG_TYPE_PCN_PERF_END_ACK_MESSAGE,
                        handle_end_ack_message);
    return 0;
}

SYSCALL_DEFINE0(popcorn_perf_start) {
    do_popcorn_perf_start();
}

SYSCALL_DEFINE0(popcorn_perf_end) {
    do_popcorn_perf_end();
}

late_initcall(pcn_perf_init);
