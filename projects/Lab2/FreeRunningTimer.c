/*
 * File:   FreeRunningTimer.c
 * Author: Robert Box
 *
 * Created on February 4, 2022, 11:00 PM
 */

#include "FreeRunningTimer.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "Protocol.h"
#include <stdio.h>

unsigned int milliSecond = 0;
unsigned int microSecond = 0;

/**
 * @Function FreeRunningTimer_Init(void)
 * @param none
 * @return None.
 * @brief  Initializes the timer module */
void FreeRunningTimer_Init(void) {
    T5CON = 0x0; // clear control registers
    TMR5 = 0x0; // clear time register
    
    PR5 = 0x270F; // load period register with dec-10000 -1
    T5CONbits.TCKPS = 0b010; // preset 4
    
    IPC5SET = 0x00000004; // priority 1
    IPC5SET = 0x00000001; // sub priority 1
    
    IFS0CLR = 0x00100000; // clear Timer5 interrupt flag
    IEC0SET = 0x00100000; // Enable Timer5 interrupt
    
    T5CONbits.ON = 1; // Start timer
}

/**
 * Function: FreeRunningTimer_GetMilliSeconds
 * @param None
 * @return the current MilliSecond Count
   */
unsigned int FreeRunningTimer_GetMilliSeconds(void) {
    return milliSecond;
}

/**
 * Function: FreeRunningTimer_GetMicroSeconds
 * @param None
 * @return the current MicroSecond Count
   */
unsigned int FreeRunningTimer_GetMicroSeconds(void) {
    return microSecond + TMR5/10;
}


void __ISR(_TIMER_5_VECTOR, ipl3auto) Timer5IntHandler(void) {
    IFS0CLR = 0x00100000; // clear Timer5 interrupt flag
    milliSecond += 1;
    microSecond += 1000;
    TMR5 = 0x0;
}

int main() {
    
}


//#define testHarness
#ifdef testHarness
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();
    FreeRunningTimer_Init();
    
    char leds = 0x01;
    char debugMessage[MAXPAYLOADLENGTH];
    char timerMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Protocol Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    while(1) {
        if (FreeRunningTimer_GetMilliSeconds()%2000 == 0) {
            sprintf(timerMessage, "Milliseconds %u Microseconds %u", milliSecond, microSecond);
            Protocol_SendDebugMessage(timerMessage);
            LEDS_SET(leds);
            leds = leds ^ 0x01; // flash led
        }
    }
}
#endif