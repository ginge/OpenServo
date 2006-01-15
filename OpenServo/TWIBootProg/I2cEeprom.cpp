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

#include "StdAfx.h"
#include "I2cEeprom.h"
#include "I2cEepromConfig.h"
#include <assert.h>
#pragma comment(lib, "I2cBridge/I2cBrdg.lib")
#pragma comment(lib, "I2cBridge/U2CCommon.lib")
#include "I2cBridge/u2c_common_func.h"
#include "ThreadUtils.h"
#include "Settings.h"
#include "resource.h"

CConcreteDeviceFactory<CI2cEeprom> CI2cEeprom::sm_Factory;
const int erU2CError = 1;
const DWORD I2C_WRITE_DELAY = 200;
CI2cEeprom::EepromTypes CI2cEeprom::sm_EepromTypes[] = {
    {"ATtiny25 Flash", 2048, 32, 0, 2},
    {"ATtiny45 Flash", 4096, 64, 0, 2},
    {"ATtiny85 Flash", 8192, 64, 0, 2},
    {"ATtiny25 EEPROM", 128, 32, 2048, 2},
    {"ATtiny45 EEPROM", 256, 64, 4096, 2},
    {"ATtiny85 EEPROM", 512, 64, 8192, 2},
};

CI2cEeprom::CI2cEeprom(void)
: m_Subtype(0), m_hDevice(INVALID_HANDLE_VALUE)
{
    m_hBusy = CreateMutex(NULL, FALSE, NULL);
}

CI2cEeprom::~CI2cEeprom(void)
{
    CloseHandle(m_hBusy);
}

TStringList CI2cEeprom::GetSubtypes()
{
    TStringList Res;
    int ArrSize = sizeof(sm_EepromTypes)/sizeof(EepromTypes);
    for (int i=0; i<ArrSize; i++)
    {
        Res.push_back(sm_EepromTypes[i].m_Name);
    }
    return Res;
}

bool CI2cEeprom::SetSubtype(std::string DevSubType)
{
    int ArrSize = sizeof(sm_EepromTypes)/sizeof(EepromTypes);
    for (int i=0; i<ArrSize; i++)
    {
        if (DevSubType == sm_EepromTypes[i].m_Name)
        {
            m_Subtype = i;
            return true;
        }
    }
    return false;
}

bool CI2cEeprom::Read(BYTE* pBuffer, UINT Length, HWND Feedback)
{
    return prStartOperation(pBuffer, Length, Feedback, &CI2cEeprom::prReadThread);
}

bool CI2cEeprom::Program(BYTE* pBuffer, UINT Length, HWND Feedback)
{
    return prStartOperation(pBuffer, Length, Feedback, &CI2cEeprom::prProgramThread);
}

bool CI2cEeprom::Verify(BYTE* pBuffer, UINT Length, HWND Feedback)
{
    return prStartOperation(pBuffer, Length, Feedback, &CI2cEeprom::prVerifyThread);
}

bool CI2cEeprom::Erase(HWND Feedback)
{
    return prStartOperation(NULL, sm_EepromTypes[m_Subtype].m_Size, Feedback, &CI2cEeprom::prEraseThread);
}

bool CI2cEeprom::Configure()
{
    CI2cEepromConfig dlg;
    dlg.DoModal();
    return true;
}

