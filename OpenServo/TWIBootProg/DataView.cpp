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

#include "stdafx.h"
#include "TwiBootProg.h"
#include "DataView.h"
#include "MainFrm.h"
#include "DeviceManager.h"
#include "I2cBridge/u2c_common_func.h"

bool gTest;

// CDataView

IMPLEMENT_DYNCREATE(CDataView, CView)

CDataView::CDataView()
{
    gTest = false;
}

CDataView::~CDataView()
{
}

BEGIN_MESSAGE_MAP(CDataView, CView)
    ON_WM_VSCROLL()
//  ON_WM_HSCROLL()
    ON_WM_SIZE()
    ON_WM_HSCROLL()
    ON_WM_CREATE()
    ON_WM_ACTIVATE()
    ON_COMMAND_RANGE(ID_FIRST_MODE, ID_LAST_MODE, OnSetMode)
    ON_UPDATE_COMMAND_UI_RANGE(ID_FIRST_MODE, ID_LAST_MODE, OnUpdateMode)
    ON_COMMAND(ID_DEVICE_PROGRAM, OnDeviceProgram)
    ON_COMMAND(ID_DEVICE_READ, OnDeviceRead)
    ON_COMMAND(ID_DEVICE_VERIFY, OnDeviceVerify)
    ON_COMMAND(ID_DEVICE_ERASE, OnDeviceErase)
    ON_COMMAND(ID_DEVICE_CONFIG, OnDeviceConfig)
    ON_COMMAND(ID_VK_LEFT, OnVkLeft)
    ON_COMMAND(ID_VK_DOWN, OnVkDown)
    ON_COMMAND(ID_VK_RIGHT, OnVkRight)
    ON_COMMAND(ID_VK_UP, OnVkUp)
    ON_COMMAND(ID_VK_PAGE_UP, OnPageUp)
    ON_COMMAND(ID_VK_PAGE_DOWN, OnPageDown)
    ON_COMMAND(ID_VK_HOME, OnVkHome)
    ON_COMMAND(ID_VK_END, OnVkEnd)
    ON_COMMAND(ID_VK_CTRL_HOME, OnVkCtrlHome)
    ON_COMMAND(ID_VK_CTRL_END, OnVkCtrlEnd)
    ON_COMMAND(ID_VK_TAB, OnVkTab)
    ON_WM_CHAR()
    ON_WM_MOUSEWHEEL()
    ON_WM_LBUTTONDOWN()
    ON_MESSAGE(WM_DEVICE_FEEDBACK, OnDeviceFeedback)
END_MESSAGE_MAP()





void CDataView::OnDraw(CDC* pDC)
{
    m_Screen.Draw(pDC);
}



// CDataView diagnostics

#ifdef _DEBUG
void CDataView::AssertValid() const
{
    CView::AssertValid();
}

void CDataView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}
#endif //_DEBUG


// CDataView message handlers

void CDataView::InitScreen()
{
    CRect clRect;
    GetClientRect(&clRect);
    CDC *pDc = GetDC();
    CFont *pOldFont = dynamic_cast<CFont*>(pDc->SelectStockObject(SYSTEM_FIXED_FONT));
    TEXTMETRIC tm;
    pDc->GetTextMetrics(&tm);

    m_Screen.Init(CSize(tm.tmAveCharWidth, tm.tmHeight), GetDocument(), clRect.Size());

    pDc->SelectObject(pOldFont);
    ReleaseDC(pDc);

    UpdateVertScrollBar();
    UpdateHorzScrollBar();
    UpdateCaret();

}

#define MIIM_FTYPE       0x00000100
void CDataView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    // TODO: Add your specialized code here and/or call the base class
    ShowScrollBar(SB_BOTH, TRUE);
    EnableScrollBarCtrl(SB_VERT, TRUE);
    EnableScrollBarCtrl(SB_HORZ, TRUE);
    InitScreen();
    UpdateCaret();

    OnSetMode(ID_BYTE_MODE);

    SetTimer(1, 100, NULL);
}

void CDataView::ScrollToCaret()
{
    if (m_Screen.ScrollToCaret())
        UpdateView();
    UpdateCaret();
}


void CDataView::UpdateCaret()
{
    CSize CharSize = m_Screen.GetCharSize();
    CreateSolidCaret(CharSize.cx, CharSize.cy);
    CPoint CaretPos;
    if (m_Screen.GetCaretPos(&CaretPos))
    {
        SetCaretPos(CaretPos);
        ShowCaret();
    }
    else
        HideCaret();
}

