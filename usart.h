/*
 * usart.h
 *
 *  Created on: Jun 29, 2013
 *      Author: mjbeals
 */
#include "stdint.h"

#ifndef USART_H_
#define USART_H_



void setup_UART(void);
uint8_t RX_ready();
int8_t oldest_RX(char* buf);

uint8_t enque_buffer(char* string);
uint8_t TX_ready();
int8_t oldest_TX(char* buf);
uint8_t process_tx(int buffers);
void uart_puts(char *str);
void uart_putc(char c);


#endif /* USART_H_ */
