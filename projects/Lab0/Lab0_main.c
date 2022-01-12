// **** Include libraries here ****
// Standard libraries
#include <stdio.h>


//CSE13E Support Library
#include "BOARD.h"

// Microchip libraries

#define PART1 0
#define PART2 0
#define NOPS_FOR_5MS 5000

// **** Set any macros or preprocessor directives here ****

void NOP_delay_5ms() {
    
    int i; 
    for ( i =0; i < NOPS_FOR_5MS; i++) {
                asm ("nop"); 
    } 
}

int main()
{
    // initialize everything
    BOARD_Init();
   
    TRISE = 0x00;
    LATE = 0x00;
    while (PART1) {
        if (PORTFbits.RF1 == 1) {
            PORTEbits.RE0 = 1;
        }
        if (PORTDbits.RD5 == 1) {
            PORTEbits.RE1 = 1;
        }
        if (PORTDbits.RD6 == 1) {
            PORTEbits.RE2 = 1;
        }
        if (PORTDbits.RD7 == 1) {
            PORTEbits.RE3 = 1;
        }
    }
    
    while (PART2) {
        PORTE = 0x01;
        while (1) {
            PORTE++;
            NOP_delay_5ms();
        }
    }

    //initialize state machine (and anything else you need to init) here

}
