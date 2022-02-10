

#include "PingSensor.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "Protocol.h"
#include <stdio.h>
#include "FreeRunningTimer.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/
#define TRIGGERPIN LATDbits.LATD3


/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function PingSensor_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware for PingSensor with the needed interrupts */
int PingSensor_Init(void) { // initialize Timer4 for 60 ms
    T4CON = 0x0; // clear control registers
    TMR4 = 0x0; // clear time register
    
    PR4 = 0x927B; // load period register with 37500-1 for 60ms
    T4CONbits.TCKPS = 0b110; // preset 64
    
    IPC4SET = 0x00000008; // priority 2
    IPC4SET = 0x00000001; // sub priority 1
    
    IFS0CLR = 0x00010000; // clear Timer4 interrupt flag
    IEC0SET = 0x00010000; // Enable Timer4 interrupt
    
    T4CONbits.ON = 1; // Start timer
    
    TRISDbits.TRISD3 = 0;
    TRIGGERPIN = 0;
}

/**
 * @Function int PingSensor_GetDistance(void)
 * @param None
 * @return Unsigned Short corresponding to distance in millimeters */
unsigned short PingSensor_GetDistance(void);


void __ISR(_TIMER_4_VECTOR) Timer4IntHandler(void) {
    IFS0CLR = 0x00010000; // clear Timer4 interrupt flag
    TRIGGERPIN = 1;
    unsigned int t = FreeRunningTimer_GetMicroSeconds();
    while(1) { // 10 microseconds
        if (FreeRunningTimer_GetMicroSeconds() > (t+10)) {
            break;
        }
    }
    TRIGGERPIN = 0;
}

#define test
#ifdef test
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    PingSensor_Init();
    while(1);
}
#endif