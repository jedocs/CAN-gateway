// PIC18F46K80 Configuration Bit Settings

// 'C' source line config statements

#include <p18F46K80.h>

// CONFIG1L
#pragma config RETEN = OFF      // VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL = HIGH   // SOSC Power Selection and mode Configuration bits (High Power SOSC circuit selected)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC = HS2       // 
#pragma config PLLCFG = OFF     // PLL x4 Enable bit (Disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power Up Timer (Enabled)
#pragma config BOREN = SBORDIS  // Brown Out Detect (Enabled in hardware, SBOREN disabled)
#pragma config BORV = 3         // Brown-out Reset Voltage bits (1.8V)
#pragma config BORPWR = ZPBORMV // BORMV Power level (ZPBORMV instead of BORMV is selected)

// CONFIG2H
#pragma config WDTEN = SWDTDIS  // Watchdog Timer (WDT enabled in hardware; SWDTEN bit disabled)
#pragma config WDTPS = 1048576  // Watchdog Postscaler (1:1048576)

// CONFIG3H
#pragma config CANMX = PORTB    // ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK = MSK7   // MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE = ON       // Master Clear Enable (MCLR Enabled, RE3 Disabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ = BB2K     // Boot Block Size (2K word Boot Block size)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protect 00800-03FFF (Disabled)
#pragma config CP1 = OFF        // Code Protect 04000-07FFF (Disabled)
#pragma config CP2 = OFF        // Code Protect 08000-0BFFF (Disabled)
#pragma config CP3 = OFF        // Code Protect 0C000-0FFFF (Disabled)

// CONFIG5H
#pragma config CPB = OFF        // Code Protect Boot (Disabled)
#pragma config CPD = OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 = OFF       // Table Write Protect 00800-03FFF (Disabled)
#pragma config WRT1 = OFF       // Table Write Protect 04000-07FFF (Disabled)
#pragma config WRT2 = OFF       // Table Write Protect 08000-0BFFF (Disabled)
#pragma config WRT3 = OFF       // Table Write Protect 0C000-0FFFF (Disabled)

// CONFIG6H
#pragma config WRTC = OFF       // Config. Write Protect (Disabled)
#pragma config WRTB = OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD = OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protect 00800-03FFF (Disabled)
#pragma config EBTR1 = OFF      // Table Read Protect 04000-07FFF (Disabled)
#pragma config EBTR2 = OFF      // Table Read Protect 08000-0BFFF (Disabled)
#pragma config EBTR3 = OFF      // Table Read Protect 0C000-0FFFF (Disabled)

// CONFIG7H
#pragma config EBTRB = OFF      // Table Read Protect Boot (Disabled)

//****************************************************************

#include "../include/define.h"

extern struct { // Holds status bits
    unsigned SerialBufferFull : 1;
    unsigned q : 1;
    unsigned w : 1;
    unsigned e : 1;
    unsigned r : 1;
    unsigned t : 1;
    unsigned u : 1;
    unsigned i : 1;
} flag1;

typedef enum {
    nodeid = 0,
    semicolon1 = 1,
    childid = 2,
    semicolon2 = 3,
    msgtype = 4,
    semicolon3 = 5,
    ack = 6,
    semicolon4 = 7,
    subtype = 8,
    semicolon5 = 9,
    data = 10
} rxstate;

struct CANMessage CAN_RX_Message, CAN_TX_Message;
struct SerialMessage Serial_Message;
extern char _serialInputString[MY_GATEWAY_MAX_RECEIVE_LENGTH];
char _data[20];
union Long_Char value;

//---------------------------------------------------------------------´
// Interrupt Code
//---------------------------------------------------------------------

#pragma interruptlow LowISR //interruptLOW!!!!!!!!!!!!!!!!!!!!!

void LowISR(void) {
    if (PIR5) CANISR();
}

#pragma interrupt HighISR

void HighISR(void) {
    if (PIR1bits.RC1IF) SerialINT();
}
/***********************************************************************/

#pragma code highVector=0x08

void HighVector(void) {
    _asm goto HighISR _endasm
}

#pragma code lowVector=0x18

void lowVector(void) {
    _asm GOTO LowISR _endasm
}

#pragma code /* return to the default code section */
//---------------------------------------------------------------------
//	Setup() initializes program variables and peripheral registers
//---------------------------------------------------------------------

