#ifndef LPIT_H
#define LPIT_H
#include <stdint.h>
#include <stdbool.h>

#define LPIT_CLOCK	48000000
#define USER_CORE_CLOCK	48000000
#define PERIOD_CNT_MODE 0 /* Periodic counter mode */
#define MAX_PERIOD_COUNT 0xFFFFFFFF     /* For checking overflow */
#define SUCCESS 0
#define ERROR 1

typedef void (*lpitTimerCallback)(void);
void RegisterLpitTimerCallback(lpitTimerCallback callback);

void LPIT_Init();
void LPIT_Deinit();
uint8_t LPIT_ConfigChannel(uint8_t channel, bool isInterruptEnabled);
void timerCheck(uint32_t milliSec);
void LPIT0_Ch0_IRQHandler();

#endif
