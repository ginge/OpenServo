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
#include "afxcmn.h"

// CBufferSizeDlg dialog

class CBufferSizeDlg : public CDialog
{
    DECLARE_DYNAMIC(CBufferSizeDlg)

public:
    CBufferSizeDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CBufferSizeDlg();
    ULONG GetBufferSize(){return m_BufferSize;};
    BYTE GetFillVal() {return m_Fill;};

// Dialog Data
    enum { IDD = IDD_BUFFERSIZEDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CSliderCtrl m_SizeSlider;
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    CString m_BufferSizeStr;
    ULONG m_BufferSize;
    BYTE m_Fill;
    virtual BOOL OnInitDialog();
private:
    void UpdateBufferSize();
public:
    afx_msg void OnBnClickedOk();
    CString m_FillString;
};
