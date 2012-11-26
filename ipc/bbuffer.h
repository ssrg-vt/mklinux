// bbuffer.h
// Author: Antonio Barbalace, Virginia Tech 2012

//
//#define WORLD_BYTES (sizeof(long))
#ifndef WORLD_BYTES
 #define WORLD_BYTES 4
#endif

#if (WORLD_BYTES == 4)
typedef u16 __index_t;
typedef u32 __indexes_t;
#else
typedef u32 __index_t;
typedef u64 __indexes_t;
#endif

#ifndef CACHE_LINE
 #define CACHE_LINE 64
#endif

typedef struct bbuffer {
	union {
		__indexes_t head_tail;
		struct _raw_indexes {
			__index_t head, tail;
		} indexes;
	};
	int size; //buffer size (must be padded)
	char pad[(CACHE_LINE - sizeof(struct _raw_indexes) - sizeof(int))];
	char buffer[]; //actual buffer (cache aligned)
} bbuffer_t;


#ifdef CACHE_ALIGNED
 /* the returned memory must be cache aligned */
#define CHECK_CACHE_ALIGNED(addr) BUG_ON(!((unsigned long)((void*)addr) & (CACHE_LINE -1)));
 //#define CHECK_CACHE_ALIGNED(addr) assert(!((unsigned long)((void*)addr) & (CACHE_LINE -1)));
//#define CHECK_CACHE_ALIGNED(addr) assert(((unsigned long)((void*)addr) & (CACHE_LINE -1)));
#else /* !CACHE_ALIGNED */
 #define CHECK_CACHE_ALIGNED(addr)
#endif /* !CACHE_ALIGNED */

/* every byte must be accessed using an __index_t index */
#define BBUFFER_LIMIT ((0x01 << sizeof(__index_t)) -1)
#define BBUFFER_CHECK(pad_size) assert((pad_size > BBUFFER_LIMIT))

#define BBUFFER_SPACE(pad_size) ((!(pad_size%CACHE_LINE)) ? \
		(pad_size) : (pad_size + (CACHE_LINE - (pad_size%CACHE_LINE))))

#define BBUFFER_SIZEOF(pad_size) (sizeof(bbuffer_t) + pad_size)

#define BBUFFER_INIT(bbuf, pad_size) ({ \
	CHECK_CACHE_ALIGNED(bbuf); \
	memset(bbuf, 0, sizeof(bbuffer_t)); \
	bbuf->size = pad_size; })

/*
 *
 */
bbuffer_t* bbuffer_init (int size, int node);
/*
 *
 */
void bbuffer_finalize (bbuffer_t* bb);
/*
 *
 */
int bbuffer_put (bbuffer_t* bb, char* src, int count);
/*
 *
 */
int bbuffer_get (bbuffer_t* bb, char* dst, int count);

int bbuffer_count (bbuffer_t * buf);