bool CI2cEeprom::prStartOperation(BYTE *pBuffer, UINT Length, HWND Feedback, unsigned (CI2cEeprom::*pFunc)())
{
    if (WaitForSingleObject(m_hBusy, 0) == WAIT_TIMEOUT)
    {
        prSendFeedback(DF_U2C_BUSY, 0);
        return false;
    }
    if (Length == 0)
    {
        assert(false);
        prEndOperation(DF_INVALID_PARAMETER);
        return false;
    }
    CSettings *pSet = CSettings::Instance();
    assert(m_hDevice == INVALID_HANDLE_VALUE);
    m_DevOperation.pBuffer = pBuffer;
    m_DevOperation.BufLen = Length;
    m_DevOperation.Feedback = Feedback;
    m_DevOperation.SlaveBase = pSet->GetProfileByte(regI2cEeepromSection, regBaseAddr, regBaseAddrDef);
    m_hDevice = OpenU2C();
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        prEndOperation(DF_U2C_OPEN_FAILED);
        return false;
    }

    WPARAM Res;
    if (DF_CHECK_SLAVE_SUCCEEDED != (Res = prCheckSlave()))
    {
        prEndOperation(Res);
        return false;
    }

    U2C_RESULT U2cRes = U2C_SUCCESS;

    BYTE Freq = pSet->GetProfileByte(regI2cEeepromSection, regI2cFreq, regI2cFreqDef);
    if (U2C_SUCCESS != (U2C_SetI2cFreq(m_hDevice, Freq)))
    {
        if (U2cRes == U2C_NOT_IMPLEMENTED)
            prEndOperation(DF_U2C_NOT_IMPLEMENTED);
        else
            prEndOperation(DF_INTERNAL_ERROR);
        return false;
    }

    BYTE Sync = pSet->GetProfileByte(regI2cEeepromSection, regI2cSync, regI2cSyncDef);
    if (U2C_SUCCESS != (U2C_SetClockSynch(m_hDevice, Sync == 0 ? FALSE : TRUE)))
    {
        if (U2cRes == U2C_NOT_IMPLEMENTED)
            prEndOperation(DF_U2C_NOT_IMPLEMENTED);
        else
            prEndOperation(DF_INTERNAL_ERROR);
        return false;
    }

    HANDLE hThread = BeginThread(this, pFunc);
    if (hThread == 0)
    {
        assert(false);
        prEndOperation(DF_U2C_OPEN_FAILED);
        return false;
    }
    CloseHandle(hThread);
    return true;
}

WPARAM CI2cEeprom::prCheckSlave()
{
    if (m_DevOperation.BufLen > sm_EepromTypes[m_Subtype].m_Size)
    {
        if (IDCANCEL == AfxMessageBox(IDS_LARGE_BUFFER, MB_OKCANCEL))
            return DF_OPERATION_CANCELLED;
        else
            m_DevOperation.BufLen = sm_EepromTypes[m_Subtype].m_Size;
    }
    U2C_SLAVE_ADDR_LIST AddrList;
    if (U2C_SUCCESS != U2C_ScanDevices(m_hDevice, &AddrList))
        return DF_INTERNAL_ERROR;
    bool bFound = false;
    for (BYTE Index = 0; Index < AddrList.nDeviceNumber; Index++)
    {
        if (AddrList.List[Index] == m_DevOperation.SlaveBase)
            bFound = true;
    }
    if (!bFound)
        return DF_CHIP_NOT_FOUND;
    return DF_CHECK_SLAVE_SUCCEEDED;
}

bool CI2cEeprom::prSendFeedback(WPARAM wParam, LPARAM lParam)
{
    if (PostMessage(m_DevOperation.Feedback, WM_DEVICE_FEEDBACK, wParam, lParam))
        return true;
    else
        return false;
}

unsigned CI2cEeprom::prEndOperation(WPARAM RetVal)
{
    if(m_hDevice != INVALID_HANDLE_VALUE)
    {
        U2C_CloseDevice(m_hDevice);
        m_hDevice = INVALID_HANDLE_VALUE;
    }
    prSendFeedback(RetVal, 0);
    ReleaseMutex(m_hBusy);
    return (unsigned) RetVal;
}

bool CI2cEeprom::prSetAddress(U2C_TRANSACTION *pTransaction, ULONG Addr)
{
    if (Addr > sm_EepromTypes[m_Subtype].m_Size)
    {
        assert(false);
        return false;
    }
    pTransaction->nSlaveDeviceAddress = m_DevOperation.SlaveBase;
    pTransaction->nMemoryAddressLength = sm_EepromTypes[m_Subtype].m_AddrByteNum;
    pTransaction->nMemoryAddress = Addr + sm_EepromTypes[m_Subtype].m_AddrOffset;
    return true;
}

unsigned CI2cEeprom::prReadThread()
{
    assert(m_hDevice != INVALID_HANDLE_VALUE);
    return prEndOperation(prDoRead());
}

