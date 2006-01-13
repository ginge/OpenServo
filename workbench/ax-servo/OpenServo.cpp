// Servo-base.cpp : Implementation of COpenServo

#include "stdafx.h"

#include "OpenServo.h"
#include "RegisterSet.h"



// COpenServo


STDMETHODIMP COpenServo::Detect()
{
	

	return S_OK;
}

STDMETHODIMP COpenServo::get_typeName(BSTR* pVal)
{
	_bstr_t v(L"OpenServo");
	*pVal = v.Detach();
	return S_OK;
}

STDMETHODIMP COpenServo::get_address(USHORT* pVal)
{
	*pVal = m_address;
	return S_OK;
}

STDMETHODIMP COpenServo::get_position(LONG* pVal)
{
	return get_currentPosition(pVal);
}

STDMETHODIMP COpenServo::put_position(LONG newVal)
{
	return put_targetPosition(newVal);
}

STDMETHODIMP COpenServo::get_targetPosition(LONG* pVal)
{
	return ReadRegister( 0x60, 1, pVal);
}

STDMETHODIMP COpenServo::put_targetPosition(LONG newVal)
{
	if(newVal>0x03ff)
		return E_FAIL;
	return WriteRegister(0x60, 1, newVal);
}

STDMETHODIMP COpenServo::get_currentPosition(LONG* pVal)
{
	return ReadRegister( 0x61, 1, pVal);
}

STDMETHODIMP COpenServo::get_current(LONG* pVal)
{
	return ReadRegister( 0x63, 1, pVal);
}

STDMETHODIMP COpenServo::get_speed(LONG* pVal)
{
	return ReadRegister( 0x62, 1, pVal);
}

STDMETHODIMP COpenServo::put_speed(LONG newVal)
{
	if(newVal>0xff)
		return E_FAIL;
	return WriteRegister(0x62, 1, newVal);
}

STDMETHODIMP COpenServo::ScanRegisters(IRegisterSet* registerSet)
{
	LONG count;
	HRESULT hr;
	if(FAILED( hr=registerSet->get_count(&count) ))
		return hr;
	for(int i=0; i<count; i++) {
		IRegister* pregister;
		if(FAILED( hr = registerSet->get_byIndex(i, &pregister) ))
			return hr;
		
		LONG address;
		LONG value;
		if(SUCCEEDED( hr = pregister->get_address(&address) )) {
			switch( address ) {
				case 0x64: //  status
					if(SUCCEEDED( hr = ReadRegister( (BYTE)address, 1, &value )))
						value = (value & 0x04) ? TRUE : FALSE;
					break;

				/* two byte registers */
				case 0x67: // overruns
				case 0x69: // iterations
					hr = ReadRegister( (BYTE)address, 2, &value );
					break;

				default:
					hr = ReadRegister( (BYTE)address, 1, &value );
					break;
			}
			_variant_t v( value );
			pregister->put_value( v );
		}
	}
	return S_OK;
}

HRESULT COpenServo::ReadRegister(BYTE address, BYTE length, LONG* pval)
{
	if(length>4) return 0;
	USHORT maddr = 0x0100 | address;
	DWORD data = 0;
	LPBYTE pdata = (LPBYTE)&data; // + 4 - length;
	HRESULT hr;
	if( SUCCEEDED( hr = m_pcontroller->Read( m_address, (BYTE*)&maddr, 2, pdata, length)) ) {
		*pval = data;
		return S_OK;
	} else
        return hr;
}

HRESULT COpenServo::WriteRegister(BYTE address, BYTE length, LONG pval)
{
	if(length>4) return 0;
	BYTE maddr = 0x0100 | address;
	return m_pcontroller->Write( m_address, &address, 2, (LPBYTE)&pval, length);
}

