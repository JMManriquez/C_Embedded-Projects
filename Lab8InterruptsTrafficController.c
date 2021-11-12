#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <xc.h>
#include <math.h>
#include <p18f4620.h>

#pragma config OSC = INTIO67
#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config BOREN = OFF
#pragma config CCP2MX = PORTBE

#define _XTAL_FREQ      8000000         // Set operation for 8 Mhz
#define TMR_CLOCK       _XTAL_FREQ/4    // Timer Clock 2 Mhz

#define TFT_DC          PORTDbits.RD0   // Location of TFT D/C
#define TFT_CS          PORTDbits.RD1   // Location of TFT Chip Select
#define TFT_RST         PORTDbits.RD2   // Location of TFT Reset
#define SEC_LED         PORTDbits.RD7   //Assign bit of SEC_LED

#define NS_RED          PORTAbits.RA1   //Assign the bit that turns on the red light for NS
#define NS_GREEN        PORTAbits.RA2   //Assign the bit that turns on the green light for NS

#define NSLT_RED        PORTBbits.RB4   //Assign tht bit that turns on the red light for NS Left Turn
#define NSLT_GREEN      PORTBbits.RB5   //Assign tht bit that turns on the green light for NS Left Turn

#define EW_RED          PORTBbits.RB6   //Assign the bit that turns on the red light for EW 
#define EW_GREEN        PORTBbits.RB7   //Assign the bit that turns on the green light for EW

#define EWLT_RED        PORTEbits.RE0   //Assign the bit that turns on the red light for EW Left Turn
#define EWLT_GREEN      PORTEbits.RE2   //Assign the bit that turns on the green light for EW Left Turn

#define NS_LT_SW        PORTAbits.RA5   //Define the input for the NS Left Turn crossing Switch Input
#define EW_LT_SW        PORTAbits.RA4   //Define the input for the EW Left Turn crossing Switch Input

#define MODE_LED        PORTEbits.RE1   //Define the input for MODE_LED
//colors
#define OFF 		0               // Defines OFF as decimal value 0
#define RED 		1               // Defines RED as decimal value 1
#define GREEN 		2               // Defines GREEN as decimal value 2
#define YELLOW 		3               // Defines YELLOW as decimal value 3

#define Circle_Size     7               // Size of Circle for Light
#define Circle_Offset   15              // Location of Circle
#define TS_1            1               // Size of Normal Text
#define TS_2            2               // Size of PED Text
#define Count_Offset    10              // Location of Count

#define XTXT            30              // X location of Title Text 
#define XRED            40              // X location of Red Circle
#define XYEL            60              // X location of Yellow Circle
#define XGRN            80              // X location of Green Circle
#define XCNT            100             // X location of Sec Count

#define EW              0               // Number definition of East/West
#define EWLT            1               // Number definition of East/West Left Turn
#define NS              2               // Number definition of North/South
#define NSLT            3               // Number definition of North/South Left Turn
 
#define Color_Off       0               // Number definition of Off Color
#define Color_Red       1               // Number definition of Red Color
#define Color_Green     2               // Number definition of Green Color
#define Color_Yellow    3               // Number definition of Yellow Color

#define EW_Txt_Y        20
#define EW_Cir_Y        EW_Txt_Y + Circle_Offset
#define EW_Count_Y      EW_Txt_Y + Count_Offset
#define EW_Color        ST7735_CYAN

#define EWLT_Txt_Y      50
#define EWLT_Cir_Y      EWLT_Txt_Y + Circle_Offset
#define EWLT_Count_Y    EWLT_Txt_Y + Count_Offset
#define EWLT_Color      ST7735_WHITE

#define NS_Txt_Y        80
#define NS_Cir_Y        NS_Txt_Y + Circle_Offset
#define NS_Count_Y      NS_Txt_Y + Count_Offset
#define NS_Color        ST7735_BLUE 

#define NSLT_Txt_Y      110
#define NSLT_Cir_Y      NSLT_Txt_Y + Circle_Offset
#define NSLT_Count_Y    NSLT_Txt_Y + Count_Offset
#define NSLT_Color      ST7735_MAGENTA

