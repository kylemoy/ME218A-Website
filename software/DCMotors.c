/****************************************************************************
 Module
   DCMotors.c

 Description
   This file contains the function that will control the DC motors for 
	 vibration motor or feather/ball dropper.
****************************************************************************/
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
#include "ES_Timers.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "utils/uartstdio.h"
#include "EnablePA25_PB23_PD7_PF0.h"
#include "DCmotors.h"

#define ALL_BITS (0xff <<2)
//#define TEST

/****************************************************************************
Pin Declarations
----------------
Port E1 (on/off) for Vibration Motor
Port E2 (on/off) and E3 (direction) for Timing Motor
****************************************************************************/
#define MOTOR_PORT_DEC SYSCTL_RCGCGPIO_R4 //port E
#define MOTOR_PORT GPIO_PORTE_BASE // port E base

#define VIB_MOTOR_PIN GPIO_PIN_1 // pin E1
#define TIMING_MOTOR_PIN1 GPIO_PIN_2 // pin E2, was TIME_MOTOR_PIN
#define TIMING_MOTOR_PIN2 GPIO_PIN_3 // pin E3, was TIME_MOTOR_DIRECTION_PIN

#define VIB_MOTOR_ON BIT1HI
#define TIME_MOTOR_ON BIT2HI
#define TIME_MOTOR_FORWARD BIT3HI

#define ON 1
#define OFF 0
#define FORWARD 1
#define BACKWARD 0


// Private Prototypes
void setMotor(int motor, int state);
bool getMotorState(int motor);


/****************************************************************************
 Function
     initMotors
 Parameters
     void
 Returns
     void
 Description
     Initializes the port pins for the vibration and timing motors
 Notes
****************************************************************************/
void initMotors(void) {
	// Port initializations
	PortFunctionInit();
	HWREG(SYSCTL_RCGCGPIO) |= (MOTOR_PORT_DEC); // Enable port A
	HWREG(MOTOR_PORT+GPIO_O_DEN) |= (VIB_MOTOR_PIN | TIMING_MOTOR_PIN1 | TIMING_MOTOR_PIN2); // Set ports to be Digital IO
	HWREG(MOTOR_PORT+GPIO_O_DIR) |= (VIB_MOTOR_PIN | TIMING_MOTOR_PIN1 | TIMING_MOTOR_PIN2); // Set ports to be Outputs
	HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~(VIB_MOTOR_PIN | TIMING_MOTOR_PIN1 | TIMING_MOTOR_PIN2); // Set ports to low state
}

/****************************************************************************
 Function
     unwindTimingMotor
 Parameters
     void
 Returns
     void
 Description
     Sets the timing motor to unwind
 Notes
****************************************************************************/
void unwindTimingMotor(void) {
	setMotor(TIMING_MOTOR_PIN1, OFF);
	setMotor(TIMING_MOTOR_PIN2, ON);
}

/****************************************************************************
 Function
     rewindTimingMotor
 Parameters
     void
 Returns
     void
 Description
     Sets the timing motor to rewind
 Notes
****************************************************************************/
void rewindTimingMotor(void) {
	setMotor(TIMING_MOTOR_PIN1, ON);
	setMotor(TIMING_MOTOR_PIN2, OFF);
}


/****************************************************************************
 Function
     stopTimingMotor
 Parameters
     void
 Returns
     void
 Description
     Sets the timing motor to stop
 Notes
****************************************************************************/
void stopTimingMotor(void) {
	setMotor(TIMING_MOTOR_PIN1, ON);
	setMotor(TIMING_MOTOR_PIN2, ON);
}

/****************************************************************************
 Function
     vibrationMotorOn
 Parameters
     void
 Returns
     void
 Description
     Turns the vibration motor on
 Notes
****************************************************************************/
void vibrationMotorOn(void) {
	setMotor(VIB_MOTOR_PIN, ON);
}

/****************************************************************************
 Function
     vibrationMotorOff
 Parameters
     void
 Returns
     void
 Description
     Turns the vibration motor off
 Notes
****************************************************************************/
void vibrationMotorOff(void) {
	setMotor(VIB_MOTOR_PIN, OFF);
}

/****************************************************************************
 Function
     setMotor
 Parameters
     int motor : the motor pin to set
     int state : the state to set the motor pin to
 Returns
     void
 Description
     Sets the state of the motor pins to high or low
 Notes
****************************************************************************/
void setMotor(int motor, int state) {
	switch (motor) {
		case VIB_MOTOR_PIN:
			if (state == ON)
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) |= (VIB_MOTOR_PIN);
			else
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~(VIB_MOTOR_PIN);
			break;
			
		case TIMING_MOTOR_PIN1:
			if (state == ON)
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) |= (TIME_MOTOR_ON);
			else
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~(TIME_MOTOR_ON);
			break;
			
		case TIMING_MOTOR_PIN2:
			if (state == FORWARD)
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) |= (TIME_MOTOR_FORWARD);
			else
				HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~(TIME_MOTOR_FORWARD);	
			break;
	}	
}

/****************************************************************************
 Function
     getMotorState
 Parameters
     int motor: the motor pin to check 
 Returns
     bool : true if motor pin is high, false if low
 Description
     Returns the state of the motor pin
 Notes
****************************************************************************/
bool getMotorState(int motor) {
	switch (motor) {
		case VIB_MOTOR_PIN:
			return ((HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) & VIB_MOTOR_PIN) == VIB_MOTOR_ON);
		case TIMING_MOTOR_PIN1:
			return ((HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) & TIMING_MOTOR_PIN1) == TIME_MOTOR_ON);
		case TIMING_MOTOR_PIN2:
			return ((HWREG(MOTOR_PORT+(GPIO_O_DATA + ALL_BITS)) & TIMING_MOTOR_PIN2) == TIME_MOTOR_FORWARD);
	}
	return true;
}
	

#ifdef TEST 
/* Test Harness for DC motor module */ 
int main(void) { 
	TERMIO_Init(); 
	puts("\n\r In Test Harness for DC Motor Module\r\n");
	initMotors();
	while(true){
		char input = getchar();
			switch (input) {
			case 'z':
				if (getMotorState(VIB_MOTOR_PIN) == ON) {
					setMotor(VIB_MOTOR_PIN, OFF);
					printf("Setting Vibration Motor Off\r\n");
				} else {
					setMotor(VIB_MOTOR_PIN, ON);
					printf("Setting Vibration Motor On\r\n");
				}
				break;
			
			case 'x':
				if (getMotorState(TIMING_MOTOR_PIN1) == ON) {
					setMotor(TIMING_MOTOR_PIN1, OFF);
					printf("Setting Time Motor Off\r\n");
				} else {
					setMotor(TIMING_MOTOR_PIN1, ON);
					printf("Setting Time Motor On\r\n");
				}
				break;
			
			case 'c':
				if (getMotorState(TIMING_MOTOR_PIN2) == FORWARD) {
					setMotor(TIMING_MOTOR_PIN2, BACKWARD);
					printf("Setting Time Motor Backward\r\n");
				} else {
					setMotor(TIMING_MOTOR_PIN2, FORWARD);
					printf("Setting Time Motor Forward\r\n");
				}
				break;
				
			case 'a':
				unwindTimingMotor();
				printf("Unwinding Timing Motor \r\n");
				break;
			
			case 's':
				rewindTimingMotor();
				printf("Rewinding Timing Motor \r\n");
				break;
			
			case 'd':
				stopTimingMotor();
				printf("Stopping Timing Motor \r\n");
				break;
			
		}
	}
}
#endif
