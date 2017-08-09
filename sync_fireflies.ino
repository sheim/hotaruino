#include "Firefly.h"

Firefly firefly(); //creating an object of the class Firefly that includes all necessary variable and functions
//with the creation of that object all basic Timer and Output configurations are made

void setup()
{
	/*
	The three pin Dil switch is attached to the Pins PD2 (Digital Pin 2), PD3 (Digital Pin 3) and PD4 (Digital Pin 4) 
	to keep the Pins for the serial communication free.
	The pins have to be somehow be pulled up or pulled down to get a defined digital state. Therefor the internal pull-ups are used to save some place.
	They are activated for the whole PORD D because that makes it much easier later to ask for the Switch state.
	*/

	DDRD &= ~0xFF; //declaring PORT D as an Output
	PORTD |= 0xFF; //activate the internal pull-ups
}

void loop()
{
	
}