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

/*******************************************************************************
 * PUBLIC #DEFINES                                                            *
 ******************************************************************************/
#define PACKETBUFFERSIZE 5  // how many payloads the buffer has to store, not bytes

#define MAXPAYLOADLENGTH 128 // note that this does include the ID

#define HEAD 204
#define TAIL 185

#define bufferLength 255 // has to be 2^n -1

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
    int state; // state of state machine
    
    static struct {
        unsigned char length;
        unsigned char data[MAXPAYLOADLENGTH];
    } RX_State_Machine;
    
    int count = 0; // index for storing into RX buffer
    unsigned char checkSum = 0; // checksum of incoming packet
    unsigned char ledState = 0; // stores current state of LEDs
    int ech = 0; // temp variable for Interrupt
    int ready = 0; // flag variable for state machine
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                           *
 ******************************************************************************/
int bufferFull();
void bufferAdd(char c);
int bufferEmpty();
char bufferRemove();
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
    unsigned char *load = (unsigned char*)Payload;
    PutChar(0xCC); 
    PutChar(len+1);
    char curChecksum = ID;
    PutChar(ID);
    int i = 0;
    for (i = 0; i<len; i++) {
        curChecksum = Protocol_CalcIterativeChecksum(*load, curChecksum);
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
int Protocol_SendDebugMessage(char *Message) {
    char *load = (char*)Message;
    PutChar(0xCC);
    int len = strlen(load);
    PutChar(len+1);
    PutChar(ID_DEBUG);
    char curChecksum = ID_DEBUG;
    int i = 0;
    for (i = 0; i<len; i++) {
        curChecksum = Protocol_CalcIterativeChecksum(*load, curChecksum);
        PutChar(*load++);
    }
    PutChar(0xB9);
    PutChar(curChecksum);
    PutChar(0x0D);
    PutChar(0x0A);
    
    return 1;
}

/**
 * @Function unsigned char Protocol_ReadNextID(void)
 * @param None
 * @return Reads ID of next Packet
 * @brief Returns ID_INVALID if no packets are available
 * @author mdunne */
unsigned char Protocol_ReadNextID(void) {
    if (RX_State_Machine.length > 0) {
        return RX_State_Machine.data[0];
    }
    return ID_INVALID;
}

/**
 * @Function int Protocol_GetPayload(void* payload)
 * @param payload, Memory location to put payload
 * @return SUCCESS or ERROR
 * @brief 
 * @author mdunne */
int Protocol_GetPayload(void* payload) {
    unsigned char i;
    for (i = 0; i < RX_State_Machine.length; i++) {
        ((unsigned char*) payload)[i] = RX_State_Machine.data[i+1];
    }
    return 1;
}

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
unsigned short Protocol_ShortEndednessConversion(unsigned short inVariable) {
    return (inVariable << 8) | (inVariable >> 8); // stackoverflow
}

/**
 * @Function char Protocol_IntEndednessConversion(unsigned int inVariable)
 * @param inVariable, int to convert endedness
 * @return converted int
 * @brief Converts endedness of a int. This is a bi-directional operation so only one function is needed
 * @author mdunne */
unsigned int Protocol_IntEndednessConversion(unsigned int inVariable) {
    inVariable = ((inVariable << 8) & 0xFF00FF00 ) | ((inVariable >> 8) & 0xFF00FF ); 
    return (inVariable << 16) | (inVariable >> 16); // stackoverflow
}

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
            RX_State_Machine.length = 0; // reset buffer
            state = LENGTH;
        }
    }
    else if (state == LENGTH) { // saves length of payload
        RX_State_Machine.length = charIn;
        state = ID;
    }
    else if (state == ID) { // checks for special IDs
        if (charIn == ID_LEDS_SET) {
            state = SETUPLEDS;
        }
        else if (charIn == ID_LEDS_GET) {
            state = TAILS;
        }
        else { // no special ID, start calculating checksum
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
            count = 0;
            RX_State_Machine.length = 0;
        }
    }
    else if (state == CHECKSUM) {
        if (ledState != LEDS_GET()) { // special case LED SET
            LEDS_SET(ledState);
        }
        else if (charIn == ID_LEDS_GET) { // special case LED GET
            unsigned char ledsget = LATE & 0xFF;
            Protocol_SendMessage(1, ID_LEDS_STATE, &ledsget);

        } 
        else if (checkSum == charIn) { // correct checksum
            checkSum = 0;
            state = START;
            count = 0;
            ready = 1;
        } else { // checksum is incorrect
            checkSum = 0;
            state = START;
            count = 0;
        }
    }
}

/**
 * @Function char PutChar(char ch)
 * @param ch, new char to add to the circular buffer
 * @return SUCCESS or ERROR
 * @brief adds to circular buffer if space exists, if not returns ERROR
 * @author mdunne */
