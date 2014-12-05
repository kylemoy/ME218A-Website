/****************************************************************************
 Module
   helperFunctions.c

 Revision
   1.0.1

 Description
   This is a delay function for implementing a simple service under the 
   Gen2 Events and Services Framework.

 NOTES: **********************************************************************			
 *****************************************************************************
 
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this module 
*/
#include "helperFunctions.h"

/*------------------------------ Module Function ------------------------------*/
/****************************************************************************
 Function
     wait

 Parameters
     int : delay time

 Returns
     void

 Description
     delay for the input int delay
		 
 Notes

****************************************************************************/
void wait (int delay) { 
	// initialize local variable time_start and time
	unsigned int time_start = ES_Timer_GetTime(); 
	unsigned int time = ES_Timer_GetTime();
	
	// loop until current time is larger than start time plus delay time
	while (time < time_start + delay) 
	{
		HWREG(SYSCTL_RCGCGPIO);
		time = ES_Timer_GetTime();
	}
}
