#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#if defined(OBDEV)

// use avrusb library
#include "../usbdrv/usbdrv.h"
#include "../usbdrv/oddebug.h"
#define usb_init()  usbInit()
#define usb_poll()  usbPoll()
//PD-TPC - those three functions must be declared/preprocessed too for avrusb
#define usb_in(par1,par2) usbFunctionRead(par1,par2)
#define usb_out(par1,par2) usbFunctionWrite(par1,par2)
#define usb_setup(par1) usbFunctionSetup(par1)

#else

// use usbtiny library 
#include "usb.h"
#include "usbtiny.h"

#endif

// common typedefs
typedef unsigned char   byte_t;
typedef unsigned int    uint_t;

// Setup
//#define OBDEV
#define F_CPU 12000000UL  // 12 MHz

#include <util/delay.h>

#define PWM_PIN         PB3
#define PORT            PORTB

#define DDR             DDRB
#define DDRMASK         0xED
// Settings for Atmega8
#define POWER_MASK      0x01
#define SCK_MASK        (1 << 5)
#define MOSI_MASK       (1 << 3)
// #define SDA_O           (1<<PC4)
// #define SCL_O           (1<<PC5)
#define RESET_DDR       DDRC
#define RESET_PORT      PORTC
#define RESET_PIN       (1 << 4)
#define RESET_DDRMASK   RESET_PIN
#define PIN             PINB
#define MISO_MASK       (1 << 4)

enum
{
    // Generic requests
    USBTINY_ECHO,           // echo test
    USBTINY_READ,           // read byte (wIndex:address)
    USBTINY_WRITE,          // write byte (wIndex:address, wValue:value)
    USBTINY_CLR,            // clear bit (wIndex:address, wValue:bitno)
    USBTINY_SET,            // set bit (wIndex:address, wValue:bitno)
    // Programming requests
    USBTINY_POWERUP,        // apply power (wValue:SCK-period, wIndex:RESET)
    USBTINY_POWERDOWN,      // remove power from chip
    USBTINY_SPI,            // issue SPI command (wValue:c1c0, wIndex:c3c2)
    USBTINY_POLL_BYTES,     // set poll bytes for write (wValue:p1p2)
    USBTINY_FLASH_READ,     // read flash (wIndex:address)
    USBTINY_FLASH_WRITE,    // write flash (wIndex:address, wValue:timeout)
    USBTINY_EEPROM_READ,    // read eeprom (wIndex:address)
    USBTINY_EEPROM_WRITE,   // write eeprom (wIndex:address, wValue:timeout)
    // I2C requests
    USBI2C_READ = 20,       // read from i2c (wValue:address)
    USBI2C_WRITE,           // write to i2c (wValue:address)
    USBI2C_STOP,            // i2c stop (wValue:address)
    USBI2C_STATUS,
    USBI2C_SET_BITRATE,
    // GPIO Requests
    USBIO_SET_DDR = 30,
    USBIO_SET_OUT,
    USBIO_GET_IN,
    USBTINY_RESET,
    // Serial Requests
    USBSER_READ = 40,
    USBSER_WRITE,
    // PWM Requests
    USBPWM_RATE = 50,
    // EEPROM commands
    EEPROM_READ = 60,
    EEPROM_WRITE,
    EEPROM_PLAYBACK
};


#endif
