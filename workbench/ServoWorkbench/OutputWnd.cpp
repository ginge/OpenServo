// OutputWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ServoWorkbench.h"
#include "OutputWnd.h"
#include ".\outputwnd.h"


// COutputWnd

IMPLEMENT_DYNAMIC(COutputWnd, CEdit)
COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}


BEGIN_MESSAGE_MAP(COutputWnd, CEdit)
	ON_WM_ERASEBKGND()
//	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// COutputWnd message handlers


BOOL COutputWnd::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	dwStyle |= ES_AUTOVSCROLL | ES_MULTILINE | ES_LEFT;
	dwStyle &= ~(ES_CENTER);
	return CEdit::Create(dwStyle, rect, pParentWnd, nID);
}

BOOL COutputWnd::Write(LPCTSTR line)
{
	ReplaceSel(line, FALSE);
	return TRUE;
}

BOOL COutputWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect bounds;
	GetClientRect(bounds);
	pDC->FillSolidRect(bounds, 0x00ffffff);
	return TRUE;
	//return CEdit::OnEraseBkgnd(pDC);
}

HBRUSH COutputWnd::CtlColor(CDC* pDC, UINT nCtlColor)
{
	return NULL;
}
