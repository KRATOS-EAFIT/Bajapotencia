/*
	mcp_can.cpp
	2012 Copyright (c) Seeed Technology Inc.  All right reserved.
	2017 Copyright (c) Cory J. Fowler  All Rights Reserved.

	Author: Loovee
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

#include "mcp_can.h"
#include "kratos_ev_defs.h"


/*********************************************************************************************************
																					MCP2515 PRIVATE METHODS
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            Initialize the controller
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock)
{
	INT8U nReturn = MCP2515_FAIL;

	MCP2515_UNSELECT();

	mcp2515_reset();
	mcpMode = MCP_LOOPBACK;
	nReturn = mcp2515_setCANCTRL_Mode(MODE_CONFIG);

	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Configuration Mode Failure...\r\n"));
#endif
		return nReturn;
	}
#if DEBUG_MODE
	Serial.print(F("Entering Configuration Mode Successful!\r\n"));
#endif

	// Set Baudrate
	if (mcp2515_configRate(canSpeed, canClock))
	{
#if DEBUG_MODE
		Serial.print(F("Setting Baudrate Failure...\r\n"));
#endif
		return nReturn;
	}
#if DEBUG_MODE
	Serial.print(F("Setting Baudrate Successful!\r\n"));
#endif

	if (nReturn == MCP2515_OK) {

		/* init canbuffers              */
		mcp2515_initCANBuffers();

		/* interrupt mode               */
		mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

		//Sets BF pins as GPO
		mcp2515_setRegister(MCP_BFPCTRL, MCP_BxBFS_MASK | MCP_BxBFE_MASK);

		//Sets RTS pins as GPI
		mcp2515_setRegister(MCP_TXRTSCTRL, 0x00);

		switch (canIDMode)
		{
		case (MCP_ANY):
			mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
			mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_ANY);
			break;
			/*          The followingn two functions of the MCP2515 do not work, there is a bug in the silicon.
			case (MCP_STD):
			mcp2515_modifyRegister(MCP_RXB0CTRL,
			MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
			MCP_RXB_RX_STD | MCP_RXB_BUKT_MASK );
			mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
			MCP_RXB_RX_STD);
			break;

			case (MCP_EXT):
			mcp2515_modifyRegister(MCP_RXB0CTRL,
			MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
			MCP_RXB_RX_EXT | MCP_RXB_BUKT_MASK );
			mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
			MCP_RXB_RX_EXT);
			break;
			*/
		case (MCP_STDEXT):
			mcp2515_modifyRegister(MCP_RXB0CTRL, MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK, MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
			mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
			break;

		default:
#if DEBUG_MODE        
			Serial.printF(("`Setting ID Mode Failure...\r\n"));
#endif           
			return MCP2515_FAIL;
			break;
		}

		nReturn = mcp2515_setCANCTRL_Mode(mcpMode);
		if (nReturn)
		{
#if DEBUG_MODE        
			Serial.print(F("Returning to Previous Mode Failure...\r\n"));
#endif           
			return nReturn;
		}
	}
	return nReturn;
}

/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            Performs a software reset
*********************************************************************************************************/
void MCP_CAN::mcp2515_reset(void)
{
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_RESET);
	MCP2515_UNSELECT();
	SPI.endTransaction();
	delayMicroseconds(10);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            Read data register
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_readRegister(const INT8U address)
{
	INT8U nReturn;

	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_READ);
	spi_readwrite(address);

	nReturn = spi_read();
	MCP2515_UNSELECT();
	SPI.endTransaction();

	return nReturn;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            Reads sucessive data registers
*********************************************************************************************************/
void MCP_CAN::mcp2515_readRegisterS(const INT8U address)
{
	INT8U i;
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_READ);
	spi_readwrite(address);
	// mcp2515 has auto-increment of address-pointer
	for (i = 0; i < Message.Lenght; i++)
		Message.DATA[i] = spi_read();

	MCP2515_UNSELECT();
	SPI.endTransaction();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            Sets data register
*********************************************************************************************************/
void MCP_CAN::mcp2515_setRegister(const INT8U address, const INT8U value)
{
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_WRITE);
	spi_readwrite(address);
	spi_readwrite(value);
	MCP2515_UNSELECT();
	SPI.endTransaction();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            Sets sucessive data registers
*********************************************************************************************************/
void MCP_CAN::mcp2515_setRegisterS(const INT8U address, const INT8U values[], const INT8U n)
{
	INT8U i;
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_WRITE);
	spi_readwrite(address);

	for (i = 0; i < n; i++)
		spi_readwrite(values[i]);

	MCP2515_UNSELECT();
	SPI.endTransaction();
}

