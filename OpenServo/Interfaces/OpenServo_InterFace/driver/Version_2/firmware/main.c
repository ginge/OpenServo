// ======================================================================
// I2C AVR IO (avrdude type "bsd") via USB.
//
// Copyright (C) 2007 Barry Carter
//
// SPI portions Copyright (C) 2006 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
//#define OBDEV
#include <util/twi.h>
#define F_CPU 12000000UL  // 12 MHz
#include <util/delay.h>
#if defined(OBDEV)
// use avrusb library
#include "../usbdrv/usbdrv.h"
#include "../usbdrv/oddebug.h"
typedef unsigned char   byte_t;
typedef unsigned int    uint_t;
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

// Status flags for the I2C reading and writing
enum
{
    I2C_PACKET              = 0x01,
    I2C_READ                = 0x02,
    I2C_READ_ON             = 0x04,
    I2C_WRITE               = 0x08,
    I2C_STATUS              = 0x10,
};

// EEPROM Flags
enum
{
    EEPROM_DIR_W            = 0x01,
    EEPROM_SEND_STOP        = 0x02,
};

// Serial mode flags
enum
{
    SER_MODE_READ           = 0x02,
    SER_MODE_WRITE          = 0x04,
};


#define PORT            PORTB
#define DDR             DDRB
#define DDRMASK         0xED
// Settings for Atmega8
#define POWER_MASK      0x01
#define SCK_MASK        (1 << 5)
#define MOSI_MASK       (1 << 3)

#define EXTRA_IO_DDR    PORTD

#define I2CPORT         PORTC
#define I2CDDR          DDRC
#define SDA_O           (1<<PC4)
#define SCL_O           (1<<PC5)
#define RESET_DDR       DDRC
#define RESET_PORT      PORTC
#define RESET_PIN       (1 << 4)
#define RESET_DDRMASK   RESET_PIN

#define PWM_PIN         PB3

// Programmer input pins:
//      MISO    PD3     (ACK)
#define PIN             PINB
#define MISO_MASK       (1 << 4)

#define I2C_TIMEOUT     6000000ul

#define TWSRMASK        0xF8

//USART Buffer and Baudrate Definitions
#define UART_PACKET_HD_SIZE 4
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16l))) - 1)
#define UART_TIMEOUT 1200
#define UART_RX_TIMEOUT 800

#define UART_RX_BUFFER_SIZE 128     /* 2,4,8,16,32,64,128 or 256 bytes */

#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1 )
#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif

// Local data
static byte_t          sck_period;      // SCK period in microseconds (1..250)
static byte_t          poll1;           // first poll byte for write
static byte_t          poll2;           // second poll byte for write
static uint_t          address;         // read/write address
static uint_t          timeout;         // write timeout in usec
static byte_t          cmd0;            // current read/write command byte
static byte_t          cmd[4];          // SPI command buffer
static byte_t          res[4];          // SPI result buffer
static byte_t          i2caddr;         // i2c address
static byte_t          i2cstats[32];    // status array buffer
static byte_t          i2cstatspos;     // status register incremental pointer
static byte_t          i2cstat;         // status of I2C comms
static byte_t          i2crecvlen;      // I2C buffer recieve length
static uint8_t         io_enabled;
static uint8_t         io_ddr;
static byte_t          modestat;        // Current enabled mode
static byte_t          pwm_on;          // Enable PWM mode

static unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;
uint8_t serstat;
uint16_t rx_timeout;
uint16_t tx_timeout;

//functions
int8_t uart_putc(uint8_t c);
int8_t uart_puts (char *s);
void pwm_init(void);
void pwm_deinit(void);
void playback_start(void);

typedef struct { 
    volatile uint8_t *p_ddr; 
    volatile uint8_t *p_port; 
    volatile uint8_t *p_pin;
    uint8_t bit;
    uint8_t mask; 
} port_table_type;

static port_table_type port_table[] = { 
    { &DDRD, &PORTD, &PIND, PD1, 0x00 },   // TX line
    { &DDRD, &PORTD, &PIND, PD0, 0x00 },   // RX line
    { &DDRB, &PORTB, &PINB, PB4, 0x00 },   // MISO
    { &DDRB, &PORTB, &PINB, PB3, 0x00 },   // MOSI
    { &DDRC, &PORTC, &PINC, PC4, 0x00 },   // SDA
    { &DDRC, &PORTC, &PINC, PC5, 0x00 }    // SCL
                                      };

#define PORT_TABLE_IO_COUNT 6

// force a reboot initiated by watchdog reset
void reboot(void)
{
    wdt_disable();
    wdt_enable(WDTO_2S);
    cli();
    while(1) ;;
}

