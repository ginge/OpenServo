
#ifndef OSIF_DLL_H
#define OSIF_DLL_H     //stop recursion

#define WIN

#define BUILD_DLL

#ifdef BUILD_DLL
/* DLL export */
#include "OSIFlib.h"
#define EXPORT __declspec(dllexport)
#else
/* EXE import */
#define EXPORT __declspec(dllimport)
#endif
#ifdef WIN
#define usleep(t) Sleep((t) / 1000)
#endif

EXPORT int OSIF_readbytes(int adapter, int servo, unsigned char addr, int length);
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
	  	  
#endif
