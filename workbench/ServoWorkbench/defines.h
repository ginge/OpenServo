/* This file contains message defines for notifications of system messages. These mostly occur between the mainframe and
	child windows that are not views. */

#pragma once

#define WM_SN_BASE				(WM_USER+0x01000)

#define WM_SN_CONNECTED			(WM_SN_BASE+0x0001)
#define WM_SN_DISCONNECTING		(WM_SN_BASE+0x0002)
#define WM_SN_DISCONNECTED		(WM_SN_BASE+0x0003)

