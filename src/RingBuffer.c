//////////////////////////////////////////////////////////////////////////////////////////////////////
// 	File name	:	RingBuffer.c
// 	Brief 		: 	A ring buffer of data stream. This ring buffer library can be used for all data
//					type: 8-bit, 16, 32,64-bit, structure data types....
//	Author 		: 	AnhNH57
//  Note 		: 	
//////////////////////////////////////////////////////////////////////////////////////////////////////
// 	MODIFICATION HISTORY:
//
// 	Ver  	PIC  		Date       	Changes
// 	----- 	-------- 	---------- 	------------------------------------------------------------------
// 	1.00  	AnhNH57  	02-07-2013 	Create Framework
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////// Include Files /////////////////////////////////////////////
#include "../include.h"

///////////////////////////////////// Constant Definitions ///////////////////////////////////////////

/////////////////////////////////////// Type Definitions /////////////////////////////////////////////

///////////////////////////// Macros (Inline Functions) Definitions //////////////////////////////////

///////////////////////////////////// Function Prototypes ////////////////////////////////////////////

///////////////////////////////////// Variable Definitions ///////////////////////////////////////////

///////////////////////////////////// Function implements ////////////////////////////////////////////

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Initialize a ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	pvBuffer is the data storage area
//	@param	:	uiBufferSize is the length of buffer or the number of elements in the buffer
//	@param	:	uiElementSize is the size in byte of each elements in the buffer
//	@param	:	ucMutexPrio is the priority of the mutex for synchronizing multi-access to the buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferInit(SRingBuffer_t* psRingBuffer, 
				void* pvBuffer, 
				uint16_t uiBufferSize, 
				uint16_t uiElementSize, 
				CallbackFunctionIO callbackLock, 
				CallbackFunctionIO callbackUnlock, 
				void* pvCallbackParam)
{
	// Initialize data for the Ring buffer structure
	psRingBuffer->pvBuffer			= pvBuffer;
	psRingBuffer->uiBufferSize		= uiBufferSize;
	psRingBuffer->uiElementSize		= uiElementSize;
	
	psRingBuffer->uiElementCount 	= 0;
	psRingBuffer->uiBufferPopPtr	= 0;
	psRingBuffer->uiBufferPushPtr	= 0;
	
	psRingBuffer->uiBKElementCount 	= 0;
	psRingBuffer->uiBKBufferPopPtr	= 0;
	psRingBuffer->uiBKBufferPushPtr	= 0;
	
	psRingBuffer->bBufferPopEnable	= TRUE;
	psRingBuffer->bBufferPushEnable	= TRUE;
	
	psRingBuffer->callbackLock		= callbackLock;
	psRingBuffer->callbackUnlock	= callbackUnlock;
	psRingBuffer->pvCallbackParam	= pvCallbackParam;
}	

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Push a stream into ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	pvStream is the data stream to be pushed into the buffer
//	@param	:	uiLength is the length of data stream
//	@return	: 	TRUE if pushing successfully and vice versa
//	@note	:
// ---------------------------------------------------------------------------------------------------
BOOL BufferPushStream(SRingBuffer_t* psRingBuffer, void* pvStream, uint16_t uiLength)
{
	uint8_t*	pvBuffer		= NULL;
	uint8_t*	pvRestStream	= NULL;

	if (((psRingBuffer->uiElementCount + uiLength) > psRingBuffer->uiBufferSize) || (psRingBuffer->bBufferPushEnable == FALSE))
	{
		return FALSE;
	}
	
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	// Calculate the start address for pushing in
	pvBuffer = (uint8_t*)psRingBuffer->pvBuffer + psRingBuffer->uiBufferPushPtr * psRingBuffer->uiElementSize;

	// If the pushing address is out of address range of the buffer then we need to push twice
	if ((psRingBuffer->uiBufferPushPtr + uiLength) > psRingBuffer->uiBufferSize)
	{
		// Push data stream from start address to the final address of the buffer
		memcpy_secure(pvBuffer, pvStream, psRingBuffer->uiElementSize * (psRingBuffer->uiBufferSize - psRingBuffer->uiBufferPushPtr));

		// Return the start address to the first address of the buffer for pushing the rest of stream
		pvBuffer = (uint8_t*)psRingBuffer->pvBuffer;

		// Point to address of the rest of stream
		pvRestStream = (uint8_t*)pvStream + (psRingBuffer->uiBufferSize - psRingBuffer->uiBufferPushPtr) * psRingBuffer->uiElementSize;

		// Push the rest of stream
		memcpy_secure(pvBuffer, pvRestStream, psRingBuffer->uiElementSize * (uiLength + psRingBuffer->uiBufferPushPtr - psRingBuffer->uiBufferSize));
	}
	else
	{
		// Push data stream
		memcpy_secure(pvBuffer, pvStream, psRingBuffer->uiElementSize * uiLength);
	}

	// Point the push pointer to the new position
	psRingBuffer->uiBufferPushPtr += uiLength;
	if (psRingBuffer->uiBufferPushPtr >= psRingBuffer->uiBufferSize)
	{
		psRingBuffer->uiBufferPushPtr -= psRingBuffer->uiBufferSize;
	}

	// Increase element count of the buffer
	psRingBuffer->uiElementCount += uiLength;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}

	return TRUE;	// Push successfully
}	

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Pop a stream from ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	pvStream is the data stream to be popped out from the buffer
//	@param	:	uiLength is the length of data stream
//	@return	: 	The number of elements popped out actually
//	@note	:
// ---------------------------------------------------------------------------------------------------
uint16_t BufferPopStream(SRingBuffer_t* psRingBuffer, void* pvStream, uint16_t uiLength)
{
	uint16_t	uiPopCount		= 0;
	uint8_t*	pvBuffer		= NULL;
	uint8_t*	pvRestStream	= NULL;

	if ((psRingBuffer->uiElementCount == 0) || (psRingBuffer->bBufferPopEnable == FALSE))
	{
		return 0;
	}
	
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	// Limit length of data stream will be popped
	if (uiLength < psRingBuffer->uiElementCount)
	{
		uiPopCount = uiLength;
	}
	else
	{
		uiPopCount = psRingBuffer->uiElementCount;
	}

	// Calculate the start address for popping out
	pvBuffer = (uint8_t*)psRingBuffer->pvBuffer + psRingBuffer->uiBufferPopPtr * psRingBuffer->uiElementSize;

	// If the popping address is out of address range of the buffer then we need to pop twice
	if ((psRingBuffer->uiBufferPopPtr + uiPopCount) > psRingBuffer->uiBufferSize)
	{
		// Pop data stream from start address to the final address of the buffer
		memcpy_secure(pvStream, pvBuffer, psRingBuffer->uiElementSize * (psRingBuffer->uiBufferSize - psRingBuffer->uiBufferPopPtr));

		// Return the start address to the first address of the buffer for popping the rest of stream
		pvBuffer = (uint8_t*)psRingBuffer->pvBuffer;

		// Point to address of the rest of stream
		pvRestStream = (uint8_t*)pvStream + (psRingBuffer->uiBufferSize - psRingBuffer->uiBufferPopPtr) * psRingBuffer->uiElementSize;

		// Pop the rest of stream
		memcpy_secure(pvRestStream, pvBuffer, psRingBuffer->uiElementSize * (uiPopCount + psRingBuffer->uiBufferPopPtr - psRingBuffer->uiBufferSize));
	}
	else
	{
		// Pop data stream
		memcpy_secure(pvStream, pvBuffer, psRingBuffer->uiElementSize * uiPopCount);
	}

	// Point the pop pointer to the new position
	psRingBuffer->uiBufferPopPtr += uiPopCount;
	if (psRingBuffer->uiBufferPopPtr >= psRingBuffer->uiBufferSize)
	{
		psRingBuffer->uiBufferPopPtr -= psRingBuffer->uiBufferSize;
	}

	// Decrease element count of the buffer
	psRingBuffer->uiElementCount -=uiPopCount;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}

	return uiPopCount;	// Return the number of elements popped out actually
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Push a data element into ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	vData is the data to be pushed into the buffer
//	@return	: 	TRUE if pushing successfully and vice versa
//	@note	:
// ---------------------------------------------------------------------------------------------------
BOOL BufferPush(SRingBuffer_t* psRingBuffer, void* pvData)
{
	uint8_t*	pvBuffer	= NULL;

	if ((psRingBuffer->uiElementCount >= psRingBuffer->uiBufferSize) || (psRingBuffer->bBufferPushEnable == FALSE))
	{
		return FALSE;
	}
	
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}

	// Calculate the start address for pushing in
	pvBuffer = (uint8_t*)psRingBuffer->pvBuffer + psRingBuffer->uiBufferPushPtr * psRingBuffer->uiElementSize;

	// Push data element
	memcpy_secure(pvBuffer, pvData, psRingBuffer->uiElementSize);

	// Point the push pointer to the new position
	psRingBuffer->uiBufferPushPtr++;
	if (psRingBuffer->uiBufferPushPtr >= psRingBuffer->uiBufferSize)
	{
		psRingBuffer->uiBufferPushPtr = 0;
	}

	// Increase element count of the buffer
	psRingBuffer->uiElementCount++;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Pop out a data element from ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	pvData is the data is popped out of the buffer
