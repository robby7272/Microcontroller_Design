#include "FreeRunningTimer.h"
#include "MessageIDs.h"
#include <stdio.h>
#include <string.h>
#include "Protocol.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "ADCFilter.h"
#include "NonVolatileMemory.h"

unsigned char currChann = 0;
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

unsigned char lowpass[64] = {0xFF, 0xCA, 0xFF, 0xC0, 0xFF, 0xAE, 0xFF, 0x9F, 0xFF, 0xA3, 0xFF, 0xD1, 0x00, 0x42, 0x01, 0x0A, 0x02, 0x32, 0x03, 0xB7, 0x05, 0x84, 0x07, 0x75, 0x09, 0x5C, 0x0B, 0x05, 0x0C, 0x40, 0x0C, 0xE8, 0x0C, 0xE8, 0x0C, 0x40, 0x0B, 0x05, 0x09, 0x5C, 0x07, 0x75, 0x05, 0x84, 0x03, 0xB7, 0x02, 0x32, 0x01, 0x0A, 0x00, 0x42, 0xFF, 0xD1, 0xFF, 0xA3, 0xFF, 0x9F, 0xFF, 0xAE, 0xFF, 0xC0, 0xFF, 0xCA};
unsigned char highpass[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0xFF, 0xA5, 0x00, 0x8B, 0xFF, 0x7F, 0x00, 0x00, 0x01, 0x0F, 0xFD, 0x9F, 0x03, 0x40, 0xFD, 0x48, 0x00, 0x00, 0x05, 0x11, 0xF4, 0x3D, 0x12, 0x93, 0xE8, 0x55, 0x19, 0x8E, 0xE8, 0x55, 0x12, 0x93, 0xF4, 0x3D, 0x05, 0x11, 0x00, 0x00, 0xFD, 0x48, 0x03, 0x40, 0xFD, 0x9F, 0x01, 0x0F, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x8B, 0xFF, 0xA5, 0x00, 0x27, 0x00, 0x00};

#define application
#ifdef application
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    LEDS_INIT();
    NonVolatileMemory_Init();
    ADCFilter_Init();
    
    //store filters in nvm
    NonVolatileMemory_WritePage(832, 64, lowpass);
    NonVolatileMemory_WritePage(833, 64, highpass);
    NonVolatileMemory_WritePage(834, 64, lowpass);
    NonVolatileMemory_WritePage(835, 64, highpass);
    NonVolatileMemory_WritePage(836, 64, lowpass);
    NonVolatileMemory_WritePage(837, 64, highpass);
    NonVolatileMemory_WritePage(838, 64, lowpass);
    NonVolatileMemory_WritePage(839, 64, highpass);
    
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Lab3 Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    
    unsigned int time1;
    unsigned int time2;
    
    while(1) {
        time1 = FreeRunningTimer_GetMilliSeconds();
        while(1) {
            time2 = FreeRunningTimer_GetMilliSeconds();
            if (time1+100 < time2) {
                break;
            }
        }
        uint8_t switchesState = SWITCH_STATES();
        if ((switchesState & SWITCH_STATE_SW2) && (switchesState & SWITCH_STATE_SW1)) {
            // channel 3
            currChann = 3;
            Protocol_SendMessage(1, ID_ADC_SELECT_CHANNEL_RESP, &currChann);
            LEDS_SET(0x01);
        } else if (switchesState & SWITCH_STATE_SW2) {
            currChann = 2;
            Protocol_SendMessage(1, ID_ADC_SELECT_CHANNEL_RESP, &currChann);
            // channel 2
            LEDS_SET(0x02);
        } else if (switchesState & SWITCH_STATE_SW1) {
            currChann = 1;
            Protocol_SendMessage(1, ID_ADC_SELECT_CHANNEL_RESP, &currChann);
            // channel 1
            LEDS_SET(0x04);
        } else {
            currChann = 0;
            Protocol_SendMessage(1, ID_ADC_SELECT_CHANNEL_RESP, &currChann);
            // channel 0
            LEDS_SET(0x08);
        }

    }
         
}
#endif

