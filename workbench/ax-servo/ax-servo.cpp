// ax-servo.cpp : Implementation of DLL Exports.

#include "stdafx.h"
#include "resource.h"

// The module attribute causes DllMain, DllRegisterServer and DllUnregisterServer to be automatically implemented for you
[ module(dll, uuid = "{D92D9986-B85C-47D1-ABCA-91F22FCD3D4C}", 
		 name = "axservo", 
		 helpstring = "ax-servo 1.0 Type Library",
		 resource_name = "IDR_AXSERVO") ]
class CaxservoModule
{
public:
// Override CAtlDllModuleT members
};
		 
