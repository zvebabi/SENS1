#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <SiLABS/c8051F060.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//defines here
#define SYSTEMCLOCK       22118400L       // System clock derived from 22.1184MHz XTL
#define SAR_CLK      2500000           // Desired SAR clock speed
#define SAMPLE_RATE 100
#define	BAUDRATE		115200						    // User-definable SW_UART baud rate
#define UART_BUFFERSIZE 8
#define INT_DEC      16               // Integrate and decimate ratio
#define	HW_TIME_COUNT	(SYSCLK/BAUD_RATE/16) // Time count for HW_UART baud rate generation. 
#define VREF 3000.0                       //mv on AV+
#define DAC_RES 4095.0                    //12bit
#define DAC_KOEFFS = (DAC_RES/VREF)         
//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F06x
//-----------------------------------------------------------------------------

sfr16 RCAP2    = 0xCA;                 // Timer2 capture/reload
sfr16 TMR2     = 0xCC;                 // Timer2
sfr16 RCAP3    = 0xCA;                 // Timer3 reload value
sfr16 TMR3     = 0xCC;                 // Timer3 counter
sfr16 RCAP4    = 0xCA;
sfr16 TMR4     = 0xCC;                 // Timer4 counter
sfr16 DAC0     = 0xD2;                 // DAC0 data
sfr16 DAC1     = 0xD2;                 // DAC1 data
sfr16 ADC2     = 0xBE;                 // ADC2 data

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

sbit LED = P0^5;

unsigned char UART_Buffer[UART_BUFFERSIZE];
//unsigned char* UART_BufferOut;
unsigned char UART_Buffer_Size = 0;
unsigned char UART_BufferOut_Size = 0;
unsigned char UART_Input_First = 0;
unsigned char UART_Output_First = 0;
unsigned char TX_Ready = 1;
unsigned char RX_Ready = 0;
static char Byte;
long Result;                           // ADC0 decimated value

//init func
void OSCILLATOR_Init (void);
void PORT_Init (void);
void UART0_Init (void);
void TIMER3_Init (int counts);
void DAC0_Init (void);
void DAC1_Init (void);
void ADC2_Init (void);
void Set_DACs(int value);
	
//-----------------------------------------------------------------------------
// Support Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// putchar
//-----------------------------------------------------------------------------
char putchar (char c)
{
   char SFRPAGE_SAVE = SFRPAGE;
   SFRPAGE = UART0_PAGE;
   if (c == '\n')                      // If carriage return
   {
      while (!TI0);
      TI0 = 0;
      SBUF0 = 0x0d;                    // Output \cr
   }
   while (!TI0);                       // Wait for transmit complete
   TI0 = 0;
   SBUF0 = c;                          // Send character
   SFRPAGE = SFRPAGE_SAVE;
   return c;
}

//-----------------------------------------------------------------------------
// _getkey
//-----------------------------------------------------------------------------
char _getkey ()
{
   char c;
   char SFRPAGE_SAVE = SFRPAGE;
   SFRPAGE = UART0_PAGE;
   while (!RI0);                       // Wait for byte to be received
   c = SBUF0;                          // Read the byte
   SFRPAGE = SFRPAGE_SAVE;
   return (c);
}

//-----------------------------------------------------------------------------
// Wait_MS
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters:
//   1) unsigned int ms - number of milliseconds of delay
//                        range is full range of integer: 0 to 65335
//
// This routine inserts a delay of <ms> milliseconds.
//
//-----------------------------------------------------------------------------
void Wait_MS(unsigned int ms)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

   SFRPAGE = TMR4_PAGE;

   TMR4CN = 0x00;                      // Stop Timer4; Clear TF4;
   TMR4CF = 0x00;                      // use SYSCLK/12 as timebase

   RCAP4 = -(SYSTEMCLOCK/1000/12);          // Timer 4overflows at 1 kHz
   TMR4 = RCAP4;
   EIE2   &= ~0x04;                    // Disable Timer4 interrupts
   TR4 = 1;                            // Start Timer 4
   while(ms)
   {
      TF4 = 0;                         // Clear flag to initialize
      while(!TF4);                     // Wait until timer overflows
      ms--;                            // Decrement ms
   }
   TR4 = 0;                            // Stop Timer 4
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}
#endif //DEFINITIONS_H