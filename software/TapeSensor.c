//#define TEST

/****************************************************************************
 Module
   TapeSensor.c

 Description
   This file contains the function that will read the tape sensor output and
   report its state back
	 
 Notes
****************************************************************************/
#include "TapeSensor.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "termio.h"
#include "ES_Port.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"

#define ALL_BITS (0xff <<2)
#define TAPE_PORT_DEC SYSCTL_RCGCGPIO_R3 //port D
#define TAPE_PORT GPIO_PORTD_BASE // port D base
#define TAPE_PIN GPIO_PIN_7 // pin 7
#define TAPE_HI BIT7HI

/****************************************************************************
 Function
     initTapeSensors

 Parameters
     none

 Returns
     none

 Description
     initializes port F4 to read the output from the NAND gate used in the three
		 tape sensors 
 Notes

****************************************************************************/
void initTapeSensors(void) {
	// Initialization of tape sensor port
	HWREG(SYSCTL_RCGCGPIO) |= (TAPE_PORT_DEC); // Enable port F
	HWREG(TAPE_PORT+GPIO_O_DEN) |= TAPE_PIN; // Pin 4
}

/****************************************************************************
 Function
     tapeSensorsCovered

 Parameters
     none

 Returns
     bool, true if all three tape sensors are covered and false otherwise

 Description
     reads the port connected to the NAND gate output, and sees HI if
		 all three tape sensors are covered, and LO if any one of them is uncovered
 Notes

****************************************************************************/
bool tapeSensorsCovered(void) {
	return !(HWREG(TAPE_PORT+(GPIO_O_DATA + ALL_BITS)) & TAPE_PIN);
}

/****************************************************************************
 Test Harness for Tape Sensors module

 Description
     intializes the relevant Tiva port and continuously prints out whether
		 the all three tape sensors are covered or not.
 Notes
****************************************************************************/
#ifdef TEST 
int main(void) 
{ 
	TERMIO_Init(); 
	printf("\n\rIn Test Harness for Tape Sensors\r\n");
	initTapeSensors();
	while(true){
		if (tapeSensorsCovered()) {
			printf("All three tape sensors are covered.\r\n");
		} else {
			printf("One or more tape sensors are not covered.\r\n");
		}
	}
}
#endif
