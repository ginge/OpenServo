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

#include "StdAfx.h"
#include "Frame.h"
#include "DataView.h"
#include "TwiBootProgDoc.h"

CDataView *gpDataView;

CFrame::CFrame()
: m_Size(0,0), m_pDoc(NULL)
{
}

CFrame::~CFrame()
{
}

void CFrame::Init(CSize CharSize, CTwiBootProgDoc *pDoc)
{
    ASSERT_VALID(pDoc);
    m_pDoc = pDoc;
    m_StartAddr = 0;
    m_CharSize= CharSize;
    m_Size.cx = GetRowWidth();
}

void CFrame::SetRowNum(unsigned int NewRowNum)
{
    m_RowNumber = NewRowNum;
    m_Size.cy = (m_RowNumber + 1) * GetRowHeight(); // reserve one row for header
}

void CFrame::SetStartAddr(unsigned int NewStartAddr)
{
    m_StartAddr = NewStartAddr;
}

bool CFrame::DrawItem(CDC* pDc, CRect Position, bool bHeader, CString Text)
{
    COLORREF crOldText, crOldBackground;
    CPen BlackPen, WhitePen, *pOldPen;
    BlackPen.CreatePen(PS_SOLID, 1, RGB(0,0,0));
    WhitePen.CreatePen(PS_SOLID, 1, RGB(255,255,255));
    pOldPen = pDc->SelectObject(&BlackPen);

    if (bHeader)
    {
        CBrush NewBrush;
        NewBrush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
        crOldBackground = pDc->SetBkColor(::GetSysColor(COLOR_BTNFACE));
        crOldText = pDc->SetTextColor(RGB(0,0,255));
        pDc->FillRect(Position, &NewBrush);
        pDc->MoveTo(Position.left, Position.bottom-1);
        pDc->LineTo(Position.right-1, Position.bottom-1);
        pDc->LineTo(Position.right-1, Position.top-1);
        pDc->MoveTo(Position.left, Position.bottom-2);
        pDc->LineTo(Position.right-2, Position.bottom-2);
        pDc->LineTo(Position.right-2, Position.top-1);
        pDc->SelectObject(&WhitePen);
        pDc->MoveTo(Position.left, Position.bottom-3);
        pDc->LineTo(Position.left, Position.top);
        pDc->LineTo(Position.right-2, Position.top);

        pDc->DrawText(Text, Position, DT_VCENTER|DT_SINGLELINE|DT_CENTER);

        pDc->SetBkColor(crOldBackground);
        pDc->SetTextColor(crOldText);
    }
    else
    {
        pDc->MoveTo(Position.right-1, Position.top);
        pDc->LineTo(Position.right-1, Position.bottom);
        pDc->MoveTo(Position.right-2, Position.top);
        pDc->LineTo(Position.right-2, Position.bottom);
        pDc->DrawText(Text, Position, DT_VCENTER|DT_SINGLELINE|DT_CENTER);
    }
    pDc->SelectObject(pOldPen);
    return true;
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::FillDc(CDC* pDc)
{
    ASSERT_VALID(pDc);
    ASSERT(m_pDoc);
    pDc->FillSolidRect(0,0,m_Size.cx, m_Size.cy, RGB(255,255,255));
    CRect Item(0, 0, m_Size.cx, GetRowHeight());
    DrawItem(pDc, Item, true, m_Repr.GetHeaderText());
    unsigned int i;
    unsigned int CurrAddr, DocSize;
    BYTE* pBuffer = new BYTE[m_Repr.GetBytesPerRow()];
    unsigned int BufLen;
    DocSize = m_pDoc->GetBufferLength();
    for(i = 0, CurrAddr = m_StartAddr;
        (i < m_RowNumber)&&(CurrAddr < DocSize);
        i++)
    {
        Item.OffsetRect(0, GetRowHeight());
        if (CurrAddr + m_Repr.GetBytesPerRow() < m_pDoc->GetBufferLength())
            BufLen = m_Repr.GetBytesPerRow();
        else
            BufLen = m_pDoc->GetBufferLength() - CurrAddr;
        m_pDoc->GetData(pBuffer, BufLen, CurrAddr);

        DrawItem(pDc, Item, false, m_Repr.GetRowText(pBuffer, BufLen));
        CurrAddr += BufLen;
    }
    pDc->MoveTo(Item.left, Item.bottom-1);
    pDc->LineTo(Item.right, Item.bottom-1);
    pDc->MoveTo(Item.left, Item.bottom-2);
    pDc->LineTo(Item.right, Item.bottom-2);
    delete[] pBuffer;
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::Draw(CDC *pViewDc, CRect Region, int Offset)
{
    ASSERT_VALID(pViewDc);
    CDC *pDc = new CDC;
    pDc->CreateCompatibleDC(pViewDc);
    pDc->SelectStockObject(SYSTEM_FIXED_FONT);
    CBitmap *pBitmap = new CBitmap;
    pBitmap->CreateCompatibleBitmap(pViewDc, m_Size.cx, m_Size.cy);
    CBitmap *pOldBitmap = pDc->SelectObject(pBitmap);
    FillDc(pDc);
    pViewDc->BitBlt(Region.left, Region.top, Region.Width(), Region.Height(),
        pDc, Offset, 0, SRCCOPY);
    pDc->SelectObject(pOldBitmap);
    pBitmap->DeleteObject();
    delete pBitmap;
    pDc->DeleteDC();
    delete pDc;
}

template <class _Repr>
unsigned int CConcreteDataFrame<_Repr>::GetRowWidth()
{
    return m_Repr.GetCharPerRow() * m_CharSize.cx + 2 * BORDER_INDENT;
}

template <class _Repr>
bool CConcreteDataFrame<_Repr>::GetCaretPos(CPoint *pPos)
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    if ((CaretAddr < m_StartAddr) || (m_StartAddr + m_RowNumber * m_Repr.GetBytesPerRow() < CaretAddr))
        return false;
    unsigned int CaretRow = (CaretAddr - m_StartAddr) / m_Repr.GetBytesPerRow();
    unsigned int CaretRowOffset = (CaretAddr - m_StartAddr) % m_Repr.GetBytesPerRow();
    unsigned int CaretChar = m_Repr.AddrToChar(CaretRowOffset, CaretByteOffset);
    pPos->y = (CaretRow + 1) * GetRowHeight() + BORDER_INDENT;
    pPos->x = CaretChar * m_CharSize.cx + BORDER_INDENT;
    return true;
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkLeft()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    m_Repr.MoveCaretLeft(&CaretAddr, &CaretByteOffset);
    if ((CaretAddr >= 0) && (CaretAddr < m_pDoc->GetBufferLength()))
        m_pDoc->m_CaretAddr = CaretAddr, m_pDoc->m_CaretByteOffset = CaretByteOffset;
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkDown()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    if (CaretAddr + m_Repr.GetBytesPerRow() < m_pDoc->GetBufferLength())
        m_pDoc->m_CaretAddr += m_Repr.GetBytesPerRow();
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkRight()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    m_Repr.MoveCaretRight(&CaretAddr, &CaretByteOffset);
    if ((CaretAddr >= 0) && (CaretAddr < m_pDoc->GetBufferLength()))
        m_pDoc->m_CaretAddr = CaretAddr, m_pDoc->m_CaretByteOffset = CaretByteOffset;

}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkUp()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    if (CaretAddr >= m_Repr.GetBytesPerRow())
        m_pDoc->m_CaretAddr -= m_Repr.GetBytesPerRow();
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkHome()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    m_Repr.MoveCaretHome(&CaretAddr, &CaretByteOffset);
    m_pDoc->m_CaretAddr = CaretAddr;
    m_pDoc->m_CaretByteOffset = CaretByteOffset;

}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkEnd()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    m_Repr.MoveCaretEnd(&CaretAddr, &CaretByteOffset);
    if (CaretAddr >= m_pDoc->GetBufferLength())
        CaretAddr = m_pDoc->GetBufferLength() - 1;
    m_pDoc->m_CaretAddr = CaretAddr;
    m_pDoc->m_CaretByteOffset = CaretByteOffset;
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkCtrlHome()
{
    m_pDoc->m_CaretAddr = 0;
    OnVkHome();
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnVkCtrlEnd()
{
    m_pDoc->m_CaretAddr = m_pDoc->GetBufferLength() - 1;
    OnVkEnd();
}

template <class _Repr>
void CConcreteDataFrame<_Repr>::OnChar(char ch)
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretByteOffset = m_pDoc->m_CaretByteOffset;
    BYTE Data;
    VERIFY(m_pDoc->GetData(&Data, 1, CaretAddr));
    if (m_Repr.OnChar(&Data, CaretByteOffset, ch))
    {
        VERIFY(m_pDoc->SetData(&Data, 1, CaretAddr));
        OnVkRight();
    }
}

