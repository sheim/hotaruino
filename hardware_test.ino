#include "Firefly.h"

Firefly firefly; //creating a new object of class Firefly

double old_flash_interval = 0; //all those variables help to recognize changes in the hardware
double old_epsilon = 0;

double noise_epsilon = 0;
double noise_flash_interval = 0;

char switch_state = 0;
char old_switch_state = 0;

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
}