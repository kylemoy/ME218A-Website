//#define TEST
/****************************************************************************
 Module
   AdafruitAudioService.c

 Revision
   1.0.1

 Description
   This is a AdafruitAudio file for implementing a simple service under the 
   Gen2 Events and Services Framework. It is used to play audio from an
   Adafruit FX Sound Board.

 NOTES: **********************************************************************
	To play an audio track, do the following:
										ES_Event ThisEvent;
										ThisEvent.EventType = PLAY_TRACK;
										ThisEvent.EventParam = 1; 	// play track 01
										PostAdafruitAudioService(ThisEvent);						
 *****************************************************************************
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KeyPadFSM.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "ES_Port.h"
#include "termio.h"
#include "inc/hw_memmap.h" 
#include "inc/hw_types.h" 
#include "inc/hw_gpio.h" 
#include "inc/hw_sysctl.h" 
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "EnablePA25_PB23_PD7_PF0.h"

/*----------------------------- Module Defines ----------------------------*/
#define clrScrn() 	printf("\x1b[2J")
#define AUDIO_TRACK01 (GPIO_PIN_2)	//PF2
#define AUDIO_TRACK02 (GPIO_PIN_3)	//PF3
#define AUDIO_TRACK03 (GPIO_PIN_7)	//PC7
#define ALL_BITS (0xFF<<2)
#define ADAFRUIT_AUDIO_PULSE 100 // must be >50 milliseconds

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
void InitAdafruitAudioPortLines(void);
void pulseLow(uint8_t);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
uint8_t track = 0;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAdafruitAudioService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

****************************************************************************/
bool InitAdafruitAudioService ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
	
  InitAdafruitAudioPortLines();
	
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostAdafruitAudioService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

****************************************************************************/
bool PostAdafruitAudioService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAdafruitAudioService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
	Triggers an audio track to play by applying a low pulse to the relevant pin 
	on the Adafruit FX Sound Board. When this function is first called externally,
	the EventType will be PLAY_TRACK, EventParam will indicate the track number,
	and the function will begin the low pulse by setting the relevant pin Low. 
	After an internal timer times out, the EventType will be ES_TIMEOUT from an
	internal AUDIO_TIMER and this function will be called to end the low pulse 
	by setting the relevant pin back to High. 
	
 Notes
		In 'ES_Configure.h':
			#define TIMER14_RESP_FUNC RunAdafruitAudioService
			#define AUDIO_TIMER 14
****************************************************************************/
ES_Event RunAdafruitAudioService( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
  
	// set track line back to high to complete LowPulse
	if((ThisEvent.EventType == ES_TIMEOUT) && (ThisEvent.EventParam == AUDIO_TIMER))
	{
		switch(track)
		{
			case 1:
				HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= AUDIO_TRACK01;
			break;
			
			case 2:
				HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= AUDIO_TRACK02;
			break;
			
			case 3:
				HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) |= AUDIO_TRACK03;
			break;
			
			default:
			break;
		}
	} else if (ThisEvent.EventType == PLAY_TRACK){ // play track by pulsing line low. 
					// Here, set line low, then set back to high after a timer expires 
					// (timer initialized in PulseLow)
		switch(ThisEvent.EventParam)
		{
			case 1: //track 01
				pulseLow(1);
				track = 1;
			break;
		
			case 2: //track 02
				pulseLow(2);
				track = 2;
			break;
			
			case 3: //track 03
				pulseLow(3);
				track = 3;
			break;
		
			default:
				track = 0;
			break;
	}
}
	
  return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/

