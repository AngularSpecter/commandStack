#include "msp430g2553.h"
#include "usart.h"
#include "stdint.h"
#include "stack.h"

const unsigned long smclk_freq = 1000000;      // SMCLK frequency in hertz
const unsigned bps = 9600;                     // Async serial bit rate

volatile unsigned int  tx_flag;			//Mailbox Flag for the tx_char.
volatile unsigned char tx_char;			//This char is the most current char to go into the UART

volatile unsigned char rx_flag;			//Mailbox Flag for the rx_char.
volatile unsigned char command_ready;

command_stack rx_stack; //incoming data
command_stack tx_stack; //outgoing data

/*--------------------------------------------------------------------------------------*/
void setup_UART(void)
{
	buffer_init(&rx_stack);
	buffer_init(&tx_stack);

	P1SEL = BIT1 + BIT2;					//Setup the I/O
	P1SEL2 = BIT1 + BIT2;

	UCA0CTL1 |= UCSSEL_2; 				//SMCLK
										//8,000,000Hz, 9600Baud, UCBRx=52, UCBRSx=0, UCBRFx=1
	UCA0BR0 = 52;                  		//8MHz, OSC16, 9600
	UCA0BR1 = 0;                   	 	//((8MHz/9600)/16) = 52.08333
	UCA0MCTL = 0x10|UCOS16; 			//UCBRFx=1,UCBRSx=0, UCOS16=1
	UCA0CTL1 &= ~UCSWRST; 				//USCI state machine
	IE2 |= UCA0RXIE; 					//Enable USCI_A0 RX interrupt

	rx_flag = 0;						//Set rx_flag to 0
	tx_flag = 0;						//Set tx_flag to 0

	return;						//Set tx_flag to 0

}

/* Functions for handling incomming data
 *
 */
uint8_t RX_ready()
{
	uint8_t index;
	return (oldest_ready(&rx_stack, &index) ) ? 1 : 0;
}

int8_t oldest_RX(char* buf)
{
	uint8_t index;

	int8_t ready =  oldest_ready(&rx_stack, &index);
	if (ready == 0) return 0;

	read_buffer(&rx_stack, buf, (uint8_t) index);
	flag_buffer(&rx_stack, index, 0);
	return 1;
}

#pragma vector = USCIAB0RX_VECTOR		//UART RX USCI Interrupt. This triggers when the USCI receives a char.
__interrupt void USCI0RX_ISR(void)
{
	char data = UCA0RXBUF;				//Copy from RX buffer, in doing so we ACK the interrupt as well
	write_char(&rx_stack,data);

	//if it is a line term char, roll the stack
	if(data == 0x0D)
	{
	   rx_flag = 1;
	   commit_buffer(&rx_stack);
	}
}

/*Functions for handling sending data
 *
 */

uint8_t enque_buffer(char* string)
{
	write_string(&tx_stack, string);
	commit_buffer(&tx_stack);
	return 1;
}



uint8_t TX_ready()
{
	uint8_t index;
	return (oldest_ready(&tx_stack, &index)) ? 1 : 0;
}

int8_t oldest_TX(char* buf)
{
	uint8_t index;

	int8_t ready;
	ready =  oldest_ready(&tx_stack, &index);

    if (ready == 0) return 0;

	read_buffer(&tx_stack, buf, (uint8_t) index);
	flag_buffer(&tx_stack, index, 0);
	return 1;
}


uint8_t process_tx(int buffers)
{
	char tx_buffer[BUFFER_LEN];
	int i;

	for (i = 0; i <= buffers; i++)
	{
		int valid = oldest_TX(tx_buffer);

		if (valid)
		{
			uart_puts(tx_buffer);
		}else{
			return i;
		}
	}

	return i;

}

void uart_putc(char c)
{
	tx_char = c;						//Put the char into the tx_char
	IE2 |= UCA0TXIE; 					//Enable USCI_A0 TX interrupt
	while(tx_flag == 1);				//Have to wait for the TX buffer
	tx_flag = 1;						//Reset the tx_flag
	return;
}

void uart_puts(char *str)				//Sends a String to the UART.
{
     while(*str) uart_putc(*str++);		//Advance though string till end
     return;
}


#pragma vector = USCIAB0TX_VECTOR		//UART TX USCI Interrupt
__interrupt void USCI0TX_ISR(void)
{
	UCA0TXBUF = tx_char;				//Copy char to the TX Buffer
	tx_flag = 0;						//ACK the tx_flag
	IE2 &= ~UCA0TXIE; 					//Turn off the interrupt to save CPU
}

