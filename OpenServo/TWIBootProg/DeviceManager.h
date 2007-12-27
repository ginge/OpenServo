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
#include "Device.h"

//class CDevice;
class CDeviceFactory;

class CDeviceManager :public CSingleton<CDeviceManager>
{
public:
    friend CSingleton<CDeviceManager>;
    CDeviceManager(void);
    virtual ~CDeviceManager(void);
    void RegisterDevice(CDeviceFactory *pFactory);
    TStringList GetDeviceTypes();
    TStringList GetDeviceSubTypes();
    bool SetDeviceType(std::string DevType);
    bool SetDeviceSubtype(std::string DevSubType);

    bool Read(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Program(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Verify(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Erase(HWND Feedback);
    bool Configure();
private:
    typedef std::list<CDevice*> TDevices;
    TDevices m_Devices;
    CDevice* m_pActiveDevice;
};
