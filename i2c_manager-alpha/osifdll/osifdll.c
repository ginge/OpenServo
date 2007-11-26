/*
 * Author       : Kevin Black
 * Date written : 15 July 2007
 * Licensing    :
 *
 * This software is released under the MIT license.
 *
    Copyright (c) 2007 Kevin Black

    Permission is hereby granted, free of charge, to any person 
    obtaining a copy of this software and associated documentation 
    files (the "Software"), to deal in the Software without 
    restriction, including without limitation the rights to use, copy, 
    modify, merge, publish, distribute, sublicense, and/or sell copies 
    of the Software, and to permit persons to whom the Software is 
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be 
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.

 * Synopsis     :
 *
 * Implements an OSIF DLL/shared library version that uses the "I2C Manager" layer
 * rather than directly interacting with specific hardware (as with the original
 * "OSIF DLL" for the OSIF board by Barry Carter) allowing an OSIF DLL/shared librtary
 * that can support more than one hardware interface board and more than one at
 * the same time (for example: Barry Carter's OSIF and/or the "Diolan/Dimax U2C12
 * USB-I2C/SPI/GPIO").
 *
 * NOTE: This does not use the "Open Servo Interface" (OSI) library.
 */
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdio.h>

#include "../i2cm/i2c_manager.h"

/*
 * Include the "OSIF DLL" header files...
 *
 * NOTE: The OSIF DLL header file to be included is the one that defines the
 *       OSIF interface, as that is the specification to which this DLL must
 *       work.
 */
#ifdef _WINDOWS
#define WIN
#define BUILD_DLL
#endif
#include "../../../OpenServo/OpenServo/Interfaces/OpenServo_InterFace/driver/Version_2/dll/osifdll.h"

/*******************************************************************************
 *
 * Locally defined structures
 */
#pragma pack(push)
#pragma pack(1) /* Prefered packing */

/*******************************************************************************
 *
 */
#define OSIF_SUCCESS                    0
#define OSIF_ERROR                     -1

/*******************************************************************************
 *
 * static and global data
 *
 * An assumption is made that an application wants global access, therefore,
 * so that handles, etc., do not need to be managed and passed around by the
 * application, many items are statically allocated or allocated to static
 * pointers, etc.
 *
 * static BOOL l_bInit
 *        o Initialised to 1 (TRUE), any non-zero values indicates that the OpenServo
 *          I2C software interface has not been initialised (or has subsequently been
 *          ended). Set to 0 when initialisation has been sucessfully performed. Any
 *          value other than 0 or 1 probably indicates memory corruption.
 */
static BOOL l_bInit=1;

/*******************************************************************************
 *
 * Endian detection (OpenServo registers are "big endian", requiring conversion
 * of 16-bit register values, etc., on little endian platforms).
 *
 * NOTE: Rather the something of the form
 *
 *          #if defined(__sparc) || defined(__mips)
 *          #define __BIG_ENDIAN__
 *          #else
 *          #define __LITTLE_ENDIAN__
 *          #endif
 *
 *       this time we are using the stored value method.
 */
BYTE m_bIsLittleEndian=0; /* NOTE: This is set to the correct value in OSI_Init, */
                          /*       as it is not really useful until after that   */
                          /*       function has been called.                     */

/* TODO: Consider if the above decision is the correct one, using the #ifdef __LITTLE_ENDIAN__
 *       strategy can remove code on big endian machines. Does it really matter?
 */

/*******************************************************************************
 *
 * Versioning and copyright information
 *
 * NOTE: You may reduce this information when used with a microcontroller.
 */
#ifndef HW_MCU_AVR
static const char *l_pVersion="Version 1.0 alpha 2007 11 24 OSIF via I2C Manager";
static const char *l_pCopyright="Copyright (c) 2007 OpenServo Project";
#endif
static const char *l_pVersionShort="V 1.0 A 20071124 OSIF";
static const char *l_pCopyrightShort="C 2007";

/*******************************************************************************
 *
 * BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
 */
