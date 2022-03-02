#include "NonVolatileMemory.h"
#include "BOARD.h"
#include "MessageIDs.h"
#include "FreeRunningTimer.h"
#include "Protocol.h"

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
    I2C1STAT=0;
    I2C1BRG = 0x00C5;//0xC2; // 100 KHz
    I2C1CONbits.ON = 1; // turn on
    I2C1CONbits.PEN = 1;
    while(I2C1CONbits.PEN);
}

/**
 * @Function NonVolatileMemory_ReadByte(int address)
 * @param address, device address to read from
 * @return value at said address
 * @brief reads one byte from device
 * @warning Default value for this EEPROM is 0xFF */
unsigned char NonVolatileMemory_ReadByte(int address) {   
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    I2C1TRN = 0b10100000; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit and ACK
    while(I2C1STATbits.ACKSTAT == 1);  
    
    I2C1TRN = address >> 8; // high bits
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit and ACK
    while(I2C1STATbits.ACKSTAT == 1);
        
    I2C1TRN = address; // low bits
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit and ACK
    while(I2C1STATbits.ACKSTAT == 1);
    
    I2C1CONbits.RSEN = 1; // Repeat Start
    while(I2C1CONbits.RSEN == 1); // wait for repeat transmission
    
    I2C1TRN = 0b10100001; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit and ACK
    while(I2C1STATbits.ACKSTAT == 1);
    
    I2C1CONbits.RCEN = 1; // receive enable bit
    while(I2C1CONbits.RCEN == 1);
    unsigned char data = I2C1RCV;
    while(I2C1STATbits.TRSTAT);
    
    I2C1CONbits.ACKDT = 1; // NACK
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.TBF == 1); // wait for ACK transmission
        
    I2C1CONbits.PEN = 1; // Send Stop bit
    while(I2C1CONbits.PEN == 1); // wait for stop transmission
    
    return data;
}

/**
 * @Function char NonVolatileMemory_WriteByte(int address, unsigned char data)
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
char NonVolatileMemory_WriteByte(int address, unsigned char data) {
    
    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
    I2C1TRN = 0b10100000; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK
    
    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
    I2C1TRN = (0xFF00 & address) >> 8; // high bits
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK

    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
    I2C1TRN = 0x00FF & address; // low bits
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK

    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
    I2C1TRN = data; // data
    while(I2C1STATbits.TRSTAT);
    while(I2C1STATbits.TBF == 1); // wait for Transmit
    while(I2C1STATbits.ACKSTAT == 1); // wait for ACK
    
    if (I2C1STATbits.BCL == 1) {
        int x = 5;
    }
    
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
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    NonVolatileMemory_Init();
    
    unsigned int time1 = 0;
    unsigned int time2 = 0;
    unsigned int load[2];
    unsigned int address = 0;
    unsigned char data = 0;
    while(1) {
        time1 = FreeRunningTimer_GetMilliSeconds();
        while(1) {
            time2 = FreeRunningTimer_GetMilliSeconds();
            if (time1+100 < time2) {
                break;
            }
        }
        if (Protocol_ReadNextID() == ID_NVM_WRITE_BYTE) {
            Protocol_GetPayload(&load);
            address = Protocol_IntEndednessConversion(load[0]);
            data = load[1] & 0b11111111;
            
            
            NonVolatileMemory_WriteByte(address, data);
            
            Protocol_SendMessage(1, ID_NVM_WRITE_BYTE_ACK, &data);
        }
        else if (Protocol_ReadNextID() == ID_NVM_READ_BYTE) {
            Protocol_GetPayload(&load);
            address = Protocol_IntEndednessConversion(load[0]);
            
            data = NonVolatileMemory_ReadByte(address);
            
            Protocol_SendMessage(1, ID_NVM_READ_BYTE_RESP, &data);
        }
        else if (Protocol_ReadNextID() == ID_NVM_WRITE_PAGE) {
            Protocol_GetPayload(&data);
        }
        else if (Protocol_ReadNextID() == ID_NVM_READ_PAGE) {
            Protocol_GetPayload(&data);
        }
    }
}
