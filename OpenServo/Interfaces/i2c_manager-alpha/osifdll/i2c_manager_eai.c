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
 * Basic I2CManager_ExternalAddInterfaces function for use with DLL/shared libraries.
 */
#ifdef _WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "../i2cm/i2c_manager.h"

/*******************************************************************************
 *
 * An array of structures naming known DLLs/shared libraries and the information
 * required to retrieve the interface retrieving function.
 */
static const struct
{
   const char *m_pLibraryName;
#ifndef _WINDOWS
   const char *m_pAltLibraryName;
#endif
   const char *m_pI2CM_GetProcAddressFuncName;
} LIBRARIES[]=
{
#ifdef _WINDOWS
   { "i2cm_diolan.dll",     "Diolan_I2CM_GetProcAddress"  },
   { "i2cm_osif.dll",       "OSIF_I2CM_GetProcAddress"    },
#else
   { "./i2cm_diolan.so",    "/lib/i2cm_diolan.so",    "Diolan_I2CM_GetProcAddress"  },
   { "./i2cm_osif.so",      "/lib/i2cm_osif.so",      "OSIF_I2CM_GetProcAddress"    },
#endif
};

/*******************************************************************************
 *
 * int I2CManager_ExternalAddInterfaces()
 *
 * Initialise each known layer and call I2CManager_AddInterface to add its
 * implementation to the I2C Manager.
 */
int I2CManager_ExternalAddInterfaces(BOOL bGetCount)
{
   int rc=0;
   int i;

/*
 * Attempt try to determine if known potential I2C Manager DLL/dynamic libraries
 * are present, then, if bGetCount is TRUE attempt to load them and add their
 * interface to the I2C Manager.
 *
 * NOTE: There may be a flaw in the logic here: A library that failed to load on the
 *       first call could, for some undefined reason, load on the second call. This
 *       would add an additional library that was not counted.
 *
 *       TODO: Fix the above issue.
 */
   for(i=0; i<sizeof(LIBRARIES)/sizeof(LIBRARIES[0]); i++)
   {
#ifdef _WINDOWS
      HMODULE hDLL=LoadLibraryEx(LIBRARIES[i].m_pLibraryName,NULL,LOAD_LIBRARY_AS_DATAFILE);
      if(hDLL!=NULL)
      {
         FreeLibrary(hDLL);
         hDLL=LoadLibraryEx(LIBRARIES[i].m_pLibraryName,NULL,0);
         if(hDLL==NULL)
         {
            rc=-1; /* TODO */
         } else
         {
            I2CMPROCGETFNADDR pProc=(I2CMPROCGETFNADDR)GetProcAddress(hDLL,
               LIBRARIES[i].m_pI2CM_GetProcAddressFuncName);
            if(pProc==NULL)
            {
               rc=-1; /* TODO */
            } else
            {
               rc=I2CManager_AddInterface(pProc);
            }
         }
      }
#else
      void *hDLL=dlopen(LIBRARIES[i].m_pLibraryName,RTLD_LAZY);
      if(hDLL==NULL)
      {
         hDLL=dlopen(LIBRARIES[i].m_pAltLibraryName,RTLD_LAZY);
      }
      if(hDLL==NULL)
      {
         rc=-1; /* TODO */
      } else
      {
         I2CMPROCGETFNADDR pProc=(I2CMPROCGETFNADDR)dlsym(hDLL,
            LIBRARIES[i].m_pI2CM_GetProcAddressFuncName);
         if(pProc==NULL)
         {
            rc=-1; /* TODO */
         } else
         {
            rc=I2CManager_AddInterface(pProc);
         }
      }
#endif
   }
   return rc;
}
