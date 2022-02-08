
#include "RotaryEncoder.h"
#include "BOARD.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/

#define ENCODER_BLOCKING_MODE 0
#define ENCODER_INTERRUPT_MODE 1

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

    SPI2CON = 0;        // Stops and resets the SPI1. 
    rData=SPI2BUF;        // clear receive buffer
    
    SPI2BRG = 0x11; // 40MHz/(2*3+1) = 5MHz
    SPI2STATbits.SPIROV = 0; // clear SPIROV bit
    SPI2CONbits.MSTEN = 1; // Master mode    
    SPI2CONbits.MODE16 = 1; // 16 bit mode
    SPI2CONbits.CKE = 1; // data sampled on falling edge
    SPI2CONbits.CKP = 0; // clock idle low
    SPI2CONbits.ON = 1;     // SPI ON

    // slave I/O pin??
    // SPI2BUF = 'A'
    
}

/**
 * @Function int RotaryEncoder_ReadRawAngle(void)
 * @param None
 * @return 14-bit number representing the raw encoder angle (0-16384) */
unsigned short RotaryEncoder_ReadRawAngle(void);



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
