#ifndef DCMOTORS_H
#define DCMOTORS_H

/****************************************************************************
 Module
   DCMotors.h

 Description
   This file contains the function that will control the DC motors for 
	 vibration motor or feather/ball dropper.
****************************************************************************/

// Public prototypes

// Initializes the port pins for the vibration and timing motors
void initMotors(void);

// Controls for the timing motor
void rewindTimingMotor(void);
void unwindTimingMotor(void);
void stopTimingMotor(void);

// Controls for the vibration motor
void vibrationMotorOn(void);
void vibrationMotorOff(void);


#endif
