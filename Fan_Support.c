#include <p18f4620.h>
#include "Main.h"
#include "Fan_Support.h"
#include "stdio.h"

#define RPM_RED     PORTDbits.RD6
#define RPM_BLUE    PORTDbits.RD7
extern char FAN;
extern char duty_cycle;

char LED[8] = {0x00, 0x02, 0x04, 0x06, 0x10, 0x12, 0x14, 0x16};

int get_RPM()
{
    T3CON = 0x03;
    int RPS = TMR3L/2;          //read the count. Since there are 2 pulses per revolution
                                //then RPS = count/2
    TMR3L = 0x00;               //clear out the count
    return (RPS * 60);          //return RPM = 60*RPS
}

void Toggle_Fan()
{
    if(FAN == 0)
        Turn_On_Fan();
    else
        Turn_Off_Fan();
}

void Turn_Off_Fan()
{
    FAN = 0;
    FAN_EN = 0;
    FAN_LED = 0;
}

void Turn_On_Fan()
{
    FAN = 1;
    do_update_pwm(duty_cycle);
    FAN_EN = 1;
    FAN_LED = 1;
}

void Increase_Speed()
{
    if(duty_cycle == 100)
    {
        Do_Beep();
        Do_Beep();
    }
    else
        duty_cycle = duty_cycle + 5;
    do_update_pwm(duty_cycle);
}

void Decrease_Speed()
{
    if(duty_cycle == 0)
    {
        Do_Beep();
        Do_Beep();
    }
    else
        duty_cycle = duty_cycle - 5;
    do_update_pwm(duty_cycle);
}

void do_update_pwm(char duty_cycle) 
{ 
    float dc_f;           
    int dc_i;
    PR2 = 0x04;                     //set the frequency for 25kHz
    T2CON = 0x07;                   //
    dc_f = (4.0*duty_cycle / 20.0); //calculate factor of duty cycle versus 25kHz signal
    
    dc_i = (int) dc_f;              //get the integer part
    if(dc_i > duty_cycle)           //This is the round up function
        dc_i++;
    CCP1CON = ((dc_i & 0x03) << 4) | 0x0C;
    CCPR1L = (dc_i) >> 2;
}

void Set_DC_RGB(char duty_cycle)
{
    int upper_Digit =  (duty_cycle/10);
    
    if  (upper_Digit >= 7) 
        upper_Digit = 7;
    else if(upper_Digit <= 0)
        upper_Digit = 0;
    if(FAN_EN == 1)
        PORTA = LED[upper_Digit] | 0x20;
    else
        PORTA = PORTA = LED[upper_Digit];
}

void Set_RPM_RGB(int rpm)
{
    if(rpm >= 2700)
    {
        RPM_RED = 0;
        RPM_BLUE = 1;
    }
    else if(rpm >= 1800 && rpm < 2700)
    {
        RPM_RED = 1;
        RPM_BLUE = 1;
    }
    else if(rpm > 0 && rpm < 1800)
    {
        RPM_RED = 1;
        RPM_BLUE = 0;
    }
    else
    {
        RPM_RED = 0;
        RPM_BLUE = 0;
    }
}




