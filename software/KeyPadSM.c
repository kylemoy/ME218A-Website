/****************************************************************************
 Module
   KeyPadSM.c

 Description
   This file contains the state machine for the key pad. It takes in the "key",
   a 4 digit number, and expects the user to enter the four digits in
   order and gives feedback as to whether it is correct or not.
   
   It will expect the user to stat from the first digit if he/she enter the wrong
   number at any point. 
****************************************************************************/

#include <stdio.h>
#include "termio.h"

//#define TEST

typedef enum { Expect_First, Expect_Second, Expect_Third, Expect_Fourth} KeyState_t ;
static KeyState_t CurrentState = Expect_First; //initializing to expect first number

// I can change this into an array, where we have an array like Code[4] = {1, 2, 3, 4} 
//and we check if it matches up or not too
void KeyPadSM (int first, int second, int third, int fourth) {
	int number; // the number that is read from the termio
	
	switch ( CurrentState ) {
		case Expect_First : //case where the keyboard expect the first value
			puts("\r\n Type in first key\r\n");
			scanf("%d", &number);
		
			if ( number == first) {//Code[0] ){
				printf("\r\n You entered %d correctly\r\n", number);
				CurrentState = Expect_Second;
			}
			else {
				printf("\r\n You entered %d incorrectly - start again\r\n", number);
				CurrentState = Expect_First;
			}
		break;

		case Expect_Second : //expecting the second value
			puts("\r\n Type in second key\r\n");
			scanf("%d", &number);
		
			if (  number == second) { //Code[1] ){
				printf("\r\n You entered %d correctly \r\n", number);
				CurrentState = Expect_Third;
			}
			else {
				puts("\r\n Second number incorrect - restart \r\n");
				CurrentState = Expect_First;
			}
		break;
			
		case Expect_Third : //expecting the third value
			puts("\r\n Type in third key\r\n");
			scanf("%d", &number);
		
			if (  number == third) { //Code[2] ){
				printf("\r\n You entered %d correctly \r\n", number);
				CurrentState = Expect_Fourth;
			}
			else {
				puts("\r\n Third number incorrect - restart \r\n");
				CurrentState = Expect_First;
			}
		break;
			
		case Expect_Fourth : //expecting the fourth value
			puts("\r\n Type in fourth key\r\n");
			scanf("%d", &number);
		
			if (  number == fourth) { //Code[3] ){
				printf("\r\n You entered %d correctly - KEY TURNS\r\n", number);
				/* here the function to turn the key would be called
				and we move on
				*/
				CurrentState = Expect_First;
			}
			else {
				puts("\r\n Fourth number incorrect - restart \r\n");
				CurrentState = Expect_First;
			}
		break;
	}
}

#ifdef TEST 
/* test Harness for testing this module */ 

int main(void) 
{ 
	TERMIO_Init(); 
	int coding1;
	int coding2;
	int coding3;
	int coding4;
	puts("\r\n Type in first number of code\r\n");
	scanf("%d", &coding1);
	puts("\r\n Type in second number of code\r\n");
	scanf("%d", &coding2);
	puts("\r\n Type in third number of code\r\n");
	scanf("%d", &coding3);
	puts("\r\n Type in fourth number of code\r\n");
	scanf("%d", &coding4);
	while(1){
		KeyPadSM (coding1, coding2, coding3, coding4);
	}
}
#endif
