

#pragma once

#include "servo.h"



// COpenServo

[
	coclass,
	implements_category("CATID_SERVO"),
	implements(dispinterfaces = IDevice),
	threading("apartment"),
	vi_progid("Servo.OpenServo"),
	progid("Servo.OpenServo.1"),
	version(1.0),
	uuid("9A668C4E-E2F3-4BEF-89BE-03D428B49D48"),
	helpstring("OpenServo Class")
]
class ATL_NO_VTABLE COpenServo : 
	public IServo
{
public:
	COpenServo()
		: m_pcontroller(NULL), m_address(0)
	{
	}

	STDMETHOD(Attach)(IServoController* pcontroller, USHORT i2cAddress) {
		m_pcontroller=pcontroller;
		m_address=i2cAddress;
		return S_OK;
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

protected:
	IServoController* m_pcontroller;
	USHORT m_address;
	_bstr_t m_name;


public:
	HRESULT ReadRegister(BYTE address, BYTE length, LONG* pval);
	HRESULT WriteRegister(BYTE address, BYTE length, LONG pval);


	STDMETHOD(Detect)();
	STDMETHOD(get_typeName)(BSTR* pVal);
	STDMETHOD(get_address)(USHORT* pVal);
	STDMETHOD(get_position)(LONG* pVal);
	STDMETHOD(put_position)(LONG newVal);
	STDMETHOD(get_targetPosition)(LONG* pVal);
	STDMETHOD(put_targetPosition)(LONG newVal);
	STDMETHOD(get_currentPosition)(LONG* pVal);
	STDMETHOD(get_current)(LONG* pVal);
	STDMETHOD(get_speed)(LONG* pVal);
	STDMETHOD(put_speed)(LONG newVal);
	STDMETHOD(ScanRegisters)(IRegisterSet* registerSet);
	STDMETHOD(get_registers)(IRegisterSet** pVal);
	STDMETHOD(get_name)(BSTR* pVal);
	STDMETHOD(put_name)(BSTR newVal);
	STDMETHOD(get_enabled)(VARIANT_BOOL* pVal);
	STDMETHOD(put_enabled)(VARIANT_BOOL newVal);
};

