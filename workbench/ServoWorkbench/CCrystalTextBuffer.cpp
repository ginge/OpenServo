////////////////////////////////////////////////////////////////////////////
//	File:		CCrystalTextBuffer.cpp
//	Version:	1.0.0.0
//	Created:	29-Dec-1998
//
//	Author:		Stcherbatchenko Andrei
//	E-mail:		windfall@gmx.de
//
//	Implementation of the CCrystalTextBuffer class, a part of Crystal Edit -
//	syntax coloring text editor.
//
//	You are free to use, distribute or modify this code
//	but LEAVE THIS HEADER INTACT.
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <malloc.h>
#include "CCrystalTextBuffer.h"
#include "CCrystalTextView.h"

#ifndef __AFXPRIV_H__
#pragma message("Include <afxpriv.h> in your stdafx.h to avoid this message")
#include <afxpriv.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	Line allocation granularity
#define		CHAR_ALIGN					16
#define		ALIGN_BUF_SIZE(size)		((size) / CHAR_ALIGN) * CHAR_ALIGN + CHAR_ALIGN;

#define		UNDO_BUF_SIZE				1024

const TCHAR crlf[] = _T("\r\n");

#ifdef _DEBUG
#define _ADVANCED_BUGCHECK	1
#endif


/////////////////////////////////////////////////////////////////////////////
// CUpdateContext

void CCrystalTextBuffer::CInsertContext::RecalcPoint(CPoint &ptPoint)
{
	ASSERT(m_ptEnd.y > m_ptStart.y ||
		   m_ptEnd.y == m_ptStart.y && m_ptEnd.x >= m_ptStart.x);
	if (ptPoint.y < m_ptStart.y)
		return;
	if (ptPoint.y > m_ptStart.y)
	{
		ptPoint.y += (m_ptEnd.y - m_ptStart.y);
		return;
	}
	if (ptPoint.x <= m_ptStart.x)
		return;
	ptPoint.y += (m_ptEnd.y - m_ptStart.y);
	ptPoint.x = m_ptEnd.x + (ptPoint.x - m_ptStart.x);
}

void CCrystalTextBuffer::CDeleteContext::RecalcPoint(CPoint &ptPoint)
{
	ASSERT(m_ptEnd.y > m_ptStart.y ||
		   m_ptEnd.y == m_ptStart.y && m_ptEnd.x >= m_ptStart.x);
	if (ptPoint.y < m_ptStart.y)
		return;
	if (ptPoint.y > m_ptEnd.y)
	{
		ptPoint.y -= (m_ptEnd.y - m_ptStart.y);
		return;
	}
	if (ptPoint.y == m_ptEnd.y && ptPoint.x >= m_ptEnd.x)
	{
		ptPoint.y = m_ptStart.y;
		ptPoint.x = m_ptStart.x + (ptPoint.x - m_ptEnd.x);
		return;
	}
	if (ptPoint.y == m_ptStart.y)
	{
		if (ptPoint.x > m_ptStart.x)
			ptPoint.x = m_ptStart.x;
		return;
	}
	ptPoint = m_ptStart;
}


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer

IMPLEMENT_DYNCREATE(CCrystalTextBuffer, CCmdTarget)

CCrystalTextBuffer::CCrystalTextBuffer()
{
	m_bInit = FALSE;
	m_bReadOnly = FALSE;
	m_bModified = FALSE;
	m_bCreateBackupFile = FALSE;
	m_nUndoPosition = 0;
}

CCrystalTextBuffer::~CCrystalTextBuffer()
{
	ASSERT(! m_bInit);			//	You must call FreeAll() before deleting the object
}


BEGIN_MESSAGE_MAP(CCrystalTextBuffer, CCmdTarget)
	//{{AFX_MSG_MAP(CCrystalTextBuffer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCrystalTextBuffer message handlers

void CCrystalTextBuffer::InsertLine(LPCTSTR pszLine, int nLength /*= -1*/, int nPosition /*= -1*/)
{
	if (nLength == -1)
	{
		if (pszLine == NULL)
			nLength = 0;
		else
			nLength = lstrlen(pszLine);
	}

	SLineInfo li;
	li.m_nLength = nLength;
	li.m_nMax = ALIGN_BUF_SIZE(li.m_nLength);
	ASSERT(li.m_nMax >= li.m_nLength);
	if (li.m_nMax > 0)
		li.m_pcLine = new TCHAR[li.m_nMax];
	if (li.m_nLength > 0)
		memcpy(li.m_pcLine, pszLine, sizeof(TCHAR) * li.m_nLength);

	if (nPosition == -1)
		m_aLines.Add(li);
	else
		m_aLines.InsertAt(nPosition, li);

#ifdef _DEBUG
	int nLines = (int)m_aLines.GetSize();
	if (nLines % 5000 == 0)
		TRACE1("%d lines loaded!\n", nLines);
#endif
}

