#include "CANMessage.h"

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            Initialize the controller
*********************************************************************************************************/
INT8U CANMessage::clearmsg()
{
	ID = 0;
	Ext = 0;
	RTR = 0;

	for (int i = 0; i < Lenght; i++)
		DATA[i] = 0x00;

	Lenght = 0;

	return MCP2515_OK;
}

