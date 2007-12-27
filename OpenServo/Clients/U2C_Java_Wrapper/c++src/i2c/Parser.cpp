// Copyright (C)2004 Dimax ( http://www.xdimax.com )
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.


#include "stdafx.h"
#include "Parser.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CParser::CParser()
: m_State(command_expected), m_bDataLengthInited(false), 
  m_bMemoryAddressLengthInited(false), m_bSlaveDeviceAddressInited(false), 
  m_bMemoryAddressInited(false), m_nMemoryAddressLength(1), m_nCurrData(0)
{
}

CParser::~CParser()
{

}

CParser::result CParser::ParseArguments(int argc, char* argv[])
{
	result rValue;
	if (argc < 2)
		return rHelpRequested;
	
	for (int i=1; i<argc; i++)
	{
		char* curArg = argv[i];
		if ((*curArg == '/') || (*curArg == '-'))
		{
			switch(*++curArg)
			{
			case 'H':
			case 'h':
				rValue = rHelpRequested;
				break;
			case 'D':
			case 'd':
				rValue = ParseDataLength(argv[i]+3);
				break;
			case 'A':
			case 'a':
				rValue = ParseMemoryAddressLength(argv[i]+3);
				break;
			case 'S':
			case 's':
				rValue = ParseSlaveDeviceAddress(argv[i]+3);
				break;

			default:
				rValue = rBadParameter;
				break;

			}
		}
		else
		{
			switch(m_State)
			{
			case command_expected:
				rValue = ParseCommand(argv[i]);
				break;
			case data_expected:
				rValue = ParseData(argv[i]);
				break;
			default:
				assert(false);
				break;
			}
		}
		if (rValue != rSuccess)
			return rValue;
	}
	return rValue;
}

CParser::result CParser::ParseDataLength(char* strDataLen)
{
	if (m_bDataLengthInited)
		return rDuplicatedDataLength;
	unsigned long DataLen;
	char *endp;
	DataLen = strtoul(strDataLen, &endp, 0);
	if (*endp)
		return rInvalidDataLength;

	if ((DataLen < 1) || (DataLen > 256))
		return rInvalidDataLength;

	m_nDataLength = (unsigned short) DataLen;
	m_bDataLengthInited = true;
	return rSuccess;
}



CParser::result CParser::ParseMemoryAddressLength(char* strAddr)
{
	if (m_bMemoryAddressLengthInited)
		return rDuplicatedMemoryAddressLengthLength;
	if (*strAddr == 0)
		return rInvalidMemoryAddressLength;
	unsigned long AddrLen;
	char *endp;
	AddrLen = strtoul(strAddr, &endp, 0);
	if (*endp!=':')
		return rInvalidMemoryAddressLength;
	if ((AddrLen != 0) && (AddrLen != 1) && (AddrLen != 2) && (AddrLen != 4))
		return rInvalidMemoryAddressLength;
	m_nMemoryAddressLength = (BYTE)AddrLen;
	m_bMemoryAddressLengthInited = true;
	strAddr = endp + 1;
	return ParseMemoryAddress(strAddr);
}

CParser::result CParser::ParseCommand(char* strCommand)
{
	switch (*strCommand)
	{
	case 'r':
	case 'R':
		m_Command = cRead;
		break;
	case 'w':
	case 'W':
		m_Command = cWrite;
		break;
	default:
		return rInvalidCommand;
	}

	m_State = data_expected;
	return rSuccess;
}

CParser::result CParser::ParseSlaveDeviceAddress(char* strSlaveAddr)
{
	char *endp;
	unsigned long SlaveAddr;
	SlaveAddr = strtoul(strSlaveAddr, &endp, 0);
	if ((*endp) || (SlaveAddr > 0xFF))
		return rInvalidSlaveDeviceAddress;

	m_nSlaveDeviceAddress = (BYTE)SlaveAddr;
	m_bSlaveDeviceAddressInited = true;
	return rSuccess;
}

CParser::result CParser::ParseMemoryAddress(char* strAddr)
{
	char *endp;
	errno = 0;	
	if (*strAddr == 0)
		return rInvalidMemoryAddress;
	m_nMemoryAddress = strtoul(strAddr, &endp, 0); 
	if ((*endp) || (errno != 0))
		return rInvalidMemoryAddress;

	if (((m_nMemoryAddressLength == 1) && (m_nMemoryAddress > 0xFF))
		|| ((m_nMemoryAddressLength == 2) && (m_nMemoryAddress > 0xFFFF))
		)
		return rInvalidMemoryAddress;
	m_bMemoryAddressInited = true;
	return rSuccess;
}


CParser::result CParser::ParseData(char* strData)
{
	if (m_nCurrData == MAX_DATA_LEN)
		return rDataLengthMismatch;
	char *endp;
	errno = 0;
	if (*strData == 0)
		return rInvalidData;
	unsigned long Data = strtoul(strData, &endp, 16);
	if ((*endp) || (errno !=0) || (Data > 0xFF))
		return rInvalidData;
	m_DataBuffer[m_nCurrData] = (BYTE) Data;
	m_nCurrData++;
	return rSuccess;
}

CParser::result CParser::FillTransaction(PU2C_TRANSACTION pRequest)
{
	ZeroMemory(pRequest, sizeof(U2C_TRANSACTION));
	result rValue;

	if (rSuccess != (rValue = FillSlaveDeviceAddress(pRequest)))
		return rValue;
	if (rSuccess != (rValue = FillMemoryAddressLength(pRequest)))
		return rValue;
	if (rSuccess != (rValue = FillMemoryAddress(pRequest)))
		return rValue;
	if (rSuccess != (rValue = CheckBuffer()))
		return rValue;
	if (rSuccess != (rValue = FillBuffer(pRequest)))
		return rValue;

	return rSuccess;
}


CParser::result CParser::FillSlaveDeviceAddress(PU2C_TRANSACTION pTransaction)
{
	if (!m_bSlaveDeviceAddressInited)
		return rAbsentSlaveDeviceAddress;
	
	pTransaction->nSlaveDeviceAddress = m_nSlaveDeviceAddress;

	return rSuccess;
}

CParser::result CParser::FillMemoryAddressLength(PU2C_TRANSACTION pTransaction)
{
//	if (!m_bMemoryAddressLengthInited)
//		return rAbsentMemoryAddressLength;

	pTransaction->nMemoryAddressLength = m_nMemoryAddressLength;

	return rSuccess;
}

CParser::result CParser::FillMemoryAddress(PU2C_TRANSACTION pTransaction)
{
	pTransaction->nMemoryAddress = m_nMemoryAddress;
	return rSuccess;
}

CParser::result CParser::CheckBuffer()
{
	switch(m_Command)
	{
	case cRead:
		if (!m_bDataLengthInited)
			return rAbsentDataLength;
		if (m_nCurrData != 0)
			return rDataInReadCommand;
		break;
	case cWrite:
		if ((m_bDataLengthInited) && (m_nDataLength != m_nCurrData))
			return rDataLengthMismatch;
		if (m_nCurrData == 0)
			return rAbsentData;
		break;
	default:
		return rInvalidCommand;
	}
	return rSuccess;
}

CParser::result CParser::FillBuffer(PU2C_TRANSACTION pTransaction)
{
	switch (m_Command)
	{
	case cRead:
		pTransaction->nBufferLength = m_nDataLength;
		break;
	case cWrite:
		pTransaction->nBufferLength = m_nCurrData;
		break;
	default:
		return rInvalidCommand;
	}
	CopyMemory(pTransaction->Buffer, m_DataBuffer, m_nCurrData);
	return rSuccess;
}














