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

// CI2cEepromConfig dialog

class CI2cEepromConfig : public CDialog
{
    DECLARE_DYNAMIC(CI2cEepromConfig)

public:
    CI2cEepromConfig(CWnd* pParent = NULL);   // standard constructor
    virtual ~CI2cEepromConfig();

    // Dialog Data
    enum { IDD = IDD_I2CEEPROMCONFIG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
private:
    struct SI2cFreq
    {
        char* m_Text;
        BYTE m_Value;
    };
    static SI2cFreq sm_Freq[];

public:
    virtual BOOL OnInitDialog();
    CString m_strBaseAddr;
    CComboBox m_cmbFreq;
    CButton m_cmbSync;
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCheck1();
    afx_msg void OnBnClickedSync();
};
