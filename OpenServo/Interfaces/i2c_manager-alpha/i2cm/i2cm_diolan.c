/*
 * Author       : Kevin Black
 * Date written : 29 June 2007
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

 *
 * Synopsis     :
 *
 * Implements the I2C Manager interface functions for the Dimax/Diolan U2C12 USB-I2C/SPI/GPIO.
 *
 * See i2c_manager.h for further information.
 *
 * Tested platforms: Microsoft Visual C++ 6.0 (Windows 2000/XP/Vista)
 *                   gcc version 4.1.2 (Ubuntu 4.1.2-0ubuntu4)
 *
 *        Hardware:  Dimax/Diolan U2C12 USB-I2C/SPI/GPIO Interface
 *
 * NOTE: Synopsis and parameter details of the functions that this module implements
 *       can be found in i2c_manager.c.
 */
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/*******************************************************************************
 *
 * This module:
 *
 *    o Although this module unlikely to be used on a microcontroller, it could
 *      still uses the #define trick (described below) to alter the names of the
 *      interface functions
 */

/*
 * If the function is to be used on its own, then do not define I2CM_CONFIG_MAKESTATIC,
 * the functions will then be directly accessible using the generic names. If
 * I2CM_CONFIG_MAKESTATIC is defined, then the functions will only be accessible
 * using the I2C Manager.
 *
 * NOTE: It would be possible to get access to the functions using both methods, in
 *       the same application, by using the #define I2CM_Read Diolan_I2CM_Read "trick".
 */
#define I2CM_CONFIG_MAKESTATIC
#include "i2c_manager.h"

#if 0 /* NOTE: Example of method to rename the functions from the generic names */
#define I2CM_Init Diolan_I2CM_Init
#define I2CM_GetLastError Diolan_I2CM_GetLastError
#define I2CM_GetLastErrorText Diolan_I2CM_GetLastErrorText
#define I2CM_SetLastError Diolan_I2CM_SetLastError
#define I2CM_GetBusLastErrorText Diolan_I2CM_GetBusLastErrorText
#define I2CM_IsInitialised Diolan_I2CM_IsInitialised
#define I2CM_Shutdown Diolan_I2CM_Shutdown
#define I2CM_GetBusCount Diolan_I2CM_GetBusCount
#define I2CM_GetBusHardwareName Diolan_I2CM_GetBusHardwareName
#define I2CM_GetBusName Diolan_I2CM_GetBusName
#define I2CM_GetBusID Diolan_I2CM_GetBusID
#define I2CM_RescanForDevices Diolan_I2CM_RescanForDevices
#define I2CM_GetDeviceCount Diolan_I2CM_GetDeviceCount
#define I2CM_GetBusDeviceCount Diolan_I2CM_GetBusDeviceCount
#define I2CM_GetDeviceAddress Diolan_I2CM_GetDeviceAddress
#define I2CM_Read Diolan_I2CM_Read
#define I2CM_Write Diolan_I2CM_Write
#define I2CM_StartTransaction Diolan_I2CM_StartTransaction
#define I2CM_RestartTransaction Diolan_I2CM_RestartTransaction
#define I2CM_StopTransaction Diolan_I2CM_StopTransaction
#define I2CM_PutAck Diolan_I2CM_PutAck
#define I2CM_GetAck Diolan_I2CM_GetAck
#define I2CM_PutByte Diolan_I2CM_PutByte
#define I2CM_GetByte Diolan_I2CM_GetByte
#define I2CM_PutBytes Diolan_I2CM_PutBytes
#define I2CM_GetBytes Diolan_I2CM_GetBytes
#endif

/*******************************************************************************
 *
 * Things needed to access the Diolan I2C bridge.
 *
 * NOTE: You may need to set the project path to point to the installed location
 *       of the i2cbridge.h.
 */
#if defined(__linux__) && defined(__cplusplus)
extern "C" { /* Note: LOL! See i2cm_diolan.cpp */
#endif
#include <i2cbridge.h>
#if defined(__linux__) && defined(__cplusplus)
};
#endif

/*******************************************************************************
 *
 * Deveopment environment fix ups
 */
#ifndef _MSC_VER
#define stricmp strcasecmp
#else
#define snprintf _snprintf
#endif

