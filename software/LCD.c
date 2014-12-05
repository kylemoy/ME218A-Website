/****************************************************************************
 Module
   LCD.c

 Description
   This file contains the functions that will be needed to initialize the LCD
	 and to write to the LCD
	 
	 It sends all the information (like RS, D0, etc. pins), pulses the shift 
	 register to send to the LCD, then pulses the shift reigster again, 
	 in quick succession with E being set low and hi.
****************************************************************************/
//#define TEST

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
#include "LCD.h"
#include "passwordGenerator.h" // random passcode gets called
#include "driverlib/gpio.h"

#define ALL_BITS (0xff <<2)

/* definitions for array bit numbers
	 Basically E_HI will set the bit connected to E HI
	 and E_LO does the opposite, and so on for all other connections
*/
#define E_HI BIT1HI
#define E_LO BIT1LO
#define RW_HI BIT2HI 
#define RW_LO BIT2LO 
#define RS_HI BIT3HI //data
#define RS_LO BIT3LO
#define D7_HI BIT4HI
#define D7_LO BIT4LO
#define D6_HI BIT5HI
#define D6_LO BIT5LO
#define D5_HI BIT6HI
#define D5_LO BIT6LO
#define D4_HI BIT7HI
#define D4_LO BIT7LO

#define BIT(x) (1<<(x))

//the order goes: LCD = D4-D5-D6-D7-RS-RW-E-empty
//on the shift register: QB-QC-QD-QE-QF-QG-QH
//bit numbering goes:7-6-5-4-3-2-1-0
// so bit 0 is always empty
/****************************************************************************
  Change these values if ports C4 (data), C5 (SCK), C6 (RCK) are
	not being used for the shift register
****************************************************************************/
#define LCD_PORT_DEC SYSCTL_RCGCGPIO_R2 //port C
#define LCD_PORT GPIO_PORTC_BASE // port C base
#define LCD_DATA GPIO_PIN_4 // pin 4
#define LCD_SCK GPIO_PIN_5 // pin 5
#define LCD_RCK GPIO_PIN_6 // pin 6

/* Module level variables */
char LCD;
static uint8_t messageNumber = 0;

/* Prviate Function Prototypes */

// transfer bit to shift register
void SCKPulseLCD (void); 

// transfers 8 bits to the world
void RCKPulseLCD (void); 

// sends an 8 bit input to shift register - doesn't pulse LCD pin E
void sendToShiftReg (char Data);

// takes 8 bit input and pulses E to input into LCD, uses the sendToShiftReg function
void sendToLCD (char Data); 

