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
 * Provides a software interface between I2C (I²C) Manager and the OpenServo.
 *
 * This is a preliminary version: WIP, with probably a great deal of room for
 * improvement.
 *
 * Written in C (not C++) as it might be used from a C only microcontroller
 * platform ("Dynamic C" on a "RabbitCore", for example), another language
 * (Visual Basic, Fotran, etc).
 *
 * There may be considerations over access to the I2C bus, if it is shared with
 * other parts of an application.
 */
#ifndef __OSI_H__
#define __OSI_H__

/*******************************************************************************
 *
 * Choose I2C Manager: still the same include file for a single independant I2C
 * layer, what you get depends on how you compile and link.
 */
#include "../i2cm/i2c_manager.h"

/*******************************************************************************
 *
 * C++ linkage conventions
 *
 * Routines specified by this file need C linkage under C++
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Preferred packing (memory vs performance)
 */
#pragma pack(push)
#pragma pack(1)

/*******************************************************************************
 *
 * Define IDs for known OpenServo "device types"
 */
typedef enum
{
   OSI_DTYPE_UNKNOWN=-1, // OpenServo device is not known, not set, or the device is not an OpenServo
   OSI_DTYPE_21_11=1,    // An v2.1 OpenServo board running version 1.1 firmware
   OSI_DTYPE_21_BANK,    // An v2.1 OpenServo board running the bank switching firmware (no BEMF)
   OSI_DTYPE_V3_11,      // An V3 (OSxV2) OpenServo board running the modified version 1.1 firmware (no BEMF)
   OSI_DTYPE_V3_BANK,    // An V3 (OSxV2) OpenServo board running the bank switching firmware
} OSI_DTYPE;

/*******************************************************************************
 *
 * Define "OpenServo Interface Addresses"- these will be indirectly mapped to
 * actual OpenServo hardware/firmware register addresses (with handling for
 * bank switch, redirection or non-existence).
 *
 * NOTE: The values assigned here are important, they define the order of the
 *       elements in the mapping arrays.
 *
 *       The order listed will be the most efficient access order for the latest
 *       firmware.
 */
