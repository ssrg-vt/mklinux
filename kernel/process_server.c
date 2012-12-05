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
#include <linux/threads.h> // NR_CPUS
#include <linux/process_server.h>

#define RCV_BUF_SZ 1024

/**
 * Module variables
 */
static struct task_struct* process_server_task;     // Remember the kthread task_struct
static comm_buffers* _comm_buf = NULL;
static comm_mapping* _comm_map = NULL;
static int _initialized = 0;
static int _cpu = -1;
static char _rcv_buf[RCV_BUF_SZ];

/**
 *
 * Public API
 */
int test_process_server() {

    char* tst_msg = "Hello World!";
    int ret;

    if(!_initialized) {
        return -1;
    }

    printk("kmkprocsrv: Test function invoked\r\n");

    // Send something
    ret = matrix_send_to( _comm_buf, 3, tst_msg, 12 );

    printk("kmkprocsrv: Sent %d in test function\r\n",ret);

    return 1;
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
    printk("kmkprocsrv: Getting processor ID\r\n");
    _cpu = smp_processor_id();
    printk("kmkprocsrv: Processor ID: %d\r\n",_cpu);
    printk("kmkprocsrv: Number of cpus detected: %d\r\n",NR_CPUS);

    // Initialize the communications module
    _comm_map = matrix_init_mapping(COMM_BUFFS_SIZE,COMM_CPU_NUM);
    if(NULL == _comm_map) goto error_init_mapping;
    _comm_buf = matrix_init_buffers(_comm_map , _cpu );
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

            rcved = matrix_recv_from( _comm_buf, i, _rcv_buf, RCV_BUF_SZ);

            // If input was found on this CPU, handle it.
            if (rcved > 0) {
                printk( "kmkprocsrv: Received data from %d\r\n", i );
                printk( _rcv_buf );

            }
        }

        // Sleep a while
		while ( schedule_timeout_interruptible( timeout*HZ ) );

    }


    // Should not reach

    matrix_finalize_mapping( _comm_map );

error_init_mapping:
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




