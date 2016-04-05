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

struct x86_pt_regs {
       unsigned long r15;
       unsigned long r14;
       unsigned long r13;
       unsigned long r12;
       unsigned long bp;
       unsigned long bx;
 /* arguments: non interrupts/non tracing syscalls only save up to here*/
       unsigned long r11;
       unsigned long r10;
       unsigned long r9;
       unsigned long r8;
       unsigned long ax;
       unsigned long cx;
       unsigned long dx;
       unsigned long si;
       unsigned long di;
       unsigned long orig_ax;
  /* end of arguments */
  /* cpu exception frame or undefined */
       unsigned long ip;
       unsigned long cs;
       unsigned long flags;
       unsigned long sp;
       unsigned long ss;
/* top of stack page */

};

/*
 * FPU structure and data
 */

struct sh_fpu_hard_struct {
        unsigned long fp_regs[16];
        unsigned long xfp_regs[16];
        unsigned long fpscr;
        unsigned long fpul;

        long status; /* software status information */
};

/* Dummy fpu emulator  */
struct sh_fpu_soft_struct {
        unsigned long fp_regs[16];
        unsigned long xfp_regs[16];
        unsigned long fpscr;
        unsigned long fpul;

        unsigned char lookahead;
        unsigned long entry_pc;
};

union thread_xstate {
        struct sh_fpu_hard_struct hardfpu;
        struct sh_fpu_soft_struct softfpu;
};

struct popcorn_regset_x86_64
{
        /* Program counter/instruction pointer */
        void* rip;

        /* General purpose registers */
        uint64_t rax, rdx, rcx, rbx,
                 rsi, rdi, rbp, rsp,
                 r8, r9, r10, r11,
                 r12, r13, r14, r15;

        /* Multimedia-extension (MMX) registers */
        uint64_t mmx[8];

        /* Streaming SIMD Extension (SSE)
         * registers */
        unsigned __int128 xmm[16];

        /* x87 floating point registers
         * */
        long double st[8];

        /* Segment registers */
        uint32_t cs, ss, ds, es, fs, gs;

        /* Flag register
         * */
        uint64_t rflags;
};

struct popcorn_regset_aarch64
{
        /* Stack pointer & program counter */
        void* sp;
        void* pc;

        /* General purpose registers */
        uint64_t x[31];

        /* FPU/SIMD registers */
        unsigned __int128 v[32];
};

/*
 * Constant macros
 *
 */
#define FIELDS_ARCH struct x86_pt_regs regs;\
	unsigned long migration_pc;\
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
	unsigned long bp;\
	unsigned long ra;\
	struct popcorn_regset_x86_64 regs_x86;\
	struct popcorn_regset_aarch64 regs_aarch;\
	union thread_xstate fpu_state;

/*
 * Structures
 */

typedef struct _fields_arch{
	FIELDS_ARCH
}field_arch;

#endif


