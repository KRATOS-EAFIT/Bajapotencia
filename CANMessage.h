// CANMessage.h

#ifndef _CANMESSAGE_h
#define _CANMESSAGE_h

#include "mcp_can_dfs.h"
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class CANMessage
{

public:
	//	PROPIEDADES
	INT8U   Ext;                                                // Identifier Type - Extended (29 bit) or Standard (11 bit)
	INT32U  ID;																									// CAN ID
	INT8U   Lenght;                                             // Data Length Code
	INT8U   DATA[MAX_CHAR_IN_MESSAGE];													// Data array
	INT8U   RTR;																								// Remote request flag

	// M�TODOS
	INT8U clearmsg();																						//	Clear message
};

#endif

