/****************************************************************************
 
  Header file for Disarm FSM
  based on the Gen2 Events and Services Framework

 ****************************************************************************/

#ifndef DisarmFSM_H
#define DisarmFSM_H

/* event definitions */
#include "ES_Configure.h" 
/* bool type for returns */
#include "ES_Types.h"     

// typedefs for the states
// State definitions for use with the query function
typedef enum { Armed, Stage1, Stage1_Stagnated, 
               Stage2, Stage3, Stage4 } DisarmState_t ;

// Public Function Prototypes
bool InitDisarmFSM ( uint8_t Priority );
bool PostDisarmFSM( ES_Event ThisEvent );
ES_Event RunDisarmFSM( ES_Event ThisEvent );
DisarmState_t QueryDisarmFSM ( void );
							 
#endif /* DisarmFSM_H */
							 