typedef enum
{

#define REG_PWM_DIRA                0x0E
#define REG_PWM_DIRB                0x0F

   OSI_REGID_DEVICETYPE=0,          // OpenServo device type
   OSI_REGID_DEVICESUBTYPE,         // OpenServo device subtype
   OSI_REGID_VERSIONMAJOR,          // Major version number of OpenServo software
   OSI_REGID_VERSIONMINOR,          // Minor version number of OpenServo software
   OSI_REGID_FLAGS,                 // Flags word
   OSI_REGID_TIMER,                 // ADC Timer
   OSI_REGID_POSITION,              // Current measured servo position
   OSI_REGID_VELOCITY,              // Current measured servo velocity
   OSI_REGID_CURRENT,               // Current measured servo current
   OSI_REGID_PWM_CW,                // PWM clockwise value
   OSI_REGID_PWM_CCW,               // PWM counter-clockwise
   OSI_REGID_BATTVOLTAGE,           // Last measured battery voltage
   OSI_REGID_CURVE_BUFFER,          // Remaining curve buffer space
   OSI_REGID_TEMPERATURE,           // Last measured "on-board temperature"
   OSI_REGID_SEEK,                  // Seek position
   OSI_REGID_SEEKVELOCITY,          // Seek velocity
   OSI_REGID_CURVE_DELTA,           // Curve Time delta
   OSI_REGID_CURVE_POSITION,        // Curve position
   OSI_REGID_CURVE_IN_VELOCITY,     // Curve in velocity
   OSI_REGID_CURVE_OUT_VELOCITY,    // Curve out velocity
   OSI_REGID_CURVE_RESERVED,        // Reserved curve data
//   OSI_REGID_GENERAL_CALL_GROUP_START, // General call: Group start
   OSI_REGID_BANKSELECT,            // Bank Selection register

//   OSI_REGID_ALERT_STATUS,          // Alert status
   OSI_REGID_BACKEMF,               // Last measured back-EMF

   OSI_REGID_TWI_ADDRESS,           // TWI address of servo
   OSI_REGID_PID_DEADBAND,          // Programmable PID deadband value
   OSI_REGID_PID_PGAIN,             // PID proportional gain
   OSI_REGID_PID_DGAIN,             // PID derivative gain
   OSI_REGID_PID_IGAIN,             // PID integral gain
   OSI_REGID_MINSEEK,               // Minimum seek position
   OSI_REGID_MAXSEEK,               // Maximum seek position
   OSI_REGID_REVERSESEEK,           // Reverse seek sense
   OSI_REGID_PWM_FREQ_DIVIDER,      // PWM frequency divider
//   OSI_REGID_PWM_MAX,
   OSI_REGID_ALERT_CURR_MAXLIMIT,   // Current limit for alerts and throttling
   OSI_REGID_ALERT_VOLT_MAXLIMIT,   // Max battery voltage limit
   OSI_REGID_ALERT_VOLT_MINLIMIT,   // Min battery voltage limit
//   OSI_REGID_ALERT_TEMP_MAXLIMIT,   // Max temperature limit
//   OSI_REGID_GENERAL_CALL_GROUP,
//   OSI_REGID_REG_BRAKE_STRENGTH,
//   OSI_REGID_EMF_COLLAPSE_DELAY,
//   OSI_REGID_EMF_CHARGE_TIME,

/*
 * To keep the code simpler, the "command addresses" are placed with the "register
 * addresses" for mapping purposes, OSI_ID_SEPARATOR is used to separate them.
 *
 * NOTE: OSI_ID_SEPARATOR takes the next value following the last OSI_REG_ID, which
 *       are sequential. The command IDs that follow are restarted at a large value
 *       so that the numerical values can be retained in case they are used/stored
 *       by the user.
 */
   OSI_ID_SEPARATOR,
   
/*
 * Command "addresses"
 */
   OSI_CMDID_RESET=8192,            // Reset microcontroller
   OSI_CMDID_CHECKED_TXN,           // Read/Write registers with simple checksum
   OSI_CMDID_PWM_ENABLE,            // Enable PWM to motors
   OSI_CMDID_PWM_DISABLE,           // Disable PWM to servo motors
   OSI_CMDID_WRITE_ENABLE,          // Enable write of read/write protected registers
   OSI_CMDID_WRITE_DISABLE,         // Disable write of read/write protected registers
   OSI_CMDID_REGISTERS_SAVE,        // Save read/write protected registers fo EEPROM
   OSI_CMDID_REGISTERS_RESTORE,     // Restore read/write protected registers from EEPROM
   OSI_CMDID_REGISTERS_DEFAULT,     // Restore read/write protected registers to defaults
   OSI_CMDID_EEPROM_ERASE,          // Erase the AVR EEPROM 
   OSI_CMDID_BATTVOLTAGE_RESAMPLE,  // Request a new Battery Voltage sample 
   OSI_CMDID_CURVE_MOTION_ENABLE,   // Enable curve based motion 
   OSI_CMDID_CURVE_MOTION_DISABLE,  // Disable curve based motion 
   OSI_CMDID_CURVE_MOTION_RESET,    // Clear the curve buffer
   OSI_CMDID_CURVE_MOTION_APPEND,   // Append a new curve 
   OSI_CMDID_GCALL_ENABLE,          // Enable General Call
   OSI_CMDID_GCALL_DISABLE,         // Disable General Call
   OSI_CMDID_GCALL_START_WAIT,      // Store the new position and wait for the start_move command
   OSI_CMDID_GCALL_START_MOVE,      // Start the syncro move

   OSI_ID_END,                      // Marks the end

} OSI_REGID, OSI_CMDID;

