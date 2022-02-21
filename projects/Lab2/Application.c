
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

#define application
#ifdef application
int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    PingSensor_Init();
    RotaryEncoder_Init(0);
    RCServo_Init();
    
    unsigned int RCPingPulse = RC_SERVO_CENTER_PULSE;
    unsigned int RCEncoderPulse = RC_SERVO_CENTER_PULSE;
    unsigned short encoderData;
    unsigned short pingData;
    unsigned short ENCODER = 0;
    unsigned short PING = 0;
    unsigned short ChosenTest = 0;
    
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Protocol Test Compiled at %s %s", __DATE__, __TIME__);
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
        if (Protocol_ReadNextID() == ID_LAB2_INPUT_SELECT) {
            Protocol_GetPayload(&ChosenTest);
        }
        if (ChosenTest == 1) {
            encoderData = RotaryEncoder_ReadRawAngle(); // maybe need to change endian here? 
            RCEncoderPulse = encoderData/13;
            //encoderData = Protocol_ShortEndednessConversion(encoderData);
            //Protocol_SendMessage(2, 0x86, &encoderData);
            if (RCEncoderPulse > RC_SERVO_MAX_PULSE) {
                RCEncoderPulse = RC_SERVO_MAX_PULSE;
            }
            if (RCEncoderPulse < RC_SERVO_MIN_PULSE) {
                RCEncoderPulse = RC_SERVO_MIN_PULSE;
            }
            RCServo_SetPulseWithCorrectTicks(RCEncoderPulse);
        }
        if (ChosenTest == 0) {
            pingData = PingSensor_GetDistance();
            RCPingPulse = pingData*3 + 600;
            if (RCPingPulse > RC_SERVO_MAX_PULSE) {
                RCPingPulse = RC_SERVO_MAX_PULSE;
            }
            if (RCPingPulse < RC_SERVO_MIN_PULSE) {
                RCPingPulse = RC_SERVO_MIN_PULSE;
            }
            RCServo_SetPulseWithCorrectTicks(RCPingPulse);
            //pingData = Protocol_ShortEndednessConversion(pingData);
            //Protocol_SendMessage(2, 0x87, &pingData);
        }
    }
         
}
#endif
