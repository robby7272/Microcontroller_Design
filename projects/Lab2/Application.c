
#include "FreeRunningTimer.h"
#include "MessageIDs.h"
#include <stdio.h>
#include <string.h>
#include "Protocol.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "PingSensor.h"
#include "RCServo.h"
#include "RotaryEncoder.h"

#define LEDS_INIT() do {LATECLR = 0xFF; TRISECLR = 0xFF;} while (0)

/**
 * Provides a way to quickly get the status of all 8 LEDs into a uint8, where a bit is 1 if the LED
 * is on and 0 if it's not. The LEDs are ordered such that bit 7 is LED8 and bit 0 is LED0.
 */
#define LEDS_GET() (LATE & 0xFF)

/**
 * This macro sets the LEDs on according to which bits are high in the argument. Bit 0 corresponds
 * to LED0.
 * @param leds Set the LEDs to this value where 1 means on and 0 means off.
 */
#define LEDS_SET(leds) do { LATE = (leds); } while (0)

//#define application
#ifdef application
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    PingSensor_Init();
    RotaryEncoder_Init(0);
    
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Protocol Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    
    
    
    while(1);
}
#endif
