//#define TEST
/****************************************************************************
 Module
   Servos.c

 Description
   This file contains the function that will control the Servo motors for 
	 the flag, tower rotate, tower tilt, and potentiometer.
****************************************************************************/
#include "Servos.h" 
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ES_Port.h"
#include "inc/hw_memmap.h" 
#include "inc/hw_types.h" 
#include "inc/hw_gpio.h" 
#include "inc/hw_sysctl.h" 
#include "driverlib/gpio.h" 
#include "driverlib/sysctl.h"
#include "termio.h"
#include "PWMTiva.h"
#include "ADCSWTrigger.h"

#define clrScrn() 	printf("\x1b[2J")

// Port pin declaration for the Potentiometer
#define POT_CHAN 8 // 8 = PE5
#define POT_HIGH 3.3 // High on Pot
#define POT_LOW 0 // Low on Pot

// Port pin declaration for the Servos
#define TOWER_SERVO 3 // Pin PB5
#define TOWER_ROTATE_SERVO 1 // Pin PB7
#define KEY_SERVO 2 // Pin PB4
#define FLAG_SERVO 0 // Pin PB6

// Define the positions for the different servos
#define TOWER_HIGH 1540
#define TOWER_ZERO_DEGREES 1440
#define TOWER_LOW 1390
#define KEY_LOCK	920
#define KEY_UNLOCK	2020
#define FLAG_RAISED 1175
#define FLAG_LOWERED 2250
#define TOWER_ROTATE_LEFT 1750
#define TOWER_ROTATE_RIGHT 1500

// Defines the extreme ends of the servos
#define PLUS_90_DEGREES		2850
#define ZERO_DEGREES 			1400 
#define MINUS_90_DEGREES 	750

// Static variables to store the servo positions
static double potZero = -1;
static bool victory = false;
static uint16_t key_servo_pos = ZERO_DEGREES;
static uint16_t tower_servo_pos = ZERO_DEGREES;
static uint16_t tower_rotate_servo_pos = ZERO_DEGREES;
static uint16_t flag_servo_pos = ZERO_DEGREES;

// Private functions
void rotateServo(uint8_t channel, uint16_t position);
void rotateServoLeft(uint8_t channel);
void rotateServoRight(uint8_t channel);
void rotateServoToPlusNinety(uint8_t channel);
void rotateServoToMinusNinety(uint8_t channel);
void rotateServoToZero(uint8_t channel);

/****************************************************************************
 Function
     rotateTowerLeft
 Parameters
     void
 Returns
     void
 Description
     Rotates the tower to the left
 Notes
****************************************************************************/
void rotateTowerLeft(void) {
	rotateServo(TOWER_ROTATE_SERVO, TOWER_ROTATE_LEFT);
}

/****************************************************************************
 Function
     rotateTowerRight
 Parameters
     void
 Returns
     void
 Description
     Rotates the tower to the right
 Notes
****************************************************************************/
void rotateTowerRight(void) {
	rotateServo(TOWER_ROTATE_SERVO, TOWER_ROTATE_RIGHT);
}

/****************************************************************************
 Function
     raiseFlag
 Parameters
     void
 Returns
     void
 Description
     Raises the flag
 Notes
****************************************************************************/
void raiseFlag(void) {
	rotateServo(FLAG_SERVO, FLAG_RAISED);
}

/****************************************************************************
 Function
     lowerFlag
 Parameters
     void
 Returns
     void
 Description
     Lowers the flag
 Notes
****************************************************************************/
void lowerFlag(void) {
	rotateServo(FLAG_SERVO, FLAG_LOWERED);
}

/****************************************************************************
 Function
     lockKeys
 Parameters
     void
 Returns
     void
 Description
     Locks the key
 Notes
****************************************************************************/
void lockKeys(void) {
	rotateServo(KEY_SERVO, KEY_LOCK);
}

/****************************************************************************
 Function
     unlockKeys
 Parameters
     void
 Returns
     void
 Description
     Unlocks the key
 Notes
****************************************************************************/
void unlockKeys(void) {
	rotateServo(KEY_SERVO, KEY_UNLOCK);
}

