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

// 
#include "common.h"
#include "i2c_master.h"
#include "uart.h"
#include "pwm_io.h"
#include "spi.h"

// EEPROM Flags
enum
{
    EEPROM_DIR_W            = 0x01,
    EEPROM_SEND_STOP        = 0x02,
};

void playback_start(void);

// force a reboot initiated by watchdog reset
void reboot(void)
{
    wdt_disable();
    wdt_enable(WDTO_2S);
    cli();
    while(1) ;;
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
        if (data[4])                                      // Read the high low status of the reset
        {
            RESET_PORT |= RESET_PIN;                      // Bring the reset line high
        }
        else
        {
            RESET_PORT &= ~RESET_PIN;                     // Bring the reset low
        }
        DDRC &= ~((1<<PC5));
        PORTC &= ~((1<<PC5));
        DDR  = DDRMASK;                                   // Reset spi port directions
        PORT = mask;                                      // setup the port mask

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
        io_set_ddr(data[4], data[2]);
        data[0] = 1;                                       // flag all ok
        return 0;
    }
    if  (req == USBIO_SET_OUT)
    {
        io_set_output(data[4]);
        data[0] = 1;
        return 0;
    }
    else if  (req == USBIO_GET_IN)
    {
        data[0] = 0x00;
        data[0] = io_get_in();
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
        pwm_set_rate(data[4]);
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
    DDRD  = (1<<0) | (1<<1);  // Set the port directions
    PORTB = (1<<PB2);         // Enable pullup on AVR reset control
    DDRB  = 0;                // Set portb input


    PORTB &= ~((1<<PB3));     // set AVR bootloader pins low, disable pullups
    PORTB &= ~((1<<PB4));
    PORTB &= ~((1<<PB5));
    //PORTB |= (1<<PB2);      // set AVR reset high

    wdt_enable(WDTO_2S);      // Enable the watchdog timer
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
        if (uart_poll() < 0)    // uart_poll() only returns negative on UART rx/tx failure.
        {
            // Handle UART errors by resetting the UART hardware
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

