#include "Arduino.h"
#include "Firefly.h"

/*
If an Object of Firefly is made all the important configurations for the pacemaker and the outputs are set in the Constructor
But that is just a part of the intitialization. In the setup has to be calculated the "constant_flash_interval_offset"
and roughly for savty reasons "phi_max". And the Pins where the switch will be connected have to be also configured
*/

Firefly::Firefly()
{
	 /*
  Initialize Timer1:
  Timer1 is used to keep track of the flashing rythm if the firefly is not entrained/triggered for another one.
  Therefor the Timer1 is driven in the "Normal-Mode". That means that it only counts up. The counting value is in the register TCNT1 or "phiRaw".
  That register can be compared to the variable "phi_max" to figure out when to flash. The value of "phi_max" can be calculated like that:

  phi_max = (F_CLK * flashInterval) / prescaler

  These values are all defined above!
  */

  TCCR1A = 0; //for Normal-Mode is no Bit in the TCCR1A register necessary
  TCCR1B = 0b00000101; //the last three bits are to set the prescaler for Timer1. For a prescaler of 1024 (the Timer1 increments it's value at every 1024th clock cycle)
  TIMSK1 = 0; //to ensure that no interrupt request is set

  /*
  Initialize Timer2:
  Timer2 is used to modulate the required frequency (~38kHz) for the IR-Receiver. Timer2 is driven in CTC-Mode (Clear Timer on Compare-Match-Mode)
  with the additional setting that if the compare-match is reached the pin OC2A (PB3, Digital Pin 11 --> the IR-LED have to be connected to that pin) is toggled.
  The compare value is stored in the register OCR2A.
  To calculate the value that have to be stored in OCR2A we have to know witch prescaler N we are using and we need to know the clock frequency f_clk
  and the frequency f of our signal:

  OCR2A = (f_clk/(2*N*f))-1

  */

  TCCR2A = 0b01000010; //the first "1" chooses the Mode to toggle pin OC2A on compare match. The second "1" is for choosing CTC-Mode
  TCCR2B = 0; //right no the counter is disabled because there is no prescaler set. That is only necessary if a flash is required
  TIMSK2 = 0; //to ensure that no interrupt is set
  OCR2A = 210; //initialize the OCR2A register with the calculated value
  TCNT2 = 0; //initialize the value of the Timer2 so that it start counting from bottom

  /*
  Initialize required pins:
  The IR-LED have to be connected to PB3 (OC2A, Digital Pin 11) like it was mentioned above. The Pin has to be declared as an output. If it's not it won't toggle.

  The visible LED is connected to the PB0 (Digital Pin 8). It has to be an Output.
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  The IR-Receiver has an internal filter circuit that filters out any frequency than 38kHz (TSOP38238). It also filters out IR light from DC light sources.
  Thats the reason why the IR-LED has to flash with this certain frequency. That means that the Output of the receiver (witch is internally pulled high) is
  not influenced from ambient light. It in not necessary to take a sample. Thats why the output of the IR-Receiver is connected to PB1 (Digital Pin 9).
  This Pin has to be an Input. And for later: we have to wait until the Pin is Low!!!
  */

  DDRB |= (1 << DDB0) | (1 << DDB3); //declaring PB0 (Digital Pin 8) and PB3 (OC2A, Digital Pin 11) as Output
  DDRB &= ~(1 << DDB1); //declaring PB1 (Digital Pin 9) as an Input
  DDRB &= ~(1 << DDB2); //declaring PB2 (Digital Pin 10) as an Input
  DDRB &= ~(1 << DDB4); //declaring PB4 (Digital Pin 12) as an Input
  DDRB &= ~(1 << DDB5); //declaring PB5 (Digital Pin 13) as an Input

  //if one Pin isn't used just pull it up to Vss or remove the variable that holds the value for the unused pin from the argument of the if-sentence below!

  /*
  The variables "epsilon" and "flash_interval" should be adjustable at first on the breadboard with some potentiometers. Those are hooked up to the Pins
  PC0 (Analog Pin A0) and PC1 (Analog Pin A1). Hence they have to be inputs
  */

  DDRC &= ~(1 << DDC0); //declaring PC0 (Analog Pin A0) as an Input
  DDRC &= ~(1 << DDC1); //declaring PC1 (Analog Pin A1) as an Input
}