//	@return	: 	The number of elements popped out actually
//	@note	:
// ---------------------------------------------------------------------------------------------------
uint16_t BufferPop(SRingBuffer_t* psRingBuffer, void* pvData)
{
	uint8_t*	pvBuffer	= NULL;

	if ((psRingBuffer->uiElementCount == 0) || (psRingBuffer->bBufferPopEnable == FALSE))
	{
		return 0;
	}
	
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}

	// Calculate the start address for popping out
	pvBuffer = (uint8_t*)psRingBuffer->pvBuffer + psRingBuffer->uiBufferPopPtr * psRingBuffer->uiElementSize;

	// Pop data element
	memcpy_secure(pvData, pvBuffer, psRingBuffer->uiElementSize);

	// Point the pop pointer to the new position
	psRingBuffer->uiBufferPopPtr++;
	if (psRingBuffer->uiBufferPopPtr >= psRingBuffer->uiBufferSize)
	{
		psRingBuffer->uiBufferPopPtr = 0;
	}

	// Decrease element count of the buffer
	psRingBuffer->uiElementCount--;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}

	return 1;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Push back some element to the buffer. This activity will increase element count and 
//				decrease the pop pointer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@param	: 	uiPushBackNumber is the push back number
//	@return	: 	TRUE if pushing successfully and vice versa
//	@note	:
// ---------------------------------------------------------------------------------------------------
BOOL BufferPushBack(SRingBuffer_t* psRingBuffer, uint16_t uiPushBackNumber)
{
	// Cannot push back if the push back number is greater than the available element count
	if (uiPushBackNumber > (psRingBuffer->uiBufferSize - psRingBuffer->uiElementCount))
	{
		return FALSE;
	}

	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}

	// Disable pushing to prohibiting changing of push pointer
	psRingBuffer->bBufferPushEnable = FALSE;
	(void)psRingBuffer->bBufferPushEnable;
	// Push back element count
	psRingBuffer->uiElementCount += uiPushBackNumber;

	// Push back pop pointer
	if (psRingBuffer->uiBufferPopPtr >= uiPushBackNumber)
	{
		psRingBuffer->uiBufferPopPtr -= uiPushBackNumber;
	}
	else
	{
		psRingBuffer->uiBufferPopPtr = psRingBuffer->uiBufferPopPtr + psRingBuffer->uiBufferSize - uiPushBackNumber;
	}

	// Enable pushing
	psRingBuffer->bBufferPushEnable = TRUE;
	
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Check if the ring buffer is enable to be pushed in
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	TRUE if it's enable and vice versa
//	@note	:
// ---------------------------------------------------------------------------------------------------
BOOL BufferIsPushEnable(SRingBuffer_t* psRingBuffer)
{
	if (psRingBuffer->uiElementCount >= psRingBuffer->uiBufferSize)
	{
		return FALSE;
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Check if the ring buffer is enable to be popped out
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	TRUE if it's enable and vice versa
//	@note	:
// ---------------------------------------------------------------------------------------------------
BOOL BufferIsPopEnable(SRingBuffer_t* psRingBuffer)
{
	if (psRingBuffer->uiElementCount == 0)
	{
		return FALSE;
	}

	return TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Enable popping data out from the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferEnablePop(SRingBuffer_t* psRingBuffer)
{
	psRingBuffer->bBufferPopEnable = TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Disable popping data out from the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferDisablePop(SRingBuffer_t* psRingBuffer)
{
	psRingBuffer->bBufferPopEnable = FALSE;
}	

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Enable pushing data into the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferEnablePush(SRingBuffer_t* psRingBuffer)
{
	psRingBuffer->bBufferPushEnable = TRUE;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Disable pushing data into the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferDisablePush(SRingBuffer_t* psRingBuffer)
{
	psRingBuffer->bBufferPushEnable = FALSE;
}	

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Get the element count of the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Element count of the buffer
//	@note	:
// ---------------------------------------------------------------------------------------------------
uint16_t BufferGetCount(SRingBuffer_t* psRingBuffer)
{
	return psRingBuffer->uiElementCount;
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Get the available (free or can be used for pushing) element count of the buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Available element count of the buffer
//	@note	:
// ---------------------------------------------------------------------------------------------------
uint16_t BufferGetAvailableCount(SRingBuffer_t* psRingBuffer)
{
	uint16_t	uiBufferAvailableCount	= 0;

	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	uiBufferAvailableCount = psRingBuffer->uiBufferSize - psRingBuffer->uiElementCount;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}
	
	return (uiBufferAvailableCount);
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Save the current state of the ring buffer
//		  
//	@param	: 	None
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferSaveState(SRingBuffer_t* psRingBuffer)
{
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	psRingBuffer->uiBKElementCount	= psRingBuffer->uiElementCount;
	psRingBuffer->uiBKBufferPopPtr	= psRingBuffer->uiBufferPopPtr;
	psRingBuffer->uiBKBufferPushPtr	= psRingBuffer->uiBufferPushPtr;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Restore the ring buffer to the last saved state
//		  
//	@param	: 	None
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferRestoreState(SRingBuffer_t* psRingBuffer)
{
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	psRingBuffer->uiElementCount	= psRingBuffer->uiBKElementCount;
	psRingBuffer->uiBufferPopPtr	= psRingBuffer->uiBKBufferPopPtr;
	psRingBuffer->uiBufferPushPtr	= psRingBuffer->uiBKBufferPushPtr;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}
}

// ---------------------------------------------------------------------------------------------------
//	@brief	: 	Flush the Ring buffer
//		  
//	@param	: 	psRingBuffer is the structure of the ring buffer
//	@return	: 	Void
//	@note	:
// ---------------------------------------------------------------------------------------------------
void BufferFlush(SRingBuffer_t* psRingBuffer)
{
	// Lock accessing to the buffer
	if (psRingBuffer->callbackLock)
	{
		psRingBuffer->callbackLock(psRingBuffer->pvCallbackParam);
	}
	
	psRingBuffer->uiElementCount 	= 0;
	psRingBuffer->uiBufferPopPtr	= 0;	
	psRingBuffer->uiBufferPushPtr	= 0;    
	psRingBuffer->bBufferPopEnable	= TRUE;
	psRingBuffer->bBufferPushEnable = TRUE;
	
	// Unlock accessing to the buffer
	if (psRingBuffer->callbackUnlock)
	{
		psRingBuffer->callbackUnlock(psRingBuffer->pvCallbackParam);
	}
}