/*********************************************************************************************************
** Function name:           mcp2515_initCANBuffers
** Descriptions:            Initialize Buffers, Masks, and Filters
*********************************************************************************************************/
void MCP_CAN::mcp2515_initCANBuffers(void)
{
	INT8U i, a1, a2, a3;
	INT32U ulMask = 0x00, ulFilt = 0x00;

	mcp2515_write_mf(MCP_RXM0SIDH, CAN_EXTID, ulMask);			/*Set both masks to 0           */
	mcp2515_write_mf(MCP_RXM1SIDH, CAN_EXTID, ulMask);			/*Mask register ignores ext bit */

																													/* Set all filters to 0         */
	mcp2515_write_mf(MCP_RXF0SIDH, CAN_EXTID, ulFilt);			/* RXB0: extended               */
	mcp2515_write_mf(MCP_RXF1SIDH, CAN_EXTID, ulFilt);			/* RXB1: standard               */
	mcp2515_write_mf(MCP_RXF2SIDH, CAN_EXTID, ulFilt);			/* RXB2: extended               */
	mcp2515_write_mf(MCP_RXF3SIDH, CAN_EXTID, ulFilt);			/* RXB3: standard               */
	mcp2515_write_mf(MCP_RXF4SIDH, CAN_EXTID, ulFilt);
	mcp2515_write_mf(MCP_RXF5SIDH, CAN_EXTID, ulFilt);

	//	Clear, deactivate the three transmit buffers TXBnCTRL -> TXBnD7
	a1 = MCP_TXB0CTRL;
	a2 = MCP_TXB1CTRL;
	a3 = MCP_TXB2CTRL;

	for (i = 0; i < 14; i++)
	{                                          /* in-buffer loop               */
		mcp2515_setRegister(a1, 0);
		mcp2515_setRegister(a2, 0);
		mcp2515_setRegister(a3, 0);
		a1++;
		a2++;
		a3++;
	}

	mcp2515_setRegister(MCP_RXB0CTRL, 0);
	mcp2515_setRegister(MCP_RXB1CTRL, 0);
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            Sets specific bits of a register
*********************************************************************************************************/
void MCP_CAN::mcp2515_modifyRegister(const INT8U address, const INT8U mask, const INT8U data)
{
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_BITMOD);
	spi_readwrite(address);
	spi_readwrite(mask);
	spi_readwrite(data);
	MCP2515_UNSELECT();
	SPI.endTransaction();
}

/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            Reads status register
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_readStatus(void)
{
	INT8U nReturn;
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	MCP2515_SELECT();
	spi_readwrite(MCP_READ_STATUS);
	nReturn = spi_read();
	MCP2515_UNSELECT();
	SPI.endTransaction();
	return nReturn;
}

/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            Set control mode
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setCANCTRL_Mode(const INT8U newmode)
{
	INT8U nReturn = MCP2515_FAIL;

	mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);

	nReturn = mcp2515_readRegister(MCP_CANCTRL);
	nReturn &= MODE_MASK;

	if (nReturn == newmode)
		return MCP2515_OK;

	return nReturn;
}

