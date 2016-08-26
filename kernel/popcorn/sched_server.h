/**
 * @file sched_server.h
 * (private interface)
 *
 * Popcorn Linux scheduler server private interface
 *
 * @author Antonio Barbalace, SSRG Virginia Tech 2016
 */

#ifndef KERNEL_POPCORN_SCHED_SERVER_H_
#define KERNEL_POPCORN_SCHED_SERVER_H_

/**
 * This function initializes the scheduler server extension
 *
 * @return Returns 0 on success, an error code otherwise. Note that the
 *         function can succeed even if it fails to registers /proc entries
 *         in that case, because is not vital to the functioning of the
 *         system a WARNING on the kernel log is issued.
 */
int sched_server_init (void);

#endif /* KERNEL_POPCORN_SCHED_SERVER_H_ */
