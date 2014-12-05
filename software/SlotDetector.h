#ifndef SLOTDETECTOR_H
#define SLOTDETECTOR_H

/****************************************************************************
 Module
   SlotDetector.h

 Description
   This file contains the function that will read the phototransistor output
   to test for the presence of a key in the slot.
****************************************************************************/

// Initializes the port pins for the slot detector
void initPhototransistor(void);

// Checks if the key is in the slot
bool isKeyInSlot(void);

#endif