#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE hModule,  DWORD  ul_reason_for_call,  LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
      case DLL_PROCESS_ATTACH:
      case DLL_THREAD_ATTACH:
      case DLL_THREAD_DETACH:
      case DLL_PROCESS_DETACH:
      break;
    }
    return TRUE;
}
#endif

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_init(void)
{
   int rc=OSIF_ERROR;

/*
 * Set the endian flag, m_bIsLittleEndian, "TRUE" on little endian platforms
 */
   union { uint16_t i; byte_t b[2]; } k;
   k.i=256;
   m_bIsLittleEndian=k.b[1];

/*
 * OSI needs to have been initialised (by the application)
 */
   if(!I2CM_IsInitialised())
   {
      I2CM_Init();
   }
   if(I2CM_IsInitialised())
   {
      if(l_bInit!=0)
      {
         rc=OSIF_SUCCESS;
         l_bInit=0;
      }
   } else
   {
      if(l_bInit==0)
      {
         OSIF_deinit();
         l_bInit=1;
      }
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_deinit(void)
{
   int rc=OSIF_ERROR;
   if(l_bInit==0)
   {
      rc=OSIF_SUCCESS;
   }
   if(!I2CM_Shutdown())
   {
      rc=OSIF_ERROR;
   }
   l_bInit=1;
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_get_adapter_count(void)
{
   return I2CM_GetBusCount()-1; /* !!!? */
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_get_adapter_name(int adapter, char* name)
{
   int rc=OSIF_ERROR;
   if(I2CM_GetBusName(adapter,name,255)==I2CM_SUCCESS)
   {
      rc=1; /* ! */
   } else
   {
      name[0]='\0';
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_scan(int adapter, int *devices, int *dev_count)
{
   int rc=OSIF_ERROR;
   int n=I2CM_GetDeviceCount(adapter);
   if(n>=0)
   {
      int i,n2=0;
      for(i=0; i<n; i++)
      {
         DWORD nAddress=I2CM_GetDeviceAddress(adapter,(BYTE)i);
         if(nAddress>=0 && nAddress<=0xFFFF && "IsAnOpenServo")
         {
            devices[n2++]=(int)nAddress;
         }
      }
      *dev_count=n2;
      rc=1; /* ! */
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_readbytes(int adapter, int servo, unsigned char addr, size_t length)
{
   BYTE buffer[2];
   int rc=OSIF_SUCCESS;
   if(length<1 || length<sizeof(buffer) || I2CM_Read((int_t)adapter,(int_devaddr_t)servo,1,(int_memaddr_t)addr,(byte_t)length,buffer)!=0)
   {
      rc=OSIF_ERROR;
   } else
   {
      if(length==1)
      {
         rc=(int)buffer[0];
      } else
      {
         if(m_bIsLittleEndian)
         {
            rc=(int)buffer[0]<<8 | buffer[1];
         } else
         {
            rc=(int)buffer[1]<<8 | buffer[0];
         }
      }
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_readonly(int adapter, int servo, unsigned char * data, size_t buflen)
{
   int rc=OSIF_SUCCESS;
   if(I2CM_Read((int_t)adapter,(int_devaddr_t)servo,0,0,(byte_t)buflen,data)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_write8(int adapter, int servo, unsigned char addr, int data)
{
   int rc=OSIF_SUCCESS;
   BYTE b=(BYTE)data; /* Cannot pass int directly on bigendian platforms */
   if(I2CM_Write((int_t)adapter,(int_devaddr_t)servo,1,(int_memaddr_t)addr,sizeof(byte_t),&b)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_write16(int adapter, int servo, unsigned char addr, int data)
{
   BYTE buffer[2];
   int rc=OSIF_SUCCESS;
   if(m_bIsLittleEndian)
   {
      buffer[0]=(data/256)%256;
      buffer[1]=data%256;
   } else
   {
      buffer[1]=(data/256)%256;
      buffer[0]=data%256;
   }
   if(I2CM_Write((int_t)adapter,(int_devaddr_t)servo,1,(int_memaddr_t)addr,sizeof(buffer),buffer)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_read(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen)
{
   int rc=OSIF_SUCCESS;
   if(I2CM_Read((int_t)adapter,(int_devaddr_t)servo,1,(int_memaddr_t)addr,(int_t)buflen,data)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_write(int adapter, int servo, unsigned char addr, unsigned char * data, size_t buflen)
{
   int rc=OSIF_SUCCESS;
   if(I2CM_Write((int_t)adapter,(int_devaddr_t)servo,1,(int_memaddr_t)addr,(byte_t)buflen,data)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT bool OSIF_probe(int adapter, int servo)
{
   fprintf(stderr,"ERROR: OSIFDLL, function OSIF_probe not implemented\n");
   fflush(stderr);
   return false;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_command(int adapter, int servo, unsigned char command)
{
   int rc=OSIF_SUCCESS;
   if(I2CM_Write((int_t)adapter,(int_devaddr_t)servo,0,0,sizeof(byte_t),&command)!=0)
   {
      rc=OSIF_ERROR;
   }
   return rc;
}

/*******************************************************************************
 *
 *
 */
EXPORT int OSIF_reflash(int adapter, int servo, int bootloader_addr, char *filename)
{
   fprintf(stderr,"ERROR: OSIFDLL, function OSIF_reflash not implemented\n");
   fflush(stderr);
   return OSIF_ERROR;
}
