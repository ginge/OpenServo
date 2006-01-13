// Device.h : Declaration of the CDevice

#pragma once
#include "resource.h"       // main symbols

#include "ServoController.h"

#include "registerset.h"

[idl_quote("interface IRegisterSet;")];
__interface IRegisterSet;



#define DEVICE_REGMAP_ADD( pset, address, name, direction, category, description) pset->AddRegister( address, L#name, direction, category, description );


[idl_quote("interface IServoController;")];
__interface IServoController;

// IDevice
[
	object,
	uuid("E11109B3-9C56-4ABB-B36C-11046CF15670"),
	dual,	helpstring("IDevice Interface"),
	pointer_default(unique)
]
__interface IDevice : IDispatch
{
	[id(1), helpstring("method Attach")] HRESULT Attach([in] IServoController* pcontroller, USHORT address);
	[id(2), helpstring("method Detect")] HRESULT Detect();
	[id(3), helpstring("method ScanRegisters")] HRESULT ScanRegisters([in] IRegisterSet* registerSet);
	[propget, id(10), helpstring("property typeName")] HRESULT typeName([out, retval] BSTR* pVal);
	[propget, id(11), helpstring("property address")] HRESULT address([out, retval] USHORT* pVal);
	[propget, id(12), helpstring("property registers")] HRESULT registers([out, retval] IRegisterSet** pVal);
};



// CDevice
#if 0
[
	coclass,
	threading("apartment"),
	vi_progid("Servo.Device"),
	progid("Servo.Device.1"),
	version(1.0),
	uuid("7AFA755C-3941-4A48-9C8F-39D2F5E4C4A8"),
	helpstring("Device Class")
]
class ATL_NO_VTABLE CDevice : 
	public IDevice
{
public:
	CDevice()
	{
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

};
#endif
