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
#include "Representation.h"

//////////////////////////////////////////////////////////////////////////
// CByteRepr class
//////////////////////////////////////////////////////////////////////////

CString CByteRepr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010X", Addr);
    return res;
}

CString CByteRepr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j++)
    {
        if (j < len)
        {
            strTmp.Format("%02X ", pBuf[j]);
            Data += strTmp;
        }
        else
            Data += "   ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int CByteRepr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    retVal = Addr * 3;
    if (ByteOffset != 0)
        retVal++;
    return retVal;
}

void CByteRepr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 3;
    *pByteOffset = nChar % 3;
    if (*pByteOffset > 1)
        *pByteOffset = 1;
}


void CByteRepr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        *pByteOffset = 0;
    else if (*pAddr > 0)
        *pByteOffset = 1, (*pAddr)--;
}

void CByteRepr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset == 0)
        *pByteOffset = 1;
    else
        *pByteOffset = 0, (*pAddr)++;
}

void CByteRepr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}

void CByteRepr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 1;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool CByteRepr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%02x", *pData);
    if (ByteOffset == 0)
        str[0] = ch;
    else
        str[1] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 16);
    if (errno || *ends)
        return false;
    *pData = (BYTE)Res;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// CAsciiRepr class
//////////////////////////////////////////////////////////////////////////

CString CAsciiRepr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010X", Addr);
    return res;
}

CString CAsciiRepr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j++)
    {
        if (j < len)
        {
            if (pBuf[j] < 0x20)
                Data+=".";
            else
            {
                strTmp.Format("%c", pBuf[j]);
                Data+=strTmp;
            }
        }
        else
            Data+=" ";
    }
    return Data;
}

unsigned int CAsciiRepr::AddrToChar(unsigned int Addr, unsigned int /*ByteOffset*/)
{
    ASSERT(Addr < GetBytesPerRow());
    return Addr;
}

void CAsciiRepr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar;
    *pByteOffset = 0;
}

void CAsciiRepr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pAddr > 0)
        *pByteOffset = 0, (*pAddr)--;
}

void CAsciiRepr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0, (*pAddr)++;
}

void CAsciiRepr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}
void CAsciiRepr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool CAsciiRepr::OnChar(BYTE* pData, unsigned int /*ByteOffset*/, char ch)
{
    *pData = ch;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// CNibbleRepr class
//////////////////////////////////////////////////////////////////////////

CString CNibbleRepr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010X", Addr*2);
    return res;
}

CString CNibbleRepr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j++)
    {
        if (j < len)
        {
            BYTE hiNibble, loNibble;
            hiNibble = BYTE(0x0F & (pBuf[j]>>4));
            loNibble = BYTE(0x0F & pBuf[j]);
            strTmp.Format("%1X %1X ", hiNibble, loNibble);
            Data += strTmp;
        }
        else
            Data += "    ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int CNibbleRepr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    retVal = Addr * 4;
    if (ByteOffset != 0)
        retVal+=2;
    return retVal;
}

void CNibbleRepr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 4;
    *pByteOffset = nChar % 4;
    *pByteOffset /= 2;
}

void CNibbleRepr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        *pByteOffset = 0;
    else if (*pAddr > 0)
        *pByteOffset = 1, (*pAddr)--;
}

void CNibbleRepr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset == 0)
        *pByteOffset = 1;
    else
        *pByteOffset = 0, (*pAddr)++;
}

void CNibbleRepr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}

void CNibbleRepr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 1;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool CNibbleRepr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%02x", *pData);
    if (ByteOffset == 0)
        str[0] = ch;
    else
        str[1] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 16);
    if (errno || *ends)
        return false;
    *pData = (BYTE)Res;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// COctalRepr class
//////////////////////////////////////////////////////////////////////////

CString COctalRepr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010o", Addr);
    return res;
}

CString COctalRepr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j++)
    {
        if (j < len)
        {
            strTmp.Format("%03o ", pBuf[j]);
            Data += strTmp;
        }
        else
            Data += "    ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int COctalRepr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    retVal = Addr * 4;
    if (ByteOffset <= 2)
        retVal+=ByteOffset;
    return retVal;
}

void COctalRepr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 4;
    *pByteOffset = nChar % 4;
    if (*pByteOffset > 2)
        *pByteOffset = 2;
}

void COctalRepr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        (*pByteOffset)--;
    else if (*pAddr > 0)
        *pByteOffset = 2, (*pAddr)--;
}

void COctalRepr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset < 2)
        (*pByteOffset)++;
    else
        *pByteOffset = 0, (*pAddr)++;
}

void COctalRepr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}

void COctalRepr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 2;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool COctalRepr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%03o", *pData);
    if (ByteOffset < 2)
        str[ByteOffset] = ch;
    else
        str[2] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 8);
    if (errno || *ends || (Res > 0xFF))
        return false;
    *pData = (BYTE)Res;
    return true;
}


//////////////////////////////////////////////////////////////////////////
// CDecimalRepr class
//////////////////////////////////////////////////////////////////////////

CString CDecimalRepr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010u", Addr);
    return res;
}

CString CDecimalRepr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j++)
    {
        if (j < len)
        {
            strTmp.Format("%03u ", pBuf[j]);
            Data += strTmp;
        }
        else
            Data += "    ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int CDecimalRepr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    retVal = Addr * 4;
    if (ByteOffset <= 2)
        retVal+=ByteOffset;
    return retVal;
}

