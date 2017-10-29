#include <definitions.h>

#ifdef MONITOR51                         /* Debugging with Monitor-51 needs   */
char code reserve [3] _at_ 0x23;         /* space for serial interrupt if     */
#endif                                   /* Stop Exection with Serial Intr.   */
                                         /* is enabled                        */

/*
 * The main C function.  Program execution starts
 * here after stack initialization.
 */
void main (void) {
	char ch;
	//Setup the serial port for 1200 baud at 16MHz.
	initUART();

  while (1) {
		ch = getchar();
		switch(ch)
		{
			case 'p':
				printHelloWorld();
				break;
			case 'l':
				blinkLED();
			  break;
      case 'a':
			default:
				printf("Char is %c\r\n", ch);
		}
  }
}

void initUART()
{
#ifndef MONITOR51
    SCON  = 0x50;		        /* SCON: mode 1, 8-bit UART, enable rcvr      */
    TMOD |= 0x20;           /* TMOD: timer 1, mode 2, 8-bit reload        */
    TH1   = 0xFF;           /* TH1:  reload value for 115200 baud @ 22.184MHz   */
    TR1   = 1;              /* TR1:  timer 1 run                          */
    TI    = 1;              /* TI:   set TI to send first char of UART    */
#endif
}

void printHelloWorld()
{
    printf ("Hello World\n");   /* Print "Hello World" */
}

void blinkLED()
{
	P1 ^=0x01;         		    /* Toggle P1.0 each time we print */
}