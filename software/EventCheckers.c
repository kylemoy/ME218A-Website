/****************************************************************************
 Module
   EventCheckers.c

 Revision
   1.0.1 

 Description
   This file contains the event checkers for the Leaning Tower of Pisa

 Notes
****************************************************************************/
#include "ES_Configure.h"
#include "ES_Events.h"
#include "ES_PostList.h"
#include "ES_ServiceHeaders.h"
#include "ES_Port.h"
#include "EventCheckers.h"
#include "TapeSensor.h"
#include "Servos.h"
#include "SlotDetector.h"
#include "DCmotors.h"
#include "AdafruitAudioService.h"

/****************************************************************************
 Function
   Check4Keystroke
 Parameters
   None
 Returns
   bool: true if a new key was detected & posted
 Description
   Takes in a key stroke and responds accordingly - used for debugging and 
	 module-by-module debugging
 Notes
	Input 1 --> emulates the three tape sensors being covered (changes state)
	Input 2 --> emulates moving to stage1 stagnated
	Input 3 --> emulates input of correct password
	Input 4 --> emulates incorrect password
	Input 5 --> emulates key being reinstered into the right palce
	Input 6 --> emulates correct pin code entry
	
	Input q --> rewinds timing motor
	Input w --> unwinds timing motor
	Input e --> stops timing motor
	Input a,s,d --> plays tracks 1, 2, and 3 respectively
	
****************************************************************************/
bool Check4Keystroke(void)
{
  if ( IsNewKeyReady() )
  {
		ES_Event ThisEvent;
    ThisEvent.EventType = ES_NEW_KEY;
    ThisEvent.EventParam = GetNewKey();
		if ( ThisEvent.EventParam == '1'){
			ThisEvent.EventType = THREE_HANDS_ON;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == '2'){
			ThisEvent.EventType = THREE_HANDS_OFF;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == '3'){
			ThisEvent.EventType = CORRECT_PASSWORD_ENTERED;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == '4'){
			ThisEvent.EventType = INCORRECT_PASSWORD_ENTERED;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == '5'){
			ThisEvent.EventType = KEY_INSERTED ;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == '6'){
			ThisEvent.EventType = CORRECT_VALUE_DIALED ;
      PostDisarmFSM( ThisEvent );
		} else if ( ThisEvent.EventParam == 'q'){
			printf("Rewinding Timing Motor\r\n");
			rewindTimingMotor();
		} else if ( ThisEvent.EventParam == 'w'){
			printf("Unwinding Timing Motor\r\n");
			unwindTimingMotor();
		} else if ( ThisEvent.EventParam == 'e'){
			printf("Stopping Timing Motor\r\n");
			stopTimingMotor();
		} else if ( ThisEvent.EventParam == 'a'){
			printf("Playing Wahooo!\r\n");
			ES_Event ThisEvent;
			ThisEvent.EventType = PLAY_TRACK;
			// play track 01
			ThisEvent.EventParam = 1; 					
			PostAdafruitAudioService(ThisEvent);	
		} else if ( ThisEvent.EventParam == 's'){
			printf("Playing the pot sound!\r\n");
			ES_Event ThisEvent;
			ThisEvent.EventType = PLAY_TRACK;
			// play track 02
			ThisEvent.EventParam = 2; 					
			PostAdafruitAudioService(ThisEvent);	
		} else if ( ThisEvent.EventParam == 'd'){
			printf("Playing Success!\r\n");
			ES_Event ThisEvent;
			ThisEvent.EventType = PLAY_TRACK;
			// play track 03
			ThisEvent.EventParam = 3; 					
			PostAdafruitAudioService(ThisEvent);	
		}else   {
			PostDisarmFSM( ThisEvent );
		}
    return true;
  }
  return false;
}

/****************************************************************************
 Function
   CheckTapeSensor
 Parameters
   None
 Returns
   bool: true if all three tape sensors are covered/uncovered
 Description
  Responds to the change of state
 Notes
	A NAND gate is being used for this circuit, so note that when all three are covered
	the Tiva sees a LO.
****************************************************************************/