void CCrystalTextBuffer::AppendLine(int nLineIndex, LPCTSTR pszChars, int nLength /*= -1*/)
{
	if (nLength == -1)
	{
		if (pszChars == NULL)
			return;
		nLength = lstrlen(pszChars);
	}

	if (nLength == 0)
		return;

	register SLineInfo &li = m_aLines[nLineIndex];
	int nBufNeeded = li.m_nLength + nLength;
	if (nBufNeeded > li.m_nMax)
	{
		li.m_nMax = ALIGN_BUF_SIZE(nBufNeeded);
		ASSERT(li.m_nMax >= li.m_nLength + nLength);
		TCHAR *pcNewBuf = new TCHAR[li.m_nMax];
		if (li.m_nLength > 0)
			memcpy(pcNewBuf, li.m_pcLine, sizeof(TCHAR) * li.m_nLength);
		delete li.m_pcLine;
		li.m_pcLine = pcNewBuf;
	}
	memcpy(li.m_pcLine + li.m_nLength, pszChars, sizeof(TCHAR) * nLength);
	li.m_nLength += nLength;
	ASSERT(li.m_nLength <= li.m_nMax);
}

void CCrystalTextBuffer::FreeAll()
{
	//	Free text
	int nCount = (int)m_aLines.GetSize();
	for (int I = 0; I < nCount; I ++)
	{
		if (m_aLines[I].m_nMax > 0)
			delete m_aLines[I].m_pcLine;
	}
	m_aLines.RemoveAll();

	//	Free undo buffer
	int nBufSize = (int)m_aUndoBuf.GetSize();
	for (I = 0; I < nBufSize; I ++)
		delete m_aUndoBuf[I].m_pcText;
	m_aUndoBuf.RemoveAll();

	m_bInit = FALSE;
}

BOOL CCrystalTextBuffer::InitNew(int nCrlfStyle /*= CRLF_STYLE_DOS*/)
{
	ASSERT(! m_bInit);
	ASSERT(m_aLines.GetSize() == 0);
	ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
	InsertLine(_T(""));
	m_bInit = TRUE;
	m_bReadOnly = FALSE;
	m_nCRLFMode = nCrlfStyle;
	m_bModified = FALSE;
	m_nSyncPosition = m_nUndoPosition = 0;
	m_bUndoGroup = m_bUndoBeginGroup = FALSE;
	m_nUndoBufSize = UNDO_BUF_SIZE;
	ASSERT(m_aUndoBuf.GetSize() == 0);
	UpdateViews(NULL, NULL, UPDATE_RESET);
	return TRUE;
}

BOOL CCrystalTextBuffer::GetReadOnly() const
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	return m_bReadOnly;
}

void CCrystalTextBuffer::SetReadOnly(BOOL bReadOnly /*= TRUE*/)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	m_bReadOnly = bReadOnly;
}

static const char *crlfs[] =
{
	"\x0d\x0a",			//	DOS/Windows style
	"\x0a\x0d",			//	UNIX style
	"\x0a"				//	Macintosh style
};

