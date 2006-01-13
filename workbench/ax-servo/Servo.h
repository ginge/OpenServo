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


