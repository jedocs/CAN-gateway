#ifndef  _CONSOLE_H_
#define  _CONSOLE_H_

/************************ HEADERS **********************************/
//#include "define.h"

#define BAUD_RATE 38400// 9600 //19200		//9600
 #define ConsoleIsGetReady()     (PIR1bits.RC1IF)
#define ConsoleIsPutReady()     (TXSTA1bits.TRMT)
/************************ DEFINITIONS ******************************/
/************************ FUNCTION PROTOTYPES **********************/

        void ConsoleInit(void);
        void ConsolePut(char c);
        void ConsolePutString(char *s);
        void ConsolePutROMString(const rom char *str);
        unsigned char ConsoleGet(void);
#endif