template <class _Repr>
bool CConcreteDataFrame<_Repr>::OnLButtonDown(CPoint point)
{
    unsigned int RowNum = point.y / GetRowHeight() - 1;
    if (point.x < BORDER_INDENT)
        point.x = 0;
    else
        point.x -= BORDER_INDENT;
    unsigned int CharNum = point.x / m_CharSize.cx;
    if (CharNum >= m_Repr.GetCharPerRow())
        CharNum = m_Repr.GetCharPerRow() - 1;
    unsigned int CharAddr, ByteOffset;
    m_Repr.CharToAddr(&CharAddr, &ByteOffset, CharNum);
    unsigned int NewCaretAddr = m_StartAddr + RowNum*m_Repr.GetBytesPerRow() + CharAddr;
    if (NewCaretAddr >= m_pDoc->GetBufferLength())
        NewCaretAddr = m_pDoc->GetBufferLength() - 1;
    m_pDoc->m_CaretAddr = NewCaretAddr;
    m_pDoc->m_CaretByteOffset = ByteOffset;
    return true;
}

template <class _Repr>
void CConcreteAddrFrame<_Repr>::FillDc(CDC* pDc)
{
    ASSERT_VALID(pDc);
    ASSERT(m_pDoc);
    pDc->FillSolidRect(0,0,m_Size.cx, m_Size.cy, RGB(255,255,255));
    CRect Item(0, 0, m_Size.cx, GetRowHeight());
    DrawItem(pDc, Item, true, m_Repr.GetAddrHeader());
    unsigned int i;
    unsigned int CurrAddr, DocSize;
    DocSize = m_pDoc->GetBufferLength();
    for(i = 0, CurrAddr = m_StartAddr;
        (i < m_RowNumber)&&(CurrAddr < DocSize);
        i++, CurrAddr+=GetBytesPerRow())
    {
        Item.OffsetRect(0, GetRowHeight());
        DrawItem(pDc, Item, true, m_Repr.GetAddrStr(CurrAddr));
    }
}

