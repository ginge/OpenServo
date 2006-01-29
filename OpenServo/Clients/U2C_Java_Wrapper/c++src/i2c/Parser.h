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


#if !defined(AFX_PARSER_H__A9227E4D_DCF5_4F24_BA7E_3C34301D83DD__INCLUDED_)
#define AFX_PARSER_H__A9227E4D_DCF5_4F24_BA7E_3C34301D83DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_DATA_LEN 256

class CParser  
{
public:
	enum result
	{
		rSuccess,
		rHelpRequested,
		rBadParameter,
		rInvalidCommand, // read or write expected

		rInvalidSlaveDeviceAddress,
		rAbsentSlaveDeviceAddress,
		
		
		rInvalidMemoryAddressLength,
		rDuplicatedMemoryAddressLengthLength,
		rAbsentMemoryAddressLength,



		rInvalidMemoryAddress,
		rAbsentMemoryAddress,


		rInvalidDataLength, 
		rDuplicatedDataLength,
		rAbsentDataLength,


		rInvalidData,
		rDataLengthMismatch,
		rAbsentData,
		rDataInReadCommand // have been found redundant data in read command
	};

	enum command
	{
		cRead,
		cWrite
	};

	command GetCommand() {return m_Command;};
	CParser();
	virtual ~CParser();
	result FillTransaction(PU2C_TRANSACTION pRequest);
	result ParseArguments(int argc, char* argv[]);

private:
//	PU2C_TRANSACTION m_pRequest;
	unsigned short m_nDataLength; 
	bool m_bDataLengthInited;

	BYTE m_nMemoryAddressLength;
	bool m_bMemoryAddressLengthInited;

	BYTE m_nSlaveDeviceAddress;
	bool m_bSlaveDeviceAddressInited;

	DWORD m_nMemoryAddress;
	bool m_bMemoryAddressInited;
	
	command m_Command;
	BYTE m_DataBuffer[MAX_DATA_LEN];
	int m_nCurrData;

	enum
	{
		command_expected,
		data_expected,
	} m_State;

	result ParseDataLength(char* strDataLen);
	result ParseMemoryAddressLength(char* strAddrLen);
	result ParseCommand(char* strCommand);
	result ParseData(char* strData);
	result ParseSlaveDeviceAddress(char* strSlaveAddr);
	result ParseMemoryAddress(char* strAddr);

	result FillSlaveDeviceAddress(PU2C_TRANSACTION pTransaction);
	result FillMemoryAddressLength(PU2C_TRANSACTION pTransaction);
	result FillMemoryAddress(PU2C_TRANSACTION pTransaction);
	result CheckBuffer();
	result FillBuffer(PU2C_TRANSACTION pTransaction);
};

#endif // !defined(AFX_PARSER_H__A9227E4D_DCF5_4F24_BA7E_3C34301D83DD__INCLUDED_)