// ----------------------------------------------------------------------
// Delay exactly <sck_period> times 0.5 microseconds (6 cycles).
// ----------------------------------------------------------------------
__attribute__((always_inline))
static void delay(void)
{
    asm volatile(
            "	mov	__tmp_reg__,%0	\n"
            "0:	rjmp	1f		\n"
            "1:	nop			\n"
            "	dec	__tmp_reg__	\n"
            "	brne	0b		\n"
            : : "r" (sck_period) );
}

// Issue one SPI command.
static void spi(byte_t* cmd, byte_t* res)
{
    byte_t	i;
    byte_t	c;
    byte_t	r;
    byte_t	mask;

    for (i = 0; i < 4; i++)
    {
        c = *cmd++;
        r = 0;
        for(mask = 0x80; mask; mask >>= 1)
        {
            if (c & mask)
            {
                PORT |= MOSI_MASK;
            }
            delay();
            PORT |= SCK_MASK;
            delay();
            r <<= 1;
            if (PIN & MISO_MASK)
            {
                r++;
            }
            PORT &= ~ MOSI_MASK;
            PORT &= ~ SCK_MASK;
        }
        *res++ = r;
    }
}

// Create and issue a read or write SPI command.
static void spi_rw (void)
{
    uint_t a;

    a = address++;
    if (cmd0 & 0x80)
    { // eeprom
        a <<= 1;
    }
    cmd[0] = cmd0;
    if (a & 1)
    {
        cmd[0] |= 0x08;
    }
    cmd[1] = a >> 9;
    cmd[2] = a >> 1;
    spi(cmd, res);
}

// Calculate the new bitrate from an input khz
int i2c_bitrate_set(int twbr_set, int twps_set)
{
    if (twps_set == 4)          // Prescaler set to 4
    {
        TWSR &= ~((1<<TWPS0));
        TWSR |= 1<<TWPS0;       // Set the prescaler for twi in the status register
    }
    else //\ TWPS 1
    {
        TWSR &= ~((1<<TWPS0) | (1<<TWPS1)); // Set the prescaler for twi in the status reegister
    } 

    TWBR = twbr_set;            // 10;//max bitrate for twi, ca 333khz by 12Mhz Crystal

    return 1;
}

// Initialise the I2C hardware in AVR
void i2c_init()
{
    // Set the port directions and disable I2C pullups
    DDRB &= ~(1<<PB5);   // Set PB5 (SCK) as input
    PORTB &= ~(1<<PB5);  // Disable the pullup
    DDRC &= ~((1<<PC4)|(1<<PC5));  // Set I2C pins PC4 and PC5 as input
    PORTC &= ~(1<<PC4);  // Disable pullups on I2C
    PORTC &= ~(1<<PC5);

    TWCR = 0;            // Clear the control register
    i2c_bitrate_set(10, 1);          // 10;//max bitrate for twi, ca 333khz by 12Mhz Crystal
    TWCR |= (1<<TWEN);   // Enable I2C in control register
}

// Wait for the interupt flag to clear in the TWI hardware
// If locks because the interrupt didn't clear, we loop
// until a timeout peroid of I2C_TIMEOUT
static inline int i2c_wait_int()
{
    uint32_t i = 0;
    while((TWCR & (1<<TWINT)) == 0);
    {
        if (i++ > I2C_TIMEOUT)
            return -1;
    }

    return 1;
}

// Send one byte of data over I2C
static inline int i2c_send (byte_t sendbyte)
{
    TWDR = sendbyte;                            // Fill Data register
    TWCR |= (1<<TWINT);                         // Send the byte
    if (i2c_wait_int() < 0) return -1;           // Wait for the byte to send
    return TWSR & TWSRMASK;                                // Return the status
}

// Read one byte over I2C
static inline int i2c_read(void)
{
    if (i2c_wait_int() < 0) return -1;           // Check to see if the interrupt is clear
    return TWDR;                                // Store the byte in the status register
}

// Send Stop condition
static inline void i2c_stop(void)
{
    TWCR |= (1<<TWINT) | (1<<TWSTO);            // Send the stop bit
}

// Send the Start condition
static inline int i2c_start(void)
{
    
    TWCR |= (1<<TWINT) | (1<<TWSTA);            // Send the I2C start command

    return i2c_wait_int();                      // and wait for it to complete
}

void i2c_error(void)
{
    i2cstat = 0;                                // Clear the status flags variable
    i2c_stop();
}

