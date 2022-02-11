

#include "PingSensor.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "Protocol.h"
#include <stdio.h>
#include "FreeRunningTimer.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/
#define TRIGGERPIN LATDbits.LATD0 // pin 3
#define INPUTCAPTURE LATDbits.LATD10 // pin 8

unsigned int t = 0;
unsigned short upgoing = 0;
unsigned short downgoing = 0;
unsigned short difference = 0;

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
    
    TRISDbits.TRISD0 = 0; // output
    TRISDbits.TRISD10 = 1; // input
    TRIGGERPIN = 0;
    
    
    
    T2CON = 0x0; // clear control registers
    TMR2 = 0x0; // clear time register
    PR2 = 0xFFFF; // max load period > 11.68 ms
    T2CONbits.TCKPS = 0b011; // preset 8
    IPC2SET = 0x0000000C; // priority 3
    IPC2SET = 0x00000001; // sub priority 1    
    IFS0CLR = 0x00000100; // clear Timer2 interrupt flag
    IEC0SET = 0x00000100; // Enable Timer2 interrupt
    
    // IC3 is pin 8
    IC3CON = 0; // clear control registers
    IC3CONbits.ICM = 0b001; // trigger on every edge
    IC3CONbits.FEDGE = 1; // first trigger rising edge
    IC3CONbits.ICI = 0b00; // interrupt on every edge
    IC3CONbits.ICTMR = 1; // Timer2 selected
    IFS0bits.IC3IF = 0; // clear interrupt flag
    IPC3bits.IC3IP = 4; // IC3 interrupt priority
    IEC0bits.IC3IE = 1; // Enable interrupt
    IC3CONbits.ON = 1;// turn IC3 on
    
    T2CONbits.ON = 1; // Start timer
    
}

/**
 * @Function int PingSensor_GetDistance(void)
 * @param None
 * @return Unsigned Short corresponding to distance in millimeters */
unsigned short PingSensor_GetDistance(void);


void __ISR(_TIMER_4_VECTOR) Timer4IntHandler(void) {
    IFS0CLR = 0x00010000; // clear Timer4 interrupt flag
    TRIGGERPIN = 1;
    t = FreeRunningTimer_GetMicroSeconds();
    while(1) { // 10 microseconds
        if (FreeRunningTimer_GetMicroSeconds() > (t+10)) {
            break;
        }
    }
    TRIGGERPIN = 0;
}

void __ISR(_INPUT_CAPTURE_3_VECTOR) __IC3Interrupt(void){
    IFS0bits.IC3IF = 0; // clear interrupt flag
    if (TRIGGERPIN == 1) {
        upgoing = (0xFFFF & IC3BUF);
    } else if(TRIGGERPIN == 0) {
        downgoing = (0xFFFF & IC3BUF);
        difference = downgoing - upgoing;
    }

    int x = 5;
}

void __ISR(_TIMER_2_VECTOR) Timer2IntHandler(void) {
    IFS0CLR = 0x00000100; // clear Timer2 interrupt flag

}

//#define test
#ifdef test
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    PingSensor_Init();
    while(1);
}
#endif
