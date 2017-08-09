#include "Firefly.h"

Firefly firefly(); //creating an object of the class Firefly that includes all necessary variable and functions
//with the creation of that object all basic Timer and Output configurations are made

char switch_state = 0;
char old_switch_state = 0;

void setup()
{
	/*
	The three pin Dil switch is attached to the Pins PD2 (Digital Pin 2), PD3 (Digital Pin 3) and PD4 (Digital Pin 4) 
	to keep the Pins for the serial communication free.
	The pins have to be somehow be pulled up or pulled down to get a defined digital state. Therefor the internal pull-ups are used to save some place.
	They are activated for the whole PORD D because that makes it much easier later to ask for the Switch state.
	*/

	DDRD &= ~(1 << PD2); //declaring PD2 as an Input
	DDRD &= ~(1 << PD3); //declaring PD3 as an Input
	DDRD &= ~(1 << PD4); //declaring PD4 as an Input
	PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4); //activate the internal pull-ups

	firefly.phi_max = (unsigned int) ((F_CLK * firefly.constant_flash_interval) / PRESCALER);
	//calculation roughly phi_max

	PHI_RAW = random(firefly.phi_max); //after start up starting with a random phase to make it easier to start the fireflies out of synchronism

	firefly.random_value = random(1000);
	firefly.constant_flash_interval_offset = firefly.mapFloat(firefly.random_value, 0, 1023, -0.05, 0.05);
	//assign a random mapped value as flash offset
}

void loop()
{
	switch_state = (PIND & (1 << PD2)) + (PIND & (1 << PD3)) + (PIND & (1 << PD4)); 
	//as state of the switch is represented through the sum of all single states

	//if a switch is changed it is recognized in the beginnig and after that the whole process can go on with the new activated mechanism
	if (old_switch_state != switch_state)
	{
		for (int i = 0; i < 6; i++)
		{
			PORTB ^= (1 << PB0);
			firefly.millisecondDelay(100);
		}//let the visible led flicker as recognition if the switch was used
		old_switch_state = switch_state;
	}

	firefly.potentiometerReadIn(); //reading in the potentiometer states an calculation of the new phi_max

	if(firefly.x <= firefly.x_reset)
	{
	    firefly.x = -a * exp(-(b / firefly.phi_max) * PHI_RAW) + c; //"x" gets it's new value according to x = f_phi
	} else 
	{
	    /*
	    if x is bigger than x_reset than must a flash occur dependent on which mode is activ. The swich states are counted through in binary.
	    So if no switch is on the PIND-register lokks like that: PIND = 0b00011100 (that is state "0" and corresponds to the model from Mirollo and Strogatz)
	    If the first switch is on it looks like this: PIND = 0b00011000 (that is state "1" and that corresponds to the phase advance mechanism)
	    And so on: PIND = 0b00010100 (state "2" correspond to the phase delay mechanism)
	    PIND = 0b00010000 (state "3")
	    PIND = 0b00001100 (state "4")
	    PIND = 0b00001000 (state "5")
	    PIND = 0b00000100 (state "6")
	    PIND = 0b00000000 (state "7")

	    So 5 more mechanisms can be added to that model.
	    */

	    if(PIND & (1 << PD2) && PIND & (1 << PD3))
	    {
    		firefly.flashMirolloStrogatzModel();
  		}
  		else if(PIND & (1 << PD3))
  		{
    		firefly.flashBuckPhaseAdvanceAndDelay();
  		}
  		else if(PIND & (1 << PD2))
  		{
    		firefly.flashBuckPhaseAdvanceAndDelay();
  		}
	}

	firefly.flashReceiveCheck(); //checking if other fierflies flashed
	//the flash receive handler has to be coosen due to the synchronization mechanism that is activ

	if(PIND & (1 << PD2) && PIND & (1 << PD3))
	{
   		firefly.receiveHandlerMirolloStrogatzModel();
  	}
  	else if(PIND & (1 << PD3))
  	{
    	firefly.receiveHandlerBuckPhaseAdvance();
  	}
  	else if(PIND & (1 << PD2))
  	{
    	firefly.receiveHandlerBuckPhaseDelay();
  	}

	firefly.flashReceiveReset(); //reset all the received flashes that they arn't processed again.
}