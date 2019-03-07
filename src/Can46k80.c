#include "../include/define.h"

#ifdef CAN_ERROR_HANDLER_ENABLE
extern void CANErrorHandler(void);
#define CAN_INT_BITS 0x3F	//CAN interrupts which should be enabled, simply what PIE3 is to be loaded with (Note all TXB IE will be set regardless of this value)
#else
#define CAN_INT_BITS 0x1F	//CAN interrupts which should be enabled, simply what PIE3 is to be loaded with (Note all TXB IE will be set regardless of this value)
#endif

union RXBuffer { //Receive buffer structure
    unsigned char Data[14]; //It has to be a bit messy/strict for

    struct { //it to go trough the PICC18 compiler

        union {
            unsigned char Byte;

            struct {
                unsigned FILHIT0 : 1;
                unsigned JTOFF : 1;
                unsigned RXB0DBEN : 1;
                unsigned RXRTRRO : 1;
                unsigned : 1;
                unsigned RXM0 : 1;
                unsigned RXM1 : 1;
                unsigned RXFUL : 1;
            } Bits;
        } RXBCON;

        union {
            unsigned char Byte;
        } NODE_ID;

        union {
            unsigned char Byte;

            struct {
                unsigned ACK : 1;
                unsigned ACK_req : 1;
                unsigned : 1;
                unsigned EXID : 1;
                unsigned SRR : 1;
                unsigned MSG_TYP : 3;
            } Fields;
        } RXBSIDL;

        union {
            unsigned char Byte;
        } CHILD_ID;

        union {
            unsigned char Byte;
        } SUBTYPE;

        union {
            unsigned char Byte;

            struct {
                unsigned DLC0 : 1;
                unsigned DLC1 : 1;
                unsigned DLC2 : 1;
                unsigned DLC3 : 1;
                unsigned RB0 : 1;
                unsigned RB1 : 1;
                unsigned RXRTR : 1;
                unsigned : 1;
            } Bits;
        } RXBDLC;

        union {
            unsigned char Array[8];

            struct {
                unsigned char RXBD0;
                unsigned char RXBD1;
                unsigned char RXBD2;
                unsigned char RXBD3;
                unsigned char RXBD4;
                unsigned char RXBD5;
                unsigned char RXBD6;
                unsigned char RXBD7;
            } Bytes;
        } RXBD;
    } Specific;
};

union TXBuffer { //Transmit buffer structure
    unsigned char Data[14];

    struct {

        union {
            unsigned char Byte;

            struct {
                unsigned TXPRI0 : 1;
                unsigned TXPRI1 : 1;
                unsigned : 1;
                unsigned TXREQ : 1;
                unsigned TXERR : 1;
                unsigned TXLARB : 1;
                unsigned TXABT : 1;
            } Bits;
        } TXBCON;

        union {
            unsigned char Byte;
        } NODE_ID;

        union {
            unsigned char Byte;

            struct {
                unsigned ACK : 1;
                unsigned ACK_req : 1;
                unsigned : 1;
                unsigned EXIDE : 1;
                unsigned : 1;
                unsigned MSG_TYP : 3;
            } Fields;
        } TXBSIDL;

        union {
            unsigned char Byte;
        } CHILD_ID;

        union {
            unsigned char Byte;
        } SUBTYPE;

        union {
            unsigned char Byte;

            struct {
                unsigned DLC0 : 1;
                unsigned DLC1 : 1;
                unsigned DLC2 : 1;
                unsigned DLC3 : 1;
                unsigned : 1;
                unsigned : 1;
                unsigned TXRTR : 1;
                unsigned : 1;
            } Bits;
        } TXBDLC;

        union {
            unsigned char Array[8];

            struct {
                unsigned char TXBD0;
                unsigned char TXBD1;
                unsigned char TXBD2;
                unsigned char TXBD3;
                unsigned char TXBD4;
                unsigned char TXBD5;
                unsigned char TXBD6;
                unsigned char TXBD7;
            } Bytes;
        } TXBD;
    } Specific;
};