STDMETHODIMP COpenServo::get_registers(IRegisterSet** pVal)
{

	IRegisterSet* pset;
	if(FAILED( CoCreateInstance(__uuidof(CRegisterSet), NULL, CLSCTX_ALL, __uuidof(IRegisterSet), (void**)&pset) ))
		return E_FAIL;

	BSTR cat_servo(L"Servo"), cat_pid(L"PID Controller"), cat_system(L"System");

	DEVICE_REGMAP_ADD( pset, 0x61, Position, RD_IN, cat_servo, L"The current position of the servo.");
	DEVICE_REGMAP_ADD( pset, 0x60, Target, RD_BOTH, cat_servo, L"The desired position of the servo.");
	DEVICE_REGMAP_ADD( pset, 0x62, Speed, RD_BOTH, cat_servo, L"The maximum speed limit of the servo.");
	DEVICE_REGMAP_ADD( pset, 0x63, Current, RD_IN, cat_servo, L"The current the motor is drawing to move to or to maintain the position.");
	DEVICE_REGMAP_ADD( pset, 0x64, Enabled, RD_BOTH, cat_servo, L"True if the servo is enabled.");
	
	DEVICE_REGMAP_ADD( pset, 0x65, E, RD_IN, cat_pid, L"The servo Error (E), where E = Target - Position, during the last iteration..");
	DEVICE_REGMAP_ADD( pset, 0x24, P, RD_IN, cat_pid, L"The Proportional Error component of the PID algorithm during the last iteration..");
	DEVICE_REGMAP_ADD( pset, 0x25, I, RD_IN, cat_pid, L"The Integral component of the PID algorithm during the last iteration..");
	DEVICE_REGMAP_ADD( pset, 0x26, D, RD_IN, cat_pid, L"The Derivitive component of the PID algorithm during the last iteration..");
	DEVICE_REGMAP_ADD( pset, 0x27, PID, RD_BOTH, cat_pid, L"The output of the PID algorithm during the last iteration.");
	DEVICE_REGMAP_ADD( pset, 0x20, Kp, RD_BOTH, cat_pid, L"The coefficient used to scale the P component.");
	DEVICE_REGMAP_ADD( pset, 0x21, Ki, RD_BOTH, cat_pid, L"The coefficient used to scale the I component.");
	DEVICE_REGMAP_ADD( pset, 0x22, Kd, RD_BOTH, cat_pid, L"The coefficient used to scale the D component.");
	DEVICE_REGMAP_ADD( pset, 0x23, Kpid, RD_BOTH, cat_pid, L"The coefficient used to scale the output.");
	
	DEVICE_REGMAP_ADD( pset, 0x6c, Address, RD_BOTH, cat_system, L"The I2C address of the servo, requires power recylce to take effect.");
	DEVICE_REGMAP_ADD( pset, 0x66, Options, RD_BOTH, cat_system, L"Options enabled on the servo.");
	DEVICE_REGMAP_ADD( pset, 0x6b, Bandwidth, RD_BOTH, cat_system, L"The bandwidth of the servo, this value has an upper limit.");
	DEVICE_REGMAP_ADD( pset, 0x67, Overruns, RD_IN, cat_system, L"A rolling performance counter of the number of overruns due to the PID loop taking too long to calculate. If enabled, the bandwidth will be reduced to compensate.");
	DEVICE_REGMAP_ADD( pset, 0x69, Iterations, RD_IN, cat_system, L"A rolling performance counter that incriments every iteration of the PID loop.");

	*pVal = pset;

	return S_OK;
}

STDMETHODIMP COpenServo::get_name(BSTR* pVal)
{
	_bstr_t v( (BSTR)m_name, true );
	*pVal = v.Detach();
	return S_OK;
}

STDMETHODIMP COpenServo::put_name(BSTR newVal)
{
	m_name = newVal;
	return S_OK;
}

STDMETHODIMP COpenServo::get_enabled(VARIANT_BOOL* pVal)
{
	USHORT maddr = 0x0164;
	BYTE data;
	if( SUCCEEDED(m_pcontroller->Read( m_address, (BYTE*)&maddr, 2, &data, 1)) ) {
		*pVal = (data & 0x04) ? TRUE : FALSE;
		return S_OK;
	} else
        return E_FAIL;
}

STDMETHODIMP COpenServo::put_enabled(VARIANT_BOOL newVal)
{
	BYTE data = (BYTE)(newVal ? 0x16 : 0x17 );
	return m_pcontroller->Write( m_address, NULL, 0, &data, 1);
}
