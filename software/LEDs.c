//#define TEST

/****************************************************************************
 Module
   LEDs.c

 Description
   This file contains the function that will control the LED array using the shift 
	 register data inputs, as well as the RCK/SCK clocks. It will take an ARRAY of 
	 8 numbers, 0 or 1 that will correspond to the 7 LEDs and one blank spot
****************************************************************************/
#include "LEDs.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "ES_Port.h"
#include "ES_Timers.h"
#include "EnablePA25_PB23_PD7_PF0.h"
#include "termio.h"
#include "helperFunctions.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
/****************************************************************************
  Change these values if ports A2 (data), A3 (RCK), A4 (SCK) are
	not being used for the shift register
****************************************************************************/
#define SHIFT_PORT_DEC SYSCTL_RCGCGPIO_R0 //port A
#define SHIFT_PORT GPIO_PORTA_BASE // port A base
#define SHIFT_DATA GPIO_PIN_2 // pin 2
#define SHIFT_SCK GPIO_PIN_3 // pin 3
#define SHIFT_RCK GPIO_PIN_4 // pin 4

#define ALL_BITS (0xff <<2)

// Prviate Function Prototypes
void SCKPulse (void);
void RCKPulse (void);

/****************************************************************************
 Function
     LEDShiftRegInit

 Parameters
     none

 Returns
     none

 Description
     initializes the three ports that the shift register used to control the LEDs
		 are connected to
		 it also initializes the timer system which are used to pulse
 Notes

****************************************************************************/
void LEDShiftRegInit (void) {
	// initializes timer -- matching the overall module which uses the 2mS rate
	_HW_Timer_Init(ES_Timer_RATE_2mS);
	
	// pin initialization to follow
	HWREG(SYSCTL_RCGCGPIO) |= (SHIFT_PORT_DEC); // enable port A
	HWREG(SHIFT_PORT + GPIO_O_DEN) |= (SHIFT_DATA | SHIFT_SCK | SHIFT_RCK); //enables pins
	HWREG(SHIFT_PORT + GPIO_O_DIR) |= (SHIFT_DATA | SHIFT_SCK | SHIFT_RCK); // makes pins as ouputs
	
	//writes pins LO to make sure it starts out LO (good for pulsing)
	HWREG(SHIFT_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~(SHIFT_DATA | SHIFT_SCK | SHIFT_RCK); 
	
	puts("\n\r Timer, Shift Register port and pins intialization complete \r\n");
}

/****************************************************************************
 Function
     SCKPulse

 Parameters
     none

 Returns
     none

 Description
     pulses the pin connected the SCK by writing it HI, then back to LO again
 Notes

****************************************************************************/
void SCKPulse (void) {
	// sets SCK pin HI
	HWREG(SHIFT_PORT+(GPIO_O_DATA + ALL_BITS)) |= SHIFT_SCK; 
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
	// sets SCK pin back LO
	HWREG(SHIFT_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~SHIFT_SCK; 
}

/****************************************************************************
 Function
     RCKPulse

 Parameters
     none

 Returns
     none

 Description
     pulses the pin connected the RCK by writing it HI, then back to LO again
 Notes

****************************************************************************/
void RCKPulse (void) {
	// sets RCK pin HI
	HWREG(SHIFT_PORT+(GPIO_O_DATA + ALL_BITS)) |= SHIFT_RCK; 
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
	// sets RCK pin back LO
	HWREG(SHIFT_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~SHIFT_RCK; 
	
}

/****************************************************************************
 Function
     setLED

 Parameters
     char array[8]

 Returns
     none

 Description
     Takes the 8 array values and writes them into the shift register one at a time
		 At the very end, it will pulse the RCK pin as well, so that this function alone
		 will turn the corresponding LEDs on/off
 Notes
		 It is assumed that each array value is only either 1 or 0. 0 corresponds
		 to the LED being ON, and 1 is OFF.
****************************************************************************/
void setLED (char LEDs[8]) {
	for (int i = 0; i < 8; i++) {
		int bit = 0;
		//clears data pin 
		HWREG(SHIFT_PORT + GPIO_O_DATA + ALL_BITS) &= ~SHIFT_DATA;
		
		/* if the input value was 1, sets the pin to HI
		   if the input value was 0, sets the pin to LO 
		
			 setting the local variable "bit" to the value being read and writes it
			 to the data pin to get pulsed 
		*/
		bit = LEDs[i];
		
		if (bit == 1) {
			HWREG(SHIFT_PORT + GPIO_O_DATA + ALL_BITS) |= SHIFT_DATA;
		}
		else if (bit == 0) {
			HWREG(SHIFT_PORT + GPIO_O_DATA + ALL_BITS) &= ~SHIFT_DATA;
		}
		
		// pulses SCK to send the single bit to shift register
		SCKPulse();
	}
	
	/* leaving the loop means it has gone through all 8 bits, so
	   the shfit register is ready to write it to LEDs
	*/
	RCKPulse();
}

/****************************************************************************
 Test Harness for LEDs shift register module

 Description
     intializes the relevant Tiva port and then turns on specified LEDs
 Notes
****************************************************************************/
#ifdef TEST 

// array index with their corresponding LED's location
#define Tier1 2
#define Tier2 0
#define Tier3 7
#define Tier4 1
#define Tier5 4
#define Tier6 5
#define pot 3

int main(void) 
{ 
	TERMIO_Init(); 
	puts("\n\r in test harness for LEDs shift register\r\n");
	
	// 0 is On, 1 is Off

	// Array index 6 is empty (no LED)
	char test[8] =  {1,1,1,1,1,1,1,1};
	test[Tier1] = 0;
	test[pot] = 0;
	LEDShiftRegInit();
	setLED (test);
}
#endif
