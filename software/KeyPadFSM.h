/****************************************************************************
 
  Header file for KeyPad Sate Machine 
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef KEYPAD_FSM_H
#define KEYPAD_FSM_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */

// typedefs for the states
// State definitions for use with the query function
 typedef enum { InitPState, ButtonPressed } KeyPadFSMState_t ;

// Public Function Prototypes
// Checks whether any key pad button was pressed
bool CheckForKeyPadButtonPress( void );

// Initializes key pad Firmware
bool InitKeyPadFSM ( uint8_t Priority );

// Post Event to key pad
bool PostKeyPadFSM( ES_Event ThisEvent );

// Records the sequence of numbers entered on the keypad
// and checks them against the internal password
ES_Event RunKeyPadFSM( ES_Event ThisEvent );

// Returns the current state of the KeyPadFSM state machine
KeyPadFSMState_t QueryKeyPadFSM ( void );


#endif /* KEYPAD_FSM_H */

