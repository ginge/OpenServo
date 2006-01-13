#pragma once


// COutputWnd

class COutputWnd : public CEdit
{
	DECLARE_DYNAMIC(COutputWnd)

public:
	COutputWnd();
	virtual ~COutputWnd();

	BOOL Write(LPCTSTR line);

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
};


