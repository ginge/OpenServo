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
#include <util/twi.h>
#define F_CPU 12000000UL  // 12 MHz
#include <util/delay.h>
#include "usb.h"



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
    USBI2C_READ = 20,
    USBI2C_WRITE,
    USBI2C_STOP,
    USBI2C_STATUS,
    USBIO_SET_DDR,
    USBIO_SET_OUT,
    USBIO_GET_IN
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
#define SDA             (0<<PC4)
#define SCL             (0<<PC5)

#define RESET_DDR       DDRC
#define RESET_PORT      PORTC
#define RESET_PIN       (1 << 4)
#define RESET_DDRMASK   RESET_PIN

// Programmer input pins:
//      MISO    PD3     (ACK)
#define PIN             PINB
#define MISO_MASK       (1 << 4)

#define I2C_TIMEOUT     6000

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

// ----------------------------------------------------------------------
// Delay exactly <sck_period> times 0.5 microseconds (6 cycles).
// ----------------------------------------------------------------------
__attribute__((always_inline))
        static	void	delay ( void )
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
static	void	spi ( byte_t* cmd, byte_t* res )
{
    byte_t	i;
    byte_t	c;
    byte_t	r;
    byte_t	mask;

    for	( i = 0; i < 4; i++ )
    {
        c = *cmd++;
        r = 0;
        for	( mask = 0x80; mask; mask >>= 1 )
        {
            if	( c & mask )
            {
                PORT |= MOSI_MASK;
            }
            delay();
            PORT |= SCK_MASK;
            delay();
            r <<= 1;
            if	( PIN & MISO_MASK )
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
static	void	spi_rw ( void )
{
    uint_t	a;

    a = address++;
    if	( cmd0 & 0x80 )
    {	// eeprom
        a <<= 1;
    }
    cmd[0] = cmd0;
    if	( a & 1 )
    {
        cmd[0] |= 0x08;
    }
    cmd[1] = a >> 9;
    cmd[2] = a >> 1;
    spi( cmd, res );
}

// Initialise the I2C hardware in AVR
void i2c_init()
{
    // Set the port directions and disable I2C pullups

    I2CPORT|=  SDA;                     // Disable pullups on I2C
    I2CPORT|=  SCL;
    I2CDDR |=  (SDA) | (SCL);               // Set I2C lines as output

    TWCR = 0;                           // Clear the control register
    TWSR &= ~(_BV(TWPS0) | _BV(TWPS1)); // Set the prescaler for twi in the status reegister
    TWBR = 10;                          // 10;//max bitrate for twi, ca 333khz by 12Mhz Crystal
    TWCR |= _BV(TWEN);                  // Enable I2C in control register
}

// Wait for the interupt flag to clear in the TWI hardware
// If locks because the interrupt didn't clear, we loop
// until a timeout peroid of I2C_TIMEOUT
int i2c_wait_int()
{
    int i =0;
    while((TWCR & _BV(TWINT)) == 0)
    {
        i++;
        if (i>I2C_TIMEOUT)
            return -1;
    }

    return 1;
}

// Send one byte of data over I2C
int i2c_send ( byte_t sendbyte )
{
    TWDR = sendbyte;                            // Fill Data register
    TWCR |= _BV(TWINT);                         // Send the byte
    if (i2c_wait_int() <0) return -1;           // Wait for the byte to send
    return TWSR;                                // Return the status
}

// Read one byte over I2C
static inline int i2c_read(void)
{
    if (i2c_wait_int() <0) return -1;           // Check to see if the interrupt is clear
    return TWDR;                                // Store the byte in the status register
}

// Send Stop condition
static inline void i2c_stop(void)
{
    TWCR |= _BV(TWINT) | _BV(TWSTO);            // Send the stop bit
}

// Send the Start condition
static inline int i2c_start(void)
{
    TWCR |= _BV(TWINT) | _BV(TWSTA);            // Send the I2C start command
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
    i2cstats[0x01] = TWSR;                                 // update the status array with the returned status
    int TWSRtmp = 0;

    // Start sending the I2C packet by following the normal I2C protocol sequence of Start data Stop
    // This USB control packet will contain the I2C address only so this function generates a start 
    // followed by the device address
    if(TWSR == TW_START || TWSR == TW_REP_START)           // If the start completed
    {
        if(direction == USBI2C_READ)
        {
            // Use the TWSRtmp temporary variable to store the return of the i2c_send function
            TWSRtmp = i2c_send ( i2caddr<<1 | 0x01 );      // Start the data send by sending device address + R
            TWCR &= ~_BV(TWSTA);                           // Disable the start flag in the control register
        }
        else
        {
            TWCR &= ~(_BV(TWSTA) | _BV(TWINT));            // Disable the start flag in the control register
            TWSRtmp = i2c_send ( i2caddr<<1 & 0xFE );      // Start the data send by sending device address + W
        }
        i2cstats[0x02] = TWSRtmp;                          // Store the return status in the status array
        if ( TWSRtmp < 0 ) i2c_error();
    }
    else                                                   // The start failed. Error handle
    {
        TWCR = _BV(TWINT) | _BV(TWEN);                     // disable all commands in the control register
        data[0] = i2cstats[0x00] = TWSR;                   // Store the return status in the status array
        if(direction == USBI2C_READ)
            i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON); // Disable the flags for reading
        else
            i2cstat &= ~(I2C_WRITE | I2C_PACKET);          // Disable the flags for reading
        return 0;
    }
    // Arbitration was lost, clean up and close. We are reading from the TWSRtmp temporary
    // variable which was filled with the return of the send command
    if(TWSRtmp == TW_MR_ARB_LOST  || TWSRtmp == TW_MR_SLA_NACK )
    {
        TWCR = _BV(TWEN);
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON);
        return 0;
    }
    // TWI Master Receiver Slave acknowledge. Working on the TWSRtmp variable
    // Checks to see if the start command and device selection generated an ACK
    if(TWSRtmp != TW_MR_SLA_ACK && direction == USBI2C_READ)
    {
        TWCR = _BV(TWEN);                                  // No ACK was generated. Disable pending flags in the Control register
        data[0] = i2cstats[0x00] = TWSRtmp;                // Store the status in the status array
        i2cstat &= ~(I2C_READ | I2C_PACKET | I2C_READ_ON); // Disable flags for reading in the status register
        return 0;
    }

    // TWI Master Transmitter Slave Not Ack
    // cleanup and disable writing to I2C
    if(TWSRtmp == TW_MT_SLA_NACK)
    {
        TWCR = _BV(TWINT) | _BV(TWEN);
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_WRITE | I2C_PACKET);
        return 0;
    }
    // TWI Master Transmitter Slave Ack
    if(TWSRtmp != TW_MT_SLA_ACK && direction == USBI2C_WRITE) // On no ACK cleanup and reset flags
    {
        TWCR = _BV(TWINT) | _BV(TWEN);                  //
        data[0] = i2cstats[0x00] = TWSRtmp;
        i2cstat &= ~(I2C_WRITE | I2C_PACKET);
        return 0;
    }

    if( direction == USBI2C_READ)
    {
        if(data[6] > 1 || data[7] > 0)
            TWCR |= _BV(TWEA);

        TWCR |= _BV(TWINT);                                // Clear the interrupt pending flag in the status register
    }
    return 1;
}

