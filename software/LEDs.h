/****************************************************************************
 
  Header file for LEDs.c -- the LEDs and their shift register 

 ****************************************************************************/

#ifndef LEDS_H
#define LEDS_H

// Public Function Prototypes

// intializes the Tiva ports for LED usage
void LEDShiftRegInit (void); 

// turns LEDs on and off based on input array
void setLED (char LEDs[8]); 

#endif
