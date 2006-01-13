// ServoWorkbench.h : main header file for the ServoWorkbench application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CServoWorkbenchApp:
// See ServoWorkbench.cpp for the implementation of this class
//

class CServoWorkbenchApp : public CWinApp
{
public:
	CServoWorkbenchApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};

extern CServoWorkbenchApp theApp;