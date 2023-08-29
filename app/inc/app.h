#ifndef USER_H
#define USER_H

#include "../../driver/inc/uart.h"
#include "../../middleware/inc/queue.h"
#include "../../driver/inc/lpit.h"

#define FRAME_CORRECT 1
#define FRAME_ERROR 0

typedef enum{
	frame_Correct = 0u,
	frame_Error_Format = 1u,
	frame_Error_Byte_Count = 2u,
	frame_Error_Record_type = 3u,
	frame_Error_Checksum = 4u,
	frame_Error_Record_Count = 5u,
	frame_Error_Terminate_Type = 6u,
}Frame_Error_t;

#define STATUS_SUCCESS 0
#define STATUS_ERROR 1

#define FORMAT_ERROR_MSG "Wrong SREC Format!"
#define BYTECOUNT_ERROR_MSG "Wrong Byte Count!"
#define RECORD_TYPE_ERROR_MSG "Wrong record type!"
#define CHECKSUM_ERROR_MSG "Wrong Checksum!"
#define RECORD_COUNT_ERROR_MSG "Wrong Checksum!"
#define TERMINATION_ERROR_MSG "Wrong SREC termination type!"

#define SREC_RECEIVE_MSG "SREC file received successfully!"
#define SREC_RECEIVE_ERROR_MSG "Cannot receive SREC file - no termination record found!"

void uartCallbackHandler(uint8_t data);
void lpitCallbackHandler();

uint8_t checkFrameFormat(uint8_t *frame);
uint8_t isCorrectHexFormat(uint8_t* frame);
uint8_t terminationCorrect();
uint8_t findAddrLength();
uint8_t findDataLength();

void delay(volatile int cycles);

#endif
