// Servo-base.h : Declaration of the CServo

#pragma once
#include "resource.h"       // main symbols

#include "servocat.h"

#include "device.h"



// IServo
[
	object,
	uuid("1B906911-33F3-4337-AF18-11A9FF4D03C1"),
	dual,	helpstring("IServo Interface"),
	pointer_default(unique)
]
__interface IServo : IDevice
{
	[propget, id(100), helpstring("property position")] HRESULT position([out, retval] LONG* pVal);
	[propput, id(100), helpstring("property position")] HRESULT position([in] LONG newVal);
	[propget, id(101), helpstring("property targetPosition")] HRESULT targetPosition([out, retval] LONG* pVal);
	[propput, id(101), helpstring("property targetPosition")] HRESULT targetPosition([in] LONG newVal);
	[propget, id(102), helpstring("property currentPosition")] HRESULT currentPosition([out, retval] LONG* pVal);
	[propget, id(103), helpstring("property current")] HRESULT current([out, retval] LONG* pVal);
	[propget, id(104), helpstring("property speed")] HRESULT speed([out, retval] LONG* pVal);
	[propput, id(104), helpstring("property speed")] HRESULT speed([in] LONG newVal);
	[propget, id(105), helpstring("property name")] HRESULT name([out, retval] BSTR* pVal);
	[propput, id(105), helpstring("property name")] HRESULT name([in] BSTR newVal);
	[propget, id(106), helpstring("property enabled")] HRESULT enabled([out, retval] VARIANT_BOOL* pVal);
	[propput, id(106), helpstring("property enabled")] HRESULT enabled([in] VARIANT_BOOL newVal);
};



// CServo

[
	coclass,
	implements_category("CATID_SERVO"),
	implements(dispinterfaces = IDevice),
	threading("apartment"),
	vi_progid("Servo.Servo"),
	progid("Servo.Servo.1"),
	version(1.0),
	uuid("9A668C4E-E2F3-4BEF-89BE-03D428B49D47"),
	helpstring("Servo Class")
]
class ATL_NO_VTABLE CServo : 
	public IServo
{
public:
	CServo()
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
	DWORD ReadRegister(BYTE address, BYTE length);


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

