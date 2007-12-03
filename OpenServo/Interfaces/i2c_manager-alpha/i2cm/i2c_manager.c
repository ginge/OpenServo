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
 * Implements the I2C Manager's published interface.
 *
 * This is a preliminary version: WIP, with probably a great deal of room for improvement.
 *
 * ### Introduction ######################################################################
 *
 * The I2C Manager is a simple general purpose interface to I2C for the higher level
 * programming languages (C, etc.). Its purpose is to isolate application code from
 * the underlying hardware implementation.
 *
 * NOTE: This initial implementation is derived from the need to generically drive
 *       OpenServos and some other simple I2C devices. It may not, initially, suit
 *       all requirements for general access to I2C (or it may!).
 *
 *       Either a single hardware implementation is used, in which case the functions
 *       in i2c_manager.c are not needed, or multiple hardware implementations are
 *       used, in which case the functions in i2c_manager.c are used. This is described
 *       later.
 *
 * ### Overview ##########################################################################
 *
 * The I2C Manager provides a set of standardized Application Interface (API) functions,
 * which are used by both the I2C Manager itself and the underlying hardware specific
 * I2C implementations (or I2C layers).
 *
 * It is intended that two or more I2C layers are accessed through the I2C Manager.
 * But, importantly, when a minimal implementation using only a single I2C layer is
 * required, the same API is available for that I2C layer, but through the code that
 * implements that. This will be of most benefit on small platforms, such as a
 * microcontroller based system. An overview of how this duality is achieved is
 * described later; examination of the source code is will also usefully show the
 * finer details.
 *
 * Use of the I2C Manager makes it easier to write portable application code across
 * different platforms, be they desktop PC running Windows or Linux or a small embedded
 * microcontroller in a free roaming robot.
 *
 * Written in C (not C++) as it might, for example, be used from a C only microcontroller
 * platform (for example: "Dynamic C" on a "RabbitCore"). It also makes it easier to call
 * from other languages and removes potential C++ overheads on microcontroller platforms.
 *
 * An example: You have just created a most awesome biped robot, 16 "OpenServos" a
 * bunch of otehr I2C devices and developed modelling code on your desktop PC to make
 * it all work. The PC is connected to the robot using a USB-I2C interface and an
 * umbilical. When it is time for the robot to "walk on its own", you want to transfer
 * the code to a microcontroller or similar- you don't want to redo the modelling code,
 * etc., to use a different programming interface to I2C, now do you?
 *
 * NOTE: A C++ wrapper class can always be written.
 *
 * The generically named functions are prepended with "I2CM_" ("i2c_" functions already
 * exist out there- for example "i2c_init" with Dynamic C on the RabbitCore).
 *
 * ### Implementation notes ##############################################################
 *
 * Remember that both the "I2C Manager" and the "I2C implementation layer(s)" use the
 * same interface (with a few extra functions for the manager, described elsewhere).
 * Therefore the same function names might be used. But
 *
 *    When the I2C Manager is used and there are one or more I2CC implementation layers,
 *    then only the I2C Manager's usage of the standard function names can be external.
 *
 *    When only a single I2C implementation layer is used, it's functions can then be
 *    exposed as the standard names externally.
 *
 *    There are several methods to achieve this: The method used here is to #define the
 *    interface names, if needed, and declare those that should not be exposed as being
 *    "static" examine this header file together with i2c_manager.c and the "Diolan"
 *    interface for an example of how this is done.
 *
 * Example: On a desktop PC, one ore more of the following I2C hardware implementations
 *          might be available through individual I2C layers:
 *
 *             Diolan, OSIF and FD232R
 *
 *         On a Gumstix and a RabbitCore, the I2C implementations are localised to those
 *         platforms.
 *
 *         If all three PC implementations are to be used at the same time, then "the
 *         application code" can use the I2C Manager to access the I2C bus(es):
 *
 *             if(I2CM_Init(...
 *             if(I2CM_Read(bus,device,...
 *
 *         On the Gumstix and RabbitCore implementations a single I2C layer would (likely)
 *         be available, if correctly implemented, using the same interface:
 *
 *             if(I2CM_Init(...
 *             if(I2CM_Read(bus,device,...
 *
 *         Thus, hopefully, identical code that uses the interface can be developed
 *         on the PC, and used on the embedded system,.
 *
 * A standard interface is provided by each hardware implementation, for example, the
 * generic interface function "I2CM_Read" (to read bytes from the of an bus) for three
 * different sets of hardware on the PC might be defined (if they need to be exposed
 * externally, otherwise, they will just be declared as "static") as
 *
 *    int_t Diolan_I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, ... , ...)
 *    int_t OSIF_I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, ... , ...)
 *    int_t FD232R_I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, ... , ...)
 *
 * NOTE: The names do not need to be adorned with "Diolan_", etc., if the functions are
 *       not going to be accessed externally (they can be declared static, or hidden
 *       through the use of a dynamic library).
 *
 * The generic names are defined in i2c_manager.h (e.g. I2CM_Read), this include file can
 * then be included by the module(s) implementing an I2C layer for specific adapter hardware
 * adapter. If this layer needs, for some reason, to expose the functions, when in the
 * I2C manager environment, then #define can be used to give the functions layer specific
 * names that can be exposed externally when using the I2C manager or retain the generic
 * names when used as a single layer without the I2C manager. For example, options include:
 *
 *      #define I2CM_Read Diolan_I2CM_Read
 *      #define Diolan_I2CM_Read I2CM_Read
 *
 * NOTE: Maybe it is better not to expose a layer's functions, which would remove the temptation
 *       to call them.
 *
 * This need only be done if there is a need for the I2C layer implementation to expose the
 * functions to the outside world. This enables the function interfaces in the I2C layer to
 * be checked at compile time, when compiled in the correct manner.
 *
 * If the I2C layer implementation needs the functions to be statically declared, then it
 * should #define I2CM_CONFIG_MAKESTATIC before including i2c_manager.h. For example:
 *
 *    #define I2CM_CONFIG_MAKESTATIC 1
 *
 * NOTE : This will result in the macro I2CMSTATIC being defined as "static", otherwise it
 *        is defined as an empty string. 
 *
 * ### Including the I2C Manager in your application #####################################
 *
 * Compiling C and C++ application code that uses the I2C Manager is simply a matter of
 * including the "i2c_manager.h" header file. If you have added the location of the
 * I2C Manager files to the include path, then
 *
 *    #include <i2c_manager.h>
 *
 * otherwise the path needs to be specified manually
 *
 *    #include "/projects/dev/libraries/i2c_manager/i2c_manager.h"
 *
 * The application needs to be linked with the version of the I2C Manager library
 * appropriate to your application and the platform on which you are building it
 * (for example, static, multi-threaded, single-threaded, etc.).
 *
 * ### Including the I2C Manager in an I2C layer build ###################################
 *
 *    TODO
 *
 * ### I2C Manager API documentation #####################################################
 *
 * Each API function has documentation in the comments that precede its implementation,
 * found below.
 *
 * ### Other information #################################################################
 *
 * See i2c_manager.h and actual implementations for other information.
 *
 * NOTE: The I2C buses seen by the application when it is using the I2C Manager are numbered
 *       slightly differently than for the individual hardware I2C layer implementations.
 *       For example, consider two hardware adapters (the same type or different), if ADAPTER1
 *       has two I2C buses and ADAPTER2 has 1, then:
 *
 *          I2C Manager bus #   ADAPTER1 bus #      ADAPTER2 bus #
 *          0                   0
 *          1                   1
 *          2                                       0
 *
 *       Of course, if the adapters were seen the in the a different order, then
 *    
 *          I2C Manager bus #   ADAPTER1 bus #      ADAPTER2 bus #
 *          0                                       0
 *          1                   0
 *          2                   1
 *
 *       Even if each of the two adapters has only the one bus each, software changes of
 *       other inadvertent actions may cause the adapters to be switched. Or, for example,
 *       the original first adapter may simply have been removed from the system.
 *
 *       To help alleviate the problem functions such as I2CM_GetBusName are provided.
 *
 * ### Platforms #########################################################################
 *
 * Tested platforms: Microsoft Visual C++ 6.0 (Windows 2000)
 *
 * Limited testing: gcc version 4.1.2 (Ubuntu 4.1.2-0ubuntu4)
 *
 */
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "i2c_manager.h"

/*******************************************************************************
 *
 * Locally defined structures
 */
#pragma pack(push)
#pragma pack(1) /* Preferred packing: but beware segment issues on some processors */

/*
 * typedef struct I2CMLAYER { ... } I2CMLAYER
 *
 * A structure that contains information on each I2C layer (normally hardware/adapter)
 * implementation.
 *
 * NOTE: Remember, there might be more than one I2C bus provided by an I2C layer implementation.
 *       This may be because there is more than one I2C bus on a physical adapter, or
 *       there is more than one adapter supported by a single layer. As this level, little
 *       attempt is made to distinguish between physical adapters on a layer. Instead,
 *       each bus is assigned a number: the software must inquire about a particular bus
 *       if it really needs to know which layer/adapter it is connected to.
 */
typedef struct I2CMLAYER
{

/*
 * Function pointers for the I2C layer
 */
   union
   {
      struct
      {
         I2CM_INITFUNC m_pInitFunc;
         I2CM_GETLASTERRORFUNC m_pGetLastErrorFunc;
         I2CM_GETLASTERRORTEXTFUNC m_pGetLastErrorTextFunc;
         I2CM_SETLASTERRORFUNC m_pSetLastErrorFunc;
         I2CM_ISINITIALISEDFUNC m_pIsInitialisedFunc;
         I2CM_SHUTDOWNFUNC m_pShutdownFunc;
         I2CM_GETBUSCOUNTFUNC m_pGetBusCountFunc;
         I2CM_GETBUSHARDWARENAMEFUNC m_pGetBusHardwareNameFunc;
         I2CM_GETBUSIDFUNC m_pGetBusIDFunc;
         I2CM_GETBUSNAMEFUNC m_pGetBusNameFunc;
         I2CM_RESCANFORDEVICESFUNC m_pRescanForDevicesFunc;
         I2CM_GETBUSDEVICECOUNTFUNC m_pGetBusDeviceCountFunc;
         I2CM_GETDEVICECOUNTFUNC m_pGetDeviceCountFunc;
         I2CM_GETDEVICEADDRESSFUNC m_pGetDeviceAddressFunc;
         I2CM_READFUNC m_pReadFunc;
         I2CM_WRITEFUNC m_pWriteFunc;
         I2CM_STARTTRANSACTIONFUNC m_pStartTransaction;
         I2CM_RESTARTTRANSACTIONFUNC m_pRestartTransaction;
         I2CM_STOPTRANSACTIONFUNC m_pStopTransaction;
         I2CM_GETBYTEFUNC m_pGetByte;
         I2CM_PUTACKFUNC m_pPutAck;
         I2CM_PUTBYTEFUNC m_pPutByte;
         I2CM_GETACKFUNC m_pGetAck;
      } m_pfx;
      I2CMPROC m_a[23];
   } m_Funcs;
} I2CMLAYER;

/*
 * typedef struct I2CBUS { ... } I2CBUS
 *
 * A structure that contains information about an I2C bus, and how to access it
 * (which bus of which layer).
 *
 * Members
 *
 * int_t m_nLayer
 *        o Indexes the layers.
 *
 * int_t m_nBus
 *        o Defines the bus on the layer identified by m_nLayer.
 *
 * int_t m_nDevices
 *        o Gets the number of devices associated with the bus (at time that a scan for
 *          devices is performed).
 *
 * int_devaddr_t *m_pAddr
 *        o Point to an array of m_nDevices that contains the addresses of the known devices
 *          on the bus.
 */
typedef struct I2CBUS
{
   int_t m_nLayer;
   int_t m_nBus;
   int_t m_nDevices;
   int_devaddr_t *m_pAddr;
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
 *          been ended). Set to 0 when initialisation has been successfully
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
 *           This string is set to an empty string when the I2CM_SetLastError
 *           function is called with NULL as the second argument.
 */
static int_t l_nLastError;
static char l_sLastErrorAdditional[32];

/*
 * static int_t l_nLayers
 *         o The number of I2C layer implementations.
 *
 *           NOTE: Layers, and therefore, adapters are numbered sequentially starting from 0.
 *                 Adapters that are added to the system after initialisation are not included
 *                 in a rescan/reinitialisation. Changes in adapter/bus numbering are outside
 *                 of the control of this code- i.e. an adapter bus that was previously #1,
 *                 might be bus #2 after additional hardware comes on-line and a reinitialisation
 *                 is performed.
 *
 * static I2CMLAYER *l_pLayers
 *         o Points to an array of l_nI2CAdapters I2LAYER structures holding a descriptor
 *           for each of the adapters that has been registered. Initialised to NULL.
 */
static int_t l_nLayers=0;
static I2CMLAYER *l_pLayers=NULL;

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
static const char *l_pVersion="Version 1.0 alpha 2007 11 24 I2C Manager";
static const char *l_pCopyright="Copyright (c) 2007 Kevin Black";
#endif
static const char *l_pVersionShort="V 1.0 A 20071124 I2CM";
static const char *l_pCopyrightShort="(c) 2007";

/*******************************************************************************
 *
 * I2CMPROC I2CM_GetProcAddress(int_t nfunc)
 *
 * The I2CM_GetProcAddress function returns the function pointer (address of)
 * the I2C Manager/layer function indicated by the nfunc argument. For example,
 * if nfunc is I2CM_FID_READ, then the address of the relevant I2CM_Read function
 * is returned.
 *
 * NOTE: The returned pointer is anonymous: it will later need to be cast to the
 *       correct function type. This allows different functions to be returned.
 *
 *       An application does not really need to call this function; the I2C Manager
 *       uses this function to identify the API functions within each I2C layer.
 *
 *       The nfunc argument is an integer, rather than a string like "ICM_Read"
 *       to save space in small memory environments such as that which might be
 *       found in a microcontroller.
 *
 * Parameters
 *
 * int_t nfunc
 *         o Defines the function for which the address is to be returned. See the
 *           definition of the I2CM_FID_* macros in "i2c_manager.h" for the full
 *           list of the functions that need to be looked up. There is a naming
 *           correspondence between the I2CM_FID_ macro and the function that it
 *           describes. For example I2CM_FID_READ is I2CM_Read, I2CM_FID_INIT is
 *           I2CM_Init, etc.
 *
 * Return
 *
 * I2CMPROC o The function returns the address of the function requested. If the
 *            request is invalid, then NULL is returned. Extended error information
 *            may be available by calling the I2CM_GetLastError or I2CM_GetLastErrorText.
 */
I2CMPROC I2CM_GetProcAddress(int_t nfunc)
{
   I2CMPROC p=NULL;
#include "i2c_manager_procaddrmap.h"
   return p;
}

/*******************************************************************************
 *
 * I2CM_GetLastError, I2CM_GetLastErrorText and I2CM_SetLastError are
 * implemented first, this allows them to be functions callable from outside
 * of this module, but their use inside to be macroed, to save on space and
 * speed- still worrying about the small memory space microcontroller.
 */

/*******************************************************************************
 *
 * int_t I2CM_GetLastError()
 *
 * Synopsis
 *
 * The I2CM_GetLastError function returns the last I2C Manager error code that
 * was set. In an I2C Manager run environment, the last error set is returned,
 * which may have come from the I2C Manager itself or one of the I2C layers.
 * In a single I2C layer environment, the error code is from that layer.
 *
 * NOTE: If the last operation performed was successful, then this function
 *       returns I2CM_SUCCESS (0).
 *
 *       In an I2C Manager controlled environment, some layers may have errors
 *       set, some may not, the I2C Manager itself may, or may not, have an error
 *       set. The value returned is the last error (or success) that occurred.
 *
 * Return
 *
 * int_t   o The function returns the current error value or I2CM_SUCCESS.
 */
int_t I2CM_GetLastError()
{
   return l_nLastError;
}
#define I2CM_GetLastError() l_nLastError

/*******************************************************************************
 *
 * const char *I2CM_GetLastErrorText()
 *
 * Synopsis
 *
 * The I2CM_GetLastErrorText function returns a pointer to any text that may be
 * associated with the last error.
 *
 * NOTE: The returned pointer may be NULL or it may point to an empty string.
 *
 *       The absence or presence of an error text does not indicate whether an
 *       error has occurred, this is determined by calling the I2CM_GetLastError()
 *       function.
 *
 *       In an I2C Manager run environment, some layers may have errors set, some
 *       may not, the I2C Manager itself may, or may not, have an error set. The
 *       value returned is the last error (or success) that occurred.
 *
 *       The buffer pointed is not guaranteed to be the same from call to call.
 *
 * Return
 *
 * const char *
 *         o The function returns a pointer to a constant string buffer.
 */
const char *I2CM_GetLastErrorText()
{
   return l_sLastErrorAdditional;
}
#define I2CM_GetLastErrorText() l_sLastErrorAdditional

/*******************************************************************************
 *
 * int_t I2CM_SetLastError(int_t nerror, const char *ptext)
 *
 * The I2CM_SetLastError function allows the caller to set the last error value
 * and text.
 *
 * NOTE: This would typically be used to clear the error flag, which is achieved
 *       with the following call:
 *
 *          I2CM_SetLastError(I2CM_SUCCESS,NULL);
 *
 *       In an I2C Manager run environment, it is the status of the I2C Manager
 *       itself which is changed; underlying I2C layers are unaffected (their own
 *       functions would need to be called to effect such a change). In a single
 *       I2C layer environment, it is the I2C layer that is affected by the call.
 *
 * Parameters
 *
 * int_t nerror
 *         o The error code to be set or I2CM_SUCCESS (0).
 *
 * const char *ptext
 *         o Point to a null terminated string to be copied to a static or semi-static
 *           buffer within the I2C Managers (or layer in a single layer implementation).
 *           Each I2C layer implementation contains its own buffer.
 *
 * TODO: Nope?
 *
 * NOTE: If the string is too long for the static buffer, it will be truncated. A static
 *       buffer is used, so that repeated reallocations do not occur in small memory
 *       environments.
 *
 *       If nerror is I2CM_SUCCESS (0), then ptext parameter is ignored: an empty string
 *       is stored. If ptext is NULL, an empty string is stored.
 *
 * Return
 *
 * int_t   o The function reurns the error code just set.
 */
int_t I2CM_SetLastError(int_t nerror, const char *pat)
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
#define I2CM_SetLastError(NERROR,PAT) (PAT==NULL?l_sLastErrorAdditional[0]='\0':(strncpy(l_sLastErrorAdditional,PAT,sizeof(l_sLastErrorAdditional)-1),l_sLastErrorAdditional[sizeof(l_sLastErrorAdditional)-1]='\0'),l_nLastError=NERROR)

/*******************************************************************************
 *
 * static __inline I2CBUS *I2CManager_GetBus(int_t nBus)
 *
 * Internal function. Returns a pointer the I2CBUS structure defining a specific
 * bus. If the software has not been initialised or the specified bus does not
 * exist, then an error is set and NULL is returned.
 */
static __inline I2CBUS *I2CManager_GetBus(int_t nBus)
{
   I2CBUS *pbus=NULL;
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
          pbus=&l_pI2CBuses[nBus];
      }
   }
   return pbus;
}

/*******************************************************************************
 *
 * static __inline I2CMLAYER *I2CManager_GetLayer(int_t nBus)
 *
 * Internal function. Returns a pointer the I2CMLAYER structure defining a
 * specific bus implementation layer. If the software has not been initialised
 * or the specified bus does not exist, then an error is set and NULL is
 * returned.
 */
static __inline I2CMLAYER *I2CManager_GetLayer(int_t nBus)
{
   I2CMLAYER *plyr=NULL;
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      plyr=&l_pLayers[pbus->m_nLayer];
   }
   return plyr;
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusLastError(int_t nBus)
 *
 * Synopsis
 *
 * The I2CM_GetBusLastError function returns the last I2C Manager error code that
 * was set for a particular bus.
 *
 * NOTE: If the last operation performed, for the specified bus, was successful,
 *       then this function returns I2CM_SUCCESS (0).
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the last error code should be returned.
 *
 * Return
 *
 * int_t   o The function returns the current error value or I2CM_SUCCESS, for the
 *           specified bus.
 */
I2CMSTATIC int_t I2CM_GetBusLastError(int_t nBus)
{
   int_t rc=-1;
   I2CMLAYER *plyr=I2CManager_GetLayer(nBus);
   if(plyr!=NULL)
   {
      rc=plyr->m_Funcs.m_pfx.m_pGetLastErrorFunc();
   }
   return rc;
}

/*******************************************************************************
 *
 * const char *I2CM_GetLastErrorText(int_t nBus)
 *
 * Synopsis
 *
 * The I2CM_GetBusLastErrorText function returns a pointer to any text that may
 * be associated with the last error, for a particular bus.
 *
 * NOTE: The returned pointer may be NULL or it may point to an empty string.
 *
 *       The absence or presence of an error text does not indicate whether an error
 *       has occurred, this is determined by calling the I2CM_GetBusLastError function.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the error text pointer should be returned.
 *
 * Return
 *
 * const char *
 *         o The function returns a pointer to a constant string buffer, for the
 *           specified bus.
 */
I2CMSTATIC const char *I2CM_GetBusLastErrorText(int_t nBus)
{
   const char *prs=NULL;
   int_t rc=-1;
   I2CMLAYER *plyr=I2CManager_GetLayer(nBus);
   if(plyr!=NULL)
   {
      prs=plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc();
   }
   return prs;
}

/*******************************************************************************
 *
 * int_t I2CM_SetBusLastError(int_t nBus, int_t nerror, const char *ptext)
 *
 * The I2CM_SetBusLastError function allows the caller to set the last error value
 * and text, for a particular bus.
 *
 * NOTE: This would typically be used to clear the error flag, which is achieved
 *       with the following call:
 *
 *          I2CM_SetBusLastError(nbus,I2CM_SUCCESS,NULL);
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the error code and string should be set.
 *
 * int_t nerror
 *         o The error code to be set or I2CM_SUCCESS (0).
 *
 * const char *ptext
 *         o Point to a null terminated string to be copied to a static or semi-static
 *           buffer within the I2C layer.
 *
 *           NOTE: If the string is too long for the static buffer, it will be truncated.
 *                 A static buffer is used, so that repeated reallocations do not occur
 *                 in small memory environments.
 *
 *                 If nerror is I2CM_SUCCESS (0), then ptext parameter is ignored: an
 *                 empty string is stored. If ptext is NULL, an empty string is stored.
 *
 * Return
 *
 * int_t   o 
 */
I2CMSTATIC int_t I2CM_SetBusLastError(int_t nBus, int_t nerror, const char *ptext)
{
   int_t rc=-1;
   I2CMLAYER *plyr=I2CManager_GetLayer(nBus);
   if(plyr!=NULL)
   {
      rc=plyr->m_Funcs.m_pfx.m_pSetLastErrorFunc(nerror,ptext);
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t I2CM_Init()
 *
 * Synopsis
 *
 * The I2CM_Init function initialises the I2C Manager software and associated
 * I2C layers. 
 *
 * The I2C Manager I2CM_init function initialises its own structures and then
 * calls the I2CManager_ExternalAddInterfaces function to identify any available
 * I2C layers (specific hardware implementations). Once it has done this, it calls
 * the I2CM_Init function of each layer in turn.
 *
 * If there the I2C Manager is not used, then the I2CM_Init function of the
 * I2C layer is being called directly.
 *
 * NOTE: If the I2C Manager/layer is already initialised, then it is left in the
 *       initialised state and an error (I2CM_ERR_ALREADYINITIALISED) is returned.
 *
 * Return
 *
 * int_t   o The function returns I2CM_SUCCESS if initialisation has been completed
 *           successfully, otherwise an error code is returned.
 */
int_t I2CM_Init()
{
   if(l_bInit==0)
   {
      I2CM_SetLastError(I2CM_ERR_ALREADYINITIALISED,NULL);
   } else
   {
      I2CM_SetLastError(-1,NULL);

/*
 * Call I2CManager_ExternalAddInterfaces (the externally provided function, which must
 * call I2CManager_AddInterface to add each hardware implementation).
 */
      l_nLayers=0; /* There are none now */
      if(l_pLayers!=NULL) /* Failsafe, of course if memory is corrupt may cause a crash */
      {
         free(l_pLayers);
      }
      l_pLayers=NULL;
      if(I2CManager_ExternalAddInterfaces()!=0 || l_nLayers<1 || l_pLayers==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_SOFTWARE,NULL);
      } else
      {

/*
 * Rescan all the layers for devices, and build the device index
 */
         l_bInit=0; /* Otherwise I2CM_RescanForDevices will fail */
         if(I2CM_RescanForDevices()!=I2CM_SUCCESS)
         {
            l_bInit=1;
         }
      }
      if(l_bInit)
      {
         l_bInit=0; /* Needed for a successful call to I2CM_Shutdown */
         I2CM_Shutdown();
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CManager_AddInterface()
 *
 * Synopsis
 *
 * The I2CManager_AddInterface function registers an I2C layer (hardware specific I2C
 * implementation) with the I2C Manager, and is normally called from the
 * I2CManager_ExternalAddInterfaces function, which is "provided externally".
 * The I2CManager_ExternalAddInterfaces function handles detection and loading
 * dynamic libraries/DLLs that may be required to support all the known I2C layers,
 * etc. An application should not call this function itself. This function is not
 * required when using a single I2C layer without the I2C Manager.
 *
 * NOTE: The I2CManager_ExternalAddInterfaces has to be careful about initialisation:
 *       loading dynamic libraries is okay, but it must be borne in mind, that if it
 *       initialises and/or scans the I2C bus, that the I2C Manager will not "know"
 *       and will still call the layer's initialisation functions later.
 *
 * Return
 *
 * int_t   o The function returns I2CM_SUCCESS (0) if the interface has been successfully
 *           processed into the I2C Manager's list of I2C layers. A non-zero return
 *           indicates that an error has occurred.
 *
 * NOTE: The I2CManager_ExternalAddInterfaces function should pass any error from this
 *       function back to its caller.
 */
int_t I2CManager_AddInterface(I2CMPROCGETFNADDR pgpaf)
{
   if(l_bInit==0)
   {
      I2CM_SetLastError(I2CM_ERR_ALREADYINITIALISED,NULL);
   } else
   {
      if(pgpaf==NULL)
      {
         I2CM_SetLastError(I2CM_ERR_BADARGS,NULL);
      } else
      {

/*
 * Allocate a new layer 
 *
 * NOTE: realloc may be bad (or unusable) on a microcontroller, however it may also have
 *       only one layer, this realloc would not be needed.
 */
         I2CMLAYER *pnew=(I2CMLAYER *)realloc(l_pLayers,(l_nLayers+1)*sizeof(I2CMLAYER));
         if(pnew==NULL)
         {
            I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
         } else
         {
            int_t i;
            l_pLayers=pnew;
            pnew=&l_pLayers[l_nLayers]; /* NOTE: l_nLayers is only incremented on success, thus it can be used as the index for now */
            for(i=0; i<sizeof(pnew->m_Funcs.m_a)/sizeof(pnew->m_Funcs.m_a[0]); i++)
            {
               pnew->m_Funcs.m_a[i]=NULL;
            }

/*
 * Initialise the layer
 */
            pnew->m_Funcs.m_pfx.m_pInitFunc=(I2CM_INITFUNC)pgpaf(I2CM_FID_INIT);
            pnew->m_Funcs.m_pfx.m_pIsInitialisedFunc=(I2CM_ISINITIALISEDFUNC)pgpaf(I2CM_FID_ISINITIALISED);
            if(pnew->m_Funcs.m_pfx.m_pInitFunc==NULL ||
               pnew->m_Funcs.m_pfx.m_pIsInitialisedFunc==NULL ||
               pnew->m_Funcs.m_pfx.m_pInitFunc()!=I2CM_SUCCESS ||
               pnew->m_Funcs.m_pfx.m_pIsInitialisedFunc()!=1)
            {
               I2CM_SetLastError(I2CM_ERR_BADARGS,NULL); // TODO: Better error code
            } else
            {

/*
 * Nasty... TODO: make better?
 */
               pnew->m_Funcs.m_pfx.m_pGetLastErrorFunc=(I2CM_GETLASTERRORFUNC)pgpaf(I2CM_FID_GETLASTERROR);
               pnew->m_Funcs.m_pfx.m_pGetLastErrorTextFunc=(I2CM_GETLASTERRORTEXTFUNC)pgpaf(I2CM_FID_GETLASTERRORTEXT);
               pnew->m_Funcs.m_pfx.m_pSetLastErrorFunc=(I2CM_SETLASTERRORFUNC)pgpaf(I2CM_FID_SETLASTERROR);
               pnew->m_Funcs.m_pfx.m_pShutdownFunc=(I2CM_SHUTDOWNFUNC)pgpaf(I2CM_FID_SHUTDOWN);
               pnew->m_Funcs.m_pfx.m_pGetBusCountFunc=(I2CM_GETBUSCOUNTFUNC)pgpaf(I2CM_FID_GETBUSCOUNT);
               pnew->m_Funcs.m_pfx.m_pGetBusHardwareNameFunc=(I2CM_GETBUSHARDWARENAMEFUNC)pgpaf(I2CM_FID_GETBUSHARDWARENAME);
               pnew->m_Funcs.m_pfx.m_pGetBusIDFunc=(I2CM_GETBUSIDFUNC)pgpaf(I2CM_FID_GETBUSID);
               pnew->m_Funcs.m_pfx.m_pGetBusNameFunc=(I2CM_GETBUSNAMEFUNC)pgpaf(I2CM_FID_GETBUSNAME);
               pnew->m_Funcs.m_pfx.m_pRescanForDevicesFunc=(I2CM_RESCANFORDEVICESFUNC)pgpaf(I2CM_FID_RESCANFORDEVICES);
               pnew->m_Funcs.m_pfx.m_pGetBusDeviceCountFunc=(I2CM_GETBUSDEVICECOUNTFUNC)pgpaf(I2CM_FID_GETBUSDEVICECOUNT);
               pnew->m_Funcs.m_pfx.m_pGetDeviceCountFunc=(I2CM_GETDEVICECOUNTFUNC)pgpaf(I2CM_FID_GETDEVICECOUNT);
               pnew->m_Funcs.m_pfx.m_pGetDeviceAddressFunc=(I2CM_GETDEVICEADDRESSFUNC)pgpaf(I2CM_FID_GETDEVICEADDRESS);
               pnew->m_Funcs.m_pfx.m_pReadFunc=(I2CM_READFUNC)pgpaf(I2CM_FID_READ);
               pnew->m_Funcs.m_pfx.m_pWriteFunc=(I2CM_WRITEFUNC)pgpaf(I2CM_FID_WRITE);
               pnew->m_Funcs.m_pfx.m_pStartTransaction=(I2CM_STARTTRANSACTIONFUNC)pgpaf(I2CM_FID_STARTTRANSACTION);
               pnew->m_Funcs.m_pfx.m_pRestartTransaction=(I2CM_RESTARTTRANSACTIONFUNC)pgpaf(I2CM_FID_RESTARTTRANSACTION);
               pnew->m_Funcs.m_pfx.m_pStopTransaction=(I2CM_STOPTRANSACTIONFUNC)pgpaf(I2CM_FID_STOPTRANSACTION);
               pnew->m_Funcs.m_pfx.m_pGetByte=(I2CM_GETBYTEFUNC)pgpaf(I2CM_FID_GETBYTE);
               pnew->m_Funcs.m_pfx.m_pPutAck=(I2CM_PUTACKFUNC)pgpaf(I2CM_FID_PUTACK);
               pnew->m_Funcs.m_pfx.m_pPutByte=(I2CM_PUTBYTEFUNC)pgpaf(I2CM_FID_PUTBYTE);
               pnew->m_Funcs.m_pfx.m_pGetAck=(I2CM_GETACKFUNC)pgpaf(I2CM_FID_GETACK);
               for(i=0; i>=0 && i<sizeof(pnew->m_Funcs.m_a)/sizeof(pnew->m_Funcs.m_a[0]); i++)
               {
                  if(pnew->m_Funcs.m_a[i]==NULL)
                  {
                     i=-2;
                  }
               }
               if(i<0)
               {
                  I2CM_SetLastError(I2CM_ERR_BADARGS,NULL); // TODO: Better error code
                  if(pnew->m_Funcs.m_pfx.m_pShutdownFunc!=NULL)
                  {
                     pnew->m_Funcs.m_pfx.m_pShutdownFunc();
                  }
                  for(i=0; i<sizeof(pnew->m_Funcs.m_a)/sizeof(pnew->m_Funcs.m_a[0]); i++)
                  {
                     pnew->m_Funcs.m_a[i]=NULL;
                  }
               } else
               {
                  I2CM_SetLastError(I2CM_SUCCESS,NULL);
                  l_nLayers++;
               }
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CManager_ExternalAddInterfaces()
 *
 * NOTE: This function does NOT actually appear in this code, this stub is
 *       for purpose of providing documentation...
 *
 * Synopsis
 *
 * The I2CManager_ExternalAddInterfaces function is supplied by the "implementation".
 * When the I2CM_Init function is called to initialise the I2C Manager, it calls this
 * function, which should call the I2CManager_AddInterface function for each I2C layer
 * (hardware specific implementation).
 *
 * NOTE: See the source code for an example.
 *
 * Return
 *
 * int_t   o The function should return the return status of the last unsuccessful
 *           call to the I2CManager_AddInterface function. If all calls to the
 *           I2CManager_AddInterface function return I2CM_SUCCESS, then the
 *           function should return I2CM_SUCCESS.
 */

/*******************************************************************************
 *
 * int_t I2CM_RescanForDevices()
 *
 * Synopsis
 *
 * The I2CM_RescanForDevices function scans all known I2C buses for devices,
 * these are held on an internal list.
 *
 * NOTE: The I2CM_RescanForDevices function is called by I2CM_Init, so it need
 *       only be recalled if devices have been added to the bus, or their addresses
 *       have changed.
 *
 * Return
 *
 * int_t   o If there were no errors, the function returns I2CM_SUCCESS (even
 *           if there were no devices). Otherwise an error code is returned.
 */
int_t I2CM_RescanForDevices()
{
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t i,j,k;
      bool_t bOK=TRUE;

/*
 * Reset...
 */
      if(l_pI2CBuses!=NULL) /* Failsafe again */
      {
         for(i=0; i<l_nI2CBuses; i++)
         {
            if(l_pI2CBuses[i].m_pAddr!=NULL)
            {
               free(l_pI2CBuses[i].m_pAddr);
               l_pI2CBuses[i].m_pAddr=NULL;
            }
         }
         free(l_pI2CBuses);
      }
      l_pI2CBuses=NULL;
      l_nI2CBuses=0;

/*
 * Rescan all the I2C layers
 */
      for(i=0; bOK && i<l_nLayers; i++)
      {
         if(l_pLayers[i].m_Funcs.m_pfx.m_pRescanForDevicesFunc()!=0)
         {
            bOK=FALSE;
         }
      }

/*
 * Get the total number of buses available
 */
      if(bOK)
      {
         int_t n=0;
         for(i=0; i<l_nLayers; i++)
         {
            n+=l_pLayers[i].m_Funcs.m_pfx.m_pGetBusCountFunc();
         }
         if(n<1) /* Treat no buses as an error */
         {
            I2CM_SetLastError(I2CM_ERR_HARDWARE,NULL);
         } else
         {

/*
 * Allocate and initialise the array of I2CBUS structures
 */
            l_pI2CBuses=(I2CBUS *)malloc(n*sizeof(I2CBUS));
            if(l_pI2CBuses==NULL)
            {
               I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
            } else
            {
               for(i=0; i<n; i++)
               {
                  l_pI2CBuses[i].m_nLayer=0xFF;
                  l_pI2CBuses[i].m_nBus=0xFF;
                  l_pI2CBuses[i].m_nDevices=0;
                  l_pI2CBuses[i].m_pAddr=NULL;
               }

/*
 * Iterate all layers and buses and build the index of current devices...
 */
               for(i=0; i<l_nLayers; i++)
               {
                  I2CMLAYER *pl=&l_pLayers[i];
                  j=pl->m_Funcs.m_pfx.m_pGetBusCountFunc();
                  if(j<0)
                  {
                     // TODO
                  } else
                  {
                     for(; bOK && j>0;)
                     {
                        I2CBUS *pbus=&l_pI2CBuses[l_nI2CBuses++];
                        pbus->m_nLayer=i;
                        j--;
                        pbus->m_nBus=j;
                        k=pl->m_Funcs.m_pfx.m_pGetBusDeviceCountFunc(j);
                        if(k<0)
                        {
                           bOK=FALSE;
                        } else
                        {
                           pbus->m_nDevices=k;
                           if(k>0)
                           {
                              pbus->m_pAddr=(int_devaddr_t *)malloc(k*sizeof(int_devaddr_t));
                              if(pbus->m_pAddr==NULL)
                              {
                                 I2CM_SetLastError(I2CM_ERR_OUTOFMEMORY,NULL);
                              } else
                              {
                                 for(; bOK && k>0;)
                                 {
                                    k--; // TODO:
                                    pbus->m_pAddr[k]=(int_devaddr_t)pl->m_Funcs.m_pfx.m_pGetDeviceAddressFunc(j,k);
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * Interface
 *
 * int_t I2CM_IsInitialised()
 *
 * Synopsis
 *
 * The I2CM_IsInitialised indicates whether the I2C Manager software and associated
 * I2C layers or the individual I2C layer is initialised.
 *
 * Return
 *
 * int_t   o The function returns 1 (TRUE) if the I2C Manager software and associated
 *           I2C layers or the individual I2C layer is initialised. Otherwise it returns
 *           0 (FALSE)- not initialised.
 */
int_t I2CM_IsInitialised()
{
   return l_bInit==0;
}

/*******************************************************************************
 *
 * int_t I2CM_Shutdown()
 *
 * Synopsis
 *
 * The I2CM_Shutdown shuts down the I2C Manager software and associated I2C layers
 * or the individual I2C layer.
 *
 * NOTE: Resources will be freed, handles closed, etc. After shutdown I2CM_IsInitialised
 *       will returns 0. I2CM_Init may be called to restart the I2C Manager.
 *
 * Return
 *
 * int_t   o The function returns I2CM_SUCCESS (0) if the I2C Manager software and associated
 *           I2C layers or the individual I2C layer are shutdown. Otherwise it returns
 *           an error code.
 */
int_t I2CM_Shutdown()
{
   if(l_bInit!=0)
   {
      I2CM_SetLastError(I2CM_ERR_NOTINITIALISED,NULL);
   } else
   {

/*
 * Close down the layers first
 */
      bool_t bOK=TRUE;
      int_t i;
      if(l_pLayers!=NULL)
      {
         for(i=0; i<l_nLayers; i++)
         {
            I2CMLAYER *plyr=&l_pLayers[i];
            if(plyr->m_Funcs.m_pfx.m_pShutdownFunc!=NULL)
            {
               if(plyr->m_Funcs.m_pfx.m_pShutdownFunc()!=0)
               {
                  bOK=FALSE;
               }
            }
         }
         free(l_pLayers);
         l_pLayers=NULL;
      }
      l_nLayers=0;

/*
 * Release resources
 */
      if(l_pI2CBuses!=NULL)
      {
         for(i=0; i<l_nI2CBuses; i++)
         {
            if(l_pI2CBuses[i].m_pAddr!=NULL)
            {
               free(l_pI2CBuses[i].m_pAddr);
               l_pI2CBuses[i].m_pAddr=NULL;
            }
         }
         free(l_pI2CBuses);
         l_pI2CBuses=NULL;
      }
      l_nI2CBuses=0;
      l_bInit=1;
      if(bOK)
      {
         I2CM_SetLastError(I2CM_SUCCESS,NULL);
      }
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusCount()
 *
 * Synopsis
 *
 * The I2CM_GetBusCount function returns the number of functional I2C buses known
 * to the I2C Manager or the individual I2C layer.
 *
 * NOTE: There may be more buses than I2C layers, as a single I2C layer implementation
 *       may support more than one bus.
 *
 * Return
 *
 * int_t   o The function returns the number of buses known, or -1 if the I2C Manager
 *           has not been initialised.
 */
int_t I2CM_GetBusCount()
{
   return l_bInit!=0 ? -1 : l_nI2CBuses;
}

/*******************************************************************************
 *
 * int_t I2CM_GetBusHardwareName(int_t nBus, char *pbuffer, int_t n)
 *
 * Synopsis
 *
 * The I2CM_GetBusHardwareName function copies the underlying "hardware" name of
 * the specified bus into a buffer.
 *
 * NOTE: There may be more buses than I2C layers, as a single I2C layer implementation
 *       may support more than one bus.
 *
 *       The "hardware" name of a bus is obtained by calling the I2CM_GetBusHardwareName
 *       function of the underlying hardware implementation- it is required that this
 *       function returns a string that uniquely identifies the layer amongst all layers.
 *
 *       The full identity of an I2C bus is comprised of both its name and "ID"- see the
 *       I2CM_GetBusID function, this information can be retrieved in a single call by
 *       calling the I2CM_GetBusName function.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the underlying hardware name is required.
 *
 * char *pbuffer
 *         o Points to the buffer into which the null terminated string is to be copied.
 *
 * int_t n   o Specifies the size of the buffer pointed to by pbuffer, including the space
 *           for the null terminator.
 *
 * Return
 *
 * int_t   o If the hardware name has been successfully copied to the buffer, then the
 *           function returns I2CM_SUCCESS. If an error occurs when retrieving the name,
 *           the I2C Manager is not initialised or the buffer provided is too small, then
 *           an error code is returned.
 */
int_t I2CM_GetBusHardwareName(int_t nBus, char *pbuffer, int_t n)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pGetBusHardwareNameFunc(pbus->m_nBus,pbuffer,n),plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_GetBusID(int_t nBus, unsigned long *pbusid)
 *
 * Synopsis
 *
 * The I2CM_GetBusID function stores the "ID" of the specified bus into a buffer.
 *
 * NOTE: The "ID" of a bus is obtained by calling the I2CM_GetBusID function of the
 *       underlying hardware implementation- it is required that this function returns
 *       an unsigned long that uniquely identifies that bus, but only for that layer.
 *
 *       The full identity of an I2C bus is comprised of both its name and "ID"- see
 *       the I2CM_GetBusHardwareName function, this information can be retrieved in a
 *       single call by calling the I2CM_GetBusName function.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the underlying hardware name is required.
 *
 * unisgned long *pbusid
 *         o Points to an unsigned long in which the bus ID is to be stored.
 *
 * Return
 *
 * int_t   o If the bus ID has been successfully retrieved and store, then the function
 *           returns I2CM_SUCCESS. If an error occurs when retrieving the ID or the
 *           I2C Manager is not initialised, then an error code is returned.
 */
int_t I2CM_GetBusID(int_t nBus, unsigned long *pbusid)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pGetBusIDFunc(pbus->m_nBus,pbusid),plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_GetBusName(int_t nBus, char *pbuffer, int_t n)
 *
 * Synopsis
 *
 * The I2CM_GetBusName function copies a string that uniquely represents the bus
 * amongst all buses known to the I2C Manager into a buffer.
 *
 * NOTE: There may be more buses than I2C layers, as a single I2C layer implementation
 *       may support more than one bus.
 *
 *       The bus name is usually constructed by calling I2CM_GetBusHardwareName
 *       function to get a name and the I2CM_GetBusID function to get an ID which
 *       is then formatted as a hexadecimal string. These are stored in the buffer
 *       separated by an underscore character. For example (assuming no errors,
 *       and that the I2C Manager has been initialised) the following code
 *
 *          char buffer[32];
 *          unsigned long id;
 *          I2CM_GetBusHardwareName(0,buffer,sizeof(buffer));
 *          fprintf(stdout,"Hardware name=\"%s\"\n",buffer);
 *          I2CM_GetBusID(0,&id);
 *          fprintf(stdout,"Bus ID=%08x\n",id);
 *          I2CM_GetBusName(0,buffer,sizeof(buffer));
 *          fprintf(stdout,"Bus name=\"%s\"\n",buffer);
 *
 *       might result in the following output
 *
 *          Hardware name="U2C12"
 *          Bus ID=00001a76
 *          Bus name="U2C12_0x00001a76"
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the name is required.
 *
 * char *pbuffer
 *         o Points to the buffer into which the null terminated string is to be copied.
 *
 * int_t n   o Specifies the size of the buffer pointed to by pbuffer, including the space
 *           for the null terminator.
 *
 * Return
 *
 * int_t   o If the name has been successfully constructed and copied to the buffer,
 *           then the function returns I2CM_SUCCESS. If an error occurs when retrieving
 *           the name, the I2C Manager is not initialised or the buffer provided is too
 *           small, then an error code is returned.
 */
int_t I2CM_GetBusName(int_t nBus, char *pbuffer, int_t n)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pGetBusNameFunc(pbus->m_nBus,pbuffer,n),plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
};

/*******************************************************************************
 *
 * int_t I2CM_GetBusDeviceCount(int_t nBus)
 *
 * Synopsis
 *
 * The I2CM_GetBusDeviceCount function returns the number of previously found
 * I2C devices on a specified bus.
 *
 * Parameters
 *
 * int_t nBus
 *
 * Specifies the bus number for which the device count is required.
 *
 * Return
 *
 * int_t   o The function returns the number of devices or -1 if the I2C Manager
 *           has not been initialised, an illegal bus is specified or an error has
 *           occurred.
 */
int_t I2CM_GetBusDeviceCount(int_t nBus)
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
 *
 * Synopsis
 *
 * The I2CM_GetDeviceCount function returns the number of previously found 
 * I2C devices across all buses.
 *
 * Return
 *
 * int_t   o The function returns the number of devices, or -1 if the I2C Manager
 *           has not been initialised or an error has occurred.
 */
int_t I2CM_GetDeviceCount()
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
 *
 * Synopsis
 *
 * The I2CM_GetDeviceAddress function returns the number of previously found I2C
 * devices on a specified bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the bus number for which the device address is required.
 *
 * int_t nDevice
 *         o Specifies the device number for which the device address is required.
 *
 * Return
 *
 * int_t   o The function returns the address of the device or -1 if the I2C Manager
 *           has not been initialised, an illegal bus or device is specified or an
 *           error has occurred.
 */
int_t I2CM_GetDeviceAddress(int_t nBus, int_t nDevice)
{
   int_t rc=(int)-1;
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      rc=pbus->m_pAddr[nDevice];
   }
   return rc;
}

/*******************************************************************************
 *
 * int_t I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, byte_t *pBuffer)
 *
 * Synopsis
 *
 * The I2CM_Read function reads 0*, 1 or more bytes from a device on an I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the device to be
 *           read is located.
 *
 * int_devaddr_t nSlaveAddress
 *         o Specifies the I2C address of the device to be read.
 *
 * int_t nMemAddressLen
 *         o Specifies the width in bytes of the memory addressing of the device
 *           to be read.
 *
 * int_memaddr_t nMemAddress
 *         o Specifies the address to be sent to the device to indicate where the
 *           bytes are to be read from.
 *
 * int_t nBytes
 *         o Specifies how many bytes are to be read.
 *
 * byte_t *pBuffer
 *         o Points to a buffer into which the bytes are to be read.
 *
 * Return
 *
 * int_t   o If the specified number of bytes have been successfully read from the
 *           device, then the function returns I2CM_SUCCESS. If the I2C Manager has
 *           not been initialised, an illegal bus is specified or an error has
 *           occurred, then an error code is returned.
 */
int_t I2CM_Read(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, byte_t *pBuffer)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pReadFunc(pbus->m_nBus,nSlaveAddress,nMemAddressLen,nMemAddress,nBytes,pBuffer),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_Write(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, const byte_t *pBuffer)
 *
 * Synopsis
 *
 * The I2CM_Write function writes 0*, 1 or more bytes to a device on an I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the device to be
 *           written is located.
 *
 * int_devaddr_t nSlaveAddress
 *         o Specifies the I2C address of the device to be written.
 *
 * int_t nMemAddressLen
 *         o Specifies the width in bytes of the memory addressing of the device
 *           to be written.
 *
 * int_memaddr_t nMemAddress
 *         o Specifies the address to be sent to the device to indicate where the
 *           bytes are to be written to.
 *
 * int_t nBytes
 *         o Specifies how many bytes are to be written.
 *
 * const byte_t *pBuffer
 *         o Points to a buffer containing the bytes to be written.
 *
 * Return
 *
 * int_t   o If the specified number of bytes have been successfully written to the
 *           device, then the function returns I2CM_SUCCESS. If the I2C Manager has
 *           not been initialised, an illegal bus is specified or an error has
 *           occurred, then an error code is returned.
 */
int_t I2CM_Write(int_t nBus, int_devaddr_t nSlaveAddress, int_t nMemAddressLen, int_memaddr_t nMemAddress, int_t nBytes, const byte_t *pBuffer)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pWriteFunc(pbus->m_nBus,nSlaveAddress,nMemAddressLen,nMemAddress,nBytes,pBuffer),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_StartTransaction(int_t nBus)
 *
 * This function attempts to set a "start" condition on the I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the start condition
 *           is to be set.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a start condition was successfully
 *           set on the bus, otherwise an error code is returned.
 */
int_t I2CM_StartTransaction(int_t nBus)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pStartTransaction(nBus),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc()); // TODO: THESE LAST ERROR STRINGS ARE WRONG?
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_RestartTransaction(int_t nBus)
 *
 * This function attempts to reset a "start" condition (restart) on the I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the start condition
 *           is to be set.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a start condition was successfully
 *           set on the bus, otherwise an error code is returned.
 */
int_t I2CM_RestartTransaction(int_t nBus)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pRestartTransaction(nBus),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc()); // TODO: THESE LAST ERROR STRINGS ARE WRONG?
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_StopTransaction(int_t nBus)
 *
 * This function attempts to set a "stop" condition on the I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the stop condition
 *           is to be set.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a stop condition was successfully
 *           set on the bus, otherwise an error code is returned.
 */
int_t I2CM_StopTransaction(int_t nBus)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pStopTransaction(nBus),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetByte(int_t nBus, byte_t *pnByte)
 *
 * Retrieves a single data byte from an I2C bus, without sending an acknowledgement.
 *
 * NOTE: The caller should send an acknowledgement, by calling the I2CM_PutAck
 *       function, after successfully retrieving a byte.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus from which the byte should
 *           be retrieved.
 *
 * byte_t *pnByte
 *         o Points to the location in which the retrieved byte should be stored.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a byte was successfully retrieved,
 *           otherwise an error code is returned.
 */
int_t I2CM_GetByte(int_t nBus, byte_t *pnByte)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pGetByte(nBus,pnByte),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_PutAck(int_t nBus, bool_t bAck)
 *
 * Sends an acknowledgement to an I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus on which the acknowledgment
 *           should be sent.
 *
 * bool_t bAck
 *         o Indicated whether a positive (TRUE) or negative acknowledgement (FALSE)
 *           should be sent. For example, when reading the a sequence of bytes,
 *           positive acknowledgements should be sent for all but the last byte. A
 *           negative acknowledgement should be sent for the last byte read, before
 *           setting the stop or restart condition.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if the acknowledgement successfully sent,
 *           otherwise an error code is returned.
 */
int_t I2CM_PutAck(int_t nBus, bool_t bAck)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pPutAck(nBus,bAck),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_PutByte(int_t nBus, byte_t nByte)
 *
 * Sends a single byte to an I2C bus, without retrieving an acknowledgement.
 *
 * NOTE: The caller should retrieve an acknowledgement, by calling the I2CM_GetAck
 *       function, after successfully writing a byte.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus to which the byte should
 *           be sent.
 *
 * byte_t nByte
 *         o The byte value to be sent.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a byte was successfully sent,
 *           otherwise an error code is returned.
 */
int_t I2CM_PutByte(int_t nBus, byte_t nByte)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pPutByte(nBus,nByte),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetAck(int_t nBus, bool_t *pbAck)
 *
 * Retrieves an acknowledgement from an I2C bus.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus from which the acknowledgment
 *           should be retrieved.
 *
 * bool_t *pbAck
 *         o Points to a location in which the acknowledgement, if successfully
 *           retrieved, should be stored: TRUE for a positive or FALSE for a negative
 *           acknowledgement.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if the acknowledgement successfully
 *           retrieved, otherwise an error code is returned.
 */
int_t I2CM_GetAck(int_t nBus, bool_t *pbAck)
{
   I2CBUS *pbus=I2CManager_GetBus(nBus);
   if(pbus!=NULL)
   {
      I2CMLAYER *plyr=&l_pLayers[pbus->m_nLayer];
      I2CM_SetLastError(plyr->m_Funcs.m_pfx.m_pGetAck(nBus,pbAck),
         plyr->m_Funcs.m_pfx.m_pGetLastErrorTextFunc());
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_GetBytePutAck(int_t nBus, byte_t *pnByte, bool_t bAck)
 *
 * Retrieves a single data byte from an I2C bus and, if successful, sends an
 * acknowledgement.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus from which the byte should
 *           be retrieved.
 *
 * byte_t *pnByte
 *         o Points to the location in which the retrieved byte should be stored.
 *
 * bool_t bAck
 *         o Indicated whether a positive (TRUE) or negative acknowledgement (FALSE)
 *           should be sent. For example, when reading the a sequence of bytes,
 *           positive acknowledgements should be sent for all but the last byte. A
 *           negative acknowledgement should be sent for the last byte read, before
 *           setting the stop or restart condition.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a data byte was successfully
 *           retrieved and an acknowledgement successfully sent, otherwise an
 *           error code is returned.
 */
int_t I2CM_GetBytePutAck(int_t nBus, byte_t *pnByte, bool_t bAck)
{
   if(I2CM_GetByte(nBus,pnByte)==I2CM_SUCCESS)
   {
      I2CM_PutAck(nBus,bAck);
   }
   return I2CM_GetLastError();
}

/*******************************************************************************
 *
 * int_t I2CM_PutByteGetAck(int_t nBus, byte_t nByte, bool_t *pbAck)
 *
 * Sends a single byte to an I2C bus and, if successful, retrieves an acknowledgement.
 *
 * Parameters
 *
 * int_t nBus
 *         o Specifies the I2C bus number of the bus to which the byte should
 *           be sent.
 *
 * byte_t nByte
 *         o The byte value to be sent.
 *
 * bool_t *pbAck
 *         o Points to a location in which the acknowledgement, if successfully
 *           retrieved, should be stored: TRUE for a positive or FALSE for a negative
 *           acknowledgement.
 *
 * Return
 *
 * int_t   o The function returns I2C_SUCCESS if a byte was successfully sent and
 *           an acknowledgement successfully retrieved, otherwise an error code is
 *           returned.
 */
int_t I2CM_PutByteGetAck(int_t nBus, byte_t nByte, bool_t *pbAck)
{
   if(I2CM_PutByte(nBus,nByte)==I2CM_SUCCESS)
   {
      I2CM_GetAck(nBus,pbAck);
   }
   return I2CM_GetLastError();
}