/****************************************************************************
 Function
     setVictory
 Parameters
     bool state: state of the game (true for victory, false for no victory)
 Returns
     void
 Description
     Sets the state of the game (true for victory, false for no victory).
     Necessary for the event checker to know whether to enable the pot or not
 Notes
****************************************************************************/
void setVictory(bool state) {
	victory = state;
}

/****************************************************************************
 Function
     getVictory
 Parameters
     void
 Returns
     bool state: state of the game (true for victory, false for no victory)
 Description
     Returns the state of the game (true for victory, false for no victory).
 Notes
****************************************************************************/
bool getVictory(void) {
	return victory;
}

/****************************************************************************
 Function
     setPotZero
 Parameters
     void
 Returns
     void
 Description
     Sets the current position of the pot to be the zero value
 Notes
****************************************************************************/
void setPotZero(void) {
	potZero = getPotValue();
	victory = false;
}

/****************************************************************************
 Function
     getPotZero
 Parameters
     void
 Returns
     double : pot zero value (in volts)
 Description
     Returns the pot zero value
 Notes
****************************************************************************/
double getPotZero(void) {
	return potZero;
}

/****************************************************************************
 Function
     getPotValue
 Parameters
     void
 Returns
     double : pot value (in volts)
 Description
     Returns the pot  value
 Notes
****************************************************************************/
double getPotValue(void) 
{
	static double conversion = POT_HIGH / 4096; //Volts/point value
	double Direct_pot = 0; //this variable will be used to store the direct readout value
	double pot_val = 0;    //the pot value converted to volts
	Direct_pot = (double) ADC0_InSeq3(); // reads pin value, cast as a double
	pot_val = Direct_pot * conversion;   // convert pot value to volts and return the value
	return pot_val;
}

/****************************************************************************
 Function
     moveTower
 Parameters
     double potValue : the pot value
 Returns
     void
 Description
     Moves the tower to a position based on the pot value
 Notes
****************************************************************************/
void moveTower(double potValue) 
{
	// the initialization is undone
	if (potZero == -1) 
	{
		printf("Error: PotZero not inititialized.");
		return;
	}
	// High on Pot = Low on Motor
	if (potZero > 1.5) 
	{
		printf("\n\r Game Type A\r\n");
		uint16_t tower_servo_pos = (potZero - potValue) / (potZero - POT_LOW) * (TOWER_ZERO_DEGREES - TOWER_LOW) + TOWER_ZERO_DEGREES;
		printf("\n\r Pot val: %lf, Tower servo pos: %d \r\n", potValue, tower_servo_pos);
		rotateServo(TOWER_SERVO, tower_servo_pos);
	} 
	// Low on Pot = High on Motor
	else 
	{
		printf("\n\r Game Type B\r\n");
		uint16_t tower_servo_pos = TOWER_ZERO_DEGREES - (potValue - potZero) / (POT_HIGH - potZero) * (TOWER_ZERO_DEGREES - TOWER_LOW);
		//uint16_t tower_servo_pos = (potValue - potZero) / (POT_HIGH - potZero) * (TOWER_HIGH - TOWER_ZERO_DEGREES) + TOWER_ZERO_DEGREES;
		printf("\n\r Pot val: %lf, Tower servo pos: %d \r\n", potValue, tower_servo_pos);
		rotateServo(TOWER_SERVO, tower_servo_pos);
	}
}

/****************************************************************************
 Function
     initializeServos
 Parameters
     void
 Returns
     void
 Description
     Initializes the port pins for all the servo motors
 Notes
****************************************************************************/
void initializeServos(void) 
{
	// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);
  // initialize the timer sub-system and console I/O
  _HW_Timer_Init(ES_Timer_RATE_1mS);
	// Initialize the pot
	ADC0_InitSWTriggerSeq3(POT_CHAN); 
	PWM_TIVA_Init();
  PWM_TIVA_SetFreq(50, 0);
  PWM_TIVA_SetFreq(50, 1);
	// initialize the PWM pulse width for key, tower and flag motor
  PWM_TIVA_SetPulseWidth(key_servo_pos, KEY_SERVO);
	PWM_TIVA_SetPulseWidth(tower_rotate_servo_pos, TOWER_ROTATE_SERVO);
	PWM_TIVA_SetPulseWidth(tower_servo_pos,TOWER_SERVO);
	PWM_TIVA_SetPulseWidth(flag_servo_pos,FLAG_SERVO);
}

