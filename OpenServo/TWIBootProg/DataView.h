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

#include "Frame.h"
#include "TwiBootProgDoc.h"

// CDataView view
class CMainFrame;

class CDataView : public CView
{
    DECLARE_DYNCREATE(CDataView)
public:
    CTwiBootProgDoc* GetDocument(){return dynamic_cast<CTwiBootProgDoc*>(CView::GetDocument());};

private:
    void UpdateCaret();
    void ScrollToCaret();
    void UpdateView();

    void ScrollToRow(unsigned int NewFirstRaw);
    void ScrollToOffset(unsigned int NewHorzOffset);
    
    CScreen m_Screen;
    void InitScreen();


    UINT m_CurrModeId;

    void UpdateVertScrollBar(bool bPositionOnly = false);
    void UpdateHorzScrollBar(bool bPositionOnly = false);
    void VertScroll(UINT nSBCode);
    void HorzScroll(UINT nSBCode);

    CMainFrame* GetMainFrame();

protected:
    CDataView();           // protected constructor used by dynamic creation
    virtual ~CDataView();

public:
    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual void OnInitialUpdate();
protected:
public:
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

protected:
    virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
    virtual BOOL CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, LPVOID lpParam = NULL);
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg void OnSetMode(UINT nId);
    afx_msg void OnUpdateMode(CCmdUI* pCmdUI);

    afx_msg void OnDeviceProgram();
    afx_msg void OnDeviceRead();
    afx_msg void OnDeviceVerify();
    afx_msg void OnDeviceErase();
    afx_msg void OnDeviceConfig();


    afx_msg void OnVkLeft(){m_Screen.OnVkLeft();ScrollToCaret();};
    afx_msg void OnVkDown(){m_Screen.OnVkDown();ScrollToCaret();};
    afx_msg void OnVkRight(){m_Screen.OnVkRight();ScrollToCaret();};
    afx_msg void OnVkUp(){m_Screen.OnVkUp();ScrollToCaret();};
    afx_msg void OnPageUp(){VertScroll(SB_PAGEUP); m_Screen.OnPageUp(); ScrollToCaret();};
    afx_msg void OnPageDown(){VertScroll(SB_PAGEDOWN); m_Screen.OnPageDown(); ScrollToCaret();};
    afx_msg void OnVkHome(){m_Screen.OnVkHome();ScrollToCaret();};
    afx_msg void OnVkEnd(){m_Screen.OnVkEnd();ScrollToCaret();};
    afx_msg void OnVkCtrlHome(){m_Screen.OnVkCtrlHome();ScrollToCaret();};
    afx_msg void OnVkCtrlEnd(){m_Screen.OnVkCtrlEnd();ScrollToCaret();};
    afx_msg void OnVkTab(){m_Screen.OnVkTab();ScrollToCaret();};
    afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point) {if(m_Screen.OnLButtonDown(point))ScrollToCaret();};
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

    afx_msg LRESULT OnDeviceFeedback(WPARAM DeviceEvent, LPARAM Parameter);

private:
    enum EState
    {
        sIdle,
        sProgramming,
        sReading,
        sErasing,
        sVerifying
    } m_State;
    void SetState(EState NewState);
};


