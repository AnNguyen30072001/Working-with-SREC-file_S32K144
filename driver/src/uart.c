#include "../../driver/inc/uart.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "device_registers.h"
#include "clock_manager.h"
#include "interrupt_manager.h"

LPUART_Type * const g_lpuartBase[LPUART_INSTANCE_COUNT] = LPUART_BASE_PTRS;
clock_names_t g_lpuartClkNames[LPUART_INSTANCE_COUNT] = LPUART_CLOCK_NAMES;
IRQn_Type g_lpuartRxTxIrqId[LPUART_INSTANCE_COUNT] = LPUART_RX_TX_IRQS;

LPUART_Type * const myLpuartBase[LPUART_INSTANCE_COUNT] = LPUART_BASE_PTRS;
static UartReceiveCallback uartCallback = NULL;

uint8_t UART_SetParityMode(uint8_t instance, uint32_t parityMode)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->CTRL |= LPUART_CTRL_PE(parityMode >> 1) | LPUART_CTRL_PT(parityMode % 2 );
    return 0;
}

uint8_t UART_SetStopBit(uint8_t instance, uint32_t stopBit)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->BAUD |= LPUART_BAUD_SBNS(stopBit);
    return 0;
}

uint8_t UART_SetBitCountPerChar(uint8_t instance, uint32_t bitCountPerChar, bool parity)
{
    LPUART_Type * base = g_lpuartBase[instance];
    if (parity && bitCountPerChar < (uint32_t)0x02U){
        bitCountPerChar++;
    }
    if (bitCountPerChar == (uint32_t)0x2U){
        base->BAUD |= LPUART_BAUD_M10(1);
    }else{
    if(bitCountPerChar == (uint32_t)0x0u){
        base->CTRL &=  ~LPUART_CTRL_M(1);
    }
    if (bitCountPerChar == (uint32_t)0x1U){
        base->CTRL |=  LPUART_CTRL_M(1);

    }
    base->BAUD &= ~LPUART_BAUD_M10_MASK;
    }
    return 0;
}

uint8_t UART_SetBaudrate(uint8_t instance, uint32_t baudRate)
{
    uint32_t uartSourceClock;
    clock_names_t instanceClkName = g_lpuartClkNames[instance];
    LPUART_Type * base = g_lpuartBase[instance];
    (void)CLOCK_SYS_GetFreq(instanceClkName, &uartSourceClock);
    if (uartSourceClock <= 0U){
        return 0x00U;
    }
    if (uartSourceClock < (baudRate * 4U)){
        return 0x00U;
    }
    uint16_t sbr;
    uint32_t osr;
    uint16_t sbrTemp, i;
    uint32_t tempDiff, calculatedBaud, baudDiff;
    osr = 4;
    sbr = (uint16_t)(uartSourceClock / (baudRate * osr));
    calculatedBaud = (uartSourceClock / (osr * sbr));

    if (calculatedBaud > baudRate){
        baudDiff = calculatedBaud - baudRate;
    }
    else{
        baudDiff = baudRate - calculatedBaud;
    }
    for (i = 5U; i <= 32U; i++){
        sbrTemp = (uint16_t)(uartSourceClock / (baudRate * i));
        calculatedBaud = (uint32_t)(uartSourceClock / (i * sbrTemp));
        if (calculatedBaud > baudRate){
            tempDiff = calculatedBaud - baudRate;
        }
        else{
            tempDiff = baudRate - calculatedBaud;
        }
        if (tempDiff <= baudDiff){
            baudDiff = tempDiff;
            osr = i;  /* update and store the best osr value calculated */
            sbr = sbrTemp;  /* update store the best sbr value calculated */
        }
    }
    if (osr>=4 && osr <= 7){
        base->BAUD |= LPUART_BAUD_BOTHEDGE(1);
    }
    else{
        base->BAUD &= ~LPUART_BAUD_BOTHEDGE(1);
    }
    uint32_t temp = LPUART_BAUD_SBR(sbr)|LPUART_BAUD_OSR(osr - 1);
    base->BAUD = temp;
    return 0;
}

