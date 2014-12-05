//#define TEST
/****************************************************************************
 Module
   ArmedLine.c

 Description
   This file contains the function that will output the pin to high when
	 the DMM is armed.
****************************************************************************/
#include "ArmedLine.h"
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

// Defines for the port pins for the armed line
#define ALL_BITS (0xff <<2)
#define ARMED_LINE_PORT_DEC SYSCTL_RCGCGPIO_R5 //port F
#define ARMED_LINE_PORT GPIO_PORTF_BASE // port F base
#define ARMED_LINE_PIN GPIO_PIN_1 // pin 1
#define ARMED_LINE_HI BIT1HI

/****************************************************************************
 Function
     initArmedLine
 Parameters
     void
 Returns
     void
 Description
     Initializes the port pins for the armed line
 Notes
****************************************************************************/
void initArmedLine(void) {
	// Initialization of armed line port
	HWREG(SYSCTL_RCGCGPIO) |= (ARMED_LINE_PORT_DEC); // Enable port F
	HWREG(ARMED_LINE_PORT+GPIO_O_DEN) |= ARMED_LINE_PIN; // Pin 1
	HWREG(ARMED_LINE_PORT+GPIO_O_DIR) |= ARMED_LINE_PIN; // Set to be output
}

/****************************************************************************
 Function
     setArmed
 Parameters
     void
 Returns
     void
 Description
     Sets the armed line to Armed (high state)
 Notes
****************************************************************************/
void setArmed(void) {
	// Sets pin to 1
	HWREG(ARMED_LINE_PORT+(GPIO_O_DATA + ALL_BITS)) |= ARMED_LINE_PIN;
}

/****************************************************************************
 Function
     setUnarmed
 Parameters
     void
 Returns
     void
 Description
     Sets the armed line to Unarmed (low state)
 Notes
****************************************************************************/
void setUnarmed(void) {
	// Sets pin to 0
	HWREG(ARMED_LINE_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~ARMED_LINE_PIN;
}

#ifdef TEST 
/* Test Harness for Armed Line module */ 
int main(void) 
{ 
	TERMIO_Init(); 
	printf("\n\rIn Test Harness for Armed Line\r\n");
	initArmedLine();
	setArmed();
}
#endif
