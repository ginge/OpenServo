// ServoController.h : Declaration of the CServoController

#pragma once
#include "resource.h"       // main symbols

#include "servocat.h"


[idl_quote("interface IDevice;")];
__interface IDevice;

// IServoController
[
	object,
	uuid("92C4BB81-191C-4220-B75C-F6747B234493"),
	dual,	helpstring("IServoController Interface"),
	pointer_default(unique)
]
__interface IServoController : IDispatch
{
	[id(1), helpstring("method Connect")] HRESULT Connect(BSTR device);
	[id(2), helpstring("method Scan")] HRESULT Scan();
	[id(4), helpstring("method Read")] HRESULT Read([in] USHORT address, [in] BYTE* mem_address, USHORT mem_length, [in] BYTE* data, USHORT data_len);
	[id(5), helpstring("method Write")] HRESULT Write([in] USHORT address, [in] BYTE* mem_address, USHORT mem_length, [in] BYTE* data, USHORT data_len);
	[propget, id(6), helpstring("property count")] HRESULT count([out, retval] LONG* pVal);
	[propget, id(7), helpstring("property item")] HRESULT item([in] LONG idx, [out, retval] IDevice** pVal);
	[id(8), helpstring("method GetDevice")] HRESULT GetDevice([in] USHORT i2cAdress, [out,retval] IDevice** pVal);
	[propget, id(9), helpstring("property isConnected")] HRESULT isConnected([out, retval] VARIANT_BOOL* pVal);
	[id(10), helpstring("method Disconnect")] HRESULT Disconnect(void);
};

#include <map>
using namespace std;

// CServoController

[
	coclass,
	implements_category("CATID_SERVOCONTROLLER"),
	threading("apartment"),
	vi_progid("Servo.ServoController"),
	progid("Servo.ServoController.1"),
	version(1.0),
	uuid("BCC0DADC-C11E-4B4E-821B-5AB7FF623FB8"),
	helpstring("ServoController Class")
]
class ATL_NO_VTABLE CServoController : 
	public IServoController
{
	typedef map< UINT, IDevice* > Devices;
public:
	CServoController()
		: m_hDevice(INVALID_HANDLE_VALUE)
	{
	}


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease();

protected:
	HANDLE m_hDevice;
	Devices m_devices;

public:

	STDMETHOD(Connect)(BSTR device);
	STDMETHOD(Scan)();
	STDMETHOD(GetDevice)(USHORT i2cAddress, IDevice** pVal);
	STDMETHOD(Read)(USHORT address, BYTE* mem_address, USHORT mem_length, BYTE* data, USHORT data_len);
	STDMETHOD(Write)(USHORT address, BYTE* mem_address, USHORT mem_length, BYTE* data, USHORT data_len);
	STDMETHOD(get_count)(LONG* pVal);
	STDMETHOD(get_item)(LONG idx, IDevice** pVal);
	STDMETHOD(get_isConnected)(VARIANT_BOOL* pVal);
	STDMETHOD(Disconnect)(void);
};

