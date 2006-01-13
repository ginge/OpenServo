// ServoWorkbenchDoc.h : interface of the CServoWorkbenchDoc class
//


#pragma once



#include "CCrystalTextBuffer.h"


class CServoWorkbenchDoc : public CDocument
{
protected: // create from serialization only
	CServoWorkbenchDoc();
	DECLARE_DYNCREATE(CServoWorkbenchDoc)

// Operations
public:
	IServoControllerPtr GetController();

// Attributes
public:
	CCrystalTextBuffer m_buffer;

	CString CServoWorkbenchDoc::GetCode();

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CServoWorkbenchDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:


// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};