void Setup(void) {

    Serial_Message.Data[8] = 0;
    CAN_RX_Message.Data[8] = 0;
    CAN_TX_Message.Data[8] = 0;

    RCON = 0b11010011; //interrupt priority, RESET flags

    TRISA = 0b11111111; // all input
    TRISB = 0b11111011;
    PORTB = 0b00000000;
    TRISC = 0b10110011;
    PORTC = 0b00000000;
    TRISD = 0b11111100;

    CM1CON = 0; //comparator off
    CM2CON = 0; //comparator off

    ADCON0 = 0; // A/D off
    ADCON1 = 0; // an0-7 analog, an 8-10 dig 46k80-nál más!!!!!!!!!!
    ADCON2 = 0;
    ANCON0 = 0; //dig inputs
    ANCON1 = 0; // dig inputs

    DE = 0; // !!!!!!!!!!!!!!!!!!!!!!!!  1 volt !!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //RE = 0;

    PIE1 = 0; //peripherial interrupt disabled
    PIE2 = 0; //peripherial interrupt disabled
    PIE5 = 0; //peripherial interrupt disabled,CAN int a CANOpen()-ben eng.ve

    RCONbits.IPEN = 1; //int priority enabled
    IPR1bits.TMR2IP = 1; // TMR2 priority: high
    PIE1bits.TMR2IE = 0; // disable TMR2 int
    INTCON2bits.INTEDG1 = 0; //INT1 int on falling edge
    INTCON3bits.INT1IP = 0; //INT1 priority low
    INTCON3bits.INT1IE = 0; //disable INT1 int (RF module)!!!!!!!!!!!!!!!!!!!!!4
    IPR5 = 0; //CAN int priority low
    IPR1bits.RC1IP = 1; // USART INT priority high
    PIE1bits.RC1IE = 1; // USART RX INT enable
    INTCONbits.GIEH = 1; //dis HIGH int
    INTCONbits.GIEL = 1; //enable LOW int
}
//---------------------------------------------------------------------
// main()
//---------------------------------------------------------------------

