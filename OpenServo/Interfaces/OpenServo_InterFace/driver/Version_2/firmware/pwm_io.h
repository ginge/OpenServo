#ifndef PWM_IO_H_INCLUDED
#define PWM_IO_H_INCLUDED

//functions
void io_init(void);
byte_t io_get_in(void);
void io_set_output(byte_t io_port);
void io_set_ddr(byte_t ddr_flags, byte_t enabled_flags);

void pwm_init(void);
void pwm_deinit(void);
void pwm_set_rate(byte_t rate);


typedef struct { 
    volatile uint8_t *p_ddr; 
    volatile uint8_t *p_port; 
    volatile uint8_t *p_pin;
    uint8_t bit;
    uint8_t mask; 
} port_table_type;

uint8_t         io_enabled;
uint8_t         io_ddr;
byte_t          modestat;        // Current enabled mode
byte_t          pwm_on;          // Enable PWM mode


#endif