/*******************************************************************************
 *
 * If a DLL is being built, then a DLL entry point is required
 *
 * TODO: Possibly the code needs to be updated to share it between applications
 *       when used in DLL form. Although, a quick test on Windows with Barry
 *       Carter's OpenServoInterface test application suggests not.
 */
#if defined(WIN32) && defined(_USRDLL)
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
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
 * Set the correct linkage once and for all
 */
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *
 * Locally defined structures
 */
#pragma pack(push)
#pragma pack(1) /* Prefered packing */

/*
 * typedef struct I2CBUS { ... } I2CBUS
 *
 * A structure that contains information about an I2C bus, and variables used to
 * access it.
 *
 * Members
 *
 * int_t m_nDevices
 *        o Gets the number of devices associated with the bus (at time that a scan for
 *          devices is performed).
 *
 * int_devaddr_t *m_pAddr
 *        o Point to an array of m_nDevices that contains the addresses of the known devices
 *          on the bus.
 * 
 * HANDLE m_hDevice
 *         o Gets a handle to the I2C bus device when it has been opened thru a
 *           call to U2C_OpenDevice, otherwise NULL.
 *
 * bool_t m_bTransactionInProgress
 *         o Set TRUE if a transaction has been started but has not ended.
 */
typedef struct I2CBUS
{
   int_t m_nDevices;
   int_devaddr_t *m_pAddr;
   HANDLE m_hDevice;
   bool_t m_bTransactionInProgress;
} I2CBUS;

/*******************************************************************************
 *
 * static and global data
 *
 * An assumption is made that an application wants global access, therefore,
 * so that handles, etc., do not need to be managed and passed around by the
 * application, many items are statically allocated or allocated to static
 * pointers, etc.
 *
 * static bool_t l_bInit
 *        o Initialised to 1 (TRUE), any non-zero values indicates that the
 *          I2C software interface has not been initialised (or has subsequently
 *          been ended). Set to 0 when initialisation has been sucessfully
 *          performed. Any value other than 0 or 1 probably indicates memory
 *          corruption.
 */
static bool_t l_bInit=1;

/*
 * static int_t l_nLastError
 *        o Set to the code of the last error that occurred (0 if there have been no
 *          errors). The application can set the value to 0 to clear the last error
 *          (by calling the I2CM_SetLastError function). The I2C software interface
 *          passes integer error codes, rather than strings, to save on microcontroller
 *          memory space.
 *
 * static char l_sLastErrorAdditional[32]
 *         o When an error occurs and l_nLastError is set, this buffer is set to
 *           either an empty string ([0]=='\0') or to the text representation of
 *           additional information (e.g. the I2C bus number and servo address).
 *           This string is set to an emprt string when the I2CM_SetLastError
 *           function is called with NULL as the second argument.
 */
static int_t l_nLastError;
static char l_sLastErrorAdditional[32];

/*
 * static int_t l_nI2CBuses
 *         o The number of I2C buses.
 *
 *           NOTE: Buses are numbered sequentially starting from 0. Buses that are
 *                 added to the system are not included unless a rescan is performed,
 *                 however, changes in bus numbering is outside of the control of this
 *                 code- i.e. a bus that was previously bus 1, might be bus 2 after
 *                 additional hardware comes on-line and a rescan performed.
 *
 * static I2CBUS *l_pI2CBuses
 *         o Points to an array of l_nI2CBuses I2CBUS structures holding a bus descriptor
 *           for each of the buses that has been recognized. Initialised to NULL.
 */
static int_t l_nI2CBuses=0;
static I2CBUS *l_pI2CBuses=NULL;

/*******************************************************************************
 *
 * Versioning and copyright information
 *
 * NOTE: You may reduce this information when used with a microcontroller.
 */
#ifndef HW_MCU_AVR
static const char *l_pVersion="Version 1.0 alpha 2007 11 24 I2C Manager layer for Dimax/Diolan U2C12 USB-I2C/SPI/GPIO Interface";
static const char *l_pCopyright="Copyright (c) 2007 Kevin Black";
#endif
static const char *l_pVersionShort="V 1.0 A 20071124 Dimax";
static const char *l_pCopyrightShort="(c) 2007";

/*******************************************************************************
 *
 * I2CM_GetLastError, I2CM_GetLastErrorAdditionalText and I2CM_SetLastError are
 * implemented first, this allows them to be functions callable by the rest
 * of the application, but their use in this module to be macroed, to save on
 * space and speed.
 */
