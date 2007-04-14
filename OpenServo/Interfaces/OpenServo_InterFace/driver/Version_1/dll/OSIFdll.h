
#ifndef OSIF_DLL_H
#define OSIF_DLL_H     //stop recursion

//you must define these. They are in the Makefile
//#define WIN
//#define BUILD_DLL
//#define BUILD_STATIC_LINKER


#ifdef BUILD_DLL
/* DLL export */
#define EXPORT __declspec(dllexport)
#endif

/* EXE import */
#ifdef BUILD_STATIC_LINKER
#define EXPORT __declspec(dllimport)
#endif

#ifdef WIN
#define usleep(t) Sleep((t) / 1000)
#include "win_usb.h"   
#else
#define EXPORT
#include <usb.h>
#endif

#include "OSIFlib.h"

EXPORT int OSIF_readbytes(int adapter, int servo, unsigned char addr, size_t length);
EXPORT int OSIF_readonly(int adapter, int servo, unsigned char * data, size_t buflen );
EXPORT int OSIF_write8(int adapter, int servo, unsigned char addr, int data);
EXPORT int OSIF_write16(int adapter, int servo, unsigned char addr, int data); 		
EXPORT int OSIF_init(void);
EXPORT int OSIF_deinit(void);
EXPORT int OSIF_read(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen );
EXPORT int OSIF_write(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen );
EXPORT int OSIF_scan(int adapter, int *devices, int *dev_count );
EXPORT bool OSIF_probe(int adapter, int servo );
EXPORT int OSIF_command(int adapter, int servo, unsigned char command);
EXPORT int OSIF_reflash(int adapter, int servo, int bootloader_addr, char *filename);
EXPORT int OSIF_get_adapter_name(int adapter, char* name);
EXPORT int OSIF_get_adapter_count(void);
	  	  
#endif
