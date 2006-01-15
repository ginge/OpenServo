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
#include "OpenTypeDlg.h"

// COpenTypeDlg dialog

IMPLEMENT_DYNAMIC(COpenTypeDlg, CDialog)
COpenTypeDlg::COpenTypeDlg(CWnd* pParent /*=NULL*/)
    : CDialog(COpenTypeDlg::IDD, pParent), m_FileType(ftUnknown)
    , m_Selection(0)
{
}

COpenTypeDlg::~COpenTypeDlg()
{
}

void COpenTypeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //  DDX_Control(pDX, IDC_BINARY, m_Binary);
    DDX_Radio(pDX, IDC_BINARY, m_Selection);
}


BEGIN_MESSAGE_MAP(COpenTypeDlg, CDialog)
END_MESSAGE_MAP()


// COpenTypeDlg message handlers

BOOL COpenTypeDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  Add extra initialization here
//  m_Binary.SetCheck(BST_CHECKED);
//  SetCheck(IDC_BINARY);
//  CWnd *pWnd = GetDlgItem(IDC_BINARY);
//  CButton * pBut = dynamic_cast<CButton*>(pWnd);
//  pBut->SetCheck(BST_CHECKED);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

eFileType COpenTypeDlg::GetType()
{
    switch(m_Selection)
    {
    case 0:
        return ftBinary;
    case 1:
        return ftIntelHex;
    default:
        ASSERT(FALSE);
        return ftUnknown;
    }
    //if (m_bBinary)
    //  return ftBinary;
    //if (m_bIntelHex)
    //  return ftIntelHex;
//  ASSERT(FALSE);
//  return ftUnknown;

}

