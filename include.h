/*
 * include.h
 *
 *  Created on: Oct 14, 2020
 *      Author: tuannt66
 */

#ifndef INCLUDE_H_
#define INCLUDE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/crc32.h"
#include "inc/slist.h"
#include "inc/RingBuffer.h"


#define NUM_RSSI_BUF_SIZE       (100)
#define NUM_DISTANCE_BUF_SIZE   (100)
#define NUM_DEVICE_BUF_SIZE     (30)

#define ARRAY_LEN(x)     ((sizeof(x)/sizeof(x[0])))

typedef struct {
    float              Distance;
    uint32_t           TimeStamp;
} SDistance_t;

typedef struct {
    uint32_t           u32Hash;
    uint8_t            ID[6];
    float              RSSI;
} SRSSI_t;

typedef struct {
    uint32_t           u32Hash;
    uint8_t            ID[6];
    float              avgDistance;

    SRingBuffer_t      rbRSSIData;
    SRingBuffer_t      rbDistanceData;

    // private data
    uint8_t            u8BuffRSSI[NUM_RSSI_BUF_SIZE * (sizeof(float))];
    uint8_t            u8BuffDistance[NUM_RSSI_BUF_SIZE * (sizeof(SDistance_t))];
} SDevice_t;

typedef struct {
    bool               Used;
    SDevice_t          DeviceObj;
} SDeviceMng_t;

SDeviceMng_t           gDeviceManager[NUM_DEVICE_BUF_SIZE];






#endif /* INCLUDE_H_ */