// Start an I2C transfer by sending the Start bit, the device address and the R/W flag
int i2c_begin(byte_t i2caddr, byte_t *data, byte_t direction)
{

    if (i2c_start() < 0)
    {
        data[0] = TW_BUS_ERROR;                            // return an error
        return 0;
    }
    i2cstats[0x01] = TWSR & TWSRMASK;                      // update the status array with the returned status
    int TWSRtmp = 0;

    // Start sending the I2C packet by following the normal I2C protocol sequence of Start data Stop
    // This USB control packet will contain the I2C address only so this function generates a start 
    // followed by the device address
    if ((TWSR & TWSRMASK) == TW_START || (TWSR & TWSRMASK) == TW_REP_START)           // If the start completed
    {
        if (direction == USBI2C_READ)
        {
            // Use the TWSRtmp temporary variable to store the return of the i2c_send function
            TWSRtmp = i2c_send(i2caddr<<1 | 0x01);         // Start the data send by sending device address + R
            TWCR &= ~(1<<TWSTA);                           // Disable the start flag in the control register
        }
        else
        {
            TWCR &= ~((1<<TWSTA) | (1<<TWINT));            // Disable the start flag in the control register
            TWSRtmp = i2c_send (i2caddr<<1 & 0xFE);        // Start the data send by sending device address + W
        }
        i2cstats[0x02] = TWSRtmp;                          // Store the return status in the status array
        if (TWSRtmp < 0) i2c_error();
    }
    else                                                   // The start failed. Error handle
    {
        TWCR = (1<<TWINT) | (1<<TWEN);                     // disable all commands in the control register
        data[0] = i2cstats[0x00] = (TWSR & TWSRMASK);      // Store the return status in the status array
        if (direction == USBI2C_READ)
            i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON); // Disable the flags for reading
        else
            i2cstat &= ~(I2C_WRITE | I2C_PACKET);          // Disable the flags for reading
        return 0;
    }
    // Arbitration was lost, clean up and close. We are reading from the TWSRtmp temporary
    // variable which was filled with the return of the send command
    if (TWSRtmp == TW_MR_ARB_LOST  || TWSRtmp == TW_MR_SLA_NACK )
    {
        TWCR = (1<<TWEN);
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON);
        return 0;
    }
    // TWI Master Receiver Slave acknowledge. Working on the TWSRtmp variable
    // Checks to see if the start command and device selection generated an ACK
    if (TWSRtmp != TW_MR_SLA_ACK && direction == USBI2C_READ)
    {
        TWCR = (1<<TWEN);                                  // No ACK was generated. Disable pending flags in the Control register
        data[0] = i2cstats[0x00] = TWSRtmp;                // Store the status in the status array
        i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON); // Disable flags for reading in the status register
        return 0;
    }

    // TWI Master Transmitter Slave Not Ack
    // cleanup and disable writing to I2C
    if (TWSRtmp == TW_MT_SLA_NACK)
    {
        TWCR = (1<<TWINT) | (1<<TWEN);
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_WRITE | I2C_PACKET);
        return 0;
    }
    // TWI Master Transmitter Slave Ack
    if (TWSRtmp != TW_MT_SLA_ACK && direction == USBI2C_WRITE) // On no ACK cleanup and reset flags
    {
        TWCR = (1<<TWINT) | (1<<TWEN);                  //
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_WRITE | I2C_PACKET);
        return 0;
    }

    if (direction == USBI2C_READ)
    {
        if (data[6] > 1 || data[7] > 0)
            TWCR |= (1<<TWEA);

        TWCR |= (1<<TWINT);                                // Clear the interrupt pending flag in the status register
    }
    return 1;
}

static inline int i2c_read_bytes(byte_t *data,byte_t len)
{
    byte_t i;
    int read;

    for (i = 0; i < len; i++)
    {
        read = i2c_read();
        if (read < 0)
        {
            return -1;                                     // Really bad!
        }
        if (TWSRtmp == TW_MR_DATA_NACK)                    // If we get a no ACK (NACK) from the slave, dont read on
        {
            i2cstat &= ~I2C_READ_ON;                       // Bail out of the loop now. Slave says no more data
            //i2c_stop();                                  // commected out -- API should do this, although that behaviour is questionable at best.
            return i;                                      // Return the real length
        }
        data[i] = (byte_t)read;                            // Store the byte in the status register
        int TWSRtmp = (TWSR & TWSRMASK);                   // Store the return status
        i2cstats[0x08+i] = TWSRtmp;                        // Store the return status in the status array
        if (i2crecvlen > 0)
        {
            i2crecvlen--;
        }
        if (i2crecvlen == 1)                               // On the last byte, we NACK the slave so we don't lock the bus
        {
            TWCR = (TWCR | (1<<TWINT)) & ~(1<<TWEA);       // Start SCL clocking without the ACK to the TWI slave
        }
        if (i2crecvlen == 0)                               // Now check if we are done
        {
            return i;                                      // Return the real length
        }
        
        TWCR |= (1<<TWINT);                                // ACK and Start SCL clocking
    }
    return len;
}

