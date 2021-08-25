#include <mem.h>


#include "ringbuf.h"


short RINGBUF_Init(RINGBUF *r, unsigned short size)
{
    r->buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
    r->bufferSize = size;
    r->readIndex = 0;
    r->writeIndex = 0;
    r->dataSize = 0;

    return 0;
}

short RINGBUF_Put(RINGBUF *r, unsigned char c)
{
    r->dataSize++;
    *(r->buffer + r->writeIndex) = c;
    r->writeIndex = (r->writeIndex + 1) % r->bufferSize;
    return 0;
}

short RINGBUF_Get(RINGBUF *r, unsigned char* c)
{
    if (r->dataSize > 0)
    {
        *c = *(r->buffer + r->readIndex);
        r->dataSize--;
        r->readIndex = (r->readIndex + 1) % r->bufferSize;
        return 0;
    }
    return -1;
}

unsigned short RINGBUF_Size(RINGBUF *r)
{
    return r->dataSize;
}

short RINGBUF_At(RINGBUF *r, unsigned short nIndex, unsigned char* c)
{
    if (r->dataSize > 0 && nIndex < r->dataSize)
    {
        *c = *(r->buffer + (r->readIndex + nIndex) % r->bufferSize);
        return 0;
    }
    return -1;
}
