/* 
 * File:   Protocol.h
 * Author: Instructor
 *
 * Created on August 15, 2019, 9:24 AM
 */
#include "MessageIDs.h"
#include <stdio.h>
#include <string.h>
#include "Protocol.h"
#include "BOARD.h"
#include <sys/attribs.h>
#include "TXBuffer.h"
/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/
#define PACKETBUFFERSIZE 5  // how many payloads the buffer has to store, not bytes

#define MAXPAYLOADLENGTH 128 // note that this does include the ID

#define HEAD 204
#define TAIL 185

#define bufferLength 15
int ech = 0;
/*******************************************************************************
 * private DATATYPES
 ******************************************************************************/
    static struct {
        unsigned int head;
        unsigned int tail;
        char data[bufferLength];
    } circBuffer;

    typedef enum {
        START = 0,
        LENGTH,
        ID,
        SETUPLEDS,
        GETLEDS,
        PAYLOAD,
        TAILS,
        CHECKSUM,
    } machineStates;
    int state;
    
    static struct {
        char length;
        char data[MAXPAYLOADLENGTH];
    } RX_State_Machine;
    int count = 0;
    unsigned char checkSum = 0;
    unsigned char ledState = 0;
    int updateLeds = 0;
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
/**
 * @Function Protocol_Init(void)
 * @param None
 * @return SUCCESS or ERROR
 * @brief 
 * @author mdunne */
int Protocol_Init(void) {

    // Step 1 clear control registers
    U1STA = 0;
    // Step 2 calculate baud rate
    U1BRG = 21; // Table 21-2 and 21.5.3
    // Step 3 Set UART1 for 8n1
    U1MODEbits.PDSEL = 0b00; // 8 bit data, no parity
    U1MODEbits.STSEL = 0b0; // 1 stop bit
    // Step 4 Enable UART1
    U1MODEbits.UARTEN = 0; //UART Enable
    U1MODEbits.ON = 1; // UART1 On
    // Step 5 Enable TX
    U1STAbits.UTXEN = 1; // Enable transmission bit
    // Step 6 Enable RX
    U1STAbits.URXEN = 1; // Enable reception bit
    
    IFS0bits.U1TXIF = 0; // clear transmit interrupt flag
    IFS0bits.U1RXIF = 0; // clear receive interrupt flag
    IPC6bits.U1IP = 3; // Interrupt priority ?
    IPC6bits.U1IS = 2; // Interrupt priority ??
    IEC0bits.U1TXIE = 1; // Transmit interrupt enable
    IEC0bits.U1RXIE = 1; // Reception interrupt enable
    U1STAbits.OERR = 0; // Set receive buffer low, has not overflowed
    U1STAbits.UTXISEL = 0b10; // Generate interrupt when TX buffer empty
    U1STAbits.URXISEL = 0b00; // Generate interrupt when RX buffer not empty
   
}

/**
 * @Function int Protocol_SendMessage(unsigned char len, void *Payload)
 * @param len, length of full <b>Payload</b> variable
 * @param Payload, pointer to data, will be copied in during the function
 * @return SUCCESS or ERROR
 * @brief 
 * @author mdunne */
int Protocol_SendMessage(unsigned char len, unsigned char ID, void *Payload) {
    char *load = (char*)Payload;
    PutChar(0xCC); 
    PutChar(len+1);
    char curChecksum = ID;
    PutChar(ID);
    for (int i = 0; i<len; i++) {
        curChecksum = Protocol_CalcIterativeChecksum(5, curChecksum);
        PutChar(*load++);
    }
    PutChar(0xB9);
    PutChar(curChecksum);
    PutChar(0x0D);
    PutChar(0x0A);
    
    return 1;
}

/**
 * @Function int Protocol_SendDebugMessage(char *Message)
 * @param Message, Proper C string to send out
 * @return SUCCESS or ERROR
 * @brief Takes in a proper C-formatted string and sends it out using ID_DEBUG
 * @warning this takes an array, do <b>NOT</b> call sprintf as an argument.
 * @author mdunne */
int Protocol_SendDebugMessage(char *Message);

/**
 * @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads ID of next Packet
 * @brief Returns ID_INVALID if no packets are available
 * @author mdunne */
unsigned char Protocol_ReadNextID(void);

/**
 * @Function int Protocol_GetPayload(void* payload)
 * @param payload, Memory location to put payload
 * @return SUCCESS or ERROR
 * @brief 
 * @author mdunne */
int Protocol_GetPayload(void* payload);

/**
 * @Function char Protocol_IsMessageAvailable(void)
 * @param None
 * @return TRUE if Queue is not Empty
 * @brief 
 * @author mdunne */
char Protocol_IsMessageAvailable(void);

/**
 * @Function char Protocol_IsQueueFull(void)
 * @param None
 * @return TRUE is QUEUE is Full
 * @brief 
 * @author mdunne */
char Protocol_IsQueueFull(void);

/**
 * @Function char Protocol_IsError(void)
 * @param None
 * @return TRUE if error
 * @brief Returns if error has occurred in processing, clears on read
 * @author mdunne */
char Protocol_IsError(void);

/**
 * @Function char Protocol_ShortEndednessConversion(unsigned short inVariable)
 * @param inVariable, short to convert endedness
 * @return converted short
 * @brief Converts endedness of a short. This is a bi-directional operation so only one function is needed
 * @author mdunne */
unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable);

/**
 * @Function char Protocol_IntEndednessConversion(unsigned int inVariable)
 * @param inVariable, int to convert endedness
 * @return converted short
 * @brief Converts endedness of a int. This is a bi-directional operation so only one function is needed
 * @author mdunne */
