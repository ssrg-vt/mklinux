// bbuffer.h
// Author: Antonio Barbalace, Virginia Tech 2012

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