void CDecimalRepr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 4;
    *pByteOffset = nChar % 4;
    if (*pByteOffset > 2)
        *pByteOffset = 2;
}


void CDecimalRepr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        (*pByteOffset)--;
    else if (*pAddr > 0)
        *pByteOffset = 2, (*pAddr)--;
}

void CDecimalRepr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset < 2)
        (*pByteOffset)++;
    else
        *pByteOffset = 0, (*pAddr)++;
}

void CDecimalRepr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}

void CDecimalRepr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 2;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool CDecimalRepr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%03u", *pData);
    if (ByteOffset < 2)
        str[ByteOffset] = ch;
    else
        str[2] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 10);
    if (errno || *ends || (Res > 0xFF))
        return false;
    *pData = (BYTE)Res;
    return true;
}

//////////////////////////////////////////////////////////////////////////
// CWordB0B1Repr class
//////////////////////////////////////////////////////////////////////////

CString CWordB0B1Repr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010X", Addr/2);
    return res;
}

CString CWordB0B1Repr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j+=2)
    {
        if (j < len)
        {
            strTmp.Format("%02X%02X ", pBuf[j], pBuf[j+1]);
            Data += strTmp;
        }
        else
            Data += "     ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int CWordB0B1Repr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    unsigned int WordAddr = Addr/2;
    retVal = WordAddr * 5;
    if (Addr%2 == 1)
        retVal += 2;
    if (ByteOffset != 0)
        retVal+=1;
    return retVal;
}

void CWordB0B1Repr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 5 * 2;
    *pByteOffset = nChar % 5;
    if (*pByteOffset > 1)
    {
        *pByteOffset -= 2;
        (*pAddr)++;
        if (*pByteOffset > 1)
            *pByteOffset = 1;
    }
}

void CWordB0B1Repr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        *pByteOffset = 0;
    else if (*pAddr > 0)
        *pByteOffset = 1, (*pAddr)--;
}

void CWordB0B1Repr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset == 0)
        *pByteOffset = 1;
    else
        *pByteOffset = 0, (*pAddr)++;
}

void CWordB0B1Repr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow();
}

void CWordB0B1Repr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 1;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 1;
}

bool CWordB0B1Repr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%02x", *pData);
    if (ByteOffset == 0)
        str[0] = ch;
    else
        str[1] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 16);
    if (errno || *ends)
        return false;
    *pData = (BYTE)Res;
    return true;
}


//////////////////////////////////////////////////////////////////////////
// CWordB1B0Repr class
//////////////////////////////////////////////////////////////////////////

CString CWordB1B0Repr::GetAddrStr(unsigned int Addr)
{
    CString res;
    res.Format("%010X", Addr/2);
    return res;
}

CString CWordB1B0Repr::GetRowText(BYTE *pBuf, unsigned int len)
{
    CString Data, strTmp;
    for (unsigned int j=0; j < GetBytesPerRow(); j+=2)
    {
        if (j < len)
        {
            strTmp.Format("%02X%02X ", pBuf[j+1], pBuf[j]);
            Data += strTmp;
        }
        else
            Data += "     ";
    }
    Data.Delete(Data.GetLength() - 1); // remove last space
    return Data;
}

unsigned int CWordB1B0Repr::AddrToChar(unsigned int Addr, unsigned int ByteOffset)
{
    ASSERT(Addr < GetBytesPerRow());
    unsigned int retVal;
    unsigned int WordAddr = Addr/2;
    retVal = WordAddr * 5;
    if (Addr%2 == 0)
        retVal += 2;
    if (ByteOffset != 0)
        retVal+=1;
    return retVal;
}

void CWordB1B0Repr::CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar)
{
    *pAddr = nChar / 5 * 2;
    *pByteOffset = nChar % 5;
    if (*pByteOffset > 1)
    {
        *pByteOffset -= 2;
        if (*pByteOffset > 1)
            *pByteOffset = 1;
    }
    else
        (*pAddr)++;
}

void CWordB1B0Repr::MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset != 0)
        *pByteOffset = 0;
    else
    {
        if (*pAddr % 2 == 0)
            *pByteOffset = 1, (*pAddr)++;
        else if (*pAddr >= 3)
            *pByteOffset = 1, *pAddr-=3;
    }
}

void CWordB1B0Repr::MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset)
{
    if (*pByteOffset == 0)
        *pByteOffset = 1;
    else
    {
        *pByteOffset = 0;
        if (*pAddr % 2 == 0)
            *pAddr+=3;
        else
            (*pAddr)--;
    }
}

void CWordB1B0Repr::MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 0;
    *pAddr = (*pAddr/GetBytesPerRow())*GetBytesPerRow() + 1;
}

void CWordB1B0Repr::MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset)
{
    *pByteOffset = 1;
    *pAddr = (*pAddr/GetBytesPerRow()+1)*GetBytesPerRow() - 2;
}

bool CWordB1B0Repr::OnChar(BYTE* pData, unsigned int ByteOffset, char ch)
{
    char str[10], *ends;
    sprintf(str, "%02x", *pData);
    if (ByteOffset == 0)
        str[0] = ch;
    else
        str[1] = ch;
    errno = 0;
    unsigned long Res = strtoul(str, &ends, 16);
    if (errno || *ends)
        return false;
    *pData = (BYTE)Res;
    return true;
}
