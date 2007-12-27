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

#include "stdafx.h"
#include "TwiBootProg.h"
#include "DeviceBar.h"
#include "DeviceManager.h"

// CDeviceBar

IMPLEMENT_DYNAMIC(CDeviceBar, CToolBar)
CDeviceBar::CDeviceBar()
{
}

CDeviceBar::~CDeviceBar()
{
}

BOOL CDeviceBar::Create(CWnd *pWnd)
{
    if (!CreateEx(pWnd, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !LoadToolBar(IDR_DEVICEBAR))
    {
        TRACE0("Failed to create toolbar\n");
        return FALSE;      // fail to create
    }

    CRect rect;
    int nIndex;
    nIndex = GetToolBarCtrl().CommandToIndex(ID_DEVICE_TYPE);
    SetButtonInfo(nIndex, ID_DEVICE_TYPE, TBBS_SEPARATOR, 205);
    GetToolBarCtrl().GetItemRect(nIndex, &rect);
    rect.top = 1;
    rect.bottom = rect.top + 250;
    if (!m_DeviceType.Create(CBS_DROPDOWNLIST | WS_VISIBLE |
        WS_TABSTOP | WS_VSCROLL, rect, this, ID_DEVICE_TYPE))
    {
        TRACE("Failed to create combo-box\n");
        return FALSE;
    }
    nIndex = GetToolBarCtrl().CommandToIndex(ID_DEVICE_SUBTYPE);
    SetButtonInfo(nIndex, ID_DEVICE_SUBTYPE, TBBS_SEPARATOR, 205);
    GetToolBarCtrl().GetItemRect(nIndex,&rect);
    rect.top = 1;
    rect.bottom = rect.top + 250;
    if (!m_DeviceSubType.Create(CBS_DROPDOWNLIST | WS_VISIBLE |
        WS_TABSTOP | WS_VSCROLL, rect, this, ID_DEVICE_SUBTYPE))
    {
        TRACE("Failed to create combo-box\n");
        return FALSE;
    }
    ResetDeviceTypes();
    ResetDeviceSubTypes();
    return TRUE;
}


BEGIN_MESSAGE_MAP(CDeviceBar, CToolBar)
    ON_CBN_SELCHANGE(ID_DEVICE_TYPE, OnChangeDeviceType)
    ON_CBN_SELCHANGE(ID_DEVICE_SUBTYPE, OnChangeDeviceSubtype)
END_MESSAGE_MAP()

// CDeviceBar message handlers

void CDeviceBar::OnChangeDeviceType()
{
    ResetDeviceSubTypes();
}

void CDeviceBar::OnChangeDeviceSubtype()
{
    CString DevSubtype;
    m_DeviceSubType.GetWindowText(DevSubtype);
    CDeviceManager *pMan = CDeviceManager::Instance();
    if (!pMan->SetDeviceSubtype((LPCTSTR)DevSubtype))
    {
        ASSERT(FALSE);
    }
}

void CDeviceBar::ResetDeviceTypes()
{
    m_DeviceType.ResetContent();
    CDeviceManager *pMan = CDeviceManager::Instance();
    TStringList Types = pMan->GetDeviceTypes();
    TStringList::iterator iter;
    for(iter = Types.begin(); iter != Types.end(); iter++)
        m_DeviceType.AddString(iter->c_str());
    if (m_DeviceType.GetCount() > 0)
    {
        m_DeviceType.SetCurSel(0);
        OnChangeDeviceType();
    }

}

void CDeviceBar::ResetDeviceSubTypes()
{
    CString DevType;
    m_DeviceType.GetWindowText(DevType);
    m_DeviceSubType.ResetContent();
    CDeviceManager *pMan = CDeviceManager::Instance();
    if (pMan->SetDeviceType((LPCTSTR)DevType))
    {
        TStringList SubTypes = pMan->GetDeviceSubTypes();
        TStringList::iterator iter;
        for(iter = SubTypes.begin(); iter != SubTypes.end(); iter++)
            m_DeviceSubType.AddString(iter->c_str());
        if (m_DeviceSubType.GetCount() > 0)
        {
            m_DeviceSubType.SetCurSel(0);
            OnChangeDeviceSubtype();
        }
    }
}
