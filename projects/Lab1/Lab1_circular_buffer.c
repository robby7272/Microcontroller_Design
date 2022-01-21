// **** Include libraries here ****
// Standard libraries
#include <stdio.h>
#include "Protocol.h"

//CSE13E Support Library

#define bufferLength 12


static struct {
    unsigned int head;
    unsigned int tail;
    char data[bufferLength];
} circBuffer;

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

int main()
{
    /*
    char c = 'H';
    char d = 'E';
    for (int i = 0; i < 10; ++i) {
        bufferAdd(c);
    }
    for (int i = 0; i < 5; ++i) {
        bufferAdd(d);
    }
    for (int i = 0; i < 4; ++i) {
        char z = bufferRemove();
        char x = circBuffer.data[0];
    }
    for (int i = 0; i < 5; ++i) {
        bufferAdd(d);
        char x = circBuffer.data[0];
    }
    bufferAdd(c);
    char x = circBuffer.data[0];
    //int y = circBuffer.tail;
     */
    unsigned char charIn = 'f';
    unsigned char curChecksum = 0;
    curChecksum = Protocol_CalcIterativeChecksum(charIn, curChecksum);
    curChecksum = Protocol_CalcIterativeChecksum(charIn, curChecksum);
    curChecksum = Protocol_CalcIterativeChecksum(charIn, curChecksum);
    int x = 0;
    int y = 5;
}