#if defined( OBDEV )
USB_PUBLIC uchar usbFunctionSetup(uchar data[8])
{
    static uchar replyBuf[4];
    usbMsgPtr = replyBuf;
#else
// Handle a non-standard SETUP packet over USB
extern byte_t usb_setup(byte_t data[8])
{
#endif
    byte_t      bit;
    byte_t      mask;
    byte_t*     addr;
    byte_t      req;
    uint8_t     x;

    // Generic requests
    req = data[1];
    if (req == USBTINY_ECHO)
    {
        return 8;
    }

    req = data[1];
    if (req == USBTINY_RESET)
    {
        reboot();
    }
    // Request a new I2C bitrate in case of high speed errors.
    if (req == USBI2C_SET_BITRATE)
    {
        i2c_bitrate_set(data[2], data[4]);
        data[0] = 1;
        return 1;
    }
    // I2C requests

    // Send the stop command over the I2C bus
    if (req == USBI2C_STOP)
    {
        if ((i2cstat & (I2C_READ | I2C_WRITE)) != 0)  // Send the stop signal if we are reading or writing
        {
            i2c_stop();
            i2cstat = 0;                             // Clear the status flags variable
            return 0;
        }
        data[0] = i2cstat;                           // Return the status flags in the data buffer
        return 1;
    }
    // Handle a status read request
    else if (req == USBI2C_STATUS)
    {
        i2cstat |= I2C_STATUS | I2C_PACKET;           // Update the status flag to enable I2C status data
        i2cstatspos = 0;                              // Reset the int pointer to the data position
        return 0xff;                                  // Will be handled by usb_in
    }
    // Handle an I2C read request
    else if (req == USBI2C_READ)
    {
        i2cstats[0x00] = 0;                          // Clear the status register
        i2cstats[0x04] = data[6];                    // Set the status
        i2cstats[0x05] = data[7];
        if (data[7] == 0)
        {
            i2crecvlen = data[6];
        }
        else
        {
            i2crecvlen = 0;
        }
        i2cstat = (i2cstat | I2C_READ | I2C_PACKET) & ~I2C_STATUS ;  // Add the read flag to the status variable
        if (i2caddr != data[4] || (i2cstat & I2C_READ_ON) == 0)       // If I2C address has not changed or we are not reading more data
        {
            i2caddr = data[4];                                     // set the I2C address to send to
            i2cstat |= I2C_READ_ON;                                // Update the status flags variable to continue read
            if (i2c_begin(i2caddr, data, USBI2C_READ) == 0)        // Begin the I2C transfer the rest to be handled by usb_in
                return 0;
        }
        return 0xff;
    }
    // Handle an I2C write transaction
    else if (req == USBI2C_WRITE)
    {
        i2cstats[0x00] = 0;
        i2cstat = I2C_PACKET;
        if (i2caddr != data[4] || (i2cstat & I2C_WRITE) == 0)    // Check to see is write is enabled in the flags
        {
            i2caddr = data[4];
            i2cstat |= I2C_WRITE;                               // Disable write in the status flags
            i2c_begin(i2caddr, data, USBI2C_WRITE);             // Begin the I2C transfer. The rest handled by usb_out
        }
        return 0;
    }
    else
    {
        i2cstat &= ~I2C_PACKET;
    }
    // I2C end section

    // SPI begin
    addr = (byte_t*) (int) data[4];
    if	(req == USBTINY_READ)                          // SPI read command
    {
        data[0] = *addr;
        return 1;
    }
    else if (req == USBTINY_WRITE)                         // SPI write
    {
        *addr = data[2];
        return 0;
    }
    bit = data[2] & 7;                                   // mask the SPI
    mask = 1 << bit;
    if	(req == USBTINY_CLR)                           // Clear command
    {
        *addr &= ~ mask;
        return 0;
    }
    if	(req == USBTINY_SET)                           // SPI set
    {
        *addr |= mask;
        return 0;
    }

    // Programming requests
    // Requred to enable the SPI hardware in the OSIF and disable I2C
    if	(req == USBTINY_POWERUP)
    {
        TWCR = 0;                                         // Disable I2C
        TWSR = 0;                                         // Clear the status register
        sck_period = data[2];                             // Store the clock period
        mask = DDRMASK;                                   // setup the port mask
        RESET_DDR |= RESET_PIN;                           // Set the port direction for the reset line
        if (data[4])                                    // Read the high low status of the reset
        {
            RESET_PORT |= RESET_PIN;                      // Bring the reset line high
        }
        else
            RESET_PORT &= ~RESET_PIN;                     // Bring the reset low
        DDRC &= ~((1<<PC5));
        PORTC &= ~((1<<PC5));
        DDR  = DDRMASK;                                   // Reset spi port directions
        PORT = mask;                                      // setup the port mask
        //PORTB |= (1<<PB2);     // set AVR reset high
        return 0;
    }
    // Disable SPI mode and enable I2C
    if	(req == USBTINY_POWERDOWN)
    {
        RESET_PORT |= RESET_PIN;                          // Bring the reset low
        i2c_init();                                       // Initialise the I2C module

        DDR  &= ~DDRMASK;                                 // Reset the spi port to defaults
        return 0;
    }
    if	(!PORT)
    {
        return 0;
    }
    if	(req == USBTINY_SPI)                            // SPI
    {
        spi(data + 2, data + 0);
        return 4;
    }
    if	(req == USBTINY_POLL_BYTES)
    {
        poll1 = data[2];
        poll2 = data[3];
        return 0;
    }
    address = * (uint_t*) & data[4];
    if	(req == USBTINY_FLASH_READ)
    {
        cmd0 = 0x20;
        return 0xff;	                                   // usb_in() will be called to get the data
    }
    if	(req == USBTINY_EEPROM_READ)
    {
        cmd0 = 0xa0;
        return 0xff;	                                   // usb_in() will be called to get the data
    }
    timeout = * (uint_t*) & data[2];
    if	(req == USBTINY_FLASH_WRITE)
    {
        cmd0 = 0x40;
        return 0;	                                   // data will be received by usb_out()
    }
    if	(req == USBTINY_EEPROM_WRITE)
    {
        cmd0 = 0xc0;
        return 0;	                                   // data will be received by usb_out()
    }
    // Set an IO line high or low
    if  (req == USBIO_SET_DDR)
    {
        io_ddr = data[4];                                  // This holds the direction bits
        io_enabled = data[2];                              // This holds the IO enabled status
        // Set the port directions
        for (x=0; x < PORT_TABLE_IO_COUNT; x++)
        {
          if (io_enabled & (1<<x))              // Check bit 0 for IO dir
          {
            // io_tx
            if (io_ddr & (1<<x))             // if set as output
            {
                *port_table[x].p_ddr |= (1<<port_table[x].bit);
                *port_table[x].p_port |= (1<<(port_table[x].bit)); //low
            }
            else                        //input
            {
                *port_table[x].p_ddr &= ~(1<<(port_table[x].bit));
            }
          }
        }
        data[0] = 1;                                       // flag all ok
        return 0;
    }
    if  (req == USBIO_SET_OUT)
    {
        // check to make sure it is an output
        for (x=0; x<PORT_TABLE_IO_COUNT; x++)
        {
            if (io_enabled & (1<<x))                 // Check bit 0 for IO dir
            {
                if (data[4] & (1<<x))                // is it high?
                {
                    *port_table[x].p_port &= ~(1<<(port_table[x].bit));
                }
                else
                {
                    *port_table[x].p_port |= (1<<(port_table[x].bit));
                }
            }
        }
        data[0] = 1;
        return 0;
    }
    else if  (req == USBIO_GET_IN)
    {
         data[0] = 0x00;
        // read anyways, regardless of ststus
        for (x=0; x < PORT_TABLE_IO_COUNT; x++)
        {
            if (*port_table[x].p_pin & (1<<port_table[x].bit))
                data[0] |= (1<<x);
        }
        return 1;
    }
    else if (req == USBSER_READ)
    {
        modestat = USBSER_READ;
        return 0xff;
    }
    else if (req == USBSER_WRITE)
    {
        return 0;
    }
    else if (req == USBPWM_RATE)
    {
        if (data[4] == 0)
        {
            pwm_deinit();
        }
        else
        {
            if (!pwm_on)
            {
                pwm_init();
            }
            else
            {
                OCR2 = data[4];
            }
        }
        data[0] = 1;
        return 1;
    }
    else if (req == EEPROM_READ)
    {
        modestat = EEPROM_READ;
        return 0xff;
    }
    else if (req == EEPROM_WRITE)
    {
        modestat = EEPROM_WRITE;
        return 0;
    }
    else if (req == EEPROM_PLAYBACK)
    {
        playback_start();
        return 1;
    }
    
    return 0;
}


// Handle an IN packet over USB
#if defined (OBDEV)
uchar usbFunctionRead(uchar *data, uchar len)
#else
extern byte_t usb_in(byte_t* data, byte_t len)
#endif
{
    byte_t	i;
    if ((i2cstat & I2C_PACKET) != 0)                           // Make sure we are in packet reading mode in the status register
    {
        if ((i2cstat & I2C_STATUS) != 0)                       // See if we are sending back the status array
        {                      
            for	(i = 0; i < len; i++ )
            {
                data[i] = i2cstats[i2cstatspos++];            // Send back the byte of the status array, and increment pointer
            }
        }
        else if ((i2cstat & I2C_READ) != 0)                   // I2C Read Data
        {                  
            len = i2c_read_bytes(data, len);
            if (len < 0)
                return 0;
        }
    }
    else if (modestat == USBSER_READ)
    {
    /*  for (i = 0; i < len; i++)
        {
        data[i] = UART_RxBuf[i]+block;
        block += 8;
    }*/
    }
    else if (modestat == EEPROM_READ)
    {
        uint8_t ee_data[len];
        eeprom_read_block((void*)&ee_data, (const void*)0, len);
        for (i = 0; i < len; i++)
        {
            data[i] = ee_data[i];
        }
    }
    else                                                      // SPI mode read
    {
        for (i = 0; i < len; i++)                             // For each byte send the spi
        {
            spi_rw();
            data[i] = res[3];
        }
    }
    return len;
}

// Handle an OUT packet over USB
#if defined(OBDEV)
uchar usbFunctionWrite(uchar *data, uchar len)
#else
extern void usb_out (byte_t* data, byte_t len)
#endif
{
    byte_t	i;
    uint_t	usec;
    byte_t	r;
    if ((i2cstat & I2C_PACKET) != 0x0)                         // Check to see if we are in packet mode
    {
        for (i = 0; i < len; i++)
        {
            int TWSRtmp = i2c_send(data[i]);                  // Send the data over I2C
            if (TWSRtmp != TW_MT_DATA_ACK
                       && TWSRtmp != TW_MT_DATA_NACK
                       && TWSRtmp != -1)                      // Check for an NACK in TWSR
            {
                i2cstats[0x03] = i2cstats[0x00] = TWSRtmp;    // Store the return status in the status array
                i2c_stop();                                   // Send stop. Close I2C
                i2cstat = 0;                                  // Clear all status flags
            }
        }
    }
    else if (modestat == USBSER_WRITE)
    {
        uart_puts((char *)data);
    }
    else if (modestat == EEPROM_WRITE)
    {
        eeprom_busy_wait();
        eeprom_write_block(data, (void*)0, len);
    }
    else                                                                // SPI write
    {
        for (i = 0; i < len; i++)
        {
            cmd[3] = data[i];
            spi_rw();
            cmd[0] ^= 0x60;	                                        // turn write into read
            for (usec = 0; usec < timeout; usec += 32 * sck_period)
            {	                                                        // when timeout > 0, poll until byte is written
                spi(cmd, res);                                          // Send the SPI packet
                r = res[3];
                if (r == cmd[3] && r != poll1 && r != poll2)
                {
                    break;
                }
            }
        }
    }
#if defined(OBDEV)
    return len;
#endif
}


void pwm_init()
{
    TCCR2 = (0<<FOC2)|(1<<WGM21)|(1<<WGM20)|(1<<CS20)|(0<<CS21)|(0<<CS22)|(1<<COM21)|(0<<COM20);//setup PMW, no prescaler
    OCR2 = 0x00;
    DDRB |= (1<<PWM_PIN);
    pwm_on = 1; 
}

void pwm_deinit()
{
    TCCR0 ^=(0<<COM21)|(0<<COM20); //disable OC2
    pwm_on = 0; 
}

void io_init(void)
{
    io_ddr = 0;
    io_enabled = 0;
}


void uart_init()
{
    byte_t x;

    UBRRH = (uint8_t)(BAUD_PRESCALE>>8); /* Set the baud rate */
    UBRRL = (uint8_t)(BAUD_PRESCALE);

    /* Enable UART receiver and transmitter, and receive interrupt */
    UCSRB = ((1<<RXCIE) | (1<<RXEN) | (1<<TXEN));

    //asynchronous 8N1
    UCSRC = (1 << URSEL) | (3 << UCSZ0);

    /* Flush receive buffer */
    for (x = 0; x < UART_RX_BUFFER_SIZE; x++)
    {
        UART_RxBuf[x] = 0;
    }

    x = 0;

    UART_RxTail = UART_RX_BUFFER_SIZE;
    UART_RxHead = x;

    rx_timeout = 0;
    tx_timeout = 0;
    UDR = '!';
}

void uart_buf_empty(void)
{
    int x;
    /* Flush receive buffer */
    for (x = 0; x < UART_RX_BUFFER_SIZE; x++)
    {
        UART_RxBuf[x] = 0;
    }
    UART_RxHead = 0;
}

int8_t uart_putc(uint8_t c)
{
   // wait until UDR ready
    while ((UCSRA & (1 << UDRE)) == 0) if (tx_timeout++ > UART_TIMEOUT) return -1; //Do nothing until clear to transmit, or return negative if transmission timeout reached
    UDR = c;    // send character
    tx_timeout = 0;
    return 1;
}

int8_t uart_puts (char *s) 
{
    //  loop until *s != NULL
    while (*s)
    {
        if ((uart_putc(*s)) < 0) return -1;
        s++;
    }
    return 1;
}

int uart_read_i2c(byte_t read_len)
{
    //Read the specified registers from the specified device, 
    //and transmit sequentially via uart.
    byte_t data[read_len + 2];
    byte_t i;

    if (usb_in(data, read_len) == 0)
    {
        return -1;
    }

    //crcappend(data, read_len + 2);  //crcappend adds the two CRC-16 bytes to the end of the data

    for (i = 0; i < read_len; i++)
    {
        uart_putc(data[i]);
    }

    return 1;
}

int uart_write_i2c(uint8_t *data, uint8_t write_len)
{
    //Write the specified values to the specified registers.
    usb_out(data, write_len);

    return 1;
}

void uart_nack(void)
{
    uart_puts("ERR");
}


void uart_i2c_scan(void)
{
    uint8_t i;
    for (i = 0; i < 128; i++)
    {
        if (i2c_begin((uint8_t) i, 0, USBI2C_READ))
        {
            uart_putc(i);  //Send the address of the found device.
            uart_putc(' '); // = '|';  //Send a line feed symbol
        }
    }
}

int uart_poll()
{
    //local declarations
    byte_t data[8];
    static uint8_t addr;
    static uint8_t len;

    // A transaction is defined as a start condition or the direction + address + length
    // Each transaction happens until the data is recived or a timeout occured.
    // > indicates send to OSIF < out of OSIF
    // Example:
    //> Wa4 1234                Write the data 
    //< OK
    //> Wa7 1234567             some more data
    //< OK
    //> Wa1 9                   and some more
    //< OK
    //> S
    //< OK STOP

    if (UART_RxHead == 0)
        return 0;

    rx_timeout++;  // Increment the timeout counter. This will be reset once data comes in.
    if ((rx_timeout > UART_RX_TIMEOUT) && serstat != 0)
    {
        uart_buf_empty();
        rx_timeout = 0;
        serstat = 0;
        uart_puts("TIMEOUT");
    }

    // If we are in read mode thenc ontinue to read until we reach the packet end or a timeout
    if (serstat == SER_MODE_READ)
    {
        if (uart_read_i2c(len) < 0) return -1;
        uart_buf_empty();
        serstat = 0;
        return 0;
    }
    else if (serstat == SER_MODE_WRITE)
    {
        if ((UART_RxHead - UART_PACKET_HD_SIZE) >= len)
        {
            uart_write_i2c(&UART_RxBuf[UART_PACKET_HD_SIZE], len);
            serstat = 0;
            uart_buf_empty();
        }
        return 0;
    }
    // We recieved the full packet header
    else if ((UART_RxHead >= UART_PACKET_HD_SIZE-1))
    {
        if ((UART_RxBuf[0] == 'R')) //read
        {
            addr = UART_RxBuf[1];
            len = UART_RxBuf[2];

            data[1] = USBI2C_READ;
            data[6] = len;
            data[7] = 0;
            data[4] = addr;
            if (usb_setup(data) > 0)
            {
                uart_puts("OK");
            }
            else
            {
                uart_nack();
                serstat = 0;
                return 0;
            }
            serstat = SER_MODE_READ;
        }
        else if ((UART_RxBuf[0] == 'W'))//write
        {
            addr = UART_RxBuf[1];
            len = UART_RxBuf[2];

            data[1] = USBI2C_WRITE;
            data[6] = len;
            data[4] = addr;
            if (usb_setup(data) == 0)
            {
                uart_puts("OK");
            }
            else
            {
                uart_nack();
                serstat = 0;
                return 0;
            }
            serstat = SER_MODE_WRITE;
        }
    }
    // Parse the single byte commands
    else if (UART_RxHead > 0)
    {
        //scan allowed only when not doing something else. ignore otherwise
        if ((UART_RxBuf[0] == 'F') && (serstat == 0)) // scan only when nothing else happening
        {
            uart_i2c_scan();
            uart_puts("OK SCAN");
            // throw away any non standard packet headers
            uart_buf_empty();
        }
        // Process the Stop command
        else if ((UART_RxBuf[0] == 'S') && ((serstat != 0))) //stop only when reading/writing
        {
            // Use the existing USB function Setup to process the stop command
            // as this handles all of the existing I2C logic
            data[1] = USBI2C_STOP;
            usb_setup(data);
            serstat = 0;
            // throw away any non standard packet headers
            uart_buf_empty();
            uart_puts("OK STOP");
        }
    }
    //default to disposing of the data if it is garbage but only if it is > packet header size and 
    //we are not in read or write mode.
    else if ((UART_RxHead >= UART_PACKET_HD_SIZE-1) && (serstat == 0))
    {
        uart_buf_empty();
        uart_puts("GARBAGE!");
    }
    return 0;
}

// init sequence code
// allows replay of saved EEPROM settings to default some I2C device(s)
// Flags:
// DIR_W = 0x01;
// STOP = 0x02;
// EEPROM format :   [data length][device address][direction (R|W) and stop bit flags][data]...[data]
void playback_init(void)
{
    ;
}

void playback_start()
{
    // read the entire eeprom byte by byte
    // first two bytes contain the length of the whole eeprom
/*    uint16_t ee_len = eeprom_read_word((uint16_t*)0);

    uint8_t data[ee_len];
    eeprom_read_block((void*)&data, (const void*)0x02, ee_len);

    modestat = EEPROM_PLAYBACK;
    
    // break into packets
    uint16_t offset = 0;
    while(icnt < ee_len)
    {
        //first byte is the number of data elements to send
        uint8_t data_cnt = data[icnt];
        uint8_t i2c_addr = data[icnt+1]
        uint8_t flags = data[icnt+2];
        uint8_t send_data[data_cnt];
        for (uint8_t i = 0; i < data_cnt; i++)
        {
            send_data[i] = data[icnt+3+i];
        }

        uint8_t s_data[8];
        s_data[1] = USBI2C_WRITE;
        s_data[6] = data_cnt;
        s_data[4] = i2c_addr;
        if (usb_setup(s_data) == 0)
        {
            ;//good
            usb_out(send_data, data_cnt);

            if (flags & EEPROM_SEND_STOP)
            {
                // Use the existing USB function Setup to process the stop command
                // as this handles all of the existing I2C logic
                s_data[1] = USBI2C_STOP;
                s_data[4] = 0;
                s_data[6] = 0;

                usb_setup(s_data);
            }
        }
        else
        {
            ;//bad
        }
        icnt+=data_cnt + 3;
    }

    modestat = 0;
    // for each packet, send to I2C
    */
}

// Main
__attribute__((naked))                                                  // suppress redundant SP initialization
extern int main (void)
{
    DDRD = (1<<0)|(1<<1);  // Set the port directions
    PORTB = (1<<PB2);     // Enable pullup on AVR reset control
    DDRB   = 0;     // Set portb input


    PORTB &= ~((1<<PB3));     // set AVR bootloader pins low, disable pullups
    PORTB &= ~((1<<PB4));
    PORTB &= ~((1<<PB5));
    //PORTB |= (1<<PB2);     // set AVR reset high

    wdt_enable(WDTO_2S);   // Enable the watchdog timer
    wdt_reset();

    //cli();
    // We need to pull USB + - low for at least 200ms to force
    // a renumeration of init
    /*DDRC = ((1<<PC0) | (1<<PC1));
    PORTC &= ~((1<<PC0) | (1<<PC1));

    _delay_ms(200);
    */ // Removed for now as it was causing problems with some hosts 22-04-09
    uart_init();
    //uart_puts("Welcome to OSIF serial port\n\r");
    //uart_puts("in>");
    

    usb_init();
    sei();
    i2c_init();
    io_init();
    sei();

    for	( ;; )
    {
        usb_poll();
        if (uart_poll() < 0) //uart_poll() only returns negative on UART rx/tx failure.
        {
            //Handle UART errors by resetting the UART hardware
            uart_init();
        }
        wdt_reset();

        // If there is an error on the bus store the status and reinitialise I2C hardware
        if ((TWSR & TWSRMASK) == TW_BUS_ERROR)
        {
            i2cstats[0x06]++;
            i2c_init();
        }
    }
    return 0;
}

ISR(USART_RXC_vect)
{
    byte_t data;

    rx_timeout = 0;
    cli();  //Disable global interrupts

    data = UDR;                 /* Read the received data */
    /* Calculate buffer index */

    if (UART_RxHead == UART_RxTail)
    {
        /* ERROR! Receive buffer overflow */
        UART_RxHead = 0;
    }

    UART_RxBuf[UART_RxHead++] = data; // Store received data in buffer

    sei(); //re-enable global interrupts
}
