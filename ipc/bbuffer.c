// bbuffer.c
// Author: Antonio Barbalace, VirginiaTech 2012

// maybe this file must be all contained in a h file...
// basically is a cicular buffer BYTE ORIENTED
// FUTURE support two usages: typed oriented and byte (memcpy style oriented)
// NUMA AWARE - allocating memory on the right node (libnuma)
// *numa_alloc_onnode(size_t size, int node);
// in MKLINUX the mklinux_alloc_onnode will be used
// for multiple writer 2 phase commit can be a solution
// for zero copy memory get/dispose of buffers is fantastic solution

//#include <stdint.h>

// the following are defined in include/asm-generic/int-*.h
//#include "types.h"
//typedef unsigned short u16;
//typedef unsigned int u32;

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>

#include <linux/smp.h>

#ifndef CACHE_ALIGNED
 #define CACHE_ALIGNED
#endif

#include "bbuffer.h"


//and maybe one cache line before and after the buffer must be a watermark
// header and buffer must be allocate together in the same private/local area

//#include <stdlib.h>
//#include <stdio.h>
//#include <assert.h>

//#include "alloc.h"

///////////////////////////////////////////////////////////////////////////////
// bbuffer_init
/*
bbuffer_t * bbuffer_init (int size, int node)
{
	int padded_size = size;

#ifdef CACHE_ALIGNED
	padded_size = BBUFFER_SPACE(size);
//#endif  CACHE_ALIGNED
	BBUFFER_CHECK(padded_size);

	bbuffer_t* bb = (bbuffer_t*) alloc_on_node(
			BBUFFER_SIZEOF(padded_size), node);
	// returns zero if malloc fails
	if (!bb)
		return bb;

	BBUFFER_INIT(bb, padded_size);
	return bb;
}
*/
///////////////////////////////////////////////////////////////////////////////
// bbuffer_finalize
/*
void bbuffer_finalize (bbuffer_t* bb)
{
	free_on_node(bb);
}
*/
///////////////////////////////////////////////////////////////////////////////
// bbuffer_put

/* further improvements
 * 1. each entry must be cache aligned (avoid false sharing)
 * 2. waiting put (on buffer overrun)
 */
int bbuffer_put (bbuffer_t* buf, char* src, int count)
{
	register struct _raw_indexes ht;
	register int size, avail_elements; // used_elements,

	ht = buf->indexes;
	size = buf->size;
	//used_elements = ((size + ht.head) - ht.tail) % size;
    avail_elements = size - ((size + ht.head) - ht.tail) % size; //used_elements;

    /* if the elements does not fit in the buffer return error */
    if (count > (avail_elements -1))
    	return -1;

    /* when tail is greater then head content is written across the end */
    if ((ht.head + count) >= size) {
    	memcpy(&(buf->buffer[ht.head]),
    			src, (size - ht.head));
    	memcpy(&(buf->buffer[0]),
    			(src + (size - ht.head)), (ht.head + count) % size);
    	ht.head = (ht.head + count) % size;
	}
	else {
		memcpy(&(buf->buffer[ht.head]), src, count);
		ht.head = (ht.head + count);
	}

    buf->indexes.head = ht.head;
    return count;
}

// TODO
//int bbuffer_puttofit (arch_header_t *buf, char *src, int count) {
//	return 0;
//}

///////////////////////////////////////////////////////////////////////////////
// bbuffer_get


// considering that a message can be written on the bound and finish on the other side, this is maybe not the best for performance but..
// an external buffer where to copy the data is not a bad idea we can study a message passing with linear address of the contents i
//order to do not require further copies (like lists of buckets each bucket can contain a different message
// RETURNS the number of characters copied

// how many bytes? right now we have no clue on how many bytes -> we need a micro header in the buffer
// otherwise we can create a circular buffer of fixed elements (every time you insert o pick up one or more elements)

/* THINK ABOUT...
CODE IS NOT RELIABLE RIGHT NOW, microheaders or object oriented buffer ?! I mean if the buffer is byte oriented is fantastic!
		i.e. the object granularity is char the code is ready to go (it is ok for many apps, like the multi cache test)
*/

/* TODO
two alternatives:

the idea to avoid the count variable is to store a bit somewhere in the
tail or head that indicate that head and tail are full but

this is the alternative to lost one element when head and tail cannot be the same
*/
int bbuffer_get (bbuffer_t * buf, char * dst, int count)
{
	register struct _raw_indexes ht;
	register int size;
	register int used_elements; // avail_elements;

	ht = buf->indexes;
	size = buf->size;

	used_elements = (((size + ht.head) - ht.tail) % size);
 	//avail_elements = size - used_elements;

 	/* nothing present in the buffer */
	if (used_elements == 0 ) // tail == head
		return -1;

	/* amount to copy */
	if (count > used_elements )
		count = used_elements;

	/* when tail is greater then head content is written across the end */
	if (ht.tail + count >= size) {
		memcpy(dst, &(buf->buffer[ht.tail]),
				(size - ht.tail));
		memcpy((dst + (size - ht.tail)), &(buf->buffer[0]),
				(ht.tail + count) % size);
		ht.tail = (ht.tail + count) % size;
	}
	else {
		memcpy(dst, &(buf->buffer[ht.tail]), count);
		ht.tail= (ht.tail + count);
	}

	buf->indexes.tail = ht.tail;
	return count;
}

int bbuffer_count (bbuffer_t * buf)
{
	register struct _raw_indexes ht;
	register int size;

	ht = buf->indexes;
	size = buf->size;

	return (((size + ht.head) - ht.tail) % size);
}

static void bbuffer_dump(bbuffer_t * bb)
{
  printk("%s: head %d tail %d size %d count %d\n",
		  __func__, bb->indexes.head, bb->indexes.tail, bb->size,
		  (((bb->size + bb->indexes.head) - bb->indexes.tail) % bb->size));
}
