/****************************************************
 *
 * Popcorn performance measurement framework api
 * DKatz
 *
 */

#ifndef PCN_PERF_H_
#define PCN_PERF_H_

#include <linux/list.h>
#include <linux/spinlock.h>

typedef struct _pcn_perf_entry pcn_perf_entry_t;
struct _pcn_perf_entry {
    unsigned long long start;
    unsigned long long end;
    int in_progress;
    int context_id;
    int tok;
    char note[512];
    pcn_perf_entry_t* next;
    pcn_perf_entry_t* prev;
};

typedef struct _pcn_perf_context pcn_perf_context_t;
struct _pcn_perf_context {
    char name[512];
    int context_id;
    int home_cpu;
    int is_active;
    spinlock_t lock;
    pcn_perf_entry_t* entry_list;
    pcn_perf_context_t* next;
    pcn_perf_context_t* prev;
};

void do_popcorn_perf_start(void);
void do_popcorn_perf_end(void);
void perf_init_context( pcn_perf_context_t * cxt, char * name );
int perf_measure_start( pcn_perf_context_t * cxt );
void perf_measure_stop ( pcn_perf_context_t * cxt, char * note, int tok );
void perf_reset_all(void);

#endif
