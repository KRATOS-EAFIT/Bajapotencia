// KRATOS_EV.h

#ifndef _KRATOS_EV_DEFS_h
#define _KRATOS_EV_DEFS_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif


//	COMANDOS CAN
//	**********************************
#define SYNC_CMD					0x80
#define RESET_CMD					0x200

#define TURNLIGHTS_CMD		0x300
#define HAZARD_OFF				0xA0
#define	HAZARD_ON					0xA1
#define	RIGHTTURN_ON			0xA2
#define	LEFTTURN_ON				0xA3

#define LIGHTS_CMD				0x400
#define LIGHTS_OFF				0xB0
#define	LOWLIGHTS_ON			0xB1
#define HIGHLIGHTS_ON			0xB3
#define	HIGHLIGHTS_OFF		0xB2
#define STOP_OFF					0xC0
#define	STOP_ON						0xC1
#define	REVERSE_OFF				0xD0
#define	REVERSE_ON				0xD1

#define HORN_CMD					0x500
#define	HORN_OFF					0xE0
#define	HORN_ON						0xE1

#define REQUEST_CMD				0x700
//	**********************************


//	PINOUT NODE
//	**********************************
#define NODE1_LucesBajas	3
#define	NODE1_LucesAltas	4
#define NODE1_DirDer			5
#define NODE1_Pito				6
#define NODE1_Aux1				7
#define	NODE1_Aux2				8

#define NODE2_LucesBajas	3
#define	NODE2_LucesAltas	4
#define NODE2_DirIzq			5
#define NODE2_Pito				6
#define NODE2_Aux1				7
#define	NODE2_Aux2				8

#define NODE3_Aux1				3
#define	NODE3_Aux2				4
#define NODE3_DirDer			5
#define NODE3_Aux3				6
#define NODE3_Aux4				7
#define	NODE3_Aux5				8

#define NODE4_Aux1				3
#define	NODE4_Aux2				4
#define NODE4_DirIzq			5
#define NODE4_Aux3				6
#define NODE4_Aux4				7
#define	NODE4_Aux5				8

#define NODE5_LucesBajas	3
#define	NODE5_Stops				4
#define NODE5_DirDer			5
#define NODE5_Reversa			6
#define NODE5_Aux1				7
#define	NODE5_Aux2				8

#define NODE6_LucesBajas	3
#define	NODE6_Stops				4
#define NODE6_DirIzq			5
#define NODE6_Reversa			6
#define NODE6_Aux1				7
#define	NODE6_Aux2				8

#define NODE7_Aux1				3
#define	NODE7_Stops				4
#define NODE7_Aux2				5
#define NODE7_Aux3				6
#define NODE7_Aux4				7
#define	NODE7_Aux5				8
//	**********************************

//	OUTPUTS BITWISE
//	**********************************
#define Pin_LowLights					3		//	PD3
#define Pin_HighLights_Stops	4		//	PD4
#define	Pin_Dir								5		//	PD5
#define	Pin_Reverse_Horn			6		//	PD6
#define	Pin_Aux1							7		//	PD7
#define	Pin_Aux2							1		//	PB0

#define	HazardPeriod			500				// Period of turn lights in ms

#endif


