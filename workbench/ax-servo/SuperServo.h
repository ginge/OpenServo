

#pragma once

#include "servo.h"



// CSuperServo

[
	coclass,
	implements_category("CATID_SERVO"),
	implements(dispinterfaces = IDevice),
	threading("apartment"),
	vi_progid("Servo.SuperServo"),
	progid("Servo.SuperServo.1"),
	version(1.0),
	uuid("9A668C4E-E2F3-4BEF-89BE-03D428B49D47"),
	helpstring("SuperServo Class")
]
class ATL_NO_VTABLE CSuperServo : 
	public IServo
{
public:
	CSuperServo()
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
	HRESULT ExecFunction(BYTE function, BYTE length, LONG pval);


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

