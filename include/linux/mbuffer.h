// mbuffer.h
// Author: Antonio Barbalace, Virginia Tech 2012
#ifndef _MBUFFER_H
#define _MBUFFER_H

#include <linux/bbuffer.h>

/*
 *
 */
static inline bbuffer_t* mbuffer_init (int size, int node)
{
	return bbuffer_init(size, node);
}

/*
 *
 */
static inline void mbuffer_finalize (bbuffer_t* bb)
{
	return bbuffer_finalize(bb);
}
/*
 *
 */
int mbuffer_put (bbuffer_t* bb, char* src, int count);
/*
 *
 */
int mbuffer_get (bbuffer_t* bb, char* dst, int count);

static inline int mbuffer_count (bbuffer_t * buf)
{
	return bbuffer_count(buf);
}

#endif //_MBUFFER_H