#define PED_EW_Count_Y  30
#define PED_NS_Count_Y  90
#define PED_Count_X     2
#define Switch_Txt_Y    140

#include "ST7735_TFT.c"

char buffer[31];                        // general buffer for display purpose
char *nbr;                              // general pointer used for buffer
char *txt;

char EW_Count[]     = "00";             // text storage for EW Count
char EWLT_Count[]   = "00";             // text storage for EW Left Turn Count
char NS_Count[]     = "00";             // text storage for NS Count
char NSLT_Count[]   = "00";             // text storage for NS Left Turn Count

char PED_EW_Count[] = "00";             // text storage for EW Pedestrian Count
char PED_NS_Count[] = "00";             // text storage for NS Pedestrian Count

char SW_EWPED_Txt[] = "0";              // text storage for EW Pedestrian Switch
char SW_EWLT_Txt[]  = "0";              // text storage for EW Left Turn Switch
char SW_NSPED_Txt[] = "0";              // text storage for NS Pedestrian Switch
char SW_NSLT_Txt[]  = "0";              // text storage for NS Left Turn Switch

char SW_MODE_Txt[]  = "D";              // text storage for Mode Light Sensor
	
char Act_Mode_Txt[]  = "D";             // text storage for Actual Mode
char FlashingS_Txt[] = "0";             // text storage for Emergency Status
char FlashingR_Txt[] = "0";             // text storage for Flashing Request

char dir;
char Count;                             // RAM variable for Second Count
char PED_Count;                         // RAM variable for Second Pedestrian Count

char SW_EWPED = 0;                      // RAM variable for EW Pedestrian Switch
char SW_NSPED = 0;                      // RAM variable for NS Pedestrian Switch
char FLASHING_REQUEST = 0;              // RAM variable for FLASING_REQUEST FLAG

char SW_EWLT;                           // RAM variable for EW Left Turn Switch
char SW_NSLT;                           // RAM variable for NS Left Turn Switch
char FLASHING;
char FLASHING_REQUEST;
char SW_MODE;                           // RAM variable for Mode Light Sensor
int MODE;
float volt;
    
unsigned int get_full_ADC(void);
void Init_ADC(void);
void init_IO(void);
void init_UART();

void interrupt high_priority checkISR();
void INT0_ISR();
void INT1_ISR();
void INT2_ISR();

void set_NS(char color);
void set_NS_LT(char color);
void set_EW(char color);
void set_EW_LT(char color);

void PED_Control(char direction, char Num_Sec);
void Day_Mode(void);
void Night_Mode(void);

void wait_One_Second(void);
void wait_Half_Second(void);
void wait_N_Seconds (char direction, char count);
void wait_One_Second_With_Beep(void);

void Initialize_Screen(void);
void update_LCD_color(char direction, char color);
void update_LCD_PED_Count(char direction, char count);
void update_LCD_misc(void);
void update_LCD_count(char direction, char count);

void Do_Flashing();

void init_UART()
{
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF &
    USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX &
    USART_BRGH_HIGH, 25);
    OSCCON = 0x70; 
    RBPU = 0;    
    // set the system clock to be 1MHz 1/4 of the 4MHz
}

