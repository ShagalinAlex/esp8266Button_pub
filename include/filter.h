/*
 * Filter.h
 *
 *  Created on: 28 но€б. 2019 г.
 *      Author: nemo
 */

#ifndef INCLUDE_FILTER_H_
#define INCLUDE_FILTER_H_

#include <stdint.h>

#define FILTER_LEN 4

struct Filter{
    uint16_t index;
    uint16_t count;
    uint16_t maxFilterSize;
    int32_t data[FILTER_LEN];
};

void FilterInit(struct Filter *filter);
void FilterAppend(struct Filter *filter, int32_t data);
uint16_t FilterValue(struct Filter *filter);
uint16_t FilterLength(struct Filter *filter);



#endif /* INCLUDE_FILTER_H_ */
