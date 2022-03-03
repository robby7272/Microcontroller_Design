#include "NonVolatileMemory.h"
#include "BOARD.h"
#include "MessageIDs.h"
#include "FreeRunningTimer.h"
#include "Protocol.h"

/*******************************************************************************
 * PUBLIC #DEFINES                                                             *
 ******************************************************************************/
// sen, rsen, pen, rcen, acken, trstat
void busIdle(void) {
    while(I2C1CONbits.SEN);
    while(I2C1CONbits.RSEN);
    while(I2C1CONbits.PEN);
    while(I2C1CONbits.RCEN);
    while(I2C1CONbits.ACKEN);
    while(I2C1STATbits.TRSTAT);
}

void busError(void) {
    while(I2C1STATbits.I2COV);
    while(I2C1STATbits.IWCOL);
    while(I2C1STATbits.BCL);
    while(I2C1STATbits.I2COV);
    while(I2C1STATbits.RBF);
}
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
    
    I2C1TRN = address >> 8; // high bits
    while(I2C1STATbits.TRSTAT);
        
    I2C1TRN = address; // low bits
    while(I2C1STATbits.TRSTAT);
    
    I2C1CONbits.RSEN = 1; // Repeat Start
    while(I2C1CONbits.RSEN == 1); // wait for repeat transmission
    
    I2C1TRN = 0b10100001; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT);
    
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
    
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    I2C1TRN = 0b10100000; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    I2C1TRN = (0xFF00 & address) >> 8; // high bits
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    I2C1TRN = 0x00FF & address; // low bits
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    I2C1TRN = data; // data
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
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
int NonVolatileMemory_ReadPage(int page, char length, unsigned char data[]) {
    
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    I2C1TRN = 0b10100000; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT); 
    
    I2C1TRN = (0xFF00 & page) >> 8; // high bits
    while(I2C1STATbits.TRSTAT);
        
    I2C1TRN = 0x00FF & page; // low bits
    while(I2C1STATbits.TRSTAT);
    
    I2C1CONbits.RSEN = 1; // Repeat Start
    while(I2C1CONbits.RSEN == 1); // wait for repeat transmission
    
    I2C1TRN = 0b10100001; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT);
    
    busIdle();
    
    int i = 0;
    for (i = 0; i < 64; i++) {
        I2C1CONbits.RCEN = 1; // receive enable bit
        while(I2C1CONbits.RCEN == 1);
        busIdle();
        data[i] = I2C1RCV;
        if (i < 63) {
            I2C1CONbits.ACKDT = 0; // ACK
            I2C1CONbits.ACKEN = 1; // acknowledge event
            while(I2C1CONbits.ACKEN == 1); // wait for ACKEN transmission
        }
    }
    
    I2C1CONbits.ACKDT = 1; // NACK
    I2C1CONbits.ACKEN = 1; // acknowledge event
    while(I2C1STATbits.TBF == 1); // wait for ACK transmission
        
    I2C1CONbits.PEN = 1; // Send Stop bit
    while(I2C1CONbits.PEN == 1); // wait for stop transmission
}

/**
 * @Function char int NonVolatileMemory_WritePage(int page, char length, unsigned char data[])
 * @param address, device address to write to
 * @param data, value to write at said address
 * @return SUCCESS or ERROR
 * @brief writes one byte to device */
int NonVolatileMemory_WritePage(int page, char length, unsigned char data[]) {
    
    I2C1CONbits.SEN = 1; // Send Start bit
    while(I2C1CONbits.SEN == 1); // wait for Send transmission to be over
    
    I2C1TRN = 0b10100000; // last bit write-0, read-1
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    I2C1TRN = (0xFF00 & page) >> 8; // high bits
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    I2C1TRN = 0x00FF & page; // low bits
    while(I2C1STATbits.TRSTAT); // wait for Transmit
    
    int i;
    for (i = 0; i < 64; i++) {
        busIdle();
        I2C1TRN = data[i]; // data
        while(I2C1STATbits.TRSTAT); // wait for Transmit
    }
    
    I2C1CONbits.PEN = 1; // Send Stop bit
    while(I2C1CONbits.PEN == 1); // wait for stop transmission
}


int main() {
    BOARD_Init();
    FreeRunningTimer_Init();
    Protocol_Init();
    NonVolatileMemory_Init();
    
    unsigned int time1 = 0;
    unsigned int time2 = 0;
    unsigned int load[17];
    unsigned int address = 0;
    unsigned char Data[64];
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
            Data[0] = load[1] & 0b11111111;
            
            
            NonVolatileMemory_WriteByte(address, Data[0]);
            
            Protocol_SendMessage(1, ID_NVM_WRITE_BYTE_ACK, &Data[0]);
        }
        else if (Protocol_ReadNextID() == ID_NVM_READ_BYTE) {
            Protocol_GetPayload(&load);
            address = Protocol_IntEndednessConversion(load[0]);
            
            Data[0] = NonVolatileMemory_ReadByte(address);
            
            Protocol_SendMessage(1, ID_NVM_READ_BYTE_RESP, &Data[0]);
        }
        else if (Protocol_ReadNextID() == ID_NVM_WRITE_PAGE) {
            Protocol_GetPayload(&load);
            address = Protocol_IntEndednessConversion(load[0]);
            
            int i;
            for (i = 0; i < 16; i++)
            {
                Data[(i*4)] = load[i+1];
                Data[(i*4)+1] = load[i+1] >> 8;
                Data[(i*4)+2] = load[i+1] >> 16;
                Data[(i*4)+3] = load[i+1] >> 24;
            }
           
            Protocol_SendMessage(64, ID_NVM_WRITE_PAGE, &Data);
            
            NonVolatileMemory_WritePage(address, 64, Data);
        }
        else if (Protocol_ReadNextID() == ID_NVM_READ_PAGE) {
            Protocol_GetPayload(&load);
            address = Protocol_IntEndednessConversion(load[0]);
            
            NonVolatileMemory_ReadPage(address, 64, Data);
            
            Protocol_SendMessage(64, ID_NVM_READ_PAGE_RESP, &Data);
        }
    }
}
