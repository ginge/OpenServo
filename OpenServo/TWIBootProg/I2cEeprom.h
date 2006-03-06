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

#include "Device.h"
#include "I2cBridge/I2cBridge.h"
#include "DeviceFactory.h"

//////////////////////////////////////////////////////////////////////////
// Registry settings:

const char* const regI2cEeepromSection = "I2cEeprom";
const char* const regBaseAddr = "BaseAddress";
const BYTE regBaseAddrDef = 0x7F;
const char* const regI2cFreq = "Frequency";
const BYTE regI2cFreqDef = 1;
const char* const regI2cSync = "Sync";
const BYTE regI2cSyncDef = 1;

class CI2cEeprom :public CDevice
{
    static CConcreteDeviceFactory<CI2cEeprom> sm_Factory;
public:
    std::string GetType() {return "TWI BOOTLOADER";};
    TStringList GetSubtypes();
    bool SetSubtype(std::string Subtype);

    bool Read(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Program(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Verify(BYTE *pBuffer, UINT Length, HWND Feedback);
    bool Erase(HWND Feedback);
    bool Configure();

    CI2cEeprom(void);
    virtual ~CI2cEeprom(void);
protected:
    struct EepromTypes
    {
        char* m_Name;
        UINT m_Size;
        USHORT m_PageSize;
        USHORT m_AddrOffset;
        BYTE m_AddrByteNum;
		USHORT m_SkipReset;
		DWORD m_Delay;
    };
    static EepromTypes sm_EepromTypes[];
    int m_Subtype;
private:

    struct SDeviceOperation
    {
        BYTE *pBuffer;
        ULONG BufLen;
        HWND Feedback;
        BYTE SlaveBase;
    } m_DevOperation;

    WPARAM prCheckSlave();

    HANDLE m_hBusy;
    bool prSendFeedback(WPARAM wParam, LPARAM lParam);
    bool prStartOperation(BYTE *pBuffer, UINT Length, HWND Feedback,
        unsigned (CI2cEeprom::*pFunc)());
    unsigned prEndOperation(WPARAM RetVal);
    HANDLE m_hDevice;

    unsigned prReadThread();
    unsigned prProgramThread();
    unsigned prVerifyThread();
    unsigned prEraseThread();

    // Functions expect the device has been already opened. 
    WPARAM prDoRead();
    WPARAM prDoProgram();
    WPARAM prDoVerify(bool bAfterErase);
    WPARAM prDoErase();

    bool prSetAddress(U2C_TRANSACTION *pTransaction, ULONG Addr);
};

