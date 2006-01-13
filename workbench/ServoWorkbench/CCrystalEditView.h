////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalEditView.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Interface of the CCrystalEditView class, a part of Crystal Edit - syntax
//	coloring text editor.
//
//	You are free to use, distribute or modify this code
//	but LEAVE THIS HEADER INTACT.
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CCRYSTALEDITVIEW_H__8F3F8B63_6F66_11D2_8C34_0080ADB86836__INCLUDED_)
#define AFX_CCRYSTALEDITVIEW_H__8F3F8B63_6F66_11D2_8C34_0080ADB86836__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "CCrystalTextView.h"

#ifndef CRYSEDIT_CLASS_DECL
#define CRYSEDIT_CLASS_DECL
#endif


/////////////////////////////////////////////////////////////////////////////
//	Forward class declarations

class CEditDropTargetImpl;


/////////////////////////////////////////////////////////////////////////////
//	CCrystalEditView view

class CRYSEDIT_CLASS_DECL CCrystalEditView : public CCrystalTextView
{
	DECLARE_DYNCREATE(CCrystalEditView)

private:
	BOOL m_bOvrMode;
	BOOL m_bDropPosVisible;
	CPoint m_ptSavedCaretPos;
	CPoint m_ptDropPos;
	BOOL m_bSelectionPushed;
	CPoint m_ptSavedSelStart, m_ptSavedSelEnd;
	BOOL DeleteCurrentSelection();

	//
	//	[JRT] Disable Backspace At Start Of Line...
	//
	bool	m_bDisableBSAtSOL;								// Disable BS At Start Of Line
	bool	m_bDisableDragAndDrop;							// Disable Drag And Drop

protected:
	CEditDropTargetImpl *m_pDropTarget;
	virtual DROPEFFECT GetDropEffect();
	virtual void OnDropSource(DROPEFFECT de);
	void Paste();
	void Cut();

// Attributes
public:
	//
	//	[JRT]:
	//
	bool	GetDisableBSAtSOL( void )
	{
		return( m_bDisableBSAtSOL );
	}
// Operations
public:
	CCrystalEditView();
	~CCrystalEditView();

	BOOL GetOverwriteMode() const;
	void SetOverwriteMode(BOOL bOvrMode = TRUE);

	void ShowDropIndicator(const CPoint &point);
	void HideDropIndicator();

	BOOL DoDropText(COleDataObject *pDataObject, const CPoint &ptClient);
	void DoDragScroll(const CPoint &point);

	virtual BOOL QueryEditable();
	virtual void UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, DWORD dwFlags, int nLineIndex = -1);

	BOOL ReplaceSelection(LPCTSTR pszNewText);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrystalEditView)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CCrystalEditView)
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditDeleteBack();
	afx_msg void OnEditUntab();
	afx_msg void OnEditTab();
	afx_msg void OnEditSwitchOvrmode();
	afx_msg void OnUpdateEditSwitchOvrmode(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnEditReplace();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnEditDisableBSatSOL();
	afx_msg void OnUpdateEditDisableBSatSOL(CCmdUI* pCmdUI);
	afx_msg void OnEditDisableDragAndDrop();
	afx_msg void OnUpdateEditDisableDragAndDrop(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnUpdateIndicatorCol(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorOvr(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorRead(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

inline BOOL CCrystalEditView::GetOverwriteMode() const
{
	return m_bOvrMode;
}

inline void CCrystalEditView::SetOverwriteMode(BOOL bOvrMode /*= TRUE*/)
{
	m_bOvrMode = bOvrMode;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCRYSTALEDITVIEW_H__8F3F8B63_6F66_11D2_8C34_0080ADB86836__INCLUDED_)
