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
#include "Settings.h"
#include "TwiBootProg.h"

CSettings::CSettings(void)
{
}

CSettings::~CSettings(void)
{
}

UINT CSettings::GetProfileUint(LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT nDefault)
{
    return theApp.GetProfileInt(lpszSection, lpszEntry, (UINT)nDefault);
}

BOOL CSettings::WriteProfileUint(LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT nValue)
{
    return theApp.WriteProfileInt(lpszSection, lpszEntry, (UINT)nValue);
}

INT CSettings::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault)
{
    return (INT)theApp.GetProfileInt(lpszSection, lpszEntry, nDefault);
}

BOOL CSettings::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue)
{
    return theApp.WriteProfileInt(lpszSection, lpszEntry, nValue);
}

BYTE CSettings::GetProfileByte(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE nDefault)
{
    UINT Val;
    Val = GetProfileUint(lpszSection, lpszEntry, nDefault);
    if (Val > 0xFF)
    {
        ASSERT(FALSE);
        return nDefault;
    }
    return (BYTE)Val;
}

BOOL CSettings::WriteProfileByte(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE nValue)
{
    return WriteProfileUint(lpszSection, lpszEntry, nValue);
}

CString CSettings::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
    return theApp.GetProfileString(lpszSection, lpszEntry, lpszDefault);
}

BOOL CSettings::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
    return theApp.WriteProfileString(lpszSection, lpszEntry, lpszValue);
}
