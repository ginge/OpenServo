#include "common.h"
#include "uart.h"
#include "i2c_master.h"

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


ISR(USART_RXC_vect)
{
    byte_t data;

    rx_timeout = 0;
    cli();                  // Disable global interrupts

    data = UDR;             // Read the received data
    // Calculate buffer index

    if (UART_RxHead == UART_RxTail)
    {
        // ERROR! Receive buffer overflow
        UART_RxHead = 0;
    }

    UART_RxBuf[UART_RxHead++] = data; // Store received data in buffer

    sei(); // re-enable global interrupts
}