uint8_t UART_Init(uint8_t instance, uart_config_t * uartConfig)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->BAUD = LPUART_BAUD_SBR(4)|LPUART_BAUD_OSR(15);
    base->STAT = 0x0U;
    base->CTRL = 0x0U;
    base->MATCH = 0x0U;

    UART_SetBaudrate(instance, uartConfig->baudRate);
    UART_SetBitCountPerChar(instance, uartConfig->bitCountPerChar, uartConfig->parityMode);
    UART_SetParityMode(instance, uartConfig->parityMode);
    UART_SetStopBit(instance, uartConfig->stopBitCount);
//    INT_SYS_EnableIRQ(33);
    INT_SYS_EnableIRQ(g_lpuartRxTxIrqId[instance]);
    return 0;
}

uint8_t UART_SendDataPolling(uint8_t instance, const uint8_t *txBuff)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->CTRL |= LPUART_CTRL_TE(1);
    while((base->CTRL & LPUART_CTRL_TE_MASK) != LPUART_CTRL_TE_MASK) {}
    int i = 0;
    while (txBuff[i] != '\0')
    {
        while((base->STAT & LPUART_STAT_TDRE_MASK)>>LPUART_STAT_TDRE_SHIFT==0);
        base->DATA = txBuff[i];
        i++;
    }
    base->CTRL &= ~LPUART_CTRL_TE(1);
    while((base->CTRL & LPUART_CTRL_TE_MASK) == LPUART_CTRL_TE_MASK) {}

    return 0;
}

uint8_t UART_ReceiveDataPolling(uint8_t instance, uint8_t *rxBuff, uint32_t rxSize)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->CTRL = (base->CTRL & ~LPUART_CTRL_RE_MASK) | (1UL << LPUART_CTRL_RE_SHIFT);
    while((base->CTRL & LPUART_CTRL_RE_MASK) != LPUART_CTRL_RE_MASK) {}
    while (rxSize > 0U)
    {
        while ((base->STAT & LPUART_STAT_RDRF_MASK) ==  0)
        {}

        *rxBuff = (uint8_t)base->DATA;
        ++rxBuff;
        --rxSize;
    }
    base->CTRL = (base->CTRL & ~LPUART_CTRL_RE_MASK) | (0UL << LPUART_CTRL_RE_SHIFT);
    while((base->CTRL & LPUART_CTRL_RE_MASK) == LPUART_CTRL_RE_MASK) {}

    return 0;
}

uint8_t UART_ReceiveDataInterrupt(uint8_t instance)
{
    LPUART_Type * base = g_lpuartBase[instance];
    base->CTRL = (base->CTRL & ~LPUART_CTRL_RE_MASK) | (1UL << LPUART_CTRL_RE_SHIFT);
    while((base->CTRL & LPUART_CTRL_RE_MASK) != LPUART_CTRL_RE_MASK) {}
    base->CTRL |= LPUART_CTRL_RIE_MASK;

    return 0;
}

void RegisterUartReceiveCallback(UartReceiveCallback callback) {
    uartCallback = callback;
}

/* LPUART1_RxTx_IRQn = 33u */
void LPUART1_RxTx_IRQHandler(){
	S32_NVIC->ICPR[1] = 1 << ((uint32_t)33 & 31); // clear interrupt pending
	uint8_t rxBuff = (uint8_t)myLpuartBase[1]->DATA;
	if(uartCallback != NULL){
		uartCallback(rxBuff);
	}
}

void LPUART0_RxTx_IRQHandler(){
	S32_NVIC->ICPR[0] = 1 << ((uint32_t)31 & 31); // clear interrupt pending
	uint8_t rxBuff = (uint8_t)myLpuartBase[0]->DATA;
	if(uartCallback != NULL){
		uartCallback(rxBuff);
	}
}
