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
    NonVolatileMemory_WritePage(832, 64, lowpass); // switch 0, filter 0
    NonVolatileMemory_WritePage(833, 64, highpass); // switch 0, filter 1
    NonVolatileMemory_WritePage(834, 64, lowpass); // switch 1, filter 0
    NonVolatileMemory_WritePage(835, 64, highpass); // switch 1, filter 1
    NonVolatileMemory_WritePage(836, 64, lowpass); // switch 2, filter 0
    NonVolatileMemory_WritePage(837, 64, highpass); // switch 2, filter 1
    NonVolatileMemory_WritePage(838, 64, lowpass); // switch 3, filter 0
    NonVolatileMemory_WritePage(839, 64, highpass); // switch 3, filter 1
    
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Lab3 Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    
    unsigned int time1;
    unsigned int time2;
    int display = 0;
    unsigned int filterValues[16];
    unsigned char Values[64];
    short raw[64];
    int count = 0;
    short filtered[64];
    int both = 0;
    int address = 832;
    short filter[32];
    int max = 0;
    int min = 0;
    int difference;
    uint8_t switchesState = 0;
    while(1) {
        time1 = FreeRunningTimer_GetMilliSeconds();
        while(1) {
            time2 = FreeRunningTimer_GetMilliSeconds();
            if (time1+100 < time2) {
                break;
            }
        }
        if (switchesState != SWITCH_STATES()) {
            switchesState = SWITCH_STATES();
            if ((switchesState & SWITCH_STATE_SW2) && (switchesState & SWITCH_STATE_SW1)) {
                currChann = 0x30 | (currChann & 0x0F); // 110000
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            } else if (switchesState & SWITCH_STATE_SW2) {
                currChann = 0x20 | (currChann & 0x0F); // 100000
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            } else if (switchesState & SWITCH_STATE_SW1) {
                currChann = 0x10 | (currChann & 0x0F); // 10000
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            } else {
                currChann = currChann & 0x0F;
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            }        
            if(switchesState & SWITCH_STATE_SW3) {
                currChann = currChann | 0x01;
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            } else {
                currChann = currChann & 0xF0;
                Protocol_SendMessage(1, ID_LAB3_CHANNEL_FILTER, &currChann);
            }        
            if(switchesState & SWITCH_STATE_SW4) {
                display = 1; // peak to peak
            } else {
                display = 0; // absolute
            }
        }
        
        
        if (Protocol_ReadNextID() == ID_ADC_FILTER_VALUES) {
            Protocol_GetPayload(&filterValues);            
            int i;
            for (i = 0; i < 16; i++)
            {
                Values[(i*4)] = filterValues[i];
                Values[(i*4)+1] = filterValues[i] >> 8;
                Values[(i*4)+2] = filterValues[i] >> 16;
                Values[(i*4)+3] = filterValues[i] >> 24;
            }
            
            address = 832 + (currChann & 0x0F) + (currChann >> 4 & 0x0F);
            NonVolatileMemory_WritePage(address, 64, Values);
        }
        
        raw[count] = ADCFilter_RawReading(currChann >> 4);
        count = (count+1)%32;
        int j = 0;
        int k = 0;
        for (k = 0,j = 0; k < 32; j = j + 2,k++) {
            filter[k] = Values[j] << 8 | Values[j+1];
        }
        
        //NonVolatileMemory_ReadPage(832, 64, data);
        
        
        filtered[count] = ADCFilter_ApplyFilter(filter, raw, count);
        both = (raw[count] << 16) | filtered[count];
        int a = 0;
        for (a = 0; a < 32; a++) {
            if(filtered[a] > max) {
                max = filtered[a];
            }
            if(filtered[a] < min) {
                min = filtered[a];
            }
        }
        
        if(display == 0) {
            if(filtered[count] < 0) {
                filtered[count] = filtered[count] * -1;
            }
            
            if(filtered[count] > 700) {
                LEDS_SET(0xFF);
            } else if(filtered[count] > 600) {
                LEDS_SET(0x7F);
            } else if(filtered[count] > 500) {
                LEDS_SET(0x3F);
            } else if(filtered[count] > 400) {
                LEDS_SET(0x1F);
            } else if(filtered[count] > 300) {
                LEDS_SET(0x0F);
            } else if(filtered[count] > 200) {
                LEDS_SET(0x07);
            } else if(filtered[count] > 100) {
                LEDS_SET(0x03);
            } else if(filtered[count] > 0) {
                LEDS_SET(0x01);
            }
        } else {
            difference = max - min;
            
            if(difference > 700) {
                LEDS_SET(0xFF);
            } else if(difference > 600) {
                LEDS_SET(0x7F);
            } else if(difference > 500) {
                LEDS_SET(0x3F);
            } else if(difference > 400) {
                LEDS_SET(0x1F);
            } else if(difference > 300) {
                LEDS_SET(0x0F);
            } else if(difference > 200) {
                LEDS_SET(0x07);
            } else if(difference > 100) {
                LEDS_SET(0x03);
            } else if(difference > 0) {
                LEDS_SET(0x01);
            }
        }
        
        both = Protocol_IntEndednessConversion(both);
        Protocol_SendMessage(4, ID_ADC_READING, &both);
        max = 0;
        min = 700;
    }
         
}
#endif