void Initialize_Screen()
{
  LCD_Reset();
  TFT_GreenTab_Initialize();
  fillScreen(ST7735_BLACK);
  
  /* TOP HEADER FIELD */
  txt = buffer;
  strcpy(txt, "ECE3301L Fall 21-S3");  
  drawtext(2, 2, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
  /* MODE FIELD */
  strcpy(txt, "Mode:");
  drawtext(2, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(35,10, Act_Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);

  /* FLASHING REQUEST FIELD */
  strcpy(txt, "FR:");
  drawtext(50, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(70, 10, FlashingR_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
  /* FLASHING STATUS FIELD */
  strcpy(txt, "FS:");
  drawtext(80, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(100, 10, FlashingS_Txt, ST7735_WHITE, ST7735_BLACK, TS_1); 
  
  /* SECOND UPDATE FIELD */
  strcpy(txt, "*");
  drawtext(120, 10, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
/* EAST/WEST UPDATE FIELD */
  strcpy(txt, "EAST/WEST");
  drawtext  (XTXT, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EW_Cir_Y-8, 60, 18, EW_Color);
  drawCircle(XRED, EW_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EW_Cir_Y, Circle_Size, ST7735_YELLOW);
  fillCircle(XGRN, EW_Cir_Y, Circle_Size, ST7735_GREEN);
  drawtext  (XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2);    


  /* EAST/WEST LEFT TURN UPDATE FIELD */
  strcpy(txt, "E/W LT");
  drawtext  (XTXT, EWLT_Txt_Y, txt, EWLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, EWLT_Cir_Y-8, 60, 18, EWLT_Color);  
  fillCircle(XRED, EWLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, EWLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, EWLT_Cir_Y, Circle_Size, ST7735_GREEN);   
  drawtext  (XCNT, EWLT_Count_Y, EWLT_Count, EWLT_Color, ST7735_BLACK, TS_2);

  /* NORTH/SOUTH UPDATE FIELD */
  strcpy(txt, "NORTH/SOUTH");
  drawtext  (XTXT, NS_Txt_Y  , txt, NS_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NS_Cir_Y-8, 60, 18, NS_Color);
  fillCircle(XRED, NS_Cir_Y  , Circle_Size, ST7735_RED);
  drawCircle(XYEL, NS_Cir_Y  , Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, NS_Cir_Y  , Circle_Size, ST7735_GREEN);
  drawtext  (XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2);
    
  /* NORTH/SOUTH LEFT TURN UPDATE FIELD */
  strcpy(txt, "N/S LT");
  drawtext  (XTXT, NSLT_Txt_Y, txt, NSLT_Color, ST7735_BLACK, TS_1);
  drawRect  (XTXT, NSLT_Cir_Y-8, 60, 18, NSLT_Color);
  fillCircle(XRED, NSLT_Cir_Y, Circle_Size, ST7735_RED);
  drawCircle(XYEL, NSLT_Cir_Y, Circle_Size, ST7735_YELLOW);
  drawCircle(XGRN, NSLT_Cir_Y, Circle_Size, ST7735_GREEN);  
  drawtext  (XCNT, NSLT_Count_Y, NSLT_Count, NSLT_Color, ST7735_BLACK, TS_2);
  
  /* EAST/WEST PEDESTRIAM UPDATE FIELD */  
  drawtext(2, PED_EW_Count_Y, PED_EW_Count, EW_Color, ST7735_BLACK, TS_2);
  strcpy(txt, "PEW");  
  drawtext(3, EW_Txt_Y, txt, EW_Color, ST7735_BLACK, TS_1);

  /* NORTH/SOUTH PEDESTRIAM UPDATE FIELD */
  strcpy(txt, "PNS");  
  drawtext(3, NS_Txt_Y, txt, NS_Color, ST7735_BLACK, TS_1);
  drawtext(2, PED_NS_Count_Y, PED_NS_Count, NS_Color, ST7735_BLACK, TS_2);
    
  /* MISCELLANEOUS UPDATE FIELD */  
  strcpy(txt, "EWP EWLT NSP NSLT MR");
  drawtext(1,  Switch_Txt_Y, txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(6,  Switch_Txt_Y+9, SW_EWPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(32, Switch_Txt_Y+9, SW_EWLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(58, Switch_Txt_Y+9, SW_NSPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  drawtext(87, Switch_Txt_Y+9, SW_NSLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
  
  drawtext(112,Switch_Txt_Y+9, SW_MODE_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}

void init_IO()
{
    TRISA = 0x31;                       //A0 = Input, A1 = Output, A2 = Output, A4 = Input, A5 = Input
    TRISB = 0x07;                       //B0 = Input B1 = Input, B3 = Input, B4 - B7 = Output 
    TRISC = 0x00;                       //C3 = Output, C5 = Output 
    TRISD = 0x00;                       //All are outputs
    TRISE = 0x00;                       //All are outputs
    
    INTCONbits.INT0IF = 0;              //Clear INT0IF
    INTCON3bits.INT1IF = 0;             //Clear INT1IF
    INTCON3bits.INT2IF = 0;             //Clear INT2IF
    
    INTCONbits.INT0IE = 1;              //Enable interrupt INT0
    INTCON3bits.INT1IE = 1;             //Enable interrupt INT1
    INTCON3bits.INT2IE = 1;             //Enable interrupt INT2
    
    INTCON2bits.INTEDG0 = 0;            //INT0 Edge Falling
    INTCON2bits.INTEDG1 = 0;            //INT1 Edge Falling
    INTCON2bits.INTEDG2 = 0;            //INT2 Edge Falling 
    
    INTCONbits.GIE = 1;                 //Set the Global Interrupt Enable
}

void Init_ADC()
{
    ADCON0 = 0x01;          //select channel AN0, and turn on the ADC subsystem
    ADCON1 = 0x0E;          //select pins AN) through AN# as analog signal, VDD-
                            //VSS as reference voltage
    ADCON2 = 0xA9;          //right justify the results. Set the bit conversion 
                            //time TAD and acquisition time
}

void putch (char c)
{
    while (!TRMT);
    TXREG = c;
}

unsigned int get_full_ADC()
{
   unsigned int result;
   ADCON0bits.GO=1;                     // Start Conversion
   while(ADCON0bits.DONE==1);           // wait for conversion to be completed
   result = (ADRESH * 0x100) + ADRESL;  // combine result of upper byte and
                                        // lower byte into result
   return result;                       // return the result.
}

void interrupt high_priority checkISR()
{
    if(INTCONbits.INT0IF == 1)  INT0_ISR();
    if(INTCON3bits.INT1IF == 1) INT1_ISR();
    if(INTCON3bits.INT2IF == 1) INT2_ISR();
}

void INT0_ISR()
{
   INTCONbits.INT0IF = 0;               //Clear the interrupt flag
   if(MODE == 1)                         //If in Day MODE allow flag to be set
       SW_EWPED = 1;                    //Pass the flag to handle it in our own code
   else                                 //If in Night MODE do NOT allow flag to be set
       SW_EWPED = 0;                    //Keep flag cleared
}

void INT1_ISR()
{
    INTCON3bits.INT1IF = 0;             //Clear the interrupt flag
    if(MODE == 1)                        //If in Day MODE allow flag to be set
        SW_NSPED = 1;                   //Pass the flag to handle it in our code
    else                                //If in Night MODE do NOT allow flag to be set
        SW_NSPED = 0;                   //Keep flag cleared 
}

void INT2_ISR()
{
    INTCON3bits.INT2IF = 0;             //Clear the interrupt flag
    FLASHING_REQUEST = 1;               //Pass the flag to handle it in our code
}

void update_LCD_color(char direction, char color)
{
    char Circle_Y;
    Circle_Y = EW_Cir_Y + direction * 30;    
    
    if (color == Color_Off)     //if Color off make all circles black but leave outline
    {
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);                       
    }    
    else if(color == Color_Red) //If color, fill in the color with the outline
    {
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN); 
    }
    else if(color == Color_Yellow)
    {
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN); 
    }
    else if(color == Color_Green)
    {
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN); 
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN); 
    }
    else
    {
        fillCircle(XRED, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XYEL, Circle_Y, Circle_Size, ST7735_BLACK);
        fillCircle(XGRN, Circle_Y, Circle_Size, ST7735_BLACK); 
        drawCircle(XRED, Circle_Y, Circle_Size, ST7735_RED);            
        drawCircle(XYEL, Circle_Y, Circle_Size, ST7735_YELLOW);
        drawCircle(XGRN, Circle_Y, Circle_Size, ST7735_GREEN);
    }
}

void update_LCD_count(char direct, char count)
{
   if(direct == EW && direct != EWLT)       //Select proper direction
   {
        EW_Count[0] = count/10  + '0';      //Display count in ASCII format to print it to LCD Screen
        EW_Count[1] = count%10  + '0';
        drawtext(XCNT, EW_Count_Y, EW_Count, EW_Color, ST7735_BLACK, TS_2); 
   }
   else if(direct == EWLT && direct != EW)
   {
        EWLT_Count[0] = count/10  + '0';
        EWLT_Count[1] = count%10  + '0';
        drawtext(XCNT, EWLT_Count_Y, EWLT_Count, EWLT_Color, ST7735_BLACK, TS_2);                
   }
   else if(direct == NS && direct != NSLT)
   {
        NS_Count[0] = count/10  + '0';
        NS_Count[1] = count%10  + '0';
        drawtext(XCNT, NS_Count_Y, NS_Count, NS_Color, ST7735_BLACK, TS_2);
   }
   else if(direct == NSLT && direct != NS)
   {
        NSLT_Count[0] = count/10  + '0';
        NSLT_Count[1] = count%10  + '0';
        drawtext(XCNT, NSLT_Count_Y, NSLT_Count, NSLT_Color, ST7735_BLACK, TS_2);
    }  
}

void update_LCD_PED_Count(char direction, char count)
{
   switch (direction)
   {
      case EW:       
        PED_EW_Count[0] = count/10  + '0';          // PED count upper digit
        PED_EW_Count[1] = count%10  + '0';          // PED Lower
        drawtext(PED_Count_X, PED_EW_Count_Y, PED_EW_Count, EW_Color, ST7735_BLACK, TS_2); 
                                                    //Put counter on digit on screen
        break;
       case NS:
        PED_NS_Count[0] = count/10  + '0';          // PED count upper digit
        PED_NS_Count[1] = count%10  + '0';          // PED Lower
        drawtext(PED_Count_X, PED_NS_Count_Y, PED_NS_Count, NS_Color, ST7735_BLACK, TS_2); 
                                                    //Put counter on digit on screen
        break;
   }
}

void update_LCD_misc()
{
    int nStep = get_full_ADC();         // calculates the # of steps for analog conversion
    volt = nStep * 5 /1024.0;           // gets the voltage in Volts, using 5V as reference s instead of 4, also divide by 1024 
    SW_MODE = volt < 2.5 ? 1:0;         // Mode = 1, Day_mode, Mode = 0 Night_mode
    
    SW_EWLT = EW_LT_SW;    
    SW_NSLT = NS_LT_SW;

    if(FLASHING_REQUEST == 0)
        FlashingR_Txt[0] = '0';
    else
        FlashingR_Txt[0] = '1';
    
    if(FLASHING == 0)
        FlashingS_Txt[0] = '0';
    else
        FlashingS_Txt[0] = '1';
    
    if(SW_MODE == 0)
        SW_MODE_Txt[0] = 'N';
    else
        SW_MODE_Txt[0] = 'D';       //Set state at bottom of screen for MODE Switch
    
    if (SW_EWPED == 0) 
      SW_EWPED_Txt[0] = '0'; 
    else 
      SW_EWPED_Txt[0] = '1';        //Set state at bottom of screen for EWPED Switch
    
    if(SW_EWLT == 0)
      SW_EWLT_Txt[0] = '0';
    else
      SW_EWLT_Txt[0] = '1';         //Set state at bottom of Screen for EWLT Switch
    
    if (SW_NSPED == 0) 
      SW_NSPED_Txt[0] = '0'; 
    else 
      SW_NSPED_Txt[0] = '1';        //Set state at bottom of screen for NSPED Switch
    
    if(SW_NSLT == 0)
      SW_NSLT_Txt[0] = '0';
    else
      SW_NSLT_Txt[0] = '1';         //Set state at bottom of Screen for NSLT Switch
    
    drawtext(35,10, Act_Mode_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);   
    drawtext(100, 10, FlashingS_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(70, 10, FlashingR_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(6,   Switch_Txt_Y+9, SW_EWPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(32,  Switch_Txt_Y+9, SW_EWLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
    drawtext(58,  Switch_Txt_Y+9, SW_NSPED_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);       // Show switch and sensor states at bottom of the screen
    drawtext(87,  Switch_Txt_Y+9, SW_NSLT_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);

    drawtext(112,  Switch_Txt_Y+9, SW_MODE_Txt, ST7735_WHITE, ST7735_BLACK, TS_1);
}

void set_NS(char color)
{
    update_LCD_color(NS, color);
    
    switch(color)
    {
        case OFF:
            NS_RED = 0;
            NS_GREEN = 0;       //Turns off the NS LED
            break;
        case RED:
            NS_RED = 1;
            NS_GREEN = 0;       //Sets NS LED RED
            break;
        case GREEN:
            NS_RED = 0;
            NS_GREEN = 1;       //Sets NS LED GREEN
            break;
        case YELLOW:
            NS_RED = 1;
            NS_GREEN = 1;       //Sets NS LED YELLOW
            break;
    }
}

void set_NSLT(char color)
{
    update_LCD_color(NSLT, color);
    
    switch(color)
    {
        case OFF:
            NSLT_RED = 0;
            NSLT_GREEN = 0;         //Turns off the NSLT LED
            break;
        case RED:
            NSLT_RED = 1;
            NSLT_GREEN = 0;         //Sets NSLT LED RED
            break;
        case GREEN:
            NSLT_RED = 0;
            NSLT_GREEN = 1;         //Sets NSLT LED GREEN
            break;
        case YELLOW:
            NSLT_RED = 1;
            NSLT_GREEN = 1;         //Sets NSLT LED YELLOW
            break;
    }
}

void set_EW(char color)
{
    update_LCD_color(EW, color);
    
    switch(color)
    {
        case OFF:
            EW_RED = 0;
            EW_GREEN = 0;       //Turns off the EW LED
            break;
        case RED:
            EW_RED = 1;
            EW_GREEN = 0;       //Sets EW LED RED
            break;
        case GREEN:
            EW_RED = 0;
            EW_GREEN = 1;       //Sets EW LED GREEN
            break;
        case YELLOW:
            EW_RED = 1;
            EW_GREEN = 1;       //Sets EW LED YELLOW
            break;
    }
}

void set_EWLT(char color)
{
    update_LCD_color(EWLT, color);
    
    switch(color)
    {
        case OFF:
            EWLT_RED = 0;
            EWLT_GREEN = 0;         //Turns off the EWLT LED
            break;
        case RED:
            EWLT_RED = 1;
            EWLT_GREEN = 0;         //Sets EWLT LED RED
            break;
        case GREEN:
            EWLT_RED = 0;
            EWLT_GREEN = 1;         //Sets EWLT LED GREEN
            break;
        case YELLOW:
            EWLT_RED = 1;
            EWLT_GREEN = 1;         //Sets EWLT LED YELLOW
            break;
    }
}

void PED_Control(char direction, char Num_Sec)
{ 
    for(char i = Num_Sec-1;i>0; i--)
    {
        update_LCD_PED_Count(direction, i);
        wait_One_Second_With_Beep();            //Hold the number on display for 1 second
    }
    
    update_LCD_PED_Count(direction, 0);         //Output a 0 back into the screen after finishing the counting
    wait_One_Second_With_Beep();                //Beeps for the last 0th second
}

void Day_Mode()
{
    MODE = 1;                                   // turns on the MODE_LED
    MODE_LED = 1;
    Act_Mode_Txt[0] = 'D';
    
    INTCONbits.INT0IF == 1;         //Call Interrupt to clear EW Pedestrian flag
    INTCON3bits.INT1IF == 1;        //Call Interrupt to clear NS Pedestrian flag
    
    set_NS(GREEN);                  //Set NS to Green
    set_NSLT(RED);                  //Set NS Left Turn to RED
    set_EW(RED);                    //Set EW to Red
    set_EWLT(RED);                  //Set EW Left Turn to RED
    
    if(SW_NSPED == 1)               //Check if a request has been placed for pedestrian crossing NS
    {
        PED_Control(NS, 7);         //Call the PED_Control to allow NS pedestrians to cross for 7 seconds 
        SW_NSPED = 0;               //Clear the Request for NS Pedestrian after being serviced
    }
                                    //If no request to cross NS for pedestrians was made, we simply exit the single if structure
    wait_N_Seconds(NS, 9);          //wait for 9 seconds
    
    set_NS(YELLOW);                 //Turn the NS light to yellow
    wait_N_Seconds(NS, 3);          //wait for 3 seconds
    
    set_NS(RED);                    //Turn the NS light to red
    
    if(EW_LT_SW == 1)               //Check if there is a request to turn left on EW
    {
        set_EWLT(GREEN);            //Turn on the EW Left Turn to Green     
        wait_N_Seconds(EWLT,7);              //Wait for 7 seconds
        
        set_EWLT(YELLOW);           //Turn the EW Left Turn to yellow
        wait_N_Seconds(EWLT, 3);              //wait 3 seconds
        
        set_EWLT(RED);              //Turn the EW Left Turn to RED
    }
                                    //If no request to turn left for EW was made we simply exit the single if structure
    set_EW(GREEN);                  //Turn on the EW light to GREEN
    
    if(SW_EWPED == 1)               //Check if a request has been placed for pedestrian crossing EW 
    {
        PED_Control(EW, 9);         //Call the PED_Control to allow EW pedestrians to cross for 9 seconds
        SW_EWPED = 0;               //Clear the Request for EW Pedestrian after being serviced
    }
                                    //If no request to cross EW for pedestrians was made, we simply exit the single if structure
    wait_N_Seconds(EW, 8);              //wait for 9 seconds
    
    set_EW(YELLOW);                 //Turn EW to YELLOW       
    wait_N_Seconds(EW, 3);                  //Wait for 3 seconds
    
    set_EW(RED);                    //Turn EW to RED
    
    if(NS_LT_SW == 1)                //Check if there is a request to turn left on NS
    {
        set_NSLT(GREEN);            //Turn on the NS Left Turn to Green             
        wait_N_Seconds(NSLT, 7);              //Wait for 7 seconds
        
        set_NSLT(YELLOW);           //Turn the NS Left Turn to yellow
        wait_N_Seconds(NSLT, 3);              //wait 3 seconds
        
        set_NSLT(RED);              //Turn the NS Left Turn to RED
    }
                                    //If no request to turn left for NS was made we simply exit the single if structure
}

void Night_Mode()
{ 
    MODE = 0;                                   // turns on the MODE_LED
    MODE_LED = 0;
    Act_Mode_Txt[0] = 'N';
    
    SW_NSPED = 0;
    SW_EWPED = 0;
    
    INTCONbits.INT0IF == 1;         //Call Interrupt to clear EW Pedestrian flag
    INTCON3bits.INT1IF == 1;        //Call Interrupt to clear NS Pedestrian flag
    
    set_NS(GREEN);                  //Set NS to Green
    set_NSLT(RED);                  //Set NS Left Turn to RED
    set_EW(RED);                    //Set EW to Red
    set_EWLT(RED);                  //Set EW Left Turn to RED
    wait_N_Seconds(NS, 9);              //wait for 9 seconds
    
    set_NS(YELLOW);                 //Turn the NS light to yellow
    wait_N_Seconds(NS, 3);              //wait for 3 seconds
    
    set_NS(RED);                    //Turn the NS light to red
    
    if(EW_LT_SW == 1)               //Check if there is a request to turn left on EW
    {
        set_EWLT(GREEN);            //Turn on the EW Left Turn to Green     
        wait_N_Seconds(EWLT, 7);          //Wait for 7 seconds
        
        set_EWLT(YELLOW);           //Turn the EW Left Turn to yellow
        wait_N_Seconds(EWLT, 3);          //wait 3 seconds
        
        set_EWLT(RED);              //Turn the EW Left Turn to RED
    }
                                    //If no request to turn left for EW was made we simply exit the single if structure
    set_EW(GREEN);                  //Turn on the EW light to GREEN
    wait_N_Seconds(EW, 9);              //wait for 9 seconds
    
    set_EW(YELLOW);                 //Turn EW to YELLOW       
    wait_N_Seconds(EW, 3);              //Wait for 3 seconds
    
    set_EW(RED);                    //Turn EW to RED
    
    if(NS_LT_SW == 1)               //Check if there is a request to turn left on NS
    {
        set_NSLT(GREEN);            //Turn on the NS Left Turn to Green             
        wait_N_Seconds(NSLT, 7);          //Wait for 7 seconds
        
        set_NSLT(YELLOW);           //Turn the NS Left Turn to yellow
        wait_N_Seconds(NSLT, 3);          //wait 3 seconds
        
        set_NSLT(RED);              //Turn the NS Left Turn to RED
    }
                                    //If no request to turn left for NS was made we simply exit the single if structure

}

void Do_Flashing()
{
    FLASHING = 1;
    
    while(FLASHING == 1)
    {
            //Check if FLASHING_REQUEST was pressed again to exit out of this function
        if(FLASHING_REQUEST == 1)
        {
            FLASHING_REQUEST = 0;   //Clear Flashing Request
            FLASHING = 0;           //Clear Flashing to exit loop
            break;                  //Break loop to ensure exit
        }
        else
        {
            //Begin Flashing all directions red
           set_NS(RED);
           set_EW(RED);
           set_NSLT(RED);
           set_EWLT(RED);
           wait_One_Second();
           set_NS(OFF);
           set_EW(OFF);
           set_NSLT(OFF);
           set_EWLT(OFF);
           wait_One_Second();
        }
    }
}

void main(void)
{
    init_IO();
    Init_ADC();
    init_UART();
    
    Initialize_Screen();                        // Initialize the TFT screen   
     

    int nStep = get_full_ADC();                 // calculates the # of steps for analog conversion
    volt = nStep * 5 /1024.0;                   // gets the voltage in Volts, using 5V as reference s instead of 4, also divide by 1024 
    SW_MODE = volt < 2.5 ? 1:0;                 // SW_Mode = 1, Day_mode, SW_Mode = 0 Night_mode

    while(1)                                    // forever loop
    {
        if (SW_MODE == 1)    
        {
            Day_Mode();                         // calls Day_Mode() function
        }
        else
        {
            Night_Mode();                       // calls Night_Mode() function
        }
        if(FLASHING_REQUEST == 0)               //If there is no FLASHING request, skip the rest of the code below
            continue;                           //and continue to next iteration of the loop
                                        
                                                //Since FLASHING_REQUEST == 1 if it reaches this point,
        FLASHING_REQUEST = 0;                   //Clear FLASHING_REQUEST service flag
        Do_Flashing();                          //Service the request for flashing
    }
}

void Activate_Buzzer()
{
   PR2 = 0b11111001;
   T2CON = 0b00000101;
   CCPR2L = 0b01001010;
   CCP2CON = 0b00111100;   
}

void Deactivate_Buzzer()
{
    CCP2CON = 0X0;
    PORTBbits.RB3 = 0;
}

void wait_Half_Second()
{
    T0CON = 0x03;                               // Timer 0, 16-bit mode, prescaler 1:16
    TMR0L = 0xDB;                               // set the lower byte of TMR
    TMR0H = 0x0B;                               // set the upper byte of TMR
    INTCONbits.TMR0IF = 0;                      // clear the Timer 0 flag
    T0CONbits.TMR0ON = 1;                       // Turn on the Timer 0
    while (INTCONbits.TMR0IF == 0);             // wait for the Timer Flag to be 1 for done
    T0CONbits.TMR0ON = 0;                       // turn off the Timer 0
}

void wait_One_Second()							//creates one second delay and blinking asterisk
{
    SEC_LED = 1;
    strcpy(txt,"*");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    wait_Half_Second();                         // Wait for half second (or 500 msec)
    update_LCD_misc();
    
    SEC_LED = 0;
    strcpy(txt," ");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
    wait_Half_Second();                         // Wait for half second (or 500 msec)
    update_LCD_misc();
}

void wait_One_Second_With_Beep()
{
    SEC_LED = 1;                                //First, turn on the SEC LED
    strcpy(txt,"*");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
                                                //Above we blink the asterisk half second tracker to ON
    update_LCD_misc();
    Activate_Buzzer();                          //Activate the buzzer
    wait_Half_Second();                         //Wait for half second (or 500msec)
    
    SEC_LED = 0;                                //then turn off the SEC LED
    strcpy(txt," ");
    drawtext(120,10,txt,ST7735_WHITE,ST7735_BLACK,TS_1);
                                                //Above we blink the asterisk half second tracker to ON
    update_LCD_misc();
    Deactivate_Buzzer();                        //Deactivate the buzzer
    wait_Half_Second();                         //Wait for half second (or 500 msec)
    
}

void wait_N_Seconds (char direc, char seconds)
{
    char I;
    for (I = seconds; I> 0; I--)
    {
        update_LCD_count(direc, I);  
        wait_One_Second();                      // calls Wait_One_Second for x number of times
      
    }                 
    update_LCD_count(direc, 0);   
}