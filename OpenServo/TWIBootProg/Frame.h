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

#pragma once
#include "Representation.h"

class CTwiBootProgDoc;

class CFrame
{
public:
    CFrame();
    virtual ~CFrame();
    virtual void Draw(CDC *pViewDc, CRect Region, int Offset=0) = 0;
    virtual void Init(CSize CharSize, CTwiBootProgDoc *pDoc);
    virtual CSize GetSize() {return m_Size;};
    virtual void SetRowNum(unsigned int NewRowNum);
    virtual void SetStartAddr(unsigned int NewStartAddr);
    virtual unsigned int GetRowHeight() {return m_CharSize.cy + BORDER_INDENT * 2;};
    virtual unsigned int GetRowWidth() = 0;
protected:
    bool DrawItem(CDC* pDc, CRect Position, bool bHeader, CString Text);
    CTwiBootProgDoc *m_pDoc;
    CSize m_Size;
    unsigned int m_RowNumber;
    unsigned int m_StartAddr;
    CSize m_CharSize;
};

class CDataFrame: public CFrame
{
public:
    CDataFrame(){};
    virtual ~CDataFrame(){};

    virtual unsigned int GetBytesPerRow() = 0;
    virtual bool GetCaretPos(CPoint *pPos) = 0; // returns false if caret is out of view
    virtual void OnVkLeft() = 0;
    virtual void OnVkDown() = 0;
    virtual void OnVkRight() = 0;
    virtual void OnVkUp() = 0;
    virtual void OnVkHome() = 0;
    virtual void OnVkEnd() = 0;
    virtual void OnVkCtrlHome() = 0;
    virtual void OnVkCtrlEnd() = 0;
    virtual void OnVkTab() = 0;
    virtual void OnChar(char ch) = 0;
    virtual bool OnLButtonDown(CPoint point) = 0;
protected:
};

template <class _Repr>
class CConcreteDataFrame:public CDataFrame
{
public:
    virtual unsigned int GetBytesPerRow() {return m_Repr.GetBytesPerRow();};
    virtual unsigned int GetRowWidth();
    virtual void Draw(CDC *pViewDc, CRect Region, int Offset=0);
    virtual bool GetCaretPos(CPoint *pPos); // returns false if caret is out of view

    virtual void OnVkLeft();
    virtual void OnVkDown();
    virtual void OnVkRight();
    virtual void OnVkUp();
    virtual void OnVkHome();
    virtual void OnVkEnd();
    virtual void OnVkCtrlHome();
    virtual void OnVkCtrlEnd();
    virtual void OnVkTab(){};
    virtual void OnChar(char ch);
    virtual bool OnLButtonDown(CPoint point);

protected:
    virtual void FillDc(CDC* pDc);
    _Repr m_Repr;
};

class CByteView: public CDataFrame
{
public:
    CByteView():m_ActiveFrame(afByte){};
    virtual ~CByteView(){};
    unsigned int GetBytesPerRow(){return m_ByteData.GetBytesPerRow();};

    virtual void Draw(CDC *pViewDc, CRect Region, int Offset=0);
    virtual void Init(CSize CharSize, CTwiBootProgDoc *pDoc);
    CSize GetSize();
    void SetRowNum(unsigned int NewRowNum);
    void SetStartAddr(unsigned int NewStartAddr);
    virtual unsigned int GetRowWidth();

    virtual bool GetCaretPos(CPoint *pPos); // returns false if caret is out of view
    virtual void OnVkLeft(){if (m_ActiveFrame==afByte) m_ByteData.OnVkLeft(); else m_AsciiData.OnVkLeft();};
    virtual void OnVkDown(){if (m_ActiveFrame==afByte) m_ByteData.OnVkDown(); else m_AsciiData.OnVkDown();};
    virtual void OnVkRight(){if (m_ActiveFrame==afByte) m_ByteData.OnVkRight(); else m_AsciiData.OnVkRight();};
    virtual void OnVkUp(){if (m_ActiveFrame==afByte) m_ByteData.OnVkUp(); else m_AsciiData.OnVkUp();};
    virtual void OnVkHome(){if (m_ActiveFrame==afByte) m_ByteData.OnVkHome(); else m_AsciiData.OnVkHome();};
    virtual void OnVkEnd(){if (m_ActiveFrame==afByte) m_ByteData.OnVkEnd(); else m_AsciiData.OnVkEnd();};
    virtual void OnVkCtrlHome(){if (m_ActiveFrame==afByte) m_ByteData.OnVkCtrlHome(); else m_AsciiData.OnVkCtrlHome();};
    virtual void OnVkCtrlEnd(){if (m_ActiveFrame==afByte) m_ByteData.OnVkCtrlEnd(); else m_AsciiData.OnVkCtrlEnd();};
    virtual void OnVkTab(){if (m_ActiveFrame==afByte) m_ActiveFrame=afAscii; else m_ActiveFrame=afByte;};
    virtual void OnChar(char ch){if (m_ActiveFrame==afByte) m_ByteData.OnChar(ch); else m_AsciiData.OnChar(ch);};
    virtual bool OnLButtonDown(CPoint point);

private:
    CConcreteDataFrame<CByteRepr> m_ByteData;
    CConcreteDataFrame<CAsciiRepr> m_AsciiData;