BOOL CCrystalTextBuffer::LoadFromFile(LPCTSTR pszFileName, int nCrlfStyle /*= CRLF_STYLE_AUTOMATIC*/)
{
	ASSERT(! m_bInit);
	ASSERT(m_aLines.GetSize() == 0);

	HANDLE hFile = NULL;
	int nCurrentMax = 256;
	char *pcLineBuf = new char[nCurrentMax];

	BOOL bSuccess = FALSE;
	__try
	{
		DWORD dwFileAttributes = ::GetFileAttributes(pszFileName);
		if (dwFileAttributes == (DWORD) -1)
			__leave;

		hFile = ::CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			__leave;

		int nCurrentLength = 0;

		const DWORD dwBufSize = 32768;
		char *pcBuf = (char *) _alloca(dwBufSize);
		DWORD dwCurSize;
		if (! ::ReadFile(hFile, pcBuf, dwBufSize, &dwCurSize, NULL))
			__leave;

		if (nCrlfStyle == CRLF_STYLE_AUTOMATIC)
		{
			//	Try to determine current CRLF mode
			for (DWORD I = 0; I < dwCurSize; I ++)
			{
				if (pcBuf[I] == _T('\x0a'))
					break;
			}
			if (I == dwCurSize)
			{
				//	By default (or in the case of empty file), set DOS style
				nCrlfStyle = CRLF_STYLE_DOS;
			}
			else
			{
				//	Otherwise, analyse the first occurance of line-feed character
				if (I > 0 && pcBuf[I - 1] == _T('\x0d'))
				{
					nCrlfStyle = CRLF_STYLE_DOS;
				}
				else
				{
					if (I < dwCurSize - 1 && pcBuf[I + 1] == _T('\x0d'))
						nCrlfStyle = CRLF_STYLE_UNIX;
					else
						nCrlfStyle = CRLF_STYLE_MAC;
				}
			}
		}

		ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
		m_nCRLFMode = nCrlfStyle;
		const char *crlf = crlfs[nCrlfStyle];

		m_aLines.SetSize(0, 4096);

		DWORD dwBufPtr = 0;
		int nCrlfPtr = 0;
		USES_CONVERSION;
		while (dwBufPtr < dwCurSize)
		{
			int c = pcBuf[dwBufPtr];
			dwBufPtr ++;
			if (dwBufPtr == dwCurSize && dwCurSize == dwBufSize)
			{
				if (! ::ReadFile(hFile, pcBuf, dwBufSize, &dwCurSize, NULL))
					__leave;
				dwBufPtr = 0;
			}

			pcLineBuf[nCurrentLength] = (char) c;
			nCurrentLength ++;
			if (nCurrentLength == nCurrentMax)
			{
				//	Reallocate line buffer
				nCurrentMax += 256;
				char *pcNewBuf = new char[nCurrentMax];
				memcpy(pcNewBuf, pcLineBuf, nCurrentLength);
				delete pcLineBuf;
				pcLineBuf = pcNewBuf;
			}

			if ((char) c == crlf[nCrlfPtr])
			{
				nCrlfPtr ++;
				if (crlf[nCrlfPtr] == 0)
				{
					pcLineBuf[nCurrentLength - nCrlfPtr] = 0;
					InsertLine(A2T(pcLineBuf));
					nCurrentLength = 0;
					nCrlfPtr = 0;
				}
			}
			else
				nCrlfPtr = 0;
		}

		pcLineBuf[nCurrentLength] = 0;
		InsertLine(A2T(pcLineBuf));

		ASSERT(m_aLines.GetSize() > 0);		//	At least one empty line must present

		m_bInit = TRUE;
		m_bReadOnly = (dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0;
		m_bModified = FALSE;
		m_bUndoGroup = m_bUndoBeginGroup = FALSE;
		m_nUndoBufSize = UNDO_BUF_SIZE;
		m_nSyncPosition = m_nUndoPosition = 0;
		ASSERT(m_aUndoBuf.GetSize() == 0);
		bSuccess = TRUE;

		UpdateViews(NULL, NULL, UPDATE_RESET);
	}
	__finally
	{
		if (pcLineBuf != NULL)
			delete pcLineBuf;
		if (hFile != NULL)
			::CloseHandle(hFile);
	}
	return bSuccess;
}

BOOL CCrystalTextBuffer::SaveToFile(LPCTSTR pszFileName, int nCrlfStyle /*= CRLF_STYLE_AUTOMATIC*/, BOOL bClearModifiedFlag /*= TRUE*/)
{
	ASSERT(nCrlfStyle == CRLF_STYLE_AUTOMATIC || nCrlfStyle == CRLF_STYLE_DOS||
			nCrlfStyle == CRLF_STYLE_UNIX || nCrlfStyle == CRLF_STYLE_MAC);
	ASSERT(m_bInit);
	HANDLE hTempFile = INVALID_HANDLE_VALUE;
	HANDLE hSearch = INVALID_HANDLE_VALUE;
	TCHAR szTempFileDir[_MAX_PATH + 1];
	TCHAR szTempFileName[_MAX_PATH + 1];
	TCHAR szBackupFileName[_MAX_PATH + 1];
	BOOL bSuccess = FALSE;
	__try
	{
		TCHAR drive[_MAX_PATH], dir[_MAX_PATH], name[_MAX_PATH], ext[_MAX_PATH];
#ifdef _UNICODE
		_wsplitpath(pszFileName, drive, dir, name, ext);
#else
		_splitpath(pszFileName, drive, dir, name, ext);
#endif
		lstrcpy(szTempFileDir, drive);
		lstrcat(szTempFileDir, dir);
		lstrcpy(szBackupFileName, pszFileName);
		lstrcat(szBackupFileName, _T(".bak"));

		if (::GetTempFileName(szTempFileDir, _T("CRE"), 0, szTempFileName) == 0)
			__leave;

		hTempFile = ::CreateFile(szTempFileName, GENERIC_WRITE, 0, NULL,
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hTempFile == INVALID_HANDLE_VALUE)
			__leave;

		if (nCrlfStyle == CRLF_STYLE_AUTOMATIC)
			nCrlfStyle = m_nCRLFMode;

		ASSERT(nCrlfStyle >= 0 && nCrlfStyle <= 2);
		const char *pszCRLF = crlfs[nCrlfStyle];
		int nCRLFLength = (int)strlen(pszCRLF);

		int nLineCount = (int)m_aLines.GetSize();
		USES_CONVERSION;
		for (int nLine = 0; nLine < nLineCount; nLine ++)
		{
			int nLength = m_aLines[nLine].m_nLength;
			DWORD dwWrittenBytes;
			if (nLength > 0)
			{
				if (! ::WriteFile(hTempFile, T2A(m_aLines[nLine].m_pcLine), nLength, &dwWrittenBytes, NULL))
					__leave;
				if (nLength != (int) dwWrittenBytes)
					__leave;
			}
			if (nLine < nLineCount - 1)	//	Last line must not end with CRLF
			{
				if (! ::WriteFile(hTempFile, pszCRLF, nCRLFLength, &dwWrittenBytes, NULL))
					__leave;
				if (nCRLFLength != (int) dwWrittenBytes)
					__leave;
			}
		}
		::CloseHandle(hTempFile);
		hTempFile = INVALID_HANDLE_VALUE;

		if (m_bCreateBackupFile)
		{
			WIN32_FIND_DATA wfd;
			HANDLE hSearch = ::FindFirstFile(pszFileName, &wfd);
			if (hSearch != INVALID_HANDLE_VALUE)
			{
				//	File exist - create backup file
				::DeleteFile(szBackupFileName);
				if (! ::MoveFile(pszFileName, szBackupFileName))
					__leave;
				::FindClose(hSearch);
				hSearch = INVALID_HANDLE_VALUE;
			}
		}
		else
		{
			::DeleteFile(pszFileName);
		}

		//	Move temporary file to target name
		if (! ::MoveFile(szTempFileName, pszFileName))
			__leave;

		if (bClearModifiedFlag)
		{
			SetModified(FALSE);
			m_nSyncPosition = m_nUndoPosition;
		}
		bSuccess = TRUE;
	}
	__finally
	{
		if (hSearch != INVALID_HANDLE_VALUE)
			::FindClose(hSearch);
		if (hTempFile != INVALID_HANDLE_VALUE)
			::CloseHandle(hTempFile);
		::DeleteFile(szTempFileName);
	}
	return bSuccess;
}

int CCrystalTextBuffer::GetCRLFMode()
{
	return m_nCRLFMode;
}

void CCrystalTextBuffer::SetCRLFMode(int nCRLFMode)
{
	ASSERT(nCRLFMode == CRLF_STYLE_DOS||
			nCRLFMode == CRLF_STYLE_UNIX ||
			nCRLFMode == CRLF_STYLE_MAC);
	m_nCRLFMode = nCRLFMode;
}

int CCrystalTextBuffer::GetLineCount()
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	return (int)m_aLines.GetSize();
}

