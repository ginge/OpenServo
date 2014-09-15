#include "common.h"
#include "spi.h"

// ----------------------------------------------------------------------
// Delay exactly <sck_period> times 0.5 microseconds (6 cycles).
// ----------------------------------------------------------------------
__attribute__((always_inline))
static void delay(void)
{
    asm volatile(
            "   mov     __tmp_reg__,%0  \n"
            "0: rjmp    1f              \n"
            "1: nop                     \n"
            "   dec     __tmp_reg__     \n"
            "   brne    0b              \n"
            : : "r" (sck_period) );
}

// Issue one SPI command.
void spi(byte_t* cmd, byte_t* res)
{
    byte_t      i;
    byte_t      c;
    byte_t      r;
    byte_t      mask;

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
void spi_rw (void)
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
