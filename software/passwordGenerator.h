#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H

/****************************************************************************
 Module
   passwordGenerator.h

 Description
   This is a class to generate and store four random passwords of length 5 numeric digits.
	Usage:
	Call randomizePasswords(void) to generate the four random passwords.
	Call checkPassword(uint8_t *guess) to check if the guess matches the correct password.
	Call getPassword(1), getPassword(2), getPassword(3), getPassword(4) to get pointers to the
		four password arrays.
****************************************************************************/

// Generates a random password and stores it in the array
void generateRandomPassword(uint8_t* randomPassword);

// Randomly generates four passwords and assigns one of the four passwords to be the correct password
void randomizePasswords(void);

// Returns a pointer to the password array at the specified index
uint8_t* getPassword(uint8_t num);

// Prints the password to console
void printPassword(uint8_t* password);

// Checks to see if the password guess corresponds to the correct password.
bool checkPassword(uint8_t *guess);

#endif