int PutChar(char ch) {
    if (bufferFull() == 0) { // buffer not full
        bufferAdd(ch);
        while(U1STAbits.TRMT == 0); // wait for previous putchar call to finish
        IFS0bits.U1TXIF = 1; // cause interrupt
        //}
        if (U1STAbits.URXDA == 1) { // Receiver has data
            IFS0bits.U1RXIF = 1; // cause interrupt
        }
    }
}

/**
 * @Function void UART Interrupt
 * @param None
 * @return none
 * @brief called when one of two UART flags are raised, recieves and sends bits into buffers
 * @author rbox */
void __ISR(_UART1_VECTOR) IntUart1Handler(void) {
    
    if (IFS0bits.U1TXIF == 1) {
        while (bufferEmpty() == 0) { // not empty
            while(U1STAbits.TRMT == 0); // wait for previous transmission to finish
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

/**
 * @Function int bufferFull()
 * @param None
 * @return SUCCESS if buffer is full, FAIL if not full
 * @brief Checks if the circular buffer is full
 * @author rbox */
int bufferFull() {
    if ((circBuffer.tail + 1) % bufferLength == circBuffer.head) {
        return 1;
    }
    return 0;
}

/**
 * @Function void bufferAdd(char c)
 * @param char c, new char to add to the circular buffer
 * @return none
 * @brief if the buffer is not full it adds char to the buffer
 * @author rbox */
void bufferAdd(char c) {
    if (bufferFull() == 0) {
        circBuffer.data[circBuffer.tail] = c;
        circBuffer.tail = (circBuffer.tail + 1) % bufferLength;
    }
}

/**
 * @Function int bufferEmpty()
 * @param none
 * @return SUCCESS if buffer is empty, FAIL if not empty
 * @brief checks if the buffer is empty
 * @author rbox */
int bufferEmpty() {
    if (circBuffer.head == circBuffer.tail) {
        return 1;
    }
    return 0;
}

/**
 * @Function char bufferRemove()
 * @param none
 * @return char c removed from the circular buffer
 * @brief if buffer not empty, removes data at the head position
 * @author rbox */
char bufferRemove() {
    if (bufferEmpty() == 0) {
        char c = circBuffer.data[circBuffer.head];
        circBuffer.head = (circBuffer.head + 1) % bufferLength;
        return c;
    }
}

/**
 * @Function int bufferCount()
 * @param none
 * @return number if data in buffer
 * @brief calculates number of data in buffer
 * @author rbox */
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


//#define testHarness
#ifdef testHarness
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();
    
    char debugMessage[MAXPAYLOADLENGTH];
    sprintf(debugMessage, "Protocol Test Compiled at %s %s", __DATE__, __TIME__);
    Protocol_SendDebugMessage(debugMessage);
    
    //LED SET
    PutChar(0xCC);
    PutChar(0x09);
    PutChar(0x81);
    
    PutChar(0xCC);
    PutChar(0x02);
    PutChar(0x81);
    PutChar(0xFF);
    PutChar(0xB9);
    PutChar(0xBF);
    PutChar(0x0D);
    PutChar(0x0A);
    
    PutChar(0xB9);
    PutChar(0x74);
    PutChar(0x0D);
    PutChar(0x0A);
    //LED GET
    PutChar(0xCC);
    PutChar(0x01);
    PutChar(0x83);
    PutChar(0xB9);
    PutChar(0x83);
    PutChar(0x0D);
    PutChar(0x0A);
    while (1) {
        if (ready == 1) {
            unsigned char g[4];
            unsigned int i = 0;
            i = RX_State_Machine.data[4] | (RX_State_Machine.data[3] << 8) | (RX_State_Machine.data[2] << 16) | (RX_State_Machine.data[1] << 24); // converts char array to unsingned int
            i = i >> 1;
            g[3] = i; // converts unsinged int to char array
            g[2] = i >> 8;
            g[1] = i >> 16;
            g[0] = i >> 24;
            Protocol_SendMessage(RX_State_Machine.length-1, ID_PONG, &g);
            RX_State_Machine.length = 0; // resets RX buffer
            ready = 0; // reset flag variable
        }   
    }
}
#endif

//#define test
#ifdef test
int main() {
    BOARD_Init();
    LEDS_INIT();
    Protocol_Init();

    char str[1] = "1";
    Protocol_SendMessage(1, ID_PING, (&str));
    unsigned int s = 50;
    Protocol_SendMessage(1, ID_PONG, (&s));
    PutChar(0xCC);
    PutChar(0x05);
    PutChar(0x85);
    PutChar(0x00);
    PutChar(0x00);
    PutChar(0x66);
    PutChar(0x2C);
    PutChar(0xB9);
    PutChar(0x37);
    PutChar(0x0D);
    PutChar(0x0A);
            
    unsigned char t = 0x81;
    unsigned char check = 0;
    check = Protocol_CalcIterativeChecksum(t, check);
    t = 0x05;
    check = Protocol_CalcIterativeChecksum(t, check);
    t = 0x00;
    check = Protocol_CalcIterativeChecksum(t, check);
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
    circBuffer.data;
    while(1);
}
#endif







