//#define TEST

/****************************************************************************
 Module
   SlotDetector.c

 Description
   This file contains the function that will read the phototransistor output
   to test for the presence of a key in the slot.
****************************************************************************/
#include "SlotDetector.h"
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

// Defines for the port pins, using B2
#define ALL_BITS (0xff <<2)
#define PHOTOTRANS_PORT_DEC SYSCTL_RCGCGPIO_R1 //port B
#define PHOTOTRANS_PORT GPIO_PORTB_BASE
#define PHOTOTRANS_PIN GPIO_PIN_2 // pin 2
#define PHOTOTRANS_HI BIT2HI


/****************************************************************************
 Function
     initPhototransistor
 Parameters
     void
 Returns
     void
 Description
     Initializes the port pins for the slot detector
 Notes
****************************************************************************/
void initPhototransistor(void) {
	// Initialization of tape sensor port
	HWREG(SYSCTL_RCGCGPIO) |= (PHOTOTRANS_PORT_DEC);
	HWREG(PHOTOTRANS_PORT+GPIO_O_DEN) |= (PHOTOTRANS_PIN); 
}

/****************************************************************************
 Function
     isKeyInSlot
 Parameters
     void
 Returns
     true if key is in slot, false is key is not in the slot
 Description
     Checks if the key is in the slot
 Notes
****************************************************************************/
bool isKeyInSlot(void) {
	return (HWREG(PHOTOTRANS_PORT+(GPIO_O_DATA + ALL_BITS)) & PHOTOTRANS_HI);
}


#ifdef TEST 
/* Test Harness for Slot Detector module */ 
int main(void) 
{ 
	TERMIO_Init(); 
	printf("\n\rIn Test Harness for Slot Detector\r\n");
	initPhototransistor();
	while(true){
		if (isKeyInSlot()) {
			printf("Slot is covered.\r\n");
		} else {
			printf("Slot is not.\r\n");
		}
	}
}
#endif