/*
The "millisecondDelay"-function is introduced to ensure that there is no change in the settings of Timer1 and Timer2!
*/

void Firefly::millisecondDelay(int time_delay)
{
	int start_time = millis();
	int current_time = millis();
	//staying in the loop until "time_delay" is over
	while((current_time - start_time) >= time_delay)
	{
		current_time = millis();
	}
}

/*
the "potetiometerReadIn"-function reads the values of the potentiometer and map the value to the right range vor the variable.
It also calculates directly the new "phi_max" value!
*/

void Firefly::potentiometerReadIn()
{
	epsilon = mapFloat(analogRead(A0), 0, 1023, 0.01, 0.2); //reading in the analog value on pin PC0/A0 and map that value to a value between 0.01 and 0.2
  	flash_interval = constant_flash_interval + mapFloat(analogRead(A1), 0, 1023, -1, 1.5) + constant_flash_interval_offset;
  	//reading in the new "flash_interval" from PC1/A1, map it and add it to the constant interval then add the previous calculated (firefly specific) flash interval offset
  	phi_max = (unsigned int)((F_CLK * flash_interval) / PRESCALER); // calculate the new "phi_max" out of the new "flash_interval"
}

/*
The "flashReceiveCheck"-function checks if a flash is received and saves that information to the flash_receive_x variable.
This information is important for the flash handler!!!
*/

void Firefly::flashReceiveCheck()
{
	flash_receive_A = PINB & (1 << PB1); //saving the state if a flash is received or not for pin PB1.
  	flash_receive_B = PINB & (1 << PB2); //saving the state if a flash is received or not for pin PB2.
  	flash_receive_C = PINB & (1 << PB4); //saving the state if a flash is received or not for pin PB4.
 	flash_receive_D = PINB & (1 << PB5); //saving the state if a flash is received or not for pin PB5.
}

/*
After a flash is received and processed the variable that contains the flash information have to be reseted so that on the next walkthrough of the
loop-function the same process is not processed again!!!
*/

void Firefly::flashReceiveReset()
{
	flash_receive_A = HIGH; //ensure that the for the next round the flash receive is HIGH
  	flash_receive_B = HIGH;
  	flash_receive_C = HIGH;
  	flash_receive_D = HIGH;
}

/*
The "flashMirolloStrogatzModel"-function makes the flash that occurs in the mathematical model used from Renato E. Mirollo and Steven H. Strogatz
in their Paper "Synchronization of Pulse-Coupled Biological Oszillators".
*/

void Firefly::flashMirolloStrogtzModel()
{
	int start_flash = millis();
  	int current_time = millis();

  	TCCR2B = 0b00000001; //setting the prescaler to "1"
  	PORTB |= (1 << PB0); //light-up the visible LED

  	//stretch the time in the loop to "flashLength"
  	while((current_time - start_flash) <= VISIBLE_FLASH_LENGTH)
  	{
  		/*
  		The IR-LED flashes only for 20ms because the processor of the other artificial fireflies is to fast. If the IR-LED flashes 200ms the others
	    would recognize the same flash several times. Even 20ms are to long! Therefor is in the end that short delay of 20ms to ensure that roughly one flash
   	 	is recognized.
    	But the flash length of the IR-LED can't just be made smaller. It has to be at least 10ms because if the flash length is smaller it's not
    	sure if the others recognize the flash!
    	*/
    	if((current_time - start_flash) >= IR_FLASH_LENGTH)
    	{
      		TCCR2B = 0; //clear the prescaler to stop flashing the IR-LED
      		PHI_RAW = 0; //clear the recent value so that it start up from bottom if it is again activated
    	}
    	current_time = millis();
  	}
  	TCCR2B = 0; //ensure that the prescaler to stop flashing the IR-LED is cleared
  	PORTB &= ~(1 << PB0); //kill the visible LED
}

