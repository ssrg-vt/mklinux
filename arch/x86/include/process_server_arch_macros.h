/*
 * File:
 * process_server_arch_macros.h
 *
 * Description:
 * 	this file provides the architecture specific macro and structures of the
 *  helper functionality implementation of the process server
 *
 * Created on:
 * 	Sep 19, 2014
 *
 * Author:
 * 	Sharath Kumar Bhat, SSRG, VirginiaTech
 *
 */

#ifndef PROCESS_SERVER_ARCH_MACROS_H_
#define PROCESS_SERVER_ARCH_MACROS_H_

/*
 * Constant macros
 *
 */
#define FIELDS_ARCH struct pt_regs regs;\
	unsigned long thread_usersp;\
	unsigned long old_rsp;\
	unsigned short thread_es;\
	unsigned short thread_ds;\
	unsigned long thread_fs;\
	unsigned short thread_fsindex;\
	unsigned long thread_gs;\
	unsigned short thread_gsindex; \
	unsigned int  task_flags;\
	unsigned char task_fpu_counter;\
	unsigned char thread_has_fpu;\
	union thread_xstate fpu_state;

/*
 * Structures
 */

typedef struct _fields_arch{
	FIELDS_ARCH
}field_arch;

#endif


