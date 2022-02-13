
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#include "RCServo.h"
#include "BOARD.h"
#include <stdio.h>
#include <sys/attribs.h>
#include "FreeRunningTimer.h"
#include "Protocol.h"


#define RC_SERVO_MIN_PULSE 600
#define RC_SERVO_CENTER_PULSE 1500
#define RC_SERVO_MAX_PULSE 2400

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
#define TRIGGER LATFbits.LATF1 // pin 4
unsigned int j;

/**
 * @Function RCServo_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes hardware required and set it to the CENTER PULSE */
int RCServo_Init(void) {
    T3CON = 0x0; // clear control registers
    TMR3 = 0x0; // clear time register
    PR3 = 0x7A12; // load period register with 31250 for 50ms
    T3CONbits.TCKPS = 0b110; // preset 64
    IPC3bits.T3IP = 1; // priority 2
    IPC3bits.T3IS = 1; // sub priority 2
    IFS0CLR = 0x00001000; // clear Timer3 interrupt flag
    IEC0bits.T3IE = 1; // Enable Timer3 interrupt
    T3CONbits.ON = 1; // Start timer
    

    TRISFbits.TRISF1 = 0; // output
    TRIGGER = 0;
}

/**
 * @Function int RCServo_SetPulse(unsigned int inPulse)
 * @param inPulse, integer representing number of microseconds
 * @return SUCCESS or ERROR
 * @brief takes in microsecond count, converts to ticks and updates the internal variables
 * @warning This will update the timing for the next pulse, not the current one */
int RCServo_SetPulse(unsigned int inPulse);

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void);

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void);


void __ISR(_TIMER_3_VECTOR) Timer3IntHandler(void) {
    IFS0CLR = 0x00001000; // clear Timer3 interrupt flag
    TRIGGER = 1;
    for (j = 0; j < 12; j++) { // 10 microseconds
        asm("nop");
    }
    TRIGGER = 0;
}

#define timer
#ifdef timer
int main() {
    BOARD_Init();
    //FreeRunningTimer_Init();
    //Protocol_Init();
    RCServo_Init();
    while(1);
}
#endif
