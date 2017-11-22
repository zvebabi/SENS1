#include <definitions.h>

/*
 * The main C function.  Program execution starts
 * here after stack initialization.
 */
void main (void) {
  char c;
  long value; //for reading float values
  char ledState[5];
  int measurement;                   // Measured voltage in mV
  
  SFRPAGE = CONFIG_PAGE;
  WDTCN = 0xDE;                       // Disable watchdog timer
  WDTCN = 0xAD;

  OSCILLATOR_Init ();                 // Initialize oscillator
  PORT_Init ();                       // Initialize crossbar and GPIO
  UART0_Init ();                      // Initialize UART0
  DAC0_Init ();                       // Initialize DAC0
  DAC1_Init ();                       // Initialize DAC1   
  TIMER3_Init(SYSTEMCLOCK/SAMPLE_RATE/12);
  ADC2_Init ();                       // Init ADC
  
  SFRPAGE = ADC2_PAGE;
  AD2EN = 1;                          // Enable ADC

  EA = 1;                             //enable global interrupt

  SFRPAGE = UART0_PAGE;
    
  while (1) {
    
//    if((TX_Ready == 1) && (UART_BufferOut_Size > 0))
//    {
//      TX_Ready = 0;                  // Set the flag to zero
//      TI0 = 1;                       // Set transmit flag to 1
//    }
    
    if(/*RX_Ready*/RI0==1) //process command
    {
      if ( scanf("%c=%4s", &c, &ledState) == 2 )
      {
        value = (ledState[0]-'0')*1000 + 
                (ledState[1]-'0')*100 + 
                (ledState[2]-'0')*10 + 
                (ledState[3]-'0');
        
        if (c =='l')
        {
          LED = (value == 1111)? 0 : 1;
        }
//        printf("c=%cl=%s\n", c, ledState);
        printf("c=%cl=%ld\n", c, value);
        while( (c = _getkey()) != '\n' ){;}
      }
      else
      {
        while( (c = _getkey()) != '\n' ){;}
      }
      
//      if (strncmp(UART_Buffer,"x=t", 3) == 0) // Light UP command
//      {
//        if(UART_Buffer_Size>3)
//        {
//          sscanf(UART_Buffer, "x=t%c", &ledState);
//          LED = (ledState == '0')? 0 : 1;
//          printf("LED%c\n", ledState);
//        }
////          LED = 0;
////          printf("LED%Bd\n", (char)(0));
//      }
//      if (strncmp(UART_Buffer,"x=l", 3) == 0) // Light UP command
//      {
//          LED = 1;
//          printf("LED%Bd\n", (char)(1));
//      }
//      if (strncmp(UART_Buffer,"x=d", 3) == 0) // DAC set command
//      {
//        if(UART_Buffer_Size>3)
//        {
//          sscanf(UART_Buffer, "x=d%d", &floatVal);
//          Set_DACs(floatVal);
//         // UART_BufferOut = "DAC\n";
//        }
//        //else
//        //  UART_BufferOut = "ERR\n";
//       // UART_BufferOut_Size = 4; //START TRANSMISSION AFTER SIZE SETTING
//      }
//      if (strncmp(UART_Buffer,"x=a", 3) == 0) // ADC read command
//      {
//        EA = 0;                          // Disable interrupts
//        measurement =  Result * 2430 / 1023;
//        EA = 1;                          // Re-enable interrupts
//        
////        UART_BufferOut = bytes;
// //       UART_BufferOut_Size = 5;
//      }
//      RX_Ready = 0;
//      UART_Buffer_Size = 0;
    }      
  }
}


//-----------------------------------------------------------------------------
// OSCILLATOR_Init
// This function initializes the system clock to use an external 22.1184MHz
// crystal.
//-----------------------------------------------------------------------------
void OSCILLATOR_Init (void)
{
   int i;                              // Software timer

   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

   SFRPAGE = CONFIG_PAGE;              // Set SFR page

   OSCICN = 0x80;                      // Set internal oscillator to run
                                       // at its slowest frequency

   CLKSEL = 0x00;                      // Select the internal osc. as
                                       // the SYSTEMCLOCK source

   // Initialize external crystal oscillator to use 22.1184 MHz crystal

   OSCXCN = 0x67;                      // Enable external crystal osc.
   for (i=0; i < 256; i++);            // Wait at least 1ms

   while (!(OSCXCN & 0x80));           // Wait for crystal osc to settle

   CLKSEL = 0x01;
   // Select external crystal as SYSTEMCLOCK source

   SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}

