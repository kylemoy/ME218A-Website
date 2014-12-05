/****************************************************************************
 
  Header file for helperFunctions service 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/
 
 /*----------------------------- Include Files -----------------------------*/
/* include header files for any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "ES_Port.h"
#include "ES_Timers.h"
#include "termio.h"
#include "driverlib/gpio.h"

// Public Function Prototype
// this is a wait(delay) function
void wait (int delay); 
