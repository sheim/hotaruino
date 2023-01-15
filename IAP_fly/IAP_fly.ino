/*
Author: Jonas Lang
Date: 17.08.2017

This is the main file that simulates the virtual firefly. 
Because of the three way switch used on the shield up to 8 different variations or mechanisms can be programmed.
Right now this code contains 3 different synchronization mechanisms. The models are the mathematical mode Renato E. Mirollo and Steven H. Strogtz
used in their paper "Synchronization of Pulse-Coupled Biological Oszillators" and the phase-advance synchronization model
and phase-delay synchronization model proposed from John Buck.
*/

#include "Firefly.h"

Firefly firefly; //creating an object of the class Firefly that includes all necessary variable and functions
//with the creation of that object all basic Timer and Output configurations are made

char switch_state = 0;
char old_switch_state = 0;

// double old_millis = 0;
// double current_millis = 0;
double delta_T = 0.0002;
double noise = 0.05;
const double max_noise = 10;

int iteration_counter = 1;

void setup()
{
    Serial.begin(9600);
	firefly.timerAndPinConfiguration(); //call the configuration function to set Timer1 and Timer2 and the Outputs/Inputs properly
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
    // old_millis = millis();
    Serial.println("Setup finished");
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

	firefly.potentiometerReadIn();
  iteration_counter = iteration_counter + 1;

	if(firefly.x <= firefly.x_reset)
	{
        // current_millis = millis();
        // delta_T = (current_millis - old_millis)/1000.;
        noise = double(random(-max_noise, max_noise));
        firefly.x += (firefly.frequency + noise)*delta_T;  // double(PHI_RAW)/firefly.phi_max;
	    // firefly.x = -a * exp(-(b / firefly.phi_max) * PHI_RAW) + c; //"x" gets it's new value according to x = f_phi
	} else {
        firefly.flashMirolloStrogatzModel();
	}

	firefly.flashReceiveCheck(); //checking if other fierflies flashed
	//the flash receive handler has to be coosen due to the synchronization mechanism that is activ

    firefly.receiveHandlerMirolloStrogatzModel();

	firefly.resetFlashReceive(); //reset all the received flashes that they arn't processed again.
    // old_millis = current_millis;
}