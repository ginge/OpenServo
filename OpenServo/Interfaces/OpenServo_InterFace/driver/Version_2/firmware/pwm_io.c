#include "common.h"
#include "pwm_io.h"

// this is the count of the number of elements in this list.
#define PORT_TABLE_IO_COUNT 6

static port_table_type port_table[] = { 
    { &DDRD, &PORTD, &PIND, PD1, 0x00 },   // TX line
    { &DDRD, &PORTD, &PIND, PD0, 0x00 },   // RX line
    { &DDRB, &PORTB, &PINB, PB4, 0x00 },   // MISO
    { &DDRB, &PORTB, &PINB, PB3, 0x00 },   // MOSI
    { &DDRC, &PORTC, &PINC, PC4, 0x00 },   // SDA
    { &DDRC, &PORTC, &PINC, PC5, 0x00 }    // SCL
                                      };

                                      
/*
 * PWM Related
 */
void pwm_init(void)
{
    TCCR2 = (0<<FOC2)|(1<<WGM21)|(1<<WGM20)|(1<<CS20)|(0<<CS21)|(0<<CS22)|(1<<COM21)|(0<<COM20);                    // setup PWM, no prescaler
    OCR2 = 0x00;
    DDRB |= (1<<PWM_PIN);
    pwm_on = 1; 
}

void pwm_deinit(void)
{
    TCCR0 ^=(0<<COM21)|(0<<COM20); // disable OC2
    pwm_on = 0; 
}

void pwm_set_rate(byte_t rate)
{
    if (rate == 0)
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
            OCR2 = rate;
        }
    }
}

/*
 * IO Section
 */
void io_init(void)
{
    io_ddr = 0;
    io_enabled = 0;
}

void io_set_ddr(byte_t ddr_flags, byte_t enabled_flags)
{
    uint8_t     x;
    io_ddr = ddr_flags;                       // This holds the direction bits
    io_enabled = enabled_flags;               // This holds the IO enabled status
    // Set the port directions
    for (x = 0; x < PORT_TABLE_IO_COUNT; x++)
    {
        if (io_enabled & (1<<x))              // Check bit 0 for IO dir
        {
            // io_tx
            if (io_ddr & (1<<x))                  // if set as output
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
}

void io_set_output(byte_t io_port)
{
    uint8_t     x;
    
    // check to make sure it is an output
    for (x = 0; x<PORT_TABLE_IO_COUNT; x++)
    {
        if (io_enabled & (1<<x))                 // Check bit 0 for IO dir
        {
            if (io_port & (1<<x))                // is it high?
            {
                *port_table[x].p_port &= ~(1<<(port_table[x].bit));
            }
            else
            {
                *port_table[x].p_port |= (1<<(port_table[x].bit));
            }
        }
    }
}

// Read the entire port array as a single byte
byte_t io_get_in(void)
{
    uint8_t     x;
    byte_t tmp_io_val = 0x00;
    
    // read anyways, regardless of status
    for (x = 0; x < PORT_TABLE_IO_COUNT; x++)
    {
        if (*port_table[x].p_pin & (1<<port_table[x].bit))
            tmp_io_val |= (1<<x);
    }
    
    return tmp_io_val;
}
