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
#include "I2cEepromConfig.h"
#include "I2cBridge/I2cBridge.h"
#include "Settings.h"
#include "I2cEeprom.h"

// CI2cEepromConfig dialog

IMPLEMENT_DYNAMIC(CI2cEepromConfig, CDialog)
CI2cEepromConfig::CI2cEepromConfig(CWnd* pParent /*=NULL*/)
    : CDialog(CI2cEepromConfig::IDD, pParent)
    , m_strBaseAddr(_T(""))
{
}

CI2cEepromConfig::~CI2cEepromConfig()
{
}

void CI2cEepromConfig::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FREQ, m_cmbFreq);
    DDX_Text(pDX, IDC_BASE_ADDR, m_strBaseAddr);
    DDX_Control(pDX, IDC_SYNC, m_cmbSync);
}


BEGIN_MESSAGE_MAP(CI2cEepromConfig, CDialog)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CI2cEepromConfig message handlers
CI2cEepromConfig::SI2cFreq CI2cEepromConfig::sm_Freq[] =
{
    {"FAST-MODE", U2C_I2C_FREQ_FAST},
    {"STANDARD-MODE", U2C_I2C_FREQ_STD},
    {"83 kHz", U2C_I2C_FREQ_83KHZ},
    {"71 kHz", U2C_I2C_FREQ_71KHZ},
    {"62 kHz", U2C_I2C_FREQ_62KHZ},
    {"50 kHz", U2C_I2C_FREQ_50KHZ},
    {"25 kHz", U2C_I2C_FREQ_25KHZ},
    {"10 kHz", U2C_I2C_FREQ_10KHZ},
    {"5 kHz", U2C_I2C_FREQ_5KHZ},
    {"2 kHz", U2C_I2C_FREQ_2KHZ},
};

BOOL CI2cEepromConfig::OnInitDialog()
{
    CDialog::OnInitDialog();
    CSettings *pSet = CSettings::Instance();
    BYTE Sync = pSet->GetProfileByte(regI2cEeepromSection, regI2cSync, regI2cSyncDef);
    BYTE Freq = pSet->GetProfileByte(regI2cEeepromSection, regI2cFreq, regI2cFreqDef);
    BYTE BaseAddr = pSet->GetProfileByte(regI2cEeepromSection, regBaseAddr, regBaseAddrDef);
    int i;
    int FreqNum = sizeof(sm_Freq) / sizeof(SI2cFreq);
    for(i = 0; i < FreqNum; i++)
    {
        m_cmbFreq.AddString(sm_Freq[i].m_Text);
    }
    for(i = 0; i < FreqNum; i++)
    {
        if (Freq <= sm_Freq[i].m_Value)
        {
            m_cmbFreq.SetCurSel(i);
            break;
        }
    }
    m_cmbSync.SetCheck(Sync == 0 ? BST_UNCHECKED : BST_CHECKED);
    m_strBaseAddr.Format("%02X", BaseAddr);
    UpdateData(FALSE);
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CI2cEepromConfig::OnBnClickedOk()
{
    UpdateData(TRUE);
    char *endp;
    m_strBaseAddr.TrimLeft();
    UINT BaseAddr = strtoul(m_strBaseAddr, &endp, 16);
    UINT Sync = m_cmbSync.GetCheck() == BST_CHECKED ? 1 : 0;
    if ((*endp) || m_strBaseAddr.IsEmpty() ||  (BaseAddr > 0xFF))
    {
        MessageBox("Bad value has been used as Base Address.\nPlease provide value in range from 0 to FF");
        GetDlgItem(IDC_BASE_ADDR)->SetFocus();
        return;
    }
    BYTE Freq = sm_Freq[m_cmbFreq.GetCurSel()].m_Value;
    CSettings *pSet = CSettings::Instance();
    pSet->WriteProfileByte(regI2cEeepromSection, regI2cFreq, Freq);
    pSet->WriteProfileByte(regI2cEeepromSection, regBaseAddr, (BYTE) BaseAddr);
    pSet->WriteProfileByte(regI2cEeepromSection, regI2cSync, (BYTE) Sync);
    OnOK();
}