/****************************************************************************
 Function
     setTowerToZero
 Parameters
     void
 Returns
     void
 Description
     Sets the Tower servo position to zero
 Notes
****************************************************************************/
void setTowerToZero(void) {
	rotateServo(TOWER_SERVO, TOWER_ZERO_DEGREES);
}

/****************************************************************************
 rotateServo
     rotateServo
 Parameters
     uint8_t channel : the servo channel (TOWER_SERVO, TOWER_ROTATE_SERVO, KEY_SERVO, or FLAG_SERVO)
     uint16_t position : the position of the servo
 Returns
     void
 Description
     Rotates the servo to the specified position
 Notes
****************************************************************************/
void rotateServo(uint8_t channel, uint16_t position) 
{
	// if the given position is in the allowed range
	if (!(position < MINUS_90_DEGREES || position > PLUS_90_DEGREES)) 
	{
		// if key servo motor is chosen
		if (channel == KEY_SERVO) 
		{
			key_servo_pos = position;
			PWM_TIVA_SetPulseWidth(key_servo_pos, KEY_SERVO);
		} 
		// if tower rotate motor is chosen
		else if (channel == TOWER_ROTATE_SERVO) 
		{
			tower_rotate_servo_pos = position;
			PWM_TIVA_SetPulseWidth(tower_rotate_servo_pos,TOWER_ROTATE_SERVO);
		} 
		// if tower servo motor is chosen
		else if (channel == TOWER_SERVO) 
		{
			tower_servo_pos = position;
			PWM_TIVA_SetPulseWidth(tower_servo_pos,TOWER_SERVO);
		} 
		// if flag servo motor is chosen
		else if (channel == FLAG_SERVO) 
		{
			flag_servo_pos = position;
			PWM_TIVA_SetPulseWidth(flag_servo_pos,FLAG_SERVO);
		}
	}
}

/****************************************************************************
 rotateServo
     rotateServoLeft
 Parameters
     uint8_t channel : the servo channel (TOWER_SERVO, TOWER_ROTATE_SERVO, KEY_SERVO, or FLAG_SERVO)
 Returns
     void
 Description
     Rotates the specified servo left by an incremental amount
 Notes
****************************************************************************/
void rotateServoLeft(uint8_t channel) {
	// if key servo motor is chosen
	if (channel == KEY_SERVO) 
	{
		// if current key_servo_pos - 10 is still in the allowed range 
		if (!(key_servo_pos - 10 < MINUS_90_DEGREES))
			// rotate key servo to the left by 10 
			key_servo_pos = key_servo_pos - 10;
		PWM_TIVA_SetPulseWidth(key_servo_pos, KEY_SERVO);
	} 
	// if tower rotate motor is chosen
	else if (channel == TOWER_ROTATE_SERVO) 
	{
		// if current tower rotate servo pos - 15 is still in the allowed range
		if (!(tower_rotate_servo_pos - 15 < MINUS_90_DEGREES))
			// rotate tower rotate servo to the left by 15
			tower_rotate_servo_pos = tower_rotate_servo_pos - 15;
		PWM_TIVA_SetPulseWidth(tower_rotate_servo_pos, TOWER_ROTATE_SERVO);
	} 
	// if tower servo motor is chosen
	else if (channel == TOWER_SERVO) {
		if (!(tower_servo_pos - 2 < MINUS_90_DEGREES))
			// rotate tower servo to the left by 2
			tower_servo_pos = tower_servo_pos - 2;
		PWM_TIVA_SetPulseWidth(tower_servo_pos, TOWER_SERVO);
	} 
	// if flag servo motor is chosen
	else if (channel == FLAG_SERVO) {
		if (!(flag_servo_pos - 10 < MINUS_90_DEGREES))
			// rotate flag servo to the left by 10
			flag_servo_pos = flag_servo_pos - 10;
		PWM_TIVA_SetPulseWidth(flag_servo_pos, FLAG_SERVO);
	}
}