unsigned int Protocol_IntEndednessConversion(unsigned int inVariable);

/*******************************************************************************
 * PRIVATE FUNCTIONS
 * generally these functions would not be exposed but due to the learning nature of the class they
 * are to give you a theory of how to organize the code internal to the module
 ******************************************************************************/

/**
 * @Function char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum)
 * @param charIn, new char to add to the checksum
 * @param curChecksum, current checksum, most likely the last return of this function, can use 0 to reset
 * @return the new checksum value
 * @brief Returns the BSD checksum of the char stream given the curChecksum and the new char
 * @author mdunne */
unsigned char Protocol_CalcIterativeChecksum(unsigned char charIn, unsigned char curChecksum) {
    curChecksum = (curChecksum >> 1) + (curChecksum << 7);
    curChecksum += charIn;
    return curChecksum;

}

/**
 * @Function void Protocol_runReceiveStateMachine(unsigned char charIn)
 * @param charIn, next character to process
 * @return None
 * @brief Runs the protocol state machine for receiving characters, it should be called from 
 * within the interrupt and process the current character
 * @author mdunne */
void Protocol_RunReceiveStateMachine(unsigned char charIn) {
    if (state == START) {
        if (charIn == 0xCC) {
            state = LENGTH;
        }
    }
    else if (state == LENGTH) {
        RX_State_Machine.length = charIn;
        state = ID;
    }
    else if (state == ID) {
        if (charIn == ID_LEDS_SET) {
            //LEDS_SET(0xFF);
            state = SETUPLEDS;
        }
        else if (charIn == ID_LEDS_GET) {
            state = GETLEDS;
        }
        else {
            RX_State_Machine.data[count] = charIn;
            checkSum = Protocol_CalcIterativeChecksum(charIn, checkSum);
            count++;
            state = PAYLOAD;
        }
    }
    else if (state == SETUPLEDS) {
        ledState = charIn;
        state = TAILS;
    }
    else if (state == PAYLOAD) {
        if (count < RX_State_Machine.length) {
            RX_State_Machine.data[count] = charIn;
            checkSum = Protocol_CalcIterativeChecksum(charIn, checkSum);
            count++;
        }
        if (count >= RX_State_Machine.length) {
            state = TAILS;
            count = 0;
        }
    }
    else if (state == TAILS) {
        if (charIn == 0xB9) {
            state = CHECKSUM;
        }
        else {
            state = START;
        }
    }
    else if (state == CHECKSUM) {
        if (ledState != LEDS_GET()) { 
            LEDS_SET(ledState);
        }
        if (checkSum == charIn) {
            //LEDS_SET(0xFF);
        }
        checkSum = 0;
        state = START;
    }
}

/**
 * @Function char PutChar(char ch)
 * @param ch, new char to add to the circular buffer
 * @return SUCCESS or ERROR
 * @brief adds to circular buffer if space exists, if not returns ERROR
 * @author mdunne */
int PutChar(char ch) {
    if (bufferFull() == 0) {
        bufferAdd(ch);
    }
    if (U1STAbits.TRMT == 1) { // UART is idle
        IFS0bits.U1TXIF = 1; // cause interrupt
    }
    if (U1STAbits.URXDA == 1) { // Receiver has data
        IFS0bits.U1RXIF = 1; // cause interrupt
    }
}

void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    
    if (IFS0bits.U1TXIF == 1) {
        
        while (bufferEmpty() == 0) {
            while(U1STAbits.TRMT == 0);
            U1TXREG = bufferRemove();
        }
        IFS0bits.U1TXIF = 0;
    }
    else {
        IFS0bits.U1RXIF = 0;
        ech = U1RXREG;
        Protocol_RunReceiveStateMachine(ech);
    }
}


int bufferFull() {
    if ((circBuffer.tail + 1) % bufferLength == circBuffer.head) {
        return 1;
    }
    return 0;
}

void bufferAdd(char c) {
    if (bufferFull() == 0) {
        circBuffer.data[circBuffer.tail] = c;
        circBuffer.tail = (circBuffer.tail + 1) % bufferLength;
    }
}

int bufferEmpty() {
    if (circBuffer.head == circBuffer.tail) {
        return 1;
    }
    return 0;
}

char bufferRemove() {
    if (bufferEmpty() == 0) {
        char c = circBuffer.data[circBuffer.head];
        circBuffer.head = (circBuffer.head + 1) % bufferLength;
        return c;
    }
}

int bufferCount() {
    if (circBuffer.tail > circBuffer.head) {
        return circBuffer.tail-circBuffer.head;
    }
    return bufferLength-(circBuffer.head-circBuffer.tail);
}


/**
 * This macro initializes all LEDs for use. It enables the proper pins as outputs and also turns all
 * LEDs off.
 */
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


#define test
#ifdef test
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();

    //char str[7] = "O123456";
    //Protocol_SendMessage(7, ID_LEDS_SET, (&str));
    //unsigned char t = 0x81;
    //unsigned char check = 0;
    //check = Protocol_CalcIterativeChecksum(t, check);
    //t = 0x05;
    //check = Protocol_CalcIterativeChecksum(t, check);
    //t = 0x00;
    //check = Protocol_CalcIterativeChecksum(t, check);
    RX_State_Machine.data;
    while(1);
}
#endif 






//#define echo
#ifdef echo
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();

    while(1) {
        if (ech != 0) {
            PutChar(ech);
            ech = 0;
        }
    }
}
#endif


//#define TX_buffer
#ifdef TX_buffer
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();
    
    
    char test[13] = "Hello, World!";
    for (int i = 0; i < 13; i++) {
        PutChar(test[i]);
    }
}
#endif