I2CMSTATIC int_t I2CM_GetLastError()
{
   return l_nLastError;
}
#define I2CM_GetLastError() l_nLastError
I2CMSTATIC const char *I2CM_GetLastErrorText()
{
   return l_sLastErrorAdditional;
}
#define I2CM_GetLastErrorText() l_sLastErrorAdditional
I2CMSTATIC int_t I2CM_SetLastError(int_t nerror, const char *pat)
{
   l_nLastError=nerror;
   if(pat==NULL)
   {
      l_sLastErrorAdditional[0]='\0';
   } else
   {
      strncpy(l_sLastErrorAdditional,pat,sizeof(l_sLastErrorAdditional)-1);
      l_sLastErrorAdditional[sizeof(l_sLastErrorAdditional)-1]='\0';
   }
   return nerror;
}
#define I2CM_SetLastError(NERROR,PAT) ((void *)PAT==NULL?l_sLastErrorAdditional[0]='\0':(strncpy(l_sLastErrorAdditional,PAT,sizeof(l_sLastErrorAdditional)-1),l_sLastErrorAdditional[sizeof(l_sLastErrorAdditional)-1]='\0'),l_nLastError=NERROR)

/*******************************************************************************
 *
 * I2CMPROC Diolan_I2CM_GetProcAddress(int_t nfunc)
 *
 * Returns the function pointer (address) of a specified I2C layer function for
 * this I2C layer (for example, this allows the I2C Manager to retrieve the address
 * of the I2CM_Read function for this layer implementation).
 *
 * NOTE: The function returns an anonymous function pointer, which needs to be
 *       cast.
 *
 *       The function uses an integer to define the functions, rather than a string
 *       such as "I2CM_Read", as this will save space in a small memory environment
 *       (such as a microcontroller).
 *
 *       If a DLL version of the application is being built, then the Diolan_I2CM_GetProcAddress
 *       function is "exported".
 *
 * Parameters
 *
 * int_t nfunc
 *         o Defines the function for which the address is to be returned.
 *
 * Return
 *
 * I2CMPROC o The function returns the address of the function requested. If the request
 *           is invalid, then NULL is returned. Extended error information may be
 *           available by calling the I2CM_GetLastError or I2CM_GetLastErrorText functions
 *           for this layer.
 */
#if defined(WIN32) && defined(_USRDLL)
__declspec(dllexport)
#endif
I2CMPROC Diolan_I2CM_GetProcAddress(int_t nfunc)
{
   I2CMPROC p=NULL;
#include "i2c_manager_procaddrmap.h"
   return p;
}

/*******************************************************************************
 *
 * I2CMSTATIC int_t I2CM_init()
 *
 * This function initialises this I2C layer.
 *
 * NOTE: If the I2C layer is already initialised, then the function does nothing,
 *       but still returns success (to support multiple callers).
 *
 * Parameters
 *
 *         o None
 *
 * Return
 *
 * int_t   o The function returns I2CM_SUCCESS if initialisation has been completed
 *           successfully, otherwise an error code is returned.
 */