unsigned CI2cEeprom::prProgramThread()
{
    WPARAM Res;
    assert(m_hDevice != INVALID_HANDLE_VALUE);
    if (DF_PROGRAM_FINISHED != (Res = prDoProgram()))
        return prEndOperation(Res);
    if (DF_VERIFY_FINISHED != (Res = prDoVerify(false)))
        return prEndOperation(Res);
    return prEndOperation(DF_PROGRAM_FINISHED);
}
unsigned CI2cEeprom::prVerifyThread()
{
    assert(m_hDevice != INVALID_HANDLE_VALUE);
    return prEndOperation(prDoVerify(false));
}

unsigned CI2cEeprom::prEraseThread()
{
    WPARAM Res;
    assert(m_hDevice != INVALID_HANDLE_VALUE);
    if (DF_ERASE_FINISHED != (Res = prDoErase()))
        return prEndOperation(Res);
    if (DF_VERIFY_FINISHED != (Res = prDoVerify(true)))
        return prEndOperation(Res);
    return prEndOperation(DF_ERASE_FINISHED);
}

WPARAM CI2cEeprom::prDoRead()
{
    if ((m_DevOperation.BufLen == 0) || (m_DevOperation.pBuffer == NULL))
    {
        assert(false);
        return DF_INVALID_PARAMETER;
    }
    DWORD CurrAddr;
    USHORT CurrBufferLen;
    U2C_TRANSACTION Transaction;
    U2C_RESULT Res = U2C_SUCCESS;
    assert(m_DevOperation.BufLen != 0);
    for(CurrAddr = 0; CurrAddr < m_DevOperation.BufLen; CurrAddr += CurrBufferLen)
    {
        prSendFeedback(DF_READ_PROCEED, CurrAddr * 100 / m_DevOperation.BufLen);
        CurrBufferLen = sm_EepromTypes[m_Subtype].m_PageSize;
        if (CurrAddr + CurrBufferLen > m_DevOperation.BufLen)
            CurrBufferLen = USHORT(m_DevOperation.BufLen - CurrAddr);
        Transaction.nBufferLength = CurrBufferLen;
        // Set the address including the address offset.
        prSetAddress(&Transaction, CurrAddr);
        if (U2C_SUCCESS != (Res = U2C_Read(m_hDevice, &Transaction)))
            return DF_READ_FAILED;
        CopyMemory(m_DevOperation.pBuffer+CurrAddr, Transaction.Buffer, CurrBufferLen);

    }
    return DF_READ_FINISHED;
}

WPARAM CI2cEeprom::prDoProgram()
{
    if ((m_DevOperation.BufLen == 0) || (m_DevOperation.pBuffer == NULL))
    {
        assert(false);
        return DF_INVALID_PARAMETER;
    }
    DWORD CurrAddr;
    USHORT CurrBufferLen;
    U2C_TRANSACTION Transaction;
    U2C_RESULT Res = U2C_SUCCESS;
    assert(m_DevOperation.BufLen != 0);
    for(CurrAddr = 0; CurrAddr < m_DevOperation.BufLen; CurrAddr += CurrBufferLen)
    {
        prSendFeedback(DF_PROGRAM_PROCEED, CurrAddr * 100 / m_DevOperation.BufLen);
        CurrBufferLen = sm_EepromTypes[m_Subtype].m_PageSize;
        if (CurrAddr + CurrBufferLen > m_DevOperation.BufLen)
            CurrBufferLen = USHORT(m_DevOperation.BufLen - CurrAddr);
        Transaction.nBufferLength = CurrBufferLen;
        // Set the address including the address offset.
        prSetAddress(&Transaction, CurrAddr);
        CopyMemory(Transaction.Buffer, m_DevOperation.pBuffer + CurrAddr, CurrBufferLen);
        if (U2C_SUCCESS != (Res = U2C_Write(m_hDevice, &Transaction)))
            return DF_PROGRAM_FAILED;
        Sleep(I2C_WRITE_DELAY);
    }
    return DF_PROGRAM_FINISHED;
}