/*********************************************************************************************************
** Function name:           mcp2515_configRate
** Descriptions:            Set baudrate
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_configRate(const INT8U canSpeed, const INT8U canClock)
{
	INT8U set, cfg1, cfg2, cfg3;
	set = 1;

	switch (canClock)
	{
	case (MCP_8MHZ):
		switch (canSpeed)
		{
		case (CAN_5KBPS):                                               //   5KBPS                  
			cfg1 = MCP_8MHz_5kBPS_CFG1;
			cfg2 = MCP_8MHz_5kBPS_CFG2;
			cfg3 = MCP_8MHz_5kBPS_CFG3;
			break;

		case (CAN_10KBPS):                                              //  10KBPS                  
			cfg1 = MCP_8MHz_10kBPS_CFG1;
			cfg2 = MCP_8MHz_10kBPS_CFG2;
			cfg3 = MCP_8MHz_10kBPS_CFG3;
			break;

		case (CAN_20KBPS):                                              //  20KBPS                  
			cfg1 = MCP_8MHz_20kBPS_CFG1;
			cfg2 = MCP_8MHz_20kBPS_CFG2;
			cfg3 = MCP_8MHz_20kBPS_CFG3;
			break;

		case (CAN_31K25BPS):                                            //  31.25KBPS                  
			cfg1 = MCP_8MHz_31k25BPS_CFG1;
			cfg2 = MCP_8MHz_31k25BPS_CFG2;
			cfg3 = MCP_8MHz_31k25BPS_CFG3;
			break;

		case (CAN_33K3BPS):                                             //  33.33KBPS                  
			cfg1 = MCP_8MHz_33k3BPS_CFG1;
			cfg2 = MCP_8MHz_33k3BPS_CFG2;
			cfg3 = MCP_8MHz_33k3BPS_CFG3;
			break;

		case (CAN_40KBPS):                                              //  40Kbps
			cfg1 = MCP_8MHz_40kBPS_CFG1;
			cfg2 = MCP_8MHz_40kBPS_CFG2;
			cfg3 = MCP_8MHz_40kBPS_CFG3;
			break;

		case (CAN_50KBPS):                                              //  50Kbps
			cfg1 = MCP_8MHz_50kBPS_CFG1;
			cfg2 = MCP_8MHz_50kBPS_CFG2;
			cfg3 = MCP_8MHz_50kBPS_CFG3;
			break;

		case (CAN_80KBPS):                                              //  80Kbps
			cfg1 = MCP_8MHz_80kBPS_CFG1;
			cfg2 = MCP_8MHz_80kBPS_CFG2;
			cfg3 = MCP_8MHz_80kBPS_CFG3;
			break;

		case (CAN_100KBPS):                                             // 100Kbps
			cfg1 = MCP_8MHz_100kBPS_CFG1;
			cfg2 = MCP_8MHz_100kBPS_CFG2;
			cfg3 = MCP_8MHz_100kBPS_CFG3;
			break;

		case (CAN_125KBPS):                                             // 125Kbps
			cfg1 = MCP_8MHz_125kBPS_CFG1;
			cfg2 = MCP_8MHz_125kBPS_CFG2;
			cfg3 = MCP_8MHz_125kBPS_CFG3;
			break;

		case (CAN_200KBPS):                                             // 200Kbps
			cfg1 = MCP_8MHz_200kBPS_CFG1;
			cfg2 = MCP_8MHz_200kBPS_CFG2;
			cfg3 = MCP_8MHz_200kBPS_CFG3;
			break;

		case (CAN_250KBPS):                                             // 250Kbps
			cfg1 = MCP_8MHz_250kBPS_CFG1;
			cfg2 = MCP_8MHz_250kBPS_CFG2;
			cfg3 = MCP_8MHz_250kBPS_CFG3;
			break;

		case (CAN_500KBPS):                                             // 500Kbps
			cfg1 = MCP_8MHz_500kBPS_CFG1;
			cfg2 = MCP_8MHz_500kBPS_CFG2;
			cfg3 = MCP_8MHz_500kBPS_CFG3;
			break;

		case (CAN_1000KBPS):                                            //   1Mbps
			cfg1 = MCP_8MHz_1000kBPS_CFG1;
			cfg2 = MCP_8MHz_1000kBPS_CFG2;
			cfg3 = MCP_8MHz_1000kBPS_CFG3;
			break;

		default:
			set = 0;
			return MCP2515_FAIL;
			break;
		}
		break;

	case (MCP_16MHZ):
		switch (canSpeed)
		{
		case (CAN_5KBPS):                                               //   5Kbps
			cfg1 = MCP_16MHz_5kBPS_CFG1;
			cfg2 = MCP_16MHz_5kBPS_CFG2;
			cfg3 = MCP_16MHz_5kBPS_CFG3;
			break;

		case (CAN_10KBPS):                                              //  10Kbps
			cfg1 = MCP_16MHz_10kBPS_CFG1;
			cfg2 = MCP_16MHz_10kBPS_CFG2;
			cfg3 = MCP_16MHz_10kBPS_CFG3;
			break;

		case (CAN_20KBPS):                                              //  20Kbps
			cfg1 = MCP_16MHz_20kBPS_CFG1;
			cfg2 = MCP_16MHz_20kBPS_CFG2;
			cfg3 = MCP_16MHz_20kBPS_CFG3;
			break;

		case (CAN_33K3BPS):                                              //  20Kbps
			cfg1 = MCP_16MHz_33k3BPS_CFG1;
			cfg2 = MCP_16MHz_33k3BPS_CFG2;
			cfg3 = MCP_16MHz_33k3BPS_CFG3;
			break;

		case (CAN_40KBPS):                                              //  40Kbps
			cfg1 = MCP_16MHz_40kBPS_CFG1;
			cfg2 = MCP_16MHz_40kBPS_CFG2;
			cfg3 = MCP_16MHz_40kBPS_CFG3;
			break;

		case (CAN_50KBPS):                                              //  50Kbps
			cfg2 = MCP_16MHz_50kBPS_CFG2;
			cfg3 = MCP_16MHz_50kBPS_CFG3;
			break;

		case (CAN_80KBPS):                                              //  80Kbps
			cfg1 = MCP_16MHz_80kBPS_CFG1;
			cfg2 = MCP_16MHz_80kBPS_CFG2;
			cfg3 = MCP_16MHz_80kBPS_CFG3;
			break;

		case (CAN_100KBPS):                                             // 100Kbps
			cfg1 = MCP_16MHz_100kBPS_CFG1;
			cfg2 = MCP_16MHz_100kBPS_CFG2;
			cfg3 = MCP_16MHz_100kBPS_CFG3;
			break;

		case (CAN_125KBPS):                                             // 125Kbps
			cfg1 = MCP_16MHz_125kBPS_CFG1;
			cfg2 = MCP_16MHz_125kBPS_CFG2;
			cfg3 = MCP_16MHz_125kBPS_CFG3;
			break;

		case (CAN_200KBPS):                                             // 200Kbps
			cfg1 = MCP_16MHz_200kBPS_CFG1;
			cfg2 = MCP_16MHz_200kBPS_CFG2;
			cfg3 = MCP_16MHz_200kBPS_CFG3;
			break;

		case (CAN_250KBPS):                                             // 250Kbps
			cfg1 = MCP_16MHz_250kBPS_CFG1;
			cfg2 = MCP_16MHz_250kBPS_CFG2;
			cfg3 = MCP_16MHz_250kBPS_CFG3;
			break;

		case (CAN_500KBPS):                                             // 500Kbps
			cfg1 = MCP_16MHz_500kBPS_CFG1;
			cfg2 = MCP_16MHz_500kBPS_CFG2;
			cfg3 = MCP_16MHz_500kBPS_CFG3;
			break;

		case (CAN_1000KBPS):                                            //   1Mbps
			cfg1 = MCP_16MHz_1000kBPS_CFG1;
			cfg2 = MCP_16MHz_1000kBPS_CFG2;
			cfg3 = MCP_16MHz_1000kBPS_CFG3;
			break;

		default:
			set = 0;
			return MCP2515_FAIL;
			break;
		}
		break;

	case (MCP_20MHZ):
		switch (canSpeed)
		{
		case (CAN_40KBPS):                                              //  40Kbps
			cfg1 = MCP_20MHz_40kBPS_CFG1;
			cfg2 = MCP_20MHz_40kBPS_CFG2;
			cfg3 = MCP_20MHz_40kBPS_CFG3;
			break;

		case (CAN_50KBPS):                                              //  50Kbps
			cfg1 = MCP_20MHz_50kBPS_CFG1;
			cfg2 = MCP_20MHz_50kBPS_CFG2;
			cfg3 = MCP_20MHz_50kBPS_CFG3;
			break;

		case (CAN_80KBPS):                                              //  80Kbps
			cfg1 = MCP_20MHz_80kBPS_CFG1;
			cfg2 = MCP_20MHz_80kBPS_CFG2;
			cfg3 = MCP_20MHz_80kBPS_CFG3;
			break;

		case (CAN_100KBPS):                                             // 100Kbps
			cfg1 = MCP_20MHz_100kBPS_CFG1;
			cfg2 = MCP_20MHz_100kBPS_CFG2;
			cfg3 = MCP_20MHz_100kBPS_CFG3;
			break;

		case (CAN_125KBPS):                                             // 125Kbps
			cfg1 = MCP_20MHz_125kBPS_CFG1;
			cfg2 = MCP_20MHz_125kBPS_CFG2;
			cfg3 = MCP_20MHz_125kBPS_CFG3;
			break;

		case (CAN_200KBPS):                                             // 200Kbps
			cfg1 = MCP_20MHz_200kBPS_CFG1;
			cfg2 = MCP_20MHz_200kBPS_CFG2;
			cfg3 = MCP_20MHz_200kBPS_CFG3;
			break;

		case (CAN_250KBPS):                                             // 250Kbps
			cfg1 = MCP_20MHz_250kBPS_CFG1;
			cfg2 = MCP_20MHz_250kBPS_CFG2;
			cfg3 = MCP_20MHz_250kBPS_CFG3;
			break;

		case (CAN_500KBPS):                                             // 500Kbps
			cfg1 = MCP_20MHz_500kBPS_CFG1;
			cfg2 = MCP_20MHz_500kBPS_CFG2;
			cfg3 = MCP_20MHz_500kBPS_CFG3;
			break;

		case (CAN_1000KBPS):                                            //   1Mbps
			cfg1 = MCP_20MHz_1000kBPS_CFG1;
			cfg2 = MCP_20MHz_1000kBPS_CFG2;
			cfg3 = MCP_20MHz_1000kBPS_CFG3;
			break;

		default:
			set = 0;
			return MCP2515_FAIL;
			break;
		}
		break;

	default:
		set = 0;
		return MCP2515_FAIL;
		break;
	}

	if (set)
	{
		mcp2515_setRegister(MCP_CNF1, cfg1);
		mcp2515_setRegister(MCP_CNF2, cfg2);
		mcp2515_setRegister(MCP_CNF3, cfg3);
		return MCP2515_OK;
	}

	return MCP2515_FAIL;
}

/*********************************************************************************************************
** Function name:           mcp2515_write_mf
** Descriptions:            Write Masks and Filters
*********************************************************************************************************/
void MCP_CAN::mcp2515_write_mf(const INT8U mcp_addr, const INT8U ext, const INT32U id)
{
	uint16_t canid;
	INT8U tbufdata[4];

	canid = (uint16_t)(id & 0x0FFFF);

	if (ext == 1)
	{
		tbufdata[MCP_EID0] = (INT8U)(canid & 0xFF);
		tbufdata[MCP_EID8] = (INT8U)(canid >> 8);
		canid = (uint16_t)(id >> 16);
		tbufdata[MCP_SIDL] = (INT8U)(canid & 0x03);
		tbufdata[MCP_SIDL] += (INT8U)((canid & 0x1C) << 3);
		tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
		tbufdata[MCP_SIDH] = (INT8U)(canid >> 5);
	}
	else
	{
		tbufdata[MCP_EID0] = (INT8U)(canid & 0xFF);
		tbufdata[MCP_EID8] = (INT8U)(canid >> 8);
		canid = (uint16_t)(id >> 16);
		tbufdata[MCP_SIDL] = (INT8U)((canid & 0x07) << 5);
		tbufdata[MCP_SIDH] = (INT8U)(canid >> 3);
	}

	mcp2515_setRegisterS(mcp_addr, tbufdata, 4);
}