template <class _Repr>
void CConcreteAddrFrame<_Repr>::Draw(CDC *pViewDc, CRect Region, int Offset)
{
    ASSERT_VALID(pViewDc);
    CDC *pDc = new CDC;
    pDc->CreateCompatibleDC(pViewDc);
    pDc->SelectStockObject(SYSTEM_FIXED_FONT);
    CBitmap *pBitmap = new CBitmap;
    pBitmap->CreateCompatibleBitmap(pViewDc, m_Size.cx, m_Size.cy);
    CBitmap *pOldBitmap = pDc->SelectObject(pBitmap);
    FillDc(pDc);
    pViewDc->BitBlt(Region.left, Region.top, Region.Width(), Region.Height(),
        pDc, Offset, 0, SRCCOPY);
    pDc->SelectObject(pOldBitmap);
    pBitmap->DeleteObject();
    delete pBitmap;
    pDc->DeleteDC();
    delete pDc;
}

void CByteView::Draw(CDC *pViewDc, CRect Region, int Offset)
{
    if (Offset < m_ByteData.GetSize().cx)
    {
        m_ByteData.Draw(pViewDc, Region, Offset);
        Region.left = Region.left + m_ByteData.GetSize().cx - Offset;
        Offset = 0;
    }
    else
    {
        Offset -= m_ByteData.GetSize().cx;
    }
    if (Region.Width() > 0)
        m_AsciiData.Draw(pViewDc, Region, Offset);
}

void CByteView::Init(CSize CharSize, CTwiBootProgDoc *pDoc)
{
    m_ByteData.Init(CharSize, pDoc);
    m_AsciiData.Init(CharSize, pDoc);
}

CSize CByteView::GetSize()
{
    CSize size = m_ByteData.GetSize();
    size.cx += m_AsciiData.GetSize().cx;
    return size;
}

void CByteView::SetRowNum(unsigned int NewRowNum)
{
    m_ByteData.SetRowNum(NewRowNum);
    m_AsciiData.SetRowNum(NewRowNum);
}

void CByteView::SetStartAddr(unsigned int NewStartAddr)
{
    m_ByteData.SetStartAddr(NewStartAddr);
    m_AsciiData.SetStartAddr(NewStartAddr);
}

unsigned int CByteView::GetRowWidth()
{
    return m_ByteData.GetSize().cx + m_AsciiData.GetSize().cx;
}

bool CByteView::GetCaretPos(CPoint *pPos)
{
    bool bRet = false;
    switch(m_ActiveFrame)
    {
    case afByte:
        bRet = m_ByteData.GetCaretPos(pPos);
        break;
    case afAscii:
        bRet = m_AsciiData.GetCaretPos(pPos);
        pPos->x += m_ByteData.GetSize().cx;
        break;
    default:
        ASSERT(FALSE);
    }
    return bRet;
}

