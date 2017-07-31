/*
the variables a, b, c and d are for the characteristics of f(phiRaw/phiMax)
*/

#define a 1.011
#define b 4.499
#define c 1.011

#define f_CPU 16000000 //clock frequency
#define prescaler 1024 //the prescaler for Timer1 (it's set in the TCCR1B register)
#define phiRaw TCNT1 //the value of the Timer1 is used as the phase variable phi
#define flashLength 200 //in ms


double x = 0; //state x --> x=f(phiRaw/phiMax) 
char xReset = 1; //the flash occurs after "x" exceeds "xReset"
double epsilon = 0.2; //coupling strength, the amount "x" gets lifted up if a flsh is received
double flashInterval = 3; //in s, the maximum timer value (phiMax) is calculated out of the the interval between two flashes --> biggest value is ~4s
unsigned int phiMax = 0; //compare value for phiRaw, the maximum value Timer1 counts to

char flashReceive = HIGH; //holds the value for PB1

int randomValue = 0; //holds later a random value for mapping noise to a "x" and the "flashInterval"
double constFlashIntervalOffset = 0;
/*
The function "flash" handles the whole process if "x" exceeds "xReset". At first we make two new variables to measure the time when the the function is entered.
The other variable stores the current time. Those are necessary to maxe a visible flash. The length of the flash is defined in "flashLength". After that it is
necessary to startup Timer2. That happens if the prescaler is set new. In the end it is again necessary to disable the timer (clear the prescaler).
If this is not done the IR-LED keep on flashing! 

*/

void flash()
{
	int startFlash = millis();
	int currentTime = millis();

	TCCR2B = 0b00000001; //setting the prescaler at "1"
	PORTB |= (1 << PB0); //light-up the visible LED

	//stretch the time in the loop to "flashLength"
	while((currentTime - startFlash) <= flashLength){
	    currentTime = millis();
	}

	TCCR2B = 0; //clear the prescaler to stop flashing the IR-LED flashing
	PORTB &= ~(1 << PB0); //kill the visible LED
}

/*
The "floatMap"-function converts a figure from one range to another. It's return value is a floating point number.
*/
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void setup()
{
	/*
	Initialize Timer1:
	Timer1 is used to keep track of the flashing rythm if the firefly is not entrained/triggered for another one.
	Therefor the Timer1 is driven in the "Normal-Mode". That means that it only counts up. The counting value is in the register TCNT1 or "phiRaw".
	That register can be compared to the variable "phiMax" to figure out when to flash. The value of "phiMax" can be calculated like that:

	phiMax = (f_CPU * flashInterval) / prescaler

	These values are all defined above!
	*/

	TCCR1A = 0; //for Normal-Mode is no Bit in the TCCR1A register necessary
	TCCR1B = 0b00000101; //the last three bits are to set the prescaler for Timer1. For a prescaler of 1024 (the Timer1 increments it's value at every 1024th clock cycle)
	TIMSK1 = 0; //to ensure that no interrupt request is set
	TCNT1 = 0; //initialize the value of Timer1 with 0 so that it starts from bottom to count up

	phiMax = (unsigned int)(f_CPU * flashInterval) / prescaler; //calculation of "phiMax"

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

	The IR-Receiver has an internal filter circuit that filters out any frequency than 38kHz. It also filters out IR light from DC light sources.
	Thats the reason why the IR-LED has to flash with this certain frequency. That means that the Output of the receiver (witch is internally pulled high) is
	not influenced from ambient light. It in not necessary to take a sample. Thats why the output of the IR-Receiver is connected to PB1 (Digital Pin 9).
	This Pin has to be an Input. And for later: we have to wait until the Pin is Low!!!
	*/

	DDRB |= (1 << DDB0) | (1 << DDB3); //declaring PB0 (Digital Pin 8) and PB3 (OC2A, Digital Pin 11) as Output
	DDRB &= ~(1 << DDB1); //declaring PB1 (Digital Pin 9) as an Input

	/*
	The variables "epsilon" and "flashInterval" should be adjustable at first on the breadboard with some potentiometers. Those are hooked up to the Pins
	PC0 (Analog Pin A0) and PC1 (Analog Pin A1). Hence they have to be inputs
	*/

	DDRC &= ~(1 << DDC0); //declaring PC0 (Analog Pin A0) as an Input
	DDRC &= ~(1 << DDC1); //declaring PC1 (Analog Pin A1) as an Input

	randomValue = random(1000);
	constFlashIntervalOffset = (double) mapFloat(randomValue, 0, 1000, -0.05, 0.05); //calculation of the constant flashInterval offset
}

void loop()
{
	/*
	start with checking if the potentiometers have changed
	*/

	epsilon = mapFloat(analogRead(A0), 0, 1023, 0.01, 0.2); //reading in the analog value on pin PC0/A0 and map that value to a value between 0.01 and 0.2
	flashInterval = mapFloat(analogRead(A1), 0, 1023, 0.5, 4) + constFlashIntervalOffset;
	//reading in the new "flashInterval" from PC1/A1 and map it from 500ms to 4s and add the previous calculated (firefly specific) flash interval offset
	phiMax = (unsigned int)((f_CPU * flashInterval) / prescaler); // calculate the new "phiMax" out of the new "flashInterval"

	/*
	If the firefly is in a group of flashing fireflies it is infuenced by the flashes of the others. By recognizing a flash from another firefly it is able
	to phase shift it's current cycle about the coupling strength "epsilon" to the received flash. For two oczillating fireflies the model from Renato E. Mirollo
	and Steven H. Strogatz is good so that it's sure that they synchronize. For more than two the model of John Buck is also good.
	*/

	flashReceive = PINB & (1 << PB1); //saving the state if a flash is received or not. 

	//The IR-receiver has an internal pull-up resistor! So it's important to look for an LOW pin if a flash is received. 

	if(flashReceive == 0)
	{
		x += epsilon; //the current pacemaker point gets lifted about "epsilon"
	}
	flashReceive = HIGH; //ensure that the for the next round the flash receive is HIGH

	/*
	a firefly alone has it's own timeinterval between the the flashes. 
	If the current point of the pacemaker is bigger than the reset point the firefly have to flash and start it's cycle again. If x is smaller than the reset
	point than x will get a new value according to x = f(phiRaw/phiMax)
	*/

	if(x >= xReset)
	{
		flash(); // calling the "flash"-function
		phiRaw = 0; //reset of "phiRaw" and "x" to start from the bottom
		x = 0;
	}else
	{
		x = -a * exp(-(b / phiMax) * phiRaw) + c; //"x" gets it's new value according to x = f_phi
		//function x = f(phiRaw/phiMax) --> since the function is just on an interval between 0 and 1 it has to be normed by the maximum value "phiMax"
	}
	randomValue = random(1000); //taking a random value
	x += mapFloat(randomValue, 0, 1000, -0.001, 0.001); //adding some noise to the current pacemaker point	
	//put that random value in an range of -0.001 to 0.001 --> if the range is to big there are strong visible differneces in the frequency of the cycle!
}