/*********************************************************************************************************
** Function name:           mcp2515_write_id
** Descriptions:            Write CAN ID
*********************************************************************************************************/
void MCP_CAN::mcp2515_write_id(const INT8U mcp_addr, const INT8U ext, const INT32U id)
{
	uint16_t canid;
	INT8U tbufdata[4];

	canid = (uint16_t)(id & 0x0FFFF);

	if (ext == 1)
	{
		tbufdata[MCP_EID0] = (INT8U)(canid & 0xFF);
		tbufdata[MCP_EID8] = (INT8U)(canid >> 8);
		canid = (uint16_t)(id >> 16);
		tbufdata[MCP_SIDL] = (INT8U)(canid & 0x03);
		tbufdata[MCP_SIDL] += (INT8U)((canid & 0x1C) << 3);
		tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
		tbufdata[MCP_SIDH] = (INT8U)(canid >> 5);
	}
	else
	{
		tbufdata[MCP_SIDH] = (INT8U)(canid >> 3);
		tbufdata[MCP_SIDL] = (INT8U)((canid & 0x07) << 5);
		tbufdata[MCP_EID0] = 0;
		tbufdata[MCP_EID8] = 0;
	}

	mcp2515_setRegisterS(mcp_addr, tbufdata, 4);
}

/*********************************************************************************************************
** Function name:           mcp2515_read_id
** Descriptions:            Read CAN ID
*********************************************************************************************************/
void MCP_CAN::mcp2515_read_id(const INT8U mcp_addr)
{
	Message.Ext = 0;
	Message.ID = 0;
	Message.Lenght = 4;

	mcp2515_readRegisterS(mcp_addr);

	Message.ID = (Message.DATA[MCP_SIDH] << 3) + (Message.DATA[MCP_SIDL] >> 5);
	if ((Message.DATA[MCP_SIDL] & MCP_TXB_EXIDE_M) == MCP_TXB_EXIDE_M)
	{
		/* extended id                  */
		Message.ID = (Message.ID << 2) + (Message.DATA[MCP_SIDL] & 0x03);
		Message.ID = (Message.ID << 8) + Message.DATA[MCP_EID8];
		Message.ID = (Message.ID << 8) + Message.DATA[MCP_EID0];
		Message.Ext = 1;
	}
}