int CCrystalTextBuffer::GetLineLength(int nLine)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	return m_aLines[nLine].m_nLength;
}

LPTSTR CCrystalTextBuffer::GetLineChars(int nLine)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	return m_aLines[nLine].m_pcLine;
}

DWORD CCrystalTextBuffer::GetLineFlags(int nLine)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	return m_aLines[nLine].m_dwFlags;
}

static int FlagToIndex(DWORD dwFlag)
{
	int nIndex = 0;
	while ((dwFlag & 1) == 0)
	{
		dwFlag = dwFlag >> 1;
		nIndex ++;
		if (nIndex == 32)
			return -1;
	}
	dwFlag = dwFlag & 0xFFFFFFFE;
	if (dwFlag != 0)
		return -1;
	return nIndex;

}

int CCrystalTextBuffer::FindLineWithFlag(DWORD dwFlag)
{
	int nSize = (int)m_aLines.GetSize();
	for (int L = 0; L < nSize; L ++)
	{
		if ((m_aLines[L].m_dwFlags & dwFlag) != 0)
			return L;
	}
	return -1;
}

int CCrystalTextBuffer::GetLineWithFlag(DWORD dwFlag)
{
	int nFlagIndex = ::FlagToIndex(dwFlag);
	if (nFlagIndex < 0)
	{
		ASSERT(FALSE);		//	Invalid flag passed in
		return -1;
	}
	return FindLineWithFlag(dwFlag);
}

void CCrystalTextBuffer::SetLineFlag(int nLine, DWORD dwFlag, BOOL bSet, BOOL bRemoveFromPreviousLine /*= TRUE*/)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	int nFlagIndex = ::FlagToIndex(dwFlag);
	if (nFlagIndex < 0)
	{
		ASSERT(FALSE);		//	Invalid flag passed in
		return;
	}

	if (nLine == -1)
	{
		ASSERT(! bSet);
		nLine = FindLineWithFlag(dwFlag);
		if (nLine == -1)
			return;
		bRemoveFromPreviousLine = FALSE;
	}

	DWORD dwNewFlags = m_aLines[nLine].m_dwFlags;
	if (bSet)
		dwNewFlags = dwNewFlags | dwFlag;
	else
		dwNewFlags = dwNewFlags & ~dwFlag;

	if (m_aLines[nLine].m_dwFlags != dwNewFlags)
	{
		if (bRemoveFromPreviousLine)
		{
			int nPrevLine = FindLineWithFlag(dwFlag);
			if (bSet)
			{
				if (nPrevLine >= 0)
				{
					ASSERT((m_aLines[nPrevLine].m_dwFlags & dwFlag) != 0);
					m_aLines[nPrevLine].m_dwFlags &= ~dwFlag;
					UpdateViews(NULL, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, nPrevLine);
				}
			}
			else
			{
				ASSERT(nPrevLine == nLine);
			}
		}

		m_aLines[nLine].m_dwFlags = dwNewFlags;
		UpdateViews(NULL, NULL, UPDATE_SINGLELINE | UPDATE_FLAGSONLY, nLine);
	}
}