/*
The model of Buck includes the neural delay from the point at which the information that a flash should occur to the visible flash.
That delay is enormous important especially for the phase delay mechanism!!!
*/

void Firelfy::flashBuckPhaseAdvanceAndDelay()
{
	millisecondDelay(NEURAL_DELAY); //neuronal delay

  	int start_flash = millis();
  	int current_time = millis();

  	TCCR2B = 0b00000001; //setting the prescaler to "1"
  	PORTB |= (1 << PB0); //light-up the visible LED

  	//stretch the time in the loop to "flashLength"
  	while((current_time - start_flash) <= VISIBLE_FLASH_LENGTH)
  	{
    	/*
    	The IR-LED flashes only for 20ms because the processor of the other artificial fireflies is to fast. If the IR-LED flashes 200ms the others
    	would recognize the same flash several times. Even 20ms are to long! Therefor is in the end that short delay of 20ms to ensure that roughly one flash
    	is recognized.
    	But the flash length of the IR-LED can't just be made smaller. It has to be at least 10ms because if the flash length is smaller it's not
    	sure if the others recognize the flash!
    	*/
    	if((current_time - start_flash) >= IR_FLASH_LENGTH)
    	{
     		TCCR2B = 0; //clear the prescaler to stop flashing the IR-LED
      		PHI_RAW = 0; //clear the recent value so that it start up from bottom if it is again activated
    	}
      	current_time = millis();
  	}

  	TCCR2B = 0; //ensure that the prescaler is cleared to stop flashing the IR-LED
  	PORTB &= ~(1 << PB0); //kill the visible LED
}

/*
The flash receive handler handles the necessary actions if a flash is received after the model of Mirollo and Strogatz.S
*/

void Firefly::receiveHandlerMirolloStrogatzModel()
{
	//The IR-receiver has an internal pull-up resistor! So it's important to look for an LOW pin if a flash is received.

  	if((flash_receive_A == 0) || (flash_receive_B == 0) || (flash_receive_C == 0) || (flash_receive_D == 0))
  	{
    	x += epsilon; //the current pacemaker point gets lifted about "epsilon"

    	millisecondDelay(IR_FLASH_LENGTH); //ensures that only one flash is recognized per cycle
  	}
}

/*
The model from John Buck predict that only if the threshold (xReset - epsilon) is exceeded entrainment (phase shifting) can occur.
 Before that threshold nothing happens!
*/

void Firefly::receiveHandlerBuckPhaseAdvance()
{
	//The IR-receiver has an internal pull-up resistor! So it's important to look for an LOW pin if a flash is received.

  	if(((flash_receive_A == 0) || (flash_receive_B == 0) || (flash_receive_C == 0) || (flash_receive_D == 0)) && (x + epsilon) >= x_reset)
  	{
  		flashBuckPhaseAdvance(); //the corresponding flash model is called because the firelfy is foreced to flash
   		x = 0; //after that the pacemaker and the state x are reseted to start the cycle again
  		PHI_RAW = 0;
  	}
}

void Firefly::receiveHandlerBuckPhaseDelay()
{
	//The IR-receiver has an internal pull-up resistor! So it's important to look for an LOW pin if a flash is received.

  	/*
  	The Phase-Delay Synchronization model from John Buck is similar to the Phase-Advance Synchronization model with the exception that no flash occurs. Instead
  	the pacemaker and the state "x" are resetted to basal line
  	*/

  	if((flash_receive_A == 0) || (flash_receive_B == 0) || (flash_receive_C == 0) || (flash_receive_D == 0) && ((x + epsilon) >= x_reset))
  	{
    	millisecondDelay(VISIBLE_FLASH_LENGTH);
    	/*
    	this delay is to balance the time between pacemaker reset and and the end of the recognized flash.
    	that is important because if the delay isn't there the reseted firefly would start his pacemaker earier than the other. The result is that they won't
    	synchronize.
    	*/
    	PHI_RAW = 0; //reset the pacemaker to start from basal line
    	x = 0;
  	}
}

/*
The "mapFloat"-function recalculates a value from an specific range to an corresponding value in an other range.
It's like the Arduino "map"-function.
*/

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (float)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}