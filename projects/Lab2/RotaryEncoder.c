

#include "RotaryEncoder.h"
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
int RotaryEncoder_Init(char interfaceMode);

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