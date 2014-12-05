/****************************************************************************
 
  Header file for AdafruitAudio service 
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef AdafruitAudioService_H
#define AdafruitAudioService_H

#include "ES_Types.h"

/* NOTES:

	Tiva Connections 
	----------------------------------------
	Track No. 			Track					Tiva Port 
	----------------------------------------
	Track 01 'Yahoo'								PF2
	Track 02	'Woah-oah-oah'						PF3
	Track 03	'See You Next Time' 				PB3


	Make sure to edit 'ES_Configure.h':
		#define TIMER14_RESP_FUNC RunAdafruitAudioService
		#define AUDIO_TIMER 14
		
	
	To play an audio track, do the following:
					ES_Event ThisEvent;
					ThisEvent.EventType = PLAY_TRACK;
					ThisEvent.EventParam = 1; 					// play track 01
					PostAdafruitAudioService(ThisEvent);	
*/
			
// Public Function Prototypes

// Initializes Audio Firmware
bool InitAdafruitAudioService ( uint8_t Priority );

// Post event to Audio Service
bool PostAdafruitAudioService( ES_Event ThisEvent );

// Plays an audio track
ES_Event RunAdafruitAudioService( ES_Event ThisEvent );

#endif /* AdafruitAudioService_H */