// using the sendToLCD function, it writes a specific letter to the LCD
void LCDputchar (char letter);
/****************************************************************************
 Function
     SCKPulseLCD

 Parameters
     none

 Returns
     none

 Description
     pulses the pin connected the SCK by writing it HI, then back to LO again
 Notes

****************************************************************************/
void SCKPulseLCD (void) {
	// pulses SCK pin HI
	HWREG(LCD_PORT+(GPIO_O_DATA + ALL_BITS)) |= LCD_SCK; 
	
	// kill time
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
	
	// pulses SCK pin LO
	HWREG(LCD_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~LCD_SCK; 
}

/****************************************************************************
 Function
     RCKPulseLCD

 Parameters
     none

 Returns
     none

 Description
     pulses the pin connected the RCK by writing it HI, then back to LO again
 Notes

****************************************************************************/
void RCKPulseLCD (void) {
	// pulses RCK pin HI
	HWREG(LCD_PORT+(GPIO_O_DATA + ALL_BITS)) |= LCD_RCK; 
	
	// kill time
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
	
	// pulses RCK pin LO
	HWREG(LCD_PORT+(GPIO_O_DATA + ALL_BITS)) &= ~LCD_RCK; 	
}

/****************************************************************************
 Function
     sendToShiftReg

 Parameters
     char

 Returns
     none

 Description
     takes in a char and send it to the shift register so that it can write to the LCD
		 pins however, the E pin on the LCD itself is not pulsed, and therefore the LCD
		 would NOT have registered the changes yet.
 Notes
		 this function is a PRIVATE function
****************************************************************************/
void sendToShiftReg (char Data) {
	char send = 0;
	for (int i= 0; i <8; i ++) {
		send = (!! ((Data << i) & (0x80))); //takes left most bit, and returns 1 if it is 1, and 0 if it is 0
		if (send == 1){
			HWREG(LCD_PORT + GPIO_O_DATA + ALL_BITS) |= LCD_DATA;
		}
		else if (send == 0) {
			HWREG(LCD_PORT + GPIO_O_DATA + ALL_BITS) &= ~(LCD_DATA);
		}
		else { // for debugging only
			//puts ("entered neither 1 nor 0");
			//puts("\n\r sendToShiftReg send value not 0 or 1 \r\n");
		}
		// pulses SCK to send the single bit to shift register
		SCKPulseLCD();
	}
	
	/* leaving the loop means it has gone through all 8 bits, so
	   the shfit register is ready to write it to LCD
	*/
	RCKPulseLCD();
}

/****************************************************************************
 Function
     sendToLCD

 Parameters
     char

 Returns
     none

 Description
     takes in a char and send it to the LCD screen and pulses the E line so that
		 the LCD also sees this change has occured
 Notes
		 this function utilizes the sendToShiftReg function to keep it short and simple
		 it basically writes the parameter on to the pins on the LCD, pulses E HI,
		 writes the input parameter onto the pins again (to make sure that the data
		 isn't lost) and then writes E back LO. effectively, it is pulsing E so that
     the LCD sees what is being fed into it.

		 this function is also a PRIVATE function
****************************************************************************/
void sendToLCD (char Data) {
	sendToShiftReg(Data);
	Data |= E_HI;
	sendToShiftReg(Data);
	wait(1);
	Data &= E_LO;
}

/****************************************************************************
 Function
     LCDInit

 Parameters
     none

 Returns
     none

 Description
     goes through the LCD initialization sequence
 Notes

****************************************************************************/
void LCDInit (void) {
	/* LaunchPad init for ports C 4, 5 and 6 and 7*/
	HWREG(SYSCTL_RCGCGPIO) |= LCD_PORT_DEC;
	// kill time
	HWREG(SYSCTL_RCGCGPIO);
	//set C4-7 as outputs
	HWREG(LCD_PORT + GPIO_O_DEN)|= (LCD_DATA | LCD_SCK | LCD_RCK);
	HWREG(LCD_PORT + GPIO_O_DIR) |= (LCD_DATA | LCD_SCK | LCD_RCK);
	
	/* initialize timer */
	_HW_Timer_Init(ES_Timer_RATE_2mS);
	
	/*
	 LCD Initialization begins
	 Wait for more than 15 ms after VCC rises to 4.5 V
	 */
	wait (110);
	LCD = 0;
	
	// sends 000011 3 times, with at least 5mS in between each
	for (int i = 1; i < 4; i ++) {
		
		LCD |= (D5_HI | D4_HI);
		
		sendToLCD(LCD); //pulse to send 000011 both through the shift register and the LCD
		wait(3); //wait 6mS between each loop
	}
	
	/*
	 * Function set ( Set interface to be 4 bits long. )
	 * Interface is 8 bits in length.
	 */
	//sending 00010
	LCD &= D4_LO; //just changed D4 to LO
	sendToLCD(LCD);
	wait (1);
	
	/*
	 * Function set ( Interface is 4 bits long. Specify
	 * the number of display lines and character font. )
	 * The number of display lines and character font
	 * can not be changed after this point.
	 */
	//sending the same value as previous step
	sendToLCD(LCD);
	wait(1);
	
	/*sending 000NF** 
	 * N is on DB7, 0 for one line
	 * F is on DB6, 1 for bigger font
	 */
	LCD |= D7_HI;
	LCD &= D6_LO;
	sendToLCD(LCD);
	wait (1);
	
	/*
	 * Display off
	*/
	//sending 000000
	LCD = 0;
	sendToLCD(LCD);
	
	//sending 001000
	LCD |= D7_HI;
	sendToLCD(LCD);
	wait (1);
	
	/*
	 * Display clear
	*/
	//sending 0000
	LCD &= D7_LO;
	sendToLCD(LCD);
	
	//sending 0001
	LCD |= D4_HI;
	sendToLCD(LCD);
	wait (1);
	
	/* 
	 * Entry mode set
	 */
	//sending 0000
	LCD &= D4_LO;
	sendToLCD(LCD);

	/* sending 0000(I/D)S
	 * I/D is on DB5, 1 is to increment cursor
	 * S is on DB4, 1 is to shift screen 
	 */
	LCD |= D4_HI;
	LCD |= D5_HI;
	sendToLCD(LCD);
	wait (1);
	
	// send 0000, 1111 to turn back on
	LCD &= D4_LO;
	LCD &= D5_LO;
	sendToLCD(LCD);
	
	LCD |= (D4_HI | D5_HI | D6_HI | D7_HI);
	sendToLCD(LCD);
	wait (1);
	
	/* sets cursor to start at right side
	 */
	
	LCD &= D4_LO;
	LCD &= D5_LO;
	LCD &= D6_LO;
	LCD &= D7_LO;
	
	sendToLCD(LCD);
		
	LCD |= (D4_HI | D5_HI | D6_HI | D7_HI);
	
	sendToLCD(LCD);
	
	wait(1);	
}

/****************************************************************************
 Function
     clearLCD

 Parameters
     none

 Returns
     none

 Description
     clears the LCD screen
 Notes

****************************************************************************/
void clearLCD(void) {
	/*
	 * Display clear
	*/
	//sending 000000
	LCD = 0;
	sendToLCD(LCD);

	//sending 0001
	LCD |= D4_HI;
	sendToLCD(LCD);
	wait (1);
}

/****************************************************************************
 Function
     LCDputchar

 Parameters
     char

 Returns
     none

 Description
     writes a single letter that was at the input onto the LCD screne
 Notes
		 Just a reminder, LCD = D4-D5-D6-D7-RS-RW-E-empty
		 The ordering of the char being written was reversed, so we use an array
		 to flip the inputs accordingly
****************************************************************************/
void LCDputchar (char letter) {

	//writes each bit of the letter into a separate array entry
	uint8_t char_bits[8] = {0, 0, 0, 0, 0, 0 ,0 ,0};
	for (uint8_t i = 0; i < 8; i++) {
		if ((letter & 0x01) == 0x01)
			char_bits[i] = 1;
		letter = letter >> 1;
	}
	
	// Clears the data pin bits of LCD but keeps RS, RW, etc.
	LCD &= D4_LO;
	LCD &= D5_LO;
	LCD &= D6_LO;
	LCD &= D7_LO;
	LCD |= RS_HI; //data mode
	
	// Sets upper 4 bits (in reverse order) to be sent
	if (char_bits[4])
		LCD |= (0x80); // Set 1000000
	if (char_bits[5])
		LCD |= (0x40); // Set 0100000
	if (char_bits[6])
		LCD |= (0x20); // Set 0010000
	if (char_bits[7])
		LCD |= (0x10); // Set 0001000

	sendToLCD (LCD); //sends upper 4 bits
	
	//kills time
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
	
	// Clears the data pin bits of LCD but keeps RS, RW.
	LCD &= D4_LO;
	LCD &= D5_LO;
	LCD &= D6_LO;
	LCD &= D7_LO;
	// Sets lower 4 bits (in reverse order) to be sent
	if (char_bits[0])
		LCD |= (0x80); // Set 1000000
	if (char_bits[1])
		LCD |= (0x40); // Set 0100000
	if (char_bits[2])
		LCD |= (0x20); // Set 0010000
	if (char_bits[3])
		LCD |= (0x10); // Set 0001000
	
	sendToLCD (LCD); //sends upper 4 bits
	
	//kills time
	for (int i = 1; i < 50; i ++){
		HWREG(SYSCTL_RCGCGPIO);
	}
}

/****************************************************************************
 Function
     resetLCDmessage

 Parameters
     none

 Returns
     none

 Description
     sets a variable "messageNumber" such that the set of messages welcoming DrEd
		 back and showing him passcodes is starting at the beginning again
		 (specifically, with "Welcome DrEd")
 Notes
		 use this function so that the next time the three tape sensors are covered again
		 the message begins with "Welcome DrEd" instead of with a passcode or another mesasge
****************************************************************************/
void resetLCDmessage(void) {
	messageNumber = 1;
}

/****************************************************************************
 Function
     printLCDmessage

 Parameters
     none

 Returns
     none

 Description
     this function runs through the messages, printing out
		 "Welcome DrEd"
		 "Passcodes:"
		 - randomly generated passcode 1 -
		 - randomly generated passcode 2 -
		 - randomly generated passcode 3 -
		 - randomly generated passcode 4 -

		 The messages being printed will loop through the above 6 messages
 Notes
****************************************************************************/
void printLCDmessage(void) {
	// c gets used to translate the randomly generated passcodes into numbers to be shown on the LCD
	char c[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	
	switch (messageNumber) {
		case 1:
			clearLCD();
			LCDputchar('W');
			LCDputchar('e');
			LCDputchar('l');
			LCDputchar('c');
			LCDputchar('o');
			LCDputchar('m');
			LCDputchar('e');
			LCDputchar(' ');
			LCDputchar('D');
			LCDputchar('r');
			LCDputchar('E');
			LCDputchar('d');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			
			messageNumber = 2;
			break;
		
		case 2:
			clearLCD();
			LCDputchar('P');
			LCDputchar('a');
			LCDputchar('s');
			LCDputchar('s');
			LCDputchar('c');
			LCDputchar('o');
			LCDputchar('d');
			LCDputchar('e');
			LCDputchar('s');
			LCDputchar(':');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			messageNumber = 3;
			break;
		
		case 3:
			clearLCD();
			LCDputchar( c[getPassword(1)[0]]);
			LCDputchar( c[getPassword(1)[1]]);
			LCDputchar( c[getPassword(1)[2]]);
			LCDputchar( c[getPassword(1)[3]]);
			LCDputchar( c[getPassword(1)[4]]);
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			messageNumber = 4;
			break;
			
		case 4:
			clearLCD();
			LCDputchar( c[getPassword(2)[0]]);
			LCDputchar( c[getPassword(2)[1]]);
			LCDputchar( c[getPassword(2)[2]]);
			LCDputchar( c[getPassword(2)[3]]);
			LCDputchar( c[getPassword(2)[4]]);
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			messageNumber = 5;
			break;
		
		case 5:
			clearLCD();
			LCDputchar( c[getPassword(3)[0]]);
			LCDputchar( c[getPassword(3)[1]]);
			LCDputchar( c[getPassword(3)[2]]);
			LCDputchar( c[getPassword(3)[3]]);
			LCDputchar( c[getPassword(3)[4]]);
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			messageNumber = 6;
			break;
			
		case 6:
			clearLCD();
			LCDputchar( c[getPassword(4)[0]]);
			LCDputchar( c[getPassword(4)[1]]);
			LCDputchar( c[getPassword(4)[2]]);
			LCDputchar( c[getPassword(4)[3]]);
			LCDputchar( c[getPassword(4)[4]]);
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			messageNumber = 1;
			break;
	}
}

/****************************************************************************
 Function
     printAuthorizedMessage

 Parameters
     none

 Returns
     none

 Description
     this function prints out
		 "Autherized!"
     to the LCD screen
 Notes
****************************************************************************/
void printAuthorizedMessage(void) {
			clearLCD();
			LCDputchar('A');
			LCDputchar('u');
			LCDputchar('t');
			LCDputchar('h');
			LCDputchar('o');
			LCDputchar('r');
			LCDputchar('i');
			LCDputchar('z');
			LCDputchar('e');
			LCDputchar('d');
			LCDputchar('!');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
}

/****************************************************************************
 Function
     printTimeUp

 Parameters
     none

 Returns
     none

 Description
     this function prints out
		 "Your Time is Up!"
     to the LCD screen
 Notes
****************************************************************************/
void printTimeUp(void) {
			clearLCD();
			LCDputchar('Y');
			LCDputchar('o');
			LCDputchar('u');
			LCDputchar('r');
			LCDputchar(' ');
			LCDputchar('T');
			LCDputchar('i');
			LCDputchar('m');
			LCDputchar('e');
			LCDputchar(' ');
			LCDputchar('I');
			LCDputchar('s');
			LCDputchar(' ');
			LCDputchar('U');
			LCDputchar('p');
			LCDputchar('!');
			LCDputchar(' ');
			LCDputchar(' ');
	
}

/****************************************************************************
 Function
     printIncorrectMessage

 Parameters
     none

 Returns
     none

 Description
     this function prints out
		 "Incorrect password"
     to the LCD screen
 Notes
****************************************************************************/
void printIncorrectMessage(void) {
			clearLCD();
			LCDputchar('I');
			LCDputchar('n');
			LCDputchar('c');
			LCDputchar('o');
			LCDputchar('r');
			LCDputchar('r');
			LCDputchar('e');
			LCDputchar('c');
			LCDputchar('t');
			LCDputchar(' ');
			LCDputchar('P');
			LCDputchar('a');
			LCDputchar('s');
			LCDputchar('s');
			LCDputchar('w');
			LCDputchar('o');
			LCDputchar('r');
			LCDputchar('d');
	
}

/****************************************************************************
 Function
     printArmedMessage

 Parameters
     none

 Returns
     none

 Description
     this function prints out
		 "Armed"
     to the LCD screen
 Notes
****************************************************************************/
void printArmedMessage(void) {
			clearLCD();
			LCDputchar('A');
			LCDputchar('r');
			LCDputchar('m');
			LCDputchar('e');
			LCDputchar('d');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
			LCDputchar(' ');
}

/****************************************************************************
 Test Harness for LCDs module

 Description
     intializes the relevant Tiva port and prints out some characters
 Notes
****************************************************************************/
#ifdef TEST 
#include "termio.h" 
int main(void) 
{ 
	TERMIO_Init(); 
	puts("\n\r In test harness of LCD\r\n");
	
	LCDInit();

	LCDputchar('W');
	LCDputchar('e');
	LCDputchar('l');
	LCDputchar('c');
	LCDputchar('o');
	LCDputchar('m');
	LCDputchar('e');
	LCDputchar(' ');
	LCDputchar('D');
	LCDputchar('r');
	return 0;
}
#endif