bool CByteView::OnLButtonDown(CPoint point)
{
    if (point.x < m_ByteData.GetSize().cx)
    {
        m_ActiveFrame = afByte;
        return m_ByteData.OnLButtonDown(point);
    }
    else
    {
        m_ActiveFrame = afAscii;
        point.x -= m_ByteData.GetSize().cx;
        return m_AsciiData.OnLButtonDown(point);
    }
}

CScreen::CScreen()
: m_pAddrFrame(NULL), m_pDataFrame(NULL), m_StartAddr(0), m_HorzOffset(0),
m_pDoc(NULL), m_bInited(false)
{
}


CScreen::~CScreen()
{
    if (m_pDataFrame)
        delete m_pDataFrame;
    if (m_pAddrFrame)
        delete m_pAddrFrame;
}

void CScreen::Draw(CDC *pViewDc)
{
    if (!m_bInited)
        return;
    ASSERT(m_pAddrFrame); ASSERT(m_pDataFrame);
    int AddrWidth = m_pAddrFrame->GetSize().cx;
    ASSERT(AddrWidth <= m_Size.cx);
    CRect DrawRegion(0, 0, AddrWidth, m_Size.cy);
    m_pAddrFrame->Draw(pViewDc, DrawRegion, 0);
    DrawRegion.left = AddrWidth;
    DrawRegion.right = m_Size.cx;
    m_pDataFrame->Draw(pViewDc, DrawRegion, m_HorzOffset);

    DrawRegion.left = AddrWidth + m_pDataFrame->GetSize().cx - m_HorzOffset;
    if (DrawRegion.Width()>0)
        pViewDc->FillSolidRect(DrawRegion, RGB(255,255,255));
}

void CScreen::Init(CSize CharSize, CTwiBootProgDoc *pDoc, CSize size)
{
    ASSERT_VALID(pDoc);
    m_pDoc = pDoc;
    m_Size = size;
    m_StartAddr = 0;
    m_CharSize = CharSize;
    m_bInited = true;
    SetMode(ModeByte);
}

void CScreen::SetMode(UINT mode)
{
    if (!m_bInited)
        return;
    if (m_pDataFrame) delete m_pDataFrame, m_pDataFrame = NULL;
    if (m_pAddrFrame) delete m_pAddrFrame, m_pAddrFrame = NULL;
    switch(mode)
    {
    case ModeNibble:
        m_pDataFrame = new CConcreteDataFrame<CNibbleRepr>;
        m_pAddrFrame = new CConcreteAddrFrame<CNibbleRepr>;
        break;
    case ModeByte:
        m_pDataFrame = new CByteView;
        m_pAddrFrame = new CConcreteAddrFrame<CByteRepr>;
        break;
    case ModeOctal:
        m_pDataFrame = new CConcreteDataFrame<COctalRepr>;
        m_pAddrFrame = new CConcreteAddrFrame<COctalRepr>;
        break;
    case ModeDecimal:
        m_pDataFrame = new CConcreteDataFrame<CDecimalRepr>;
        m_pAddrFrame = new CConcreteAddrFrame<CDecimalRepr>;
        break;
    case ModeWordB0B1:
        m_pDataFrame = new CConcreteDataFrame<CWordB0B1Repr>;
        m_pAddrFrame = new CConcreteAddrFrame<CWordB0B1Repr>;
        break;
    case ModeWordB1B0:
        m_pDataFrame = new CConcreteDataFrame<CWordB1B0Repr>;
        m_pAddrFrame = new CConcreteAddrFrame<CWordB1B0Repr>;
        break;
    default:
        ASSERT(FALSE);
        m_pDataFrame = new CByteView;
        m_pAddrFrame = new CConcreteAddrFrame<CByteRepr>;
        break;
    }
    m_pDataFrame->Init(m_CharSize, m_pDoc);
    m_pAddrFrame->Init(m_CharSize, m_pDoc);
    SetSize(m_Size);
    SetStartAddr(m_StartAddr);
}

void CScreen::SetSize(CSize NewSize)
{
    if (!m_bInited)
        return;
    m_Size = NewSize;
    if (m_Size.cy > GetRowHeight())
    {
        m_VisibleRowNum = m_Size.cy /GetRowHeight() - 1;
        m_pAddrFrame->SetRowNum(m_VisibleRowNum+1);
        m_pDataFrame->SetRowNum(m_VisibleRowNum+1);
    }
}

unsigned int CScreen::GetRealWidth()
{
    if (!m_bInited)
        return 0;
    return m_pAddrFrame->GetSize().cx + m_pDataFrame->GetSize().cx;
}

