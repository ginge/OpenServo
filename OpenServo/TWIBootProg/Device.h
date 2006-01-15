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

const UINT WM_DEVICE_FEEDBACK = WM_USER+1;
const WPARAM DF_U2C_OPEN_FAILED = 1;
const WPARAM DF_READ_FAILED = 2;
const WPARAM DF_PROGRAM_FAILED = 3;
const WPARAM DF_VERIFY_FAILED = 4;
const WPARAM DF_ERASE_FAILED = 5;
// next messages have to contain percentage 
// of the operation done in LPARAM
const WPARAM DF_READ_PROCEED = 6;
const WPARAM DF_PROGRAM_PROCEED = 7;
const WPARAM DF_VERIFY_PROCEED = 8;
const WPARAM DF_ERASE_PROCEED = 9;

const WPARAM DF_READ_FINISHED = 10;
const WPARAM DF_PROGRAM_FINISHED = 11;
const WPARAM DF_VERIFY_FINISHED = 12;
const WPARAM DF_ERASE_FINISHED = 13;

const WPARAM DF_CHIP_NOT_FOUND = 14; // Chip for programming with current configuration is not found
const WPARAM DF_INVALID_PARAMETER = 15;
const WPARAM DF_U2C_BUSY = 16;
const WPARAM DF_INTERNAL_ERROR = 17;
const WPARAM DF_CHECK_SLAVE_SUCCEEDED = 18;
const WPARAM DF_OPERATION_CANCELLED = 19;
const WPARAM DF_OUTDATED_DRIVER_VER = 20;
const WPARAM DF_U2C_NOT_IMPLEMENTED = 21; // Function not supported by current version of the device

class CDevice
{
public:
    CDevice(void);
    virtual ~CDevice(void);
    virtual std::string GetType() = 0;
    virtual TStringList GetSubtypes() = 0;
    virtual bool SetSubtype(std::string Subtype) = 0;
    virtual bool Read(BYTE *pBuffer, UINT Length, HWND Feedback) = 0;
    virtual bool Program(BYTE *pBuffer, UINT Length, HWND Feedback) = 0;
    virtual bool Verify(BYTE *pBuffer, UINT Length, HWND Feedback) = 0;
    virtual bool Erase(HWND Feedback) = 0;
    virtual bool Configure() = 0;
};