void CCrystalTextBuffer::GetText(int nStartLine, int nStartChar, int nEndLine, int nEndChar, CString &text, LPCTSTR pszCRLF /*= NULL*/)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	ASSERT(nStartLine >= 0 && nStartLine < m_aLines.GetSize());
	ASSERT(nStartChar >= 0 && nStartChar <= m_aLines[nStartLine].m_nLength);
	ASSERT(nEndLine >= 0 && nEndLine < m_aLines.GetSize());
	ASSERT(nEndChar >= 0 && nEndChar <= m_aLines[nEndLine].m_nLength);
	ASSERT(nStartLine < nEndLine || nStartLine == nEndLine && nStartChar < nEndChar);
	
	if (pszCRLF == NULL)
		pszCRLF = crlf;
	int nCRLFLength = lstrlen(pszCRLF);
	ASSERT(nCRLFLength > 0);

	int nBufSize = 0;
	for (int L = nStartLine; L <= nEndLine; L ++)
	{
		nBufSize += m_aLines[L].m_nLength;
		nBufSize += nCRLFLength;
	}

	LPTSTR pszBuf = text.GetBuffer(nBufSize);
	LPTSTR pszCurPos = pszBuf;

	if (nStartLine < nEndLine)
	{
		int nCount = m_aLines[nStartLine].m_nLength - nStartChar;
		if (nCount > 0)
		{
			memcpy(pszBuf, m_aLines[nStartLine].m_pcLine + nStartChar, sizeof(TCHAR) * nCount);
			pszBuf += nCount;
		}
		memcpy(pszBuf, pszCRLF, sizeof(TCHAR) * nCRLFLength);
		pszBuf += nCRLFLength;
		for (int I = nStartLine + 1; I < nEndLine; I ++)
		{
			nCount = m_aLines[I].m_nLength;
			if (nCount > 0)
			{
				memcpy(pszBuf, m_aLines[I].m_pcLine, sizeof(TCHAR) * nCount);
				pszBuf += nCount;
			}
			memcpy(pszBuf, pszCRLF, sizeof(TCHAR) * nCRLFLength);
			pszBuf += nCRLFLength;
		}
		if (nEndChar > 0)
		{
			memcpy(pszBuf, m_aLines[nEndLine].m_pcLine, sizeof(TCHAR) * nEndChar);
			pszBuf += nEndChar;
		}
	}
	else
	{
		int nCount = nEndChar - nStartChar;
		memcpy(pszBuf, m_aLines[nStartLine].m_pcLine + nStartChar, sizeof(TCHAR) * nCount);
		pszBuf += nCount;
	}
	pszBuf[0] = 0;
	text.ReleaseBuffer();
	text.FreeExtra();
}

void CCrystalTextBuffer::AddView(CCrystalTextView *pView)
{
	m_lpViews.AddTail(pView);
}

void CCrystalTextBuffer::RemoveView(CCrystalTextView *pView)
{
	POSITION pos = m_lpViews.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION thispos = pos;
		CCrystalTextView *pvw = m_lpViews.GetNext(pos);
		if (pvw == pView)
		{
			m_lpViews.RemoveAt(thispos);
			return;
		}
	}
	ASSERT(FALSE);
}

void CCrystalTextBuffer::UpdateViews(CCrystalTextView *pSource, CUpdateContext *pContext, DWORD dwUpdateFlags, int nLineIndex /*= -1*/)
{
	POSITION pos = m_lpViews.GetHeadPosition();
	while (pos != NULL)
	{
		CCrystalTextView *pView = m_lpViews.GetNext(pos);
		pView->UpdateView(pSource, pContext, dwUpdateFlags, nLineIndex);
	}
}