//-----------------------------------------------------------------------------
// PORT_Init
// Pinout:
// P0.0   digital   push-pull     UART TX
// P0.1   digital   open-drain    UART RX
// P0.5   digital   push-pull     LED
//-----------------------------------------------------------------------------
void PORT_Init (void)
{
  char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

  SFRPAGE = CONFIG_PAGE;              // Set SFR page

  XBR0     = 0x04;                    // Enable UART0
  XBR1     = 0x00;
  XBR2     = 0x40;                    // Enable crossbar and weak pull-up


  P0MDOUT |= 0x21;                    // Set P0.5(led) and P0.0(TX) pin to push-pull
//  P1MDOUT |= 0x40;                    // Set P1.6(LED) to push-pull
  P1MDIN = 0xFE;                      // P1.0 Analog Input, Open Drain, from dac
  SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//-----------------------------------------------------------------------------
// UART0_Init   Variable baud rate, Timer 2, 8-N-1
// Configure UART0 for operation at <BAUDRATE> 8-N-1 using Timer2 as
// baud rate source.
//-----------------------------------------------------------------------------
void UART0_Init (void)
{
  char SFRPAGE_SAVE;

  SFRPAGE_SAVE = SFRPAGE;             // Preserve SFRPAGE
//tmr2 init
  SFRPAGE = TMR2_PAGE;

  TMR2CN = 0x00;                      // Timer in 16-bit auto-reload up timer
                                      // mode
  TMR2CF = 0x08;                      // SYSCLK is time base; no output;
                                      // up count only
  RCAP2 = - ((long) SYSTEMCLOCK/BAUDRATE/16);
  TMR2 = RCAP2;
  TR2= 1;                             // Start Timer2
//tmr2 init end
  SFRPAGE = UART0_PAGE;

  SCON0 = 0x50;                       // 8-bit variable baud rate;
                                      // 9th bit ignored; RX enabled
                                      // clear all flags
  SSTA0 = 0x15;                       // Clear all flags; enable baud rate
                                      // doubler (not relevant for these
                                      // timers);
                                      // Use Timer2 as RX and TX baud rate
                                      // source;
   ES0 = 1;
   IP |= 0x10;
    TI0 = 1;
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}

//-----------------------------------------------------------------------------
// DAC0_Init
//-----------------------------------------------------------------------------
void DAC0_Init(void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = DAC0_PAGE;
   DAC0CN = 0x80;                      // Enable DAC0 in right-justified mode
                                       // managed by on-demand while write to DAC0H
   SFRPAGE = ADC2_PAGE;
   REF2CN |= 0x0F;                     // Enable the external AV+(3.0v) and
                                       // the Bias Generator, Tempsensor
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//-----------------------------------------------------------------------------
// DAC1_Init
//-----------------------------------------------------------------------------
void DAC1_Init(void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = DAC1_PAGE;
  
   DAC1CN = 0x80;                      // Enable DAC1 in right-justified mode
                                       // managed by on-demand while write to DAC1H
   SFRPAGE = ADC2_PAGE;
   REF2CN |= 0x0F;                     // Enable the internal Vref(2.4v) and
                                       // the Bias Generator, Tempsensor
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}


void ADC2_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = ADC2_PAGE;
   ADC2CN = 0x04;                      // ADC2 disabled; normal tracking
                                       // mode; ADC2 conversions are initiated
                                       // on overflow of Timer3; ADC2 data is
                                       // right-justified
   REF2CN |= 0x03;                      // Enable on-chip VREF,
                                       // and VREF output buffer
   AMX2CF = 0x00;                      // AIN inputs are single-ended (default)
   AMX2SL = 0x01;                      // Select AIN2.1 pin as ADC mux input
   ADC2CF = (SYSTEMCLOCK/SAR_CLK) << 3;     // ADC conversion clock = 2.5MHz
   EIE2 |= 0x10;                       // enable ADC interrupts
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//-----------------------------------------------------------------------------
// TIMER3_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1)  int counts - calculated Timer overflow rate
//                    range is postive range of integer: 0 to 32767
//
// Configure Timer3 to auto-reload at interval specified by <counts> (no
// interrupt generated) using SYSCLK as its time base.
//
//-----------------------------------------------------------------------------
void TIMER3_Init (int counts)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = TMR3_PAGE;
   TMR3CN = 0x00;                      // Stop Timer3; Clear TF3;
   TMR3CF = 0x00;                      // use SYSCLK/12 as timebase
   RCAP3   = -counts;                  // Init reload values
   TMR3    = RCAP3;                    // Set to reload immediately
   EIE2   &= ~0x01;                    // Disable Timer3 interrupts
   TR3     = 1;                        // start Timer3
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//-----------------------------------------------------------------------------
// Set_DACs
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : value - voltage in mV
//-----------------------------------------------------------------------------
void Set_DACs(int value)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   int toDAC;
   toDAC = value*DAC_KOEFFS; 
  
   SFRPAGE = DAC1_PAGE;
   DAC1 = toDAC;           // Write to DAC1
   SFRPAGE = DAC0_PAGE;
   DAC0 = toDAC;           // Write to DAC0
   SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//------------------------------------------------------------------------------------
// Interrupt Service Routines
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// ADC2_ISR
//-----------------------------------------------------------------------------
//
// Here we take the ADC2 sample, add it to a running total <accumulator>, and
// decrement our local decimation counter <int_dec>.  When <int_dec> reaches
// zero, we post the decimated result in the global variable <Result>.
//
//-----------------------------------------------------------------------------
void ADC2_ISR (void) interrupt 18
{
   static unsigned int_dec=INT_DEC;    // Integrate/decimate counter
                                       // we post a new result when
                                       // int_dec = 0
   static long accumulator=0L;         // Here's where we integrate the
                                       // ADC samples
   AD2INT = 0;                         // Clear ADC conversion complete
                                       // indicator
   accumulator += ADC2;                // Read ADC value and add to running
                                       // total
   int_dec--;                          // Update decimation counter
   if (int_dec == 0)                   // If zero, then post result
   {
      int_dec = INT_DEC;               // Reset counter
      Result = accumulator >> 4;
      accumulator = 0L;                // Reset accumulator
   }
}


//void UART0_Interrupt (void) interrupt 4
//{
//   SFRPAGE = UART0_PAGE;

//   if (RI0 == 1)
//   {
//     // Check if a new word is being entered
//     if( UART_Buffer_Size == 0) {
//        UART_Input_First = 0; }

//     RI0 = 0;
//     Byte = SBUF0;   // Read a character from Hyperterminal
//     
//     if (UART_Buffer_Size < UART_BUFFERSIZE)
//     {
//       UART_Buffer[UART_Input_First] = Byte;
//       UART_Buffer_Size++;            // Update array's size
//       UART_Input_First++;            // Update counter
//       if (Byte == '\n')
//       {
//         RX_Ready = 1;
//       }
//     }
//   }

////   if (TI0 == 1)           // Check if transmit flag is set
////   {
////      TI0 = 0;

////      if (UART_BufferOut_Size > 0)        // If buffer not empty
////      {
////         // Check if a new word is being output
////         //if ( UART_BufferOut_Size == UART_Output_First )  {
////           //   UART_Output_First = 0;   }

////         Byte = UART_BufferOut[UART_Output_First];

////         //if ((Byte >= 0x61) && (Byte <= 0x7A)) { // If upper case letter
////           // Byte -= 32; }

////         SBUF0 = Byte;                   // Transmit to Hyperterminal
////         UART_Output_First++;             // Update counter
////         UART_BufferOut_Size--;              // Decrease array size
////      }
////      else
////      {
////        UART_BufferOut_Size = 0;          // Set the array size to 0
////        UART_Output_First = 0;
////        TX_Ready = 1;                  // Transmission complete
////      }
////   }
//}

