#ifndef I2C_MASTER_H_INCLUDED
#define I2C_MASTER_H_INCLUDED
#include <util/twi.h>

#define I2C_TIMEOUT     6000000ul
#define TWSRMASK        0xF8


// Status flags for the I2C reading and writing
enum
{
    I2C_PACKET              = 0x01,
    I2C_READ                = 0x02,
    I2C_READ_ON             = 0x04,
    I2C_WRITE               = 0x08,
    I2C_STATUS              = 0x10,
};

byte_t  i2caddr;         // i2c address
byte_t  i2cstatspos;     // status register incremental pointer
byte_t  i2cstats[32];    // status array buffer
byte_t  i2cstat;         // status of I2C comms
byte_t  i2crecvlen;      // I2C buffer recieve length

int i2c_bitrate_set(int twbr_set, int twps_set);
void i2c_init();
inline int i2c_wait_int();
int i2c_send(byte_t sendbyte);
int i2c_read(void);
inline void i2c_stop(void);
int i2c_start(void);
void i2c_error(void);
int i2c_begin(byte_t i2caddr, byte_t *data, byte_t direction);
int i2c_read_bytes(byte_t *data,byte_t len);

#endif
