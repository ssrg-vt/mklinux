/*
 * process_server_macro.h
 *
 *  Created on: Jun 13, 2014
 *      Author: marina
 */

#ifndef PROCESS_SERVER_MACRO_H_
#define PROCESS_SERVER_MACRO_H_

#define SUPPORT_FOR_CLUSTERING

#define READ_PAGE 0
#define PAGE_ADDR 0

#define STATISTICS 0
#define TIMING 0

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


#if PARTIAL_VMA_MANAGEMENT
#undef NOT_REPLICATED_VMA_MANAGEMENT
#define NOT_REPLICATED_VMA_MANAGEMENT 0
#endif

#define PAD_LONG_MESSAGE(x) (((x-(x%PCN_KMSG_PAYLOAD_SIZE))/PCN_KMSG_PAYLOAD_SIZE)*PCN_KMSG_PAYLOAD_SIZE)+PCN_KMSG_PAYLOAD_SIZE

#define NR_THREAD_PULL 1
#define THREAD_POOL_SIZE 64

#if TIMING
#define FRL 0 //fetch read local
#define FWL 1 //fetch write local
#define FRR 2 //fetch read remote
#define FWR 3 //fetch write remote
#define VW 4 // write on a valid copy
#define VR 5 // read on a valid copy (in multithread)
#define MW 6 // write on a mofified copy (in multithread)
#define MR 7 // read on a mofified copy (in multithread)
#define IW 8 // write on a invalid copy (only 2 kernels)
#define IR 9 // read invalid copy
#define NRR 10 //read on a not replicated copy (in multithread)
#define NRW 11 // write on a not replicated copy (in multithread)
#define NR_TYPES 12

#define FIRST_MIG 0
#define FIRST_MIG_WITH_FORK 1
#define NORMAL_MIG 2
#define BACK_MIG 3
#define FIRST_MIG_R 4
#define NORMAL_MIG_R 5
#define BACK_MIG_R 6
#define NR_MIG 7
#endif

#endif /* PROCESS_SERVER_MACRO_H_ */
