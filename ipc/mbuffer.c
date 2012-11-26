// mbufer.c
// Author: Antonio Barbalace, Virginia Tech 2012
// messagge buffer (based on byte buffer)


//#include <stdint.h>

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
#include "mbuffer.h"



//and maybe one cache line before and after the buffer must be a watermark
// header and buffer must be allocate together in the same private/local area
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>



typedef struct hheader {
	union {
		__indexes_t head_head;
		struct _raw_heads {
			__index_t part, total;
		} header;
	};
} hheader_t;

//#define MALIGNMENT CACHE_LINE
#define MALIGNMENT sizeof(hheader_t)

int mbuffer_put (bbuffer_t* buf, char* src, int count)
{
	register struct _raw_indexes ht;
	struct _raw_heads hh;
	register int acount, size, avail_elements; // used_elements,

	ht = buf->indexes;
	size = buf->size;

	acount = sizeof(hheader_t) + count;
	acount += MALIGNMENT - (acount % MALIGNMENT);

	//used_elements = ((size + ht.head) - ht.tail) % size;
    avail_elements = size - ((size + ht.head) - ht.tail) % size; //used_elements;

    /* if the elements does not fit in the buffer return error */
    if (acount > (avail_elements -1))
    	return -1;

    hh.part = count;
    hh.total = acount;

    /* head will cross the buffer */
    if ((ht.head + sizeof(hheader_t)) >= size) {
    	// copy the header
    	memcpy(&(buf->buffer[ht.head]),
    			&hh, (size - ht.head) );
    	memcpy(&(buf->buffer[0]),
    	    			(&hh + (size - ht.head)), (ht.head + sizeof(hheader_t)) % size);
    	// copy the data
    	memcpy(&(buf->buffer[((ht.head + sizeof(hheader_t)) % size)]),
    	    			src, count);
    	// mem set aligned data
    	memset(&(buf->buffer[((ht.head + sizeof(hheader_t)) % size) + count]),
    			-1, (acount - count - sizeof(hheader_t)) );
    	// new head value
    	ht.head = (ht.head + acount) % size;
    }
    /* data will cross the buffer */
    else if ((ht.head + sizeof(hheader_t) + count) >= size) {
    	// copy the header
    	memcpy(&(buf->buffer[ht.head]),
    			&hh, sizeof(hheader_t));
    	// copy the data
    	memcpy(&(buf->buffer[ht.head]) + sizeof(hheader_t),
    			src, (size - sizeof(hheader_t) - ht.head));
    	memcpy(&(buf->buffer[0]),
    			(src + (size - sizeof(hheader_t) - ht.head)), (ht.head + sizeof(hheader_t) + count) % size);
    	// memset aligned data
    	memset(&(buf->buffer[((ht.head + sizeof(hheader_t) + count) % size)]),
    			-1, (acount - count));
    	// new head value
    	ht.head = (ht.head + acount) % size;
	}
    /* aligned data will cross the buffer */
    else if ((ht.head + acount) >= size) {
		// copy the header
		memcpy(&(buf->buffer[ht.head]), &hh, sizeof(hheader_t));
		// copy the data
		memcpy(&(buf->buffer[(ht.head + sizeof(hheader_t))]), src, count);
		// memset the alignment
		memset(&(buf->buffer[(ht.head + sizeof(hheader_t) + count)]),
				-1, (size -ht.head -sizeof(hheader_t) -count));
		memset(&(buf->buffer[0]),
				-1, ((ht.head + acount) % size));
		// new head value
    	ht.head = (ht.head + acount) % size;
    }
    /* no data crossing */
	else {
		// copy the header
		memcpy(&(buf->buffer[ht.head]), &hh, sizeof(hheader_t));
		// copy the data
		memcpy(&(buf->buffer[(ht.head + sizeof(hheader_t))]), src, count);
		// memset the alignment
		memset(&(buf->buffer[(ht.head + sizeof(hheader_t) + count)]), -1, (acount -count));
		// new head value
		ht.head = (ht.head + acount);
	}

    buf->indexes.head = ht.head;
    return count;
}

int mbuffer_get (bbuffer_t * buf, char * dst, int count)
{
	register struct _raw_indexes ht;
	struct _raw_heads hh;
	register int size;
	register int used_elements; // avail_elements;

	ht = buf->indexes;
	size = buf->size;

	used_elements = (((size + ht.head) - ht.tail) % size);
 	//avail_elements = size - used_elements;

 	/* nothing present in the buffer */
	if (used_elements == 0 ) // tail == head
		return -1;

	/* read the header, can cross the buffer */
	if (ht.tail + sizeof(hheader_t) >= size) {
		memcpy(&hh, &(buf->buffer[ht.tail]), size -ht.tail);
		memcpy(&hh, &(buf->buffer[0]), (ht.tail + sizeof(hheader_t) % size) );
	}
	else {
		memcpy(&hh, &(buf->buffer[ht.tail]), sizeof(hheader_t));
	}
	ht.tail = (ht.tail + sizeof(hheader_t) % size);

	/* amount to copy */
	if (count >= hh.part )
		count = hh.part;
	else
		// exit if there is no enough space
		return -1;

	/* when tail is greater then head content is written across the end */
	if (ht.tail + count >= size) {
		memcpy(dst, &(buf->buffer[ht.tail]),
				(size - ht.tail));
		memcpy((dst + (size - ht.tail)), &(buf->buffer[0]),
				(ht.tail + count) % size);
	}
	else {
		memcpy(dst, &(buf->buffer[ht.tail]), count);
	}
	ht.tail = (ht.tail + count) % size;

	/* check for the align space */
	register acount = hh.total - hh.part - sizeof(hheader_t);
	if ( acount )
		ht.tail = (ht.tail + acount) % size;

	buf->indexes.tail = ht.tail;
	return count;
}


static void mbuffer_dump(bbuffer_t * bb)
{
  printk("mbuffer_dump: head %d tail %d size %d count %d\n",
		  bb->indexes.head, bb->indexes.tail, bb->size,
		  (((bb->size + bb->indexes.head) - bb->indexes.tail) % bb->size));
}