bool CheckTapeSensor(void) {
	static bool lastState = true;
	// lastState == false corresponds to being uncovered
	// lastState == true corresponds to being covered
	if (tapeSensorsCovered() && (lastState == false)) {
		ES_Event ThisEvent;
    ThisEvent.EventType = THREE_HANDS_ON ;
    PostDisarmFSM( ThisEvent );
		lastState = true;
		return true;
	} else if (!(tapeSensorsCovered()) && (lastState == true)) {
		ES_Event ThisEvent;
    ThisEvent.EventType = THREE_HANDS_OFF ;
    PostDisarmFSM( ThisEvent );
		lastState = false;
		return true;
	}
	return false;
}

/****************************************************************************
 Function
   CheckSlotDetector
 Parameters
   None
 Returns
   bool: true if the key slot detects a change
 Description
  Responds to the change of state
 Notes
	There is only one tape sensor, so the code will only note if the tape sensor
	saw a change of state either from covered to uncovered or uncovered to covered.
	Both types of change will trigger the KEY_INSERTED event.
****************************************************************************/
bool CheckSlotDetector(void) {
	static bool lastState = true;
	bool keyInSlot = isKeyInSlot();

	// Checks if key has been placed into slot
	if (keyInSlot && (lastState == false)) {
		printf("Key has been placed.\r\n");
		lastState = true;
		ES_Event ThisEvent;
    	ThisEvent.EventType = KEY_INSERTED;
    PostDisarmFSM( ThisEvent );
		return true;

	// Checks if key has been removed from slot
	} else if (!keyInSlot && (lastState == true)) {
		printf("Key has been removed.\r\n");
		ES_Event ThisEvent;
    	ThisEvent.EventType = KEY_INSERTED;
    PostDisarmFSM( ThisEvent );
		lastState = false;
		return true;
	}
	return false;
}

/****************************************************************************
 Function
   CheckPot
 Parameters
   None
 Returns
   bool: true if the pot has been changed
 Description
   Event checker to detect changes in the pot value
 Notes
	
****************************************************************************/

bool CheckPot(void) {
	static double lastPotValue = -1;
	double potZero = getPotZero();
	double potValue = getPotValue();
	bool victory = getVictory();
	// If pot has not been initialized yet or if victory has been achieved, do nothing
	if (potZero == -1 || victory) {
		return false;
	// Check victory condition for Game Type A (pot must be dialed left)
	// The conditional checks if potZero is above 1.5 (Game Type A)
	// Then checks if the pot value is within an acceptable range for victory
	} else if (potZero > 1.5 && (potValue < (potZero - 1.45)) && (potValue > (potZero - 1.55))) {
		printf("Pot Value: %f\r\n", potValue);
		printf("Correct Value!\r\n");
		ES_Event ThisEvent;
    ThisEvent.EventType = CORRECT_VALUE_DIALED ;
    PostDisarmFSM( ThisEvent );
		setVictory(true);
		return true;
	// Check victory condition for Game Type B (pot must be dialed right)
	// The conditional checks if potZero is below 1.5 (Game Type B)
	// Then checks if the pot value is within an acceptable range for victory
	} else if (potZero < 1.5 && (potValue < (potZero + 1.55)) && (potValue > (potZero + 1.45))) {
		printf("Pot Value: %f\r\n", potValue);
		printf("Correct Value!\r\n");
		ES_Event ThisEvent;
    ThisEvent.EventType = CORRECT_VALUE_DIALED ;
    PostDisarmFSM( ThisEvent );
		setVictory(true);
		return true;
	// Check for pot changes and moves the tower accordingly
	} else if (potValue < (lastPotValue - 0.05) || potValue > (lastPotValue + 0.05)) {
		moveTower(potValue);
		lastPotValue = potValue;
	}
	return false;
	
}