/*******************************************************************************
 *
 * Register access management
 *
 * Since August 2007 the implementation for accessing registers in the OpenServo
 * is partitioned
 *
 *   o 0x00 to 0x3f   o These are the permanently mapped registers, split into
 *                      three:
 *
 *                         0x00 to 0x1f read-only
 *                         0x20 to 0x2f read-write
 *                         0x30 to 0x3f write protected
 *
 *   o 0x40 to 0x7f   o These registers are bank mapped
 *
 *   o 0x80 to 0xff   o These are the command addresses
 *
 * The banked address range also holds a bank that is "redirected" (i.e. allowing
 * you to place the registers you want in the order you want).
 *
 * For further details, consult http://openservo.org 
 *
 * To simplify access to the registers in the OpenServo, and to provide backwards
 * compatibility the OpenServo Interface "flattens" the address range and uses a
 * form of indirect addressing. This is driven through an addressing structure,
 * which is probably slightly wasteful of memory, but reliable and simple to keep
 * up to date
 *
 * typedef struct OPENSERVOADDRESSMAP { ... } OPENSERVOADDRESSMAP, OPENSERVOCOMMANDMAP
 *
 * A structure which defines the "true" address of an OpenServo register.
 *
 * Members
 *
 * OSI_REGID m_nID
 *         o The "OpenServo Interface Software" "address" of the register.
 *
 *           TODO: Allow this to be compiled out for release builds...
 *
 * int_t m_nSize
 *         o The size of the register, e.g. 1, 2, etc. For example the ADC Timer
 *           register is 2 bytes (a word), at addresses 0x06 (high) and 0x07 (low)
 *           in the original OpenServo firmware on the 2.1 boards.
 *
 * byte_t m_nBank
 *         o Indicates the bank in the OpenServo in which the register is found.
 *
 *           NOTE: 0xFF is used to indicate that the register appears in the
 *                 non-bank mapped registers.
 *
 *                 The assumption is that bank 0xFF is unlikely to be used.
 *
 * byte_t m_nAddress
 *         o Defines the registers address in the OpenServo.
 *
 */
typedef struct OPENSERVOADDRESSMAP
{
#ifdef _DEBUG
   OSI_REGID m_nID;
#endif
   int_t m_nSize;
   byte_t m_nBank;
   byte_t m_nAddress;
} OPENSERVOADDRESSMAP, OPENSERVOCOMMANDMAP;

/*******************************************************************************
 *
 * typedef struct OPENSERVOTRANSACTIONSETITEM { ... } OPENSERVOTRANSACTIONSETITEM
 *
 * Structure defines an item to be transacted between the master and an OpenServo
 * (i.e. a register address to read, a location in which to store the data, etc.).
 * This allows read/writes from/to an OpenServo to be optimised in terms of I2C bus
 * bandwidth usage (or other): see the TODO:NAMESHERE function.
 *
 * Members
 *
 * OSI_REGID m_nID
 *         o The ID of the register to be accessed.
 *
 * byte_t m_nrpw
 *         o Indicates whether the item is being read or written (commands are implemented
 *           as "writes") and provides optional extra control over the order in which items
 *           are acted on: when a transaction set is "sorted", see the description of the
 *           m_nSort member of the OPENSERVOTRANSACTIONSET structure.
 *
 *           NOTE: Sorting a transaction set into order is optional, but may increase
 *                 performance. If the transaction set is left unsorted, then the items
 *                 will be acted in in the order listed.
 *
 *           The least significant bit of this byte is used to indicate if the item is
 *           to be read (1) or written (0). The upper seven bits of the byte form an
 *           unsigned quantity which indicate the priority (order) in which _groups_
 *           of items should be handled: but is only used if the transaction set is to
 *           be sorted.
 *
 * void *m_pBuffer
 *         o Points to the location in which the data to be written/read is
 *           retrieved from/stored to.
 *
 *           NOTE: Should be NULL if m_nID is a command ID.
 *
 * int_t_t m_nSize
 *         o Gives the size of the buffer pointed to by the m_pBuffer argument,
 *           which must be the correct size for the value identified by the
 *           m_nID argument.
 *
 *           NOTE: m_nSize should be zero if m_nID is a command ID.
 */
typedef struct OPENSERVOTRANSACTIONSETITEM
{
   OSI_REGID m_nID;
   byte_t m_nrwp;
   void *m_pBuffer;
   int_t m_nSize;
} OPENSERVOTRANSACTIONSETITEM;

