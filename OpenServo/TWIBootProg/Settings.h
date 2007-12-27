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

#include "Singleton.h"

class CSettings : public CSingleton<CSettings>
{
public:
    friend class CSingleton<CSettings>;
    UINT GetProfileUint(LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT nDefault);
    BOOL WriteProfileUint(LPCTSTR lpszSection, LPCTSTR lpszEntry, UINT nValue);
    INT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nDefault);
    BOOL WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, INT nValue);
    BYTE GetProfileByte(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE nDefault);
    BOOL WriteProfileByte(LPCTSTR lpszSection, LPCTSTR lpszEntry, BYTE nValue);
    CString GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault);
    BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);
private:
    CSettings(void);
    ~CSettings(void);
};
