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
#include "StaticUrl.h"


// CStaticUrl

IMPLEMENT_DYNAMIC(CStaticUrl, CStatic)
CStaticUrl::CStaticUrl()
: m_bVisited(false), m_hCursor(NULL)
{
}
CStaticUrl::CStaticUrl(CString Link)
: m_bVisited(false), m_hCursor(NULL), m_Link(Link)
{
}

CStaticUrl::~CStaticUrl()
{
}


BEGIN_MESSAGE_MAP(CStaticUrl, CStatic)
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()



// CStaticUrl message handlers
HBRUSH CStaticUrl::CtlColor(CDC* pDC, UINT)
{
	if (!(HFONT)m_Font) 
	{
		LOGFONT lf;
		GetFont()->GetObject(sizeof(lf), &lf);
		lf.lfUnderline = TRUE;
		m_Font.CreateFontIndirect(&lf);
	}

	// use underline font and visited/unvisited colors
	pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&m_Font);
	if (m_bVisited)
		pDC->SetTextColor(RGB(128,0,128));
	else
		pDC->SetTextColor(RGB(0,0,255));

	return (HBRUSH)::GetStockObject(NULL_BRUSH);
}

void CStaticUrl::OnLButtonDown(UINT, CPoint)
{
	CString strUrl;
	if (m_Link.GetLength() == 0)
		GetWindowText(strUrl);
	else
		strUrl = m_Link;
	ShellExecute(0, "open", strUrl, 0, 0, SW_SHOWNORMAL);
	m_bVisited = true;
	Invalidate();
}

BOOL CStaticUrl::OnSetCursor(CWnd*, UINT, UINT)
{
	if (m_hCursor == NULL)
	{
		
		TCHAR WinDir[MAX_PATH];
		GetWindowsDirectory(WinDir, MAX_PATH);
		_tcscat(WinDir, _T("\\winhlp32.exe"));
		HMODULE hModule = LoadLibrary(WinDir);
		if (hModule) {
			m_hCursor =
				CopyCursor(::LoadCursor(hModule, MAKEINTRESOURCE(106)));
		}
		FreeLibrary(hModule);
//		m_hCursor = theApp.LoadCursor(IDC_HAND);
	}
	if (m_hCursor)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	}
	else
		return FALSE;
}