/*******************************************************************************
 *
 * typedef struct OPENSERVOTRANSACTIONSET { ... } OPENSERVOTRANSACTIONSET
 *
 * Structure stores one or more OPENSERVOTRANSACTIONSETITEM structures, allowing
 * read/writes from/to an OpenServo to be optimised in terms of I2C bus bandwidth
 * usage: see the TODO:NAMESHERE function.
 *
 * Members
 *
 * int_t m_n
 *         o Indicates the number of OPENSERVOTRANSACTIONSETITEM structures
 *           referenced by this structure (see m_p).
 *
 * int_t m_nSort
 *         o Indicates whether the array pointed to by m_p has been, should, or should
 *           not be sorted into ServoRegister address order:
 *
 *              -1 o Should not be sorted.
 *               0 o Is not sorted into order, but should be.
 *               1 o Is sorted into order.
 *
 *           The sort order for transaction set is determined by the m_nrwpl member
 *           of the OPENSERVOTRANSACTIONSETITEM structures and the OpenServo address
 *           of the item. The items are sorted into order first on the priority
 *           defined by m_nrwpl, then on whether the operation is a read or a write
 *           (also defined by m_nrwpl, writes first, then reads) and then finally on
 *           the OpenServo bank and address of the item.
 *
 * OPENSERVOTRANSACTIONSETITEM **m_p
 *         o Points to an array of m_n pointers to OPENSERVOTRANSACTIONSETITEM 
 *           structures.
 */
typedef struct OPENSERVOTRANSACTIONSET
{
   int_t m_n;
   int_t m_nSort;
   OPENSERVOTRANSACTIONSETITEM **m_p;
} OPENSERVOTRANSACTIONSET;

/*******************************************************************************
 *
 * The following defines a structure that can be used to hold all the OpenServo
 * registers.
 *
 * TODO: Need to check/update these with understating as to what are, and are not,
 *       signed quantities, etc.
 *
 *       Should there be an attempt to order these in the same order as the
 *       register maps? Doing so could lead to a retrieval optimisation.
 */
typedef struct OPENSERVOPIDDATA
{
   uint8_t m_Deadband;       // Programmable PID deadband value
   int16_t m_PGain;          // PID proportional gain
   int16_t m_DGain;          // PID derivative gain
   int16_t m_IGain;          // PID integral gain
} OPENSERVOPIDDATA;
typedef struct OPENSERVOREGSET
{
   uint8_t m_DeviceType;        // Device type
   uint8_t m_DeviceSubtype;     // Device subtype
   uint8_t m_VersionMajor;      // Major version number of OpenServo software
   uint8_t m_VersionMinor;      // Minor version number of OpenServo software
   uint16_t m_Flags;            // Flags
   uint16_t m_Timer;            // ADC timer
   uint16_t m_Position;         // Current servo position
   int16_t m_Velocity;          // Current servo velocity
   uint16_t m_Current;          // Servo current
   uint16_t m_Temperature;      // Servo temperature
   uint8_t m_PWM_CW;            // PWM clockwise value
   uint8_t m_PWM_CCW;           // PWM counter-clockwise value
   uint16_t m_Seek;             // Read/Write, Seek position
   uint16_t m_SeekVelocity;     // Read/Write, Seek position velocity (speed)
   uint16_t m_BattVoltage;      // Read/Write, Battery voltage
   uint8_t m_Curve_Reserved;    // Reserved curve data
   uint8_t m_Curve_Buffer;      // Remaining curve buffer space
   int16_t m_Curve_Delta;       // Curve Time delta
   int16_t m_Curve_Position;    // Curve position
   int16_t m_Curve_InVelocity;  // Curve in velocity
   int16_t m_Curve_OutVelocity; // Curve out velocity
   byte_t m_BankSelect;         // Currently selected register bank
   int16_t m_BackEMF;           // Last measured back EMF
   byte_t m_I2CAddress;         // TWI (I2C) address of servo
   OPENSERVOPIDDATA m_PID;
   uint16_t m_PWM_FreqDivider;  // PWM frequency divider
   uint16_t m_MinSeek;          // Minimum seek position
   uint16_t m_MaxSeek;          // Maximum seek position
   uint8_t m_ReverseSeek;       // Reverse seek sense
   uint16_t m_Alert_MaxCurrent; // Maximum current (alert)
   uint16_t m_Alert_MaxVoltage; // Maximum battery voltage (alert)
   uint16_t m_Alert_MinVoltage; // Minimum battery voltage (alert)

//   byte_t m_Reserved;           // Read/Write Protected

} OPENSERVOREGSET;

