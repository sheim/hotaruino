/*
Author: Jonas Lang
Date: 17.08.2017

The Firefly.h file contains all the proto functions for the virtual firefly. This file have to be added later to access the functions in Firefly.cpp
*/

#ifndef Firefly_h
#define Firefly_h

#include "Arduino.h"

#define a 1.011
#define b 4.499
#define c 1.011

#define F_CLK 16000000 //clock frequency
#define PRESCALER 1024 //the PRESCALER for Timer1 (it's set in the TCCR1B register)
#define PHI_RAW TCNT1 //the value of the Timer1 is used as the phase variable phi
#define VISIBLE_FLASH_LENGTH 200 //in ms --> flashing time ot the visible LED
#define IR_FLASH_LENGTH 20 //in ms --> flashing time of the IR-LED
#define NEURAL_DELAY 100 //the flash signal needs that time to get to the abdomen

class Firefly
{
	public:
		Firefly();

		double x = 0; //state x --> x=f(phiRaw/phi_max)
    	char x_reset = 1; //the flash occurs after "x" exceeds "x_reset"
    	double epsilon = 0.1; //coupling strength, the amount "x" gets lifted up if a flash is received
    	double constant_flash_interval = 1.5; //in s --> the firefly runs with this frequency if the mapped Potentiometer is 0
    	double flash_interval = 0; //in s --> overall interval between flashes
    	unsigned int phi_max = 0; //compare value for PHI_RAW, the maximum value Timer1 counts to

    	char flash_receive_A = 1; //holds the value for PB1 --> since it's a digital input the value can be either 0 or 1
    	char flash_receive_B = 1; //holds the value for PB2
    	char flash_receive_C = 1; //holds the value for PB4
    	char flash_receive_D = 1; //holds the value for PB5

    	int random_value = 0; //holds later a random value for mapping noise to a "x" and the "flash_interval"
    	double constant_flash_interval_offset = 0; //random offset that not all fireflies flash with the same frequency

    	void timerAndPinConfiguration();
    	void millisecondDelay(int timer_delay);
    	void potentiometerReadIn();
    	void flashReceiveCheck();
    	void resetFlashReceive();

    	void flashMirolloStrogatzModel();
    	void flashBuckPhaseAdvanceAndDelay();
    	
    	void receiveHandlerMirolloStrogatzModel();
    	void receiveHandlerBuckPhaseAdvance();
    	void receiveHandlerBuckPhaseDelay();

    	float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
		
};

#endif