void main(void) {

    char index, ack, NodeId, ChldId, MsgTyp, SubTyp;

    Setup();
    Console_init();
    CAN_init(CAN_CONFIG_1, CAN_CONFIG_2, CAN_CONFIG_3); //Initialize CAN module

    //if (CANRXMessageIsPending()) CAN_RX_Message = CANGet(); //Get the message
    //if (CANRXMessageIsPending()) CAN_RX_Message = CANGet(); //Get the message

    //************************* presentation of local devides ***********************************

    NodeId = NODEID;
    ChldId = 255;
    MsgTyp = C_INTERNAL;
    SubTyp = I_GATEWAY_READY;
    ack = 0;

    btoa(NodeId, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");
    btoa(ChldId, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");
    btoa(MsgTyp, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");
    btoa(ack, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");
    btoa(SubTyp, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";Gateway startup complete\n");

    CAN_TX_Message.NODE_ID = Serial_Message.NODE_ID;
    CAN_TX_Message.MSG_TYP = Serial_Message.MSG_TYP;
    CAN_TX_Message.SUBTYPE = Serial_Message.SUBTYPE;
    CAN_TX_Message.ACK = Serial_Message.ACK;
    CAN_TX_Message.Remote = 0; //clear the remote flag
    CAN_TX_Message.Priority = 0;
    CAN_TX_Message.CHILD_ID = Serial_Message.CHILD_ID;
    CAN_TX_Message.NoOfBytes = 8; //RF_RxPacketLen;	//Set number of bytes to send

    for (index = 0; index < 8; index++) {
        CAN_TX_Message.Data[index] = (40 + index);
    }

    CANPut(CAN_TX_Message);

    //CANPut(CAN_TX_Message);
    //ConsolePutROMString("\r\nCAN ment\r\n");



    //********************** presentation end ******************


    //CANPut(CAN_TX_Message);CANPut(CAN_TX_Message);CANPut(CAN_TX_Message);

    while (1) {

        if (CANRXMessageIsPending()) CANRx();

        if (flag1.SerialBufferFull) {

            if (protocolParse()) {

                CAN_TX_Message.NODE_ID = Serial_Message.NODE_ID;
                CAN_TX_Message.MSG_TYP = Serial_Message.MSG_TYP;
                CAN_TX_Message.SUBTYPE = Serial_Message.SUBTYPE;
                CAN_TX_Message.ACK = Serial_Message.ACK;
                CAN_TX_Message.Remote = 0; //clear the remote flag
                CAN_TX_Message.Priority = 0;
                CAN_TX_Message.CHILD_ID = Serial_Message.CHILD_ID;
                CAN_TX_Message.NoOfBytes = 8; //RF_RxPacketLen;	//Set number of bytes to send

                for (index = 0; index < 8; index++) {
                    CAN_TX_Message.Data[index] = Serial_Message.Data[index];
                }

                CANPut(CAN_TX_Message);
            }
        }
    }
}

/****************************************************************************/

void CANRx(void) {

    char ii;

    CAN_RX_Message = CANGet(); //Get the message

    itoa(CAN_RX_Message.NODE_ID, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");

    itoa(CAN_RX_Message.CHILD_ID, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");

    btoa(CAN_RX_Message.MSG_TYP, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");

    btoa(CAN_RX_Message.ACK, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");

    itoa(CAN_RX_Message.SUBTYPE, _data);
    ConsolePutString(_data);
    ConsolePutROMString(";");

    if ((CAN_RX_Message.MSG_TYP == C_SET) && ((CAN_RX_Message.SUBTYPE == V_VOLUME) | (CAN_RX_Message.SUBTYPE == V_KWH))) {

        value.Char[0]=CAN_RX_Message.Data[0];
        value.Char[1]=CAN_RX_Message.Data[1];
        value.Char[2]=CAN_RX_Message.Data[2];
        value.Char[3]=CAN_RX_Message.Data[3];
        ltoa(value.Long, _data);
        ConsolePutString(_data);
        ConsolePutROMString("\n");

    } else {
        for (ii = 0; ii < CAN_RX_Message.NoOfBytes; ii++) ConsolePut(CAN_RX_Message.Data[ii]);
        ConsolePutROMString("\n");
    }
}

/***********************************************************************/

void CANACK(char ack) {

    //    CAN_TX_Message.NoOfBytes = 1; //Set number of bytes to send
    //  CAN_TX_Message.Remote = 0;
    // CAN_TX_Message.Priority = 0; //Internal CAN module priority 0-> least priority, 3-> most priority
    // CAN_TX_Message.Data[0] = ack;
    // CANPut(CAN_TX_Message); //Put the message in the FIFO
}

/*************************************************************************/

void CANErrorHandler(void) {
}

/****************************************************************************/

char protocolParse(void) {
    char *str, *value = NULL;
    unsigned char lastCharacter, i = 0;
    const char s[2] = ";"; //!!!!!!!!!!!!!! csak így jó !!!!!!!!4

    for (str = strtok(_serialInputString, s); str && i < 6; str = strtok(NULL, s)) // loop while str is not null an max 5 times// get subsequent tokens
    {
        switch (i) {
            case 0:
                Serial_Message.NODE_ID = atoi(str);
                break;
            case 1: // Childid
                Serial_Message.CHILD_ID = atoi(str);
                break;
            case 2: // Serial_Message type
                Serial_Message.MSG_TYP = atoi(str);
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!       mSetCommand(Serial_Message, command);
                break;
            case 3: // Should we request ack from destination?

                if (atoi(str)) {
                    Serial_Message.ACK = 1;
                } else {
                    Serial_Message.ACK = 0;
                }
                break;

            case 4: // Data type
                Serial_Message.SUBTYPE = atoi(str);
                break;
            case 5: // Variable value
                value = str;
                // Remove trailing carriage return and newline character (if it exists)
                lastCharacter = strlen(value) - 1;
                if (value[lastCharacter] == '\r')
                    value[lastCharacter] = 0;
                if (value[lastCharacter] == '\n')
                    value[lastCharacter] = 0;

                break;
        }
        i++;
    }
    if (i < 5) {// Check for invalid input
        flag1.SerialBufferFull = 0;
        return 0;
    }
    //!!!!!!!!!!!!!!444 mSetRequestAck(Serial_Message, ack ? 1 : 0);
    //!!!!!!!!!!!!!!!!  mSetAck(Serial_Message, false);

    memcpy(&Serial_Message.Data, value, 8);
    flag1.SerialBufferFull = 0;
    return 1;
}





/*char* MyMessage::getString(char *buffer) const {
        uint8_t payloadType = miGetPayloadType();
        if (buffer != NULL) {
                if (payloadType == P_STRING) {
                        strncpy(buffer, data, miGetLength());
                        buffer[miGetLength()] = 0;
                } else if (payloadType == P_BYTE) {
                        itoa(bValue, buffer, 10);
                } else if (payloadType == P_INT16) {
                        itoa(iValue, buffer, 10);
                } else if (payloadType == P_UINT16) {
                        utoa(uiValue, buffer, 10);
                } else if (payloadType == P_LONG32) {
                        ltoa(lValue, buffer, 10);
                } else if (payloadType == P_ULONG32) {
                        ultoa(ulValue, buffer, 10);
                } else if (payloadType == P_FLOAT32) {
                        dtostrf(fValue,2,min(fPrecision, 8),buffer);
                } else if (payloadType == P_CUSTOM) {
                        return getCustomString(buffer);
                }
                return buffer;
        } else {
                return NULL;
        }
}
 */