/*********************************************************************************************************
** Function name:           mcp2515_write_canMsg
** Descriptions:            Write message
*********************************************************************************************************/
void MCP_CAN::mcp2515_write_canMsg(const INT8U buffer_sidh_addr)
{
	INT8U mcp_addr;
	mcp_addr = buffer_sidh_addr;
	mcp2515_setRegisterS(mcp_addr + 5, Message.DATA, Message.Lenght);                  /* write data bytes             */

	if (Message.RTR == 1)                                                   /* if RTR set bit in byte       */
		Message.Lenght |= MCP_RTR_MASK;

	mcp2515_setRegister((mcp_addr + 4), Message.Lenght);                         /* write the RTR and DLC        */
	mcp2515_write_id(mcp_addr, Message.Ext, Message.ID);                      /* write CAN id                 */
}

/*********************************************************************************************************
** Function name:           mcp2515_read_canMsg
** Descriptions:            Read message
*********************************************************************************************************/
void MCP_CAN::mcp2515_read_canMsg(const INT8U buffer_sidh_addr)        /* read can msg                 */
{
	INT8U mcp_addr, ctrl;

	mcp_addr = buffer_sidh_addr;

	mcp2515_read_id(mcp_addr);

	ctrl = mcp2515_readRegister(mcp_addr - 1);
	Message.Lenght = mcp2515_readRegister(mcp_addr + 4);

	if (ctrl & 0x08)
		Message.RTR = 1;
	else
		Message.RTR = 0;

	Message.Lenght &= MCP_DLC_MASK;
	mcp2515_readRegisterS(mcp_addr + 5);
}

