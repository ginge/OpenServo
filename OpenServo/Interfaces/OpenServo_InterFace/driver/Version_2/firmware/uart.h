#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

void uart_init();
int uart_poll();


// Serial mode flags
enum
{
    SER_MODE_READ           = 0x02,
    SER_MODE_WRITE          = 0x04,
};

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


unsigned char UART_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART_RxHead;
static volatile unsigned char UART_RxTail;
uint8_t serstat;
uint16_t rx_timeout;
uint16_t tx_timeout;

int8_t uart_putc(uint8_t c);
int8_t uart_puts(char *s);

#endif