#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////// Include Files /////////////////////////////////////////////
#include <stdint.h>
#include <string.h>

typedef void (*CallbackFunctionIO)(void *params);
typedef uint8_t    BOOL;
#define TRUE       (1)
#define FALSE      (0)
#define memcpy_secure memcpy

///////////////////////////////////// Constant Definitions ///////////////////////////////////////////

/////////////////////////////////////// Type Definitions /////////////////////////////////////////////
// Generic Ring buffer structure
typedef struct S_RING_BUFFER
{
    void*                   pvBuffer;                      // Data buffer
    uint16_t                uiBufferSize;                  // Size of buffer or the total of elements
    uint16_t                uiElementCount;                // The element count of buffer
    uint16_t                uiBufferPopPtr;                // The pointer to start reading
    uint16_t                uiBufferPushPtr;               // The pointer to start writing

    uint16_t                uiBKElementCount;              // Backup value of the element count of buffer
    uint16_t                uiBKBufferPopPtr;              // Backup value of the pointer to start reading
    uint16_t                uiBKBufferPushPtr;             // Backup value of the pointer to start writing    
    
    uint16_t                uiElementSize;                 // Size of each element of the buffer in byte
    BOOL                    bBufferPopEnable;              // Data popping enabling flag
    BOOL                    bBufferPushEnable;             // Data pushing enabling flag
    
    CallbackFunctionIO      callbackLock;                  // Call-back function for locking multi-access
    CallbackFunctionIO      callbackUnlock;                // Call-back function for unlocking multi-access
    void*                   pvCallbackParam;              // Parameter of the call-back function
} SRingBuffer_t;

///////////////////////////// Macros (Inline Functions) Definitions //////////////////////////////////

///////////////////////////////////// Function Prototypes ////////////////////////////////////////////
void            BufferInit(SRingBuffer_t* psRingBuffer, 
                           void* pvBuffer, 
                           uint16_t uiBufferSize, 
                           uint16_t uiElementSize, 
                           CallbackFunctionIO callbackLock, 
                           CallbackFunctionIO callbackUnlock, 
                           void* pvCallbackParam);

BOOL            BufferPushStream(SRingBuffer_t* psRingBuffer, void* pvStream, uint16_t uiLength);
uint16_t        BufferPopStream(SRingBuffer_t* psRingBuffer, void* pvStream, uint16_t uiLength);
BOOL            BufferPush(SRingBuffer_t* psRingBuffer, void* pvData);
uint16_t        BufferPop(SRingBuffer_t* psRingBuffer, void* pvData);
BOOL            BufferPushBack(SRingBuffer_t* psRingBuffer, uint16_t uiPushBackNumber);
BOOL            BufferIsPushEnable(SRingBuffer_t* psRingBuffer);
BOOL            BufferIsPopEnable(SRingBuffer_t* psRingBuffer);
void            BufferEnablePop(SRingBuffer_t* psRingBuffer);
void            BufferDisablePop(SRingBuffer_t* psRingBuffer);
void            BufferEnablePush(SRingBuffer_t* psRingBuffer);
void            BufferDisablePush(SRingBuffer_t* psRingBuffer);
uint16_t        BufferGetCount(SRingBuffer_t* psRingBuffer);
uint16_t        BufferGetAvailableCount(SRingBuffer_t* psRingBuffer);
void            BufferSaveState(SRingBuffer_t* psRingBuffer);
void            BufferRestoreState(SRingBuffer_t* psRingBuffer);
void            BufferFlush(SRingBuffer_t* psRingBuffer);

///////////////////////////////////// Variable Definitions ///////////////////////////////////////////


#ifdef __cplusplus
}
#endif

#endif	// _RING_BUFFER_H_
