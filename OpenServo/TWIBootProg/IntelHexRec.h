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

#pragma once

const BYTE cHexTypeData = 0;
const BYTE cHexTypeEndOfData = 1;
const BYTE cHexTypeExtLinearAddr = 4;

class CIntelHexRec
{
public:
    CIntelHexRec(void){};
    ~CIntelHexRec(void){};
    bool InitFromString(CString str);
    bool InitData(BYTE Size, USHORT Addr, BYTE* pData);
    bool InitEndOfData();
    bool InitExtAddr(USHORT ExtAddr);
    ULONG GetExtAddr();
    CString GetString();
    BYTE CalcCrc();

    BYTE m_Size;
    USHORT m_Addr;
    BYTE m_Type;
    BYTE m_Data[256];
};
