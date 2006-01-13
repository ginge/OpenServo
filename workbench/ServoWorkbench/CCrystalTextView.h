////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextView.h
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Interface of the CCrystalTextView class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use, distribute or modify this code
//	but LEAVE THIS HEADER INTACT.
////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CCRYSTALTEXTVIEW_H__AD7F2F41_6CB3_11D2_8C32_0080ADB86836__INCLUDED_)
#define AFX_CCRYSTALTEXTVIEW_H__AD7F2F41_6CB3_11D2_8C32_0080ADB86836__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//	Edit classes declspec
#ifndef CRYSEDIT_CLASS_DECL
#define CRYSEDIT_CLASS_DECL
#endif


////////////////////////////////////////////////////////////////////////////
// Forward class declarations

class CCrystalTextBuffer;
class CUpdateContext;


////////////////////////////////////////////////////////////////////////////
// CCrystalTextView class declaration

//	CCrystalTextView::FindText() flags
enum
{
	FIND_MATCH_CASE		= 0x0001,
	FIND_WHOLE_WORD		= 0x0002,
	FIND_DIRECTION_UP	= 0x0010,
	REPLACE_SELECTION	= 0x0100,
};

//	CCrystalTextView::UpdateView() flags
enum
{
	UPDATE_HORZRANGE	= 0x0001,		//	update horz scrollbar
	UPDATE_VERTRANGE	= 0x0002,		//	update vert scrollbar
	UPDATE_SINGLELINE	= 0x0100,		//	single line has changed
	UPDATE_FLAGSONLY	= 0x0200,		//	only line-flags were changed

	UPDATE_RESET		= 0x1000,		//	document was reloaded, update all!
};

class CRYSEDIT_CLASS_DECL CCrystalTextView : public CView
{
	DECLARE_DYNCREATE(CCrystalTextView)

private:
	//	Search parameters
	BOOL m_bLastSearch;
	DWORD m_dwLastSearchFlags;
	LPTSTR m_pszLastFindWhat;

	BOOL m_bCursorHidden;

	//	Painting caching bitmap
	CBitmap *m_pCacheBitmap;

	//	Line/character dimensions
	int m_nLineHeight, m_nCharWidth;
	void CalcLineCharDim();

	//	Text attributes
	int m_nTabSize;
	BOOL m_bViewTabs;
	BOOL m_bSelMargin;

	//	Amount of lines/characters that completely fits the client area
	int m_nScreenLines, m_nScreenChars;

	int m_nMaxLineLength;
	int m_nIdealCharPos;

	BOOL m_bFocused;
	CPoint m_ptAnchor;
	LOGFONT m_lfBaseFont;
	CFont *m_apFonts[4];

	//	Parsing stuff
	DWORD *m_pdwParseCookies;
	int m_nParseArraySize;
	DWORD GetParseCookie(int nLineIndex);

	//	Pre-calculated line lengths (in characters)
	int m_nActualLengthArraySize;
	int *m_pnActualLineLength;

	BOOL m_bPreparingToDrag;
	BOOL m_bDraggingText;
	BOOL m_bDragSelection, m_bWordSelection, m_bLineSelection;
	UINT m_nDragSelTimer;
	CPoint WordToRight(CPoint pt);
	CPoint WordToLeft(CPoint pt);

	CPoint m_ptDrawSelStart, m_ptDrawSelEnd;
	CPoint m_ptCursorPos;
	CPoint m_ptSelStart, m_ptSelEnd;
	void PrepareSelBounds();

	//	Helper functions
	void ExpandChars(LPCTSTR pszChars, int nOffset, int nCount, CString &line);

	int ApproxActualOffset(int nLineIndex, int nOffset);
	void AdjustTextPoint(CPoint &point);
	void DrawLineHelperImpl(CDC *pdc, CPoint &ptOrigin, const CRect &rcClip,
							LPCTSTR pszChars, int nOffset, int nCount);
	BOOL IsInsideSelBlock(CPoint ptTextPos);

protected: // create from serialization only
	CImageList *m_pIcons;
	CCrystalTextBuffer *m_pTextBuffer;
	HACCEL m_hAccel;
	BOOL m_bVertScrollBarLocked, m_bHorzScrollBarLocked;
	CPoint m_ptDraggedTextBegin, m_ptDraggedTextEnd;
	void ResetView();
	void UpdateCaret();
	void SetAnchor(const CPoint &ptNewAnchor);
	int GetMarginWidth();

	BOOL m_bShowInactiveSelection;

	//
	//	[JRT]:
	//
	bool	m_bDisableDragAndDrop;

	CPoint ClientToText(const CPoint &point);
	CPoint TextToClient(const CPoint &point);
	void InvalidateLines(int nLine1, int nLine2, BOOL bInvalidateMargin = FALSE);
	int CalculateActualOffset(int nLineIndex, int nCharIndex);

