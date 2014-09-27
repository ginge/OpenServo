#include "common.h"
#include "i2c_master.h"

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
inline int i2c_wait_int()
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
int i2c_send(byte_t sendbyte)
{
    TWDR = sendbyte;                            // Fill Data register
    TWCR |= (1<<TWINT);                         // Send the byte
    if (i2c_wait_int() < 0) return -1;           // Wait for the byte to send
    return TWSR & TWSRMASK;                                // Return the status
}

// Read one byte over I2C
int i2c_read(void)
{
    if (i2c_wait_int() < 0) return -1;           // Check to see if the interrupt is clear
    return TWDR;                                // Store the byte in the status register
}

// Send Stop condition
inline void i2c_stop(void)
{
    TWCR |= (1<<TWINT) | (1<<TWSTO);            // Send the stop bit
}

// Send the Start condition
int i2c_start(void)
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
        i2c_stop();
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

int i2c_read_bytes(byte_t *data, byte_t len)
{
    byte_t i;
    int read;

    for (i = 0; i < len; i++)
    {
        read = i2c_read();
        int TWSRtmp = (TWSR & TWSRMASK);                   // Store the return status
        /*if (read < 0)
        {
            return -1;                                     // Really bad!
        }*/
        data[i] = (byte_t)read;                            // Store the byte in the status register
        i2cstats[0x08+i] = TWSRtmp;                        // Store the return status in the status array
        
        if (TWSRtmp == TW_MR_DATA_NACK)                    // If we get a no ACK (NACK) from the slave, dont read on
        {
            // 16-09-14: This logic actually serves little purpose. According to the I2C spec, the slave
            // when xmitting will not be able to NACK as the receiver controls the ACK/NACK bit.
            // This function actually is called when the master reader is ready to terminate
            // the read.
            i2cstat &= ~I2C_READ_ON;                       // Bail out of the loop now. Slave says no more data
//            i2c_stop();                                  // commented out -- API should do this, although that behaviour is questionable at best.
            return i + 1;                                  // Return the real length
        }
        
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
            return i + 1;                                  // Return the real length
        }
        
        TWCR |= (1<<TWINT);                                // ACK and Start SCL clocking
    }
    return len;
}
