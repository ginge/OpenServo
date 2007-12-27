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
#include "../common/I2CBridge.h"
#include "I2C.h"

void PrintUsage()
{

	std::cout << "I2C command -s:slave_addr  [-a:addr_length:address] [-d:data_length] [data]\n";
	std::cout << "I2C -h\n";
	std::cout << "\n\n";
	std::cout << "  command         Specifies the command to be executed. Possible values are:\n";
	std::cout << "                  \"r[ead]\" or \"w[rite]\".\n";
	std::cout << "  -h              Prints this help message.\n";
	std::cout << "  -s:slave_addr   Specifies the I2C slave device address.\n";
	std::cout << "  -a:addr_length:address\n"; 
	std::cout << "                  Specifies the length of the slave memory address in bytes\n";
	std::cout << "                  and the slave memory address itself. Possible values for\n";
	std::cout << "                  address length are: 0, 1, 2 or 4. If 0 is provided as\n";
	std::cout << "                  address length, addr parameter have to be omitted.\n";
	std::cout << "                  Default value is 0.\n";
	std::cout << "  -d:data_length  Specifies the length of the data in bytes (decimal value).\n";
	std::cout << "                  For write command data_length can be omitted - it will be\n";
	std::cout << "                  calculated from the data passed. If data_length is\n";
	std::cout << "                  provided for write command it will be compared with \n";
	std::cout << "                  the actual data length and user will be warned in case of\n";
	std::cout << "                  inconsistence.\n";
	std::cout << "                  For read command data_length is obligatory.\n";
	std::cout << "  data            Specifies the data to be sent to device.\n";
	std::cout << "                  For read command data will be omitted.\n";
	std::cout << std::endl;
	std::cout << "  slave_addr, addr_length, address and data_length have to be provided in\n";
	std::cout << "  the following format: [0[x]][digits].\n";
	std::cout << "  If the first character is '0' and the second character is not 'x',\n";
	std::cout << "  the string is interpreted as an octal integer.\n";
	std::cout << "  If the first two characters are \"0x\", the string is interpreted\n";
	std::cout << "  as a hexadecimal integer.\n";
	std::cout << "  If the first character is '1' through '9', the string is interpreted\n";
	std::cout << "  as a decimal integer.\n";
	std::cout << "  data has to be provided as hex string with single bytes separated by space\n";
	std::cout << std::endl;
}

void PrintError(CParser::result res)
{
	if (res == CParser::rHelpRequested)
	{
		PrintUsage();
		return;
	}

	std::string tmp_results[] =
	{
		"rSuccess",
		"rHelprequested",
		"rBadParameter",
		"rInvalidCommand",
		"rInvalidSlaveDeviceAddress",
		"rAbsentSlaveDeviceAddress",
		"rInvalidMemoryAddressLength",
		"rDuplicatedMemoryAddressLengthLength",
		"rAbsentMemoryAddressLength",
		"rInvalidMemoryAddress",
		"rAbsentMemoryAddress",
		"rInvalidDataLength", 
		"rDuplicatedDataLength",
		"rAbsentDataLength",
		"rInvalidData",
		"rDataLengthMismatch",
		"rAbsentData",
		"rDataInreadCommand"
	};
	std::cout << "Parsing error: " << tmp_results[res] << std::endl;
}




int main(int argc, char* argv[])
{
	U2C_TRANSACTION Transaction;
	CParser Parser;
	CParser::result res;
	res = Parser.ParseArguments(argc, argv);
	if (res != CParser::rSuccess)
	{
		PrintError(res);
		return res;
	}
	res = Parser.FillTransaction(&Transaction);
	if (res != CParser::rSuccess)
	{
		PrintError(res);
		return res;
	}
	CI2c I2C;
	if (Parser.GetCommand() == CParser::cRead)
	{
		if (I2C.Read(&Transaction))
		{
			for(unsigned short i = 0; i < Transaction.nBufferLength; i++)
			{
				printf("%02x ", Transaction.Buffer[i]);
				if (i%16 == 15)
					printf("\n");
			}
			if (i%16 != 0)
				printf("\n");
		}
		else
			I2C.PrintError();
	}
	else
	{
		if (!I2C.Write(&Transaction))
			I2C.PrintError();
	}
	return res;
}
