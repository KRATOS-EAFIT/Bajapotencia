#include "kratos_ev.h"


//	Constructor
kratos_ev::kratos_ev(INT8U _nodeid, INT8U _cs)
{
	NodeID = _nodeid;
	CAN.MCPCS = _cs;
	pinMode(CAN.MCPCS, OUTPUT);
}


//	Init function
INT8U kratos_ev::init(INT8U idmode, INT8U canspeed, INT8U clock)
{
	INT8U nReturn = CAN_OK;

	SPI.begin();

	if (CAN.init(idmode, canspeed, clock) != CAN_OK)
		return nReturn;

	//	Setting filters in MCP2515
	if (CAN.setKratosEVFilters(NodeID) != CAN_OK)
		return nReturn;

	//	Set normal mode
	if (CAN.setMode(MCP_NORMAL) != CAN_OK)
		return nReturn;

	// Set pinmode according node id
	set_pinconfig();

	return nReturn;
}

// Set pinmode according node
void kratos_ev::set_pinconfig()
{
	switch (NodeID)
	{
	case 1:
		pinMode(NODE1_LucesBajas, OUTPUT);
		pinMode(NODE1_LucesAltas, OUTPUT);
		pinMode(NODE1_DirDer, OUTPUT);
		pinMode(NODE1_Pito, OUTPUT);
		pinMode(NODE1_Aux1, INPUT);
		pinMode(NODE1_Aux2, INPUT);
		break;

	case 2:
		pinMode(NODE2_LucesBajas, OUTPUT);
		pinMode(NODE2_LucesAltas, OUTPUT);
		pinMode(NODE2_DirIzq, OUTPUT);
		pinMode(NODE2_Pito, OUTPUT);
		pinMode(NODE2_Aux1, INPUT);
		pinMode(NODE2_Aux2, INPUT);
		break;

	case 3:
		pinMode(NODE3_Aux1, INPUT);
		pinMode(NODE3_Aux2, INPUT);
		pinMode(NODE3_DirDer, OUTPUT);
		pinMode(NODE3_Aux3, INPUT);
		pinMode(NODE3_Aux4, INPUT);
		pinMode(NODE3_Aux5, INPUT);
		break;

	case 4:
		pinMode(NODE4_Aux1, INPUT);
		pinMode(NODE4_Aux2, INPUT);
		pinMode(NODE4_DirIzq, OUTPUT);
		pinMode(NODE4_Aux3, INPUT);
		pinMode(NODE4_Aux4, INPUT);
		pinMode(NODE4_Aux5, INPUT);
		break;

	case 5:
		pinMode(NODE5_DirDer, OUTPUT);
		pinMode(NODE5_Aux1, INPUT);
		pinMode(NODE5_Aux2, INPUT);
		pinMode(NODE5_LucesBajas, OUTPUT);
		pinMode(NODE5_Reversa, OUTPUT);
		pinMode(NODE5_Stops, OUTPUT);
		break;

	case 6:
		pinMode(NODE6_DirIzq, OUTPUT);
		pinMode(NODE6_Aux1, INPUT);
		pinMode(NODE6_Aux2, INPUT);
		pinMode(NODE6_LucesBajas, OUTPUT);
		pinMode(NODE6_Reversa, OUTPUT);
		pinMode(NODE6_Stops, OUTPUT);
		break;

	case 7:
		pinMode(NODE7_Stops, OUTPUT);
		pinMode(NODE7_Aux1, INPUT);
		pinMode(NODE7_Aux2, INPUT);
		pinMode(NODE7_Aux3, INPUT);
		pinMode(NODE7_Aux4, INPUT);
		pinMode(NODE7_Aux5, INPUT);
		break;

	default:
		break;
	}
}

//	Reset outputs
void kratos_ev::reset_outputs()
{
	PORTB = 0x00;
	PORTD = 0x00;
}

//	Execute the pending commands
void kratos_ev::sync()
{
	PORTB = PortB;
	PORTD = PortD;

	PortB_Ant = PortB;
	PortD_Ant = PortD;
}

INT8U kratos_ev::check_message()
{
	CAN.Message.clearmsg();

	INT8U nReturn = CAN.readMsgBuf();

	if (nReturn == CAN_OK)										// Read data: len = data length, buf = data byte(s)
	{
		if (CAN.Message.ID == SYNC_CMD)
		{
			// Ejecutar los comandos pendientes
			if ((PortB_Ant != PortB) || (PortD_Ant != PortD))
				sync();
		}
		else if (CAN.Message.ID == RESET_CMD + NodeID)
			reset_outputs();
		else
		{
			switch (CAN.Message.ID - NodeID)
			{
			case TURNLIGHTS_CMD:
				switch (CAN.Message.DATA[0])
				{
				case HAZARD_OFF:
					//	HAZARD OFF
					PortD &= ~(1 << Pin_Dir);
					Intermitent = false;
					break;

				case HAZARD_ON:
					//	HAZARD ON
					PortD |= (1 << Pin_Dir);
					Intermitent = true;
					break;

				case RIGHTTURN_ON:
					PortD |= (1 << Pin_Dir);
					Intermitent = true;
					break;

				case LEFTTURN_ON:
					PortD |= (1 << Pin_Dir);
					Intermitent = true;
					break;
				}

			case LIGHTS_CMD:
				switch (CAN.Message.DATA[0])
				{
				case LIGHTS_OFF:
					PortD &= ~(1 << Pin_HighLights_Stops | 1 << Pin_LowLights);
					break;

				case LOWLIGHTS_ON:
					PortD |= (1 << Pin_LowLights);
					break;

				case HIGHLIGHTS_OFF:
					PortD &= ~(1 << Pin_HighLights_Stops);
					break;

				case HIGHLIGHTS_ON:
					PortD |= (1 << Pin_HighLights_Stops);
					break;

				case STOP_OFF:
					PortD &= ~(1 << Pin_HighLights_Stops);
					break;

				case STOP_ON:
					PortD |= (1 << Pin_HighLights_Stops);
					break;

				case REVERSE_OFF:
					PortD &= ~(1 << Pin_Reverse_Horn);
					break;

				case REVERSE_ON:
					PortD |= (1 << Pin_Reverse_Horn);
					break;
				}

			case HORN_CMD:
				switch (CAN.Message.DATA[0])
				{
				case HORN_OFF:
					//	HORN OFF
					PortD &= ~(1 << Pin_Reverse_Horn);
					break;

				case HORN_ON:
					//	HORN ON
					PortD |= (1 << Pin_Reverse_Horn);
					break;
				}
			}
		}
#if DEBUG_MODE 
		Serial.println();
		Serial.print("ID: ");
		Serial.print(CAN.Message.ID, HEX);
		Serial.print(" Data[0]: ");
		Serial.print(CAN.Message.DATA[0], HEX);
#endif
	}

	return nReturn;
}

void kratos_ev::intermitent_ligth()
{
	if (Intermitent)
	{
		t1 = millis();

		if (t1 - t1Ant >= nPeriod)
		{
			t1Ant = t1;

			if (digitalRead(Pin_Dir) == LOW)
				PortD |= (1 << Pin_Dir);
			else
				PortD &= ~(1 << Pin_Dir);
		}
	}
}

