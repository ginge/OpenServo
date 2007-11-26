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
 * Provides the assignment functionality for the I2CM_GetProcAddress function(s).
 */
{
   switch (nfunc)
   {

      case I2CM_FID_INIT:
         p=(I2CMPROC)I2CM_Init;
         break;

      case I2CM_FID_GETLASTERROR:
         p=(I2CMPROC)I2CM_GetLastError;
         break;

      case I2CM_FID_GETLASTERRORTEXT:
         p=(I2CMPROC)I2CM_GetLastErrorText;
         break;

      case I2CM_FID_SETLASTERROR:
         p=(I2CMPROC)I2CM_SetLastError;
         break;

      case I2CM_FID_ISINITIALISED:
         p=(I2CMPROC)I2CM_IsInitialised;
         break;

      case I2CM_FID_SHUTDOWN:
         p=(I2CMPROC)I2CM_Shutdown;
         break;

      case I2CM_FID_GETBUSCOUNT:
         p=(I2CMPROC)I2CM_GetBusCount;
         break;

      case I2CM_FID_GETBUSHARDWARENAME:
         p=(I2CMPROC)I2CM_GetBusHardwareName;
         break;

      case I2CM_FID_GETBUSID:
         p=(I2CMPROC)I2CM_GetBusID;
         break;

      case I2CM_FID_GETBUSNAME:
         p=(I2CMPROC)I2CM_GetBusName;
         break;

      case I2CM_FID_RESCANFORDEVICES:
         p=(I2CMPROC)I2CM_RescanForDevices;
         break;

      case I2CM_FID_GETBUSDEVICECOUNT:
         p=(I2CMPROC)I2CM_GetBusDeviceCount;
         break;

      case I2CM_FID_GETDEVICECOUNT:
         p=(I2CMPROC)I2CM_GetDeviceCount;
         break;

      case I2CM_FID_GETDEVICEADDRESS:
         p=(I2CMPROC)I2CM_GetDeviceAddress;
         break;

      case I2CM_FID_READ:
         p=(I2CMPROC)I2CM_Read;
         break;

      case I2CM_FID_WRITE:
         p=(I2CMPROC)I2CM_Write;
         break;

      case I2CM_FID_STARTTRANSACTION:
         p=(I2CMPROC)I2CM_StartTransaction;
         break;

      case I2CM_FID_RESTARTTRANSACTION:
         p=(I2CMPROC)I2CM_RestartTransaction;
         break;

      case I2CM_FID_STOPTRANSACTION:
         p=(I2CMPROC)I2CM_StopTransaction;
         break;

      case I2CM_FID_GETBYTE:
         p=(I2CMPROC)I2CM_GetByte;
         break;

      case I2CM_FID_PUTACK:
         p=(I2CMPROC)I2CM_PutAck;
         break;

      case I2CM_FID_PUTBYTE:
         p=(I2CMPROC)I2CM_PutByte;
         break;

      case I2CM_FID_GETACK:
         p=(I2CMPROC)I2CM_GetAck;
         break;

      default:
         p=NULL;
         break;
   }
}