	//	Printing
	int m_nPrintPages;
	int *m_pnPages;
	CFont *m_pPrintFont;
	int m_nPrintLineHeight;
	BOOL m_bPrintHeader, m_bPrintFooter;
	CRect m_ptPageArea, m_rcPrintArea;
	int PrintLineHeight(CDC *pdc, int nLine);
	void RecalcPageLayouts(CDC *pdc, CPrintInfo *pInfo);
	virtual void PrintHeader(CDC *pdc, int nPageNum);
	virtual void PrintFooter(CDC *pdc, int nPageNum);
	virtual void GetPrintHeaderText(int nPageNum, CString &text);
	virtual void GetPrintFooterText(int nPageNum, CString &text);

	//	Keyboard handlers
	void MoveLeft(BOOL bSelect);
	void MoveRight(BOOL bSelect);
	void MoveWordLeft(BOOL bSelect);
	void MoveWordRight(BOOL bSelect);
	void MoveUp(BOOL bSelect);
	void MoveDown(BOOL bSelect);
	void MoveHome(BOOL bSelect);
	void MoveEnd(BOOL bSelect);
	void MovePgUp(BOOL bSelect);
	void MovePgDn(BOOL bSelect);
	void MoveCtrlHome(BOOL bSelect);
	void MoveCtrlEnd(BOOL bSelect);

	void SelectAll();
	void Copy();

	BOOL IsSelection();
	BOOL IsInsideSelection(const CPoint &ptTextPos);
	void GetSelection(CPoint &ptStart, CPoint &ptEnd);
	void SetSelection(const CPoint &ptStart, const CPoint &ptEnd);

	int m_nTopLine, m_nOffsetChar;

	int GetLineHeight();
	int GetCharWidth();
	int GetMaxLineLength();
	int GetScreenLines();
	int GetScreenChars();
	CFont *GetFont(BOOL bItalic = FALSE, BOOL bBold = FALSE);

	void RecalcVertScrollBar(BOOL bPositionOnly = FALSE);
	void RecalcHorzScrollBar(BOOL bPositionOnly = FALSE);

	//	Splitter support
	virtual void UpdateSiblingScrollPos(BOOL bHorz);
	virtual void OnUpdateSibling(CCrystalTextView *pUpdateSource, BOOL bHorz);
	CCrystalTextView *GetSiblingView(int nRow, int nCol);

	virtual int GetLineCount();
	virtual int GetLineLength(int nLineIndex);
	virtual int GetLineActualLength(int nLineIndex);
	virtual LPCTSTR GetLineChars(int nLineIndex);
	virtual DWORD GetLineFlags(int nLineIndex);
	virtual void GetText(const CPoint &ptStart, const CPoint &ptEnd, CString &text);

	//	Clipboard overridable
	virtual BOOL TextInClipboard();
	virtual BOOL PutToClipboard(LPCTSTR pszText);
	virtual BOOL GetFromClipboard(CString &text);

	//	Drag-n-drop overrideable
	virtual HGLOBAL PrepareDragData();
	virtual DROPEFFECT GetDropEffect();
	virtual void OnDropSource(DROPEFFECT de);
	BOOL IsDraggingText() const { return m_bDraggingText; };

	virtual COLORREF GetColor(int nColorIndex);
	virtual void GetLineColors(int nLineIndex, COLORREF &crBkgnd,
					COLORREF &crText, BOOL &bDrawWhitespace);
	virtual BOOL GetItalic(int nColorIndex);
	virtual BOOL GetBold(int nColorIndex);

	void DrawLineHelper(CDC *pdc, CPoint &ptOrigin, const CRect &rcClip, int nColorIndex,
						LPCTSTR pszChars, int nOffset, int nCount, CPoint ptTextPos);
	virtual void DrawSingleLine(CDC *pdc, const CRect &rect, int nLineIndex);
	virtual void DrawMargin(CDC *pdc, const CRect &rect, int nLineIndex);

	//	Syntax coloring overrides
	struct TEXTBLOCK
	{
		int	m_nCharPos;
		int m_nColorIndex;
	};

	virtual DWORD ParseLine(DWORD dwCookie, int nLineIndex, TEXTBLOCK *pBuf, int &nActualItems);
	virtual HINSTANCE GetResourceHandle();

// Attributes
public:
	BOOL GetViewTabs();
	void SetViewTabs(BOOL bViewTabs);
	int GetTabSize();
	void SetTabSize(int nTabSize);
	BOOL GetSelectionMargin();
	void SetSelectionMargin(BOOL bSelMargin);
	void GetFont(LOGFONT &lf);
	void SetFont(const LOGFONT &lf);
	
