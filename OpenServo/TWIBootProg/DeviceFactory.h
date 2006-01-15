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
#include "DeviceManager.h"

class CDevice;

class CDeviceFactory
{
public:
    virtual CDevice* Create() = 0;
};


template <class _T>
class CConcreteDeviceFactory: public CDeviceFactory
{
public:
    CConcreteDeviceFactory()
    {
        CDeviceManager *pManager = CDeviceManager::Instance();
        pManager->RegisterDevice(this);
    };
    ~CConcreteDeviceFactory() {};
    CDevice* Create(){return new _T;} ;
};
