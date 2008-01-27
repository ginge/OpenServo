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
 * Implements the interface between OpenServo devices and the I2C (I²C) Manager
 *
 * This is a preliminary version: WIP, with probably a great deal of room for improvement.
 *
 * ### Introduction ######################################################################
 *
 * Provides a software interface between OpenServo and I2C using the "I2C Manager".
 *
 * NOTE: The initial version of this software is being developed against an OpenServo 2.1
 *       board with "May 2007" firmware and V3 boards with "bank switched" firmware
 *       (OpenServo_V3-dev).
 *
 *       The firmware in general is currently being advanced to include the new facilities
 *       of the OpenServo V3 board (e.g. back EMG) as well as software advances (e.g. "bank
 *       switching").
 *
 *       It is intended that OpenServo interface software will allow both the old and new
 *       boards to be accommodated as well as different firmware versions.
 *
 * ### Overview ##########################################################################
 *
 * This OpenServo Interface (OSI) provides a set of standardized Application Interface
 * (API) functions for accessing OpenServo devices.
 *
 * Is written in C (not C++) as it might, for example, be used from a C only microcontroller
 * platform (for example: avr-grcc or "Dynamic C" on a "RabbitCore"). It also makes it easier
 * to call from other languages and removes potential C++ overheads on microcontroller platforms.
 *
 * An example: You have just created a most awesome biped robot, 16 "OpenServos" a
 * bunch of other I2C devices and developed modeling code on your desktop PC to make
 * it all work. The PC is connected to the robot using a USB-I2C interface and an
 * umbilcal. When is is time for the robot to "walk on its own", you want to transfer
 * the code to a mircocontroller or similar- you don't want to redo the modelling code,
 * etc., to use a different programming interface to I2C, now do you?
 *
 * NOTE: A C++ wrapper class can always be written.
 *
 * The interface uses the "I2C Manager" (included in the project delivery) to access
 * the I2C bus: insulating it from the hardware implementation of the I2C interface.
 *
 * As well as supporting different firmware versions, the code here is performing "over
 * the top" optimisation of the usage of the I2C bus: this adds code complexity which
 * you may consider unnecessary! The optimisation works by allowing the caller to "open",
 * "access" and the "close" a servo- it will then only stop a transaction when "close"
 * is called and only restart transaction if it is necessary. For example of the caller
 * "opens" a servo and then reads two registers that are adjacent; it will not start
 * a new transaction or addressing operation for the second register.
 *
 * If an existing I2C transfer cannot continue from the current address (due to
 * address skipping, wrong bank, etc.), then the current transaction must end and
 * a new one started. However, for the OpenServo, when reading and when the bank
 * selection does not need to be changed, we can continue to read and discard the
 * additional bytes if this is more efficient than stopping and re-starting the
 * I2C transaction. I.e., when reading, to go to a new address, the following
 * basic sequence on the I2C bus might occur:
 *
 *    start
 *    put byte (slave address<<1) (write) get ack
 *    put byte (first reg address) get ack
 *    stop
 *    start
 *    put byte (slave address<<1|1) (read) get ack
 *    get byte put ack [i]
 *    get byte put ack [ii]
 *    get byte put ack [iii]
 *    get byte no ack  [iv]
 *    stop
 *    start
 *    put byte (slave address<<1) (write) get ack
 *    put byte (second reg address) get ack
 *    stop
 *    start
 *    put byte (slave address<<1|1) (read) get ack
 *    get byte put ack [v]
 *    get byte no ack  [vi]
 *    stop
 *
 * Observing that reading from the OpenServo (currently) has no detrimental
 * effect, it can be seen that if the second reg address (in the example above)
 * is separated by one, two or three bytes from the last address after the
 * sequence read starting at the first reg address, then the second read set
 * up does not need to occur, instead one, two or three as required, bytes
 * are read and discarded. For example, if the two sequences are above were
 * separated by two bytes then, the sequence above would become
 *
 *    start
 *    put byte (slave address<<1) (write) get ack
 *    put byte (first reg address) get ack
 *    repeated-start
 *    put byte (slave address<<1|1) (read) get ack
 *    get byte put ack [i]
 *    get byte put ack [ii]
 *    get byte put ack [iii]
 *    get byte put ack  [iv]
 *    get byte put ack  [discarded]
 *    get byte put ack  [discarded]
 *    get byte put ack [v]
 *    get byte no ack  [vi]
 *    stop
 *
 * In this example, this eliminates two stop/start cycles (which may have other
 * benefits) and one byte transfer. This may seem trivial, but every byte or delay
 * saved may count towards reducing I2C bus bandwidth usage...
 *
 * But it is also "simple" to implement....
 *
 * NOTE: Stop/start pairs can be converted in to a repeated start. As well
 *       as saving a miniscule bus cycle, this may help prevent a second master
 *       gaining control whilst the first is addressing a particular servo...
 *
 * TODO: Test for/validity of and implement the following...
 *
 * A further optimisation that appears to be possible when reading the servo is
 * if a previously accessed read/write address has incremented to an address that
 * is just about to be read (including the skip bytes). When this is the case
 * it is not necessary to address the servo with a write operation to set the
 * address. Thus the following sequence, for such read operations the addressing
 * sequence would no longer be necessary. For example:
 *
 *    start
 *    put byte (slave address<<1) (write) get ack
 *    put byte (first reg address) get ack
 *    repeated-start
 *    put byte (slave address<<1|1) (read) get ack
 *    get byte put ack [i]
 *    get byte put ack [ii]
 *    stop
 *
 * would become
 *
 *    start
 *    put byte (slave address<<1|1) (read) get ack
 *    get byte put ack [i]
 *    get byte put ack [ii]
 *    stop
 *
 * ### API documentation #################################################################
 *
 * Each API function has documentation in the comments that precede its implementation,
 * found below.
 *
 * ### Other information #################################################################
 *
 * See ois.h and the code for other information.
 *
 * NOTE: typedefed types, such as "uint16_t" for basic types are used. These are defined
 *       in "i2c_manager.h" and are used for the reasons as stated there.
 *
 * ### Platforms #########################################################################
 *
 * Tested platforms: Microsoft Visual C++ 6.0 (Windows 2000)
 *
 * Limited testing: gcc version 4.1.2 (Ubuntu 4.1.2-0ubuntu4)
 */
#ifdef _MSC_VER
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "osi.h"

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
byte_t m_bIsLittleEndian=0; /* NOTE: This is set to the correct value in OSI_Init, */
                            /*       as it is not really useful until after that   */
                            /*       function has been called.                     */

/* TODO: Consider if the above decision is the correct one, using the #ifdef __LITTLE_ENDIAN__
 *       strategy can remove code on big endian machines. Does it really matter?
 */

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
 * Locally defined structures
 */
#pragma pack(push)
#pragma pack(1) /* Prefered packing */

/*******************************************************************************
 *
 * static and global data
 *
 * An assumption is made that an application wants global access, therefore,
 * so that handles, etc., do not need to be managed and passed around by the
 * application, many items are statically allocated or allocated to static
 * pointers, etc. This make implementation easier and improves eventual
 * portability to microcontroller platforms.
 *
 * static bool_t l_bInit
 *        o Initialised to 1 (TRUE), any non-zero values indicates that the OpenServo
 *          I2C software interface has not been initialised (or has subsequently been
 *          ended). Set to 0 when initialisation has been sucessfully performed. Any
 *          value other than 0 or 1 probably indicates memory corruption.
 */
static bool_t l_bInit=1;

/*
 * static int_t l_nLastError
 *        o Set to the code of the last error that occurred (0 if there have been no
 *          errors). The application can set the value to 0 to clear the last error
 *          (by calling the OSI_SetLastError function). The OpenServo I2C software
 *          interface typically passes interer error codes, rather than strings, to
 *          save on microcontroller memory space.
 *
 * static char l_sLastErrorAdditional[32]
 *         o When an error occurs and l_nLastError is set, this buffer is set to
 *           either an empty string ([0]=='\0') or to the text representation of
 *           additional information (e.g. the I2C bus number and servo address).
 *           This string is set to an emprt string when the OSI_SetLastError
 *           function is called with NULL as the second argument.
 */
static int_t l_nLastError;
static char l_sLastErrorAdditional[32];

/*
 * static OPENSERVO **l_pServos
 *         o Points to an array of pointers to OPENSERVO structures: a servo descriptor
 *           for each OpenServo identified or added to the list. Initialised to NULL.
 *
 * static int_t l_nServos
 *         o The number of OPENSERVO structure's pointed to by l_pServos.
 *
 * static int_t l_nServosAlloc
 *         o Gets the size of the current allocation for the l_pServos array,
 *           this is so that each time OSI_AddServo() is called, for example,
 *           the array does not need to be reallocated. Potentially only matters
 *           on a microcontroller.
 */
OPENSERVO **l_pServos=NULL;
static int_t l_nServos=0;
static int_t l_nServosAlloc=0;
#define PSERVOSGROW 32 /* The rate at which the l_pServos array should grow when reallocated */

/*******************************************************************************
 *
 * The following provide the indirect register and command address mappings
 * between the "OpenServo Interface addresses" and versions of the OpenServo
 * hardware/firmware: remember that different OpenServo hardware/firmware may
 * have different addressing depending on the firmware revision level.
 *
 * IMPORTANT: The "OpenServo Interface addressing" indexes these arrays, therefore
 *            "gaps" may need to be used (suggested fill: {0xff, 0xff }) to that
 *            OpenServo Interface addresses map correctly.
 */
#ifdef _DEBUG
#define REGITEM(ID,SIZE,BANK,ADDRESS) { OSI_REGID_##ID, SIZE, BANK, ADDRESS }
#define CMDITEM(ID,ADDRESS) { OSI_CMDID_##ID, 0, 0xFF, ADDRESS }
#else
#define REGITEM(ID,SIZE,BANK,ADDRESS) { SIZE, BANK, ADDRESS }
#define CMDITEM(ID,ADDRESS) { 0, 0xFF, ADDRESS }
#endif

/*
 * OpenServo version 3 (OSxV2) with bank switching firmware (BEMF)
 *
 * NOTE: This is currently the definitive "optimised order".
 */
static const OPENSERVOADDRESSMAP l_basemap_V3_bank[]=
{
   REGITEM(DEVICETYPE,               1, 0xFF, 0x00),
   REGITEM(DEVICESUBTYPE,            1, 0xFF, 0x01),
   REGITEM(VERSIONMAJOR,             1, 0xFF, 0x02),
   REGITEM(VERSIONMINOR,             1, 0xFF, 0x03),
   REGITEM(FLAGS,                    2, 0xFF, 0x04),
   REGITEM(TIMER,                    2, 0xFF, 0x06),
   REGITEM(POSITION,                 2, 0xFF, 0x08),
   REGITEM(VELOCITY,                 2, 0xFF, 0x0A),
   REGITEM(CURRENT,                  2, 0xFF, 0x0C),
   REGITEM(PWM_CW,                   1, 0xFF, 0x0E),
   REGITEM(PWM_CCW,                  1, 0xFF, 0x0F),
   REGITEM(BATTVOLTAGE,              2, 0xFF, 0x10),
   REGITEM(CURVE_BUFFER,             1, 0xFF, 0x12),
   REGITEM(TEMPERATURE,              2, 0xFF, 0x13),
   REGITEM(SEEK,                     2, 0xFF, 0x20),
   REGITEM(SEEKVELOCITY,             2, 0xFF, 0x22),
   REGITEM(CURVE_DELTA,              2, 0xFF, 0x24),
   REGITEM(CURVE_POSITION,           2, 0xFF, 0x26),
   REGITEM(CURVE_IN_VELOCITY,        2, 0xFF, 0x28),
   REGITEM(CURVE_OUT_VELOCITY,       2, 0xFF, 0x2A),
   REGITEM(CURVE_RESERVED,           1, 0xFF, 0x2C),
   REGITEM(BANKSELECT,               1, 0xFF, 0x2F),

   REGITEM(BACKEMF,                  2, 0x00, 0x41),

   REGITEM(TWI_ADDRESS,              1, 0x01, 0x40),
   REGITEM(PID_DEADBAND,             1, 0x01, 0x41),
   REGITEM(PID_PGAIN,                2, 0x01, 0x42),
   REGITEM(PID_DGAIN,                2, 0x01, 0x44),
   REGITEM(PID_IGAIN,                2, 0x01, 0x46),
   REGITEM(MINSEEK,                  2, 0x01, 0x48),
   REGITEM(MAXSEEK,                  2, 0x01, 0x4A),
   REGITEM(REVERSESEEK,              1, 0x01, 0x4C),
   REGITEM(PWM_FREQ_DIVIDER,         2, 0x01, 0x4D),
   REGITEM(ALERT_CURR_MAXLIMIT,      2, 0x01, 0x50),
   REGITEM(ALERT_VOLT_MAXLIMIT,      2, 0x01, 0x52),
   REGITEM(ALERT_VOLT_MINLIMIT,      2, 0x01, 0x54),

/*
 * To keep the code simpler, the "command addresses" are placed with the "register
 * addresses" for mapping purposes, OSI_ID_SEPARATOR is used to separate them.
 *
 * NOTE: OSI_ID_SEPARATOR takes the next value following the last OSI_REG_ID, which
 *       are sequential. The command IDs that follow are restarted at a large value
 *       so that the numerical values can be retained in case they are used/stored
 *       by the user.
 */
#ifdef _DEBUG
   { OSI_ID_SEPARATOR,               0, 0XFF, 0xFF }, /* Registers/commands seperator */
#else
   {                                 0, 0XFF, 0xFF }, /* Registers/commands seperator */
#endif

/*
 * Commands
 */
   CMDITEM(RESET,                    0x80),
   CMDITEM(CHECKED_TXN,              0x81),
   CMDITEM(PWM_ENABLE,               0x82),
   CMDITEM(PWM_DISABLE,              0x83),
   CMDITEM(WRITE_ENABLE,             0x84),
   CMDITEM(WRITE_DISABLE,            0x85),
   CMDITEM(REGISTERS_SAVE,           0x86),
   CMDITEM(REGISTERS_RESTORE,        0x87),
   CMDITEM(REGISTERS_DEFAULT,        0x88),
   CMDITEM(EEPROM_ERASE,             0x89),
   CMDITEM(BATTVOLTAGE_RESAMPLE,     0x90),
   CMDITEM(CURVE_MOTION_ENABLE,      0x91),
   CMDITEM(CURVE_MOTION_DISABLE,     0x92),
   CMDITEM(CURVE_MOTION_RESET,       0x93),
   CMDITEM(CURVE_MOTION_APPEND,      0x94),
   CMDITEM(GCALL_ENABLE,             0x95),
   CMDITEM(GCALL_DISABLE,            0x96),
   CMDITEM(GCALL_START_WAIT,         0x97),
   CMDITEM(GCALL_START_MOVE,         0x98),
};

/*
 * OpenServo board version 2.1 with "original" 1.1 (pre bank switching) firmware
 */
static const OPENSERVOADDRESSMAP l_basemap_21_11[]=
{
   REGITEM(DEVICETYPE,               1, 0xFF, 0x00),
   REGITEM(DEVICESUBTYPE,            1, 0xFF, 0x01),
   REGITEM(VERSIONMAJOR,             1, 0xFF, 0x02),
   REGITEM(VERSIONMINOR,             1, 0xFF, 0x03),
   REGITEM(FLAGS,                    2, 0xFF, 0x04),
   REGITEM(TIMER,                    2, 0xFF, 0x06),
   REGITEM(POSITION,                 2, 0xFF, 0x08),
   REGITEM(VELOCITY,                 2, 0xFF, 0x0A),
   REGITEM(CURRENT,                  2, 0xFF, 0x0C),
   REGITEM(PWM_CW,                   1, 0xFF, 0x0E),
   REGITEM(PWM_CCW,                  1, 0xFF, 0x0F),
   REGITEM(BATTVOLTAGE,              2, 0xFF, 0x14),
   REGITEM(CURVE_BUFFER,             1, 0xFF, 0x17),
   REGITEM(TEMPERATURE,              2, 0xFF, 0xFF),
   REGITEM(SEEK,                     2, 0xFF, 0x10),
   REGITEM(SEEKVELOCITY,             2, 0xFF, 0x12),
   REGITEM(CURVE_DELTA,              2, 0xFF, 0x18),
   REGITEM(CURVE_POSITION,           2, 0xFF, 0x1A),
   REGITEM(CURVE_IN_VELOCITY,        2, 0xFF, 0x1C),
   REGITEM(CURVE_OUT_VELOCITY,       2, 0xFF, 0x1E),
   REGITEM(CURVE_RESERVED,           1, 0xFF, 0x16),
   REGITEM(BANKSELECT,               1, 0xFF, 0xFF), /* No "Bank select" in OpenServo 2.1 and earlier */

   REGITEM(BACKEMF,                  2, 0xFF, 0xFF),

   REGITEM(TWI_ADDRESS,              1, 0xFF, 0x20),
   REGITEM(PID_DEADBAND,             1, 0xFF, 0x21),
   REGITEM(PID_PGAIN,                2, 0xFF, 0x22),
   REGITEM(PID_DGAIN,                2, 0xFF, 0x24),
   REGITEM(PID_IGAIN,                2, 0xFF, 0x26),
   REGITEM(MINSEEK,                  2, 0xFF, 0x2A),
   REGITEM(MAXSEEK,                  2, 0xFF, 0x2C),
   REGITEM(REVERSESEEK,              1, 0xFF, 0x2E),
   REGITEM(PWM_FREQ_DIVIDER,         2, 0xFF, 0x28),
   REGITEM(ALERT_CURR_MAXLIMIT,      2, 0xFF, 0xFF), /* No "Current limit for alerts and throttling" in OpenServo 2.1 and earlier */
   REGITEM(ALERT_VOLT_MAXLIMIT,      2, 0xFF, 0xFF), /* No "Max Battery Voltage limit" in OpenServo 2.1 and earlier */
   REGITEM(ALERT_VOLT_MINLIMIT,      2, 0xFF, 0xFF), /* No "Min Battery Voltage limit" in OpenServo 2.1 and earlier */

#ifdef _DEBUG
   { OSI_ID_SEPARATOR,               0, 0XFF, 0xFF }, /* Registers/commands seperator */
#else
   {                                 0, 0XFF, 0xFF }, /* Registers/commands seperator */
#endif

   CMDITEM(RESET,                    0x80),
   CMDITEM(CHECKED_TXN,              0x81),
   CMDITEM(PWM_ENABLE,               0x82),
   CMDITEM(PWM_DISABLE,              0x83),
   CMDITEM(WRITE_ENABLE,             0x84),
   CMDITEM(WRITE_DISABLE,            0x85),
   CMDITEM(REGISTERS_SAVE,           0x86),
   CMDITEM(REGISTERS_RESTORE,        0x87),
   CMDITEM(REGISTERS_DEFAULT,        0x88),
   CMDITEM(EEPROM_ERASE,             0x89),
   CMDITEM(BATTVOLTAGE_RESAMPLE,     0x90),
   CMDITEM(CURVE_MOTION_ENABLE,      0x91),
   CMDITEM(CURVE_MOTION_DISABLE,     0x92),
   CMDITEM(CURVE_MOTION_RESET,       0x93),
   CMDITEM(CURVE_MOTION_APPEND,      0x94),
   CMDITEM(GCALL_ENABLE,             0x95),
   CMDITEM(GCALL_DISABLE,            0x96),
   CMDITEM(GCALL_START_WAIT,         0x97),
   CMDITEM(GCALL_START_MOVE,         0x98),
};

/*
 * OpenServo board version 2.1 with bank switching firmware
 */
static const OPENSERVOADDRESSMAP l_basemap_21_bank[]=
{
   REGITEM(DEVICETYPE,               1, 0xFF, 0x00),
   REGITEM(DEVICESUBTYPE ,           1, 0xFF, 0x01),
   REGITEM(VERSIONMAJOR,             1, 0xFF, 0x02),
   REGITEM(VERSIONMINOR,             1, 0xFF, 0x03),
   REGITEM(FLAGS,                    2, 0xFF, 0x04),
   REGITEM(TIMER,                    2, 0xFF, 0x06),
   REGITEM(POSITION,                 2, 0xFF, 0x08),
   REGITEM(VELOCITY,                 2, 0xFF, 0x0A),
   REGITEM(CURRENT,                  2, 0xFF, 0x0C),
   REGITEM(PWM_CW,                   1, 0xFF, 0x0E),
   REGITEM(PWM_CCW,                  1, 0xFF, 0x0F),
   REGITEM(BATTVOLTAGE,              2, 0xFF, 0x14),
   REGITEM(CURVE_BUFFER,             1, 0xFF, 0x17),
   REGITEM(TEMPERATURE,              2, 0xFF, 0xFF),
   REGITEM(SEEK,                     2, 0xFF, 0x10),
   REGITEM(SEEKVELOCITY,             2, 0xFF, 0x12),
   REGITEM(CURVE_DELTA,              2, 0xFF, 0x18),
   REGITEM(CURVE_POSITION,           2, 0xFF, 0x1A),
   REGITEM(CURVE_IN_VELOCITY,        2, 0xFF, 0x1C),
   REGITEM(CURVE_OUT_VELOCITY,       2, 0xFF, 0x1E),
   REGITEM(CURVE_RESERVED,           1, 0xFF, 0x16),
   REGITEM(BANKSELECT,               1, 0xFF, 0x2F),

   REGITEM(BACKEMF,                  2, 0x00, 0x41),

   REGITEM(TWI_ADDRESS,              1, 0x01, 0x20),
   REGITEM(PID_DEADBAND,             1, 0x01, 0x21),
   REGITEM(PID_PGAIN,                2, 0x01, 0x22),
   REGITEM(PID_DGAIN,                2, 0x01, 0x24),
   REGITEM(PID_IGAIN,                2, 0x01, 0x26),
   REGITEM(MINSEEK,                  2, 0x01, 0x2A),
   REGITEM(MAXSEEK,                  2, 0x01, 0x2C),
   REGITEM(REVERSESEEK,              1, 0x01, 0x2E),
   REGITEM(PWM_FREQ_DIVIDER,         2, 0x01, 0x28),
   REGITEM(ALERT_CURR_MAXLIMIT,      2, 0x01, 0xFF),
   REGITEM(ALERT_VOLT_MAXLIMIT,      2, 0x01, 0xFF),
   REGITEM(ALERT_VOLT_MINLIMIT,      2, 0x01, 0xFF),

#ifdef _DEBUG
   { OSI_ID_SEPARATOR,               0, 0XFF, 0xFF }, /* Registers/commands seperator */
#else
   {                                 0, 0XFF, 0xFF }, /* Registers/commands seperator */
#endif

   CMDITEM(RESET,                    0x80),
   CMDITEM(CHECKED_TXN,              0x81),
   CMDITEM(PWM_ENABLE,               0x82),
   CMDITEM(PWM_DISABLE,              0x83),
   CMDITEM(WRITE_ENABLE,             0x84),
   CMDITEM(WRITE_DISABLE,            0x85),
   CMDITEM(REGISTERS_SAVE,           0x86),
   CMDITEM(REGISTERS_RESTORE,        0x87),
   CMDITEM(REGISTERS_DEFAULT,        0x88),
   CMDITEM(EEPROM_ERASE,             0x89),
   CMDITEM(BATTVOLTAGE_RESAMPLE,     0x90),
   CMDITEM(CURVE_MOTION_ENABLE,      0x91),
   CMDITEM(CURVE_MOTION_DISABLE,     0x92),
   CMDITEM(CURVE_MOTION_RESET,       0x93),
   CMDITEM(CURVE_MOTION_APPEND,      0x94),
   CMDITEM(GCALL_ENABLE,             0x95),
   CMDITEM(GCALL_DISABLE,            0x96),
   CMDITEM(GCALL_START_WAIT,         0x97),
   CMDITEM(GCALL_START_MOVE,         0x98),
};

/*
 * OpenServo version 3 (OSxV2) with "1.1" (pre bank switching) firmware
 */
static const OPENSERVOADDRESSMAP l_basemap_V3_11[]=
{
   REGITEM(DEVICETYPE,               1, 0xFF, 0x00),
   REGITEM(DEVICESUBTYPE,            1, 0xFF, 0x01),
   REGITEM(VERSIONMAJOR,             1, 0xFF, 0x02),
   REGITEM(VERSIONMINOR,             1, 0xFF, 0x03),
   REGITEM(FLAGS,                    2, 0xFF, 0x04),
   REGITEM(TIMER,                    2, 0xFF, 0x06),
   REGITEM(POSITION,                 2, 0xFF, 0x08),
   REGITEM(VELOCITY,                 2, 0xFF, 0x0A),
   REGITEM(CURRENT,                  2, 0xFF, 0x0C),
   REGITEM(PWM_CW,                   1, 0xFF, 0x0E),
   REGITEM(PWM_CCW,                  1, 0xFF, 0x0F),
   REGITEM(BATTVOLTAGE,              2, 0xFF, 0x14),
   REGITEM(CURVE_BUFFER,             1, 0xFF, 0x17),
   REGITEM(TEMPERATURE,              2, 0xFF, 0xFF),
   REGITEM(SEEK,                     2, 0xFF, 0x10),
   REGITEM(SEEKVELOCITY,             2, 0xFF, 0x12),
   REGITEM(CURVE_DELTA,              2, 0xFF, 0x18),
   REGITEM(CURVE_POSITION,           2, 0xFF, 0x1A),
   REGITEM(CURVE_IN_VELOCITY,        2, 0xFF, 0x1C),
   REGITEM(CURVE_OUT_VELOCITY,       2, 0xFF, 0x1E),
   REGITEM(CURVE_RESERVED,           1, 0xFF, 0x16),
   REGITEM(BANKSELECT,               1, 0xFF, 0xFF), /* No "Bank select" in OpenServo 2.1 and earlier */

   REGITEM(BACKEMF,                  2, 0xFF, 0xFF),

   REGITEM(TWI_ADDRESS,              1, 0xFF, 0x20),
   REGITEM(PID_DEADBAND,             1, 0xFF, 0x21),
   REGITEM(PID_PGAIN,                2, 0xFF, 0x22),
   REGITEM(PID_DGAIN,                2, 0xFF, 0x24),
   REGITEM(PID_IGAIN,                2, 0xFF, 0x26),
   REGITEM(MINSEEK,                  2, 0xFF, 0x2A),
   REGITEM(MAXSEEK,                  2, 0xFF, 0x2C),
   REGITEM(REVERSESEEK,              1, 0xFF, 0x2E),
   REGITEM(PWM_FREQ_DIVIDER,         2, 0xFF, 0x28),
   REGITEM(ALERT_CURR_MAXLIMIT,      2, 0xFF, 0xFF), /* No "Current limit for alerts and throttling" in OpenServo 2.1 and earlier */
   REGITEM(ALERT_VOLT_MAXLIMIT,      2, 0xFF, 0xFF), /* No "Max Battery Voltage limit" in OpenServo 2.1 and earlier */
   REGITEM(ALERT_VOLT_MINLIMIT,      2, 0xFF, 0xFF), /* No "Min Battery Voltage limit" in OpenServo 2.1 and earlier */

#ifdef _DEBUG
   { OSI_ID_SEPARATOR,               0, 0XFF, 0xFF }, /* Registers/commands seperator */
#else
   {                                 0, 0XFF, 0xFF }, /* Registers/commands seperator */
#endif

   CMDITEM(RESET,                    0x80),
   CMDITEM(CHECKED_TXN,              0x81),
   CMDITEM(PWM_ENABLE,               0x82),
   CMDITEM(PWM_DISABLE,              0x83),
   CMDITEM(WRITE_ENABLE,             0x84),
   CMDITEM(WRITE_DISABLE,            0x85),
   CMDITEM(REGISTERS_SAVE,           0x86),
   CMDITEM(REGISTERS_RESTORE,        0x87),
   CMDITEM(REGISTERS_DEFAULT,        0x88),
   CMDITEM(EEPROM_ERASE,             0x89),
   CMDITEM(BATTVOLTAGE_RESAMPLE,     0x90),
   CMDITEM(CURVE_MOTION_ENABLE,      0x91),
   CMDITEM(CURVE_MOTION_DISABLE,     0x92),
   CMDITEM(CURVE_MOTION_RESET,       0x93),
   CMDITEM(CURVE_MOTION_APPEND,      0x94),
   CMDITEM(GCALL_ENABLE,             0x95),
   CMDITEM(GCALL_DISABLE,            0x96),
   CMDITEM(GCALL_START_WAIT,         0x97),
   CMDITEM(GCALL_START_MOVE,         0x98),
};

/*******************************************************************************
 *
 * A handy index of configuration items, including basic default configurations
 * and OPENSERVOADDRESSMAP structures...
 *
 * NOTE: All the maps must be the same size and in the same order...
 */
static struct 
{
   OSI_DTYPE m_nID;
   int_t m_nMapSize;
   const OPENSERVOADDRESSMAP *m_pAddressMap;
   OPENSERVOBOARDCONF m_DefBoardConf;
} l_pTypeToDefConf[]=
{
   { OSI_DTYPE_21_11,     sizeof(l_basemap_21_11), l_basemap_21_11,      { 5000,  4700,  2000,  1000 } },
   { OSI_DTYPE_21_BANK,   sizeof(l_basemap_21_bank), l_basemap_21_bank,  { 5000,  4700,  2000,  1000 } },
   { OSI_DTYPE_V3_11,     sizeof(l_basemap_V3_11), l_basemap_V3_11,      { 5000, 10000, 20000, 20000 } },
   { OSI_DTYPE_V3_BANK,   sizeof(l_basemap_V3_bank), l_basemap_V3_bank,  { 5000, 10000, 20000, 20000 } },
};

/*******************************************************************************
 *
 * Local store for global register maps
 */
static struct
{
   const OSI_DTYPE m_nID;
   OPENSERVOADDRESSMAP *m_pGlobalMap;
} l_pGlobalMaps[]=
{
   { OSI_DTYPE_21_11,     NULL },
   { OSI_DTYPE_21_BANK,   NULL },
   { OSI_DTYPE_V3_11,     NULL },
   { OSI_DTYPE_V3_BANK,   NULL },
};

/*******************************************************************************
 *
 * Non-inline local function prototypes
 */
static OPENSERVO *OSI_NewOPENSERVO();
static void OSI_DeleteOPENSERVO(OPENSERVO *posid);
static OPENSERVO *OSI_AddServoInt(int_t nbus, byte_t naddress, bool_t bSharedMap, bool_t bManual);
static int_t OSI_IdentifyDevice(OPENSERVO *posid);
static int_t OSI_SetRegisterMappings(OPENSERVO *posid, bool_t bSharedMap, OPENSERVOADDRESSMAP *pDefMap);
// TODO: Delete: static int OSI_InitialiseRegisterMaps();
// TODO: Delete: static OPENSERVOADDRESSMAP *OSI_CreateRegisterMap(OSI_DTYPE nType);
// TODO: Delete: static void OSI_ReleaseRegisterMaps();
// TODO: Delete: static int OSI_SetRegisterMapEntry(OPENSERVOADDRESSMAP *pMap, int n,
//    OSI_REGID nID, int nOffset, BYTE nSize0, BYTE nSize1, BYTE nBank, BYTE nAddress);

/*******************************************************************************
 *
 * OSI_GetLastError, OSI_GetLastErrorText and OSI_SetLastError are
 * implemented first, this allows them to be functions callable from outside
 * of this module, but their use inside to be macroed, to save on space and
 * speed- still worrying about the small memory space microcontroller.
 */

/*******************************************************************************
 *
 * int_t OSI_GetLastError()
 *
 * Synopsis
 *
 * The OSI_GetLastError function returns the last OSI error code that was set.
 *
 * NOTE: If the last operation performed was successful, then this function
 *       returns OSI_SUCCESS (0).
 *
 * Return
 *
 * int_t   o The function returns the current error value or OSI_SUCCESS.
 */
int_t OSI_GetLastError()
{
   return l_nLastError;
}
#define OSI_GetLastError() l_nLastError

/*******************************************************************************
 *
 * const char *OSI_GetLastErrorText()
 *
 * Synopsis
 *
 * The OSI_GetLastErrorText function returns a pointer to any text that may be
 * associated with the last error.
 *
 * NOTE: The returned pointer may be NULL or it may point to an empty string.
 *
 *       The absence or presence of an error text does not indicate whether an
 *       error has occurred, this is determined by calling the OSI_GetLastError()
 *       function.
 *
 *       The buffer pointed is not guaranteed to be the same from call to call.
 *
 * Return
 *
 * const char *
 *         o The function returns a pointer to a constant string buffer.
 */
const char *OSI_GetLastErrorText()
{
   return l_sLastErrorAdditional;
}
#define OSI_GetLastErrorText() l_sLastErrorAdditional

/*******************************************************************************
 *
 * int_t OSI_SetLastError(int_t nerror, const char *ptext)
 *
 * The OSI_SetLastError function allows the caller to set the last error value
 * and text.
 *
 * NOTE: This would typically be used to clear the error flag, which is achieved
 *       with the following call:
 *
 *          OSI_SetLastError(OSI_SUCCESS,NULL);
 *
 * Parameters
 *
 * int_t nerror
 *         o The error code to be set or OSI_SUCCESS (0).
 *
 * const char *ptext
 *         o Point to a null terminated string to be copied to a static or semi-static
 *           buffer.
 *
 * NOTE: If the string is too long for the static buffer, it will be truncated. A static
 *       buffer is used, so that repeated reallocations do not occur in small memory
 *       environments.
 *
 *       If nerror is OSI_SUCCESS (0), then ptext parameter is ignored: an empty string
 *       is stored. If ptext is NULL, an empty string is stored.
 *
 * Return
 *
 * int_t   o The function returns the error value that has been set.
 */
int_t OSI_SetLastError(int_t nerror, char *pat)
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
#define OSI_SetLastError(NERROR,PAT) (PAT==NULL?l_sLastErrorAdditional[0]='\0':(strncpy(l_sLastErrorAdditional,PAT,sizeof(l_sLastErrorAdditional)-1),l_sLastErrorAdditional[sizeof(l_sLastErrorAdditional)-1]='\0'),l_nLastError=NERROR)

/*******************************************************************************
 *
 * __inline int_t IDtoIndex(OSI_REGID nID, bool_t *pbIsCommandID)
 *
 * Translates a register or command ID into an address mapping index.
 *
 * WARNING: Currently the funcion is returning an int_t, with -1 as a "not found"
 *          indicator. If int_t is defined as an 8-bit quantity, then there is
 *          only room for 127 items in a table.
 */
__inline int_t IDtoIndex(OSI_REGID nID, bool_t *pbIsCommandID)
{
   if((int_t)nID>=(int_t)OSI_REGID_DEVICETYPE && (int_t)nID<(int_t)OSI_ID_SEPARATOR)
   {
      *pbIsCommandID=FALSE;
      OSI_SetLastError(OSI_SUCCESS,NULL);
   } else
   {
      if((int_t)nID>(int_t)OSI_ID_SEPARATOR && (int_t)nID<(int_t)OSI_ID_END)
      {
         nID=1+(int_t)nID-(int_t)OSI_CMDID_RESET+(int_t)OSI_ID_SEPARATOR;
         *pbIsCommandID=TRUE;
         OSI_SetLastError(OSI_SUCCESS,NULL);
      } else
      {
         nID=-1;
         OSI_SetLastError(OSI_ERR_BADID,NULL);
      }
   }
   return (int_t)nID;
}

/*******************************************************************************
 *
 * __inline const OPENSERVOADDRESSMAP *IDtoMap(OPENSERVO *posid, OSI_REGID nID, bool_t *pbIsCommand)
 *
 * Translates a register or command ID into an address mapping pointer.
 *
 * WARNING: If int_t is defined as an 8-bit quantity, then there is only room for
 *          127 items in a table.
 */
__inline const OPENSERVOADDRESSMAP *IDtoMap(OPENSERVO *posid, OSI_REGID nID, bool_t *pbIsCommand)
{
   const OPENSERVOADDRESSMAP *p=NULL;
   int_t r=IDtoIndex(nID,pbIsCommand);
   if(r>=0)
   {
      p=&posid->m_pAddressMap[r];
   }
   return p;
}

/*******************************************************************************
 *
 * __inline const OPENSERVOADDRESSMAP *RegisterIDtoAddressMap(OPENSERVO *posid, OSI_REGID nID)
 *
 * Translates a register ID into an register address mapping pointer.
 *
 * WARNING: If int_t is defined as an 8-bit quantity, then there is only room for
 *          127 items in a table.
 */
__inline const OPENSERVOADDRESSMAP *RegisterIDtoAddressMap(OPENSERVO *posid, OSI_REGID nID)
{
   const OPENSERVOADDRESSMAP *p=NULL;
   bool_t bIsCommand;
   int_t r=IDtoIndex(nID,&bIsCommand);
   if(r>=0 && !bIsCommand)
   {
      p=&posid->m_pAddressMap[r];
   }
   return p;
}

/*******************************************************************************
 *
 * __inline const OPENSERVOCOMMANDMAP *CommandIDtoCommandMap(OPENSERVO *posid, OSI_CMDID nID)
 *
 * Translates a command ID into an command mapping pointer.
 *
 * WARNING: If int_t is defined as an 8-bit quantity, then there is only room for
 *          127 items in a table.
 */
__inline const OPENSERVOCOMMANDMAP *CommandIDtoCommandMap(OPENSERVO *posid, OSI_CMDID nID)
{
   const OPENSERVOCOMMANDMAP *p=NULL;
   bool_t bIsCommand;
   int_t r=IDtoIndex(nID,&bIsCommand);
   if(r>=0 && bIsCommand)
   {
      p=&posid->m_pAddressMap[r];
   }
   return p;
}

/*******************************************************************************
 *
 * int_t OSI_Init(bool_t bScan, bool_t bSharedMaps)
 *
 * Synopsis
 *
 * Initialises the static data structures, initialises I2C bus (if it is not
 * already), opens devices, scans for servos, etc.
 *
 * Parameters
 *
 * bool_t bScan
 *         o Specify FALSE (0) to prevent the initialisation function from scanning for
 *           servos: the application will be calling OSI_RescanForServos or manually adding
 *           servos with calls to OSI_AddServo at a later date. If specified as TRUE (1),
 *           the function calls OSI_RescanForServos to look for servos.
 *
 * bool_t bSharedMaps
 *         o Specify FALSE (0) if the servos found shoule use a private "OpenServo Interface"
 *           "address" map (they start with a copy of the default for servos of their
 *           type), or (TRUE) 1 if the shared "OpenServo Interface" "address" maps for servos
 *           of the same type should be used.
 *
 *           NOTE: A shared map will save memory, but may come with I2C bus bandwidth
 *                 caveats (depending on what use is made of the OpenServo redirected
 *                 registers facility).
 *
 * Returns
 *
 * int_t   o The function returns OSI_SUCCESS (0) if successful. A non-zero return
 *           indidates that an error has occurred.
 */
int_t OSI_Init(bool_t bScan, bool_t bSharedMaps)
{

/*
 * Set the endian flag, m_bIsLittleEndian, "TRUE" on little endian platforms
 */
   union { uint16_t i; byte_t b[2]; } k;
   k.i=256;
   m_bIsLittleEndian=k.b[1];

/*
 * I2CManager needs to have been initialised (by the application) is it is global
 * and may be applicable to more I2C devices than just OpenServos.
 */
   if(!I2CM_IsInitialised())
   {
      OSI_SetLastError(OSI_ERR_HARDWARE,NULL);
   } else
   {
      if(l_bInit==0)
      {
         OSI_SetLastError(OSI_ERR_ALREADYINITIALISED,NULL);
      } else
      {
         bool_t bOK=TRUE;
         l_bInit=0;
         if(bScan!=0 && OSI_RescanForServos(bSharedMaps)!=OSI_SUCCESS)
         {
            l_bInit=0; /* Needed for a successfull call to OSI_Shutdown */
            OSI_Shutdown();
            l_bInit=1;
         } else
         {
            OSI_SetLastError(OSI_SUCCESS,NULL);
         }
      }
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * bool_t OSI_IsInitialised()
 *
 * Synopsis
 *
 * Function to determine if the OpenServo interface is initialised.
 *
 * Parameters
 *
 *         o None
 *
 * Returns
 *
 * bool_t  o Returns TRUE (1) if the OpenServo interface is initialised (by a previous
 *           call to OSI_Init and prior to any calls to OSI_Shutdown). otherwise the
 *           function returns FALSE (0).
 */
bool_t OSI_IsInitialised()
{
   return l_bInit==0;
}

/*******************************************************************************
 *
 * int_t OSI_Shutdown()
 *
 * Synopsis
 *
 * Shuts down the OpenServo interface (releases resources, etc.)
 *
 * NOTE: Does NOT shutdown the I2C Manager.
 *
 *       Even if an error occurs during shutdown (resulting in a return other than
 *       OSI_SUCCESS), the OpenServo interface will be considered to be in the
 *       uninitialised state- relevant internal variables are set to that effect.
 *
 * Parameters
 *
 *         o None
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if the shutdown of the OpenServo
 *           interface has completed successfully. A non-zero return indicates that
 *           an error has occurred.
 */
int_t OSI_Shutdown()
{
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t i;
      for(i=0; i<l_nServos; i++)
      {
         if(l_pServos[i]!=NULL)
         {
            OSI_DeleteOPENSERVO(l_pServos[i]);
            l_pServos[i]=NULL;
         }
      }
      if(l_pServos!=NULL)
      {
         free(l_pServos);
         l_pServos=NULL;
      }
      l_nServos=0;
      l_nServosAlloc=0;
      l_bInit=1;
      OSI_SetLastError(OSI_SUCCESS,NULL);
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_GetDeviceCount()
 *
 * Synopsis
 *
 * Returns the number of automatically identified and manually added OpenServo
 * devices.
 *
 * Parameters
 *
 * None.
 *
 * Return
 *
 * int_t   o The function returns  the number of known OpenServo devices, this may
 *           be 0. If OSI is not initialised or some other error has occurred,
 *           the function returns -1.
 */
int_t OSI_GetDeviceCount()
{
   int_t rc=-1;
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      rc=l_nServos;
   }
   return rc;
}

/*******************************************************************************
 *
 * static OPENSERVO *OSI_NewOPENSERVO()
 *
 * Synopsis
 *
 * Allocates and initialises to empty a new OPENSERVO structure.
 *
 * Return
 *
 * OPENSERVO *
 *         o If successful, the function returns a pointer to a new "empty"
 *           OPENSERVO structure. If an error has occurred, then the function
 *           returns NULL: the caller can call OSI_GetLastError to get extended
 *           error information.
 */
static OPENSERVO *OSI_NewOPENSERVO()
{
   OPENSERVO *posid=(OPENSERVO *)malloc(sizeof(OPENSERVO));
   if(posid==NULL)
   {
      OSI_SetLastError(OSI_ERR_OUTOFMEMORY,NULL);
   } else
   {
      memset(posid,0,sizeof(OPENSERVO));
      posid->m_nDType=OSI_DTYPE_UNKNOWN;
      posid->m_nBus=0xFF;
      posid->m_nAddress=0xFF;
      posid->m_pAddressMap=NULL;
      posid->m_nBankSelect=0xFF;
      posid->m_nCurRegAddress=0xFF;
      posid->m_nIOStatus=0x00;
   }
   return posid;
}

/*******************************************************************************
 *
 * static void OSI_DeleteOPENSERVO(OPENSERVO *posid)
 *
 * Synopsis
 *
 * The function release the OPENSERVO structure pointed to by the posid argument,
 * and well as any private resources that may be associated with it.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OEPNSERVO structure to be released.
 *
 * Return
 *
 * None.
 */
static void OSI_DeleteOPENSERVO(OPENSERVO *posid)
{
   if(posid!=NULL)
   {
      free(posid); /* TODO: Private resources */
   }
}

/*******************************************************************************
 *
 * static OPENSERVO *OSI_AddServoInt(int_t nbus, byte_t naddress, bool_t bSharedMap, bool_t bManual)
 *
 * Synopsis
 *
 * Adds a servo to the internal list. This function is called by the OSI_AddServo
 * and OSI_RescanForServos functions: it actually does the work!
 *
 * NOTE: The function does not verify that
 * Parameters
 *
 * int_t nbus
 *         o Specifies the I2C Manager bus number to which the servo is attached.
 *
 * byte_t naddress
 *         o Specifies the I2C address of the server.
 *
 * bool_t bSharedMap
 *         o Specify FALSE (0) if the OpenServo is to use a private "OpenServo Interface" "address"
 *           map (it starts with a copy of the default for OpenServos of its type), or
 *           TRUE (1) if the shared "OpenServo Interface" "address" map for OpenServos of the same
 *           type should be used.
 *
 *           NOTE: A shared map will save memory, but may come with I2C bus bandwidth
 *                 caveats (depending on what use is made of the OpenServo redirected
 *                 registers facility).
 *
 * bool_t bManual
 *         o Specify FALSE (0) to indicate that the servo is being added as part of an
 *           automatic scan (i.e. when called from OSI_RescanForServos), or TRUE (1)
 *           for manually added servos (i.e. when called from OSI_AddServo).
 *
 * Return
 *
 * OPENSERVO *
 *         o If the specified servo has been successfully added to the internal
 *           list, then the function returns a pointer to the OPENSERVO structure
 *           that defines it. If an error has occurred, then NULL is returned: the
 *           caller can call OSI_GetLastError to get extended error information.
 */
static OPENSERVO *OSI_AddServoInt(int_t nbus, byte_t naddress, bool_t bSharedMap, bool_t bManual)
{

/*
 * Allocate and initialise a new OPENSERVO structure to store this OpenServo
 */
   OPENSERVO *posid=OSI_NewOPENSERVO();
   if(posid!=NULL)
   {
      posid->m_nBus=nbus;
      posid->m_nAddress=naddress;

/*
 * Is this an OpenServo?
 *
 * NOTE: Currently assume that even read errors imply "not an OpenServo".
 */
      if(OSI_IdentifyDevice(posid)!=OSI_SUCCESS || posid->m_nDType==OSI_DTYPE_UNKNOWN)
      {
         OSI_SetLastError(OSI_ERR_NOTANOPENSERVO,NULL);
         OSI_DeleteOPENSERVO(posid);
         posid=NULL;
      } else
      {

/*
 * Set the default "hardware configuration" and assign register mappings
 */
         int_t i;
         for(i=0; i>=0 && i<sizeof(l_pTypeToDefConf)/sizeof(l_pTypeToDefConf[0]); i++)
         {
            if(l_pTypeToDefConf[i].m_nID==posid->m_nDType)
            {
               if(OSI_SetHardwareConf(posid,
                  l_pTypeToDefConf[i].m_DefBoardConf.m_VREG,
                  l_pTypeToDefConf[i].m_DefBoardConf.m_R3R4,
                  l_pTypeToDefConf[i].m_DefBoardConf.m_R4R6,
                  l_pTypeToDefConf[i].m_DefBoardConf.m_nClock)==OSI_SUCCESS)
               {
                  posid->m_pAddressMap=l_pTypeToDefConf[i].m_pAddressMap;
               }
               i=-2; /* Found! */
            }
         }
         if(i>=0) /* Not found ? */
         {
            OSI_SetLastError(OSI_ERR_SOFTWARE,NULL);
         }
         if(OSI_GetLastError()!=OSI_SUCCESS)
         {
            OSI_DeleteOPENSERVO(posid);
            posid=NULL;
         } else
         {


/*
 * Store it in the list, however, on failure the new openervo is deleted
 *
 * NOTE: May need to grow the list, of allocate it for the first time.
 */
            if(l_nServos>=l_nServosAlloc) /* NOTE: Technically, > means there has been a software failure or corruption */
            {
               OPENSERVO **pnew=(OPENSERVO **)realloc(l_pServos,(l_nServosAlloc+PSERVOSGROW)*sizeof(OPENSERVO *));
               if(pnew==NULL)
               {
                  OSI_SetLastError(OSI_ERR_OUTOFMEMORY,NULL);
               } else
               {
                  int_t i;
                  l_pServos=pnew;
                  l_nServosAlloc+=PSERVOSGROW;
                  for(i=l_nServos; i<l_nServosAlloc; i++)
                  {
                     l_pServos[i]=NULL;
                  }
               }
               if(l_nServos>=l_nServosAlloc)
               {
                  OSI_DeleteOPENSERVO(posid);
                  posid=NULL;
               } else
               {
                  l_pServos[l_nServos++]=posid;
               }
            }
         }
      }
   }
   return posid;
}

/*******************************************************************************
 *
 * static int_t OSI_IdentifyDevice(OPENSERVO *posid)
 *
 * Synopsis
 *
 * Internal function: determines if a newly identified (the OPENSERVO structure
 * is only initialised with bus and device address) I2C device is an OpenServo,
 * and if it is, which version it is.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that defines the I2C bus and address
 *           of the device being tested.
 *
 *           NOTE: If an OpenServo device is successfully identified, then the
 *                 function sets the following members:
 *
 *                    m_nDType
 *                    m_regs.m_DeviceType
 *                    m_regs.m_DeviceSubtype
 *                    m_regs.m_VersionMajor
 *                    m_regs.m_VersionMinor
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if an OpenServo device has been
 *           successfully identified. A non-zero return indicates that the device
 *           was not recognisable as an OpenServo, or that an error has occurred.
 */
static int_t OSI_IdentifyDevice(OPENSERVO *posid)
{
#if 0
   posid->m_nDType=OSI_DTYPE_V3_BANK; /* A big assumption for test and devlopment!!! TODO!!! */
   return OSI_SUCCESS;
}
#else // Needs clarification from the team as to versioning
   byte_t l_DeviceType=0xFF;
   byte_t l_DeviceSubtype=0xFF;
   byte_t l_VersionMajor=0xFF;
   byte_t l_VersionMinor=0xFF;

/*
 * Ensure it defaults to an unknown device
 */
   posid->m_nDType=OSI_DTYPE_UNKNOWN;

/*
 * For the purposes of interrogating the servo, provide a temporary base address map...
 *
 * NOTE: For ease of implementation, this function will use the OSI functions themselves.
 */
   posid->m_pAddressMap=l_basemap_V3_bank;

/*
 * Open the servo...
 */
   if(OSI_Open(posid,0)==OSI_SUCCESS)
   {

/*
 * Read the device major type, currently all OpenServo versions have a device type of
 * either 1 (v2.1 boards) or 2 (v3 boards).
 */
      if(OSI_ReadRegister(posid,OSI_REGID_DEVICETYPE,&l_DeviceType,sizeof(l_DeviceType))==OSI_SUCCESS &&
         (l_DeviceType==1 || l_DeviceType==2))
      {

/*
 * Read the device subtype: either this repeatedly reads as 1, or returns the sequence
 * { 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x65, 0x72, 0x76, 0x6F }  (i.e. "OpenServo").
 *
 * NOTE: If the OpenServo sequence is returned, then we know that the "bank switching"
 * firmware is loaded.
 *
 * NOTE: It has to be assumed that an OpenServo could start anywhere in the 0x4F, 0x70,
 *       0x65, 0x6E, 0x53, 0x65, 0x72, 0x76, 0x6F sequence. The string contains two
 *       lower case "e" characters, but none of the other characters are duplicated.
 *       In order to reduces reading from devices that might not be an OpenServo, but
 *       return one of the characters from the string, we discard the first character
 *       read, if it is an "e", and then process the buffer in a circular fashion.
 */
         bool_t bOK=0;
static byte_t STR_OPENSERVO[]={ 0x4F, 0x70, 0x65, 0x6E, 0x53, 0x65, 0x72, 0x76 /*, 0x6F */ };
         if(OSI_ReadRegister(posid,OSI_REGID_DEVICESUBTYPE,&l_DeviceSubtype,sizeof(l_DeviceSubtype))==OSI_SUCCESS)
         {
            if(l_DeviceSubtype==1)
            {
               if(OSI_ReadRegister(posid,OSI_REGID_DEVICESUBTYPE,&l_DeviceSubtype,sizeof(l_DeviceSubtype))==OSI_SUCCESS &&
                  l_DeviceSubtype==0x01)
               {
                  bOK=1;
               }
            } else
            {
               if(l_DeviceSubtype!='e' || OSI_ReadRegister(posid,OSI_REGID_DEVICESUBTYPE,&l_DeviceSubtype,sizeof(l_DeviceSubtype))==OSI_SUCCESS)
               {
                  int_t iget=-1,i;
                  for(i=0; iget==-1 && i<sizeof(STR_OPENSERVO); i++)
                  {
                     if(STR_OPENSERVO[i]==l_DeviceSubtype)
                     {
                        iget=i;
                     }
                  }
                  if(iget>=0)
                  {
                     bOK=1; /* Assume a match, set FALSE again if no match or an error occurs */
                     for(i=0; bOK && i<sizeof(STR_OPENSERVO)-1; i++)
                     {
                        if(iget>=sizeof(STR_OPENSERVO)-1)
                        {
                           iget=0;
                        } else
                        {
                           iget++;
                        }
                        if(OSI_ReadRegister(posid,OSI_REGID_DEVICESUBTYPE,&l_DeviceSubtype,sizeof(l_DeviceSubtype))!=OSI_SUCCESS ||
                           l_DeviceSubtype!=STR_OPENSERVO[iget])
                        {
                           bOK=0;
                        }
                     }
                     if(bOK)
                     {
                        l_DeviceSubtype='O';
                     }
                  }
               }
            }
         }

/*
 * If the device type appears to be an OpenServo, then read and check the version
 */
         if(bOK)
         {
            if(OSI_ReadRegister(posid,OSI_REGID_VERSIONMAJOR,&l_VersionMajor,sizeof(l_VersionMajor))==OSI_SUCCESS &&
               (l_DeviceSubtype=='O' || l_VersionMajor==0x00) &&
               OSI_ReadRegister(posid,OSI_REGID_VERSIONMINOR,&l_VersionMinor,sizeof(l_VersionMinor))==OSI_SUCCESS &&
               (l_DeviceSubtype=='O' || l_VersionMinor==0x02))
            {

/*
 * Classify this version
 */
               switch(l_DeviceSubtype)
               {

/*
 * Older firmware: but is it a OpenServo 2.1 or OSxV2 board?
 */
                  case 0x01:
                     posid->m_nDType=OSI_DTYPE_V3_11; // TODO
                     break;

/*
 * Bank switched firmware: but is it a OpenServo 2.1 or OSxV2 board?
 */
                  case 'O':
                     posid->m_nDType=OSI_DTYPE_V3_BANK; // TODO
                     break;

                  default:
                     bOK=0;
                     break;
               }
            }
         }
      }

/*
 * Close the servo
 */
      OSI_Close(posid);
   }

/*
 * If there has been no error set, but the OpenServo device type is not set, then
 * assume it was not recogised and set an error
 */
   if(posid->m_nDType==OSI_DTYPE_UNKNOWN)
   {
      OSI_SetLastError(OSI_ERR_NOTANOPENSERVO,NULL);
   }

/*
 * Clear any temporary map
 */
   posid->m_pAddressMap=NULL;
   posid->m_nBankSelect=0xFF;
   posid->m_nCurRegAddress=0xFF;

   return OSI_GetLastError();
}
#endif

/*******************************************************************************
 *
 * int_t OSI_RescanForServos(bool_t bSharedMaps)
 *
 * Synopsis
 *
 * Releases any existing list of OpenServo devices, including any that may have
 * been added by application calling the OSI_AddServoInt function. It then attempts
 * to scan the I2C Manager for buses and devices that appear to be OpenServos,
 * rebuilding the internal index.
 *
 * TODO: Keep manually added servos.
 *
 * Parameters
 *
 * bool_t bSharedMaps
 *         o Specify FALSE (0) if the OpenServos found shoule use a private "OpenServo Interface"
 *           "address" map (they start with a copy of the default for OpenServos of their
 *           type), or TRUE (1) if the shared "OpenServo Interface" "address" maps for OpenServos
 *           of the same type should be used.
 *
 *           NOTE: A shared map will save memory, but may come with I2C bus bandwidth
 *                 caveats (depending on what use is made of the OpenServo redirected
 *                 registers facility).
 *
 * Return
 *
 * int     o The function returns OSI_SUCCESS (0) if successful. A non-zero return
 *           indidates that an error has occurred.
 */
int_t OSI_RescanForServos(bool_t bSharedMaps)
{
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t i,j;
      bool_t bOK=TRUE;

/*
 * Reset...
 */
      for(i=0; i<l_nServos; i++)
      {
         if(l_pServos[i]!=NULL)
         {
            OSI_DeleteOPENSERVO(l_pServos[i]);
            l_pServos[i]=NULL;
         }
      }
      if(l_pServos!=NULL)
      {
         free(l_pServos);
         l_pServos=NULL;
      }
      l_nServos=0;
      l_nServosAlloc=0;

/*
 * Rescan the I2C bus for devices
 *
 * NOTE: This could affect all users of the I2C bus, when used through the
 *       "I2C Manager" interface.
 */
      if(I2CM_RescanForDevices()!=I2CM_SUCCESS)
      {
         OSI_SetLastError(OSI_ERR_SOFTWARE,"I2C interface");
      } else
      {

/*
 * The number of devices currently attached to all buses is the maximum number of
 * OpenServos. Allocate an array for storing their identities.
 */
         int_t nb=0,n=0;
         nb=I2CM_GetBusCount();
         n=I2CM_GetDeviceCount();
         if(nb<1 || n<0)
         {
            OSI_SetLastError(OSI_ERR_SOFTWARE,"I2C interface");
         } else
         {
            if(n>0)
            {

/*
 * For each address on each bus that appears to be an OpenServo, add its address
 * to the list.
 *
 * TODO: What is a non interfering way to detect that a device is an OpenServo?
 */
               for(j=0; bOK && j<nb; j++)
               {
                  n=I2CM_GetDeviceCount(j);
                  if(n<0)
                  {
                     OSI_SetLastError(OSI_ERR_SOFTWARE,"I2C interface");
                     bOK=FALSE;
                  } else
                  {
                     for(i=0; bOK && i<n; i++)
                     {
                        int_devaddr_t nAddress=I2CM_GetDeviceAddress(j,(byte_t)i);
                        if(nAddress!=(int_devaddr_t)-1 && nAddress<=0xFF)
                        {
                           bOK=OSI_AddServoInt(j,(byte_t)nAddress,bSharedMaps,0)!=NULL || /* OSI_AddServoInt sets any error code */
                              OSI_GetLastError()==OSI_ERR_NOTANOPENSERVO;
                        }
                     }
                  }
               }
            }
         }
         l_bInit=0; /* Also needed for a successfull call to OSI_Shutdown */
         if(!bOK)
         {
            OSI_Shutdown();
            l_bInit=1;
         } else
         {
            OSI_SetLastError(OSI_SUCCESS,NULL);
         }
      }
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * OPENSERVO *OSI_AddServo(int_t nbus, byte_t naddress, bool_t bSharedMap)
 *
 * Synopsis
 *
 * Adds a manual entered servo to the internal list.
 *
 * NOTE: The function does not verify that there is an OpenServo at the address
 *       given on the specified bus.
 *
 * Parameters
 *
 * int_t nbus
 *         o Specifies the I2C Manager bus number to which the servo is attached.
 *
 * byte_t naddress
 *         o Specifies the I2C address of the server.
 *
 * bool_t bSharedMap
 *         o Specify FALSE (0) if the OpenServo is to use a private "OpenServo Interface" "address"
 *           map (it starts with a copy of the default for OpenServos of its type), or
 *           TRUE (1) if the shared "OpenServo Interface" "address" map for OpenServos of the same
 *           type should be used.
 *
 *           NOTE: A shared map will save memory, but may come with I2C bus bandwidth
 *                 caveats (depending on what use is made of the OpenServo redirected
 *                 registers facility).
 *
 * Return
 *
 * OPENSERVO *
 *         o If the specified servo has been successfully added to the internal
 *           list, then the function returns a pointer to the OPENSERVO structure
 *           that defines it. If an error has occurred, then NULL is returned: the
 *           caller can call OSI_GetLastError to get extended error information.
 */
OPENSERVO *OSI_AddServo(int_t nbus, byte_t naddress, bool_t bSharedMap)
{
   return OSI_AddServoInt(nbus,naddress,bSharedMap,1);
}

/*******************************************************************************
 *
 * int_t OSI_SetHardwareConf(OPENSERVO *posid, uint16_t nVREG, uint16_t nR3R4, uint16_t nR4R6, uint16_t nClock)
 *
 * Synopsis
 *
 * Sets the board configuation values.
 */
int_t OSI_SetHardwareConf(OPENSERVO *posid, uint16_t nVREG, uint16_t nR3R4, uint16_t nR4R6, uint16_t nClock)
{
   posid->m_conf.m_VREG=nVREG;
   posid->m_conf.m_R3R4=nR3R4;
   posid->m_conf.m_R4R6=nR4R6;
   posid->m_conf.m_nClock=nClock;
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * OPENSERVO *OSI_GetDeviceIDByPOS(int_t ndevice)
 */
OPENSERVO *OSI_GetDeviceIDByPOS(int_t ndevice)
{
   OPENSERVO *posid=NULL;
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      if(ndevice<0 || ndevice>=l_nServos)
      {
         OSI_SetLastError(OSI_ERR_BADARGS,NULL);
      } else
      {
         posid=l_pServos[ndevice];
      }
   }
   return posid;
}

/*******************************************************************************
 *
 * OPENSERVO *OSI_GetDeviceIDByAddress(int_t nbus, byte_t naddress)
 */
OPENSERVO *OSI_GetDeviceIDByAddress(int_t nbus, byte_t naddress)
{
   OPENSERVO *posid=NULL;
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      int_t i;
      for(i=0; posid==NULL && i<l_nServos; i++)
      {
         if(l_pServos[i]!=NULL &&
            l_pServos[i]->m_nBus==nbus && l_pServos[i]->m_nAddress==naddress)
         {
            posid=l_pServos[i];
         }
      }
   }
   return posid;
}

/*******************************************************************************
 *
 * OPENSERVO *OSI_GetDeviceIDByAddressA(const char *pBusName, byte_t naddress)
 */
OPENSERVO *OSI_GetDeviceIDByAddressA(const char *pBusName, byte_t naddress)
{
   OPENSERVO *posid=NULL;
   if(l_bInit!=0)
   {
      OSI_SetLastError(OSI_ERR_NOTINITIALISED,NULL);
   } else
   {
      char buffer[32];
      int_t i,nbus=I2CM_GetBusCount();
      for(; posid==NULL && nbus>=0; nbus--)
      {
         if(I2CM_GetBusName(nbus,buffer,sizeof(buffer))!=I2CM_SUCCESS)
         {
            nbus=-1;
            OSI_SetLastError(OSI_ERR_HARDWARE,NULL);
         } else
         {
            if(stricmp(pBusName,buffer)==0)
            {
               for(i=0; posid==NULL && i<l_nServos; i++)
               {
                  if(l_pServos[i]!=NULL &&
                     l_pServos[i]->m_nBus==nbus && l_pServos[i]->m_nAddress==naddress)
                  {
                     posid=l_pServos[i];
                  }
               }
               nbus=-1; /* Bus search complete, bail out of loop */
            }
         }
      }
   }
   return posid;
}

/*******************************************************************************
 *
 * static int_t OSI_SortTransactionSet(OPENSERVO *posid, OPENSERVOTRANSACTIONSET *pts)
 *
 * Sorts a OPENSERVOTRANSACTIONSET into order based on function and OpenServo
 * register address.
 */
static int_t OSI_SortTransactionSet(OPENSERVO *posid, OPENSERVOTRANSACTIONSET *pts)
{

/*
 * NOTE: Uses a classic shell-metzner sort, rather than relying on the presence of
 *       the "qsort" function for mircocontroller support.
 */
   if(pts->m_nSort==0 && pts->m_n>0)
   {
      if(pts->m_n>1)
      {
         int_t i,j,k,l,m;
         m=pts->m_n;
         do // once and then while j>k
         {
            m=m/2;
            if(m>0)
            {
               k=pts->m_n-m;
               j=1;
               do // once and then while j<=k
               {
                  i=j;
                  do // once and then while i>1
                  {
                     l=i+m;

/*
 * NOTE: The items are sorted into order first on the priority defined by m_nrwpl,
 *       then on whether the operation is a read or a write (also defined by m_nrwpl,
 *       writes first, then reads) and then finally on the OpenServo bank and address
 *       of the item.
 */
                     { // TODO: Prove new scope does not interfere with break;
                        int_t c=(int_t)(pts->m_p[i-1]->m_nrwp&1)-(int_t)(pts->m_p[l-1]->m_nrwp&1);
                        if(c==0)
                        {
                           c=(int_t)(pts->m_p[i-1]->m_nrwp&0xFE)-(int_t)(pts->m_p[l-1]->m_nrwp&&0xFE);
                           if(c==0)
                           {
                              const OPENSERVOADDRESSMAP *pMapi=RegisterIDtoAddressMap(posid,pts->m_p[i-1]->m_nID);
                              const OPENSERVOADDRESSMAP *pMapl=RegisterIDtoAddressMap(posid,pts->m_p[l-1]->m_nID);
                              c=pMapi->m_nBank-pMapl->m_nBank; // TODO: Non bank first....
                              if(c==0)
                              {
                                 c=pMapi->m_nAddress-pMapl->m_nAddress;
                              }
                           }
                        }
                        if(c<0)
                        {
                           OPENSERVOTRANSACTIONSETITEM *tmp=pts->m_p[i-1];
                           pts->m_p[i-1]=pts->m_p[l-1];
                           pts->m_p[l-1]=tmp;
                           i=i-m;
                        } else
                        {
                           break;
                        }
                     }
                  } while (i >= 1);
                  j++;
               } while (j <= k);
            } else
            {
               break;
            }
         } while (j > k);
      }
      pts->m_nSort=1;
   }
   return pts->m_nSort;
}

/*******************************************************************************
 *
 * int_t OSI_Open(OPENSERVO *posid, byte_t nrw)
 *
 * Synopsis
 *
 * Starts or restarts transactions for the OpenServo on the I2C bus. This function
 * provides the starting point to allow continuous transactions to be made with a
 * specific servo on a particular I2C bus.
 *
 * TODO: Better description? More?
 *
 * NOTE: If I2C communications have already been started for the servo, then they
 *       are "restarted". Any outstanding acknowledgement for a previous read is
 *       sent (a negative acknowledgement is sent as it the last read was the last).
 *
 * Explanation of the rationales behind using OSI_Open... OSI_Close are found in
 * the overview at the start of this module.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that described the OpenServo for which
 *           I2C bus communications are to be started.
 *
 * BYTE nrw
 *         o TODO
 *
 * Return
 *
 * int     o The function returns OSI_SUCCESS (0) if an I2C transactions is successfully
 *           started/restarted on the bus to which the OpenServo is connected. Any other
 *           return value indicates that an error has occurred.
 *
 *           NOTE: If an error has occurred, then the state of the I2C bus is not known: for
 *                 example it could be that the application already has another transaction
 *                 in progress in the bus.
 */
int_t OSI_Open(OPENSERVO *posid, byte_t nrw)
{
/*
 * If there is already a transaction in progress and it was a read and the acknowledgement
 * is still outstanding, then send a negative acknowledgement to indicate that the last byte
 * that was retrieved is in fact the last byte being retrieved in the current transaction
 * (otherwise odd behaviour of the servo may ensue!).
 */
   if((posid->m_nIOStatus&0x85)==0x85 && I2CM_PutAck(posid->m_nBus,FALSE)!=I2CM_SUCCESS)
   {
      OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
   } else
   {

/*
 * Start or restart I2C transactions for the OpenServo
 *
 * NOTE: A call to I2CM_StartTransaction should fail if a transaction is already in progress,
 *       it is assumed that if the transaction is still marked as active for this OpenServo,
 *       that this is indeed the case.
 */
      if(posid->m_nIOStatus==0 && I2CM_StartTransaction(posid->m_nBus)!=I2CM_SUCCESS ||
         posid->m_nIOStatus!=0 && I2CM_RestartTransaction(posid->m_nBus)!=I2CM_SUCCESS)
      {
         OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
      } else
      {

/*
 * Setup the OpenServo's address
 */
         bool_t bAck;
         posid->m_nIOStatus=0x80;
         if(I2CM_PutByteGetAck((int_t)posid->m_nBus,(byte_t)((posid->m_nAddress<<1)|(!!nrw)),&bAck)!=I2CM_SUCCESS) // TODO, what's wrong? || !bAck)
         {
            OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
         } else
         {
            OSI_SetLastError(OSI_SUCCESS,NULL);
         }
      }
   }

/*
 * End the I2C transactions for this specific OpenServo, if there has been an error
 */
   if(OSI_GetLastError()!=OSI_SUCCESS && posid->m_nIOStatus!=0)
   {
      I2CM_StopTransaction(posid->m_nBus);
      posid->m_nIOStatus=0;
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_Close(OPENSERVO *posid)
 *
 * Synopsis
 *
 * Stops the current I2C transaction for the OpenServo on the I2C bus.
 *
 * NOTE: Any outstanding acknowledgement for a previous read is sent (a negative
 *       acknowledgement is sent as it the last read was the last).
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that described the OpenServo for which
 *           I2C bus communications are to be stopped.
 *
 *           NOTE: A transaction must, of course, have been started for the servo.
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if an I2C transaction is successfully
 *           stopped. Any other return value indicates that an error has occurred.
 *
 *           NOTE: If an error has occurred, then the state of the I2C bus is not known.
 */
int_t OSI_Close(OPENSERVO *posid)
{
/*
 * No transaction running for the servo means there has been an error.
 */
   if((posid->m_nIOStatus&0x80)==0)
   {
      OSI_SetLastError(OSI_ERR_NOI2CTRANSACTION,NULL);
   } else
   {

/*
 * If there is already a transaction in progress and it was a read and the acknowledgement
 * is still outstanding, then send a negative acknowledgement to indicate that the last byte
 * that was retrieved is in fact the last byte being retrieved in the current transaction
 * (otherwise odd behaviour of the servo may ensue!).
 */
      if((posid->m_nIOStatus&0x04)!=0 && I2CM_PutAck(posid->m_nBus,FALSE)!=I2CM_SUCCESS)
      {
         OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
         I2CM_StopTransaction(posid->m_nBus); /* Still need to stop */
      } else
      {
         if(I2CM_StopTransaction(posid->m_nBus)!=I2CM_SUCCESS)
         {
            OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
         } else
         {
            OSI_SetLastError(OSI_SUCCESS,NULL);
         }
      }
   }
   posid->m_nIOStatus=0;
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_ReadRegister(OPENSERVO *posid, OSI_REGID nID, void *pBuffer, int_t nSize)
 *
 * Synopsis
 *
 * Reads the specified register in to the buffer provided.
 *
 * NOTE: The buffer must be of sufficient size to store the register value being read.
 *
 *       The size specified for the buffer must match that of the register being read.
 *
 *       Endian conversion is performed, where needed, on relevant platforms (e.g.
 *       16-bit registers).
 *
 *       This function can be called irrespective of whether OSI_Open has been called
 *       for the servo. If a transaction has not been started, then one will be started
 *       for the duration of the call.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that described the servo from which
 *           the register is to be read.
 *
 * OSI_REGID nID
 *         o Specifies the ID of the register to be read.
 *
 * void *pBuffer
 *         o Points to the location in which the value read is to be stored.
 *
 *           NOTE: The location pointed to must be large enough to store the
 *                 register being read.
 *
 * int_t nSize
 *         o Specifies the size, in bytes, of the register to be read.
 *
 *           NOTE: This must be the size, of the register being read, as defined
 *                 by the OpenServo Interface Address mapping- if it is not, an
 *                 error will be returned and no data will be read.
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if the specified register has been
 *           successfully read from the OpenServo. Any other return value indicates
 *           that an error has occurred.
 *
 *           NOTE: Even if an error has occurred, some or all of the buffer pointed
 *                 to by pBuffer may have been overwritten, 
 */
int_t OSI_ReadRegister(OPENSERVO *posid, OSI_REGID nID, void *pBuffer, int_t nSize)
{
   bool_t bAck;
   int_t i,j;
   
/*
 * Get the starting access state (transaction started, or not)
 */
   byte_t nss=posid->m_nIOStatus;

/*
 * Get the address map to use.
 */
   const OPENSERVOADDRESSMAP *pMap=RegisterIDtoAddressMap(posid,nID);
   if(pMap==NULL || pMap->m_nSize!=nSize)
   {
      OSI_SetLastError(OSI_ERR_BADARGS,NULL);
   } else
   {

/*
 * If the register requested does not exist in this servo firmware/hardware version, then
 * set the memory location to 0 and return an OSI_ERR_NOREGISTER error code.
 *
 * NOTE: The caller can treat this as non-fatal, if desired, in terms of servo transaction,
 *       but may need to handle the absence of the data requested.
 */
      if(pMap->m_nAddress==0xFF)
      {
         memset(pBuffer,0x00,nSize);
         OSI_SetLastError(OSI_ERR_NOREGISTER,NULL); /* NOTE: Call can treat this as non-fatal if desired */
      } else
      {

/*
 * If an I2C transaction is not currently running for the servo, start one
 */
         if(nss==0 && OSI_Open(posid,0)!=OSI_SUCCESS)
         {
            OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
         } else
         {

/*
 * Bank switch handling...
 *
 * If the register to be read is in a different bank than the currently selected
 * bank, then the bank selection needs to be changed.
 */
            if(pMap->m_nBank!=0xFF && pMap->m_nBank!=posid->m_nBankSelect &&
               OSI_WriteRegister(posid,OSI_REGID_BANKSELECT,(const void *)(&pMap->m_nBank),sizeof(pMap->m_nBank))!=OSI_SUCCESS)
            {
               posid->m_nBankSelect=0xFF; /* NOTE: Set to 0xFF as we do not know if the bank selection register was written or not */
               OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
            } else
            {
               byte_t cs=posid->m_nIOStatus&0x3;
               if(pMap->m_nBank!=0xFF)
               {
                  posid->m_nBankSelect=pMap->m_nBank;
               }

/*
 * Check for change of state requirement (e.g. from write to read). Any change of state
 * requires the existing transaction to be restarted.
 *
 * NOTE: When reading from the servo, it is more efficient to read and dump up to three
 *       bytes, than change the address to skip them, if a suitable read is already in
 *       progress... every byte of bandwidth saved on the I2C bus can count!
 *
 *       Register addresses are in the range [0x00 to 0xFF], this fact is used in the
 *       software: for example 0xFF is used to indicate that the current address latched
 *       in the servo is not known.
 */
               if((cs!=0 && cs!=1 || // TODO: Check and sanitise this lot
                  cs==1 && (pMap->m_nAddress!=posid->m_nCurRegAddress && 
                           (pMap->m_nAddress>posid->m_nCurRegAddress && pMap->m_nAddress-posid->m_nCurRegAddress>3 ||
                            pMap->m_nAddress<posid->m_nCurRegAddress && (pMap->m_nAddress+0xFF)-posid->m_nCurRegAddress>3))) &&
                  OSI_Open(posid,0)!=OSI_SUCCESS)
               { /* NOTE: OSI_Open has already set the error condition */
               } else
               {

/*
 * If no transaction type is running, then send servo register address sequence
 */
                  cs=posid->m_nIOStatus&0x3; /* Update in case OSI_Open was called */
                  if(cs==0)
                  {
                     posid->m_nIOStatus|=0x2; /* NOTE: Entering write mode, failsafe */
                     if(I2CM_PutByteGetAck(posid->m_nBus,pMap->m_nAddress,&bAck)!=I2CM_SUCCESS || !bAck ||
                        OSI_Open(posid,1)!=OSI_SUCCESS)
                     {
                        posid->m_nCurRegAddress=0xFF; /* No longer know that it is correct */
                        OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
                     } else
                     {
                        posid->m_nCurRegAddress=pMap->m_nAddress;
                        posid->m_nIOStatus|=0x01; /* NOTE: Now in read mode */
                     }
                  }

/*
 * If there has been no error, then read and dump any intervening bytes
 *
 * NOTE: Any previous acknowledgement is sent now, the last acknowledgement is held
 *       back, so that a negative acknowledgement can be sent for the last byte in
 *       a read sequence when the current I2C transaction is stopped/restarted.
 */
                  while(pMap->m_nAddress!=posid->m_nCurRegAddress && OSI_GetLastError()==OSI_SUCCESS)
                  {
                     if((posid->m_nIOStatus&0x04)!=05 && I2CM_PutAck(posid->m_nBus,TRUE)!=I2CM_SUCCESS)
                     {
                        OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
                     } else
                     {
                        if(I2CM_GetByte(posid->m_nBus,(byte_t *)&bAck/* Used as a dump */)!=I2CM_SUCCESS)
                        {
                           posid->m_nCurRegAddress=0xFF; /* No longer know that it is correct */
                           OSI_SetLastError(OSI_ERR_I2CREADERROR,NULL);
                        } else
                        {
                           posid->m_nIOStatus|=0x04; /* NOTE: Acknowledgement for byte read is now outstanding */
                           posid->m_nCurRegAddress++;
                        }
                     }
                  }
                  if(OSI_GetLastError()==OSI_SUCCESS)
                  {
                     posid->m_nCurRegAddress&=0x7F;
                  }

/*
 * If there has been no error, then actually read the data bytes requested...
 *
 * NOTE: Any previous acknowledgement is sent now, the last acknowledgement is held
 *       back, so that a negative acknowledgement can be sent for the last byte in
 *       a read sequence when the current I2C transaction is stopped/restarted.
 *
 * NOTE: Currently the following code crudely assumes that an even number of bytes means
 *       that endian conversion of 16-bit registers is required and an odd number of bytes
 *       means that the buffer is to transferred as is.
 */
                  if((pMap->m_nSize&1)==0)
                  {
                     j=1;
                  } else
                  {
                     j=0;
                  }
                  for(i=0; i<pMap->m_nSize && OSI_GetLastError()==OSI_SUCCESS; i++)
                  {
                     if((posid->m_nIOStatus&0x04)!=0 && I2CM_PutAck(posid->m_nBus,TRUE)!=I2CM_SUCCESS)
                     {
                        OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
                     } else
                     {
                        if(I2CM_GetByte(posid->m_nBus,&((byte_t *)pBuffer)[i+j])!=I2CM_SUCCESS)
                        {
                           posid->m_nCurRegAddress=0xFF; /* No longer know that it is correct */
                           OSI_SetLastError(OSI_ERR_I2CREADERROR,NULL);
                        } else
                        {
                           posid->m_nIOStatus|=0x04; /* NOTE: Acknowledgement for byte read is now outstanding */
                           posid->m_nCurRegAddress++;
                        }
                     }
                     j=-j;
                  }

/*
 * If there has been no error, then ensure that the "current addresses" stored in the open
 * servo has wrapped, if required. Also, updated the stored bank selection register if the
 * caller read back the currently selected bank.
 */
                  if(OSI_GetLastError()==OSI_SUCCESS)
                  {
                     posid->m_nCurRegAddress&=0x7F;
                     if(nID==OSI_REGID_BANKSELECT)
                     {
                        posid->m_nBankSelect=*(byte_t *)pBuffer;
                     }
                  }
               }
            }

/*
 * If this function started the current transaction, end it
 */
            if(nss==0)
            {
               OSI_Close(posid); /* NOTE: Sets the OSI error if there is a problem */
            }
         }
      }
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_WriteRegister(OPENSERVO *posid, OSI_REGID nID, const void *pBuffer, int_t nSize)
 *
 * Synopsis
 *
 * Writes the specified register from the buffer provided.
 *
 * NOTE: The buffer must be of sufficient size to provide data for the register being
 *       written.
 *
 *       The size specified for the buffer must match that of the register being written.
 *
 *       Endian conversion is performed, where needed, on relevant platforms (e.g.
 *       16-bit registers).
 *
 *       This function can be called irrespective of whether OSI_Open has been called
 *       for the servo. If a transaction has not been started, then one will be started
 *       for the duration of the call.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that described the servo of which
 *           the register value is to be written.
 *
 * OSI_REGID nID
 *         o Specifies the ID of the register to be written.
 *
 * const void *pBuffer
 *         o Points to the location from which the value written is to be retrieved.
 *
 *           NOTE: The location pointed to must be large enough to provide the
 *                 data for the register being written.
 *
 * int_t nSize
 *         o Specifies the size, in bytes, of the register to be written.
 *
 *           NOTE: This must be the size, of the register being written, as defined
 *                 by the OpenServo Interface Address mapping- if it is not, an
 *                 error will be returned and no data will be written.
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if the specified register has been
 *           successfully written to the OpenServo. Any other return value indicates
 *           that an error has occurred.
 *
 *           NOTE: Even if an error has occurred, some or all of the buffer pointed
 *                 to by pBuffer may have been transferred to the servo, 
 */
int_t OSI_WriteRegister(OPENSERVO *posid, OSI_REGID nID, const void *pBuffer, int_t nSize)
{
   bool_t bAck;
   int_t i,j;

/*
 * Get the starting access state (transaction started, or not)
 */
   byte_t nss=posid->m_nIOStatus;

/*
 * Get the address map to use.
 */
   const OPENSERVOADDRESSMAP *pMap=RegisterIDtoAddressMap(posid,nID);
   if(pMap==NULL || pMap->m_nSize!=nSize)
   {
      OSI_SetLastError(OSI_ERR_BADARGS,NULL);
   } else
   {

/*
 * If the register requested does not exist in this servo firmware/hardware version, then
 * return an OSI_ERR_NOREGISTER error code.
 *
 * NOTE: The caller can treat this as non-fatal, if desired, in terms of servo transactions,
 *       but may need to handle the inability to write the register.
 */
      if(pMap->m_nAddress==0xFF)
      {
         OSI_SetLastError(OSI_ERR_NOREGISTER,NULL); /* NOTE: Call can treat this as non-fatal if desired */
      } else
      {

/*
 * If an I2C transaction is not currently running, start one
 */
         if(nss==0 && OSI_Open(posid,0)!=OSI_SUCCESS)
         {
            OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
         } else
         {

/*
 * Bank switch handling...
 *
 * If the register to be written is in a different bank than the currently selected
 * bank, then the bank selection needs to be changed.
 */
            if(pMap->m_nBank!=0xFF && pMap->m_nBank!=posid->m_nBankSelect &&
               OSI_WriteRegister(posid,OSI_REGID_BANKSELECT,&pMap->m_nBank,sizeof(pMap->m_nBank))!=OSI_SUCCESS)
            {
               posid->m_nBankSelect=0xFF; /* NOTE: Set to 0xFF as we do not know if the register was, written or not*/
               OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
            } else
            {
               int_t cs=posid->m_nIOStatus&0x3;
               if(pMap->m_nBank!=0xFF)
               {
                  posid->m_nBankSelect=pMap->m_nBank;
               }

/*
 * Check for change of state requirement (e.g. from read to write). Any change of state
 * require the existing transaction to be restarted.
 *
 * NOTE: There are no optimisations available for register writes.
 */
               if((cs!=0 && cs!=2 || cs==2 && pMap->m_nAddress!=posid->m_nCurRegAddress) &&
                  OSI_Open(posid,0)!=OSI_SUCCESS)
               { /* NOTE: OSI_Open has already set the error condition */
               } else
               {

/*
 * If no transaction type is running, then send servo register address sequence
 */
                  cs=posid->m_nIOStatus&0x3; /* Update in case OSI_Open was called */
                  if(cs==0)
                  {
                     if(I2CM_PutByteGetAck(posid->m_nBus,pMap->m_nAddress,&bAck)!=I2CM_SUCCESS || !bAck)
                     {
                        posid->m_nCurRegAddress=0xFF; /* No longer know that it is correct */
                        OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
                     } else
                     {
                        posid->m_nCurRegAddress=pMap->m_nAddress;
                        posid->m_nIOStatus|=0x2; /* Now in write mode */
                     }
                  }

/*
 * If there has been no error, then write the data bytes requested...
 *
 * NOTE: The following code currently crudely assumes that an even number of bytes means
 *       that endian conversion of 16-bit registers is required and an odd number of bytes
 *       means that the buffer is to transferred as is.
 */
                  if((pMap->m_nSize&1)==0)
                  {
                     j=1;
                  } else
                  {
                     j=0;
                  }
                  for(i=0; i<pMap->m_nSize && OSI_GetLastError()==OSI_SUCCESS; i++)
                  {
                     if(I2CM_PutByteGetAck(posid->m_nBus,((byte_t *)pBuffer)[i+j],&bAck)!=I2CM_SUCCESS || !bAck)
                     {
                        posid->m_nCurRegAddress=0xFF; /* No longer know that it is correct */
                        OSI_SetLastError(OSI_ERR_I2CWRITEERROR,NULL);
                     } else
                     {
                        posid->m_nCurRegAddress++;
                     }
                     j=-j;
                  }

/*
 * If there has been no error, then ensure that the "current addresses" stored in the open
 * servo has wrapped, if required. Also, updated the stored bank selection register if the
 * caller changed the currently selected bank.
 */
                  if(OSI_GetLastError()==OSI_SUCCESS)
                  {
                     posid->m_nCurRegAddress&=0x7F;
                     if(nID==OSI_REGID_BANKSELECT)
                     {
                        posid->m_nBankSelect=*(byte_t *)pBuffer;
                     }
                  }
               }
            }

/*
 * If this function started the current transaction, end it
 */
            if(nss==0)
            {
               OSI_Close(posid); /* NOTE: Sets the OSI error if there is a problem */
            }
         }
      }
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_Command(OPENSERVO *posid, OSI_CMDID nID)
 *
 * Synopsis
 *
 * Sends the specified command to a servo
 *
 *       This function can be called irrespective of whether OSI_Open has been called
 *       for the servo. If a transaction has not been started, then one will be started
 *       for the duration of the call.
 *
 * Parameters
 *
 * OPENSERVO *posid
 *         o Points to the OPENSERVO structure that described the servo to which the
 *           command is to be sent.
 *
 * OSI_CMDID nID
 *         o Specifies the ID of the command to be sent.
 *
 * Return
 *
 * int_t   o The function returns OSI_SUCCESS (0) if the specified command has been
 *           successfully sent to the OpenServo. Any other return value indicates
 *           that an error has occurred.
 */
int_t OSI_Command(OPENSERVO *posid, OSI_CMDID nID)
{
   bool_t bAck;

/*
 * Get the starting access state (transaction started, or not)
 */
   byte_t nss=posid->m_nIOStatus;

/*
 * Get the address map to use.
 */
   const OPENSERVOCOMMANDMAP *pMap=CommandIDtoCommandMap(posid,nID);
   if(pMap==NULL)
   {
      OSI_SetLastError(OSI_ERR_BADARGS,NULL);
   } else
   {

/*
 * If an I2C transaction is not currently running, start one
 */
      if(nss==0 && OSI_Open(posid,0)!=OSI_SUCCESS)
      {
         OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
      } else
      {

/*
 * Check for change of state requirement (e.g. from read to command). Any change of state
 * require the existing transaction to be restarted.
 *
 * NOTE: There are no optimisations available for commands.
 */
         int_t cs=posid->m_nIOStatus&0x3;
         if(cs!=0 && cs!=4 && OSI_Open(posid,0)!=OSI_SUCCESS)
         { /* NOTE: OSI_Open has already set the error condition */
         } else
         {

/*
 * Send the servo the command
 */
            if(I2CM_PutByteGetAck(posid->m_nBus,(byte_t)(pMap->m_nAddress|0x80),&bAck)!=I2CM_SUCCESS || !bAck)
            {
               OSI_SetLastError(OSI_ERR_I2CMTRANSACTIONERROR,NULL);
            } else
            {
               posid->m_nIOStatus|=0x3;

/*
 * TODO: Required: Special handling for reset, etc?
 */
            }
         }

/*
 * If this function started the current transaction, end it
 */
         if(nss==0)
         {
            OSI_Close(posid); /* NOTE: Sets the OSI error if there is a problem */
         }
      }
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *******************************************************************************
 *******************************************************************************
 *
 * "Advanced interace functions" (see osi.h)...
 */
#if !defined(OSI_VERYLEAN) && !defined(OSI_LEAN)

/*******************************************************************************
 *
 * int OSI_GetPID(OPENSERVO *posid, OPENSERVOPIDDATA *pPIDData)
 *
 * NOTE: To read the PID registers, the code does not assume anything about the
 *       addressing of the registers (i.e. that they may be consecutive and/or
 *       in a particular order), instead it reads them individually. This means
 *       that there are more transactions on the I2C bus than there would be
 *       with an "optimised read": however, reading the PID registers is unlikely
 *       to be something that the master would do on a regular basis in an
 *       "operational setting".
 *
 *       If an error occurs, the target structure (*pPIDData) may have been partially
 *       overwritten: but the contents will not have been endian converted, nor will
 *       they have been transferred to the locally stored values (in *posid).
 *
 * TODO: Improve "optimised".
 */
int OSI_GetPID(OPENSERVO *posid, OPENSERVOPIDDATA *pPIDData)
{
   if(OSI_ReadRegister(posid,OSI_REGID_PID_DEADBAND,&pPIDData->m_Deadband,sizeof(pPIDData->m_Deadband))==OSI_SUCCESS &&
      OSI_ReadRegister(posid,OSI_REGID_PID_PGAIN,&pPIDData->m_PGain,sizeof(pPIDData->m_PGain))==OSI_SUCCESS &&
      OSI_ReadRegister(posid,OSI_REGID_PID_DGAIN,&pPIDData->m_DGain,sizeof(pPIDData->m_DGain))==OSI_SUCCESS)
   {
      OSI_ReadRegister(posid,OSI_REGID_PID_IGAIN,&pPIDData->m_IGain,sizeof(pPIDData->m_IGain));
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int OSI_SetPID(OPENSERVO *posid, const OPENSERVOPIDDATA *pPIDData)
 *
 * NOTE: To write the PID registers, the code does not assume anything about the
 *       addressing of the registers (i.e. that they may be consecutive and/or
 *       in a particular order), instead it writes them individually. This means
 *       that there are more transactions on the I2C bus than there would be
 *       with an "optimised read": however, writing of the PID registers is unlikely
 *       to be something that the master would do on a regular basis in an
 *       "operational setting".
 *
 *       If an error occurs, the source structure (*pPIDData) may have been partially
 *       delivered to the OpenServo and only those values that have been successfully
 *       delivered will have been updated locally (*posid).
 *
 * TODO: Improve "optimised".
 */
int OSI_SetPID(OPENSERVO *posid, const OPENSERVOPIDDATA *pPIDData)
{
   if(OSI_WriteRegister(posid,OSI_REGID_PID_DEADBAND,&pPIDData->m_Deadband,sizeof(pPIDData->m_Deadband))==OSI_SUCCESS &&
      OSI_WriteRegister(posid,OSI_REGID_PID_PGAIN,&pPIDData->m_PGain,sizeof(pPIDData->m_PGain))==OSI_SUCCESS &&
      OSI_WriteRegister(posid,OSI_REGID_PID_DGAIN,&pPIDData->m_DGain,sizeof(pPIDData->m_DGain))==OSI_SUCCESS)
   {
      OSI_WriteRegister(posid,OSI_REGID_PID_IGAIN,&pPIDData->m_IGain,sizeof(pPIDData->m_IGain));
   }
   return OSI_GetLastError();
}

/*******************************************************************************
 *
 * int_t OSI_GetBatteryVoltageAsV(OPENSERVO *posid, uint16_t *pnVoltage)
 *
 * Return the current battery voltage in real units (V*100).
 *
 * NOTE: Relies on correct servo hardware/software setup/configuration.
 *
 *       Vbatt= Vraw*Vreg/1023*(R3R4+R4R6)/R4R6)   Eqn. 1
 *
 *       Eqn. 1 may be rearranged to suit microcontroller platforms when
 *       dealing with limited sized integers. This function deals with
 *       integers and returns an integer for that reason.
 */
int_t OSI_GetBatteryVoltageAsV(OPENSERVO *posid, uint16_t *pnVoltage)
{
   if(OSI_GetBatteryVoltage(posid,pnVoltage)==OSI_SUCCESS)
   {
      /* TODO: Check this for "accuracy" and microcontroller support */
      *pnVoltage=*pnVoltage*posid->m_conf.m_VREG*(posid->m_conf.m_R3R4+posid->m_conf.m_R4R6)/posid->m_conf.m_R4R6/1023;
   }
   return OSI_GetLastError();
}

#endif // !defined(OSI_VERYLEAN) && !defined(OSI_LEAN)

/*******************************************************************************
 *******************************************************************************
 ******************************************************************************/
