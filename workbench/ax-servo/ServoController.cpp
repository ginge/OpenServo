// ServoController.cpp : Implementation of CServoController


#include "stdafx.h"

#define INITGUID

#include "ServoController.h"
#include "../u2c/u2c_common_func.h"
#include "../u2c/i2cbridge.h"

#pragma comment(lib, "..\\u2c\\I2cBrdg.lib")
#pragma comment(lib, "..\\u2c\\U2CCommon.lib")


#include "Servo.h"
#include ".\servocontroller.h"

// CServoController

void CServoController::FinalRelease() 
{
	for(Devices::iterator s=m_devices.begin(), _s=m_devices.end(); s!=_s; s++)
		s->second->Release();
}


STDMETHODIMP CServoController::Connect(BSTR device)
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		U2C_CloseDevice(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
	
	if((m_hDevice = OpenU2C()) != INVALID_HANDLE_VALUE) {
        U2C_SetClockSynch(m_hDevice, TRUE);
		return S_OK;
	}
	return E_FAIL;
}

STDMETHODIMP CServoController::Scan()
{
	U2C_SLAVE_ADDR_LIST list;
	if( U2C_ScanDevices( m_hDevice, &list) ==U2C_SUCCESS) {
		for(int i=0, c=list.nDeviceNumber; i<c; i++) {
			IDevice* pdevice;
			if(FAILED( CoCreateInstance(__uuidof(CServo), NULL, CLSCTX_ALL, __uuidof(IDevice), (void**)&pdevice) ))
				return false;
			pdevice->Attach(  this, list.List[i] );
			m_devices[ list.List[i] ] = pdevice;
		}
		return S_OK;
	} else
		return E_FAIL;
}

STDMETHODIMP CServoController::GetDevice(USHORT i2cAddress, IDevice** pVal)
{
	IDevice* pdevice = m_devices[ i2cAddress ];
	if( pdevice ) {
		*pVal = pdevice;
		pdevice->AddRef();
		return S_OK;
	} else {
		if(FAILED( CoCreateInstance(__uuidof(CServo), NULL, CLSCTX_ALL, __uuidof(IServo), (void**)&pdevice) ))
			return false;
		pdevice->Attach(  this, i2cAddress );
		return S_OK;
	}
}

STDMETHODIMP CServoController::Read(USHORT address, BYTE* mem_address, USHORT mem_length, BYTE* data, USHORT data_len)
{
	if((address>0xff)||(mem_length>4))
		return E_FAIL;	// only supports 4byte mem address
	U2C_TRANSACTION t;
	memset(&t, 0, sizeof(U2C_TRANSACTION));

	t.nSlaveDeviceAddress = (BYTE)address;
	t.nMemoryAddressLength = (BYTE)mem_length;
	if(mem_address)
		memcpy( ((LPBYTE)&t.nMemoryAddress) , mem_address, mem_length );
	else
		t.nMemoryAddress = 0;
	t.nBufferLength = data_len;

	if( U2C_Read( m_hDevice, &t) ==U2C_SUCCESS) {
		memcpy( data, t.Buffer, data_len);
		return S_OK;
	}

	return E_FAIL;
}

STDMETHODIMP CServoController::Write(USHORT address, BYTE* mem_address, USHORT mem_length, BYTE* data, USHORT data_len)
{
	if((address>0xff)||(mem_length>4))
		return E_FAIL;	// only supports 4byte mem address
	U2C_TRANSACTION t;

	t.nSlaveDeviceAddress = (BYTE)address;
	t.nMemoryAddressLength = (BYTE)mem_length;
	if(mem_address)
		memcpy( ((LPBYTE)&t.nMemoryAddress) , mem_address, mem_length );		//	t.nMemoryAddress = mem_address ? *(DWORD*)mem_address : NULL;
	else
		t.nMemoryAddress = 0;

	t.nBufferLength = data_len;
	memcpy( t.Buffer, data, data_len);
	
	if( U2C_Write( m_hDevice, &t) ==U2C_SUCCESS) {
		return S_OK;
	}

	return E_FAIL;
}

STDMETHODIMP CServoController::get_count(LONG* pVal)
{
	*pVal = (LONG)m_devices.size();
	return S_OK;
}

STDMETHODIMP CServoController::get_item(LONG idx, IDevice** pVal)
{
	int i=0;
	for(Devices::iterator s=m_devices.begin(), _s=m_devices.end(); s!=_s; s++)
		if( i++ == idx) {
			*pVal = s->second;
			s->second->AddRef();
			return S_OK;
		}
	return E_INVALIDARG;
}

STDMETHODIMP CServoController::get_isConnected(VARIANT_BOOL* pVal)
{
	*pVal = (m_hDevice != INVALID_HANDLE_VALUE);
	return S_OK;
}

STDMETHODIMP CServoController::Disconnect(void)
{
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		U2C_CloseDevice(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
	for(Devices::iterator s=m_devices.begin(), _s=m_devices.end(); s!=_s; s++)
		s->second->Release();
	return S_OK;
}
