#include <definitions.h>

/*
 * The main C function.  Program execution starts
 * here after stack initialization.
 */
void main (void) {
  char c;
  long value; //for reading values
  char convertBuf[5];
  int measurementDAC;                   // Measured voltage in mV for adc1
  long measurement;                   // Measured voltage in mV
  long measurement2;                   // Measured voltage in mV for adc1
  SFRPAGE = CONFIG_PAGE;
  WDTCN = 0xDE;                       // Disable watchdog timer
  WDTCN = 0xAD;

  OSCILLATOR_Init ();                 // Initialize oscillator
  PORT_Init ();                       // Initialize crossbar and GPIO
  UART0_Init ();                      // Initialize UART0
  DAC0_Init ();                       // Initialize DAC0
  DAC1_Init ();                       // Initialize DAC1   
  //TIMER3_Init(); //1ms //10us
  ADC0_Init ();
  ADC1_Init ();
  ADC2_Init ();                       // Init ADC
  
  SFRPAGE = ADC2_PAGE;
  AD2EN = 1;                          // Enable ADC
  
  SFRPAGE = CONFIG_PAGE;
  EA = 1;                             //enable global interrupt

  SFRPAGE = UART0_PAGE;
  stateSTR = 0;  
  delayImpulses =1000;
  impulseWidth = 50;
  while (1) {
    if(RI0==1) //process command
    {
      if ( scanf("%c=%4s", &c, &convertBuf) == 2 )
      {
        value = (convertBuf[0]-'0')*1000 + 
                (convertBuf[1]-'0')*100 + 
                (convertBuf[2]-'0')*10 + 
                (convertBuf[3]-'0');
        
        if (c =='b') //LED state
        {
          LED = (value == 1111)? 0 : 1;
          printf("LED\n");
        }
        if (c =='r') //change reference
        {
          if (value == 1111)
          {
            Set_REFs('i');
            printf("REFInt\n");
          }
          else
          {
            Set_REFs('e');
            printf("REFOut\n");
          }
          
        }
        if (c == 'd') //DAC set
        {
          Set_DACs(value);
          printf("DAC%d\n", value*DAC_KOEFFS);
        }
        if (c == 'a') //read Value from DAC
        {
          Wait_MS(500);
          EA=0;
          SFRPAGE = ADC2_PAGE;
          AD2BUSY =1;
          while (!AD2INT) {;}
          AD2INT = 0;
          Result = ADC2;
          measurementDAC = (Result/1023.0)*2500.0;
          EA=1;
          SFRPAGE = UART0_PAGE;
          printf("ADC%d\n", measurementDAC);
        }
        if (c == 'l') //set impulse delay
        {
          delayImpulses = value;
//          printf("set to %d", (int)value);
        }
        if (c == 'm') //set impulse width
        { 
          if(value < 20)
            value = 20;
          impulseWidth = value;
//          printf("set to %d", (int)value);
        }
        if (c == 'n') //start impulse
        {
          stateSTR = (value == 1111)? 1 : 0;
        }
        else {
          stateSTR = 0; //always stop impulses
        }
        while( (c = _getkey()) != '\n' ){;}
      }
      else
      {
        while( (c = _getkey()) != '\n' ){;}
      }
    }    
    //make impulse and send adc data to uart0
    if(stateSTR == 1) //do impulses
    {
      Wait_MS(delayImpulses); 
      //do pulse
      //printf("Pulse!\n");
      LED =0;
      STR = 0;
      //EA=0; //disable interrupts
      Wait_US(10);
      
      //read 16bit ADCs
      SFRPAGE = ADC0_PAGE;  //
      AD0BUSY =1;           //start adc 0,1 conversion
      //while (!AD0INT) {;}
      AD0INT = 0;
      Result = ADC0;
      measurement = (Result);///65536.0)*2500.0;
      
      SFRPAGE = ADC1_PAGE;
      //while (!AD1INT) {;}
      AD0INT = 0;
      Result = ADC1;
      measurement2 = (Result);//65536.0)*2500.0;
      
      Wait_US(impulseWidth-15); 
      
      STR=1;
      LED=1;
        
      //SFRPAGE = ADC2_PAGE;  //
      //AD2BUSY =1;           //start adc 2  conversion
      //while (!AD2INT) {;}
      //AD2INT = 0;
      //Result = ADC2;
      //measurementDAC = (Result/1023.0)*2500.0;
      EA=1; //enable interrupts 
      SFRPAGE = UART0_PAGE;  
      printf("ADC0:%d\n", measurement);
      printf("ADC1:%d\n", measurement2);
      //printf("DAC:%d\n", measurementDAC);
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


  P0MDOUT |= 0xA1;                    // Set P0.7(STR), P0.5(led) and P0.0(TX) pin to push-pull
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

void ADC0_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;
   int i;

   SFRPAGE = ADC0_PAGE;                // Switch to ADC0 Page
   ADC0CN = 0x00;                      // ADC Disabled, convertion on AD0BUSY
   REF0CN = 0x03;                      // turn on bias generator and internal reference.
   for(i=0;i<10000;i++);               // Wait for Vref to settle (large cap used on target board)
   AMX0SL = 0x00;                      // Single-ended mode
   ADC0CF = (SYSTEMCLOCK/SAR_CLK) << 4;    // Select SAR clock frequency =~ 2.5MHz
   SFRPAGE = SFRPAGE_SAVE;              // restore SFRPAGE
}
void ADC1_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;
   int i;

   SFRPAGE = ADC1_PAGE;                // Switch to ADC0 Page
   ADC1CN = 0x02;                      // ADC Disabled, convertion on AD0BUSY
   REF1CN = 0x03;                      // turn on bias generator and internal reference.
   for(i=0;i<10000;i++);               // Wait for Vref to settle (large cap used on target board)
   ADC1CF = (SYSTEMCLOCK/SAR_CLK) << 4;    // Select SAR clock frequency =~ 2.5MHz
   SFRPAGE = SFRPAGE_SAVE;              // restore SFRPAGE
}
void ADC2_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = ADC2_PAGE;
   ADC2CN = 0x00;                      // ADC2 disabled; normal tracking
                                       // mode; ADC2 conversions are initiated
                                       // on AD2BUSY; ADC2 data is
                                       // right-justified
   REF2CN |= 0x03;                      // Enable on-chip VREF,
                                       // and VREF output buffer
   AMX2CF = 0x00;                      // AIN inputs are single-ended (default)
   AMX2SL = 0x00;                      // Select AIN2.1 pin as ADC mux input
   ADC2CF = (SYSTEMCLOCK/SAR_CLK) << 3;     // ADC conversion clock = 2.5MHz
  // EIE2 |= 0x10;                       // enable ADC interrupts
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
void TIMER3_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page
   SFRPAGE = TMR3_PAGE;
   TMR3CN = 0x00;                      // Stop Timer3; Clear TF3;
   TMR3CF = 0x00;//0x08                      // use SYSCLK/12 as timebase
   RCAP3   = -(SYSTEMCLOCK/1000/12);                  // Init reload values
   TMR3    = RCAP3;                    // Set to reload immediately
   EIE2   |= 0x01;                    // Enable Timer3 interrupts
   //TR3     = 1;                        // start Timer3
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

void Set_REFs(char internalRef)
{
  char SFRPAGE_SAVE = SFRPAGE;

  if(internalRef == 'i')
  {
    
    SFRPAGE = ADC0_PAGE;
       REF0CN |= 0x03; 
    SFRPAGE = ADC1_PAGE;
       REF1CN |= 0x03; 
    SFRPAGE = ADC2_PAGE;
       REF2CN |= 0x03;
  }
  else
  {
    SFRPAGE = ADC0_PAGE;
    REF0CN |= 0x02; 
    SFRPAGE = ADC1_PAGE;
    REF1CN |= 0x02; 
    SFRPAGE = ADC2_PAGE;
    REF2CN |= 0x02;
  }
  SFRPAGE = SFRPAGE_SAVE;
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
//void ADC2_ISR (void) interrupt 18
//{
//   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

//   static unsigned int_dec=INT_DEC;    // Integrate/decimate counter
//                                       // we post a new result when
//                                       // int_dec = 0
//   static long accumulator=0L;         // Here's where we integrate the
//                                       // ADC samples
//     SFRPAGE = ADC2_PAGE;

//   AD2INT = 0;                         // Clear ADC conversion complete
//                                       // indicator
//   accumulator += ADC2;                // Read ADC value and add to running
//                                       // total
//   int_dec--;                          // Update decimation counter
//  // ++adc;
//   if (int_dec == 0)                   // If zero, then post result
//   {
//      int_dec = INT_DEC;               // Reset counter
//      Result = accumulator >> 4;
//      accumulator = 0L;                // Reset accumulator

//   }
//   SFRPAGE = SFRPAGE_SAVE;
//}
//void Timer3_ISR (void) interrupt 14
//{
//  char SFRPAGE_SAVE = SFRPAGE; 
//  SFRPAGE = TMR3_PAGE;
//  TF3 = 0;                          //clear interrupt
//  if(impulseWidth > 0 )
//  {
//      impulseWidth--;                            // Decrement ms
//  }
//  else
//  {
//    impulseWidth = delayImpulses;
//    LED = ~LED;
//  }

//  
//  if(impulseWidth >= delayImpulses)
//  {
//    impulseWidth = 0;               //clear counter
//    if ( state_tmr3 == 1 )          //after pulse set level to high
//    {
//      state_tmr3 = 0;               //next will high level on pin
//      delayImpulses = 100000;          //1sec if timer resolution is 1 us
//      LED = 1;                      //set high level on pin
//    }
//    else if ( state_tmr3 == 0 )     //after bckgnd set level to low
//    {
//      state_tmr3 = 1;               //next will low level on pin
//      delayImpulses = 50000;           // 0.5 sec if timer resolution is 1 us
//      LED = 0;                      //set low level on pin
//    }
//  }
//  else
//  {
//    impulseWidth++;                 // Increment counter
//  }

//  SFRPAGE = SFRPAGE_SAVE;
//}
