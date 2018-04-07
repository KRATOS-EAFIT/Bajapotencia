#include "kratos_ev.h"
#include <SPI.h>

#define DEBUG_MODE	0										// if print debug information	
#define NODE				1
#define CAN_INT			2                         // Set INT to pin 2
#define CS_PIN			10												// Set CS to pin 10


//	Global objects and variables                          
kratos_ev EV(NODE, CS_PIN);


void setup()
{
	Serial.begin(9600);

	//	CAN CONFIGURATION
	pinMode(CAN_INT, INPUT);	 // Configuring pin for INT input
	if (EV.init(MCP_STDEXT, CAN_250KBPS, MCP_16MHZ) == CAN_OK)
		Serial.println(F("MCP2515 Initialized Successfully!"));
	else
		Serial.println(F("Error Initializing MCP2515..."));

	//	PINOUT CONFIGURATION ACCORDING WITH NODEID
	EV.set_pinconfig();

	//	ALL PINS ARE SET IN LOW
	EV.reset_outputs();

	//	PRINT NODE INFORMATION
	Serial.print(F("KRATOS EV - NODE "));
	Serial.println(EV.NodeID, DEC);
}


void loop()
{
	EV.intermitent_ligth();

	if (!digitalRead(CAN_INT))													// If CAN_INT pin is low, read receive buffer
	{
		EV.check_message();
	}
}

