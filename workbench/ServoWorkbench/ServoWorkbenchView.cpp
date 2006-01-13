// ServoWorkbenchView.cpp : implementation of the CServoWorkbenchView class
//

#include "stdafx.h"
#include "ServoWorkbench.h"

#include "ServoWorkbenchDoc.h"
#include "ServoWorkbenchView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CServoWorkbenchView

IMPLEMENT_DYNCREATE(CServoWorkbenchView, CCrystalEditView)

BEGIN_MESSAGE_MAP(CServoWorkbenchView, CCrystalEditView)
END_MESSAGE_MAP()

// CServoWorkbenchView construction/destruction

CServoWorkbenchView::CServoWorkbenchView()
{
	// TODO: add construction code here

}

CServoWorkbenchView::~CServoWorkbenchView()
{
}

CCrystalTextBuffer* CServoWorkbenchView::LocateTextBuffer()
{ 
	return &GetDocument()->m_buffer; 
}

BOOL CServoWorkbenchView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CCrystalEditView::PreCreateWindow(cs);
}



// CServoWorkbenchView diagnostics

#ifdef _DEBUG
void CServoWorkbenchView::AssertValid() const
{
	CCrystalEditView::AssertValid();
}

void CServoWorkbenchView::Dump(CDumpContext& dc) const
{
	CCrystalEditView::Dump(dc);
}

CServoWorkbenchDoc* CServoWorkbenchView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CServoWorkbenchDoc)));
	return (CServoWorkbenchDoc*)m_pDocument;
}
#endif //_DEBUG


// CServoWorkbenchView message handlers
