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
#include "afxwin.h"

// COpenTypeDlg dialog

class COpenTypeDlg : public CDialog
{
    DECLARE_DYNAMIC(COpenTypeDlg)

public:
    COpenTypeDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~COpenTypeDlg();

// Dialog Data
    enum { IDD = IDD_OPENTYPEDLG };
    eFileType GetType();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
//  CButton m_Binary;
    eFileType m_FileType;
    int m_Selection;
};