volatile union RXBuffer RXMessage[RXBUF]; //Received messages FIFO buffer
volatile union TXBuffer TXMessage[TXBUF]; //Pending messages to transmit FIFO buffer
volatile char RXRPtr = 0; //Read pointer for RXMessage buffer
volatile char RXWPtr = 0; //Write pointer for RXMessage buffer
volatile char TXRPtr = 0; //Read pointer for TXMessage buffer
volatile char TXWPtr = 0; //Write pointer for TXMessage buffer

/*********************************************************************
 * Function:        void CANISR(void)
 *
 * Side Effects:    Will modify the RX/TX interrupt flags
 *                  and interrupt enable bits
 *
 * Overview:        Checks if a CAN reception/transmission was complete
 *                  and if so write/read to the CAN RX/TX FIFO buffers
 *
 ********************************************************************/
void CANISR(void) {

    unsigned char TempCANCON;

    if (PIR5 & PIE5) {
        TempCANCON = CANCON;

        if (RXB0IF && RXB0IE) {
            RXB0IF = 0; //Clear interrupt flag
            CANCON = CANCON & 0xF0 | RXB0Interrupt;
            CANGetMessage();
        } else if (RXB1IF && RXB1IE) {
            RXB1IF = 0; //Clear interrupt flag
            CANCON = CANCON & 0xF0 | RXB1Interrupt;
            CANGetMessage();
        } else if (TXB0IF && TXB0IE) {
            CANCON = CANCON & 0xF0 | TXB0Interrupt;
           
            if (CANPutMessage()) {
                TXB0IE = 0;
            } else
                TXB0IF = 0;
        } else if (TXB1IF && TXB1IE) {
            CANCON = CANCON & 0xF0 | TXB1Interrupt;
            if (CANPutMessage())
                TXB1IE = 0;
            else
                TXB1IF = 0;
        } else if (TXB2IF && TXB2IE) {
            CANCON = CANCON & 0xF0 | TXB2Interrupt;
            if (CANPutMessage()) //if there wasn't any more messages to send
                TXB2IE = 0; //disable interrupts for TXB2 and leave TXB2IF
            else //still on so PutCAN() can reenable the interrupt and instantly vector to ISR
                TXB2IF = 0; //message was sent, just clear the interrupt flag.
        } else if (ERRIF && ERRIE) {

            ERRIF = 0; //Clear interrupt flag
#ifdef CAN_ERROR_HANDLER_ENABLE
            CANErrorHandler();
#endif
            //No error handler implemented!!!
        } else if (WAKIF && WAKIE) {
            WAKIF = 0; //Clear interrupt flag
        }

        CANCON = TempCANCON;
    }

}

/*********************************************************************
 * Function:        char CANOpen(void)
 *
 * Input:           Values to be written into BRGCON1 -> BRGCON3
 *
 * Output:          0 -> Initialasation succeeded
 *
 * Overview:        Sets up the appropriate register for the device to act
 *                  as a CAN node
 *
 ********************************************************************/