BOOL CCrystalTextBuffer::InternalDeleteText(CCrystalTextView *pSource, int nStartLine, int nStartChar, int nEndLine, int nEndChar)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
	ASSERT(nStartLine >= 0 && nStartLine < m_aLines.GetSize());
	ASSERT(nStartChar >= 0 && nStartChar <= m_aLines[nStartLine].m_nLength);
	ASSERT(nEndLine >= 0 && nEndLine < m_aLines.GetSize());
	ASSERT(nEndChar >= 0 && nEndChar <= m_aLines[nEndLine].m_nLength);
	ASSERT(nStartLine < nEndLine || nStartLine == nEndLine && nStartChar < nEndChar);
	if (m_bReadOnly)
		return FALSE;

	CDeleteContext context;
	context.m_ptStart.y = nStartLine;
	context.m_ptStart.x = nStartChar;
	context.m_ptEnd.y = nEndLine;
	context.m_ptEnd.x = nEndChar;
	if (nStartLine == nEndLine)
	{
		SLineInfo &li = m_aLines[nStartLine];
		if (nEndChar < li.m_nLength)
		{
			memcpy(li.m_pcLine + nStartChar, li.m_pcLine + nEndChar,
					sizeof(TCHAR) * (li.m_nLength - nEndChar));
		}
		li.m_nLength -= (nEndChar - nStartChar);

		UpdateViews(pSource, &context, UPDATE_SINGLELINE | UPDATE_HORZRANGE, nStartLine);
	}
	else
	{
		int nRestCount = m_aLines[nEndLine].m_nLength - nEndChar;
		LPTSTR pszRestChars = NULL;
		if (nRestCount > 0)
		{
			pszRestChars = new TCHAR[nRestCount];
			memcpy(pszRestChars, m_aLines[nEndLine].m_pcLine + nEndChar, nRestCount * sizeof(TCHAR));
		}

		int nDelCount = nEndLine - nStartLine;
		for (int L = nStartLine + 1; L <= nEndLine; L ++)
			delete m_aLines[L].m_pcLine;
		m_aLines.RemoveAt(nStartLine + 1, nDelCount);

		//	nEndLine is no more valid
		m_aLines[nStartLine].m_nLength = nStartChar;
		if (nRestCount > 0)
		{
			AppendLine(nStartLine, pszRestChars, nRestCount);
			delete pszRestChars;
		}

		UpdateViews(pSource, &context, UPDATE_HORZRANGE | UPDATE_VERTRANGE, nStartLine);
	}

	if (! m_bModified)
		SetModified(TRUE);
	return TRUE;
}

BOOL CCrystalTextBuffer::InternalInsertText(CCrystalTextView *pSource, int nLine, int nPos, LPCTSTR pszText, int &nEndLine, int &nEndChar)
{
	ASSERT(m_bInit);	//	Text buffer not yet initialized.
						//	You must call InitNew() or LoadFromFile() first!
//	ASSERT(nLine >= 0 && nLine < m_aLines.GetSize());
//	ASSERT(nPos >= 0 && nPos <= m_aLines[nLine].m_nLength);
	if (m_bReadOnly)
		return FALSE;

	CInsertContext context;
	context.m_ptStart.x = nPos;
	context.m_ptStart.y = nLine;

	int nRestCount = m_aLines[nLine].m_nLength - nPos;
	LPTSTR pszRestChars = NULL;
	if (nRestCount > 0)
	{
		pszRestChars = new TCHAR[nRestCount];
		memcpy(pszRestChars, m_aLines[nLine].m_pcLine + nPos, nRestCount * sizeof(TCHAR));
		m_aLines[nLine].m_nLength = nPos;
	}

	int nCurrentLine = nLine;
	BOOL bNewLines = FALSE;
	int nTextPos;
	for (;;)
	{
		nTextPos = 0;
		while (pszText[nTextPos] != 0 && pszText[nTextPos] != _T('\r'))
			nTextPos ++;

		if (nCurrentLine == nLine)
		{
			AppendLine(nLine, pszText, nTextPos);
		}
		else
		{
			InsertLine(pszText, nTextPos, nCurrentLine);
			bNewLines = TRUE;
		}

		if (pszText[nTextPos] == 0)
		{
			nEndLine = nCurrentLine;
			nEndChar = m_aLines[nCurrentLine].m_nLength;
			AppendLine(nCurrentLine, pszRestChars, nRestCount);
			break;
		}

		nCurrentLine ++;
		nTextPos ++;

		if (pszText[nTextPos] == _T('\n'))
		{
			nTextPos ++;
		}
		else
		{
			ASSERT(FALSE);			//	Invalid line-end format passed
		}

		pszText += nTextPos;
	}

	if (pszRestChars != NULL)
		delete pszRestChars;

	context.m_ptEnd.x = nEndChar;
	context.m_ptEnd.y = nEndLine;

	if (bNewLines)
		UpdateViews(pSource, &context, UPDATE_HORZRANGE | UPDATE_VERTRANGE, nLine);
	else
		UpdateViews(pSource, &context, UPDATE_SINGLELINE | UPDATE_HORZRANGE, nLine);

	if (! m_bModified)
		SetModified(TRUE);
	return TRUE;
}


