#ifndef _RING_BUF_H_
#define _RING_BUF_H_

#include <stdlib.h>

typedef struct{
	unsigned char *buffer;            /**< Original pointer */
    unsigned short readIndex;         /**< Read pointer */
    unsigned short writeIndex;		  /**< Write pointer */
    unsigned short dataSize;          /**< Number of filled slots */
    unsigned short bufferSize;        /**< Buffer size */
}RINGBUF;


short RINGBUF_Init(RINGBUF *r, unsigned short size);
short RINGBUF_Put(RINGBUF *r, unsigned char c);
short RINGBUF_Get(RINGBUF *r, unsigned char* c);
unsigned short RINGBUF_Size(RINGBUF *r);
short RINGBUF_At(RINGBUF *r, unsigned short nIndex, unsigned char* c);

#endif