/*******************************************************************************
 *
 * typedef struct OPENSERVOBOARDCONF { ... } OPENSERVOBOARDCONF
 *
 * The following defines a structure that holds some additional details about
 * an OpenServo, for example the values of certain resistors and the estimated
 * AVCC so that, for example, actually "battery voltage" can be computed, rather
 * then returning just the battery voltage register content.
 *
 * NOTE: At some point this may be loaded into a microcontroller, so the choice
 *       of types (i.e. whether to use double/float, etc.) may be important.
 *       Additionally, the use of longs/ints etc., means care must be taken
 *       over the "maths".
 */
typedef struct OPENSERVOBOARDCONF
{

/*
 * "VREG"
 *
 * uint16_t m_VREG
 *         o Stores the value of VREG (the voltage of the onboard regulator that
 *           supplies VCC, AVCC, etc.).
 *
 *           The value is the actual voltage multiplied by 1000, for example 5.0
 *           volts would be 5000 and 3.3 volts would be 3300.
 *
 *           The value is set to the designed voltage (e.g. 5.0, 3.3, etc.), or to
 *           a measured voltage (e.g. 4.973).
 */
   uint16_t m_VREG;

/*
 * "VOLTAGE"
 *
 * uint16_t m_R3R4, m_R4R6
 *         o Store the values of the two potential divider resistors, R3 and R4 (OpenServo
 *           version 2.1) or R4 and R6 (OpenServo version X3), which are used to measure
 *           the battery voltage. The values are relative, and do not need the same magnitude
 *           as the actual resistors, in fact the "integer only maths" that is implemented
 *           for microcontroller use requires that the smallest representable value is used.
 *           For example, for the default configuration of a version 2.1 OpenServo, R3 is 4.7K
 *           and R4 is 2.0K; rather than setting these values to 4700 and 2000, 47 and 20
 *           are stored.
 *
 * NOTE: The OSI_SetHardwareConf function will convert 47000 and 2000 to 47 and 20.
 */
   uint16_t m_R3R4, m_R4R6;

/*
 * System clock
 *
 * uint16_t m_nClock
 *         o Stores the clock frequency (X1) of the board, in KHz.
 */
   uint16_t m_nClock;

} OPENSERVOBOARDCONF;

