/****************************************************************************
 Module
   DisarmFSM.c

 Revision
   1.0.1

 Description
   The state machine for the diarmment process.

 NOTES: **********************************************************************
	
 *****************************************************************************
 
 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from DisarmFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "DisarmFSM.h"
#include "TapeSensor.h"
#include "DCmotors.h"
#include "LEDs.h"
#include "Servos.h"
#include "LCD.h"
#include "passwordGenerator.h"
#include "ES_Timers.h"
#include "AudioService.h"
#include "SlotDetector.h"
#include "ArmedLine.h"
#include "AdafruitAudioService.h"

/*----------------------------- Module Defines ----------------------------*/
// low output voltage turns on the LED
#define ON 0
#define OFF 1

// LED number corresponding to tower layer and pot
#define Tier1 2
#define Tier2 0
#define Tier3 7
#define Tier4 1
#define Tier5 4
#define Tier6 5
#define pot 3

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this machine.They should be functions
   relevant to the behavior of this state machine
*/

/*---------------------------- Module Variables ---------------------------*/
// everybody needs a state variable, you may need others as well.
// type of state variable should match htat of enum in header file
static DisarmState_t CurrentState;
static uint16_t startTime;
static uint16_t endTime;

// with the introduction of Gen2, we need a module level Priority var as well
static uint8_t MyPriority;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitDisarmFSM

 Parameters
     uint8_t : the priorty of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, sets up the initial transition and does any
     other required initialization for this state machine
 Notes

****************************************************************************/

bool InitDisarmFSM ( uint8_t Priority )
{
  ES_Event ThisEvent;
	
	PortFunctionInit(); //initialize pins PA2-5, PB2-3, PD7, PF0 as GPIO
	initTapeSensors();  //initialize tape sensor
	initPhototransistor(); //initialize phototransisor
	initMotors();				//initialize motors
	initializeServos(); //initialize servo motors
	LEDShiftRegInit();  //initialize LED shift register
	LCDInit();					//initialize LCD display
	InitAdafruitAudioPortLines();	//initialize audio 
	initArmedLine();
	
  MyPriority = Priority;
  CurrentState = Armed;
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
	// if initialization failed
  {
      return false;
  }
}

