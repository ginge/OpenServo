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
#include "BufferSizeDlg.h"

// CBufferSizeDlg dialog

IMPLEMENT_DYNAMIC(CBufferSizeDlg, CDialog)
CBufferSizeDlg::CBufferSizeDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CBufferSizeDlg::IDD, pParent)
    , m_BufferSizeStr(_T(""))
    , m_FillString(_T(""))
{
}

CBufferSizeDlg::~CBufferSizeDlg()
{
}

void CBufferSizeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUFFER_SIZE_SLIDER, m_SizeSlider);
    DDX_Text(pDX, IDC_BUFFER_SIZE, m_BufferSizeStr);
    DDX_Text(pDX, IDC_FILL_WITH, m_FillString);
}


BEGIN_MESSAGE_MAP(CBufferSizeDlg, CDialog)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CBufferSizeDlg message handlers

void CBufferSizeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    UpdateBufferSize();
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CBufferSizeDlg::UpdateBufferSize()
{
    UpdateData(TRUE);
    int SliderPos = m_SizeSlider.GetPos();
    m_BufferSize = (UINT)1 << SliderPos;
    UINT BufferSize = m_BufferSize;
    if (BufferSize / 1024 == 0)
        m_BufferSizeStr.Format("%u Byte", BufferSize);
    else
    {
        BufferSize = BufferSize / 1024;
        if (BufferSize / 1024 == 0)
            m_BufferSizeStr.Format("%u KByte", BufferSize);
        else
        {
            BufferSize = BufferSize / 1024;
            m_BufferSizeStr.Format("%u MByte", BufferSize);
        }
    }
    UpdateData(FALSE);
}


BOOL CBufferSizeDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    UpdateData(TRUE);
    m_SizeSlider.SetRange(MIN_BUFFER_SIZE_POW, MAX_BUFFER_SIZE_POW);
    m_SizeSlider.SetPos(7);
    m_FillString = "FF";
    UpdateData(FALSE);
    UpdateBufferSize();
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CBufferSizeDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    errno = 0;
    char* pEnds;
    ULONG Fill = strtoul(m_FillString, &pEnds, 16);
    if ((Fill > 0xFF) || (*pEnds) || errno)
    {
        AfxMessageBox(IDS_INVALID_FILL_VAL);
        GetDlgItem(IDC_FILL_WITH)->SetFocus();
        return;
    }
    m_Fill = (BYTE)Fill;
    OnOK();
}
