/*
 * Filter.c
 *
 *  Created on: 28 но€б. 2019 г.
 *      Author: nemo
 */

#include "filter.h"

void FilterInit(struct Filter *filter) {
    filter->index = 0;
    filter->count = 0;
    int i;
    for(i = 0; i < FILTER_LEN; i++) {
        filter->data[i] = 0;
    }
}

void FilterAppend(struct Filter *filter, int32_t data) {
    filter->data[filter->index++] = data;
    if(filter->count < FILTER_LEN)
        filter->count++;
    if(filter->index >= FILTER_LEN)
        filter->index = 0;
}

uint16_t FilterValue(struct Filter *filter) {
    uint32_t sum = 0;
    if(filter->count == 0)
        return 0;
    int i;
    for(i = 0; i < filter->count; i++)
        sum += filter->data[i];
    return sum / filter->count;
}

uint16_t SP5060FilterLength(struct Filter *filter) {
    return filter->count;
}


