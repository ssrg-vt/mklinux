/**
 * Serve up processes to a remote client cpu
 *
 * DKatz
 */

#include <../ipc/mcomm.h> // IPC
#include <linux/kthread.h>
#include <linux/export.h>
#include <linux/delay.h>

/**
 * Module variables
 */
static struct task_struct* process_server_task;     // Remember the kthread task_struct

/**
 * process_server
 * The kthread process loop.
 */
static int process_server(void* dummy) {

    
    unsigned long timeout = 1;  // Process loop wait duration

    // Initialize the communications module

    while (1) {

        // Check for work to do.

        // Sleep a while
		while (schedule_timeout_interruptible(timeout*HZ));

    }

error:

    // Finalize the communications module

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
module_init(process_server_init);




