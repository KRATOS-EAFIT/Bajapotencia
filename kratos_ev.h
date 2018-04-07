// kratos_ev.h

#ifndef _KRATOS_EV_h
#define _KRATOS_EV_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include "kratos_ev_defs.h"
#include "mcp_can_dfs.h"
#include "mcp_can.h"

class kratos_ev
{
private:
	INT8U PortB;
	INT8U PortD;
	INT8U PortB_Ant;
	INT8U PortD_Ant;
	INT32U nPeriod = 500;					// Period of turn lights in ms
	INT32U t1 = 0;
	INT32U t1Ant = 0;

	//	Properties
public:
	INT8U NodeID;
	MCP_CAN CAN;
	bool Intermitent;

	//	Methods
public:
	kratos_ev(INT8U _nodeid, INT8U _cs);
	INT8U init(INT8U idmode, INT8U canspeed, INT8U clock);
	void set_pinconfig();
	void reset_outputs();
	void sync();
	INT8U check_message();
	void intermitent_ligth();
};


#endif