void CDataView::VertScroll(UINT nSBCode)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(SB_VERT, &si));
    gTest = true;

    unsigned int NewFirstRow = 0;
    switch(nSBCode)
    {
    case SB_TOP:
        NewFirstRow = 0;
        break;
    case SB_BOTTOM:
        NewFirstRow = m_Screen.GetRealRowNum() - m_Screen.GetVisibleRowNum() + 1;
        break;
    case SB_LINEUP:
        if (m_Screen.GetFirstRow() != 0)
            NewFirstRow = m_Screen.GetFirstRow() - 1;
        else
            NewFirstRow = m_Screen.GetFirstRow();
        break;
    case SB_LINEDOWN:
        NewFirstRow = m_Screen.GetFirstRow() + 1;
        break;
    case SB_PAGEUP:
        if (m_Screen.GetFirstRow() >= si.nPage)
            NewFirstRow = m_Screen.GetFirstRow() - si.nPage;
        else
            NewFirstRow = 0;
        break;
    case SB_PAGEDOWN:
        NewFirstRow = m_Screen.GetFirstRow() + si.nPage;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        NewFirstRow = si.nTrackPos;
        break;
    default:
        return;
    }
    ScrollToRow(NewFirstRow);
}
void CDataView::HorzScroll(UINT nSBCode)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    VERIFY(GetScrollInfo(SB_HORZ, &si));

    unsigned int NewHorzOffset = 0;
    CRect clRect;
    GetClientRect(clRect);
    int RealWidth = m_Screen.GetRealWidth();
    switch(nSBCode)
    {
    case SB_TOP:
        NewHorzOffset = 0;
        break;
    case SB_BOTTOM:
        NewHorzOffset = RealWidth - clRect.Width();
        break;
    case SB_LINEUP:
        if (m_Screen.GetHorzOffset() != 0)
            NewHorzOffset = m_Screen.GetHorzOffset() - 1;
        else
            NewHorzOffset = m_Screen.GetHorzOffset();
        break;
    case SB_LINEDOWN:
        NewHorzOffset = m_Screen.GetHorzOffset() + 1;
        break;
    case SB_PAGEUP:
        NewHorzOffset = m_Screen.GetHorzOffset() - si.nPage + 1;
        break;
    case SB_PAGEDOWN:
        NewHorzOffset = m_Screen.GetHorzOffset() + si.nPage - 1;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        NewHorzOffset = si.nTrackPos;
        break;
    default:
        return;
    }
    ScrollToOffset(NewHorzOffset);
}


void CDataView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CView::OnHScroll(nSBCode, nPos, pScrollBar);
    HorzScroll(nSBCode);
}


void CDataView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CView::OnVScroll(nSBCode, nPos, pScrollBar);
    VertScroll(nSBCode);
}
void CDataView::ScrollToOffset(unsigned int NewHorzOffset)
{
    unsigned int RealWidth = m_Screen.GetRealWidth();
    if (RealWidth - m_Screen.GetVisibleWidth() < NewHorzOffset)
        NewHorzOffset = RealWidth - m_Screen.GetVisibleWidth() + 1;
    if (NewHorzOffset == m_Screen.GetHorzOffset())
        return;
    m_Screen.SetHorzOffset(NewHorzOffset);
    Invalidate(FALSE);
    UpdateWindow();
    UpdateHorzScrollBar(true);
}

void CDataView::ScrollToRow(unsigned int NewFirstRaw)
{
    if (m_Screen.GetRealRowNum()-m_Screen.GetVisibleRowNum() < NewFirstRaw)
        NewFirstRaw = m_Screen.GetRealRowNum()-m_Screen.GetVisibleRowNum();
    if (NewFirstRaw == m_Screen.GetFirstRow())
        return;
    m_Screen.SetFirstRow(NewFirstRaw);
    Invalidate(FALSE);
    UpdateWindow();
    UpdateVertScrollBar(true);
}

void CDataView::UpdateHorzScrollBar(bool bPositionOnly /*= false*/)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    if (bPositionOnly)
    {
        si.fMask = SIF_POS;
        si.nPos = m_Screen.GetHorzOffset();
    }
    else
    {
        if (m_Screen.GetVisibleWidth() >= m_Screen.GetRealWidth())
        {
            m_Screen.SetHorzOffset(0);
            Invalidate(FALSE);
        }
        si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin = 0;
        si.nMax = m_Screen.GetRealWidth();
        si.nPage = m_Screen.GetVisibleWidth();
        si.nPos = m_Screen.GetHorzOffset();
    }
    VERIFY(SetScrollInfo(SB_HORZ, &si));
    UpdateCaret();
}
void CDataView::UpdateVertScrollBar(bool bPositionOnly /*= false*/)
{
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    if (bPositionOnly)
    {
        si.fMask = SIF_POS;
        si.nPos = m_Screen.GetFirstRow();
    }
    else
    {
        if (m_Screen.GetVisibleRowNum() >= m_Screen.GetRealRowNum())
        {
            m_Screen.SetFirstRow(0);
            Invalidate(FALSE);
        }
        si.fMask = SIF_DISABLENOSCROLL | SIF_PAGE | SIF_POS | SIF_RANGE;
        si.nMin = 0;
        si.nMax = m_Screen.GetRealRowNum() - 1;
        si.nPage = m_Screen.GetVisibleRowNum();
        si.nPos = m_Screen.GetFirstRow();
    }
    VERIFY(SetScrollInfo(SB_VERT, &si));
    UpdateCaret();
}

