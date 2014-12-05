//#define TEST
/****************************************************************************
 Module
   passwordGenerator.c

 Description
   This is a class to generate and store four random passwords of length 5 numeric digits.
	Usage:
	Call randomizePasswords(void) to generate the four random passwords.
	Call checkPassword(uint8_t *guess) to check if the guess matches the correct password.
	Call getPassword(1), getPassword(2), getPassword(3), getPassword(4) to get pointers to the
		four password arrays.
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "passwordGenerator.h"
#include "ADCSWTrigger.h"

// Maximum password length
#define MAX_PASS_LENGTH 5
// Number of passwords to store
#define NUM_PASSWORDS 4

// Variable to store the correct password index
static uint8_t correctPassword = 0;
// Variables to store the passwords
static uint8_t password1[MAX_PASS_LENGTH];
static uint8_t password2[MAX_PASS_LENGTH];
static uint8_t password3[MAX_PASS_LENGTH];
static uint8_t password4[MAX_PASS_LENGTH];

/****************************************************************************
 Function
     generateRandomPassword
 Parameters
     uint8_t* randomPassword : a pointer to the password array
 Returns
     void
 Description
     Generates a random password and stores it in the array
 Notes
****************************************************************************/
void generateRandomPassword(uint8_t* randomPassword) {
	// gets MAX_PASS_LENGTH (5) random numbers  
	for (uint8_t i = 0; i < MAX_PASS_LENGTH; i++) {
		randomPassword[i] = rand() % 10;
	}
}

/****************************************************************************
 Function
     randomizePasswords
 Parameters
     void
 Returns
     void
 Description
     Randomly generates four passwords and assigns one of the four passwords 
     to be the correct password
 Notes
****************************************************************************/
void randomizePasswords(void) {
	srand(ADC0_InSeq3());
	generateRandomPassword(password1);
	generateRandomPassword(password2);
	generateRandomPassword(password3);
	generateRandomPassword(password4);
	// randomly selects which of the passwords is the correct one
	correctPassword = rand() % NUM_PASSWORDS + 1;
}

/****************************************************************************
 Function
     getPassword
 Parameters
     uint8_t num : password index
 Returns
     uint8_t* : pointer to the password array at the specified index
 Description
     Returns a pointer to the password array at the specified index
 Notes
****************************************************************************/
uint8_t* getPassword(uint8_t num) {
	switch (num) {
		case 1:
			return password1;
		case 2:
			return password2;
		case 3:
			return password3;
		case 4:
			return password4;
	}
	return NULL;
}

/****************************************************************************
 Function
     printPassword
 Parameters
     uint8_t* password : pointer to the password array
 Returns
     void
 Description
     Prints the password to console
 Notes
****************************************************************************/
void printPassword(uint8_t* password) {
	for (uint8_t i = 0; i < MAX_PASS_LENGTH; i++) {
		printf("%d ", password[i]);
	}
	printf("\r\n");
}

/****************************************************************************
 Function
     printPassword
 Parameters
     uint8_t* password : pointer to the password array
 Returns
     void
 Description
     Prints the password to console
 Notes
****************************************************************************/
// this function gets used to compare a password to a password being entered
bool compareIntArrays(uint8_t *array1, uint8_t *array2) {
	if (sizeof(array1) != sizeof(array2))
		return false;
	else {
		for (uint8_t i = 0; i < sizeof(array1); i++) {
			if (array1[i] != array2[i])
				return false;
		}
		return true;
	}
}

/****************************************************************************
 Function
     checkPassword
 Parameters
     uint8_t *guess : pointer to the password guess array
 Returns
     void
 Description
     Checks to see if the password guess corresponds to the correct password.
 Notes
****************************************************************************/
// the function that actually checks to make sure the password is matching
bool checkPassword(uint8_t *guess) {
	switch (correctPassword) {
		case 1:
			if (compareIntArrays(password1, guess))
				return true;
			break;
		case 2:
			if (compareIntArrays(password2, guess))
				return true;
			break;
		case 3:
			if (compareIntArrays(password3, guess))
				return true;
			break;
		case 4:
			if (compareIntArrays(password4, guess))
				return true;
			break;
	}
	return false;
}

#ifdef TEST 
/* Test Harness for the Random Password Generator */
#include "termio.h" 
int main(void) 
{ 
	TERMIO_Init(); 
	printf("\n\r In test harness of passwords\r\n");
	/* Intializes random number generator */
  srand(rand());
	randomizePasswords();
	printPassword(getPassword(1));
	printPassword(getPassword(2));
	printPassword(getPassword(3));
	printPassword(getPassword(4));
	printf("\r\nThe correct password is %d\r\n\r\n", correctPassword);
	if (checkPassword(getPassword(1)))
		printf("Password1 is correct\r\n");
	else
		printf("Password1 is incorrect\r\n");
	if (checkPassword(getPassword(2)))
		printf("Password2 is correct\r\n");
	else
		printf("Password2 is incorrect\r\n");
	if (checkPassword(getPassword(3)))
		printf("Password3 is correct\r\n");
	else
		printf("Password3 is incorrect\r\n");
	if (checkPassword(getPassword(4)))
		printf("Password4 is correct\r\n");
	else
		printf("Password4 is incorrect\r\n");
		
	// Int to Character Stuff
	char c[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	for (int i = 0; i < 5; i++) {
		printf("%c\r\n", c[getPassword(1)[i]]);
	}
}
#endif
