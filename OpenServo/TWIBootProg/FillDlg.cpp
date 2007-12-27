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
#include "FillDlg.h"

// CFillDlg dialog

IMPLEMENT_DYNAMIC(CFillDlg, CDialog)
CFillDlg::CFillDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CFillDlg::IDD, pParent)
    , m_FromStr(_T("")), m_ToStr(_T("")), m_WithStr(_T(""))
    , m_MaxAddr(0), m_From(0), m_To(0), m_With(0)
{
}

CFillDlg::~CFillDlg()
{
}

void CFillDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FILL_FROM, m_FromStr);
    DDX_Text(pDX, IDC_FILL_TO, m_ToStr);
    DDX_Text(pDX, IDC_FILL_WITH, m_WithStr);
}


BEGIN_MESSAGE_MAP(CFillDlg, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CFillDlg message handlers

BOOL CFillDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_FromStr.Format("%X", 0);
    m_ToStr.Format("%X", m_MaxAddr);
    m_WithStr.Format("%X", 0xFF);
    UpdateData(FALSE);

    return TRUE;
}

void CFillDlg::OnBnClickedOk()
{
    UpdateData(TRUE);
    errno = 0;
    char* pEnds;
    CString ErrMes;
    ULONG Val;
    Val = strtoul(m_FromStr, &pEnds, 16);
    if ((Val > m_MaxAddr) || (*pEnds) || errno)
    {
        ErrMes.Format("\"From\" value can be in range from 0 to %x", m_MaxAddr);
        AfxMessageBox(ErrMes);
        GetDlgItem(IDC_FILL_FROM)->SetFocus();
        return;
    }
    m_From = Val;
    Val = strtoul(m_ToStr, &pEnds, 16);
    if ((Val > m_MaxAddr) || (*pEnds) || errno)
    {
        ErrMes.Format("\"To\" value can be in range from 0 to %x", m_MaxAddr);
        AfxMessageBox(ErrMes);
        GetDlgItem(IDC_FILL_TO)->SetFocus();
        return;
    }
    m_To = Val;
    if (m_From > m_To)
    {
        ErrMes.Format("\"From\" value has to be less than \"To\" value");
        AfxMessageBox(ErrMes);
        GetDlgItem(IDC_FILL_FROM)->SetFocus();
        return;
    }
    Val = strtoul(m_WithStr, &pEnds, 16);
    if ((Val > 0xFF) || (*pEnds) || errno)
    {
        ErrMes.Format("\"To\" value can be in range from 0 to FF");
        AfxMessageBox(ErrMes);
        GetDlgItem(IDC_FILL_WITH)->SetFocus();
        return;
    }
    m_With = (BYTE)Val;
    OnOK();
}