	enum
	{
		//	Base colors
		COLORINDEX_WHITESPACE,
		COLORINDEX_BKGND,
		COLORINDEX_NORMALTEXT,
		COLORINDEX_SELMARGIN,
		COLORINDEX_SELBKGND,
		COLORINDEX_SELTEXT,
		//	Syntax colors
		COLORINDEX_KEYWORD,
		COLORINDEX_COMMENT,
		COLORINDEX_NUMBER,
		COLORINDEX_OPERATOR,
		COLORINDEX_STRING,
		COLORINDEX_PREPROCESSOR,
		//	Compiler/debugger colors
		COLORINDEX_ERRORBKGND,
		COLORINDEX_ERRORTEXT,
		COLORINDEX_EXECUTIONBKGND,
		COLORINDEX_EXECUTIONTEXT,
		COLORINDEX_BREAKPOINTBKGND,
		COLORINDEX_BREAKPOINTTEXT,
		//	...
		//	Expandable: custom elements are allowed.
	};

	//
	//	[JRT]:
	//
	bool	GetDisableDragAndDrop( void );
	void	SetDisableDragAndDrop( bool bDDAD );

	// Operations
public:
	void AttachToBuffer(CCrystalTextBuffer *pBuf = NULL);
	void DetachFromBuffer();

	//	Buffer-view interaction, multiple views
	virtual CCrystalTextBuffer *LocateTextBuffer();
	virtual void UpdateView(CCrystalTextView *pSource, CUpdateContext *pContext, DWORD dwFlags, int nLineIndex = -1);

	//	Attributes
	CPoint GetCursorPos();
	void SetCursorPos(const CPoint &ptCursorPos);
	void ShowCursor();
	void HideCursor();

	//	Operations
	void EnsureVisible(CPoint pt);

	//	Text search helpers
	BOOL FindText(LPCTSTR pszText, const CPoint &ptStartPos, DWORD dwFlags, BOOL bWrapSearch, CPoint *pptFoundPos);
	BOOL FindTextInBlock(LPCTSTR pszText, const CPoint &ptStartPos, const CPoint &ptBlockBegin, const CPoint &ptBlockEnd,
						DWORD dwFlags, BOOL bWrapSearch, CPoint *pptFoundPos);
	BOOL HighlightText(const CPoint &ptStartPos, int nLength);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCrystalTextView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = NULL);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	CCrystalTextView();
	~CCrystalTextView();

protected:

// Generated message map functions
protected:
#ifdef _DEBUG
	void AssertValidTextPos(const CPoint &pt);
#endif

	//{{AFX_MSG(CCrystalTextView)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnEditFind();
	afx_msg void OnEditRepeat();
	afx_msg void OnUpdateEditRepeat(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnFilePageSetup();

	afx_msg void OnCharLeft();
	afx_msg void OnExtCharLeft();
	afx_msg void OnCharRight();
	afx_msg void OnExtCharRight();
	afx_msg void OnWordLeft();
	afx_msg void OnExtWordLeft();
	afx_msg void OnWordRight();
	afx_msg void OnExtWordRight();
	afx_msg void OnLineUp();
	afx_msg void OnExtLineUp();
	afx_msg void OnLineDown();
	afx_msg void OnExtLineDown();
	afx_msg void OnPageUp();
	afx_msg void OnExtPageUp();
	afx_msg void OnPageDown();
	afx_msg void OnExtPageDown();
	afx_msg void OnLineEnd();
	afx_msg void OnExtLineEnd();
	afx_msg void OnHome();
	afx_msg void OnExtHome();
	afx_msg void OnTextBegin();
	afx_msg void OnExtTextBegin();
	afx_msg void OnTextEnd();
	afx_msg void OnExtTextEnd();
	afx_msg void OnUpdateIndicatorCRLF(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorPosition(CCmdUI* pCmdUI);
	afx_msg void OnToggleBookmark(UINT nCmdID);
	afx_msg void OnGoBookmark(UINT nCmdID);
	afx_msg void OnClearBookmarks();

	afx_msg void ScrollUp();
	afx_msg void ScrollDown();
	afx_msg void ScrollLeft();
	afx_msg void ScrollRight();
	DECLARE_MESSAGE_MAP()
};

#ifdef _DEBUG
#define ASSERT_VALIDTEXTPOS(pt)		AssertValidTextPos(pt);
#else
#define ASSERT_VALIDTEXTPOS(pt)
#endif

inline BOOL CCrystalTextView::GetViewTabs()
{
	return m_bViewTabs;
}

inline void CCrystalTextView::SetViewTabs(BOOL bViewTabs)
{
	if (bViewTabs != m_bViewTabs)
	{
		m_bViewTabs = bViewTabs;
		if (::IsWindow(m_hWnd))
			Invalidate();
	}
}

inline BOOL CCrystalTextView::GetSelectionMargin()
{
	return m_bSelMargin;
}

inline int CCrystalTextView::GetMarginWidth()
{
	return m_bSelMargin ? 20 : 1;
}

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CCRYSTALTEXTVIEW_H__AD7F2F41_6CB3_11D2_8C32_0080ADB86836__INCLUDED_)