int i2c_read_bytes(byte_t *data,byte_t len)
{
    byte_t i;
    int read;

    for ( i = 0; i < len; i++ )
    {
        read = i2c_read();
        if (read < 0) return -1;
        data[i] = (byte_t)read;                            // Store the byte in the status register
        int TWSRtmp = TWSR;                                // Store the return status 
        i2cstats[0x08+i] = TWSRtmp;                        // Store the return status in the status array
        if(i2crecvlen > 0)
            i2crecvlen--;
        if(i2crecvlen == 1)
        {
            TWCR = (TWCR | _BV(TWINT)) & ~_BV(TWEA);        // Dont Send the ACK to the TWI slave
        }
        else
        {
            TWCR |= _BV(TWINT);                             //
        }
        if(TWSRtmp == TW_MR_DATA_NACK)                      // If we get a no ACK from the slave, dont read on
            i2cstat &= ~I2C_READ_ON;
        TWCR |= _BV(TWINT);
    }
    return 0;
}

// Handle a non-standard SETUP packet over USB
extern	byte_t	usb_setup ( byte_t data[8] )
{
    byte_t      bit;
    byte_t      mask;
    byte_t*     addr;
    byte_t      req;
    uint8_t     x;

    // Generic requests
    req = data[1];
    if      ( req == USBTINY_ECHO )
    {
        return 8;
    }
    // I2C requests

    // Send the stop command over the I2C bus
    if ( req == USBI2C_STOP)
    {
        if((i2cstat & (I2C_READ | I2C_WRITE)) != 0)  // Send the stop signal if we are reading or writing
        {
            i2c_stop();
            i2cstat = 0;                             // Clear the status flags variable
            return 0;
        }
        data[0] = i2cstat;                           // Return the status flags in the data buffer
        return 1;
    }
    // Handle a status read request
    else if ( req == USBI2C_STATUS)
    {
        i2cstat |= I2C_STATUS | I2C_PACKET;           // Update the status flag to enable I2C status data
        i2cstatspos = 0;                              // Reset the int pointer to the data position
        return 0xff;                                  // Will be handled by usb_in
    }
    // Handle an I2C read request
    else if ( req == USBI2C_READ)
    {
        i2cstats[0x00] = 0;                          // Clear the status register
        i2cstats[0x04] = data[6];                    // Set the status
        i2cstats[0x05] = data[7];
        if(data[7] == 0)
        {
            i2crecvlen = data[6];
        }
        else
        {
            i2crecvlen = 0;
        }
        i2cstat = (i2cstat | I2C_READ | I2C_PACKET) & ~I2C_STATUS ;  // Add the read flag to the status variable
        if(i2caddr != data[4] || (i2cstat & I2C_READ_ON) == 0)
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
        if(i2caddr != data[4] || (i2cstat & I2C_WRITE) == 0)    // Check to see is write is enabled in the flags
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
    if	( req == USBTINY_READ )                          // SPI read command
    {
        data[0] = *addr;
        return 1;
    }
    if	( req == USBTINY_WRITE )                         // SPI write
    {
        *addr = data[2];
        return 0;
    }
    bit = data[2] & 7;                                   // mask the SPI
    mask = 1 << bit;
    if	( req == USBTINY_CLR )                           // Clear command
    {
        *addr &= ~ mask;
        return 0;
    }
    if	( req == USBTINY_SET )                           // SPI set
    {
        *addr |= mask;
        return 0;
    }

    // Programming requests
    // Requred to enable the SPI hardware in the OSIF and disable I2C
    if	( req == USBTINY_POWERUP )
    {
        TWCR = 0;                                         // Disable I2C
        TWSR = 0;                                         // Clear the status register
        sck_period = data[2];                             // Store the clock period
        mask = DDRMASK;                                   // setup the port mask
        RESET_DDR |= RESET_PIN;                           // Set the port direction for the reset line
        if ( data[4] )                                    // Read the high low status of the reset
        {
            RESET_PORT |= RESET_PIN;                      // Bring the reset line high
        }
        else
            RESET_PORT &= ~RESET_PIN;                     // Bring the reset low
        DDR  = DDRMASK;                                   // Reset spi port directions
        PORT = mask;                                      // setup the port mask
        return 0;
    }
    // Disable SPI mode and enable I2C
    if	( req == USBTINY_POWERDOWN )
    {
        RESET_PORT |= RESET_PIN;                          // Bring the reset low
        i2c_init();                                       // Initialise the I2C module

        DDR  &= ~DDRMASK;                                 // Reset the spi port to defaults
        return 0;
    }
    if	( ! PORT )
    {
        return 0;
    }
    if	( req == USBTINY_SPI )                            // SPI
    {
        spi( data + 2, data + 0 );
        return 4;
    }
    if	( req == USBTINY_POLL_BYTES )
    {
        poll1 = data[2];
        poll2 = data[3];
        return 0;
    }
    address = * (uint_t*) & data[4];
    if	( req == USBTINY_FLASH_READ )
    {
        cmd0 = 0x20;
        return 0xff;	                                   // usb_in() will be called to get the data
    }
    if	( req == USBTINY_EEPROM_READ )
    {
        cmd0 = 0xa0;
        return 0xff;	                                   // usb_in() will be called to get the data
    }
    timeout = * (uint_t*) & data[2];
    if	( req == USBTINY_FLASH_WRITE )
    {
        cmd0 = 0x40;
        return 0;	                                   // data will be received by usb_out()
    }
    if	( req == USBTINY_EEPROM_WRITE )
    {
        cmd0 = 0xc0;
        return 0;	                                   // data will be received by usb_out()
    }
    // Set an IO line high or low
    if  ( req == USBIO_SET_DDR )
    {
        io_ddr = data[4];                                  // This holds the direction bits
        io_enabled = data[6];                              // This holds the IO enabled status
        // Set the port directions
        for ( x=0; x<PORT_TABLE_IO_COUNT; x++)
        {
          if (io_enabled & (1<<x))              // Check bit 0 for IO dir
          {
            // io_tx
            if (io_ddr & (1<<x))             // if set as output
            {
                *port_table[x].p_ddr |= _BV(port_table[x].bit);
            }
            else                        //input
            {
                *port_table[x].p_ddr &= ~_BV((port_table[x].bit));
            }
          }
        }
        data[0] = 1;                                       // flag all ok
        return 0;
    }
    if  ( req == USBIO_SET_OUT )
    {
        // check to make sure it is an output
        for ( x=0; x<PORT_TABLE_IO_COUNT; x++)
        {
            if ((io_enabled & (1<<x)) && (io_ddr & (1<<x)))              // Check bit 0 for IO dir
            {
                if ( data[4] & (1<<x) )              // is it high?
                {
                    *port_table[x].p_port &= ~_BV((port_table[x].bit));
                }
                else
                {
                    *port_table[x].p_port |= _BV((port_table[x].bit));
                }
            }
        }
        data[0] = 1;
        return 0;
    }
    if  ( req == USBIO_GET_IN )
    {
        // read anyways, regardless of ststus
        for ( x=0; x<PORT_TABLE_IO_COUNT; x++)
        {
            if (bit_is_clear((port_table[x].p_pin), (port_table[x].bit)))
                data[0] ^= (1<<x);
        }
        return 0;
    }
    return 0;
}


// Handle an IN packet over USB
extern	byte_t	usb_in ( byte_t* data, byte_t len )
{
    byte_t	i;
    if((i2cstat & I2C_PACKET) != 0)                           // Make sure we are in packet reading more in the status register
    {
        if((i2cstat & I2C_STATUS) != 0){                      // See if we are sending back the status array
            for	( i = 0; i < len; i++ )
            {
                data[i]=i2cstats[i2cstatspos++];              // Send back the byte of the status array, and increment pointer
            }
        }
        else if ((i2cstat & I2C_READ) != 0){                  // I2C Read in status flag
            if (i2c_read_bytes(data, len) < 0) return 0;
        }
    }
    else                                                      // SPI mode read
    {
        for	( i = 0; i < len; i++ )                       // For each byte send the spi
        {
            spi_rw();
            data[i] = res[3];
        }
    }
    return len;
}

// Handle an OUT packet over USB
extern	void	usb_out ( byte_t* data, byte_t len )
{
    byte_t	i;
    uint_t	usec;
    byte_t	r;
    if((i2cstat & I2C_PACKET) != 0x0)                                   // Check to see if we are in packet mode
    {
        for	( i = 0; i < len; i++ )
        {
            int TWSRtmp = i2c_send ( data[i] );                      // Send the data over I2C
            if(TWSRtmp != TW_MT_DATA_ACK 
                       && TWSRtmp != TW_MT_DATA_NACK
                       && TWSRtmp != -1)                                // Check for an NACK in TWSR
            {
                i2cstats[0x03] = i2cstats[0x00] = TWSRtmp;              // Store the return status in the status array
                i2c_stop();                                             // Send stop. Close I2C
                i2cstat = 0;                                            // Clear all status flags
            }
        }
    }
    else                                                                // SPI write
    {
        for	( i = 0; i < len; i++ )
        {
            cmd[3] = data[i];
            spi_rw();
            cmd[0] ^= 0x60;	                                        // turn write into read
            for	( usec = 0; usec < timeout; usec += 32 * sck_period )
            {	                                                        // when timeout > 0, poll until byte is written
                spi( cmd, res );                                        // Send the SPI packet
                r = res[3];
                if	( r == cmd[3] && r != poll1 && r != poll2 )
                {
                    break;
                }
            }
        }
    }
}

void io_init(void)
{
    io_ddr = 0;
    io_enabled = 0;
}
// Main
__attribute__((naked))                                                  // suppress redundant SP initialization
extern int main ( void )
{
    DDRD = (1<<0)|(1<<1);  // Set the port directions
    PORTB |= _BV(PB2);     // Enable pullup on AVR reset control
    DDRB   = _BV(PB2);     // Set portb output
    PORTB |= _BV(PB2);     // set AVR reset high

    PORTD |= _BV(PD3);
    wdt_enable(WDTO_2S);   // Enable the watchdog timer
    wdt_reset();

    // We need to pull USB + - low for at least 200ms to force
    // a renumeration of init
    PORTC |= _BV(PC0) | _BV(PC1);
    _delay_ms(200);

    usb_init();
    i2c_init();
    io_init();

    for	( ;; )
    {
        usb_poll();
        wdt_reset();

        // If there is an error on the bus store the status and reinitialise I2C hardware
        if(TWSR == TW_BUS_ERROR){
            i2cstats[0x06]++;
            i2c_init();
        }
    }
    return 0;
}
