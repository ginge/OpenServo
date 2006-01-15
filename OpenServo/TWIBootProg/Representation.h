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

class CRepresentation
{
public:
    virtual CString GetAddrHeader()=0;
    virtual CString GetAddrStr(unsigned int Addr) = 0;
    virtual unsigned int GetBytesPerRow() = 0;
    virtual CString GetHeaderText() = 0;
    virtual CString GetRowText(BYTE *pBuf, unsigned int len) = 0;
    virtual unsigned int GetCharPerRow() = 0;
    virtual unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset) = 0;
    virtual void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar) = 0;
    virtual void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset) = 0;
    virtual void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset) = 0;
    virtual void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset) = 0;
    virtual void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset) = 0;
    virtual bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch) = 0;
};


class CByteRepr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Byte";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 0x10;};
    CString GetHeaderText(){return "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 0x10 * 3 -1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

class CAsciiRepr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Byte";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 0x10;};
    CString GetHeaderText(){return "0123456789ABCDEF";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 0x10;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};


class CNibbleRepr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Nibble";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 0x8;};
    CString GetHeaderText(){return "0 1 2 3 4 5 6 7 8 9 A B C D E F";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 0x10*2-1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

class COctalRepr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Octal";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 020;};
    CString GetHeaderText(){return "000 001 002 003 004 005 006 007 010 011 012 013 014 015 016 017";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 020*4-1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

class CDecimalRepr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Decimal";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 10;};
    CString GetHeaderText(){return "000 001 002 003 004 005 006 007 008 009";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 10*4-1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

class CWordB0B1Repr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Word B0:B1";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 0x10;};
    CString GetHeaderText(){return "0000 0001 0002 0003 0004 0005 0006 0007";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 0x8*5-1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

class CWordB1B0Repr: public CRepresentation
{
public:
    CString GetAddrHeader() {return "Word B1:B0";};
    CString GetAddrStr(unsigned int Addr);
    unsigned int GetBytesPerRow(){return 0x10;};
    CString GetHeaderText(){return "0000 0001 0002 0003 0004 0005 0006 0007";};
    CString GetRowText(BYTE *pBuf, unsigned int len);
    unsigned int GetCharPerRow(){return 0x8*5-1;};
    unsigned int AddrToChar(unsigned int Addr, unsigned int ByteOffset);
    void CharToAddr(unsigned int *pAddr, unsigned int *pByteOffset, unsigned int nChar);
    void MoveCaretLeft(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretRight(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretHome(unsigned int *pAddr, unsigned int *pByteOffset);
    void MoveCaretEnd(unsigned int *pAddr, unsigned int *pByteOffset);
    bool OnChar(BYTE* pData, unsigned int ByteOffset, char ch);
};

