/**
 * @file page_server.h
 * (private interface)
 *
 * Popcorn Linux page server private interface
 * This work is an extension of Marina Sadini MS Thesis, plese refer to the
 * Thesis for further information about the algorithm.
 *
 * @author Antonio Barbalace, SSRG Virginia Tech 2016
 */

#ifndef KERNEL_POPCORN_PAGE_SERVER_H_
#define KERNEL_POPCORN_PAGE_SERVER_H_

/**
 * Initialization function to register the page server with the messaging layer and
 * related dispatching functions. Should be called by process server initialization routine.
 *
 * @return Returns 0 on success, an error code otherwise.
 */
int page_server_init (void);

#endif /* KERNEL_POPCORN_PAGE_SERVER_H_ */