//void CDataView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//  // TODO: Add your message handler code here and/or call default
//
//  CView::OnHScroll(nSBCode, nPos, pScrollBar);
//}

void CDataView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
    UpdateView();
}



void CDataView::UpdateView()
{
    Invalidate(FALSE);
    UpdateVertScrollBar();
    UpdateHorzScrollBar();
    UpdateCaret();
}


void CDataView::OnSize(UINT nType, int cx, int cy)
{
    CView::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    m_Screen.SetSize(CSize(cx, cy));
    UpdateView();
}

int CDataView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CView::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  Add your specialized creation code here

    return 0;
}

BOOL CDataView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
    // TODO: Add your specialized code here and/or call the base class

    return CView::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

BOOL CDataView::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam)
{
    // TODO: Add your specialized code here and/or call the base class

    return CView::CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, lpParam);
}

void CDataView::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CView::OnActivate(nState, pWndOther, bMinimized);

    // TODO: Add your message handler code here
}

void CDataView::OnSetMode(UINT nId)
{
    m_CurrModeId = nId;
    CWnd* pMain = AfxGetMainWnd();
    if (pMain != NULL)
    {
        CMenu* pMenu = pMain->GetMenu();
        pMenu->CheckMenuRadioItem(ID_FIRST_MODE, ID_LAST_MODE, nId, MF_BYCOMMAND);
    }
    m_Screen.SetMode(nId - ID_FIRST_MODE);
    UpdateView();
}

void CDataView::OnUpdateMode(CCmdUI* pCmdUI)
{
    if (pCmdUI->m_nID == m_CurrModeId)
        pCmdUI->SetCheck(TRUE);
    else
        pCmdUI->SetCheck(FALSE);
}



void CDataView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: Add your message handler code here and/or call default
    ASSERT(nChar < 0xFF);
    m_Screen.OnChar(char(nChar));
    ScrollToCaret();
    CView::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CDataView::OnMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
{
    // TODO: Add your message handler code here and/or call default
    UINT ScrollRows = abs(zDelta/WHEEL_DELTA);
    UINT FirstRow = m_Screen.GetFirstRow();
    if (zDelta > 0)
    {
        if (ScrollRows > FirstRow)
            ScrollToRow(0);
        else
            ScrollToRow(FirstRow - ScrollRows);
    }
    else
        ScrollToRow(FirstRow + ScrollRows);
    return TRUE;
}

void CDataView::OnDeviceProgram()
{
    CDeviceManager *pMan = CDeviceManager::Instance();
    CTwiBootProgDoc *pDoc = GetDocument();
    if ((pMan == NULL) || (pDoc == NULL))
    {
        ASSERT(FALSE);
        return;
    }
    if (!pMan->Program(pDoc->GetBuffer(), pDoc->GetBufferLength(), m_hWnd))
    {
        ASSERT(FALSE);
        return;
    }
    SetState(sProgramming);
}
void CDataView::OnDeviceRead()
{
    CDeviceManager *pMan = CDeviceManager::Instance();
    CTwiBootProgDoc *pDoc = GetDocument();
    if ((pMan == NULL) || (pDoc == NULL))
    {
        ASSERT(FALSE);
        return;
    }
    if (!pMan->Read(pDoc->GetBuffer(), pDoc->GetBufferLength(), m_hWnd))
    {
        ASSERT(FALSE);
        return;
    }
    SetState(sReading);
}
void CDataView::OnDeviceVerify()
{
    CDeviceManager *pMan = CDeviceManager::Instance();
    CTwiBootProgDoc *pDoc = GetDocument();
    if ((pMan == NULL) || (pDoc == NULL))
    {
        ASSERT(FALSE);
        return;
    }
    if (!pMan->Verify(pDoc->GetBuffer(), pDoc->GetBufferLength(), m_hWnd))
    {
        ASSERT(FALSE);
        return;
    }
    SetState(sVerifying);
}

void CDataView::OnDeviceConfig()
{
    CDeviceManager *pMan = CDeviceManager::Instance();
    if (!pMan->Configure())
    {
        ASSERT(FALSE);
        return;
    }
}

void CDataView::OnDeviceErase()
{
    CDeviceManager *pMan = CDeviceManager::Instance();
    if (pMan == NULL)
    {
        ASSERT(FALSE);
        return;
    }
    if (!pMan->Erase(m_hWnd))
    {
        ASSERT(FALSE);
        return;
    }
    SetState(sErasing);
}