char CAN_init(unsigned char CONFIG1, unsigned char CONFIG2, unsigned char CONFIG3) {

    TRISB2 = 0;
    TRISB3 = 1;
    PIE5 = 0; //Disable all CAN interrupts
    PIR5 = 0; //and clear all CAN interrupt flags
    CANCON = (CONFIG_MODE & MODE_MASK) | (CANCON & 0x3F); //Set configuration mode
    while ((CANSTAT & MODE_MASK) != (CONFIG_MODE & MODE_MASK)); //Wait until config mode is set

    BRGCON1 = 0x05;//CONFIG1;
    BRGCON2 = 0xba;//CONFIG2;
    BRGCON3 = 0x07;//CONFIG3;

    RXB0CON = 0b01000100; //Receive only ext. id. valid messages, receive buffer overflow
    RXB1CON = 0b01000000; //receive only ext. id valid msg

    //Set the acceptance filters for all the filters
    RXF0EIDL = 0;
    RXF0EIDH = 0;
    RXF0SIDL = 8;
    RXF0SIDH = NODEID;
    RXF1EIDL = 0;
    RXF1EIDH = 0;
    RXF1SIDL = 8;
    RXF1SIDH = NODEID;
    RXF2EIDL = 0;
    RXF2EIDH = 0;
    RXF2SIDL = 8;
    RXF2SIDH = BROADCASTID;
    RXF3EIDL = 0;
    RXF3EIDH = 0;
    RXF3SIDL = 8;
    RXF3SIDH = BROADCASTID;
    RXF4EIDL = 0;
    RXF4EIDH = 0;
    RXF4SIDL = 8;
    RXF4SIDH = NODEID;
    RXF5EIDL = 0;
    RXF5EIDH = 0;
    RXF5SIDL = 8;
    RXF5SIDH = NODEID;


    //Set the acceptance masks
    RXM0EIDL = 0; //(unsigned char) (ACCEPTANCE_MASK_0_1 & 0xFF);
    RXM0EIDH = 0; //(unsigned char) ((ACCEPTANCE_MASK_0_1 >> 8) & 0xFF);
    RXM0SIDL = 0; //(unsigned char) ((ACCEPTANCE_MASK_0_1 >> 16) & 0x03) | (unsigned char) ((ACCEPTANCE_MASK_0_1 >> 13) & 0xE0) | 0x08;
    RXM0SIDH = 0; //255; //(unsigned char) ((ACCEPTANCE_MASK_0_1 >> 21) & 0xFF);
    RXM1EIDL = 0; //(unsigned char) (ACCEPTANCE_MASK_2_5 & 0xFF);
    RXM1EIDH = 0; //(unsigned char) ((ACCEPTANCE_MASK_2_5 >> 8) & 0xFF);
    RXM1SIDL = 0; //(unsigned char) ((ACCEPTANCE_MASK_2_5 >> 16) & 0x03) | (unsigned char) ((ACCEPTANCE_MASK_2_5 >> 13) & 0xE0) | 0x08;
    RXM1SIDH = 0; //; //(unsigned char) ((ACCEPTANCE_MASK_2_5 >> 21) & 0xFF);

    CIOCON = 0x21; //clk sel!! Drive TX pin Vdd when recessive, disable capture !!!!!!21!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!4

    CANCON = (NORMAL_MODE & MODE_MASK) | (CANCON & (MODE_MASK ^ 0xFF)); //Set normal mode
    while ((CANSTAT & MODE_MASK) != (NORMAL_MODE & MODE_MASK)); //Wait until normal mode is set

    //IPR3 = 0xFF;  //priority, set in main
    PIE5 = CAN_INT_BITS & 0xE3; //Enable CAN interrupts except TX interrupts
    PIR5 = 0x18; //Set TXB1 & TXB2 int flags

    return 0;
}

/*********************************************************************
 * Function:        void CANGetMessage(void)
 *
 * PreCondition:    <WIN2:WIN0> in the CANCON register has to set
 *                  to reflect the desired RXB registers
 *
 * Side Effects:    Will modify the RX FIFO Write pointer (RXWPtr)
 *
 * Overview:        Gets the registers for a RXB and puts them in the
 *                  CAN Receive buffer
 *
 * Note:            Care is not taken if buffer is full
 ********************************************************************/
void CANGetMessage(void) {
    unsigned char i;
    if (++RXWPtr >= RXBUF) //If pointer overflowed
        RXWPtr = 0; //Clear it

    for (i = 0; i < 14; i++) {
        RXMessage[RXWPtr].Data[i] = *(unsigned char *) (0xF60 + i);
    }
    RXB0FUL = 0;
}

