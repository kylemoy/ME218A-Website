#ifndef ARMEDLINE_H
#define ARMEDLINE_H
/****************************************************************************
 Module
   ArmedLine.h

 Description
   This file contains the function that will output the pin to high when
	 the DMM is armed.
****************************************************************************/

// Initializes the port pins for the armed line
void initArmedLine(void);

// Sets the armed line to Armed (high state)
void setArmed(void);

// Sets the armed line to Unarmed (low state)
void setUnarmed(void);

#endif
