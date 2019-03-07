
#include "../include/define.h"

//****************** serial ****************************************
//
//
//******************************************************************

#define USART_USE_BRGH_HIGH
#if defined(USART_USE_BRGH_LOW)
#define SPBRG_VAL   ( ((CLOCK_FREQ/BAUD_RATE)/64) - 1)
#else
#define SPBRG_VAL   ( ((CLOCK_FREQ/BAUD_RATE)/16) - 1)
#endif

#if SPBRG_VAL > 255
#error "Calculated SPBRG value is out of range for currnet CLOCK_FREQ."
#endif

char _serialInputString[MY_GATEWAY_MAX_RECEIVE_LENGTH];
char _serialInputPos;

struct { // Holds status bits
    unsigned SerialBufferFull : 1;
    unsigned q : 1;
    unsigned w : 1;
    unsigned e : 1;
    unsigned r : 1;
    unsigned t : 1;
    unsigned u : 1;
    unsigned i : 1;
} flag1;

void Console_init(void) {

    TRISCbits.TRISC7 = 1;
    TRISCbits.TRISC6 = 0;

#if defined(USART_USE_BRGH_HIGH)
    TXSTA1 = 0b00100100; //0x24;
#else
    TXSTA1 = 0x20;
#endif

    RCSTA1 = 0x90; // 0b10010000;
    BAUDCON1 = 0b01000000;
    SPBRG1 = 12;//7;//SPBRG_VAL;//8;//SPBRG_VAL;//7;//23; //SPBRG_VAL; 
}

//*********************************************************************

void ConsolePutROMString(const rom char* str) {
    unsigned char c;

    while (c = *str++)
        ConsolePut(c);

}

//*********************************************************************

void ConsolePutString(char *s) {
    unsigned char c;

    while (c = *s++)
        ConsolePut(c);
}

//*********************************************************************

void ConsolePut(char c) {
    while (!TXSTA1bits.TRMT);
    // RE = 1;
    DE = 1;
    TXREG = c;
    while (!TXSTA1bits.TRMT);
    DE = 0;
    // RE = 0;
}

//*********************************************************************

unsigned char ConsoleGet(void) {
    // Clear overrun error if it has occured
    // New unsigned chars cannot be received if the error occurs and isn't cleared
    if (RCSTA1bits.OERR) {
        RCSTA1bits.CREN = 0; // Disable UART receiver
        RCSTA1bits.CREN = 1; // Enable UART receiver
    }
    return RCREG1;
}

//**********************************************************************

void SerialINT(void) {

    if ((PIR1bits.RC1IF) && (PIE1bits.RC1IE)) {
        // Overrun error bit
        if (RCSTA1bits.OERR) {
            RCSTA1bits.CREN = 0; // Restart USART
            RCSTA1bits.CREN = 1; //
        }
        // If error ...
        if (RCSTA1bits.FERR) {
            // Flush buffer
            RCREG1 = RCREG1;
            RCREG1 = RCREG1;
            RCREG1 = RCREG1;
        } else {
            char inChar = RCREG1;

            if (!flag1.SerialBufferFull) {
                if (_serialInputPos < MY_GATEWAY_MAX_RECEIVE_LENGTH - 1) {
                    if (inChar == '\n') {
                        _serialInputString[_serialInputPos] = 0;
                        flag1.SerialBufferFull = 1;
                        _serialInputPos = 0;
                    } else {
                        _serialInputString[_serialInputPos] = inChar; // add it to the inputString
                        _serialInputPos++;
                    }
                } else {
                    _serialInputPos = 0; // Incoming message too long. Throw away
                }
            }
        }
        PIR1bits.RC1IF = 0;
    }
}