/*********************************************************************************************************
** Function name:           mcp2515_getNextFreeTXBuf
** Descriptions:            Send message
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_getNextFreeTXBuf(INT8U *txbuf_n)                 /* get Next free txbuf          */
{
	INT8U res, i, ctrlval;
	INT8U ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };

	res = MCP_ALLTXBUSY;
	*txbuf_n = 0x00;

	/* check all 3 TX-Buffers       */
	for (i = 0; i < MCP_N_TXBUFFERS; i++) {
		ctrlval = mcp2515_readRegister(ctrlregs[i]);
		if ((ctrlval & MCP_TXB_TXREQ_M) == 0) {
			*txbuf_n = ctrlregs[i] + 1;                                   /* return SIDH-address of Buffer*/

			res = MCP2515_OK;
			return res;                                                 /* ! function exit              */
		}
	}
	return res;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            function to set mask(s).
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setmask(INT8U num, INT8U ext, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;
#if DEBUG_MODE
	Serial.print(F("Starting to Set Mask!\r\n"));
#endif
	nReturn = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
	if (nReturn != MCP2515_OK) {
#if DEBUG_MODE
		Serial.print(F("Entering Configuration Mode Failure...\r\n"));
#endif
		return nReturn;
	}

	if (num == 0)
	{
		mcp2515_write_mf(MCP_RXM0SIDH, ext, ulData);
	}
	else if (num == 1) {
		mcp2515_write_mf(MCP_RXM1SIDH, ext, ulData);
	}
	else nReturn = MCP2515_FAIL;

	nReturn = mcp2515_setCANCTRL_Mode(mcpMode);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Previous Mode Failure...\r\nSetting Mask Failure...\r\n"));
#endif
		return nReturn;
	}
#if DEBUG_MODE
	Serial.print(F("Setting Mask Successful!\r\n"));
#endif
	return nReturn;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            Public function to set mask(s).
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setmask(INT8U num, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;
	INT8U ext = 0;

#if DEBUG_MODE
	Serial.print(F("Starting to Set Mask!\r\n"));
#endif
	nReturn = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Configuration Mode Failure...\r\n"));
#endif
		return nReturn;
	}

	if ((ulData & 0x80000000) == 0x80000000)
		ext = 1;

	if (num == 0) {
		mcp2515_write_mf(MCP_RXM0SIDH, ext, ulData);

	}
	else if (num == 1) {
		mcp2515_write_mf(MCP_RXM1SIDH, ext, ulData);
	}
	else nReturn = MCP2515_FAIL;

	nReturn = mcp2515_setCANCTRL_Mode(mcpMode);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Previous Mode Failure...\r\nSetting Mask Failure...\r\n"));
#endif
		return nReturn;
	}
#if DEBUG_MODE
	Serial.print(F("Setting Mask Successful!\r\n"));
#endif

	return nReturn;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            Public function to set filter(s).
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setfilter(INT8U num, INT8U ext, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;

#if DEBUG_MODE
	Serial.print(F("Starting to Set Filter!\r\n"));
#endif
	nReturn = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Enter Configuration Mode Failure...\r\n"));
#endif
		return nReturn;
	}

	switch (num)
	{
	case 0:
		mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
		break;

	case 1:
		mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
		break;

	case 2:
		mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
		break;

	case 3:
		mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
		break;

	case 4:
		mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
		break;

	case 5:
		mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
		break;

	default:
		nReturn = MCP2515_FAIL;
	}

	nReturn = mcp2515_setCANCTRL_Mode(mcpMode);
	if (nReturn > 0)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Previous Mode Failure...\r\nSetting Filter Failure...\r\n"));
#endif
		return nReturn;
	}

#if DEBUG_MODE
	Serial.print(F("Setting Filter Successfull!\r\n"));
#endif

	return nReturn;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            Public function to set filter(s).
*********************************************************************************************************/
INT8U MCP_CAN::mcp2515_setfilter(INT8U num, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;
	INT8U ext = 0;

#if DEBUG_MODE
	Serial.print(F("Starting to Set Filter!\r\n"));
#endif
	nReturn = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Enter Configuration Mode Failure...\r\n"));
#endif
		return nReturn;
	}

	if ((ulData & 0x80000000) == 0x80000000)
		ext = 1;

	switch (num)
	{
	case 0:
		mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
		break;

	case 1:
		mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
		break;

	case 2:
		mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
		break;

	case 3:
		mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
		break;

	case 4:
		mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
		break;

	case 5:
		mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
		break;

	default:
		nReturn = MCP2515_FAIL;
	}

	nReturn = mcp2515_setCANCTRL_Mode(mcpMode);
	if (nReturn != MCP2515_OK)
	{
#if DEBUG_MODE
		Serial.print(F("Entering Previous Mode Failure...\r\nSetting Filter Failure...\r\n"));
#endif
		return nReturn;
	}