/*******************************************************************************
 *
 * typedef struct OPENSERVO { ... } OPENSERVO
 * 
 * Identifies an OpenServo by I2C bus and I2C address. Stores data related to an
 * OpenServo.
 *
 * Members
 *
 * OSI_DTYPE m_nDType
 *         o Set to the OpenSero "device type".
 *
 * int_t m_nBus
 *         o Defines the I2C bus on which the servo is located, for example the
 *           first Dimax/Diolan U2C12 is bus 0, the second bus 1, etc.
 *
 * byte_t m_nAddress
 *         o Defines the address on the I2C bus of the servo (e.g. the OpenServos
 *           default to an adress of 16 (0x10)).
 *
 * const OPENSERVOADDRESSMAP m_pAddressMap
 *         o Points to an array of OPENSERVOADDRESSMAP structures, which define the
 *           mapping of "OpenServo Interface" "Addresses" to the register addressing
 *           of the specific OpenServo defined by an instance of this structure.
 *
 * OPENSERVOBOARDCONF m_conf
 *         o Stores the OpenServo's "configuration".
 *
 * byte_t m_nBankSelect
 *        o Current recorded register bank (for bank selected registers).
 *
 *          NOTE: This assumes that the application does not change the bank selection
 *                outside of the normal interface access, doesn't alter this if it does,
 *                etc.
 *
 *                TODO: This can be reset as known, or set by...?
 *
 * byte_t m_nCurRegAddress
 *         o Current recorded register address- used when chaining access together
 *           using the TODO: insert function description here.
 *
 *          NOTE: This assumes that the application does not change the address mid
 *                transaction, etc.
 *
 *                Register addresses are in the range [0x00 to 0xFF], this fact is
 *                used in the software: for example 0xFF is used to indicate that
 *                the current address latched in the servo is not known.
 *
 *                TODO: This can be reset as known, or set by...?
 *
 * byte_t m_nIOStatus
 *         o Records the current "I/O status" of the OpenServo, this indicates whether the
 *           OpenServo is "open" (I2C trsnaction started for it, etc.). This variable is
 *           used to control chained accesses to the OpenServo across the I2C used to
 *           reduce I2C bandwidth usage, etc.
 *
 *           TODO: Better description.
 *
 *           This is treated as a bit array, all bits clear indicate that the OpenServo
 *           does not have an I2C transaction started for it. The bits are assigned as
 *           follows:
 *
 *              0 (1.)    o Bits 0 and 1 form a two bit quantity which indicate the type
 *              1 (2.)      of I2C transaction that is current open against the OpenServo:
 *
 *                             0   o None
 *                             1   o Read
 *                             2   o Write
 *                             3   o Command
 *
 *              2 (4.)    o When set indicates that the OpenServo has an I2C acknowledgement
 *                          (master to slave) after read outstanding.
 *
 *              7 (128.)  o An I2C transaction has been started. Initially set when an I2C
 *                          start/restart condition has been set on the I2C bus to which the
 *                          servo is connected and the servo's device address has been
 *                          successfully sent. Remains set until the I2C bus is released with
 *                          regards to this servo. Other status information about how far
 *                          the I2C transaction might have progressed is indicated by the
 *                          other bits.
 */
typedef struct OPENSERVO
{
   OSI_DTYPE m_nDType;
   int_t m_nBus;
   byte_t m_nAddress;
   const OPENSERVOADDRESSMAP *m_pAddressMap;
   OPENSERVOBOARDCONF m_conf;
   byte_t m_nBankSelect;
   byte_t m_nCurRegAddress;
   byte_t m_nIOStatus;
} OPENSERVO;

/*******************************************************************************
 *
 * OSI error codes, i.e. values that will be returned by the OSI_GetLastError
 * function.
 *
 * NOTE: Rather than using enum, the values are #defined so that later versions
 *       of the software can used codes that have the same meaning.
 */
#define OSI_SUCCESS                    0
#define OSI_ERR_ALREADYINITIALISED     1
#define OSI_ERR_NOTINITIALISED         2
#define OSI_ERR_HARDWARE               3 /* I2C interface hardware */
#define OSI_ERR_SOFTWARE               4 /* A software problem (e.g. an internal error detected) */
#define OSI_ERR_OUTOFMEMORY            5
#define OSI_ERR_BADARGS                6 /* Function was called with invalid arguments */
#define OSI_ERR_I2CREADERROR           7
#define OSI_ERR_I2CWRITEERROR          8
#define OSI_ERR_I2CMTRANSACTIONERROR   9
#define OSI_ERR_NOI2CTRANSACTION      10
#define OSI_ERR_BADREGISTERADDRESS    11
#define OSI_ERR_BADCOMMANDID          12
#define OSI_ERR_BADID                 13
#define OSI_ERR_NOREGISTER            14
#define OSI_ERR_NOTANOPENSERVO        20

/*******************************************************************************
 *
 * Function prototypes
 */
int_t OSI_GetLastError();
const char *OSI_GetLastErrorText();
int_t OSI_SetLastError(int_t nerror_t, char *pat);
int_t OSI_Init(bool_t bScan, bool_t bSharedMaps);
bool_t OSI_IsInitialised();
int_t OSI_Shutdown();
int_t OSI_RescanForServos(bool_t bSharedMaps);
OPENSERVO *OSI_AddServo(int_t nbus, byte_t naddress, bool_t bSharedMap);
int_t OSI_SetHardwareConf(OPENSERVO *posid, uint16_t nVREG, uint16_t nR3R4, uint16_t nR4R6, uint16_t nClock);
int_t OSI_GetDeviceCount();
OPENSERVO *OSI_GetDeviceIDByPOS(int_t ndevice);
OPENSERVO *OSI_GetDeviceIDByAddress(int_t nbus, byte_t nAddress);
OPENSERVO *OSI_GetDeviceIDByAddressA(const char *pBusName, byte_t nAddress);

