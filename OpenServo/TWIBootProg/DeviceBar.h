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

// CDeviceBar
const char* const regDeviceBarSection = "DeviceBar";
const char* const regDeviceType = "DeviceType";
const char* const regDeviceSubtype = "DeviceSubtype";

class CDeviceBar : public CToolBar
{
    DECLARE_DYNAMIC(CDeviceBar)

public:
    CDeviceBar();
    virtual ~CDeviceBar();
    BOOL Create(CWnd *pWnd);
private:
    void ResetDeviceTypes();
    void ResetDeviceSubTypes();
    CComboBox m_DeviceType;
    CComboBox m_DeviceSubType;
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnChangeDeviceType();
    afx_msg void OnChangeDeviceSubtype();
};