/****************************************************************************
 Function
     PostDisarmFSM

 Parameters
     EF_Event ThisEvent , the event to post to the queue

 Returns
     boolean False if the Enqueue operation failed, True otherwise

 Description
     Posts an event to this state machine's queue
 Notes

****************************************************************************/
bool PostDisarmFSM( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunDisarmFSM

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   The state machine for the overall disarming process - state machine for
	 keypad, etc. are elsewhere
 Notes

****************************************************************************/
ES_Event RunDisarmFSM( ES_Event ThisEvent )
{
  ES_Event ReturnEvent;
  ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
	
	// initial state: all the LEDs are off.
	static char LEDs[8] = {OFF, OFF, OFF, OFF, OFF, OFF , OFF, OFF}; 
	static bool tower_rotate_direction = true;
	
  switch ( CurrentState )
  {
    case Armed :
			switch ( ThisEvent.EventType ) {
				case ES_INIT :
					// intializing timer to be at 2mS rate (for 60 second count down)
					ES_Timer_Init(ES_Timer_RATE_2mS);	
				
					printf("Arming...\r\n");
					// sets the armed line to +5V
					setArmed(); 
					printf(" Setting all tower LEDS off...\r\n");
					LEDShiftRegInit();
					// all LEDs are off when armed
					setLED (LEDs); 
				
					printf(" Generating random passwords...\r\n");
				  // generate random password for keyboard input
					randomizePasswords();
					printArmedMessage();
				
					printf(" Lowering flag...\r\n");
				  // lower the flag when armed
					lowerFlag();
					printf(" Locking the keys\r\n");
					// lock the key when armed
					lockKeys();
		
					printf(" Setting tower to 0...\r\n\r\n");
					// the tower doesn't lean when armed
					setTowerToZero();
					printf("STATE: Armed\r\n\r\n");
					
					rotateTowerLeft();
					ES_Timer_InitTimer(PANIC_TIMER, 350);
					// demonstrate panic and start timing
					ES_Timer_StartTimer(PANIC_TIMER);

					break;
        
				case THREE_HANDS_ON :
					
					printf("EVENT: Three hands detected.\r\n");
				
					printf(" Setting Tower Tier 1 LED on...\r\n");
					LEDs[Tier1] = ON;
					// light up the LED for bottom layer to show success in task 1
					setLED(LEDs);
				
					printf(" Begin printing LCD passcode...\r\n");
					resetLCDmessage();
					printLCDmessage();
					// sends out a message every 2 seconds (2mS timer rate)
					ES_Timer_InitTimer(MESSAGE_TIMER, 1000);
					ES_Timer_StartTimer(MESSAGE_TIMER);

					printf(" Starting 60s disarm timer...\r\n");
					//the timer can only go up to 32000
					ES_Timer_InitTimer(DISARM_TIMER, 30000);
					ES_Timer_StartTimer(DISARM_TIMER);
					//begins to run the timing motor
					unwindTimingMotor(); 
				
					// grabs current time to store for rewinding the timer motor
					startTime = ES_Timer_GetTime();

					ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					// play sound track 01
					ThisEvent.EventParam = 1; 					
					PostAdafruitAudioService(ThisEvent);
					
					printf(" Transitioning to Stage 1...\r\n\r\n");
					CurrentState = Stage1;
					printf("STATE: Stage1\r\n\r\n");
					break;
	
				case ES_TIMEOUT :
					// if panic timer expires
					if (ThisEvent.EventParam == PANIC_TIMER) {
						if (tower_rotate_direction) {
							tower_rotate_direction = false;
							rotateTowerRight();
						} else {
							tower_rotate_direction = true;
							rotateTowerLeft();
						}
						ES_Timer_InitTimer(PANIC_TIMER, 350);
						ES_Timer_StartTimer(PANIC_TIMER);
					}
					// if timing motor rewind timer expires
					if (ThisEvent.EventParam == REWIND_TIMER) {
						// stop timing motor when the ball reaches the top
						stopTimingMotor();
					}

				default:
					;
			}
		break;

    case Stage1 :
      switch ( ThisEvent.EventType )
      {
				case ES_TIMEOUT :
					if (ThisEvent.EventParam == DISARM_TIMER) {
						// if the disarm timer expires
						printf("EVENT: Time has run out!\r\n");
						printTimeUp();
						// gets the time when the game ended
						endTime = ES_Timer_GetTime(); 
						// calculate how much rewinding needs to be done
						ES_Timer_InitTimer(REWIND_TIMER, (endTime-startTime)/2);
						ES_Timer_StartTimer(REWIND_TIMER);
						// begins rewinding the clock motor
						rewindTimingMotor(); 
						
						ThisEvent.EventType = ES_INIT;
						PostDisarmFSM(ThisEvent);
						// go back to armed state
						CurrentState = Armed;
					}
					// if vibration timer expires
					if (ThisEvent.EventParam == VIBRATION_TIMER) {
						printf(" Turning off vibration pulse...\r\n\r\n");
						vibrationMotorOff();
					}
					// if message timer expires
					if (ThisEvent.EventParam == MESSAGE_TIMER) {
						printf("EVENT: Printing out the next message...\r\n");
						printLCDmessage();
						ES_Timer_InitTimer(MESSAGE_TIMER, 1000);
						ES_Timer_StartTimer(MESSAGE_TIMER);
					}
        break;
					
				// one or more hands have been released
				case THREE_HANDS_OFF :
					printf("EVENT: One or more hands have been released.\r\n");
					printf(" Clearing the LCD screen\r\n");
					printArmedMessage();
					printf(" Setting Tower Tier 1 LED off...\r\n");
					// task 1 is not completed 
					LEDs[Tier1] = OFF;
					// turn off the LED for bottom layer
				  setLED(LEDs);
				
					printf(" Transitioning to Stage1_Stagnated...\r\n\r\n");
					CurrentState = Stage1_Stagnated;
					printf("STATE: Stage1_Stagnated\r\n\r\n");
        break;
				
				// if the user entered the correct password
				case CORRECT_PASSWORD_ENTERED :
					printf("EVENT: The correct password has been entered.\r\n");
					printf(" Unlocking the keys\r\n");
					// unlock the key and move to task 3
					unlockKeys();
					printAuthorizedMessage();
				
					printf(" Setting Tower Tier 2 LED on...\r\n");
					LEDs[Tier2] = ON;
					// turn on LED on the second tier to show success
				  setLED(LEDs);
				
					printf(" Playing audio: Wahoo!...\r\n");
				  ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					// play sound track 01
					ThisEvent.EventParam = 1; 					
					PostAdafruitAudioService(ThisEvent);	
					printf(" Transitioning to Stage2...\r\n\r\n");
					// set the current state to state 2
					CurrentState = Stage2;
					printf("STATE: Stage2\r\n\r\n");
          break;
				
				// if the user entered incorrect password
				case INCORRECT_PASSWORD_ENTERED :
					printf("EVENT: The incorrect password has been entered.\r\n");
					printIncorrectMessage();
					ES_Timer_InitTimer(MESSAGE_TIMER, 1000);
					ES_Timer_StartTimer(MESSAGE_TIMER);
					printf(" Generating vibration pulse...\r\n");
          break;

        default :
            ; 
      }
      break;
		
		case Stage1_Stagnated :
      switch ( ThisEvent.EventType )
      {
        case ES_TIMEOUT :
					// if disarm timer expires
          if (ThisEvent.EventParam == DISARM_TIMER) {
						printf("EVENT: Time has run out!\r\n");
						printTimeUp();
						endTime = ES_Timer_GetTime();
						// calculate how much rewinding needs to be done
						ES_Timer_InitTimer(REWIND_TIMER, (endTime-startTime)/2);
						ES_Timer_StartTimer(REWIND_TIMER);
						// begins rewinding the timing motor
						rewindTimingMotor(); 
						
						ThisEvent.EventType = ES_INIT;
						PostDisarmFSM(ThisEvent);
						// go back to armed state
						CurrentState = Armed;
					}
          break;
				
				// if all three tape sensors are covered
				case THREE_HANDS_ON :
					printf("EVENT: Three hands detected.\r\n");
				
					printf(" Setting Tower Tier 1 LED on...\r\n");
					LEDs[Tier1] = ON;
					// light up the LED for bottom layer to show success in task 1
					setLED(LEDs);
				
					printf(" Begin printing LCD passcode...\r\n");
					resetLCDmessage();
					printLCDmessage();
					// print message from LCD
					ES_Timer_InitTimer(MESSAGE_TIMER, 1000);
					ES_Timer_StartTimer(MESSAGE_TIMER);
				
					// play feedback audio wahoo
					ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					// play sound track 01
					ThisEvent.EventParam = 1; 					
					PostAdafruitAudioService(ThisEvent);

					printf(" Transitioning to Stage1...\r\n\r\n");
					// set current stage to stage 1
					CurrentState = Stage1;
					printf("STATE: Stage1\r\n\r\n");
          break;

        default :
            ; 
      }
      break;
			
		case Stage2 :
      switch ( ThisEvent.EventType )
      {
        case ES_TIMEOUT :
					// if the disarm timer expires
          if (ThisEvent.EventParam == DISARM_TIMER) {
						printf("EVENT: Time has run out!\r\n");
						printTimeUp();
						// gets the time when the game ended
						endTime = ES_Timer_GetTime();
						// calculate how much rewinding needs to be done
						ES_Timer_InitTimer(REWIND_TIMER, (endTime-startTime)/2);
						ES_Timer_StartTimer(REWIND_TIMER);
						// begins rewinding the clock moto
						rewindTimingMotor(); r
						
						ThisEvent.EventType = ES_INIT;
						PostDisarmFSM(ThisEvent);
						CurrentState = Armed;
          }
					break;
					
				// if the key is inserted
				case KEY_INSERTED:
					printf("EVENT: Key has been inserted.\r\n");
				
					printf(" Setting Tower Tier 3 LED on...\r\n");
					LEDs[Tier3] = ON;
					// light up LED on tier 3 to show successful completion of task 3
					setLED(LEDs);
				
					printf(" Setting Dial LED on...\r\n");
					LEDs[pot] = ON;
					setLED(LEDs);
				
					printf(" Playing audio: Wahoo!...\r\n");
					ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					// play sound track 01
					ThisEvent.EventParam = 1; 					
					PostAdafruitAudioService(ThisEvent);
				
					printf("Initializing the pot value...\r\n");
					setPotZero();
					printf(" Transitioning to Stage3...\r\n\r\n");
					// set current state to state 3
					CurrentState = Stage3;
					printf("STATE: Stage3\r\n\r\n");
          break;

        default :
            ; 
      }
      break;			

		case Stage3 :
      switch ( ThisEvent.EventType )
      {
        case ES_TIMEOUT :
					// if the disarm timer expires
					if (ThisEvent.EventParam == DISARM_TIMER) {  
						printf("EVENT: Time has run out!\r\n");
						printTimeUp();
						endTime = ES_Timer_GetTime();
						// calculate how much rewinding needs to be done
						ES_Timer_InitTimer(REWIND_TIMER, (endTime-startTime)/2);
						ES_Timer_StartTimer(REWIND_TIMER);
						// begins rewinding the clock motor
						rewindTimingMotor(); 
						
						ThisEvent.EventType = ES_INIT;
						PostDisarmFSM(ThisEvent);
						// set current state to armed
						CurrentState = Armed;
					}
					if (ThisEvent.EventParam == FAST_LEDS) {  
						printf("EVENT: \r\n");
						ThisEvent.EventType = CORRECT_VALUE_DIALED;
						PostDisarmFSM(ThisEvent);
					}
          break;
				
				// if the pot is dialed to correct value 
				case CORRECT_VALUE_DIALED :
					printf("EVENT: The correct pot value has been dialed.\r\n");
					// Sets the armed line to 0V
					setUnarmed(); 
					printf(" Setting Tower Tier 4-6 LED on with delay...\r\n");
					
					ES_Timer_InitTimer(FAST_LEDS, 150);
					static int i = Tier4;
					if (i<=Tier6){
						printf("\n\r looping for LED i + %d\n\r", i);
						// turn on all the remaining LEDs one by one
						LEDs[i] = 0;
						setLED(LEDs);
						ES_Timer_StartTimer(FAST_LEDS);
						i++;
						break;
					}
					printf(" Raising the flag...\r\n");
					// raise flag to show hope and joy
					raiseFlag();
					printf(" Playing audio: victory song...\r\n");
					ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					// play sound track 03
					ThisEvent.EventParam = 3; 					
					PostAdafruitAudioService(ThisEvent);	
					printf(" Starting 30s post-disarm timer...\r\n");
					ES_Timer_InitTimer(POST_DISARM_TIMER, 30000);
					printf(" Raising ball and feather...\r\n");
					// stop falling ball
					stopTimingMotor();
					// get the time when the disarment ends and rearm DDM
					endTime = ES_Timer_GetTime();
					ES_Timer_InitTimer(REWIND_TIMER, (endTime-startTime)/2);
					ES_Timer_StartTimer(REWIND_TIMER);
					// begins rewinding the clock motor
					rewindTimingMotor(); 
					
					printf(" Transitioning to Stage4...\r\n\r\n");
					// set current stage to stage 4
					CurrentState = Stage4;
					printf("STATE: Stage4\r\n\r\n");
          break;

        default :
            ; 
      }
      break;	

		case Stage4 :
      switch ( ThisEvent.EventType )
      {
        case ES_TIMEOUT :
					// if timer expires after successful disarment
					if (ThisEvent.EventParam == POST_DISARM_TIMER) {
						printf("EVENT: Post-disarm timer expired.\r\n");
						ThisEvent.EventType = ES_INIT;
						PostDisarmFSM(ThisEvent);
						// set current state to armed
						CurrentState = Armed;
					}
					// if rewind timer expires
					if (ThisEvent.EventParam == REWIND_TIMER) {
						// stop timing motor rewinding
						stopTimingMotor();
					}
          break;

        default :
            ; 
      }
      break;	

    default :
      ;
  }
  return ReturnEvent;
}

/****************************************************************************
 Function
     QueryDisarmFSM

 Parameters
     None

 Returns
     DisarmState_t The current state of the Disarm state machine

 Description
     returns the current state of the Disarm state machine
 Notes

****************************************************************************/
DisarmState_t QueryDisarmFSM ( void )
{
   return(CurrentState);
}
