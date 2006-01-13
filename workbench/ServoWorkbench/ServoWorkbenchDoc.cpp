// ServoWorkbenchDoc.cpp : implementation of the CServoWorkbenchDoc class
//

#include "stdafx.h"
#include "ServoWorkbench.h"
#include "MainFrm.h"

#include "ServoWorkbenchDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


inline CMainFrame* GetMainFrame() {
	CMainFrame* pmainframe = (CMainFrame*)AfxGetMainWnd();
	ASSERT_KINDOF( CMainFrame, pmainframe);
	return pmainframe;
}


// CServoWorkbenchDoc

IMPLEMENT_DYNCREATE(CServoWorkbenchDoc, CDocument)

BEGIN_MESSAGE_MAP(CServoWorkbenchDoc, CDocument)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CServoWorkbenchDoc, CDocument)
END_DISPATCH_MAP()

// Note: we add support for IID_IServoWorkbench to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {7C314D49-8E58-4DC4-83D5-802FEB298443}
static const IID IID_IServoWorkbench =
{ 0x7C314D49, 0x8E58, 0x4DC4, { 0x83, 0xD5, 0x80, 0x2F, 0xEB, 0x29, 0x84, 0x43 } };

BEGIN_INTERFACE_MAP(CServoWorkbenchDoc, CDocument)
	INTERFACE_PART(CServoWorkbenchDoc, IID_IServoWorkbench, Dispatch)
END_INTERFACE_MAP()


// CServoWorkbenchDoc construction/destruction

CServoWorkbenchDoc::CServoWorkbenchDoc()
{
	EnableAutomation();
	AfxOleLockApp();
}

CServoWorkbenchDoc::~CServoWorkbenchDoc()
{
	m_buffer.FreeAll();
	AfxOleUnlockApp();
}

IServoControllerPtr CServoWorkbenchDoc::GetController()
{
	CMainFrame* pmainframe = GetMainFrame();
	return pmainframe->m_pcontroller;
}

BOOL CServoWorkbenchDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_buffer.InitNew();

//	CMainFrame* pmainframe = GetMainFrame();
//	if( !pmainframe->isConnected() )
//		pmainframe->Connect();

	return TRUE;
}

BOOL CServoWorkbenchDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
//	CMainFrame* pmainframe = GetMainFrame();
//	if( !pmainframe->isConnected() )
//		pmainframe->Connect();

	m_buffer.LoadFromFile(lpszPathName);
	return TRUE;
}

BOOL CServoWorkbenchDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	m_buffer.SaveToFile( lpszPathName );
	SetModifiedFlag(FALSE);
	return TRUE;
}


// CServoWorkbenchDoc serialization

void CServoWorkbenchDoc::Serialize(CArchive& ar)
{
	ASSERT(FALSE);
}


// CServoWorkbenchDoc diagnostics

#ifdef _DEBUG
void CServoWorkbenchDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CServoWorkbenchDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CServoWorkbenchDoc commands
CString CServoWorkbenchDoc::GetCode() {
	CString m_strCode;
	m_strCode.Empty();

	int nlines = m_buffer.GetLineCount();
	m_buffer.GetText(0, 0, nlines-1, m_buffer.GetLineLength(nlines-1), m_strCode);
	
	return m_strCode;
}
