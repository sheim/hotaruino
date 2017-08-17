/*
Author: Jonas Lang
Date: 17.08.2017

This file can be used to test the hardware of the firefly if the soldering process is done.
If the Arduino IDE is available the Serial monitor can be opend to get the the information of the hardware. If the Arduino IDE is not available
just load the program and watch the visible LED. It blinks if the potentiometer or the switches are changed or of a flash is received by the receiver.

To test the hardware just switch the switches or turn the potentiometer and observe the LED or the Serial monitor.
The IR-receivers can be testet with any remote control. 

Don't make the changes to fast!!!
*/

#include "Firefly.h"

Firefly firefly; //creating a new object of class Firefly

double old_flash_interval = 0; //all those variables help to recognize changes in the hardware
double old_epsilon = 0;

double noise_epsilon = 0;
double noise_flash_interval = 0;

char switch_state = 0;
char old_switch_state = 0;

void visibleInsuranceThatHardwareWork()
{
	for (int i = 0; i < 6; i++)
	{
		PORTB ^= (1 << PB0); //toggle the visible LED
		firefly.misslisecondDelay(100);
	}
}

void setup()
{
	Serial.begin(9600); //starting the serial communication
	Serial.println("Start Hardware test");

	firefly.timerAndPinConfiguration(); //initialize the Pins that have to be tested

	/*
	Initialize the Pins where the Dil switches are attached and enable their pull-ups
	*/
	DDRD &= ~(1 << PD2); //declaring PD2 as an Input
  	DDRD &= ~(1 << PD3); //declaring PD3 as an Input
  	DDRD &= ~(1 << PD4); //declaring PD4 as an Input
  	PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4); //activate the internal pull-ups

  	firefly.potentiometerReadIn(); //read in the potentiometer to get a start value
  
  	old_flash_interval = firefly.flash_interval; //setting the start value for all variables
 	old_epsilon = firefly.epsilon;

  	switch_state = (PIND & (1 << PD2)) + (PIND & (1 << PD3)) + (PIND & (1 << PD4));
  	old_switch_state = switch_state;
}

void loop()
{
	firefly.potentiometerReadIn(); //updating the potentiometer values

  	if(old_epsilon > (firefly.epsilon + 0.05) || old_epsilon < (firefly.epsilon - 0.05))
  	{//checking if the potentiometer for epsilon changed about the value 0.05 (0.05 is chosen to eliminate noise)
  	 	old_epsilon = firefly.epsilon;
  		 Serial.println("Adjusted epsilon with the potentiometer."); 
  	 	Serial.println();
  	 	visibleInsuranceThatHardwareWork();
  	 	//show that the change is recognized via Serial comunication and the visible LED
  	}
  	if(old_flash_interval > (firefly.flash_interval + 0.5) || old_flash_interval < (firefly.flash_interval - 0.5))
  	{//checking if the potentiometer for flash interval changed about the value 0.5 (0.5 is chosen to eliminate noise)
  	 	old_flash_interval = firefly.flash_interval;
  	 	Serial.println("Adjusted the flash interval with the potentiometer.");
  	 	Serial.println();
  	 	visibleInsuranceThatHardwareWork();
  	 	//show that the change is recognized
  	}

  	//checking if a IR-Flash is received from the IR-Receivers
  	firefly.flashReceiveCheck();

  	if(firefly.flash_receive_A == 0)
  	{
  		Serial.println("Receiver at PB1 (Digital Pin 9) recognized a flash.");
 	  	Serial.println();
 	  	visibleInsuranceThatHardwareWork();
 	  	//show that the Receiver at Pin PB1 received a flash
 	}
 	if(firefly.flash_receive_B == 0)
 	{
 		Serial.println("Receiver at PB2 (Digital Pin 10) recognized a flash.");
 		Serial.println();
 		visibleInsuranceThatHardwareWork();
 		//show that the Receiver at Pin PB2 received a flash
	}
 	if(firefly.flash_receive_C == 0)
 	{
		Serial.println("Receiver at PB4 (Digital Pin 12) recognized a flash.");
 		Serial.println();
		visibleInsuranceThatHardwareWork();
		//show that the Receiver at Pin PB4 received a flash
	}
 	if(firefly.flash_receive_D == 0)
 	{
 		Serial.println("Receiver at PB5 (Digital Pin 13) recognized a flash.");
  		Serial.println();
  		visibleInsuranceThatHardwareWork();
  		//show that the Receiver at Pin PB5 received a flash
 	}

 	firefly.resetFlashReceive();
 	//reset the values of the receiver to prevent that they're processed again

 	switch_state = (PIND & (1 << PD2)) + (PIND & (1 << PD3)) + (PIND & (1 << PD4));
 	if (switch_state != old_switch_state)
 	{
 		old_switch_state = switch_state;
 		if(PIND & (1 << PD2)) //check the state of the Switch
 		{
 			Serial.println("Switch 1 is off.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}else
 		{
 			Serial.println("Switch 1 is on.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}//show that the state of Pin PD2 changed

 		if(PIND & (1 << PD3)) //check the state of the Switch
 		{
 			Serial.println("Switch 2 is off.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}else
 		{
 			Serial.println("Switch 2 is on.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}//show that the state of Pin PD3 changed

 		if(PIND & (1 << PD4)) //check the state of the Switch
 		{
 			Serial.println("Switch 3 is off.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}else
 		{
 			Serial.println("Switch 3 is on.");
      		Serial.println();
      		visibleInsuranceThatHardwareWork();
 		}//show that the state of Pin PD4 changed
 	}
}