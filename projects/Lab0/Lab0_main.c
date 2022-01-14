// **** Include libraries here ****
// Standard libraries
#include <stdio.h>


//CSE13E Support Library
#include "BOARD.h"

#define PART1 1 // set to 1 for part1
#define PART2 0 // set to 1 for part2
#define NOPS_FOR_5MS 6000 // 6000 found based on the 1 minute counter


void NOP_delay_5ms() {
    int i; 
    for ( i =0; i < NOPS_FOR_5MS; i++) {
                asm ("nop"); 
    } 
}

int main()
{
    BOARD_Init();
   
    TRISE = 0x00; // sets leds to output mode
    LATE = 0x00; // sets leds to off
    while (PART1) {
        if (PORTFbits.RF1 == 1) { // if a button is pressed an led will light up
            PORTEbits.RE0 = 1;
        } else {
            PORTEbits.RE0 = 0;
        }
        if (PORTDbits.RD5 == 1) {
            PORTEbits.RE1 = 1;
        } else {
            PORTEbits.RE1 = 0;
        }
        if (PORTDbits.RD6 == 1) {
            PORTEbits.RE2 = 1;
        } else {
            PORTEbits.RE2 = 0;
        }
        if (PORTDbits.RD7 == 1) {
            PORTEbits.RE3 = 1;
        } else {
            PORTEbits.RE3 = 0;
        }
    }
    
    while (PART2) {
        PORTE = 0x00; // initialize count to zero
        while (1) {
            PORTE++;
            int i; 
            for ( i =0; i < 50; i++) { // checks every 5ms for a button press
                // loop lasts 250ms
                if (PORTFbits.RF1 == 1) { // if button pressed reset count
                PORTE = 0x00;
                }
                NOP_delay_5ms();
            }
        }
    }
}
