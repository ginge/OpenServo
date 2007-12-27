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

// CFillDlg dialog

class CFillDlg : public CDialog
{
    DECLARE_DYNAMIC(CFillDlg)

public:
    CFillDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CFillDlg();
    void SetMaxAddr(UINT MaxAddr){m_MaxAddr = MaxAddr;};
    UINT GetFromValue(){return m_From;};
    UINT GetToValue() {return m_To;};
    BYTE GetFillValue() {return m_With;};

// Dialog Data
    enum { IDD = IDD_FILLDLG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    UINT m_MaxAddr;
    UINT m_From;
    UINT m_To;
    BYTE m_With;
    CString m_FromStr;
    CString m_ToStr;
    CString m_WithStr;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
};
