
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#include "RCServo.h"
#include "BOARD.h"
#include <stdio.h>
#include <sys/attribs.h>
#include "FreeRunningTimer.h"
#include "Protocol.h"


//preset 64
#define RC_SERVO_MIN_PULSE 625 // 1 millisecond
#define RC_SERVO_CENTER_PULSE 938 // 1.5 milliseconds
#define RC_SERVO_MAX_PULSE 1250 // 2 milliseconds

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
#define TRIGGER LATFbits.LATF1 // pin 4
#define OUTPUTCAPTURE LATDbits.LATD2 // pin 6
unsigned int j;
unsigned int tickPulse = RC_SERVO_CENTER_PULSE;
unsigned int microsPulse;

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
    

    OC3CON = 0x0; // clear control registers
    OC3CONbits.OCTSEL = 1; // select Timer3
    OC3CONbits.OCM = 0b110; // PWM, no fault pin
    OC3RS = RC_SERVO_CENTER_PULSE; // duty cycle, controlled by user
    OC3R = 900; // initial value before RS is copied in
    
    OC3CONbits.ON = 1; // turn output capture on
            
    TRISFbits.TRISF1 = 0; // output
    TRIGGER = 0;
}

/**
 * @Function int RCServo_SetPulse(unsigned int inPulse)
 * @param inPulse, integer representing number of microseconds
 * @return SUCCESS or ERROR
 * @brief takes in microsecond count, converts to ticks and updates the internal variables
 * @warning This will update the timing for the next pulse, not the current one */
int RCServo_SetPulse(unsigned int inPulse) {
    unsigned int outPulse = 0;
    microsPulse = inPulse;
    outPulse = inPulse / 8;
    outPulse = outPulse * 5; // multiplies by 0.625 to convert to ticks
    if (outPulse > RC_SERVO_MAX_PULSE) {
        outPulse = RC_SERVO_MAX_PULSE;
    }
    if (outPulse < RC_SERVO_MIN_PULSE) {
        outPulse = RC_SERVO_MIN_PULSE;
    }
    tickPulse = outPulse;
    
}

/**
 * @Function int RCServo_GetPulse(void)
 * @param None
 * @return Pulse in microseconds currently set */
unsigned int RCServo_GetPulse(void) {
    return microsPulse;
}

/**
 * @Function int RCServo_GetRawTicks(void)
 * @param None
 * @return raw timer ticks required to generate current pulse. */
unsigned int RCServo_GetRawTicks(void) {
    return tickPulse;
}


void __ISR(_TIMER_3_VECTOR) Timer3IntHandler(void) {
    IFS0CLR = 0x00001000; // clear Timer3 interrupt flag
}


#define timer
#ifdef timer
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    RCServo_Init();
    
    unsigned short Data = 0;
    unsigned short prevData = 0;
    unsigned int num = 0;
    while(1) {
        Protocol_GetPayload(&Data);
        Protocol_SendMessage(4, 0x89, &Data);

    }
}
#endif
