/*
 * process_server_macro.h
 *
 *  Created on: Jun 13, 2014
 *      Author: marina
 */

#ifndef PROCESS_SERVER_MACRO_H_
#define PROCESS_SERVER_MACRO_H_

#define READ_PAGE 0
#define PAGE_ADDR 0

#define STATISTICS 0

#if PROCESS_SERVER_VERBOSE
#define PSPRINTK(...) printk(__VA_ARGS__)
#undef STATISTICS
#define STATISTICS 1
#else
#define PSPRINTK(...) ;
#endif

#if PROCESS_SERVER_VMA_VERBOSE
#define PSVMAPRINTK(...) printk(__VA_ARGS__)
#else
#define PSVMAPRINTK(...) ;
#endif

#if PROCESS_SERVER_NEW_THREAD_VERBOSE
#define PSNEWTHREADPRINTK(...) printk(__VA_ARGS__)
#else
#define PSNEWTHREADPRINTK(...) ;
#endif

#if PROCESS_SERVER_MINIMAL_PGF_VERBOSE
#define PSMINPRINTK(...) printk(__VA_ARGS__)
#else
#define PSMINPRINTK(...) ;
#endif


#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

#define NR_THREAD_PULL 32
#define THREAD_POOL_SIZE 64

#endif /* PROCESS_SERVER_MACRO_H_ */