//
//	[JRT]: Support For Descriptions On Undo/Redo Actions...
//
BOOL CCrystalTextBuffer::CanUndo( PTSTR cpDesc )
{
	ASSERT(m_nUndoPosition >= 0 && m_nUndoPosition <= m_aUndoBuf.GetSize());
	if( m_nUndoPosition > 0 )								// If  Undo-Able Action Available
	{
		if( cpDesc )										// If Description Buffer Supplied
		{
			_tcscat( cpDesc, m_aUndoBuf[ m_nUndoPosition -
					1 ].m_cDescription );					// Add Description Text
		}
		return( TRUE );										// Return True; Redo Available
	}
	return( FALSE );										// Return False; Redo Unavailable
}


//
//	[JRT]: Support For Descriptions On Undo/Redo Actions...
//
BOOL CCrystalTextBuffer::CanRedo( PTSTR cpDesc )
{
	ASSERT(m_nUndoPosition >= 0 && m_nUndoPosition <= m_aUndoBuf.GetSize());
	if( m_nUndoPosition < m_aUndoBuf.GetSize() )			// If  Redo-Able Action Available
	{
		if( cpDesc )										// If Description Buffer Supplied
		{
			_tcscat( cpDesc, m_aUndoBuf[ m_nUndoPosition 
					].m_cDescription );						// Add Description Text
		}
		return( TRUE );										// Return True; Redo Available
	}
	return( FALSE );										// Return False; Redo Unavailable
}

BOOL CCrystalTextBuffer::Undo(CPoint &ptCursorPos)
{
	ASSERT(CanUndo());
	ASSERT((m_aUndoBuf[0].m_dwFlags & UNDO_BEGINGROUP) != 0);
	for (;;)
	{
		m_nUndoPosition --;
		const SUndoRecord &ur = m_aUndoBuf[m_nUndoPosition];
		if (ur.m_dwFlags & UNDO_INSERT)
		{
#ifdef _ADVANCED_BUGCHECK
			//	Try to ensure that we undoing correctly...
			//	Just compare the text as it was before Undo operation
			CString text;
			GetText(ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x, text);
			ASSERT(lstrcmp(text, ur.m_pcText) == 0);
#endif
			VERIFY(InternalDeleteText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x));
			ptCursorPos = ur.m_ptStartPos;
		}
		else
		{
			int nEndLine, nEndChar;
			VERIFY(InternalInsertText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_pcText, nEndLine, nEndChar));
#ifdef _ADVANCED_BUGCHECK
			ASSERT(ur.m_ptEndPos.y == nEndLine);
			ASSERT(ur.m_ptEndPos.x == nEndChar);
#endif
			ptCursorPos = ur.m_ptEndPos;
		}
		if (ur.m_dwFlags & UNDO_BEGINGROUP)
			break;
	}
	if (m_bModified && m_nSyncPosition == m_nUndoPosition)
		SetModified(FALSE);
	if (! m_bModified && m_nSyncPosition != m_nUndoPosition)
		SetModified(TRUE);
	return TRUE;
}

BOOL CCrystalTextBuffer::Redo(CPoint &ptCursorPos)
{
	ASSERT(CanRedo());
	ASSERT((m_aUndoBuf[0].m_dwFlags & UNDO_BEGINGROUP) != 0);
	ASSERT((m_aUndoBuf[m_nUndoPosition].m_dwFlags & UNDO_BEGINGROUP) != 0);
	for (;;)
	{
		const SUndoRecord &ur = m_aUndoBuf[m_nUndoPosition];
		if (ur.m_dwFlags & UNDO_INSERT)
		{
			int nEndLine, nEndChar;
			VERIFY(InternalInsertText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_pcText, nEndLine, nEndChar));
	#ifdef _ADVANCED_BUGCHECK
			ASSERT(ur.m_ptEndPos.y == nEndLine);
			ASSERT(ur.m_ptEndPos.x == nEndChar);
	#endif
			ptCursorPos = ur.m_ptEndPos;
		}
		else
		{
	#ifdef _ADVANCED_BUGCHECK
			CString text;
			GetText(ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x, text);
			ASSERT(lstrcmp(text, ur.m_pcText) == 0);
	#endif
			VERIFY(InternalDeleteText(NULL, ur.m_ptStartPos.y, ur.m_ptStartPos.x, ur.m_ptEndPos.y, ur.m_ptEndPos.x));
			ptCursorPos = ur.m_ptStartPos;
		}
		m_nUndoPosition ++;
		if (m_nUndoPosition == m_aUndoBuf.GetSize())
			break;
		if ((m_aUndoBuf[m_nUndoPosition].m_dwFlags & UNDO_BEGINGROUP) != 0)
			break;
	}
	if (m_bModified && m_nSyncPosition == m_nUndoPosition)
		SetModified(FALSE);
	if (! m_bModified && m_nSyncPosition != m_nUndoPosition)
		SetModified(TRUE);
	return TRUE;
}

