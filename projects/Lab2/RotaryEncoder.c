
#include "RotaryEncoder.h"
#include "BOARD.h"
#include "FreeRunningTimer.h"
#include "Protocol.h"
#include <sys/attribs.h>
#include <stdio.h>
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#define ENCODER_BLOCKING_MODE 0
#define ENCODER_INTERRUPT_MODE 1

#define CS LATDbits.LATD4 // pin 10
unsigned short rData;
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/

/**
 * @Function RotaryEncoder_Init(char interfaceMode)
 * @param interfaceMode, one of the two #defines determining the interface
 * @return SUCCESS or ERROR
 * @brief initializes hardware in appropriate mode along with the needed interrupts */
int RotaryEncoder_Init(char interfaceMode) {
    int rData;

    SPI2CON = 0;        // Stops and resets the SPI2. 
    rData=SPI2BUF;        // clear receive buffer
    
    SPI2BRG = 0x11; // 40MHz/(2*3+1) = 5MHz
    SPI2STATbits.SPIROV = 0; // clear SPIROV bit
    SPI2CONbits.MSTEN = 1; // Master mode    
    SPI2CONbits.MODE16 = 1; // 16 bit mode
    SPI2CONbits.CKE = 1; // data sampled on falling edge
    SPI2CONbits.CKP = 0; // clock idle low
    SPI2CONbits.ON = 1;     // SPI ON
    
    TRISDbits.TRISD4 = 0; // output mode for CS
    CS = 1; // slave I/O pin

    // SPI2BUF = 'A'
    
}

/**
 * @Function int RotaryEncoder_ReadRawAngle(void)
 * @param None
 * @return 14-bit number representing the raw encoder angle (0-16384) */
unsigned short RotaryEncoder_ReadRawAngle(void) {
    return rData & 0b0011111111111111;
}



unsigned int parityCheck(unsigned int in) {
    unsigned int p = 0;
    for (unsigned int i = 1; i < 0x8000;) {
        if (in & i) {
            p += 1;
        }
        i = i << 1;
    }
    p = p%2;
    return p;
}

void delay(void) {
    
    unsigned int time = 0;
    unsigned int time2 = 0;
    time = FreeRunningTimer_GetMilliSeconds();
    while(1) {
        time2 = FreeRunningTimer_GetMilliSeconds();
        if (time+10 < time2) {
            break;
        }
    }
    
}

#define sendToEncoder
#ifdef sendToEncoder
int main() {
    unsigned short dData;
    BOARD_Init();
    Protocol_Init();
    FreeRunningTimer_Init();
    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    unsigned short NOP = 0xC000;
    unsigned short packet = 0x7FFE;
    
    
    while(1) {
        delay();
        CS = 0;
        SPI2BUF = packet; // copy packet into appropriate buffer
        while(SPI2STATbits.SPIBUSY == 1); // packet transfer in progress
        CS = 1;
        rData=SPI2BUF; // read data
        rData = RotaryEncoder_ReadRawAngle();
        char timerMessage[MAXPAYLOADLENGTH];
        sprintf(timerMessage, "%u", rData);
        Protocol_SendDebugMessage(timerMessage);
        
    }
}
#endif

//#define sendMessage
#ifdef sendMessage
int main() {


    RotaryEncoder_Init(ENCODER_BLOCKING_MODE);
    unsigned int p;
    unsigned int packet = 0xF0;
    p = parityCheck(packet); // compute parity
    packet = packet | (p << 15); // insert parity bit
    CS = 0; // set slave select low
    SPI2BUF = packet; // copy packet into appropriate buffer
    
    
    while(1) {
        if (SPI2STATbits.SPITBE == 1) {
            CS = 1;
            //SPI2BUF = packet;
            //CS = 0;
        }
    }
}
#endif

//#define parityCheck
#ifdef parityCheck
int main() {
    unsigned int p;
    unsigned int in = 0b011111;
    p = parityCheck(in);
    unsigned int in2 = 0b011110;
    p = parityCheck(in2);
    while(1);
    
}
#endif
