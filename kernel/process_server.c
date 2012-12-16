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

#define RCV_BUF_SZ 0x4000

/**
 * Message passing data type definitions
 */
#define MAX_MSG_LEN sizeof(clone_request_t) // should always be the largest msg defined.
#define PROCESS_SERVER_MSG_CLONE_REQUEST 1

/**
 *
 */
typedef struct _msg_header {
    int msg_type;
    int msg_len; // not including header
    int msg_id;
} msg_header_t;

/**
 *
 */
typedef struct _clone_request {
    msg_header_t header;
    unsigned long clone_flags;
    unsigned long stack_start;
    struct pt_regs regs;
    unsigned long stack_size;
    int* parent_tidptr;
    int* child_tidptr;
    struct task_struct* task_ptr;
    char exe_path[512];
} clone_request_t;

/**
 * External prototypes
 */

/**
 * Internal prototypes
 */
static void handle_clone_request(clone_request_t* request);
static int msg_tx(int dst, void* data, int data_len);
static void comms_handler(int source, char* data, int data_len);
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,                                                                                                                   
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          int __user *parent_tidptr,
                          int __user *child_tidptr,
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

/**
 * Request implementation
 */


/**
 *
 */
static void handle_clone_request(clone_request_t* request) {
    printk("Handling clone request: %lu\n",request->clone_flags);
    struct task_struct* ts = request->task_ptr;
    printk("Received ts: pid{%d} mm{%lu}\n",ts->pid,ts->active_mm);

    struct subprocess_info* sub_info;
    char* argv[] = {request->exe_path,NULL,NULL};
    static char *envp[] = {
        "HOME=/",
        "TERM=linux",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL
    };

    sub_info = call_usermodehelper_setup( argv[0], argv, envp, GFP_ATOMIC );
    if (sub_info == NULL) return;

    call_usermodehelper_exec(sub_info, UMH_NO_WAIT);

}

/**
 * Message passing helper functions
 */

/**
 *
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
 *
 */
static void comms_handler(int source, char* data, int data_len) {
    msg_header_t* hdr = NULL;
    if (data == NULL) {
        goto error;
    }

    if (data_len <= 0) {
        goto error;
    }

    if (source < 0) {
        goto error;
    }

    // Grab the header to figure out the message type
    hdr = (msg_header_t*)data;
    printk("kmkprocsrv: Routing message to handler msg_type{%d}\n",hdr->msg_type);
    if (hdr->msg_type == PROCESS_SERVER_MSG_CLONE_REQUEST) {
        clone_request_t* request = (clone_request_t*)data;
        // Check message integrity.
        if(request->header.msg_len == sizeof(clone_request_t) - sizeof(msg_header_t)) {
            // Handle this message.
            handle_clone_request((clone_request_t*)data);
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
long process_server_clone(unsigned long clone_flags,
                          unsigned long stack_start,                                                                                                                   
                          struct pt_regs *regs,
                          unsigned long stack_size,
                          int __user *parent_tidptr,
                          int __user *child_tidptr,
                          struct task_struct* task) {

    clone_request_t request;
    int tx_ret = -1;

    printk("kmkprocsrv: process_server_clone invoked\n");
    printk("kmkprocsrv: mount - %s\n",task->
            active_mm->
            exe_file->
            f_path.
            mnt->
            mnt_mountpoint->
            d_iname);
    printk("kmkprocsrv: file - %s\n",task->
            active_mm->
            exe_file->
            f_path.
            dentry->
            d_iname);
    char path[512] = {0};
    char* rpath = d_path(&task->active_mm->exe_file->f_path,
           path,512);
    printk("kmkprocsrv: path - %s\n",rpath);


    // Build request
    request.header.msg_type = PROCESS_SERVER_MSG_CLONE_REQUEST;
    request.header.msg_len = sizeof(clone_request_t) - sizeof(msg_header_t);
    request.clone_flags = clone_flags;
    request.stack_start = stack_start;
    memcpy(&request.regs,regs,sizeof(struct pt_regs));
    request.stack_size = stack_size;
    request.parent_tidptr = parent_tidptr;
    request.child_tidptr = child_tidptr;
    request.task_ptr = task;
    strncpy(request.exe_path,rpath,512);

    // Send request
    if(_cpu == 0) {
        tx_ret = msg_tx(3, &request, sizeof(request));
    }

    printk("kmkprocsrv: transmitted %d\n",tx_ret);

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
    char rcv_buf[RCV_BUF_SZ];
    int i = 0;
    unsigned long timeout = 1;  // Process loop wait duration

    // Initialize knowledge of local environment
    printk("kmkprocsrv: Getting processor ID\r\n");
    _cpu = smp_processor_id();
    printk("kmkprocsrv: Processor ID: %d\r\n",_cpu);
    printk("kmkprocsrv: Number of cpus detected: %d\r\n",NR_CPUS);

    // Retrieve the communications buffers pointer from
    // the mcomm module.
    _comm_buf = matrix_get_buffers();
    if(NULL == _comm_buf) goto error_init_buffers;

    printk("kmkprocsrv: Initialized local process server\r\n");
    _initialized = 1;

    while (1) {

        // Check for work to do.
        // Look for input from all CPUs
        for(i = 0; i < NR_CPUS; i++) {
            
            // Hello!
            //printk("kmkprocsrv: Checking for messages\r\n");

            // Don't read from self.
            if( i == _cpu) continue;

            rcved = matrix_recv_from( _comm_buf, i, rcv_buf, RCV_BUF_SZ);

            // If input was found on this CPU, handle it.
            if (rcved > 0) {
                printk( "kmkprocsrv: Received data from cpu{%d}, len{%d}\r\n", i, rcved );
                printk( rcv_buf );
                comms_handler(i, rcv_buf, rcved);

            }
        }

        // Sleep a while
		while ( schedule_timeout_interruptible( timeout*HZ ) );

    }


    // Should not reach

error_init_buffers:

    printk("pmkprocsrv: Error, exiting\r\n");

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




