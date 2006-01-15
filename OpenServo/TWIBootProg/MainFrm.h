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

#include "DeviceBar.h"

class CMainFrame : public CMDIFrameWnd
{
    DECLARE_DYNAMIC(CMainFrame)
public:
    CMainFrame();

// Attributes
public:

// Operations
public:
    void SetProgress(UINT StatusString, int Progress);
    void ShowProgress(bool bShow);
    void SetStatusString(UINT StatusString);
// Overrides
public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    CStatusBar  m_wndStatusBar;
    CToolBar    m_wndToolBar;
    CToolBar    m_DisplayModeBar;
    CDeviceBar  m_DeviceBar;

    CProgressCtrl m_ProgressCtrl;
    bool m_bShowProgress;
    UINT m_StatusString;
// Generated message map functions
protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
    afx_msg void OnClose();
    afx_msg void OnUpdateDeviceOperations(CCmdUI *pCmdUI);
    DECLARE_MESSAGE_MAP()
public:

};


