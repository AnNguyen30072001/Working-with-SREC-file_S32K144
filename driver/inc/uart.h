#ifndef UART_H
#define UART_H
#include <stdint.h>
#include <stdio.h>
#include "device_registers.h"
#include "clock_manager.h"
#include "interrupt_manager.h"

typedef enum
{
    UART_8_BITS_PER_CHAR  = 0x0U, /*!< 8-bit data characters */
    UART_9_BITS_PER_CHAR  = 0x1U, /*!< 9-bit data characters */
    UART_10_BITS_PER_CHAR = 0x2U  /*!< 10-bit data characters */
} uart_bit_count_per_char_t;


typedef enum
{
    UART_PARITY_DISABLED = 0x0U, /*!< parity disabled */
    UART_PARITY_EVEN     = 0x2U, /*!< parity enabled, type even, bit setting: PE|PT = 10 */
    UART_PARITY_ODD      = 0x3U  /*!< parity enabled, type odd,  bit setting: PE|PT = 11 */
} uart_parity_mode_t;

typedef enum
{
    UART_ONE_STOP_BIT = 0x0U, /*!< one stop bit */
    UART_TWO_STOP_BIT = 0x1U  /*!< two stop bits */
} uart_stop_bit_count_t;

typedef enum
{
    UART_USING_DMA         = 0,    /*!< The driver will use DMA to perform UART transfer */
    UART_USING_INTERRUPTS          /*!< The driver will use interrupts to perform UART transfer */
} uart_transfer_type_t;


typedef struct
{
    uint32_t baudRate;
    uart_parity_mode_t parityMode;
    uart_stop_bit_count_t stopBitCount;
    uart_bit_count_per_char_t bitCountPerChar;
    uart_transfer_type_t transferType;
} uart_config_t;

uint8_t UART_SetParityMode(uint8_t instance, uint32_t parityMode);
uint8_t UART_SetStopBit(uint8_t instance, uint32_t stopBit);
uint8_t UART_SetBitCountPerChar(uint8_t instance, uint32_t bitCountPerChar, bool parity);
uint8_t UART_SetBaudrate(uint8_t instance, uint32_t baudRate);
uint8_t UART_Init(uint8_t instance, uart_config_t * uartConfig);
uint8_t UART_SendDataPolling(uint8_t instance, const uint8_t *txBuff);
uint8_t UART_ReceiveDataPolling(uint8_t instance, uint8_t *rxBuff, uint32_t rxSize);
uint8_t UART_ReceiveDataInterrupt(uint8_t instance);

typedef void (*UartReceiveCallback)(uint8_t data);
void RegisterUartReceiveCallback(UartReceiveCallback callback);
void LPUART1_RxTx_IRQHandler();
void LPUART0_RxTx_IRQHandler();



#endif
