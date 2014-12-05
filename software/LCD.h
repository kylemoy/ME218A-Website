/****************************************************************************
 
  Header file for LCD.c -- the LCD and its shift register 

 ****************************************************************************/


#ifndef LCD_H
#define LCD_H
#include <stdint.h>
#include "helperFunctions.h"

// Public Function Prototypes

 //initializes Tiva ports and then 4 bit mode of LCD
void LCDInit (void);

 //clears the LCD screen
void clearLCD(void);

// resets the LCD screen so that it beings at "Welcome DrEd" if LCD is activated
void resetLCDmessage(void);

// prints out sequence of LCD messages that welcome DrEd back and gives him passcodes
void printLCDmessage(void);

// prints out a message saying "Authorized!"
void printAuthorizedMessage(void);

// prints out "incorrect passcode"
void printIncorrectMessage(void);

// prints out  "Armed"
void printArmedMessage(void);

// prints out "Your Time is up!"
void printTimeUp(void);

#endif