/*********************************************************************
 * Function:        char CANPutMessage(void)
 *
 * PreCondition:    <WIN2:WIN0> in the CANCON register has to set
 *                  to reflect the desired TXB registers
 *
 * Output:          0 -> A new message has been put in the transmit queue
 *                  1 -> There was no messages in the TX buffer to send
 *
 * Side Effects:    Will modify the TX buffer´s Read pointer (TXRPtr)
 *
 * Overview:        Checks if there is any messages to transmit and if so
 *                  place it in the registers reflected by <WIN2:WIN0>
 *
 ********************************************************************/
char CANPutMessage(void) {
    unsigned char i;
    if (TXWPtr != TXRPtr) //If there are any more data to send
    {
        if (++TXRPtr >= TXBUF) //then increment the TX read pointer
            TXRPtr = 0;
        for (i = 0; i < 14; i++)
            *(unsigned char *) (0xF60 + i) = TXMessage[TXRPtr].Data[i];
        RXB0RTRRO = 1; //a kontrollregiszterek ablakolása miatt ez a TXBnCON.TXREQ-nak felel meg
        return 0;
    } else
        return 1;
}

/*********************************************************************
 * Function:        char CANPut(struct CANMessage Message)
 *
 * Input:           A CAN message
 *
 * Output:          1 -> Failed to put a CAN on the buffer, buffer is full
 *                  0 -> The CAN message is put on the buffer and will be
 *                       transmitted eventually
 *
 * Side Effects:    Will modify the TX Buffer register´s Write pointer
 *
 * Overview:        Initially checks if at least one buffer slot is available
 *                  and if so push the requested message in the buffer.
 *                  Checks if the TX modules are idle and if they are, reactivate one.
 ********************************************************************/
char CANPut(struct CANMessage Message) {
    unsigned char TempPtr, i;


    if (TXWPtr == TXRPtr - 1 || (TXWPtr == TXBUF - 1 && TXRPtr == 0)) //if all transmit buffers are full return 1
        return 1;

    //Do not modify the TX pointer until the message has been successfully copied so the CANISR don't
    //send a message that isn't ready
    if (TXWPtr >= TXBUF - 1) //check if transmit write pointer will overflow
    {
        TempPtr = 0; //and post clear write pointer
    } else {
        TempPtr = TXWPtr + 1; //and postincrement write pointer
    }

    if (Message.NoOfBytes > 8) //Ensure we don't handle more than 8 bytes
        Message.NoOfBytes = 8;

    TXMessage[TempPtr].Specific.TXBDLC.Byte = Message.NoOfBytes; //Set the data count

    if (!Message.Remote) //If dataframe
    {
        TXMessage[TempPtr].Specific.TXBDLC.Bits.TXRTR = 0; //Clear the Remote Transfer Request bit

        for (i = 0; i < Message.NoOfBytes; i++) //Load data registers
        {
            TXMessage[TempPtr].Specific.TXBD.Array[i] = Message.Data[i];
        }
    } else //Remote frame
    {
        TXMessage[TempPtr].Specific.TXBDLC.Bits.TXRTR = 1; //Set TXRTR bit
    }

    TXMessage[TempPtr].Specific.NODE_ID.Byte = Message.NODE_ID; //Put address <10:3> in SIDH
    TXMessage[TempPtr].Specific.CHILD_ID.Byte = Message.CHILD_ID;
    TXMessage[TempPtr].Specific.TXBSIDL.Fields.MSG_TYP = Message.MSG_TYP; //Put address <2:0> in SIDL
    TXMessage[TempPtr].Specific.SUBTYPE.Byte = Message.SUBTYPE;
    TXMessage[TempPtr].Specific.TXBSIDL.Fields.EXIDE = 1;
    TXMessage[TempPtr].Specific.TXBSIDL.Fields.ACK = Message.ACK; //********************************************************************
    TXMessage[TempPtr].Specific.TXBSIDL.Fields.ACK_req = 0; //*************************************************************************
    TXMessage[TempPtr].Specific.TXBCON.Byte = Message.Priority & 0x03; //Set the internal priority of the data frame
    TXWPtr = TempPtr;

    //Reenable an interrupt if it is idle, it doesn't matter if another TX source caught the pending message
    //before the source that is enabled here does since the interrupt CHECKS if there are any messages pending
    //and if the buffer is empty, it will disable itself again.
    if (!TXB0IE) //TXB0 is idle
        TXB0IE = 1; //enable TXB0 to fire an interrupt to send the pending message
    else if (!TXB1IE) //else if TXB1 is idle
        TXB1IE = 1; //same procedure
    else if (!TXB2IE) //finally try TXB2
        TXB2IE = 1;

    return 0;
}

