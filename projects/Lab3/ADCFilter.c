#include "FreeRunningTimer.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "Protocol.h"
#include <stdio.h>
#include "ADCFilter.h"
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
    AD1CON2bits.SMPI = 0b000; // interrupt after each measurement
    
    AD1CON3bits.ADRC = 0; // peripheral bus clock
    AD1CON2bits.BUFM = 0; // 16 bit buffer
    
    AD1CON3bits.ADCS = 0b10101101; // Tad = 348 Tpb
    AD1CON3bits.SAMC = 0b10000; // Sample time 16 Tad
    
    AD1CON1bits.ON = 1; // turn on
    IFS1bits.AD1IF = 0; // clear interrupt
    IEC1bits.AD1IE = 1; // enable interrupt
    
    // turn on timer?
    
}

/**
 * @Function ADCFilter_RawReading(short pin)
 * @param pin, which channel to return
 * @return un-filtered AD Value
 * @brief returns current reading for desired channel */
short ADCFilter_RawReading(short pin);

/**
 * @Function ADCFilter_FilteredReading(short pin)
 * @param pin, which channel to return
 * @return Filtered AD Value
 * @brief returns filtered signal using weights loaded for that channel */
short ADCFilter_FilteredReading(short pin);

/**
 * @Function short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex)
 * @param filter, pointer to filter weights
 * @param values, pointer to circular buffer of values
 * @param startIndex, location of first sample so filter can be applied correctly
 * @return Filtered and Scaled Value
 * @brief returns final signal given the input arguments
 * @warning returns a short but internally calculated value should be an int */
short ADCFilter_ApplyFilter(short filter[], short values[], short startIndex);

/**
 * @Function ADCFilter_SetWeights(short pin, short weights[])
 * @param pin, which channel to return
 * @param pin, array of shorts to load into the filter for the channel
 * @return SUCCESS or ERROR
 * @brief loads new filter weights for selected channel */
int ADCFilter_SetWeights(short pin, short weights[]);



void __ISR(_ADC_VECTOR) ADCIntHandler(void) {
IFS1bits.AD1IF = 0; // clear interrupt
}

int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();
    FreeRunningTimer_Init();
    ADCFilter_Init();
    
    while(1);
}
