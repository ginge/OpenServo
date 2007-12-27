/***************************************************************************
 *   Copyright (C) 2007 by Barry Carter   *
 *   barry.carter@gmail.com   *
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
#include "osif.h"

OSIF::OSIF()
//load the OSIF dll
{
	OSIFinit = false;
#ifdef WIN32

	/*LoadLibrary*/
	WCHAR libname[20] = {'O','S','I','F','d','l','l','.','d','l','l'};
	
	hdll = LoadLibraryW(libname);

	if (!hdll) { printf("Error loading dll\n"); return; }

	/*GetProcAddress*/
	OSIF_init    = (OSIF_initfunc)GetProcAddress(hdll, "OSIF_init");
	OSIF_deinit  = (OSIF_deinitfunc)GetProcAddress(hdll, "OSIF_deinit");
	OSIF_write   = (OSIF_writefunc)GetProcAddress(hdll, "OSIF_write");
	OSIF_read    = (OSIF_readfunc)GetProcAddress(hdll, "OSIF_read");
	OSIF_readonly= (OSIF_readonlyfunc)GetProcAddress(hdll, "OSIF_readonly");
	OSIF_reflash = (OSIF_reflashfunc)GetProcAddress(hdll, "OSIF_reflash");
	OSIF_scan    = (OSIF_scanfunc)GetProcAddress(hdll, "OSIF_scan");
	OSIF_probe   = (OSIF_probefunc)GetProcAddress(hdll, "OSIF_probe");
	OSIF_command = (OSIF_commandfunc)GetProcAddress(hdll, "OSIF_command");
	OSIF_get_adapter_name  = (OSIF_get_adapter_namefunc)GetProcAddress(hdll, "OSIF_get_adapter_name");
	OSIF_get_adapter_count = (OSIF_get_adapter_countfunc)GetProcAddress(hdll, "OSIF_get_adapter_count");

#else

	libhandle = dlopen ( "libOSIFlib.so.1", RTLD_LAZY ); // open the shared lib

	// if the open failed, NULL was returned.  Print the error code
	if ( libhandle == NULL ) 
	{
		fprintf ( stderr, "fail 1: %s\n", dlerror() );
		return;
	} 

	/*GetProcAddress*/
	OSIF_init    = (OSIF_initfunc)dlsym(libhandle, "OSIF_init");
	OSIF_deinit  = (OSIF_deinitfunc)dlsym(libhandle, "OSIF_deinit");
	OSIF_write   = (OSIF_writefunc)dlsym(libhandle, "OSIF_write");
	OSIF_read    = (OSIF_readfunc)dlsym(libhandle, "OSIF_read");
	OSIF_reflash = (OSIF_reflashfunc)dlsym(libhandle, "OSIF_reflash");
	OSIF_scan    = (OSIF_scanfunc)dlsym(libhandle, "OSIF_scan");
	OSIF_probe   = (OSIF_probefunc)dlsym(libhandle, "OSIF_probe");
	OSIF_command = (OSIF_commandfunc)dlsym(libhandle, "OSIF_command");
	OSIF_get_adapter_name  = (OSIF_get_adapter_namefunc)dlsym(libhandle, "OSIF_get_adapter_name");
	OSIF_get_adapter_count = (OSIF_get_adapter_countfunc)dlsym(libhandle, "OSIF_get_adapter_count");

	// if bar is NULL, print() wasn't found in the lib, print error message	
	if ( OSIF_init == NULL ) 
	{
		char buf[255];
		sprintf(buf,"fail 2: %s", dlerror() );
		logPrint( buf );
	} 

#endif
	if (OSIF_init() < 0)
	{
		logPrint("Error initialising USB");
		//do some more stuff disable buttons 
	}
	else { OSIFinit = true; }

	servo = 0x00;
	adapterCount = 0;
	OpenServoCount = 0;
	genericDeviceCount = 0;
}


OSIF::~OSIF()
{
}

int OSIF::scanBus()
{
	int n;
	char devname[255];
	char logbuf[255];

	//check to see if the bus is initialised. If so deinitialise and rescan all busses.
	//thisis the only way to detect for new adapters on the bus.
	if (OSIFinit)
	{
		OSIF_deinit();
		if (OSIF_init()<0)
		{
			logPrint("Error: No compatible adapters found");
			OSIFinit =false;
			return -1;
		}
		else
		{ 
			OSIFinit =true;
		}
	}

	if (OSIFinit == false )
	{
		//error no adapters
		return -1;
	}
	//get list of adapters
	adapterCount = OSIF_get_adapter_count();

	if (adapterCount >=0)
	{
		for (n=0;n<=adapterCount;n++)
		{
			OSIF_get_adapter_name(n, &devname[0]);
			sprintf( logbuf, "Found adapter %s", devname);
			logPrint(logbuf);
		}
	}
	else
	{
		logPrint( "No adapters found, but got this far regardless. Something went horribly wrong.");
		return -1;
	}

	adapter = 0;
	return 1;
}

int OSIF::scanDevices(int adapterScan)
{
	int n;
	OSIF_scan( adapterScan, devices, &devCount );

	char logbuf[255];
	unsigned char buf[2];

	for( n = 0; n< devCount; n++)
	{
		//check to see if it is an openservo
		if (readData(adapterScan,devices[n],0x00,buf,1)>0)
		{
			if (buf[0] == 0x01)
			{
				sprintf( logbuf, "OpenServo at 0x%02x", devices[n]);
				logPrint( logbuf );
				OpenServo[OpenServoCount] = devices[n];
				OpenServoCount++;
			}
			else	//not an OpenServo. Add to other list
			{
				sprintf( logbuf, "I2C device at 0x%02x", devices[n]);
				logPrint( logbuf );
				genericDevices[genericDeviceCount] = devices[n];
				genericDeviceCount++;
			}
		}
	}

	if( OpenServoCount >0)
	{
		return OpenServoCount;
	}
	else
	{
		return -1;
	}
}

int OSIF::writeData( int adapter, int servo, int addr, char *val, size_t len )
{

	//check validity of options
	if (!OSIFinit || adapterCount <0)
	{
		logPrint("No adapters!");
		return -1;
	}

	//write
	if (OSIF_write(adapter,servo,addr,(unsigned char *)val,len) < 0)
	{
		logPrint("Write failed");
		return -1;
	}
	logPrint("Wrote data OK");
	return 1;
}

int OSIF::readData(int adapter, int servo, int addr, unsigned char *buf, size_t len)
{
	if (!OSIFinit || adapterCount <0)
	{
		logPrint("No adapters!");
		return -1;
	}

	if (OSIF_read(adapter,servo,addr,buf,len) < 0)
	{
		logPrint("I2C read failed");
		return -1;
	}
	logPrint("Read data OK");
	return 1;
}

int OSIF::deinit(void)
{
	if (isOSIFinitialised() )
	{
		OSIF_deinit();
#ifdef WIN32
		FreeLibrary((HMODULE)hdll);
#else
		dlclose(libhandle);
#endif
		OSIFinit = false;
		return 1;
	}

	return -1;
}

void OSIF::logPrint( char *logData)
{
	printf("%s\n", logData);
	sprintf(lastError, "%s",logData);
}