int_t OSI_Open(OPENSERVO *posid, byte_t nrw);
int_t OSI_Close(OPENSERVO *posid);

int_t OSI_ReadRegister(OPENSERVO *posid, OSI_REGID nID, void *pBuffer, int_t nSize);
int_t OSI_WriteRegister(OPENSERVO *posid, OSI_REGID nID, const void *pBuffer, int_t nSize);
int_t OSI_Command(OPENSERVO *posid, OSI_CMDID nID);

/*******************************************************************************
 *
 * Declare the "advanced interface functions"
 *
 * The following prototypes declare function interfaces to read or write
 * specific registers or issue specific commands "by name". For the most part
 * the functions are declared as being "inline". Some functions are implemented
 * directly (in osi.c), because they are more complex and would not "save space
 * and/or time" by being inlined, for example the OSI_GetBatteryVoltageAsV
 * function.
 *
 * NOTE: The macro OSI_LEAN can be defined to prevent the "larger" "advanced
 *       interface functions" from being declared or included in the compilation
 *       of osi.c.
 *
 *       The macro OSI_VERYLEAN can be defined to prevent all of the "advanced
 *       interface functions" from being declared or included, where relevant,
 *       in the compilation of osi.c.
 *
 * The purpose of each function should be self evident from the names, comments
 * will be included where special attention is required (typically for those that
 * are implemented in osi.c). Any registers stored in the OPENSERVO structure are
 * only updated if the values are successfully read from, or written to, the servo.
 *
 * Each function takes an OPENSERVO *posid argument which points to an OPENSERVO
 * structure that defines the servo that is to be read or written and either a
 * pointer (to the location in which the value read should be stored: "get") or
 * the value to be written to the servo ("set").
 *
 * Each function returns OSI_SUCCESS if successful, otherwise an error value is
 * returned.
 *
 * NOTE: The OSI_ReadRegister/OSI_WriteRegister/OSI_Command functions are not, should
 *       not be, bypassed- otherwise there is no guarentee that the correct "state"
 *       is maintained in the software: such as the currently selected bank or last
 *       address read.
 */
#ifndef OSI_VERYLEAN

#ifdef __GNUC__ /* TODO: Need to find a better way to fix this GNU C problem */
#define __inline static inline
#endif
#define OSIAIDEF_Set_FUNC(ID,NAME,ARGTYPE) __inline int_t OSI_Set##NAME(OPENSERVO *posid, ARGTYPE n##NAME) { return OSI_WriteRegister(posid,OSI_REGID_##ID,&n##NAME,sizeof(ARGTYPE)); }
#define OSIAIDEF_Get_FUNC(ID,NAME,ARGTYPE) __inline int_t OSI_Get##NAME(OPENSERVO *posid, ARGTYPE *pn##NAME) { return OSI_ReadRegister(posid,OSI_REGID_##ID,pn##NAME,sizeof(ARGTYPE)); }