/****************************************************************************
 Function
    InitAdafruitAudioPortLines

 Parameters
	none
	
 Returns
	nothing	

Description
	Initializes all Tiva port lines to be used with the Adafruit FX Sound 
	Board
 Notes
****************************************************************************/
 void InitAdafruitAudioPortLines(void)
{
	// wait a few	clock cycles
	uint8_t pause = HWREG(SYSCTL_RCGCGPIO); 
	
	/* Enable Ports on the Tiva
	 * Port F | Port E | Port D | Port C | Port B | Port A
	 * Bit 5	|	Bit 4  | Bit 3  | Bit 2  | Bit 1  | Bit 0 
	 * example: HWREG(SYSCTL_RCGCGPIO) |= BIT1HI; // Enable GPIO Port B
	 */

	HWREG(SYSCTL_RCGCGPIO) |= BIT5HI; // Enable GPIO Port F
	HWREG(SYSCTL_RCGCGPIO) |= BIT2HI; // Enable GPIO Port C

	
	// wait a few	clock cycles after enabling clock
	pause = HWREG(SYSCTL_RCGCGPIO); 
	
	/* Set Port pins to digital or analog function
	 * GPIO_O_DEN bit: 1 = Digital, 0 = Analog
	 */
	HWREG(GPIO_PORTF_BASE+GPIO_O_DEN) |= (AUDIO_TRACK01 | AUDIO_TRACK02);// set audio port to Digital function
	HWREG(GPIO_PORTC_BASE+GPIO_O_DEN) |= (AUDIO_TRACK03);// set audio port to Digital function

	/* Set Port pins to input or output
	 * GPIO_O_DIR bit: 1 = Output, 0 = Input
	 */
	HWREG(GPIO_PORTF_BASE+GPIO_O_DIR) |= (AUDIO_TRACK01 | AUDIO_TRACK02);// set audio port to Output
	HWREG(GPIO_PORTC_BASE+GPIO_O_DIR) |= (AUDIO_TRACK03);// set audio port to Output
	
	printf("Done Initializing Adafruit Audio Port Lines\n\r");
	
	// initially set to HI because pulsing low makes it run.
	HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) |= (AUDIO_TRACK01 | AUDIO_TRACK02);
	HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) |= (AUDIO_TRACK03);
}

/****************************************************************************
 Function
    pulseLow

 Parameters
   uint8_t : the track number to pulse low

 Returns
	nothing
	
 Description
	Takes in a track number and pulls Low the Tiva port that is connected
	to that track number on the Adafruit FX Sound Board. Audio Track pins
	are #defined at the top of this file.
 Notes
****************************************************************************/
void pulseLow(uint8_t track)
{
	switch (track)
	{
		case 1:
			HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~AUDIO_TRACK01;
			ES_Timer_InitTimer(AUDIO_TIMER, ADAFRUIT_AUDIO_PULSE);
			break;
		
		case 2:
			HWREG(GPIO_PORTF_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~AUDIO_TRACK02;
			ES_Timer_InitTimer(AUDIO_TIMER, ADAFRUIT_AUDIO_PULSE);
			break;
		
		case 3:
			HWREG(GPIO_PORTC_BASE+(GPIO_O_DATA + ALL_BITS)) &= ~AUDIO_TRACK03;
			ES_Timer_InitTimer(AUDIO_TIMER, ADAFRUIT_AUDIO_PULSE);
			break;	
	}
}

/*------------------------------- Footnotes -------------------------------*/
# ifdef TEST
/* test Harness for testing this module */ 
#include "termio.h" 
int main(void)
{  
	// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);
	TERMIO_Init();
	clrScrn();

	ES_Return_t ErrorType;


	// When doing testing, it is useful to announce just which program
	// is running.
	puts("\rStarting Test Harness for \r");
	printf("the 2nd Generation Events & Services Framework V2.2\r\n");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");
	printf("Press any key to post key-stroke events to Service 0\n\r");
	printf("Press 'd' to test event deferral \n\r");
	printf("Press 'r' to test event recall \n\r");

	// Your hardware initialization function calls go here

	// now initialize the Events and Services Framework and start it running
	ErrorType = ES_Initialize(ES_Timer_RATE_1mS);
	if ( ErrorType == Success ) {

	  ErrorType = ES_Run();

	}
	//if we got to here, there was an error
	switch (ErrorType){
	  case FailedPost:
	    printf("Failed on attempt to Post\n");
	    break;
	  case FailedPointer:
	    printf("Failed on NULL pointer\n");
	    break;
	  case FailedInit:
	    printf("Failed Initialization\n");
	    break;
	 default:
	    printf("Other Failure\n");
	    break;
	}
	for(;;)
	  ;
	
	return 0;
}
#endif
	

/*------------------------------ End of file ------------------------------*/

