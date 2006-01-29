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


// CStaticUrl

class CStaticUrl : public CStatic
{
	DECLARE_DYNAMIC(CStaticUrl)

public:
	CStaticUrl();
	CStaticUrl(CString Link);
	virtual ~CStaticUrl();
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg UINT OnNcHitTest(CPoint){return HTCLIENT;};
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
private:
	COLORREF m_Color;
	HCURSOR m_hCursor;
	CFont m_Font;
	bool m_bVisited;
	CString m_Link;
};


