#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <SiLABS/c8051F060.h>
#include <stdio.h>

//defines here
#define SYSTEMCLOCK       22118400L // System clock derived from 22.1184MHz XTL

#define	BAUDRATE		115200						// User-definable SW_UART baud rate
#define UART_BUFFERSIZE 64

#define	HW_TIME_COUNT	SYSCLK/BAUD_RATE/16 	// Time count for HW_UART baud rate generation. 

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F06x
//-----------------------------------------------------------------------------

sfr16 RCAP2    = 0xCA;                 // Timer2 capture/reload
sfr16 TMR2     = 0xCC;                 // Timer2
sfr16 RCAP3    = 0xCA;                 // Timer3 reload value
sfr16 TMR3     = 0xCC;                 // Timer3 counter
sfr16 DAC0     = 0xD2;                 // DAC0 data
sfr16 DAC1     = 0xD2;                 // DAC1 data

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

sbit LED = P0^5;

unsigned char UART_Buffer[UART_BUFFERSIZE];
//unsigned char UART_BufferOut[UART_BUFFERSIZE];
unsigned char UART_Buffer_Size = 0;
//unsigned char UART_BufferOut_Size = 0;
unsigned char UART_Input_First = 0;
unsigned char UART_Output_First = 0;
unsigned char TX_Ready =1;
static char Byte;

//init func
void OSCILLATOR_Init (void);
void PORT_Init (void);
void UART0_Init (void);
void Timer3_Init (int counts);
void DAC0_Init (void);
void DAC1_Init (void);
void Set_DACs(void);
	
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
      SBUF0 = 0x0d;                    // Output CR
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

#endif //DEFINITIONS_H