I2CMSTATIC int_t I2CM_Init()
{
   if(l_bInit==0)
   {
      I2CM_SetLastError(I2CM_ERR_ALREADYINITIALISED,NULL);
   } else
   {
      I2CM_SetLastError(-1,NULL);

/*
 * Initialise the hardware and get the number of I2C buses
 */
      l_nI2CBuses=U2C_GetDeviceCount();
      if(l_nI2CBuses==0)
      {
         I2CM_SetLastError(I2CM_ERR_HARDWARE,"Dimax: no devices");
      } else
      {

/*
 * Allocate and initialise the array of I2CBUS structures
 */
         l_pI2CBuses=(I2CBUS *)malloc(l_nI2CBuses*sizeof(I2CBUS));
         if(l_pI2CBuses==NULL)
         {
            I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,"I2CBUS array");
         } else
         {
            bool_t bOK=TRUE;
            int_t i;
            for(i=0; i<l_nI2CBuses; i++)
            {
               l_pI2CBuses[i].m_nDevices=0;
               l_pI2CBuses[i].m_pAddr=NULL;
               l_pI2CBuses[i].m_hDevice=NULL;
               l_pI2CBuses[i].m_bTransactionInProgress=FALSE;
            }

/*
 * Open all the I2C buses
 */
            for(i=0; bOK && i<l_nI2CBuses; i++)
            {
               HANDLE hDevice=U2C_OpenDevice(i);
               if(hDevice==NULL || hDevice==INVALID_HANDLE_VALUE)
               {
                  I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
                  sprintf(l_sLastErrorAdditional,"Bus: %d",i);
                  bOK=FALSE;
               } else
               {
                  if(U2C_SetClockSynch(hDevice,TRUE)!=U2C_SUCCESS)
                  {
                     I2CM_SetLastError(I2CM_ERR_HARDWARE,"No synch");
                     bOK=FALSE;
                     U2C_CloseDevice(hDevice);
                  } else
                  {
                     l_pI2CBuses[i].m_hDevice=hDevice;
                  }
               }
            }

/*
 * If all is okay, scan the buses for devices
 */
            if(bOK)
            {
               l_bInit=0; /* Otherwise I2CM_RescanForDevices will fail */
               if(I2CM_RescanForDevices()!=I2CM_SUCCESS)
               {
                  l_bInit=1;
               }
            }
         }
      }
      if(l_bInit)
      {
         l_bInit=0; /* Needed for a successfull call to I2CM_Shutdown */
         I2CM_Shutdown();
      } else
      {
         I2CM_SetLastError(I2CM_SUCCESS,NULL);
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_IsInitialised()
 */
I2CMSTATIC int_t I2CM_IsInitialised()
{
   return l_bInit==0;
}

/*******************************************************************************
 *
 * int_t I2CM_Shutdown()
 */
I2CMSTATIC int_t I2CM_Shutdown()
{
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t i;
      for(i=0; i<l_nI2CBuses; i++)
      {
         HANDLE hDevice=l_pI2CBuses[i].m_hDevice;
         if(hDevice!=NULL && hDevice!=INVALID_HANDLE_VALUE)
         {
            U2C_CloseDevice(hDevice);
         }
         l_pI2CBuses[i].m_hDevice=NULL;
         if(l_pI2CBuses[i].m_pAddr!=NULL)
         {
            free(l_pI2CBuses[i].m_pAddr);
            l_pI2CBuses[i].m_pAddr=NULL;
         }
         l_pI2CBuses[i].m_nDevices=0;
      }
      free(l_pI2CBuses);
      l_pI2CBuses=NULL;
      l_nI2CBuses=0;
      l_bInit=1;
      I2CM_SetLastError(I2CM_SUCCESS,NULL);
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusCount()
 */
I2CMSTATIC int_t I2CM_GetBusCount()
{
   return l_bInit!=0 ? -1 : l_nI2CBuses;
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusHardwareName(int_t nBus, char *pbuffer, int_t n)
 */
int_t I2CM_GetBusHardwareName(int_t nBus, char *pbuffer, int_t n)
{
static const char STR_HARDWARENAME[]="U2C12";
   if(n>0)
   {
      pbuffer[0]='\0';
   }
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      if(nBus<0 || nBus>=l_nI2CBuses || n<sizeof(STR_HARDWARENAME))
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         strncpy(pbuffer,STR_HARDWARENAME,n);
         I2CM_SetLastError(I2CM_SUCCESS,NULL);
      }
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_GetBusID(int_t nBus, unsigned long *pbusid)
 */
int_t I2CM_GetBusID(int_t nBus, unsigned long *pbusid)
{
   *pbusid=0xFFFF;
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      if(nBus<0 || nBus>=l_nI2CBuses)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(U2C_GetSerialNum(l_pI2CBuses[nBus].m_hDevice,(long *)pbusid)!=U2C_SUCCESS)
         {
            I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
         } else
         {
            I2CM_SetLastError(I2CM_SUCCESS,NULL);
         }
      }
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_GetBusName(int_t nBus, char *pbuffer, int_t n)
 */
int_t I2CM_GetBusName(int_t nBus, char *pbuffer, int_t n)
{
   if(n>0)
   {
      pbuffer[0]='\0';
   }
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      if(I2CM_GetBusHardwareName(nBus,pbuffer,n)==I2CM_SUCCESS)
      {
         int_t k=strlen(pbuffer);
         if(k+12>n)
         {
            pbuffer[0]='\0';
            I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
         } else
         {
            unsigned long id;
            if(U2C_GetSerialNum(l_pI2CBuses[nBus].m_hDevice,(long *)&id)!=U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            } else
            {
               snprintf(&pbuffer[k],n-k,"_0x%08X",id);
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            }
         } 
      }
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_RescanForDevices()
 *
 * Scans the I2C buses for devices.
 *
 * Parameters
 *
 * None.
 *
 * Returns
 *
 * int_t   o The function returns 0 if successful. A non-zero return indidates
 *           that an error has occurred.
 */
I2CMSTATIC int_t I2CM_RescanForDevices()
{
   if(l_bInit!=0 || l_nI2CBuses<1)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t nBus;
      I2CM_SetLastError(I2CM_SUCCESS,NULL);
      for(nBus=0; nBus<l_nI2CBuses; nBus++)
      {
         I2CBUS *pbus=&l_pI2CBuses[nBus];
         if(pbus->m_pAddr!=NULL)
         {
            free(pbus->m_pAddr);
            pbus->m_pAddr=NULL;
         }
         pbus->m_nDevices=0;
      }
      for(nBus=0; I2CM_GetLastError()==I2CM_SUCCESS && nBus<l_nI2CBuses; nBus++)
      {
         I2CBUS *pbus=&l_pI2CBuses[nBus];
         pbus->m_nDevices=0;
         if(pbus->m_hDevice==NULL)
         {
            I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
         } else
         {
            if(pbus->m_bTransactionInProgress)
            {
               I2CM_SetLastError(I2CM_ERR_TRANSACTIONEXISTS,NULL);
            } else
            {
               U2C_SLAVE_ADDR_LIST *plist=(U2C_SLAVE_ADDR_LIST *)malloc(sizeof(U2C_SLAVE_ADDR_LIST));
               if(plist==NULL)
               {
                  I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
               } else
               {
                  if(U2C_ScanDevices(pbus->m_hDevice,plist)!=U2C_SUCCESS)
                  {
                     I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
                  } else
                  {
                     if(plist->nDeviceNumber==0)
                     {
                        I2CM_SetLastError(I2CM_SUCCESS,NULL);
                     } else
                     {
                        pbus->m_pAddr=(int_devaddr_t *)malloc(plist->nDeviceNumber*sizeof(int_devaddr_t));
                        if(pbus->m_pAddr==NULL)
                        {
                           I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
                        } else
                        {
                           int_t i;
                           pbus->m_nDevices=plist->nDeviceNumber;
                           for(i=0; i<pbus->m_nDevices; i++)
                           {
                              pbus->m_pAddr[i]=plist->List[i];
                           }
                        }
                     }
                  }
                  free(plist);
               }
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusDeviceCount(int_t nBus)
 */
I2CMSTATIC int_t I2CM_GetBusDeviceCount(int_t nBus)
{
   int_t rc=-1;
   if(l_bInit==0 && nBus>=0 && nBus<l_nI2CBuses)
   {
      rc=l_pI2CBuses[nBus].m_nDevices;
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t I2CM_GetDeviceCount()
 */
I2CMSTATIC int_t I2CM_GetDeviceCount()
{
   int_t rc=-1;
   if(l_bInit==0)
   {
      int_t i;
      rc=0;
      for(i=0; i<l_nI2CBuses; i++)
      {
         rc+=l_pI2CBuses[i].m_nDevices;
      }
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t I2CM_GetDeviceAddress(int_t nBus, int_t nDevice)
 */
I2CMSTATIC int_t I2CM_GetDeviceAddress(int_t nBus, int_t nDevice)
{
   int_t rc=(int_t)-1;
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(nDevice<0 || nDevice>=pbus->m_nDevices)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         rc=(DWORD)pbus->m_pAddr[nDevice];
      }
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, byte_t *pBuffer)
 */
I2CMSTATIC int_t I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, byte_t *pBuffer)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_TRANSACTIONEXISTS,NULL);
         } else
         {
            U2C_TRANSACTION *ptrans=(U2C_TRANSACTION *)malloc(sizeof(U2C_TRANSACTION)-sizeof(ptrans->Buffer)+nBytes);
            if(ptrans==NULL)
            {
               I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
            } else
            {
	            ptrans->nSlaveDeviceAddress=(unsigned char)nSlaveAddress;
	            ptrans->nMemoryAddressLength=nMemAddressLen;
	            ptrans->nMemoryAddress=nMemAddress;
	            ptrans->nBufferLength=nBytes;
               if(U2C_Read(pbus->m_hDevice,ptrans)!=U2C_SUCCESS)
               {
                  I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
               } else
               {
                  memcpy(pBuffer,ptrans->Buffer,nBytes);
                  I2CM_SetLastError(I2CM_SUCCESS,NULL);
               }
               free(ptrans);
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_Write(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, const byte_t *pBuffer)
 */
I2CMSTATIC int_t I2CM_Write(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, const byte_t *pBuffer)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_TRANSACTIONEXISTS,NULL);
         } else
         {
            U2C_TRANSACTION *ptrans=(U2C_TRANSACTION *)malloc(sizeof(U2C_TRANSACTION)-sizeof(ptrans->Buffer)+nBytes);
            if(ptrans==NULL)
            {
               I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
            } else
            {
	            ptrans->nSlaveDeviceAddress=(unsigned char)nSlaveAddress;
	            ptrans->nMemoryAddressLength=nMemAddressLen;
	            ptrans->nMemoryAddress=nMemAddress;
	            ptrans->nBufferLength=nBytes;
               memcpy(ptrans->Buffer,pBuffer,nBytes);
               if(U2C_Write(pbus->m_hDevice,ptrans)!=U2C_SUCCESS)
               {
                  I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
               } else
               {
                  I2CM_SetLastError(I2CM_SUCCESS,NULL);
               }
               free(ptrans);
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_StartTransaction(int_t nBus)
 */
I2CMSTATIC int_t I2CM_StartTransaction(int_t nBus)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_TRANSACTIONEXISTS,NULL);
         } else
         {
            if(U2C_Start(pbus->m_hDevice)!=U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            } else
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
               pbus->m_bTransactionInProgress=TRUE;
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_RestartTransaction(int_t nBus)
 */
I2CMSTATIC int_t I2CM_RestartTransaction(int_t nBus)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            if(U2C_RepeatedStart(pbus->m_hDevice)!=U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            } else
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
               pbus->m_bTransactionInProgress=TRUE;
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_StopTransaction(int_t nBus)
 */
I2CMSTATIC int_t I2CM_StopTransaction(int_t nBus)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            if(U2C_Stop(pbus->m_hDevice)!=U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            } else
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            }
         }
         pbus->m_bTransactionInProgress=FALSE;
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetByte(int_t nBus, byte_t *pnByte)
 */
I2CMSTATIC int_t I2CM_GetByte(int_t nBus, byte_t *pnByte)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            U2C_RESULT lrc=U2C_GetByte(pbus->m_hDevice,pnByte);
            if(lrc==U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            } else
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_PutAck(int_t nBus, bool_t bAck)
 */
I2CMSTATIC int_t I2CM_PutAck(int_t nBus, bool_t bAck)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            U2C_RESULT lrc=U2C_PutAck(pbus->m_hDevice,bAck);
            if(lrc==U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_PutByte(int_t nBus, byte_t nByte)
 */
I2CMSTATIC int_t I2CM_PutByte(int_t nBus, byte_t nByte)
{
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            U2C_RESULT lrc=U2C_PutByte(pbus->m_hDevice,nByte);
            if(lrc==U2C_SUCCESS)
            {
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            } else
            {
               I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetAck(int_t nBus, bool_t *pbAck)
 */
I2CMSTATIC int_t I2CM_GetAck(int_t nBus, bool_t *pbAck)
{
   *pbAck=FALSE;
   if(nBus<0 || nBus>=l_nI2CBuses)
   {
      I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
   } else
   {
      I2CBUS *pbus=&l_pI2CBuses[nBus];
      if(pbus->m_hDevice==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {
         if(!pbus->m_bTransactionInProgress)
         {
            I2CM_SetLastError(I2CM_ERR_NOTRANSACTION,NULL);
         } else
         {
            U2C_RESULT lrc=U2C_GetAck(pbus->m_hDevice);
            if(lrc==U2C_SUCCESS)
            {
               *pbAck=TRUE;
               I2CM_SetLastError(I2CM_SUCCESS,NULL);
            } else
            {
               if(lrc==U2C_NO_ACK)
               {
                  *pbAck=FALSE;
                  I2CM_SetLastError(I2CM_SUCCESS,NULL);
               } else
               {
                  I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
               }
            }
         }
      }
   }
   return I2CM_GetLastError();
}

#ifdef __cplusplus
}
#endif
