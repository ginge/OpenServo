// Register.h : Declaration of the CRegister

#pragma once
#include "resource.h"       // main symbols
#include "comdef.h"

typedef enum {
	RD_BOTH		=0,
	RD_IN		=1,
	RD_OUT		=2
} RegisterDirection;


// IRegister
[
	object,
	uuid("03D915EA-C8BD-4FDC-A827-7E671D92554A"),
	dual,	helpstring("IRegister Interface"),
	pointer_default(unique)
]
__interface IRegister : IDispatch
{
	[propget, id(1), helpstring("property address")] HRESULT address([out, retval] LONG* pVal);
	[propget, id(2), helpstring("property name")] HRESULT name([out, retval] BSTR* pVal);
	[propget, id(3), helpstring("property direction")] HRESULT direction([out, retval] USHORT* pVal);
	[propget, id(4), helpstring("property category")] HRESULT category([out, retval] BSTR* pVal);
	[propget, id(5), helpstring("property value")] HRESULT value([out, retval] VARIANT* pVal);
	[propput, id(5), helpstring("property value")] HRESULT value([in] VARIANT newVal);
	[propget, id(6), helpstring("property isDirty")] HRESULT isDirty([out, retval] VARIANT_BOOL* pVal);
	[propput, id(6), helpstring("property isDirty")] HRESULT isDirty([in] VARIANT_BOOL newVal);
	[propget, id(7), helpstring("property description")] HRESULT description([out, retval] BSTR* pVal);
};



// CRegister

[
	coclass,
	threading("apartment"),
	vi_progid("Servo.Register"),
	progid("Servo.Register.1"),
	version(1.0),
	uuid("37343200-E6B1-470E-85C8-0226B3E034B4"),
	helpstring("Register Class")
]
class ATL_NO_VTABLE CRegister : 
	public IRegister
{
public:
	CRegister()
		: m_address(-1), m_direction(RD_BOTH), m_isDirty(false)
	{
	}

	CRegister(LONG _address, LPCTSTR _name, RegisterDirection _direction, LPCTSTR _category)
		: m_address(_address), m_name(_name), m_direction(_direction), m_category(_category), m_isDirty(false)
	{
		m_value.vt = VT_EMPTY;
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

	LONG m_address;
	_bstr_t	m_name, m_category, m_description;
	RegisterDirection m_direction;
	_variant_t m_value;
	bool m_isDirty;

public:

	STDMETHOD(get_address)(LONG* pVal);
	STDMETHOD(get_name)(BSTR* pVal);
	STDMETHOD(get_value)(VARIANT* pVal);
	STDMETHOD(put_value)(VARIANT newVal);
	STDMETHOD(get_isDirty)(VARIANT_BOOL* pVal);
	STDMETHOD(put_isDirty)(VARIANT_BOOL newVal);
	STDMETHOD(get_direction)(USHORT* pVal);
	STDMETHOD(get_category)(BSTR* pVal);
	STDMETHOD(get_description)(BSTR* pVal);
};