/****************************************************************************
 rotateServo
     rotateServoRight
 Parameters
     uint8_t channel : the servo channel (TOWER_SERVO, TOWER_ROTATE_SERVO, KEY_SERVO, or FLAG_SERVO)
 Returns
     void
 Description
     Rotates the specified servo right by an incremental amount
 Notes
****************************************************************************/
void rotateServoRight(uint8_t channel) {
	// if key servo motor is chosen
	if (channel == KEY_SERVO) {
		if (!(key_servo_pos + 10 > PLUS_90_DEGREES))
			key_servo_pos = key_servo_pos + 10;
		PWM_TIVA_SetPulseWidth(key_servo_pos, KEY_SERVO);
	} 
	// if tower rotate motor is chosen
	else if (channel == TOWER_ROTATE_SERVO) {
		if (!(tower_rotate_servo_pos + 15 > PLUS_90_DEGREES))
			tower_rotate_servo_pos = tower_rotate_servo_pos + 15;
		PWM_TIVA_SetPulseWidth(tower_rotate_servo_pos, TOWER_ROTATE_SERVO);
	} 
	// if tower servo motor is chosen
	else if (channel == TOWER_SERVO) {
		if (!(tower_servo_pos + 2 > PLUS_90_DEGREES))
			tower_servo_pos = tower_servo_pos + 2;
		PWM_TIVA_SetPulseWidth(tower_servo_pos, TOWER_SERVO);
	} 
	// if flag servo motor is chosen
	else if (channel == FLAG_SERVO) {
		if (!(flag_servo_pos + 10 > PLUS_90_DEGREES))
			flag_servo_pos = flag_servo_pos + 10;
		PWM_TIVA_SetPulseWidth(flag_servo_pos, FLAG_SERVO);
	}
}



#ifdef TEST
/* Test Harness for testing the servo motor module */
int main(void)
{
	// Set the clock to run at 40MhZ using the PLL and 16MHz external crystal
	SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN
			| SYSCTL_XTAL_16MHZ);
  // initialize the timer sub-system and console I/O
  _HW_Timer_Init(ES_Timer_RATE_1mS);
	TERMIO_Init();
	clrScrn();

	puts("\rStarting Servo Test \r");
	printf("%s %s\n",__TIME__, __DATE__);
	printf("\n\r\n");
  initializeServos();
	while (true) {
		char input = getchar();
		switch (input) {
			case 'a':
				rotateServoLeft(TOWER_SERVO);
				printf("Servo Position is %d\r\n", tower_servo_pos);
				break;
			
			case 's':
				rotateServoRight(TOWER_SERVO);
				printf("Servo Position is %d\r\n", tower_servo_pos);
				break;
			
			case 'q':
				rotateServoLeft(FLAG_SERVO);
				printf("Servo Position is %d\r\n", flag_servo_pos);
				break;
			
			case 'w':
				rotateServoRight(FLAG_SERVO);
				printf("Servo Position is %d\r\n", flag_servo_pos);
				break;
			
			case 'e':
				lockKeys();
				printf("Locking Keys...\r\n");
				printf("Servo Position is %d\r\n", key_servo_pos);
				break;
			
			case 'r':
				unlockKeys();
				printf("Unlocking Keys...\r\n");
				printf("Servo Position is %d\r\n", key_servo_pos);
				break;
			
			case 't':
				raiseFlag();
				printf("Raising Flag...\r\n");
				printf("Servo Position is %d\r\n", flag_servo_pos);
				break;
			
			case 'y':
				lowerFlag();
				printf("Lowering Flag...\r\n");
				printf("Servo Position is %d\r\n", flag_servo_pos);
				break;
			
			case 'f':
				printf("Invoking Fear and Panic...\r\n");
				while (true) {
					rotateServoRight(TOWER_ROTATE_SERVO);

					printf("Servo Position is %d\r\n", tower_rotate_servo_pos);
					rotateServoLeft(TOWER_ROTATE_SERVO);
					printf("Servo Position is %d\r\n", tower_rotate_servo_pos);
				}
				break;
		}
	}
}
#endif
