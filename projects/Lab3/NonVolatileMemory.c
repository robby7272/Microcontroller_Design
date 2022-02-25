#include "NonVolatileMemory.h"
#include "BOARD.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTION PROTOTYPES                                                  *
 ******************************************************************************/

/**
 * @Function NonVolatileMemory_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief initializes I2C for usage */
int NonVolatileMemory_Init(void) {
    I2C1CON = 0; // clear control bits
    I2C1BRG = 0xC2; // 100 KHz
    I2C1CONbits.DISSLW = 1; // Disable slew rate
    I2C1CONbits.ON = 1; // turn on
}

/**
 * @Function NonVolatileMemory_ReadByte(int address)
 * @param address, device address to read from
 * @return value at said address
 * @brief reads one byte from device
 * @warning Default value for this EEPROM is 0xFF */
unsigned char NonVolatileMemory_ReadByte(int address);

/**
 * @Function char NonVolatileMemory_WriteByte(int address, unsigned char data)
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
char NonVolatileMemory_WriteByte(int address, unsigned char data) {
    
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    I2C1TRN = 0b10100000;
    while(I2C1STATbits.TBF == 1); // wait for Transmit buffer 
    
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK transmission
    
    I2C1TRN = (char) address >> 8; // high bits
    while(I2C1STATbits.TBF == 1); // wait for Transmit buffer 
    
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK transmission
    
    I2C1TRN = (char) address; // low bits
    while(I2C1STATbits.TBF == 1); // wait for Transmit buffer 
    
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK transmission
    
    I2C1TRN = data; // low bits
    while(I2C1STATbits.TBF == 1); // wait for Transmit buffer 
    
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK transmission
    
    I2C1CONbits.PEN = 1; // Send Stop bit
    while(I2C1CONbits.PEN == 1); // wait for stop transmission
}

/**
 * @Function int NonVolatileMemory_ReadPage(int page, char length, unsigned char data[])
 * @param page, page value to read from
 * @param length, value between 1 and 64 bytes to read
 * @param data, array to store values into
 * @return SUCCESS or ERROR
 * @brief reads bytes in page mode, up to 64 at once
 * @warning Default value for this EEPROM is 0xFF */
int NonVolatileMemory_ReadPage(int page, char length, unsigned char data[]);

/**
 * @Function char int NonVolatileMemory_WritePage(int page, char length, unsigned char data[])
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
int NonVolatileMemory_WritePage(int page, char length, unsigned char data[]);


int main() {
    while(1);
}
