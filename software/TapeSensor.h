/****************************************************************************
 
  Header file for TapeSensor.c -- the three hands tape sensors 

 ****************************************************************************/

#ifndef TAPESENSOR_H
#define TAPESENSOR_H

// Public Function Prototypes

//initializes port F4 to read the output from the three tape sensors 
void initTapeSensors(void); 

// returns true if all three tape sensors are covered, false otherwise
bool tapeSensorsCovered(void); 

#endif
