/***************************************************************************
 *   Copyright (C) 2008 by Barry Carter,,,   *
 *   barry.carter@robotfuzz.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef ADAPTER_ENUMERATE_H
#define ADAPTER_ENUMERATE_H

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <stdio.h>

int adapter_enumerate(void);
int load_plugins(void);

/*Typedef the functions*/
typedef int  (*adapter_initfunc     )();
typedef int  (*adapter_deinitfunc   )();
typedef int  (*adapter_writefunc    )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
typedef int  (*adapter_readfunc     )(int adapter, int servo, unsigned char addr, unsigned char *data, size_t len);
typedef int  (*adapter_readonlyfunc )(int adapter, int servo, unsigned char *data, size_t len);
typedef int  (*adapter_reflashfunc  )(int adapter, int servo, int bootloader_addr, char *filename);
typedef int  (*adapter_commandfunc  )(int adapter, int servo, unsigned char command);
typedef int  (*adapter_scanfunc     )(int adapter, int devices[], int *dev_count);
typedef int  (*adapter_probefunc    )(int adapter, int servo);
typedef int  (*adapter_get_adapter_namefunc  )(int adapter, char *name);
typedef int  (*adapter_get_adapter_countfunc )(void);
typedef int  (*adapter_set_bitratefunc )(int adapter, int bitrate);


#ifdef WIN32
    HINSTANCE hdll; //Windows handle
#else
    void *libhandle; // handle to the shared lib when opened
#endif

#endif