unsigned int CScreen::GetRealRowNum()
{
    if (!m_bInited)
        return 0;
    ASSERT_VALID(m_pDoc);
    unsigned int DocLen, BytesPerRow;
    DocLen = m_pDoc->GetBufferLength();
    BytesPerRow = m_pDataFrame->GetBytesPerRow();
    if (DocLen%BytesPerRow == 0)
        return DocLen/BytesPerRow;
    else
        return DocLen/BytesPerRow + 1;
}

unsigned int CScreen::GetFirstRow()
{
    if (!m_bInited)
        return 0;
    return m_StartAddr/m_pDataFrame->GetBytesPerRow();
}

void CScreen::SetFirstRow(unsigned int NewFirstRow)
{
    if (!m_bInited)
        return;
    unsigned int StartAddr = NewFirstRow * m_pDataFrame->GetBytesPerRow();
    ASSERT(StartAddr < m_pDoc->GetBufferLength());
    SetStartAddr(StartAddr);
}

void CScreen::SetStartAddr(unsigned int NewStartAddr)
{
    if (!m_bInited)
        return;
    NewStartAddr = (NewStartAddr / m_pDataFrame->GetBytesPerRow()) * m_pDataFrame->GetBytesPerRow();
    m_StartAddr = NewStartAddr;
    m_pDataFrame->SetStartAddr(m_StartAddr);
    m_pAddrFrame->SetStartAddr(m_StartAddr);

}

bool CScreen::GetCaretPos(CPoint *pPos)
{
    if (!m_bInited)
        return false;
    if (!m_pDataFrame->GetCaretPos(pPos))
        return false;
    pPos->x += m_pAddrFrame->GetSize().cx;
    pPos->x -= m_HorzOffset;
    if ((pPos->x > m_Size.cx - m_CharSize.cx) || (pPos->x < 0)
        || (pPos->y > m_Size.cy - m_CharSize.cy) || (pPos->y < 0))
        return false;
    return true;
}

bool CScreen::ScrollToCaret()
{
    if (!m_bInited)
        return false;
    bool retVal = false;
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int CaretRow = CaretAddr / m_pDataFrame->GetBytesPerRow();
    unsigned int StartRow = m_StartAddr / m_pDataFrame->GetBytesPerRow();
    if (CaretAddr < m_StartAddr)
        SetStartAddr(CaretRow * m_pDataFrame->GetBytesPerRow()), retVal = true;
    if (CaretRow >= StartRow + m_VisibleRowNum)
        SetStartAddr((CaretRow - m_VisibleRowNum + 1) * m_pDataFrame->GetBytesPerRow()), retVal = true;
    CPoint pos;
    VERIFY(m_pDataFrame->GetCaretPos(&pos));
    if (pos.x < m_HorzOffset)
        m_HorzOffset = pos.x, retVal = true;
    if (m_Size.cx < pos.x + m_CharSize.cx - m_HorzOffset)
        m_HorzOffset = pos.x + m_CharSize.cx - m_Size.cx, retVal = true;
    return retVal;
}

void CScreen::OnPageUp()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int BytesPerRow = m_pDataFrame->GetBytesPerRow();
    unsigned int BytesPerPage = BytesPerRow * m_VisibleRowNum;
    if (CaretAddr >= BytesPerPage)
        CaretAddr -= BytesPerPage;
    else
        CaretAddr = CaretAddr % BytesPerRow;
    m_pDoc->m_CaretAddr = CaretAddr;
}

void CScreen::OnPageDown()
{
    unsigned int CaretAddr = m_pDoc->m_CaretAddr;
    unsigned int BytesPerRow = m_pDataFrame->GetBytesPerRow();
    unsigned int BytesPerPage = BytesPerRow * m_VisibleRowNum;
    unsigned int DocSize = m_pDoc->GetBufferLength();
    if (CaretAddr < DocSize - BytesPerPage)
        CaretAddr += BytesPerPage;
    else
        CaretAddr = CaretAddr % BytesPerRow + ((DocSize-1)/BytesPerRow)*BytesPerRow;
    m_pDoc->m_CaretAddr = CaretAddr;

}

bool CScreen::OnLButtonDown(CPoint point)
{
    if (point.y < GetRowHeight())
        return false;
    if (point.x < m_pAddrFrame->GetSize().cx)
        point.x = 0;
    else
        point.x -= m_pAddrFrame->GetSize().cx;
    if (point.x >= m_pDataFrame->GetSize().cx)
        point.x = m_pDataFrame->GetSize().cx - 1;
    return m_pDataFrame->OnLButtonDown(point);
}