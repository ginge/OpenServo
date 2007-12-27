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
#include <afxpriv.h>

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    ON_WM_CREATE()
    ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
    ON_WM_CLOSE()
    ON_UPDATE_COMMAND_UI(ID_DEVICE_ERASE, OnUpdateDeviceOperations)
    ON_UPDATE_COMMAND_UI(ID_DEVICE_PROGRAM, OnUpdateDeviceOperations)
    ON_UPDATE_COMMAND_UI(ID_DEVICE_VERIFY, OnUpdateDeviceOperations)
    ON_UPDATE_COMMAND_UI(ID_DEVICE_READ, OnUpdateDeviceOperations)
    ON_UPDATE_COMMAND_UI(ID_DEVICE_CONFIG, OnUpdateDeviceOperations)
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
: m_bShowProgress(false), m_StatusString(AFX_IDS_IDLEMESSAGE)
{
    // TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_DisplayModeBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_DisplayModeBar.LoadToolBar(IDR_DISPLAY_MODE))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }
    for (UINT nBut = ID_FIRST_MODE; nBut <= ID_LAST_MODE; nBut++)
        m_DisplayModeBar.SetButtonStyle(m_DisplayModeBar.CommandToIndex(nBut), TBBS_GROUP | TBSTYLE_CHECK);
    m_DisplayModeBar.SetButtonStyle(m_DisplayModeBar.CommandToIndex(ID_FIRST_MODE), TBBS_CHECKGROUP);

    if (!m_DeviceBar.Create(this))
    {
        TRACE0("Failed to create device bar\n");
        return -1;
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }
    // TODO: Delete these three lines if you don't want the toolbar to be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    m_DisplayModeBar.EnableDocking(CBRS_ALIGN_ANY);
    m_DeviceBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    RecalcLayout();
    DockControlBar(&m_wndToolBar);
    CRect rect;
    m_wndToolBar.GetWindowRect(&rect);
    DockControlBar(&m_DisplayModeBar, UINT(0), &rect);
    DockControlBar(&m_DeviceBar);

    m_ProgressCtrl.Create(WS_CHILD, CRect(0,0,0,0), &m_wndStatusBar, 5);
    m_ProgressCtrl.ShowWindow(SW_HIDE);
    m_ProgressCtrl.SetRange(0, 100);
    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return TRUE;
}

LRESULT CMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
    if ((m_bShowProgress) || (wParam == AFX_IDS_IDLEMESSAGE))
        return CFrameWnd::OnSetMessageString(m_StatusString, 0);
    else
        return CFrameWnd::OnSetMessageString(wParam, lParam);
}

void CMainFrame::ShowProgress(bool bShow)
{
    m_bShowProgress = bShow;
    if (bShow)
    {
        CClientDC dc(&m_wndStatusBar);
        CFont *pOldFont = dc.SelectObject(m_wndStatusBar.GetFont());
        CString Str;
        Str.LoadString(IDS_STATUS_PROGRAMMING);
        Str+=" ";
        CSize TextSize = dc.GetTextExtent(Str);
        dc.SelectObject(pOldFont);

        CRect rect;
        m_wndStatusBar.GetItemRect(0, &rect);
        rect.left = rect.left + TextSize.cx;
        m_ProgressCtrl.MoveWindow(&rect);
        m_ProgressCtrl.ShowWindow(SW_SHOW);
    }
    else
    {
        m_bShowProgress = false;
        m_ProgressCtrl.ShowWindow(SW_HIDE);
    }
}

void CMainFrame::SetStatusString(UINT StatusString)
{
    m_StatusString = StatusString;
    OnSetMessageString(StatusString, 0);
}

void CMainFrame::SetProgress(UINT StatusString, int Progress)
{
    m_ProgressCtrl.SetPos(Progress);
    SetStatusString(StatusString);
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

// CMainFrame message handlers
void CMainFrame::OnClose()
{
//  SaveBarState("Bars\\");
    CMDIFrameWnd::OnClose();
}

void CMainFrame::OnUpdateDeviceOperations(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
    if((m_bShowProgress) || (this == GetActiveFrame()))
        pCmdUI->Enable(FALSE);
    else
        pCmdUI->Enable(TRUE);

}
