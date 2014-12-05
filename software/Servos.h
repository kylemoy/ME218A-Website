#ifndef SERVOS_H
#define SERVOS_H
/****************************************************************************
 Module
   Servos.h

 Description
   This file contains the function that will control the Servo motors for 
	 the flag, tower rotate, tower tilt, and potentiometer.
****************************************************************************/

// Public function prototypes

// Initialization function for the servos
void initializeServos(void);

// Controls for rotating the tower
void rotateTowerLeft(void);
void rotateTowerRight(void);
void setTowerToZero(void);

// Controls for the flag
void raiseFlag(void);
void lowerFlag(void);

// Controls for the keys
void lockKeys(void);
void unlockKeys(void);


/* Potentiometer Controls */

// Sets the victory condition of the game (determines whether pot is enabled or not)
void setVictory(bool state);

// Returns the victory condition of the game
bool getVictory(void);

// Sets the current position of the pot to zero
// This is needed to alternate between turning the pot left or turning the pot right for victory
void setPotZero(void);

// Gets the pot zero value
double getPotZero(void);

// Gets the current pot value
double getPotValue(void);

// Moves the tower to the corresponding pot value
void moveTower(double potValue);




#endif
