/*
	mcp_can.h
	2012 Copyright (c) Seeed Technology Inc.  All right reserved.
	2017 Copyright (c) Cory J. Fowler  All Rights Reserved.

	Author:Loovee
	Contributor: Cory J. Fowler
	2017-09-25
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
	1301  USA
*/

#ifndef _MCP_CAN_H_
#define _MCP_CAN_H_

#include "mcp_can_dfs.h"
#include "CANMessage.h"

class MCP_CAN
{
	//	Private properties and methods
private:
	INT8U   mcpMode;                                                    // Mode to return to after configurations are performed.
	INT8U		NodeID;

	//	MCP2515 methods
	INT8U mcp2515_init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock);
	void mcp2515_reset(void);																																// Soft Reset MCP2515
	INT8U mcp2515_readRegister(const INT8U address);																				// Read MCP2515 register
	void mcp2515_readRegisterS(const INT8U address);									// Read MCP2515 successive registers
	void mcp2515_setRegister(const INT8U address, const INT8U value);												// Set MCP2515 register
	void mcp2515_setRegisterS(const INT8U address, const INT8U values[], const INT8U n);		// Set MCP2515 successive registers
	void mcp2515_initCANBuffers(void);
	void mcp2515_modifyRegister(const INT8U address, const INT8U mask, const INT8U data);		// Set specific bit(s) of a register
	INT8U mcp2515_readStatus(void);																													// Read MCP2515 Status
	INT8U mcp2515_setCANCTRL_Mode(const INT8U newmode);																			// Set mode
	INT8U mcp2515_configRate(const INT8U canSpeed, const INT8U canClock);										// Set baud rate
	void mcp2515_write_mf(const INT8U mcp_addr, const INT8U ext, const INT32U id);					// Write CAN Mask or Filter
	void mcp2515_write_id(const INT8U mcp_addr, const INT8U ext, const INT32U id);					// Write CAN ID	
	void mcp2515_read_id(const INT8U mcp_addr);																							// Read CAN ID	
	void mcp2515_write_canMsg(const INT8U buffer_sidh_addr);																// Write CAN message
	void mcp2515_read_canMsg(const INT8U buffer_sidh_addr);																	// Read CAN message
	INT8U mcp2515_getNextFreeTXBuf(INT8U *txbuf_n);																					// Find empty transmit buffer
	INT8U mcp2515_setmask(INT8U num, INT32U ulData);
	INT8U mcp2515_setmask(INT8U num, INT8U ext, INT32U ulData);
	INT8U mcp2515_setfilter(INT8U num, INT32U ulData);
	INT8U mcp2515_setfilter(INT8U num, INT8U ext, INT32U ulData);

	//	CAN methods
	INT8U readMsg();																																// Read message
	INT8U sendMsg();																																// Send message


	//	Public properties and methods
public:
	CANMessage Message;
	INT8U   MCPCS;																																	// Chip Select pin number

	//	Methods
	INT8U init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock);	// Initialize Controller
	INT8U setMask(INT8U num, INT8U ext, INT32U ulData);															// Initialize Mask(s)
	INT8U setFilter(INT8U num, INT8U ext, INT32U ulData);														// Initialize Filter(s)
	INT8U setMode(INT8U opMode);																										// Set operational mode
	INT8U sendMsgBuf();																															// Send message to transmit buffer
	INT8U readMsgBuf();																															// Read message from receive buffer
	INT8U checkReceive(void);																												// Check for received data
	INT8U checkError(void);																													// Check for errors
	INT8U getError(void);																														// Check for errors
	INT8U errorCountRX(void);																												// Get error count
	INT8U errorCountTX(void);																												// Get error count
	INT8U enOneShotTX(void);																												// Enable one-shot transmission
	INT8U disOneShotTX(void);																												// Disable one-shot transmission
	INT8U abortTX(void);																														// Abort queued transmission(s)
	INT8U setGPO(INT8U data);																												// Sets GPO
	INT8U getGPI(void);																															// Reads GPI
	INT8U setKratosEVFilters(INT8U nodeid);																					// Set kratos defined ids filters
};

#endif

