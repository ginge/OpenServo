#ifndef SPI_H_INCLUDED
#define SPI_H_INCLUDED

// Local data
byte_t                 poll1;           // first poll byte for write
byte_t                 poll2;           // second poll byte for write
uint_t                 timeout;         // write timeout in usec
static byte_t          sck_period;      // SCK period in microseconds (1..250)
static uint_t          address;         // read/write address
static byte_t          cmd0;            // current read/write command byte
static byte_t          cmd[4];          // SPI command buffer
static byte_t          res[4];          // SPI result buffer

void spi(byte_t* cmd, byte_t* res);
void spi_rw (void);

#endif