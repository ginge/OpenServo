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
#include "Device.h"
#include "DeviceManager.h"
#include "DeviceFactory.h"

CDeviceManager::CDeviceManager(void)
:m_pActiveDevice(NULL)
{
}

CDeviceManager::~CDeviceManager(void)
{
    TDevices::iterator iter;
    for(iter = m_Devices.begin(); iter != m_Devices.end(); iter++)
    {
        delete (*iter);
    }
    m_Devices.clear();
}

void CDeviceManager::RegisterDevice(CDeviceFactory *pFactory)
{
    CDevice* pDevice = pFactory->Create();
    m_Devices.push_back(pDevice);
}

TStringList CDeviceManager::GetDeviceTypes()
{
    TStringList Res;
    TDevices::iterator iter;
    for(iter = m_Devices.begin(); iter != m_Devices.end(); iter++)
    {
        Res.push_back((*iter)->GetType());
    }
    Res.sort();
    return Res;
}

bool CDeviceManager::SetDeviceType(std::string DevType)
{
    TDevices::iterator iter;
    for(iter = m_Devices.begin(); iter != m_Devices.end(); iter++)
    {
        if ((*iter)->GetType() == DevType)
        {
            m_pActiveDevice = (*iter);
            return true;
        }
    }
    return false;
}

bool CDeviceManager::SetDeviceSubtype(std::string DevSubType)
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->SetSubtype(DevSubType);
}

TStringList CDeviceManager::GetDeviceSubTypes()
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        TStringList empty;
        return empty;
    }
    return m_pActiveDevice->GetSubtypes();
}

bool CDeviceManager::Read(BYTE *pBuffer, UINT Length, HWND Feedback)
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->Read(pBuffer, Length, Feedback);
}

bool CDeviceManager::Program(BYTE *pBuffer, UINT Length, HWND Feedback)
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->Program(pBuffer, Length, Feedback);
}

bool CDeviceManager::Verify(BYTE *pBuffer, UINT Length, HWND Feedback)
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->Verify(pBuffer, Length, Feedback);
}

bool CDeviceManager::Erase(HWND Feedback)
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->Erase(Feedback);
}

bool CDeviceManager::Configure()
{
    if (!m_pActiveDevice)
    {
        ASSERT(FALSE);
        return false;
    }
    return m_pActiveDevice->Configure();
}