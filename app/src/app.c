#include <stdint.h>
#include <stdbool.h>
#include "interrupt_manager.h"
#include "clock_manager.h"
#include "sdk_project_config.h"
#include "app.h"

const uint8_t UART_INSTANCE = 0;
bool receiveError = false;

uart_config_t myUART={
		.transferType = UART_USING_INTERRUPTS,
		.baudRate = 115200,
		.parityMode = UART_PARITY_DISABLED,
		.stopBitCount = UART_ONE_STOP_BIT,
		.bitCountPerChar = UART_8_BITS_PER_CHAR
};

queue_t myQueue = {
		.front = 0,
		.size = 0,
		.capacity = QUEUE_SIZE,
		.rear = 0,
};

void delay(volatile int cycles){
	while(cycles--);
}

void uartCallbackHandler(uint8_t data){
	push(&myQueue, data);
	if(data == 10){		// '\n'
		enqueue(&myQueue);
	}
}

void lpitCallbackHandler(){
	if(!terminationCorrect()){
		receiveError = true;
	}
}

int appMain(void)
{
	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                 g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
    CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

    PCC->PCCn[PCC_PORTA_INDEX] |= PCC_PCCn_CGC_MASK; /* Enable clock for PORTA */
    PORTA->PCR[2] |= PORT_PCR_MUX(6); /* Port A2: MUX = ALT2,UART0 RX */
    PORTA->PCR[3] |= PORT_PCR_MUX(6); /* Port A3: MUX = ALT2,UART0 TX */

    /* if choosing UART1, use this code
    PCC->PCCn[PCC_PORTC_INDEX] |= PCC_PCCn_CGC_MASK; // Enable clock for PORTC
	PORTC->PCR[6] |= PORT_PCR_MUX(2); // Port C6: MUX = UART1 RX
	PORTC->PCR[7] |= PORT_PCR_MUX(2); // Port C7: MUX = UART1 TX
	*/

	UART_Init(UART_INSTANCE, &myUART);
	UART_ReceiveDataInterrupt(UART_INSTANCE);
	RegisterUartReceiveCallback(&uartCallbackHandler);
	RegisterLpitTimerCallback(&lpitCallbackHandler);

	uint8_t feedback_length;
	uint8_t srec_status;
	bool timer_started = false;

	while(1){
		if(receiveError == true){
			UART_SendDataPolling(UART_INSTANCE, (uint8_t *)SREC_RECEIVE_ERROR_MSG);
			return STATUS_ERROR;
		}
		if(!isEmpty(&myQueue))
		{
			srec_status = checkFrameFormat(myQueue.array[myQueue.front]);
			if(srec_status == frame_Error_Format){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)FORMAT_ERROR_MSG);
				return STATUS_ERROR;
			}
			if(srec_status == frame_Error_Byte_Count){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)BYTECOUNT_ERROR_MSG);
				return STATUS_ERROR;
			}
			if(srec_status == frame_Error_Record_type){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)RECORD_TYPE_ERROR_MSG);
				return STATUS_ERROR;
			}
			if(srec_status == frame_Error_Checksum){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)CHECKSUM_ERROR_MSG);
				return STATUS_ERROR;
			}
			if(srec_status == frame_Error_Record_Count){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)RECORD_COUNT_ERROR_MSG);
				return STATUS_ERROR;
			}
			if(srec_status == frame_Error_Terminate_Type){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)TERMINATION_ERROR_MSG);
				return STATUS_ERROR;
			}
			feedback_length = findAddrLength() + findDataLength();
			UART_SendDataPolling(UART_INSTANCE, (uint8_t *)(peek(&myQueue, feedback_length)));
			dequeue(&myQueue);
			if(timer_started == false){
				timerCheck(5000);
				timer_started = true;
			}
		}
		if(terminationCorrect()){
			delay(720000);
			if(isEmpty(&myQueue)){
				UART_SendDataPolling(UART_INSTANCE, (uint8_t *)SREC_RECEIVE_MSG);
				break;
			}
		}
	}
    return STATUS_SUCCESS;
}