OSIAIDEF_Set_FUNC(BANKSELECT,Bank,byte_t)
OSIAIDEF_Get_FUNC(BANKSELECT,Bank,byte_t)
OSIAIDEF_Get_FUNC(FLAGS,Flags,uint16_t)
OSIAIDEF_Get_FUNC(TIMER,Timer,uint16_t)
OSIAIDEF_Get_FUNC(POSITION,Position,uint16_t)
OSIAIDEF_Get_FUNC(VELOCITY,Velocity,int16_t)
OSIAIDEF_Get_FUNC(CURRENT,Current,uint16_t)
OSIAIDEF_Get_FUNC(BACKEMF,BackEMF,uint16_t)
OSIAIDEF_Get_FUNC(SEEK,Seek,uint16_t)
OSIAIDEF_Set_FUNC(SEEK,Seek,uint16_t)
OSIAIDEF_Get_FUNC(SEEKVELOCITY,SeekVelocity,uint16_t)
OSIAIDEF_Set_FUNC(SEEKVELOCITY,SeekVelocity,uint16_t)
OSIAIDEF_Get_FUNC(TWI_ADDRESS,I2CAddress,int_devaddr_t)
OSIAIDEF_Set_FUNC(TWI_ADDRESS,I2CAddress,int_devaddr_t)
OSIAIDEF_Get_FUNC(PWM_FREQ_DIVIDER,PWM_FreqDivider,uint16_t)
OSIAIDEF_Set_FUNC(PWM_FREQ_DIVIDER,PWM_FreqDivider,uint16_t)
OSIAIDEF_Get_FUNC(MINSEEK,MinSeek,uint16_t)
OSIAIDEF_Set_FUNC(MINSEEK,MinSeek,uint16_t)
OSIAIDEF_Get_FUNC(MAXSEEK,MaxSeek,uint16_t)
OSIAIDEF_Set_FUNC(MAXSEEK,MaxSeek,uint16_t)
OSIAIDEF_Get_FUNC(REVERSESEEK,ReverseSeek,uint8_t)
OSIAIDEF_Set_FUNC(REVERSESEEK,ReverseSeek,uint8_t)
OSIAIDEF_Set_FUNC(ALERT_CURR_MAXLIMIT,MaxCurrent,uint16_t)
OSIAIDEF_Get_FUNC(ALERT_CURR_MAXLIMIT,MaxCurrent,uint16_t)
OSIAIDEF_Set_FUNC(ALERT_VOLT_MAXLIMIT,MaxVoltage,uint16_t)
OSIAIDEF_Get_FUNC(ALERT_VOLT_MAXLIMIT,MaxVoltage,uint16_t)
OSIAIDEF_Set_FUNC(ALERT_VOLT_MINLIMIT,MinVoltage,uint16_t)
OSIAIDEF_Get_FUNC(ALERT_VOLT_MINLIMIT,MinVoltage,uint16_t)

__inline int_t OSI_GetBatteryVoltage(OPENSERVO *posid, uint16_t *pnVoltage)
{
   int_t rc=OSI_ReadRegister(posid,OSI_REGID_BATTVOLTAGE,pnVoltage,sizeof(*pnVoltage));
   if(rc==OSI_SUCCESS)
   {

/*
 * NOTE: Every time a voltage reading is requested, send the command to measure
 *       the voltage. The command takes time to complete, so it is sent after
 *       retrieving the currently stored value.
 *
 * TODO: Later firmware may not require the command to be sent.
 */
      rc=OSI_Command(posid,OSI_CMDID_BATTVOLTAGE_RESAMPLE);
   }
   return rc;
}

__inline int_t OSI_GetPWM(OPENSERVO *posid, uint8_t *pnCW, uint8_t *pnCCW)
{
   int rc=OSI_ReadRegister(posid,OSI_REGID_PWM_CW,pnCW,sizeof(*pnCW));
   if(rc==OSI_SUCCESS)
   {
      rc=OSI_ReadRegister(posid,OSI_REGID_PWM_CCW,pnCCW,sizeof(*pnCCW));
   }
   return rc;
}

#ifndef OSI_LEAN
int_t OSI_GetPID(OPENSERVO *posid, OPENSERVOPIDDATA *pPIDData);
int_t OSI_SetPID(OPENSERVO *posid, const OPENSERVOPIDDATA *pPIDData);
int_t OSI_RedirectRegister(OPENSERVO *posid, OSI_REGID nReg, byte_t nAddress); // TODO: Implement
int_t OSI_GetBatteryVoltageAsV(OPENSERVO *posid, uint16_t *pnVoltage);
#endif // OSI_LEAN

#undef OSIAIDEF_Set_FUNC
#undef OSIAIDEF_Get_FUNC

#endif // OSI_VERYLEAN

#pragma pack(pop) /* End of prefered packing */

#ifdef __cplusplus /* End of C++ linkage conventions */
}
#endif

#endif /* __OSI_H__ */
