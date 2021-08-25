#include <stdio.h>
#include "string.h"

#include "message.h"
#include "ringbuf.h"


void packetManagerInit(RINGBUF *ringbuff)
{
	RINGBUF_Init(ringbuff, 280);
}

static void skipForHeader(RINGBUF *ringbuff)
{
	uint8_t headerChar1 = 0x00;
	uint8_t headerChar2 = 0x00;


    RINGBUF_At(ringbuff, 0, &headerChar1);
    RINGBUF_At(ringbuff, 1, &headerChar2);

    while(headerChar1 != PACKET_HEADER1
			&& headerChar2 != PACKET_HEADER2
			&& RINGBUF_Size(ringbuff) >= MIN_MESSAGE_SIZE) //»щем первый заголовок
    {
		uint8_t c;
    	RINGBUF_Get(ringbuff, &c);
	    RINGBUF_At(ringbuff, 0, &headerChar1);
	    RINGBUF_At(ringbuff, 0, &headerChar2);
    }
}

int packetManagerGetMessage(RINGBUF *ringbuff, uint8_t *data)
{
	uint8_t c;
	uint8_t crcChar = 0x00;
	uint16_t lenght = 0;

    if(RINGBUF_Size(ringbuff) < 2)
    	return -1;
//    printf("1\n");
    skipForHeader(ringbuff);

    if(RINGBUF_Size(ringbuff) < MIN_MESSAGE_SIZE)
    	return -1;

//    printf("2\n");

    RINGBUF_At(ringbuff, MESSAGE_LENGTH_OFFSET, (uint8_t*)&lenght);

//    printf("Size: %d %d\n", lenght, RINGBUF_Size(ringbuff));

    if(RINGBUF_Size(ringbuff) < MIN_MESSAGE_SIZE + lenght)
    	return -1;

    RINGBUF_At(ringbuff, PACKET_HEADER_SIZE + lenght, &crcChar);

    if(crcChar == PACKET_FINITE)
	{
    	int i;

    	for(i = 0; i < PACKET_HEADER_SIZE; i++)
    		RINGBUF_Get(ringbuff, &c);
    	for(i = 0; i < lenght; i++)
		{
			if(RINGBUF_Get(ringbuff, &c) != -1)
				data[i] = c;
		}
//    	printf("CRC true");
		return i;
	}
	RINGBUF_Get(ringbuff, &c);
    return -1;
}

void packetManagerAppend(RINGBUF *ringbuff, uint8_t nData)
{
	RINGBUF_Put(ringbuff, nData);
}

short createPacket(char *buffer, int len, char *outBuffer)
{
	outBuffer[0] = PACKET_HEADER1;
	outBuffer[1] = PACKET_HEADER2;
	((uint8_t*)outBuffer)[2] = len;
	memcpy(outBuffer + 3, buffer, len);
	outBuffer[len + 3] = PACKET_FINITE;
	return len + 4;
}
