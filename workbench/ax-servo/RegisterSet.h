// RegisterSet.h : Declaration of the CRegisterSet

#pragma once
#include "resource.h"       // main symbols
#include "register.h"
#include <map>

using namespace std;


[idl_quote("interface IRegister;")];
__interface IRegister;


// IRegisterSet
[
	object,
	uuid("50D3120E-4ACD-48CD-9132-7545208032CE"),
	dual,	helpstring("IRegisterSet Interface"),
	pointer_default(unique)
]
__interface IRegisterSet : IDispatch
{
	[propget, id(1), helpstring("property byName")] HRESULT byName([in] BSTR name, [out, retval] IRegister** pVal);
	[propput, id(1), helpstring("property byName")] HRESULT byName([in] BSTR name, [in] IRegister* newVal);
	[propget, id(2), helpstring("property byIndex")] HRESULT byIndex([in] LONG idx, [out, retval] IRegister** pVal);
	[propput, id(2), helpstring("property byIndex")] HRESULT byIndex([in] LONG idx, [in] IRegister* newVal);
	[propget, id(3), helpstring("property byAddress")] HRESULT byAddress([in] LONG address, [out, retval] IRegister** pVal);
	[propput, id(3), helpstring("property byAddress")] HRESULT byAddress([in] LONG address, [in] IRegister* newVal);
	[propget, id(4), helpstring("property ByCategory")] HRESULT ByCategory([in] BSTR catname, [out, retval] IRegisterSet** pVal);
	[id(5), helpstring("method Add")] HRESULT Add([in] IRegister* reg);
	[id(6), helpstring("method AddRegister")] HRESULT AddRegister([in] LONG address, [in] BSTR name, [in] USHORT direction, [in] BSTR category, [in] BSTR description);
	[propget, id(7), helpstring("property count")] HRESULT count([out, retval] LONG* pVal);
};



// CRegisterSet

[
	coclass,
	threading("apartment"),
	vi_progid("Servo.RegisterSet"),
	progid("Servo.RegisterSet.1"),
	version(1.0),
	uuid("8C5E37A8-8DE4-4315-95CA-3BC36E697F51"),
	helpstring("RegisterSet Class")
]
class ATL_NO_VTABLE CRegisterSet : 
	public IRegisterSet
{
	typedef map< _bstr_t, IRegister* > RegisterMap;
public:
	CRegisterSet()
	{
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	
	void FinalRelease();

protected:
	RegisterMap m_registers;

public:
	STDMETHOD(get_byName)(BSTR name, IRegister** pVal);
	STDMETHOD(put_byName)(BSTR name, IRegister* newVal);
	STDMETHOD(get_byIndex)(LONG idx, IRegister** pVal);
	STDMETHOD(put_byIndex)(LONG idx, IRegister* newVal);
	STDMETHOD(get_byAddress)(LONG address, IRegister** pVal);
	STDMETHOD(put_byAddress)(LONG address, IRegister* newVal);
	STDMETHOD(get_ByCategory)(BSTR catname, IRegisterSet** pVal);
	STDMETHOD(Add)(IRegister* reg);
	STDMETHOD(AddRegister)(LONG address, BSTR name, USHORT direction, BSTR category, BSTR description);
	STDMETHOD(get_count)(LONG* pVal);
};

