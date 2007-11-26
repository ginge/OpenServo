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

 * Synopsis     :
 *
 * Basic I2CManager_ExternalAddInterfaces function.
 */
#ifdef _WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "i2c_manager.h"

I2CMPROC Diolan_I2CM_GetProcAddress(int nfunc);

/*******************************************************************************
 *
 * int I2CManager_ExternalAddInterfaces();
 */
int I2CManager_ExternalAddInterfaces()
{
   int rc=0;

/*
 * If the Diolan I2C Manager DLL/dynamic library is present on the system, then attempt to load
 * it and add it's interface
 */
#if 1
#ifdef _WINDOWS
   HMODULE hDLL=LoadLibraryEx("i2cm_diolan.dll",NULL,LOAD_LIBRARY_AS_DATAFILE);
   if(hDLL!=NULL)
   {
      FreeLibrary(hDLL);
      hDLL=LoadLibraryEx("i2cm_diolan.dll",NULL,0);
      if(hDLL==NULL)
      {
         rc=-1; /* TODO */
      } else
      {
         I2CMPROCGETFNADDR pProc=(I2CMPROCGETFNADDR)GetProcAddress(hDLL,"Diolan_I2CM_GetProcAddress");
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
   void *hDLL=dlopen("./i2cm_diolan.so",RTLD_LAZY);
   if(hDLL==NULL)
   {
      hDLL=dlopen("/lib/i2cm_diolan.so",RTLD_LAZY);
   }
   if(hDLL==NULL)
   {
      rc=-1; /* TODO */
   } else
   {
      I2CMPROCGETFNADDR pProc=(I2CMPROCGETFNADDR)dlsym(hDLL,"Diolan_I2CM_GetProcAddress");
      if(pProc==NULL)
      {
         rc=-1; /* TODO */
      } else
      {
         rc=I2CManager_AddInterface(pProc);
      }
   }
#endif
#else
   rc=I2CManager_AddInterface(Diolan_I2CM_GetProcAddress);
#endif
   return rc;
}