CMainFrame* CDataView::GetMainFrame()
{
    CWnd *pMainWnd = AfxGetMainWnd();
    return dynamic_cast<CMainFrame*>(pMainWnd);

}

LRESULT CDataView::OnDeviceFeedback(WPARAM DeviceEvent, LPARAM Parameter)
{
    CMainFrame *pMainFrame = GetMainFrame();
    ASSERT_VALID(pMainFrame);
    switch(DeviceEvent)
    {
    case DF_U2C_OPEN_FAILED:
        pMainFrame->SetStatusString(IDS_STATUS_FAILED_OPEN_U2C);
        SetState(sIdle);
        break;
    case DF_READ_FAILED:
        AfxMessageBox(IDS_ERR_READ_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_READ_FAILED);
        SetState(sIdle);
        break;
    case DF_PROGRAM_FAILED:
        AfxMessageBox(IDS_ERR_PROGRAM_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_PROGRAM_FAILED);
        SetState(sIdle);
        break;
    case DF_VERIFY_FAILED:
        AfxMessageBox(IDS_ERR_VERIFY_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_VERIFY_FAILED);
        SetState(sIdle);
        break;
    case DF_ERASE_FAILED:
        AfxMessageBox(IDS_ERR_ERASE_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_ERASE_FAILED);
        SetState(sIdle);
        break;
    case DF_READ_PROCEED:
        pMainFrame->SetProgress(IDS_STATUS_READING, (int) Parameter);
        break;
    case DF_PROGRAM_PROCEED:
        pMainFrame->SetProgress(IDS_STATUS_PROGRAMMING, (int) Parameter);
        break;
    case DF_VERIFY_PROCEED:
        pMainFrame->SetProgress(IDS_STATUS_VERIFYING, (int) Parameter);
        break;
    case DF_ERASE_PROCEED:
        pMainFrame->SetProgress(IDS_STATUS_ERASING, (int) Parameter);
        break;
    case DF_READ_FINISHED:
        pMainFrame->SetStatusString(IDS_STATUS_READ_SUCCEED);
        SetState(sIdle);
        UpdateView();
        break;
    case DF_PROGRAM_FINISHED:
        pMainFrame->SetStatusString(IDS_STATUS_PROGRAM_SUCCEED);
        SetState(sIdle);
        break;
    case DF_VERIFY_FINISHED:
        pMainFrame->SetStatusString(IDS_STATUS_VERIFY_SUCCEED);
        SetState(sIdle);
        break;
    case DF_ERASE_FINISHED:
        pMainFrame->SetStatusString(IDS_STATUS_ERASE_SUCCEED);
        SetState(sIdle);
        break;
    case DF_CHIP_NOT_FOUND:
        AfxMessageBox(IDS_ERR_CHIP_NOT_FOUND);
        pMainFrame->SetStatusString(IDS_STATUS_CHIP_NOT_FOUND);
        SetState(sIdle);
        break;
    case DF_INVALID_PARAMETER:
        ASSERT(FALSE);
        AfxMessageBox(IDS_ERR_OPERATION_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_OPERATION_FAILED);
        SetState(sIdle);
        break;
    case DF_U2C_BUSY:
        AfxMessageBox(IDS_ERR_U2C_BUSY);
        break;
    case DF_INTERNAL_ERROR:
        AfxMessageBox(IDS_ERR_OPERATION_FAILED);
        pMainFrame->SetStatusString(IDS_STATUS_OPERATION_FAILED);
        SetState(sIdle);
        break;
    case DF_OPERATION_CANCELLED:
        pMainFrame->SetStatusString(IDS_STATUS_OPERATION_CANCELLED);
        SetState(sIdle);
        break;
    case DF_OUTDATED_DRIVER_VER:
        pMainFrame->SetStatusString(IDS_STATUS_OUTDATED_DRIVER_VER);
        SetState(sIdle);
        break;
    case DF_U2C_NOT_IMPLEMENTED:
        ShowNotImplementedMessage();
        SetState(sIdle);
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    return 0;
}

void CDataView::SetState(EState NewState)
{
    CMainFrame *pFrame = GetMainFrame();
    ASSERT_VALID(pFrame);
    CTwiBootProgDoc *pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    m_State = NewState;
    switch(NewState)
    {
    case sIdle:
        pFrame->ShowProgress(FALSE);
        pDoc->SetChangeEnabled(true);
        break;
    case sProgramming:
    case sReading:
    case sErasing:
    case sVerifying:
        pFrame->ShowProgress(TRUE);
        pDoc->SetChangeEnabled(false);
        break;
    default:
        ASSERT(FALSE);
        break;
    }
}