#if DEBUG_MODE
	Serial.print(F("Setting Filter Successfull!\r\n"));
#endif

	return nReturn;
}




/*********************************************************************************************************
																			CAN PRIVATE METHODS
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:           readMsg
** Descriptions:            Read message
*********************************************************************************************************/
INT8U MCP_CAN::readMsg()
{
	INT8U stat, nReturn;

	stat = mcp2515_readStatus();

	if (stat & MCP_STAT_RX0IF)                                        /* Msg in Buffer 0              */
	{
		mcp2515_read_canMsg(MCP_RXBUF_0);
		mcp2515_modifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
		nReturn = CAN_OK;
	}
	else if (stat & MCP_STAT_RX1IF)                                   /* Msg in Buffer 1              */
	{
		mcp2515_read_canMsg(MCP_RXBUF_1);
		mcp2515_modifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
		nReturn = CAN_OK;
	}
	else
		nReturn = CAN_NOMSG;

	return nReturn;
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            Send message
*********************************************************************************************************/
INT8U MCP_CAN::sendMsg()
{
	INT8U res, res1, txbuf_n;
	uint16_t uiTimeOut = 0;

	do {
		res = mcp2515_getNextFreeTXBuf(&txbuf_n);                       /* info = addr.                 */
		uiTimeOut++;
	} while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

	if (uiTimeOut == TIMEOUTVALUE)
	{
		return CAN_GETTXBFTIMEOUT;                                      /* get tx buff time out         */
	}
	uiTimeOut = 0;
	mcp2515_write_canMsg(txbuf_n);
	mcp2515_modifyRegister(txbuf_n - 1, MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);

	do
	{
		uiTimeOut++;
		res1 = mcp2515_readRegister(txbuf_n - 1);                         /* read send buff ctrl reg 	*/
		res1 = res1 & 0x08;
	} while (res1 && (uiTimeOut < TIMEOUTVALUE));

	if (uiTimeOut == TIMEOUTVALUE)                                       /* send msg timeout             */
		return CAN_SENDMSGTIMEOUT;

	return CAN_OK;
}


/*********************************************************************************************************
																							PUBLIC METHODS
*********************************************************************************************************/

/*********************************************************************************************************
** Function name:           init
** Descriptions:            Public initialize the controller
*********************************************************************************************************/
INT8U MCP_CAN::init(const INT8U canIDMode, const INT8U canSpeed, const INT8U canClock)
{
	INT8U nReturn = CAN_OK;

	nReturn = mcp2515_init(canIDMode, canSpeed, canClock);

	return nReturn;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            Public function to set mask(s).
*********************************************************************************************************/
INT8U MCP_CAN::setMask(INT8U num, INT8U ext, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;

	nReturn = mcp2515_setmask(num, ext, ulData);

	return nReturn;
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            Public function to set filter(s).
*********************************************************************************************************/
INT8U MCP_CAN::setFilter(INT8U num, INT8U ext, INT32U ulData)
{
	INT8U nReturn = MCP2515_OK;

	nReturn = mcp2515_setfilter(num, ext, ulData);

	return nReturn;
}

/*********************************************************************************************************
** Function name:           setMode
** Descriptions:            Sets control mode
*********************************************************************************************************/
INT8U MCP_CAN::setMode(const INT8U opMode)
{
	mcpMode = opMode;
	return mcp2515_setCANCTRL_Mode(mcpMode);
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            Send message to transmitt buffer
*********************************************************************************************************/
INT8U MCP_CAN::sendMsgBuf()
{
	INT8U ext = 0, rtr = 0;
	INT8U nReturn;

	if ((Message.ID & 0x80000000) == 0x80000000)
		Message.Ext = 1;

	if ((Message.ID & 0x40000000) == 0x40000000)
		Message.RTR = 1;

	nReturn = sendMsg();

	return nReturn;
}

/*********************************************************************************************************
** Function name:           readMsgBuf
** Descriptions:            Public function, Reads message from receive buffer.
*********************************************************************************************************/
INT8U MCP_CAN::readMsgBuf()
{
	if (readMsg() == CAN_NOMSG)
		return CAN_NOMSG;

	if (Message.Ext)
		Message.ID |= 0x80000000;

	if (Message.RTR)
		Message.ID |= 0x40000000;

	return CAN_OK;
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            Public function, Checks for received data.  (Used if not using the interrupt output)
*********************************************************************************************************/
INT8U MCP_CAN::checkReceive(void)
{
	INT8U res;
	res = mcp2515_readStatus();                                         /* RXnIF in Bit 1 and 0         */
	if (res & MCP_STAT_RXIF_MASK)
		return CAN_MSGAVAIL;
	else
		return CAN_NOMSG;
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            Public function, Returns error register data.
*********************************************************************************************************/
INT8U MCP_CAN::checkError(void)
{
	INT8U eflg = mcp2515_readRegister(MCP_EFLG);

	if (eflg & MCP_EFLG_ERRORMASK)
		return CAN_CTRLERROR;
	else
		return CAN_OK;
}

/*********************************************************************************************************
** Function name:           getError
** Descriptions:            Returns error register value.
*********************************************************************************************************/
INT8U MCP_CAN::getError(void)
{
	return mcp2515_readRegister(MCP_EFLG);
}

/*********************************************************************************************************
** Function name:           mcp2515_errorCountRX
** Descriptions:            Returns REC register value
*********************************************************************************************************/
INT8U MCP_CAN::errorCountRX(void)
{
	return mcp2515_readRegister(MCP_REC);
}

/*********************************************************************************************************
** Function name:           mcp2515_errorCountTX
** Descriptions:            Returns TEC register value
*********************************************************************************************************/
INT8U MCP_CAN::errorCountTX(void)
{
	return mcp2515_readRegister(MCP_TEC);
}

/*********************************************************************************************************
** Function name:           mcp2515_enOneShotTX
** Descriptions:            Enables one shot transmission mode
*********************************************************************************************************/
INT8U MCP_CAN::enOneShotTX(void)
{
	mcp2515_modifyRegister(MCP_CANCTRL, MODE_ONESHOT, MODE_ONESHOT);
	if ((mcp2515_readRegister(MCP_CANCTRL) & MODE_ONESHOT) != MODE_ONESHOT)
		return CAN_FAIL;
	else
		return CAN_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_disOneShotTX
** Descriptions:            Disables one shot transmission mode
*********************************************************************************************************/
INT8U MCP_CAN::disOneShotTX(void)
{
	mcp2515_modifyRegister(MCP_CANCTRL, MODE_ONESHOT, 0);
	if ((mcp2515_readRegister(MCP_CANCTRL) & MODE_ONESHOT) != 0)
		return CAN_FAIL;
	else
		return CAN_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_abortTX
** Descriptions:            Aborts any queued transmissions
*********************************************************************************************************/
INT8U MCP_CAN::abortTX(void)
{
	mcp2515_modifyRegister(MCP_CANCTRL, ABORT_TX, ABORT_TX);

	// Maybe check to see if the TX buffer transmission request bits are cleared instead?
	if ((mcp2515_readRegister(MCP_CANCTRL) & ABORT_TX) != ABORT_TX)
		return CAN_FAIL;
	else
		return CAN_OK;
}

/*********************************************************************************************************
** Function name:           setGPO
** Descriptions:            Public function, Checks for r
*********************************************************************************************************/
INT8U MCP_CAN::setGPO(INT8U data)
{
	mcp2515_modifyRegister(MCP_BFPCTRL, MCP_BxBFS_MASK, (data << 4));

	return 0;
}

/*********************************************************************************************************
** Function name:           getGPI
** Descriptions:            Public function, Checks for r
*********************************************************************************************************/
INT8U MCP_CAN::getGPI(void)
{
	INT8U res;
	res = mcp2515_readRegister(MCP_TXRTSCTRL) & MCP_BxRTS_MASK;
	return (res >> 3);
}

/*********************************************************************************************************
** Function name:           setKratosEVFilters
** Descriptions:            Public which sets Kratos EV filters on mcp2515
*********************************************************************************************************/
INT8U MCP_CAN::setKratosEVFilters(INT8U nodeid)
{
	INT8U nReturn = MCP2515_OK;
	INT32U nVal = 0;

	if (mcp2515_setmask(0, CAN_STDID, 0x07FF0000) != MCP2515_OK)
		return MCP2515_FAIL;

	if (mcp2515_setmask(1, CAN_STDID, 0x07FF0000) != MCP2515_OK)
		return MCP2515_FAIL;
	
	nVal = SYNC_CMD;
	nVal = nVal << 16;
	if (mcp2515_setfilter(0, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	nVal = RESET_CMD + nodeid;
	nVal = nVal << 16;
	if (mcp2515_setfilter(1, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	nVal = TURNLIGHTS_CMD + nodeid;
	nVal = nVal << 16;
	if (mcp2515_setfilter(2, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	nVal = LIGHTS_CMD + nodeid;
	nVal = nVal << 16;
	if (mcp2515_setfilter(3, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	nVal = HORN_CMD + nodeid;
	nVal = nVal << 16;
	if (mcp2515_setfilter(4, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	nVal = REQUEST_CMD + nodeid;
	nVal = nVal << 16;
	if (mcp2515_setfilter(5, CAN_STDID, nVal) != MCP2515_OK)
		return MCP2515_FAIL;

	return nReturn;
}

/*********************************************************************************************************
	END FILE
*********************************************************************************************************/

