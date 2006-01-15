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
#include "MainFrm.h"
#include "ChildFrm.h"
#include "DataView.h"
#include "TwiBootProg.h"
#include "TwiBootProgDoc.h"
#include "I2cBridge/u2c_common_func.h"
#include "I2cBridge/StaticUrl.h"
#include "afxwin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CTwiBootProgApp

BEGIN_MESSAGE_MAP(CTwiBootProgApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CTwiBootProgApp construction

CTwiBootProgApp::CTwiBootProgApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

// The one and only CTwiBootProgApp object

CTwiBootProgApp theApp;

// CTwiBootProgApp initialization

BOOL CTwiBootProgApp::InitInstance()
{
    // InitCommonControls() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    InitCommonControls();

    CWinApp::InitInstance();

    // Initialize OLE libraries
    if (!AfxOleInit())
    {
        AfxMessageBox(IDP_OLE_INIT_FAILED);
        return FALSE;
    }
    AfxEnableControlContainer();
    SetRegistryKey(_T("Dimax\\U2C-12"));
    LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
    // Register the application's document templates.  Document templates
    //  serve as the connection between documents, frame windows and views
    CMultiDocTemplate* pDocTemplate;
    pDocTemplate = new CMultiDocTemplate(IDR_TwiBootProgTYPE,
        RUNTIME_CLASS(CTwiBootProgDoc),
        RUNTIME_CLASS(CChildFrame), // custom MDI child frame
        RUNTIME_CLASS(CDataView));
    if (!pDocTemplate)
        return FALSE;
    AddDocTemplate(pDocTemplate);
    // create main MDI Frame window
    CMainFrame* pMainFrame = new CMainFrame;
    if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
        return FALSE;
    m_pMainWnd = pMainFrame;
    // call DragAcceptFiles only if there's a suffix
    //  In an MDI app, this should occur immediately after setting m_pMainWnd
    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);
    // Dispatch commands specified on the command line.  Will return FALSE if
    // app was launched with /RegServer, /Register, /Unregserver or /Unregister.
//  if (!ProcessShellCommand(cmdInfo))
//      return FALSE;
    // The main window has been initialized, so show and update it
    pMainFrame->ShowWindow(m_nCmdShow);
    pMainFrame->UpdateWindow();
    return TRUE;
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
    CStaticUrl m_Link;
    virtual BOOL OnInitDialog();
    CStatic m_Logo;
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_A_LINK, m_Link);
    DDX_Control(pDX, IDC_LOGO, m_Logo);
}

BOOL CAboutDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CRect LogoRect, DialogRect;
    m_Logo.GetWindowRect(LogoRect);
    ScreenToClient(LogoRect);
    GetClientRect(DialogRect);
    int LogoWidth = LogoRect.Width();
    int LogoStart = (DialogRect.Width() - LogoWidth)/2;
    LogoRect.left = LogoStart;
    LogoRect.right = LogoStart + LogoWidth;
    m_Logo.MoveWindow(LogoRect);
    // TODO:  Add extra initialization here

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CTwiBootProgApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

// CTwiBootProgApp message handlers

void CAboutDlg::OnLvnItemchangedList1(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
//  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    // TODO: Add your control notification handler code here
    *pResult = 0;
}

