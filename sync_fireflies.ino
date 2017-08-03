/*
the variables a, b, c and d are for the characteristics of f(phiRaw/phi_max)
*/

#define a 1.011
#define b 4.499
#define c 1.011

#define F_CPU 16000000 //clock frequency
#define PRESCALER 1024 //the PRESCALER for Timer1 (it's set in the TCCR1B register)
#define PHI_RAW TCNT1 //the value of the Timer1 is used as the phase variable phi
#define VISIBLE_FLASH_LENGTH 200 //in ms --> flashing time ot the visible LED
#define IR_FLASH_LENGTH 20 //in ms --> flashing time of the IR-LED


double x = 0; //state x --> x=f(phiRaw/phi_max) 
char x_reset = 1; //the flash occurs after "x" exceeds "x_reset"
double epsilon = 0.2; //coupling strength, the amount "x" gets lifted up if a flash is received
double flash_interval = 3; //in s, the maximum timer value (phi_max) is calculated out of the the interval between two flashes --> biggest value is ~4s
unsigned int phi_max = 0; //compare value for PHI_RAW, the maximum value Timer1 counts to

char flash_receive_A = HIGH; //holds the value for PB1
char flash_receive_B = HIGH; //holds the value for PB2
char flash_receive_C = HIGH; //holds the value for PB4
char flash_receive_D = HIGH; //holds the value for PB5

int random_value = 0; //holds later a random value for mapping noise to a "x" and the "flash_interval"
double constant_flash_interval_offset = 0;

/*
The function "flash" handles the whole process if "x" exceeds "x_reset". At first we make two new variables to measure the time when the the function is entered.
The other variable stores the current time. Those are necessary to make a visible flash. The length of the flash is defined in "FLASH_LENGTH". After that it is
necessary to startup Timer2. That happens if the prescaler is set new in the register TCCR2B.
In the end it is again necessary to disable the timer (clear the prescaler). If this is not done the IR-LED keep on flashing! 

*/

void flash()
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
		if((current_time - start_tlash) >= IR_FLASH_LENGTH)
		{
			TCCR2B = 0; //clear the prescaler to stop flashing the IR-LED flashing
			PHI_RAW = 0; //clear the recent value so that it start up from bottom if it is again activated  
		}
	    current_time = millis();
	}

	TCCR2B = 0; //clear the prescaler to stop flashing the IR-LED flashing
	PORTB &= ~(1 << PB0); //kill the visible LED
}

/*
The "floatMap"-function converts a figure from one range to another like the Arduino map-function does. But the mapFloat-function returns a floating point number.
*/
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*
The millisecond_delay-function is used to ensure that no timer (especially not the timer1) is modified for the original Arduino delay-function.
*/

void millisecond_delay(int time_delay)
{
	int start_time = millis();
	int current_time = millis();

	while((current_time - start_time) >= time_delay)
	{
		current_time = millis();
	}
}

void setup()
{
	/*
	Initialize Timer1:
	Timer1 is used to keep track of the flashing rythm if the firefly is not entrained/triggered for another one.
	Therefor the Timer1 is driven in the "Normal-Mode". That means that it only counts up. The counting value is in the register TCNT1 or "phiRaw".
	That register can be compared to the variable "phi_max" to figure out when to flash. The value of "phi_max" can be calculated like that:

	phi_max = (f_CPU * flashInterval) / prescaler

	These values are all defined above!
	*/

	TCCR1A = 0; //for Normal-Mode is no Bit in the TCCR1A register necessary
	TCCR1B = 0b00000101; //the last three bits are to set the prescaler for Timer1. For a prescaler of 1024 (the Timer1 increments it's value at every 1024th clock cycle)
	TIMSK1 = 0; //to ensure that no interrupt request is set

	phi_maxi = (unsigned int)(f_CPU * flashInterval) / prescaler; //calculation of "phi_max"
	TCNT1 = random(phi_max); //initialize the pacemaker with a random value

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

	random_value = random(1000);
	constant_flash_interval_offset = (double) mapFloat(randomValue, 0, 1000, -0.05, 0.05); //calculation of the constant flash_interval offset
}

void loop()
{
	/*
	start with checking if the potentiometers have changed
	*/

	epsilon = mapFloat(analogRead(A0), 0, 1023, 0.01, 0.2); //reading in the analog value on pin PC0/A0 and map that value to a value between 0.01 and 0.2
	flash_interval = mapFloat(analogRead(A1), 0, 1023, 0.5, 4) + constant_flash_interval_offset;
	//reading in the new "flash_interval" from PC1/A1 and map it from 500ms to 4s and add the previous calculated (firefly specific) flash interval offset
	phi_max = (unsigned int)((F_CPU * flash_interval) / PRESCALER); // calculate the new "phi_max" out of the new "flash_interval"

	/*
	a firefly alone has it's own timeinterval between the the flashes. 
	If the current point of the pacemaker is bigger than the reset point the firefly have to flash and start it's cycle again. If x is smaller than the reset
	point x_reset than x will get a new value according to x = f(PHI_RAW/phi_max). This function is concave down on an interval phi = [0;1] and x = [0;1]
	*/

	if(x >= x_reset)
	{
		flash(); // calling the "flash"-function
		PHI_RAW = 0; //reset of "PHI_RAW" and "x" to start from the bottom
		x = 0;
	}else
	{
		x = -a * exp(-(b / phi_max) * PHI_RAW) + c; //"x" gets it's new value according to x = f_phi
		//function x = f(PHI_RAW/phi_max) --> since the function is just on an interval between 0 and 1 it has to be normed by the maximum value "phi_max"
	}
	random_value = random(1000); //taking a random value
	x += mapFloat(random_value, 0, 1000, -0.001, 0.001); //adding some noise to the current pacemaker point	
	//put that random value in an range of -0.001 to 0.001 --> if the range is to big there are strong visible differneces in the frequency of the cycle!

	/*
	If the firefly is in a group of flashing fireflies it is infuenced by the flashes of the others. By recognizing a flash from another firefly it is able
	to phase shift it's current cycle about the coupling strength "epsilon" to the received flash. For two oczillating fireflies the model from Renato E. Mirollo
	and Steven H. Strogatz is good so that it's sure that they synchronize. For more than two the model of John Buck is also good.
	*/

	flash_receive_A = PINB & (1 << PB1); //saving the state if a flash is received or not for bin PB1.
	flash_receive_B = PINB & (1 << PB2); //saving the state if a flash is received or not for bin PB2.
	flash_receive_C = PINB & (1 << PB4); //saving the state if a flash is received or not for bin PB4.
	flash_receive_D = PINB & (1 << PB5); //saving the state if a flash is received or not for bin PB5.

	//The IR-receiver has an internal pull-up resistor! So it's important to look for an LOW pin if a flash is received. 

	if((flash_receive_A == 0) || (flash_receive_B == 0) || (flash_receive_C == 0) || (flash_receive_D == 0)) 
	{
		x += epsilon; //the current pacemaker point gets lifted about "epsilon"
	}

	flash_receive_A = HIGH; //ensure that the for the next round the flash receive is HIGH
	flash_receive_B = HIGH;
	flash_receive_C = HIGH;
	flash_receive_D = HIGH;

	millisecond_delay(IR_flash); //ensures that only one flash is recognized per cycle
}
