#include <xc.h>
#include <p18f4620.h>
#include "Interrupt.h"

extern unsigned char Nec_state;
extern short nec_ok;
extern char Nec_code1;
unsigned long long Nec_code;

unsigned char bit_count;
unsigned int Time_Elapsed;


void init_INTERRUPT()
{
    INTCONbits.INT0IF = 0;                  // Clear external interrupt
    INTCON2bits.INTEDG0 = 0;                // Edge programming for INT0 falling edge H to L
    INTCONbits.INT0IE = 1;                  // Enable external interrupt
    TMR1H = 0;                              // Reset Timer1
    TMR1L = 0;                              //
    PIR1bits.TMR1IF = 0;                    // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;                    // Enable Timer 1 interrupt
    INTCONbits.PEIE = 1;                    // Enable Peripheral interrupt
    INTCONbits.GIE = 1; 
}

void interrupt high_priority chkisr() 
{
                                        //Here we handle the Interrupt bit and setup the timer1 bits
    if (PIR1bits.TMR1IF == 1) TIMER1_isr();
    if (INTCONbits.INT0IF == 1) INT0_isr();
}

void TIMER1_isr(void)
{
    Nec_state = 0;                          // Reset decoding process
    INTCON2bits.INTEDG0 = 0;                // Edge programming for INT0 falling edge
    T1CONbits.TMR1ON = 0;                   // Disable T1 Timer
    PIR1bits.TMR1IF = 0;                    // Clear interrupt flag
}

void force_nec_state0()
{
    Nec_state=0;                            //Make Nec_state 0
    T1CONbits.TMR1ON = 0;                   //Disable T1 Timer
}

void INT0_isr() 
{    
    INTCONbits.INT0IF = 0;                  // Clear external interrupt
    if (Nec_state != 0)
    {
        Time_Elapsed = (TMR1H << 8) | TMR1L; // Store Timer1 value
        TMR1H = 0;                          // Reset Timer1
        TMR1L = 0;
    }
    switch(Nec_state)
    {
        case 0 :
        {
            TMR1H = 0;                      // Reset Timer1
            TMR1L = 0;                      //
            PIR1bits.TMR1IF = 0;            //Clear TMR 1 Interrupt flag
            T1CON= 0x90;                    // Program Timer1 mode with count = 1usec using System clock running at 8Mhz
            T1CONbits.TMR1ON = 1;           // Enable Timer 1
            bit_count = 0;                  // Force bit count (bit_count) to 0
            Nec_code = 0;                   // Set Nec_code = 0
            Nec_state = 1;                  // Set Nec_State to state 1
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to Low to High            
            return;
        }
        case 1 :
        {
            if(Time_Elapsed > 8500 && Time_Elapsed < 9500)  //Time in microseconds
                Nec_state = 2;                              //if time elapsed is between 8500 and 9500 microseconds
                                                            //then set Nec_state to 2
            else
                force_nec_state0();                         //else go back to Nec_state 0
            INTCON2bits.INTEDG0 = 0;        // Change Edge interrupt of INT0 to High to Low
            return;
        }
        
        case 2 :
        {
            if(Time_Elapsed > 4000 && Time_Elapsed < 5000)  //Time in microseconds
                Nec_state = 3;                              //if time elapsed is between 4000 and 5000 microseconds
                                                            //then set Nec_state to 3
            else
                force_nec_state0();                         //else go back to Nec state 0
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to Low to High
            return;
        }
        
        case 3 :
        {
            if(Time_Elapsed > 400 && Time_Elapsed < 700)  //Time in microseconds
                Nec_state = 4;                            //if time elapsed is between 400 and 700 microseconds
                                                          //then set Nec_state to 4
            else
                force_nec_state0();                       //else go back to Nec state 0
            INTCON2bits.INTEDG0 = 0;        // Change Edge interrupt of INT0 to High to Low
            return;
        }
        
        case 4 :
        {
            if(Time_Elapsed > 400 && Time_Elapsed < 1800)   //Time in microseconds
            {                                               //if time elapsed is between 400 and 1800 microseconds do the following:
                Nec_code = Nec_code << 1;                   //Left shift Nec_code by 1
                if(Time_Elapsed > 1000)                     //If time elapsed is greater than 1000
                    Nec_code++;                             //increment Nec_code by 1
                
                bit_count++;                                //Increment bit count by 1 since in this state we properly record one bit
                
                if(bit_count > 31)                          //if we have recorded more than 31 bits
                {
                    nec_ok = 1;                             //Put nec_ok so that we can read the Nec_code
                    INTCONbits.INT0IE = 0;                  // Disable external interrupt
                    Nec_state = 0;                          //Go back to Nec_state 0 to begin the process
                    Nec_code1 = (char) ((Nec_code >> 8));
                }
                else
                    Nec_state = 3;                          //If we have recorded less than 31 bits, go back to Nec_state 3
            }
            else
                force_nec_state0();                         //if time elapsed is not between 400 and 1800 microseconds go back to Nec state 0
            
            INTCON2bits.INTEDG0 = 1;        // Change Edge interrupt of INT0 to Low to High
            return;
        }
    }
}





