#include "../include/define.h"

/****************************************************************************
  Function:
    void Delay10us( unsigned long int tenMicroSecondCounter )
  ***************************************************************************/
void Delay10us( unsigned long int tenMicroSecondCounter )
{
    volatile signed long int cyclesRequiredForEntireDelay;
    
        if (GetInstructionClock() <= 2500000) //for all FCY speeds under 2MHz (FOSC <= 10MHz)
        {
            //26 cycles burned through this path (includes return to caller).
            //For FOSC == 1MHZ, it takes 104us.
            //For FOSC == 4MHZ, it takes 26us
            //For FOSC == 8MHZ, it takes 13us.
            //For FOSC == 10MHZ, it takes 10.5us.
        }
        else
        {
            //14 cycles burned to this point.
            
            //We want to pre-calculate number of cycles required to delay 10us * tenMicroSecondCounter using a 1 cycle granule.
            cyclesRequiredForEntireDelay = (signed long int)(GetInstructionClock()/100000) * tenMicroSecondCounter;
            
            //We subtract all the cycles used up until we reach the while loop below, where each loop cycle count is subtracted.
            //Also we subtract the 22 cycle function return.
            cyclesRequiredForEntireDelay -= (153 + 22);
            
            if (cyclesRequiredForEntireDelay <= 45)
            {
                // If we have exceeded the cycle count already, bail! Best compromise between FOSC == 12MHz and FOSC == 24MHz.
            }    
            else
            {
                //Try as you may, you can't get out of this heavier-duty case under 30us. ;]
                
                while (cyclesRequiredForEntireDelay>0) //153 cycles used to this point.
                {
                    Nop(); //Delay one instruction cycle at a time, not absolutely necessary.
                    cyclesRequiredForEntireDelay -= 42; //Subtract cycles burned while doing each delay stage, 42 in this case.
                }
            }
        }
}

/****************************************************************************
  Function:
    void DelayMs( unsigned short int ms )

  Description:
    This routine performs a software delay in intervals of 1 millisecond.
  ***************************************************************************/
void DelayMs( unsigned short int ms )
{
        signed long int cyclesRequiredForEntireDelay;
        
        // We want to pre-calculate number of cycles required to delay 1ms, using a 1 cycle granule.
        cyclesRequiredForEntireDelay = (signed long)(GetInstructionClock()/1000) * ms;
        
        // We subtract all the cycles used up until we reach the while loop below, where each loop cycle count is subtracted.
        // Also we subtract the 22 cycle function return.
        cyclesRequiredForEntireDelay -= (148 + 22);

        if (cyclesRequiredForEntireDelay <= (170+25)) 
        {
            return;     // If we have exceeded the cycle count already, bail!
        }    
        else
        {
            while (cyclesRequiredForEntireDelay > 0) //148 cycles used to this point.
            {
                Nop();                              // Delay one instruction cycle at a time, not absolutely necessary.
                cyclesRequiredForEntireDelay -= 39; // Subtract cycles burned while doing each delay stage, 39 in this case.
            }
        }
}

