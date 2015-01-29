/*
 * process_server_macro.h
 *
 *  Created on: Jun 13, 2014
 *      Author: marina
 */

#ifndef PROCESS_SERVER_MACRO_H_
#define PROCESS_SERVER_MACRO_H_

#define PROCESS_SERVER_CLONE_SUCCESS 0
#define PROCESS_SERVER_CLONE_FAIL 1

#define REPLICATION_STATUS_VALID 3
#define REPLICATION_STATUS_WRITTEN 1
#define REPLICATION_STATUS_INVALID 2
#define REPLICATION_STATUS_NOT_REPLICATED 0

#define VMA_OP_NOP 0
#define VMA_OP_UNMAP 1
#define VMA_OP_PROTECT 2
#define VMA_OP_REMAP 3
#define VMA_OP_MAP 4
#define VMA_OP_BRK 5

#define VMA_OP_SAVE -70
#define VMA_OP_NOT_SAVE -71

#define EXIT_ALIVE 0
#define EXIT_THREAD 1
#define EXIT_PROCESS 2
#define EXIT_FLUSHING 3
#define EXIT_NOT_ACTIVE 4

#define PARTIAL_VMA_MANAGEMENT 0
#define NOT_REPLICATED_VMA_MANAGEMENT 1

#define FOR_2_KERNELS 1
#define DIFF_PAGE 0

#if !FOR_2_KERNELS
#undef DIFF_PAGE
#define DIFF_PAGE 0
#endif

#define SUPPORT_FOR_CLUSTERING

#define MIGRATE_FPU 0

//#define MAX_KERNEL_IDS NR_CPUS
#define MAX_KERNEL_IDS 2

/**
 * Use the preprocessor to turn off printk.
 */
#define PROCESS_SERVER_VERBOSE 1
#define PROCESS_SERVER_VMA_VERBOSE 1
#define PROCESS_SERVER_NEW_THREAD_VERBOSE 0
#define PROCESS_SERVER_MINIMAL_PGF_VERBOSE 0

#define READ_PAGE 0
#define PAGE_ADDR 0

#define CHECKSUM 0
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
