int phi = 0; //current phase of the firefly cycle
int x = 0; //state x --> x=f(phi) 
int epsilon = 0.2 //coupling strength, the amount "x" gets lifted up if a flsh is received

int frequency = 1 //in Hz, the maximum timer value (phi) is calculated out of the frequency

int trigger_threshold = 0 //threshold after that entrainment could occur --> trigger_threshold = x_max - epsilon
int ambient_treshold = 0 //the IR-Receiver has an internal pull-up resistor, so ambient_treshold has to be smaller if a flash is received 

void void setup()
{
	
}

void void loop()
{
	
}