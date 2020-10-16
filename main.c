#include "include.h"


SRingBuffer_t rbRssiBuf;
uint8_t rssiBuffer[1024*sizeof(SRSSI_t)];


uint8_t SampleMAC[][6] = {
    {0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a},
    {0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b},
    {0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c},
    {0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d},
    {0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e},
    {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f}
};

void GenRSSISample(SRingBuffer_t* buff, uint32_t time)
{
    if(buff == NULL)
    {
        printf("\r\nBuffer not valid");
        return;
    }

    int i;
    for(i = 0; i < time; i++)
    {
        uint8_t u8rand = rand() % ARRAY_LEN(SampleMAC);
        SRSSI_t item;
        memcpy(&item.ID[0], &SampleMAC[u8rand], 6);
        item.RSSI = -50 - (rand() % 100);
        item.u32Hash = crc32(0xFFFFFFFF, &item.ID[0], 6);
        BufferPush(buff, &item);
    }
}

/*************************************************/
static bool DeviceMng_Find(uint32_t hashKey);
SDevice_t* DeviceMng_Get(uint32_t hashKey);
void DeviceMng_InsertRSSI(SRSSI_t devRssi);
void DeviceMng_Remove(uint32_t hashKey);
/*************************************************/

void DeviceMng_Init() {
    for(int i = 0; i < ARRAY_LEN(gDeviceManager); i++)
    {
        // Init device to manager
        gDeviceManager[i].Used = false;
        gDeviceManager[i].DeviceObj.u32Hash = 0;
        memset(&gDeviceManager[i].DeviceObj.ID[0], 0, sizeof(gDeviceManager[i].DeviceObj.ID));
        memset(&gDeviceManager[i].DeviceObj.u8BuffRSSI[0], 0, sizeof(gDeviceManager[i].DeviceObj.u8BuffRSSI));
        memset(&gDeviceManager[i].DeviceObj.u8BuffDistance[0], 0, sizeof(gDeviceManager[i].DeviceObj.u8BuffDistance));
        BufferInit(&gDeviceManager[i].DeviceObj.rbRSSIData,
                   &gDeviceManager[i].DeviceObj.u8BuffRSSI[0],
                   NUM_RSSI_BUF_SIZE,
                   sizeof(float),
                   NULL, NULL, NULL);
        BufferInit(&gDeviceManager[i].DeviceObj.rbDistanceData,
                   &gDeviceManager[i].DeviceObj.u8BuffDistance[0],
                   NUM_DISTANCE_BUF_SIZE,
                   sizeof(SDistance_t),
                   NULL, NULL, NULL);
    }
}

void DeviceMng_InsertRSSI(SRSSI_t devRssi)
{
    if(DeviceMng_Find(devRssi.u32Hash)) {
        // Device is existing
        SDeviceMng_t* device = DeviceMng_Get(devRssi.u32Hash);

        // Check device
        if(device == NULL) return;

        // Insert data to device
        BufferPush(&device->DeviceObj.rbRSSIData, &devRssi.RSSI);
    }
    else {
        // Device not existing
        for(int i = 0; i < ARRAY_LEN(gDeviceManager); i++)
        {
            if(gDeviceManager[i].Used == FALSE)
            {
                // Insert device to manager
                gDeviceManager[i].Used = TRUE;
                gDeviceManager[i].DeviceObj.u32Hash = devRssi.u32Hash;
                memcpy(&gDeviceManager[i].DeviceObj.ID[0], &devRssi.ID[0], sizeof(devRssi.ID));
                memset(&gDeviceManager[i].DeviceObj.u8BuffRSSI[0], 0, sizeof(gDeviceManager[i].DeviceObj.u8BuffRSSI));
                memset(&gDeviceManager[i].DeviceObj.u8BuffDistance[0], 0, sizeof(gDeviceManager[i].DeviceObj.u8BuffDistance));
                BufferFlush(&gDeviceManager[i].DeviceObj.rbRSSIData);
                BufferFlush(&gDeviceManager[i].DeviceObj.rbDistanceData);
            }
        }
    }
}

SDevice_t* DeviceMng_Get(uint32_t hashKey)
{
    for(int i = 0; i < ARRAY_LEN(gDeviceManager); i++)
    {
        if(hashKey == gDeviceManager[i].DeviceObj.u32Hash && gDeviceManager[i].Used == TRUE)
        {
            return &gDeviceManager[i].DeviceObj;
        }
    }
    return NULL;
}

static bool DeviceMng_Find(uint32_t hashKey)
{
    for(int i = 0; i < ARRAY_LEN(gDeviceManager); i++)
    {
        if(hashKey == gDeviceManager[i].DeviceObj.u32Hash && gDeviceManager[i].Used == TRUE)
        {
            return true;
        }
    }

    return false;
}

void DeviceMng_Remove(uint32_t hashKey)
{
    for(int i = 0; i < ARRAY_LEN(gDeviceManager); i++)
    {
        if(hashKey == gDeviceManager[i].DeviceObj.u32Hash && gDeviceManager[i].Used == TRUE)
        {
            gDeviceManager[i].Used = FALSE;
        }
    }
}

int main()
{
	printf("\r\nTest RSSI Manager");
	DeviceMng_Init();
	memset(&rssiBuffer[0],0,sizeof(rssiBuffer));
	BufferInit(&rbRssiBuf, &rssiBuffer, 1024, sizeof(SRSSI_t), NULL, NULL, NULL);

	GenRSSISample(&rbRssiBuf, 700);

	printf("\r\nBuff count: %d", BufferGetCount(&rbRssiBuf));

	for(int i = 0; i < 500; i++)
	{
		SRSSI_t item;
		BufferPop(&rbRssiBuf, &item);

		printf("\r\nHaskey = 0x%08x - RSSI = %0.8f", item.u32Hash, item.RSSI);

		DeviceMng_InsertRSSI(item);

	}

	for(int i = 0; i < 100; i++)
	{
		SRSSI_t	device;
		BufferPop(&gDeviceManager[0].DeviceObj.rbRSSIData, &device);

		printf("\r\nHaskey = 0x%08x - RSSI = %0.8f", device.u32Hash, device.RSSI);
	}

	return 0;
}