/*********************************************************************
 * Function:        char CANRXMessageIsPending(void)
 *
 * Output:          1 -> At least one received message is pending in the RX buffer
 *                  0 -> No received messages are pending
 *
 * Overview:        Checks if the RX Write pointer is equal to RX Read pointer and
 *                  if so returns 0, else returns 1
 *
 * Note:            Since no care is taken if the buffer overflow
 *                  this function has to be polled frequently to
 *                  prevent a software receive buffer overflow
 ********************************************************************/
char CANRXMessageIsPending(void) {
    if (RXWPtr != RXRPtr)
        return 1;
    else
        return 0;
}

/*********************************************************************
 * Function:        struct CANMessage CANGet(void)
 *
 * PreCondition:    An unread message has to be in the buffer
 *                  use RXCANMessageIsPending(void) prior to
 *                  calling this function in order to determine
 *                  if an unread message is pending.
 *
 * Output:          The received message
 *
 * Side Effects:    Will modify the RX Buffer register´s Read pointer
 *
 * Overview:        Pops the the first message of the RX buffer
 *
 ********************************************************************/
struct CANMessage CANGet(void) {
    struct CANMessage ReturnMessage;
    unsigned char TempPtr, i;

    /*Do not modify the RX pointer until the message has been successfully
      copied to prevent ISR to overwrite it (Note. this is not implemented in the ISR yet)*/
    if (RXRPtr >= RXBUF - 1) //Check if pointer will overflow
    {
        TempPtr = 0;
    } else {
        TempPtr = RXRPtr + 1;
    }

    ReturnMessage.NoOfBytes = RXMessage[TempPtr].Specific.RXBDLC.Byte & 0x0F; //copy data count
    if (RXMessage[TempPtr].Specific.RXBCON.Bits.RXRTRRO) //Remote frame request
    {
        ReturnMessage.Remote = 1;
    } else //Data frame
    {
        ReturnMessage.Remote = 0; //Clear remote flag
        for (i = 0; i < ReturnMessage.NoOfBytes; i++) //copy data content
            ReturnMessage.Data[i] = RXMessage[TempPtr].Specific.RXBD.Array[i];
    }

    ReturnMessage.NODE_ID = RXMessage[TempPtr].Specific.NODE_ID.Byte; //Load the standard identifier into the address
    ReturnMessage.CHILD_ID = RXMessage[TempPtr].Specific.CHILD_ID.Byte;
    ReturnMessage.MSG_TYP = RXMessage[TempPtr].Specific.RXBSIDL.Fields.MSG_TYP; //Load the standard identifier into the address
    ReturnMessage.SUBTYPE = RXMessage[TempPtr].Specific.SUBTYPE.Byte;
    ReturnMessage.ACK = RXMessage[TempPtr].Specific.RXBSIDL.Fields.ACK;

    CANCON = CANCON & 0xF0;
    // ReturnMessage.Ext = 0; //clear the extended identifier flag

    RXRPtr = TempPtr;
    return ReturnMessage;
}

/*********************************************************************
 * Function:        void CANSetMode(unsigned char Mode)
 *
 * Overview:        Requests to set the desired mode and waits until
 *                  the mode has been set.
 ********************************************************************/
void CANSetMode(unsigned char Mode) {
    CANCON = (Mode & MODE_MASK) | (CANCON & (MODE_MASK ^ 0xFF));
    while ((CANSTAT & MODE_MASK) != (Mode & MODE_MASK));
}
