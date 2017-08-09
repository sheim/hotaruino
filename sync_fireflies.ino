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

	firefly.phi_max = (unsigned int) ((F_CLK * firefly.constant_flash_interval) / PRESCALER);
	//calculation roughly phi_max

	PHI_RAW = random(firefly.phi_max); //after start up starting with a random phase to make it easier to start the fireflies out of synchronism

	firefly.random_value = random(1000);
	firefly.constant_flash_interval_offset = firefly.mapFloat(firefly.random_value, 0, 1023, -0.05, 0.05);
	//assign a random mapped value as flash offset
}

void loop()
{
	firefly.potentiometerReadIn(); //reading in the potentiometer states an calculation of the new phi_max

	if(firefly.x <= firefly.x_reset)
	{
	    firefly.x = -a * exp(-(b / firefly.phi_max) * PHI_RAW) + c; //"x" gets it's new value according to x = f_phi
	} else 
	{
	    //if x is bigger than x_reset than must a flash occur dependent on which mode is active
	}

	firefly.flashReceiveCheck(); //checking if other fierflies flashed
	//the flash receive handler has to be coosen due to the synchronization mechanism that is activ


	firefly.flashReceiveReset(); //reset all the received flashes that they arn't processed again.
}