WPARAM CI2cEeprom::prDoVerify(bool bAfterErase)
{
    if (m_DevOperation.BufLen == 0)
    {
        assert(false);
        return DF_INVALID_PARAMETER;
    }
    if ((!bAfterErase) && (m_DevOperation.pBuffer == NULL))
    {
        assert(false);
        return DF_INVALID_PARAMETER;
    }
    DWORD CurrAddr;
    USHORT CurrBufferLen;
    U2C_TRANSACTION Transaction;
    U2C_RESULT Res = U2C_SUCCESS;
    BYTE ffBuffer[256];
    FillMemory(ffBuffer, 256, 0xFF);
    assert(m_DevOperation.BufLen != 0);
    for(CurrAddr = 0; CurrAddr < m_DevOperation.BufLen; CurrAddr += CurrBufferLen)
    {
        prSendFeedback(DF_VERIFY_PROCEED, CurrAddr * 100 / m_DevOperation.BufLen);
        CurrBufferLen = sm_EepromTypes[m_Subtype].m_PageSize;
        if (CurrAddr + CurrBufferLen > m_DevOperation.BufLen)
            CurrBufferLen = USHORT(m_DevOperation.BufLen - CurrAddr);
        Transaction.nBufferLength = CurrBufferLen;
        // Set the address including the address offset.
        prSetAddress(&Transaction, CurrAddr);
        if (U2C_SUCCESS != (Res = U2C_Read(m_hDevice, &Transaction)))
            return DF_VERIFY_FAILED;
        if (bAfterErase)
        {
            // The first two bytes of the boot loader will never between written
            // or erased so don't include it in the verification of the programming.
            if (CurrAddr == 0)
            {
                // Sanity check.
                if (CurrBufferLen > 2)
                {
                    // Ignore the first two bytes and compare the rest of the transaction
                    // buffer with the erased buffer values.
                    if (memcmp(ffBuffer + 2, Transaction.Buffer + 2, CurrBufferLen - 2) != 0)
                        return DF_VERIFY_FAILED;
                }
            }
            else
            {
                // Compare the transaction buffer with the erased buffer values.
                if (memcmp(ffBuffer, Transaction.Buffer, CurrBufferLen) != 0)
                    return DF_VERIFY_FAILED;
            }
        }
        else
        {
            // The first two bytes of the boot loader will never between written
            // or erased so don't include it in the verification of the programming.
            if (CurrAddr == 0)
            {
                // Sanity check.
                if (CurrBufferLen > 2)
                {
                    // Ignore the first two bytes and compare the rest of the transaction
                    // buffer with the erased buffer values.
                    if (memcmp(m_DevOperation.pBuffer + CurrAddr + 2, Transaction.Buffer + 2, CurrBufferLen - 2) != 0)
                        return DF_VERIFY_FAILED;
                }
            }
            else
            {
                // Compare the transaction buffer with the erased buffer values.
                if (memcmp(m_DevOperation.pBuffer + CurrAddr, Transaction.Buffer, CurrBufferLen) != 0)
                    return DF_VERIFY_FAILED;
            }
        }
    }
    return DF_VERIFY_FINISHED;
}

WPARAM CI2cEeprom::prDoErase()
{
    if (m_DevOperation.BufLen == 0)
    {
        assert(false);
        return DF_INVALID_PARAMETER;
    }
    DWORD CurrAddr;
    USHORT CurrBufferLen;
    U2C_TRANSACTION Transaction;
    FillMemory(Transaction.Buffer, 256, 0xFF);
    U2C_RESULT Res = U2C_SUCCESS;
    assert(m_DevOperation.BufLen != 0);
    for(CurrAddr = 0; CurrAddr < m_DevOperation.BufLen; CurrAddr += CurrBufferLen)
    {
        prSendFeedback(DF_ERASE_PROCEED, CurrAddr* 100 / m_DevOperation.BufLen);
        CurrBufferLen = sm_EepromTypes[m_Subtype].m_PageSize;
        if (CurrAddr + CurrBufferLen > m_DevOperation.BufLen)
            CurrBufferLen = USHORT(m_DevOperation.BufLen - CurrAddr);
        Transaction.nBufferLength = CurrBufferLen;
        // Set the address including the address offset.
        prSetAddress(&Transaction, CurrAddr);
        if (U2C_SUCCESS != (Res = U2C_Write(m_hDevice, &Transaction)))
            return DF_ERASE_FAILED;
        Sleep(I2C_WRITE_DELAY);
    }
    return DF_ERASE_FINISHED;
}