void CCrystalTextBuffer::AddUndoRecord(BOOL bInsert, const CPoint &ptStartPos, const CPoint &ptEndPos, LPCTSTR pszText, LPCTSTR cpDesc )
{
	//	Forgot to call BeginUndoGroup()?
	ASSERT(m_bUndoGroup);
	ASSERT(m_aUndoBuf.GetSize() == 0 || (m_aUndoBuf[0].m_dwFlags & UNDO_BEGINGROUP) != 0);

	//	Strip unnecessary undo records (edit after undo)
	int nBufSize = (int)m_aUndoBuf.GetSize();
	if (m_nUndoPosition < nBufSize)
	{
		for (int I = m_nUndoPosition; I < nBufSize; I ++)
			delete m_aUndoBuf[I].m_pcText;
		m_aUndoBuf.SetSize(m_nUndoPosition);
	}

	//	If undo buffer size is close to critical, remove the oldest records
	ASSERT(m_aUndoBuf.GetSize() <= m_nUndoBufSize);
	nBufSize = (int)m_aUndoBuf.GetSize();
	if (nBufSize >= m_nUndoBufSize)
	{
		int nIndex = 0;
		for (;;)
		{
			delete m_aUndoBuf[nIndex].m_pcText;
			nIndex ++;
			if (nIndex == nBufSize || (m_aUndoBuf[nIndex].m_dwFlags & UNDO_BEGINGROUP) != 0)
				break;
		}
		m_aUndoBuf.RemoveAt(0, nIndex);
	}
	ASSERT(m_aUndoBuf.GetSize() < m_nUndoBufSize);

	//	Add new record
	SUndoRecord ur;
	ur.m_dwFlags = bInsert ? UNDO_INSERT : 0;
	if( ( cpDesc ) && ( _tcslen( cpDesc ) ) )				// If Description Text Supplied
	{
		_tcscpy( ur.m_cDescription, cpDesc );				// Copy Description
	}
	else													// If Not
	{
		_tcscpy( ur.m_cDescription, _T( "Unknown" ) );		// Copy Unknown Description
	}
	if (m_bUndoBeginGroup)
	{
		ur.m_dwFlags |= UNDO_BEGINGROUP;
		m_bUndoBeginGroup = FALSE;
	}
	ur.m_ptStartPos = ptStartPos;
	ur.m_ptEndPos = ptEndPos;
	if (pszText != NULL && pszText[0] != 0)
	{
		ur.m_pcText = new TCHAR[sizeof(TCHAR) * (lstrlen(pszText) + 1)];
		lstrcpy(ur.m_pcText, pszText);
	}
	else
		ur.m_pcText = NULL;

	m_aUndoBuf.Add(ur);
	m_nUndoPosition = (int)m_aUndoBuf.GetSize();

	ASSERT(m_aUndoBuf.GetSize() <= m_nUndoBufSize);
}

BOOL CCrystalTextBuffer::InsertText(CCrystalTextView *pSource, int nLine, int nPos, LPCTSTR pszText, int &nEndLine, int &nEndChar, LPCTSTR cpDesc )
{
	if (! InternalInsertText(pSource, nLine, nPos, pszText, nEndLine, nEndChar))
		return FALSE;

	BOOL bGroupFlag = FALSE;
	if (! m_bUndoGroup)
	{
		BeginUndoGroup();
		bGroupFlag = TRUE;
	}
	AddUndoRecord(TRUE, CPoint(nPos, nLine), CPoint(nEndChar, nEndLine), pszText, cpDesc );
	if (bGroupFlag)
		FlushUndoGroup();
	return TRUE;
}

BOOL CCrystalTextBuffer::DeleteText(CCrystalTextView *pSource, int nStartLine, int nStartChar, int nEndLine, int nEndChar, LPCTSTR cpDesc )
{
	CString sTextToDelete;
	GetText(nStartLine, nStartChar, nEndLine, nEndChar, sTextToDelete);

	if (! InternalDeleteText(pSource, nStartLine, nStartChar, nEndLine, nEndChar))
		return FALSE;

	BOOL bGroupFlag = FALSE;
	if (! m_bUndoGroup)
	{
		BeginUndoGroup();
		bGroupFlag = TRUE;
	}
	AddUndoRecord(FALSE, CPoint(nStartChar, nStartLine), CPoint(nEndChar, nEndLine), sTextToDelete, cpDesc );
	if (bGroupFlag)
		FlushUndoGroup();
	return TRUE;
}

void CCrystalTextBuffer::SetModified(BOOL bModified /*= TRUE*/)
{
	m_bModified = bModified;
}

void CCrystalTextBuffer::BeginUndoGroup(BOOL bMergeWithPrevious /*= FALSE*/)
{
	ASSERT(! m_bUndoGroup);
	m_bUndoGroup = TRUE;
	m_bUndoBeginGroup = m_nUndoPosition == 0 || ! bMergeWithPrevious;
}

void CCrystalTextBuffer::FlushUndoGroup()
{
	ASSERT(m_bUndoGroup);
	m_bUndoGroup = FALSE;
}
