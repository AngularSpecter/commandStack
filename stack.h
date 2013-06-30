/*
 * stack.h
 *
 *  Created on: Jun 29, 2013
 *      Author: mjbeals
 */
#include "stdint.h"

#ifndef STACK_H_
#define STACK_H_



#define BUFFER_LEN 8
#define STACK_LEN 4
typedef struct
{
	char buffer[STACK_LEN][BUFFER_LEN + 1];		//buffer data store.
	uint8_t status[STACK_LEN];              // 0x1 = process status; 0x2 = overflow status
	uint8_t stack_index;					//which buffer are we writing to
	uint8_t buffer_index[STACK_LEN];		//which element of the buffer

}command_stack;


void advance_stack(command_stack* stack);
void write_char(command_stack* stack, char c);
uint8_t write_string(command_stack* stack, char* string);
void flag_buffer(command_stack* stack, uint8_t buffer, uint8_t value);
void commit_buffer(command_stack* stack);


void buffer_init(command_stack* stack);
void flush_buffer(command_stack* stack, uint8_t index);
void flush_stack(command_stack* stack);

uint8_t oldest_ready(command_stack* stack, uint8_t* index);
uint8_t read_current_buffer(command_stack* stack, char* string);
uint8_t read_buffer(command_stack* stack, char* string, uint8_t buffer);

#endif /* STACK_H_ */
