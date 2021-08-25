#ifndef INCLUDE_PROCESSMESSAGE_H_
#define INCLUDE_PROCESSMESSAGE_H_

#include "message.h"

typedef enum {
	STATE_OFF = 0,
	STATE_ON = 1
} ChannelState_enum;

void SetChannelState(ChannelState_enum state);
ChannelState_enum GetChannelState();
ChannelState_enum ToggleChannelState();
int processMessage(uint8_t *data);

#endif /* INCLUDE_PROCESSMESSAGE_H_ */
