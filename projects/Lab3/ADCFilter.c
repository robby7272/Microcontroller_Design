#include "FreeRunningTimer.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "Protocol.h"
#include <stdio.h>
#include "ADCFilter.h"
#include "MessageIDs.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

#define FILTERLENGTH 32

#define AN2 LATBbits.LATB2
#define AN4 LATBbits.LATB4
#define AN8 LATBbits.LATB8
#define AN10 LATBbits.LATB10
/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

short data[4][FILTERLENGTH]; // initialized to 0's
short filters[4][FILTERLENGTH]; // initialized to 0's
unsigned int Head = -1;
short test;
unsigned char prevChannel = -1;
unsigned char currChannel = 0;
unsigned short prevValue;

/**
 * @Function ADCFilter_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes ADC system along with naive filters */
int ADCFilter_Init(void) {
    AD1PCFG = 0x0000; // all pins analog
    TRISBbits.TRISB2 = 1; // Analog pins set to input
    TRISBbits.TRISB4 = 1;
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB10 = 1;
    
    AD1CSSLbits.CSSL2 = 1; // Analog pins part of scan
    AD1CSSLbits.CSSL4 = 1;
    AD1CSSLbits.CSSL8 = 1;
    AD1CSSLbits.CSSL10 = 1;
    
    AD1CON1bits.ASAM = 1; // auto sample
    AD1CON1bits.SSRC = 0b111; // auto convert
    AD1CON1bits.FORM = 0b000; // unsigned integer 16 bit
    
    AD1CON2bits.CSCNA = 1; // Scan mode on
    AD1CON2bits.VCFG = 0b000; // internal voltage reference
    AD1CON2bits.SMPI = 0b011; // interrupt after 4th measurement
    
    AD1CON3bits.ADRC = 0; // peripheral bus clock
    AD1CON2bits.BUFM = 0; // 16 bit buffer
    
    AD1CON3bits.ADCS = 0b10101101; // Tad = 348 Tpb
    AD1CON3bits.SAMC = 0b10000; // Sample time 16 Tad
    
    AN2 = 1;
    AN4 = 1;
    AN8 = 1;
    AN10 = 1;
    
    AD1CON1bits.ON = 1; // turn on
    IFS1bits.AD1IF = 0; // clear interrupt
    IPC6bits.AD1IP = 4; // interrupt priority
    IEC1bits.AD1IE = 1; // enable interrupt
}

/**
 * @Function ADCFilter_RawReading(short pin)
 * @param pin, which channel to return
 * @return un-filtered AD Value
 * @brief returns current reading for desired channel */
short ADCFilter_RawReading(short pin) {
    return data[pin][Head];
}

/**
 * @Function ADCFilter_FilteredReading(short pin)
 * @param pin, which channel to return
 * @return Filtered AD Value
 * @brief returns filtered signal using weights loaded for that channel */
short ADCFilter_FilteredReading(short pin) {
    return ADCFilter_ApplyFilter(filters[pin], data[pin], Head);
}

/**
 * @Function short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex)
 * @param filter, pointer to filter weights
 * @param values, pointer to circular buffer of values
 * @param startIndex, location of first sample so filter can be applied correctly
 * @return Filtered and Scaled Value
 * @brief returns final signal given the input arguments
 * @warning returns a short but internally calculated value should be an int */
short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex) {
    int i = 0;
    unsigned int sum = 0;
    short count = startIndex;
    for (i = 0; i < FILTERLENGTH; i++) {
        sum += filter[i] * values[count];
        count = (count+1)%FILTERLENGTH;
    }
    return (short) sum >> 15;
}

/**
 * @Function ADCFilter_SetWeights(short pin, short weights[])
 * @param pin, which channel to return
 * @param pin, array of shorts to load into the filter for the channel
 * @return SUCCESS or ERROR
 * @brief loads new filter weights for selected channel */
int ADCFilter_SetWeights(short pin, short weights[]) {
    int i = 0;
    for (i = 0; i < FILTERLENGTH; i++) {
        filters[pin][i] = weights[i];
    }
    return 1;
}



void __ISR(_ADC_VECTOR) ADCIntHandler(void) {
    IFS1bits.AD1IF = 0; // clear interrupt
    
    Head = (Head+1)%FILTERLENGTH;
    
    data[0][Head] = ADC1BUF0;
    data[1][Head] = ADC1BUF1;
    data[2][Head] = ADC1BUF2;
    data[3][Head] = ADC1BUF3;
}

#define filterTest
#ifdef filterTest
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();
    FreeRunningTimer_Init();
    ADCFilter_Init();
    
    
    unsigned short filterValues[FILTERLENGTH];
    unsigned int time1 = 0;
    unsigned int time2 = 0;
    short raw[2];
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Protocol Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    
    while(1) {
        while(1) {
            time2 = FreeRunningTimer_GetMilliSeconds();
            if (time1+1000 < time2) {
                break;
            }
        }
        raw[0] = ADCFilter_RawReading(currChannel);
        raw[1] = ADCFilter_FilteredReading(currChannel);
        Protocol_SendMessage(1, ID_ADC_READING, &raw);
        
        if (Protocol_ReadNextID() == ID_ADC_SELECT_CHANNEL) {
            Protocol_GetPayload(&currChannel);
            if (prevChannel != currChannel) {
                Protocol_SendMessage(1, ID_ADC_SELECT_CHANNEL_RESP, &currChannel);
                prevChannel = currChannel;
            }
        }
        if (Protocol_ReadNextID() == ID_ADC_FILTER_VALUES) {
            Protocol_GetPayload(&filterValues);
            if (prevValue != filterValues[0]) {
                int i = 0;
                for (i = 0; i < FILTERLENGTH; i++) {
                    filterValues[i] = Protocol_ShortEndednessConversion(filterValues[i]);
                }
                ADCFilter_SetWeights(currChannel, filterValues);
                Protocol_SendMessage(1, ID_ADC_FILTER_VALUES_RESP, &currChannel);
                filterValues[0] = Protocol_ShortEndednessConversion(filterValues[0]);
                prevValue = filterValues[0];
            }
            
        }
        
        
    }
}
#endif
