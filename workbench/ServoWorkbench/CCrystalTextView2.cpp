////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextView2.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CCrystalTextView class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use, distribute or modify this code
//	but LEAVE THIS HEADER INTACT.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CCrystalTextView.h"
#include "CCrystalTextBuffer.h"
#include <malloc.h>

#ifndef __AFXPRIV_H__
#pragma message("Include <afxpriv.h> in your stdafx.h to avoid this message")
#include <afxpriv.h>
#endif
#ifndef __AFXOLE_H__
#pragma message("Include <afxole.h> in your stdafx.h to avoid this message")
#include <afxole.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	CRYSTAL_TIMER_DRAGSEL	1001


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextView

void CCrystalTextView::MoveLeft(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		m_ptCursorPos = m_ptDrawSelStart;
	}
	else
	{
		if (m_ptCursorPos.x == 0)
		{
			if (m_ptCursorPos.y > 0)
			{
				m_ptCursorPos.y --;
				m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
			}
		}
		else
			m_ptCursorPos.x --;
	}
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveRight(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		m_ptCursorPos = m_ptDrawSelEnd;
	}
	else
	{
		if (m_ptCursorPos.x == GetLineLength(m_ptCursorPos.y))
		{
			if (m_ptCursorPos.y < GetLineCount() - 1)
			{
				m_ptCursorPos.y ++;
				m_ptCursorPos.x = 0;
			}
		}
		else
		{
			m_ptCursorPos.x ++;
		}
	}
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveWordLeft(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		MoveLeft(bSelect);
		return;
	}

	if (m_ptCursorPos.x == 0)
	{
		if (m_ptCursorPos.y == 0)
			return;
		m_ptCursorPos.y --;
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}

	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nPos = m_ptCursorPos.x;
	while (nPos > 0 && isspace(pszChars[nPos - 1]))
		nPos --;

	if (nPos > 0)
	{
		nPos --;
		if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
		{
			while (nPos > 0 && (isalnum(pszChars[nPos - 1]) || pszChars[nPos - 1] == _T('_')))
				nPos --;
		}
		else
		{
			while (nPos > 0 && ! isalnum(pszChars[nPos - 1])
						&& pszChars[nPos - 1] != _T('_') && ! isspace(pszChars[nPos - 1]))
				nPos --;
		}
	}

	m_ptCursorPos.x = nPos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveWordRight(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
	{
		MoveRight(bSelect);
		return;
	}

	if (m_ptCursorPos.x == GetLineLength(m_ptCursorPos.y))
	{
		if (m_ptCursorPos.y == GetLineCount() - 1)
			return;
		m_ptCursorPos.y ++;
		m_ptCursorPos.x = 0;
	}

	int nLength = GetLineLength(m_ptCursorPos.y);
	if (m_ptCursorPos.x == nLength)
	{
		MoveRight(bSelect);
		return;
	}

	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nPos = m_ptCursorPos.x;
	if (isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
	{
		while (nPos < nLength && isalnum(pszChars[nPos]) || pszChars[nPos] == _T('_'))
			nPos ++;
	}
	else
	{
		while (nPos < nLength && ! isalnum(pszChars[nPos])
						&& pszChars[nPos] != _T('_') && ! isspace(pszChars[nPos]))
			nPos ++;
	}

	while (nPos < nLength && isspace(pszChars[nPos]))
		nPos ++;

	m_ptCursorPos.x = nPos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveUp(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
		m_ptCursorPos = m_ptDrawSelStart;

	if (m_ptCursorPos.y > 0)
	{
		if (m_nIdealCharPos == -1)
			m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
		m_ptCursorPos.y --;
		m_ptCursorPos.x = ApproxActualOffset(m_ptCursorPos.y, m_nIdealCharPos);
		if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
			m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveDown(BOOL bSelect)
{
	PrepareSelBounds();
	if (m_ptDrawSelStart != m_ptDrawSelEnd && ! bSelect)
		m_ptCursorPos = m_ptDrawSelEnd;

	if (m_ptCursorPos.y < GetLineCount() - 1)
	{
		if (m_nIdealCharPos == -1)
			m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
		m_ptCursorPos.y ++;
		m_ptCursorPos.x = ApproxActualOffset(m_ptCursorPos.y, m_nIdealCharPos);
		if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
			m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	}
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveHome(BOOL bSelect)
{
	int nLength = GetLineLength(m_ptCursorPos.y);
	LPCTSTR pszChars = GetLineChars(m_ptCursorPos.y);
	int nHomePos = 0;
	while (nHomePos < nLength && isspace(pszChars[nHomePos]))
		nHomePos ++;
	if (nHomePos == nLength || m_ptCursorPos.x == nHomePos)
		m_ptCursorPos.x = 0;
	else
		m_ptCursorPos.x = nHomePos;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveEnd(BOOL bSelect)
{
	m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MovePgUp(BOOL bSelect)
{
	int nNewTopLine = m_nTopLine - GetScreenLines() + 1;
	if (nNewTopLine < 0)
		nNewTopLine = 0;
	if (m_nTopLine != nNewTopLine)
	{
		int nScrollLines = m_nTopLine - nNewTopLine;
		m_nTopLine = nNewTopLine;
		ScrollWindow(0, nScrollLines * GetLineHeight());
		UpdateWindow();
		RecalcVertScrollBar(TRUE);
		UpdateSiblingScrollPos(TRUE);
	}

	m_ptCursorPos.y -= GetScreenLines() - 1;
	if (m_ptCursorPos.y < 0)
		m_ptCursorPos.y = 0;
	if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);	//todo: no vertical scroll
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MovePgDn(BOOL bSelect)
{
	int nNewTopLine = m_nTopLine + GetScreenLines() - 1;
	if (nNewTopLine >= GetLineCount())
		nNewTopLine = GetLineCount() - 1;
	if (m_nTopLine != nNewTopLine)
	{
		int nScrollLines = m_nTopLine - nNewTopLine;
		m_nTopLine = nNewTopLine;
		ScrollWindow(0, nScrollLines * GetLineHeight());
		UpdateWindow();
		RecalcVertScrollBar(TRUE);
		UpdateSiblingScrollPos(TRUE);
	}

	m_ptCursorPos.y += GetScreenLines() - 1;
	if (m_ptCursorPos.y >= GetLineCount())
		m_ptCursorPos.y = GetLineCount() - 1;
	if (m_ptCursorPos.x > GetLineLength(m_ptCursorPos.y))
		m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);	//todo: no vertical scroll
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveCtrlHome(BOOL bSelect)
{
	m_ptCursorPos.x = 0;
	m_ptCursorPos.y = 0;
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::MoveCtrlEnd(BOOL bSelect)
{
	m_ptCursorPos.y = GetLineCount() - 1;
	m_ptCursorPos.x = GetLineLength(m_ptCursorPos.y);
	m_nIdealCharPos = CalculateActualOffset(m_ptCursorPos.y, m_ptCursorPos.x);
	EnsureVisible(m_ptCursorPos);
	UpdateCaret();
	if (! bSelect)
		m_ptAnchor = m_ptCursorPos;
	SetSelection(m_ptAnchor, m_ptCursorPos);
}

void CCrystalTextView::ScrollUp()
{
	if (m_nTopLine > 0)
	{
		m_nTopLine --;
		ScrollWindow(0, GetLineHeight());
		RecalcVertScrollBar(TRUE);
		UpdateSiblingScrollPos(TRUE);
	}
}

void CCrystalTextView::ScrollDown()
{
	if (m_nTopLine < GetLineCount() - 1)
	{
		m_nTopLine ++;
		ScrollWindow(0, - GetLineHeight());
		RecalcVertScrollBar(TRUE);
		UpdateSiblingScrollPos(TRUE);
	}
}

void CCrystalTextView::ScrollLeft()
{
	if (m_nOffsetChar > 0)
	{
		m_nOffsetChar --;
		CRect rcScroll;
		GetClientRect(&rcScroll);
		rcScroll.left += GetMarginWidth();
		ScrollWindow(GetCharWidth(), 0, &rcScroll, &rcScroll);
		UpdateCaret();
		RecalcHorzScrollBar(TRUE);
	}
}

void CCrystalTextView::ScrollRight()
{
	if (m_nOffsetChar < GetMaxLineLength() - 1)
	{
		m_nOffsetChar ++;
		CRect rcScroll;
		GetClientRect(&rcScroll);
		rcScroll.left += GetMarginWidth();
		ScrollWindow(-GetCharWidth(), 0, &rcScroll, &rcScroll);
		UpdateCaret();
		RecalcHorzScrollBar(TRUE);
	}
}

CPoint CCrystalTextView::WordToRight(CPoint pt)
{
	ASSERT_VALIDTEXTPOS(pt);
	int nLength = GetLineLength(pt.y);
	LPCTSTR pszChars = GetLineChars(pt.y);
	while (pt.x < nLength)
	{
		if (! isalnum(pszChars[pt.x]) && pszChars[pt.x] != _T('_'))
			break;
		pt.x ++;
	}
	ASSERT_VALIDTEXTPOS(pt);
	return pt;
}

CPoint CCrystalTextView::WordToLeft(CPoint pt)
{
	ASSERT_VALIDTEXTPOS(pt);
	LPCTSTR pszChars = GetLineChars(pt.y);
	while (pt.x > 0)
	{
		if (! isalnum(pszChars[pt.x - 1]) && pszChars[pt.x - 1] != _T('_'))
			break;
		pt.x --;
	}
	ASSERT_VALIDTEXTPOS(pt);
	return pt;
}

void CCrystalTextView::SelectAll()
{
	int nLineCount = GetLineCount();
	m_ptCursorPos.x = GetLineLength(nLineCount - 1);
	m_ptCursorPos.y = nLineCount - 1;
	SetSelection(CPoint(0, 0), m_ptCursorPos);
	UpdateCaret();
}


void CCrystalTextView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDown(nFlags, point);

	BOOL bShift = GetKeyState(VK_SHIFT) & 0x8000;
	BOOL bControl = GetKeyState(VK_CONTROL) & 0x8000;

	if (point.x < GetMarginWidth())
	{
		AdjustTextPoint(point);
		if (bControl)
		{
			SelectAll();
		}
		else
		{
			m_ptCursorPos = ClientToText(point);
			m_ptCursorPos.x = 0;				//	Force beginning of the line
			if (! bShift)
				m_ptAnchor = m_ptCursorPos;

			CPoint ptStart, ptEnd;
			ptStart = m_ptAnchor;
			if (ptStart.y == GetLineCount() - 1)
				ptStart.x = GetLineLength(ptStart.y);
			else
			{
				ptStart.y ++;
				ptStart.x = 0;
			}

			ptEnd = m_ptCursorPos;
			ptEnd.x = 0;

			m_ptCursorPos = ptEnd;
			UpdateCaret();
			EnsureVisible(m_ptCursorPos);
			SetSelection(ptStart, ptEnd);

			SetCapture();
			m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
			ASSERT(m_nDragSelTimer != 0);
			m_bWordSelection = FALSE;
			m_bLineSelection = TRUE;
			m_bDragSelection = TRUE;
		}
	}
	else
	{
		CPoint ptText = ClientToText(point);
		PrepareSelBounds();
		//
		//	[JRT]:	Support For Disabling Drag and Drop...
		//
		if( ( IsInsideSelBlock( ptText ) ) &&				// If Inside Selection Area
				( !m_bDisableDragAndDrop ) )				// And D&D Not Disabled
		{
			m_bPreparingToDrag = TRUE;
		}
		else
		{
			AdjustTextPoint(point);
			m_ptCursorPos = ClientToText(point);
			if (! bShift)
				m_ptAnchor = m_ptCursorPos;

			CPoint ptStart, ptEnd;
			if (bControl)
			{
				if (m_ptCursorPos.y < m_ptAnchor.y ||
					m_ptCursorPos.y == m_ptAnchor.y && m_ptCursorPos.x < m_ptAnchor.x)
				{
					ptStart = WordToLeft(m_ptCursorPos);
					ptEnd = WordToRight(m_ptAnchor);
				}
				else
				{
					ptStart = WordToLeft(m_ptAnchor);
					ptEnd = WordToRight(m_ptCursorPos);
				}
			}
			else
			{
				ptStart = m_ptAnchor;
				ptEnd = m_ptCursorPos;
			}

			m_ptCursorPos = ptEnd;
			UpdateCaret();
			EnsureVisible(m_ptCursorPos);
			SetSelection(ptStart, ptEnd);

			SetCapture();
			m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
			ASSERT(m_nDragSelTimer != 0);
			m_bWordSelection = bControl;
			m_bLineSelection = FALSE;
			m_bDragSelection = TRUE;
		}
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CView::OnMouseMove(nFlags, point);

	if (m_bDragSelection)
	{
		BOOL bOnMargin = point.x < GetMarginWidth();

		AdjustTextPoint(point);
		CPoint ptNewCursorPos = ClientToText(point);

		CPoint ptStart, ptEnd;
		if (m_bLineSelection)
		{
			if (bOnMargin)
			{
				if (ptNewCursorPos.y < m_ptAnchor.y ||
					ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
				{
					ptEnd = m_ptAnchor;
					if (ptEnd.y == GetLineCount() - 1)
					{
						ptEnd.x = GetLineLength(ptEnd.y);
					}
					else
					{
						ptEnd.y ++;
						ptEnd.x = 0;
					}
					ptNewCursorPos.x = 0;
					m_ptCursorPos = ptNewCursorPos;
				}
				else
				{
					ptEnd = m_ptAnchor;
					ptEnd.x = 0;
					m_ptCursorPos = ptNewCursorPos;
					if (ptNewCursorPos.y == GetLineCount() - 1)
					{
						ptNewCursorPos.x = GetLineLength(ptNewCursorPos.y);
					}
					else
					{
						ptNewCursorPos.y ++;
						ptNewCursorPos.x = 0;
					}
					m_ptCursorPos.x = 0;
				}
				UpdateCaret();
				SetSelection(ptNewCursorPos, ptEnd);
				return;
			}

			//	Moving to normal selection mode
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_IBEAM)));
			m_bLineSelection = m_bWordSelection = FALSE;
		}

		if (m_bWordSelection)
		{
			if (ptNewCursorPos.y < m_ptAnchor.y ||
				ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
			{
				ptStart = WordToLeft(ptNewCursorPos);
				ptEnd = WordToRight(m_ptAnchor);
			}
			else
			{
				ptStart = WordToLeft(m_ptAnchor);
				ptEnd = WordToRight(ptNewCursorPos);
			}
		}
		else
		{
			ptStart = m_ptAnchor;
			ptEnd = ptNewCursorPos;
		}

		m_ptCursorPos = ptEnd;
		UpdateCaret();
		SetSelection(ptStart, ptEnd);
	}

	if (m_bPreparingToDrag)
	{
		m_bPreparingToDrag = FALSE;
		HGLOBAL hData = PrepareDragData();
		if (hData != NULL)
		{
			if (m_pTextBuffer != NULL)
				m_pTextBuffer->BeginUndoGroup();

			COleDataSource ds;
			ds.CacheGlobalData(CF_TEXT, hData);
			m_bDraggingText = TRUE;
			DROPEFFECT de = ds.DoDragDrop(GetDropEffect());
			if (de != DROPEFFECT_NONE)
				OnDropSource(de);
			m_bDraggingText = FALSE;

			if (m_pTextBuffer != NULL)
				m_pTextBuffer->FlushUndoGroup();
		}
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CView::OnLButtonUp(nFlags, point);

	if (m_bDragSelection)
	{
		AdjustTextPoint(point);
		CPoint ptNewCursorPos = ClientToText(point);

		CPoint ptStart, ptEnd;
		if (m_bLineSelection)
		{
			CPoint ptEnd;
			if (ptNewCursorPos.y < m_ptAnchor.y ||
				ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
			{
				ptEnd = m_ptAnchor;
				if (ptEnd.y == GetLineCount() - 1)
				{
					ptEnd.x = GetLineLength(ptEnd.y);
				}
				else
				{
					ptEnd.y ++;
					ptEnd.x = 0;
				}
				ptNewCursorPos.x = 0;
				m_ptCursorPos = ptNewCursorPos;
			}
			else
			{
				ptEnd = m_ptAnchor;
				ptEnd.x = 0;
				if (ptNewCursorPos.y == GetLineCount() - 1)
				{
					ptNewCursorPos.x = GetLineLength(ptNewCursorPos.y);
				}
				else
				{
					ptNewCursorPos.y ++;
					ptNewCursorPos.x = 0;
				}
				m_ptCursorPos = ptNewCursorPos;
			}
			EnsureVisible(m_ptCursorPos);
			UpdateCaret();
			SetSelection(ptNewCursorPos, ptEnd);
		}
		else
		{
			if (m_bWordSelection)
			{
				if (ptNewCursorPos.y < m_ptAnchor.y ||
					ptNewCursorPos.y == m_ptAnchor.y && ptNewCursorPos.x < m_ptAnchor.x)
				{
					ptStart = WordToLeft(ptNewCursorPos);
					ptEnd = WordToRight(m_ptAnchor);
				}
				else
				{
					ptStart = WordToLeft(m_ptAnchor);
					ptEnd = WordToRight(ptNewCursorPos);
				}
			}
			else
			{
				ptStart = m_ptAnchor;
				ptEnd = m_ptCursorPos;
			}

			m_ptCursorPos = ptEnd;
			EnsureVisible(m_ptCursorPos);
			UpdateCaret();
			SetSelection(ptStart, ptEnd);
		}

		ReleaseCapture();
		KillTimer(m_nDragSelTimer);
		m_bDragSelection = FALSE;
	}

	if (m_bPreparingToDrag)
	{
		m_bPreparingToDrag = FALSE;

		AdjustTextPoint(point);
		m_ptCursorPos = ClientToText(point);
		EnsureVisible(m_ptCursorPos);
		UpdateCaret();
		SetSelection(m_ptCursorPos, m_ptCursorPos);
	}

	ASSERT_VALIDTEXTPOS(m_ptCursorPos);
}

void CCrystalTextView::OnTimer(UINT nIDEvent) 
{
	CView::OnTimer(nIDEvent);

	if (nIDEvent == CRYSTAL_TIMER_DRAGSEL)
	{
		ASSERT(m_bDragSelection);
		CPoint pt;
		::GetCursorPos(&pt);
		ScreenToClient(&pt);
		CRect rcClient;
		GetClientRect(&rcClient);

		BOOL bChanged = FALSE;

		//	Scroll vertically, if necessary
		int nNewTopLine = m_nTopLine;
		int nLineCount = GetLineCount();
		if (pt.y < rcClient.top)
		{
			nNewTopLine --;
			if (pt.y < rcClient.top - GetLineHeight())
				nNewTopLine -= 2;
		}
		else
		if (pt.y >= rcClient.bottom)
		{
			nNewTopLine ++;
			if (pt.y >= rcClient.bottom + GetLineHeight())
				nNewTopLine += 2;
		}

		if (nNewTopLine < 0)
			nNewTopLine = 0;
		if (nNewTopLine >= nLineCount)
			nNewTopLine = nLineCount - 1;

		if (m_nTopLine != nNewTopLine)
		{
			int nScrollLines = m_nTopLine - nNewTopLine;
			m_nTopLine = nNewTopLine;
			ScrollWindow(0, nScrollLines * GetLineHeight());
			RecalcVertScrollBar(TRUE);
			UpdateSiblingScrollPos(TRUE);
			bChanged = TRUE;
		}

		//	Scroll horizontally, if necessary
		int nNewOffsetChar = m_nOffsetChar;
		int nMaxLineLength = GetMaxLineLength();
		if (pt.x < rcClient.left)
			nNewOffsetChar --;
		else
		if (pt.x >= rcClient.right)
			nNewOffsetChar ++;

		if (nNewOffsetChar >= nMaxLineLength)
			nNewOffsetChar = nMaxLineLength - 1;
		if (nNewOffsetChar < 0)
			nNewOffsetChar = 0;

		if (m_nOffsetChar != nNewOffsetChar)
		{
			int nScrollChars = m_nOffsetChar - nNewOffsetChar;
			m_nOffsetChar = nNewOffsetChar;
			CRect rcScroll;
			GetClientRect(&rcScroll);
			rcScroll.left += GetMarginWidth();
			ScrollWindow(nScrollChars * GetCharWidth(), 0, &rcScroll, &rcScroll);
			UpdateCaret();
			RecalcHorzScrollBar(TRUE);
			UpdateSiblingScrollPos(FALSE);
			bChanged = TRUE;
		}

		//	Fix changes
		if (bChanged)
		{
			AdjustTextPoint(pt);
			CPoint ptNewCursorPos = ClientToText(pt);
			if (ptNewCursorPos != m_ptCursorPos)
			{
				m_ptCursorPos = ptNewCursorPos;
				UpdateCaret();
			}
			SetSelection(m_ptAnchor, m_ptCursorPos);
		}
	}
}

void CCrystalTextView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	CView::OnLButtonDblClk(nFlags, point);

	if (! m_bDragSelection)
	{
		AdjustTextPoint(point);

		m_ptCursorPos = ClientToText(point);
		m_ptAnchor = m_ptCursorPos;

		CPoint ptStart, ptEnd;
		if (m_ptCursorPos.y < m_ptAnchor.y ||
			m_ptCursorPos.y == m_ptAnchor.y && m_ptCursorPos.x < m_ptAnchor.x)
		{
			ptStart = WordToLeft(m_ptCursorPos);
			ptEnd = WordToRight(m_ptAnchor);
		}
		else
		{
			ptStart = WordToLeft(m_ptAnchor);
			ptEnd = WordToRight(m_ptCursorPos);
		}

		m_ptCursorPos = ptEnd;
		UpdateCaret();
		EnsureVisible(m_ptCursorPos);
		SetSelection(ptStart, ptEnd);

		SetCapture();
		m_nDragSelTimer = SetTimer(CRYSTAL_TIMER_DRAGSEL, 100, NULL);
		ASSERT(m_nDragSelTimer != 0);
		m_bWordSelection = TRUE;
		m_bLineSelection = FALSE;
		m_bDragSelection = TRUE;
	}
}

void CCrystalTextView::OnEditCopy() 
{
	Copy();
}

void CCrystalTextView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_ptSelStart != m_ptSelEnd);
}

void CCrystalTextView::OnEditSelectAll() 
{
	SelectAll();
}

void CCrystalTextView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(TRUE);
}

void CCrystalTextView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CPoint pt = point;
	AdjustTextPoint(pt);
	pt = ClientToText(pt);
	if (! IsInsideSelBlock(pt))
	{
		m_ptAnchor = m_ptCursorPos = pt;
		SetSelection(m_ptCursorPos, m_ptCursorPos);
		EnsureVisible(m_ptCursorPos);
		UpdateCaret();
	}

	CView::OnRButtonDown(nFlags, point);
}

BOOL CCrystalTextView::IsSelection()
{
	return m_ptSelStart != m_ptSelEnd;
}

void CCrystalTextView::Copy()
{
	if (m_ptSelStart == m_ptSelEnd)
		return;

	PrepareSelBounds();
	CString text;
	GetText(m_ptDrawSelStart, m_ptDrawSelEnd, text);
	PutToClipboard(text);
}

BOOL CCrystalTextView::TextInClipboard()
{
	return IsClipboardFormatAvailable(CF_TEXT);
}

BOOL CCrystalTextView::PutToClipboard(LPCTSTR pszText)
{
	if (pszText == NULL || lstrlen(pszText) == 0)
		return FALSE;

	CWaitCursor wc;
	BOOL bOK = FALSE;
	if (OpenClipboard())
	{
		EmptyClipboard();
		HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, lstrlen(pszText) + 1);
		if (hData != NULL)
		{
			LPSTR pszData = (LPSTR) ::GlobalLock(hData);
			USES_CONVERSION;
			strcpy(pszData, T2A((LPTSTR) pszText));
			GlobalUnlock(hData);
			bOK = SetClipboardData(CF_TEXT, hData) != NULL;
		}
		CloseClipboard();
	}
	return bOK;
}

BOOL CCrystalTextView::GetFromClipboard(CString &text)
{
	BOOL bSuccess = FALSE;
	if (OpenClipboard())
	{
		HGLOBAL hData = GetClipboardData(CF_TEXT);
		if (hData != NULL)
		{
			LPSTR pszData = (LPSTR) GlobalLock(hData);
			if (pszData != NULL)
			{
				text = pszData;
				GlobalUnlock(hData);
				bSuccess = TRUE;
			}
		}
		CloseClipboard();
	}
	return bSuccess;
}
