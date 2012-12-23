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
    unsigned long clone_flags;
    unsigned long stack_start;
    struct pt_regs regs;
    unsigned long stack_size;
    char exe_path[512];
    int placeholder_pid;
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


/**
 * Module variables
 */
static struct task_struct* process_server_task;     // Remember the kthread task_struct
static comm_buffers* _comm_buf = NULL;
static int _initialized = 0;
static int _msg_id = 0;
static int _cpu = -1;
//static DEFINE_SPINLOCK(msg_response_lock);
char rcv_buf[RCV_BUF_SZ];

/**
 * Request implementations
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

    struct subprocess_info* sub_info;
    char* argv[] = {request->exe_path,NULL,NULL};
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
    };

    PSPRINTK("Handling clone request: %lu\n",request->clone_flags);

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
    msg->msg_id = _msg_id++;

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

error:
    return;
}


/**
 *
 * Public API
 */

/**
 *
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


    return tx_ret;
}

/**
 * Create a pairing between a newly created delegate process and the
 * remote placeholder process.  This function creates the local
 * pairing first, then sends a message to the originating cpu
 * so that it can do the same.
 */
int process_server_notify_delegated_subprocess_starting(pid_t pid, pid_t remote_pid, int remote_cpu) {

    PSPRINTK("kmkprocsrv: notify_subprocess_starting: pid{%d}, remote_pid{%d}, remote_cpu{%d}\n",pid,remote_pid,remote_cpu);

    create_process_pairing_t msg;
    int tx_ret = -1;

    // Locally note pairing between current task and remote representative
    // task.
    if(current->pid == pid) {
        // should always be the case!
        current->remote_pid = remote_pid;
        current->remote_cpu = remote_cpu;
        current->executing_for_remote = 1;
        current->represents_remote = 0;
    } else {
        // TODO: Scan task list.  This should never occur though, since this function
        // is called only by the new kthread itself in kmod module.  For now, just
        // error out... notice this is always printed out, and is not turned off by 
        // the verbose switch.
        printk("ERROR: process_server_notify_delegated_subprocess_starting called by wrong task!\n");
        return -1;
    }

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
 * Request delegation to another cpu.
 */
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,                                                                                                                   
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          struct task_struct* task) {

    clone_request_t request;
    int tx_ret = -1;

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

    char path[512] = {0};
    char* rpath = d_path(&task->active_mm->exe_file->f_path,
           path,512);

    PSPRINTK("kmkprocsrv: path - %s\n",rpath);


    // Build request
    request.header.msg_type = PROCESS_SERVER_MSG_CLONE_REQUEST;
    request.header.msg_len = sizeof(clone_request_t) - sizeof(msg_header_t);
    request.clone_flags = clone_flags;
    request.stack_start = stack_start;
    memcpy(&request.regs,regs,sizeof(struct pt_regs));
    request.stack_size = stack_size;
    strncpy(request.exe_path,rpath,512);
    request.placeholder_pid = task->pid;

    // Send request
    // TODO: figure out who to send this to.  Currently, only cpu 0
    // will delegate, and it will only ever delegate to cpu 3.  In
    // the future, there should be some mechanism for determining which
    // cpu to ask to do this work.
    if(_cpu == 0) {
        tx_ret = msg_tx(3, &request, sizeof(request));
    }

    PSPRINTK("kmkprocsrv: transmitted %d\n",tx_ret);

    return 0;
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

            rcved = matrix_recv_from( _comm_buf, i, rcv_buf, RCV_BUF_SZ);

            // If input was found on this CPU, handle it.
            if (rcved > 0) {

                PSPRINTK( "kmkprocsrv: Received data from cpu{%d}, len{%d}\r\n", i, rcved );
                PSPRINTK( rcv_buf );
                comms_handler(i, rcv_buf, rcved);

            }
        }

        // Sleep a while
		while ( schedule_timeout_interruptible( timeout*HZ ) );

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




