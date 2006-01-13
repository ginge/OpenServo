// ServoWorkbenchView.h : interface of the CServoWorkbenchView class
//


#pragma once

#include "CCrystalEditView.h"

class CServoWorkbenchDoc;

class CServoWorkbenchView : public CCrystalEditView
{
protected: // create from serialization only
	CServoWorkbenchView();
	DECLARE_DYNCREATE(CServoWorkbenchView)

// Attributes
public:
	CServoWorkbenchDoc* GetDocument() const;

	virtual CCrystalTextBuffer *LocateTextBuffer(); 
	virtual DWORD ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);


// Implementation
public:
	virtual ~CServoWorkbenchView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ServoWorkbenchView.cpp
inline CServoWorkbenchDoc* CServoWorkbenchView::GetDocument() const
   { return reinterpret_cast<CServoWorkbenchDoc*>(m_pDocument); }
#endif

