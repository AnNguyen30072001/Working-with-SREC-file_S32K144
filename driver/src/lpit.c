#include "../../driver/inc/lpit.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "device_registers.h"
#include "clock_manager.h"
#include "interrupt_manager.h"

const IRQn_Type lpitIrqId[] = LPIT_IRQS;
static volatile uint8_t delayFlag;

static lpitTimerCallback lpitCallback = NULL;

void LPIT_Init(void)
{
    uint32_t delay;
    SCG->RCCR |= SCG_RCCR_DIVCORE(0);
    SCG->FIRCDIV |= SCG_FIRCDIV_FIRCDIV2(1);
    PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_PCS(3);
    PCC->PCCn[PCC_LPIT_INDEX] |= PCC_PCCn_CGC_MASK;
     /* Reset LPIT module */
    // Set bit RST in MCR register of LPIT0 by manipulating LPIT0->MCR
    LPIT0->MCR |= LPIT_MCR_SW_RST_MASK;
    // Calculate 4 LPIT's clock cycles by core's clock
    delay = (USER_CORE_CLOCK / LPIT_CLOCK) << 2;
    // Delay 4 peripheral clock cycles
    while(delay != 0){
    	delay--;
    }
    // Clear bit RST in MCR register of LPIT0
    LPIT0->MCR &= ~(1 << LPIT_MCR_SW_RST_SHIFT);
    /* Enable LPIT module */
    // Set M_CEN bit in MCR register of LPIT0
    LPIT0->MCR |= LPIT_MCR_M_CEN_MASK;
    // Delay 4 peripheral clock cycles
    while(delay != 0)
        delay--;
}

void LPIT_Deinit(void)
{
    // Clear M_CEN bit in MCR register of LPIT0
    LPIT0->MCR &= ~(1 << LPIT_MCR_M_CEN_SHIFT);
}

uint8_t LPIT_ConfigChannel(uint8_t channel, bool isInterruptEnabled){
	if(channel > (LPIT_TMR_COUNT-1))
	    {
	        return ERROR;
	    }
	/* Setup 32-bit periodic counter mode */
	// Clear TCTRL register's MODE field of the corresponding
	// timer channel
	LPIT0->TMR[channel].TCTRL &=  ~LPIT_TMR_TCTRL_MODE_MASK;
	LPIT0->TMR[channel].TCTRL |=  LPIT_TMR_TCTRL_MODE(PERIOD_CNT_MODE);

	if (isInterruptEnabled){
		/* Enable interrupt */
		 INT_SYS_EnableIRQ(lpitIrqId[channel]);
		// Set the corresponding TIE bit in MIER register of LPIT0
		LPIT0->MIER |= (1U << channel);
	}
	else{
		/* Disable interrupt */
		INT_SYS_DisableIRQ(lpitIrqId[channel]);
		// Clear the corresponding TIE bit in MIER register  of LPIT0
		LPIT0->MIER &= ~(1U << channel);
	}

	return SUCCESS;
}

void timerCheck(uint32_t milliSec){
	delayFlag = 1;
	uint64_t count = 0;
	// Convert from milisecond to number of counter
	count = (uint64_t)milliSec * (LPIT_CLOCK / 1000) / 6;
	if(count-1 > MAX_PERIOD_COUNT)
	{
		return;
	}
	LPIT_Init();
	/* Configure channel 0, enable interrupt */
	LPIT_ConfigChannel(0, true);
	/* LPIT in 32-bit periodic counter mode */
	// Assign (count - 1) to TVAL register of channel 0
	LPIT0->TMR[0].TVAL = count-1;
	// Enable channel 0 by setting corresponding bit in SETTEN register
	LPIT0->SETTEN |= LPIT_SETTEN_SET_T_EN_0_MASK;
}

void RegisterLpitTimerCallback(lpitTimerCallback callback){
	lpitCallback = callback;
}

void LPIT0_Ch0_IRQHandler(){
	LPIT0->MSR |= LPIT_MSR_TIF0_MASK; // clear interrupt flag of channel 0
	LPIT0->CLRTEN |=  LPIT_CLRTEN_CLR_T_EN_0_MASK;
	if(lpitCallback != NULL){
		lpitCallback();
	}
}


