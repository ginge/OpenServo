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



#include "StdAfx.h"
#include "IntelHexRec.h"

bool CIntelHexRec::InitFromString(CString str)
{
	CString strVal;
	char *pEnds;
	ULONG uVal;
	int strLen, i;
	str.TrimLeft();
	str.TrimRight();
	if ((str.GetLength() < 10) || (str[0] != ':')) 
		return false;
	errno = 0;
	// extract size
	strVal = str.Mid(1, 2);
	uVal = strtoul(strVal, &pEnds, 16);
	if ((*pEnds) || errno || (uVal > 255))
		return false;
	m_Size = (BYTE)uVal;
	strLen = m_Size * 2 + 11;
	if (str.GetLength() != strLen)
		return false;
	// extract address
	strVal = str.Mid(3, 4);
	uVal = strtoul(strVal, &pEnds, 16);
	if ((*pEnds) || errno || (uVal > 0xFFFF))
		return false;
	m_Addr = (USHORT)uVal;
	// extract type
	strVal = str.Mid(7, 2);
	uVal = strtoul(strVal, &pEnds, 16);
	if ((*pEnds) || errno || (uVal > 4))
		return false;
	m_Type = (BYTE)uVal;
	// extract data
	for (i = 0; i < m_Size; i++)
	{
		strVal = str.Mid(9+i*2, 2);
		uVal = strtoul(strVal, &pEnds, 16);
		if ((*pEnds) || errno || (uVal > 0xFF))
			return false;
		m_Data[i] = (BYTE)uVal;
	}
	// compare crc
	strVal = str.Mid(9+m_Size*2, 2);
	uVal = strtoul(strVal, &pEnds, 16);
	if ((*pEnds) || errno || (uVal > 0xFF))
		return false;
	if (CalcCrc()!=(BYTE)uVal)
		return false;
	return true;
}

BYTE CIntelHexRec::CalcCrc()
{
	ULONG Res = 0;
	Res += m_Size;
	Res += HIBYTE(m_Addr);
	Res += LOBYTE(m_Addr);
	Res += m_Type;
	for (int i = 0; i < m_Size; i++)
		Res += m_Data[i];
	Res = 0x100 - (Res & 0xFF);
	return (BYTE)Res;
}

CString CIntelHexRec::GetString()
{
	CString Res, strTmp;
	Res.Format(":%02X%04X%02X", m_Size, m_Addr, m_Type);
	for (int i = 0; i < m_Size; i++)
	{
		strTmp.Format("%02X", m_Data[i]);
		Res+=strTmp;
	}
	strTmp.Format("%02X\n", CalcCrc());
	Res+=strTmp;
	return Res;
}

bool CIntelHexRec::InitData(BYTE Size, USHORT Addr, BYTE* pData)
{
	ASSERT(pData);
	m_Type = cHexTypeData;
	m_Size = Size;
	m_Addr = Addr;
	CopyMemory(m_Data, pData, Size);
	return true;
}

bool CIntelHexRec::InitEndOfData()
{
	m_Type = cHexTypeEndOfData;
	m_Size = 0;
	m_Addr = 0;
	return true;
}

bool CIntelHexRec::InitExtAddr(USHORT ExtAddr)
{
	m_Type = cHexTypeExtLinearAddr;
	m_Size = 2;
	m_Addr = 0;
	m_Data[0] = HIBYTE(ExtAddr);
	m_Data[1] = LOBYTE(ExtAddr);
	return true;
}

ULONG CIntelHexRec::GetExtAddr()
{
	ASSERT(m_Type == cHexTypeExtLinearAddr);
	ASSERT(m_Size == 2);
	ULONG Res = 0;
	Res |= m_Data[0] << 24;
	Res |= m_Data[1] << 16;
	return Res;
}