    CString GetHeaderText(){ASSERT(FALSE);return "";};
    CString GetRowText(unsigned int){ASSERT(FALSE); return "";};

    enum ActiveFrame { afByte, afAscii} m_ActiveFrame;

};




class CAddrFrame: public CFrame
{
public:
    CAddrFrame(){};
    virtual ~CAddrFrame(){};
protected:
    virtual unsigned int GetRowWidth() {return 10 * m_CharSize.cx + 2 * BORDER_INDENT;};
};

template <class _Repr>
class CConcreteAddrFrame: public CAddrFrame
{
public:
    virtual void Draw(CDC *pViewDc, CRect Region, int Offset=0);
private:
    virtual unsigned int GetBytesPerRow(){return m_Repr.GetBytesPerRow();};
    virtual void FillDc(CDC* pDc);
    _Repr m_Repr;
};

const UINT ModeNibble = 0;
const UINT ModeByte = 1;
const UINT ModeOctal = 2;
const UINT ModeDecimal = 3;
const UINT ModeWordB0B1 = 4;
const UINT ModeWordB1B0 = 5;


class CScreen
{
public:
    CScreen();
    virtual ~CScreen();
    void Draw(CDC *pViewDc);
    void Init(CSize CharSize, CTwiBootProgDoc *pDoc, CSize size);
    void SetMode(UINT mode);
    void SetSize(CSize NewSize);

    CSize GetCharSize() {return m_CharSize;};

    unsigned int GetRealWidth();
    unsigned int GetVisibleWidth() {return m_Size.cx;};
    unsigned int GetHorzOffset() { return m_HorzOffset;};
    void SetHorzOffset(unsigned int NewHorzOffset) {m_HorzOffset = NewHorzOffset;};

    unsigned int GetRealRowNum();
    unsigned int GetVisibleRowNum() { return m_VisibleRowNum;};
    unsigned int GetFirstRow();
    void SetFirstRow(unsigned int NewFirstRow);

    bool GetCaretPos(CPoint *pPos); // returns false if caret is out of view
    bool ScrollToCaret();
    void OnVkLeft(){m_pDataFrame->OnVkLeft();};
    void OnVkDown(){m_pDataFrame->OnVkDown();};
    void OnVkRight(){m_pDataFrame->OnVkRight();};
    void OnVkUp(){m_pDataFrame->OnVkUp();};
    void OnPageUp();
    void OnPageDown();
    void OnVkHome(){m_pDataFrame->OnVkHome();};
    void OnVkEnd(){m_pDataFrame->OnVkEnd();};
    void OnVkCtrlHome(){m_pDataFrame->OnVkCtrlHome();};
    void OnVkCtrlEnd(){m_pDataFrame->OnVkCtrlEnd();};
    void OnVkTab(){m_pDataFrame->OnVkTab();};
    void OnChar(char ch) {m_pDataFrame->OnChar(ch);};
    bool OnLButtonDown(CPoint point);
private:
    void SetStartAddr(unsigned int NewStartAddr);

    int GetRowHeight() {return m_CharSize.cy + BORDER_INDENT * 2;};
    bool m_bInited;

    CTwiBootProgDoc* m_pDoc;

    UINT m_Mode;
    CAddrFrame *m_pAddrFrame;
    CDataFrame *m_pDataFrame;

    CSize m_Size;
    CSize m_CharSize;

    int m_HorzOffset;

    unsigned int m_StartAddr;
    unsigned int m_VisibleRowNum;
};