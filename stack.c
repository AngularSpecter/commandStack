#include "msp430g2553.h"
#include "stack.h"
#include "stdint.h"
#include "string.h"

const uint8_t READY = 1;
const uint8_t OVERFLOW = 2;

void buffer_init(command_stack* stack)
{
	memset(stack, 0, sizeof(command_stack));
    flush_stack(stack);
}


/*Functions to manipulate the state of the stack*/

// Advance the stack to the next buffer
void advance_stack(command_stack* stack)
{
	//advance the stack pointer and reset to 0 on wrap
    stack->stack_index++;
    if(stack->stack_index == STACK_LEN) stack->stack_index = 0;

    //reset the buffer pointer to the beginning
    stack->buffer_index[stack->stack_index] = 0;
    stack->buffer[stack->stack_index][0]    = '\0';
}


// Write a single character to the active buffer element
void write_char(command_stack* stack, char c)
{
	//Find the stack index and buffer index
	uint8_t si = stack->stack_index;
	uint8_t bi = stack->buffer_index[si];

	//if the buffer index is the same as the buffer length
	//we are overflowing the buffer, so handle it
	if (bi == BUFFER_LEN )
	{
		stack->status[stack->stack_index] |= OVERFLOW;
	}else{
		stack->buffer[si][bi]   = c;	   //store the character
		stack->buffer[si][bi+1] = '\0';    //move the string end char
		stack->buffer_index[si]++;         //advance the pointer
	}
}


//Write an entire string into the buffer.  Return the number of chars written
uint8_t write_string(command_stack* stack, char* string)
{
	uint8_t old_idx = stack->buffer_index[stack->stack_index];

	//write each character to the buffer
	while(*string) write_char(stack, *string++);

	//if there is overflow, the buffer stops writing, so determine the
	//difference in the buffer pointers.
	return stack->buffer_index[stack->stack_index] - old_idx;

}


//Change the status flag on a buffer to indicate it is ready or not
//ready to be processed
void flag_buffer(command_stack* stack, uint8_t buffer, uint8_t value)
{
	if(value)
	{
		stack->status[buffer] |= READY;
	}else{
		stack->status[buffer] &= ~READY;
	}

}

//Mark current buffer as 'ready' and advance to the next for writing
void commit_buffer(command_stack* stack)
{
	flag_buffer(stack, stack->stack_index, 1);
	advance_stack(stack);
}

/* Functions to read the buffers */

/* Start at the active buffer and look forward (to older buffers) to
 * find one that is marked ready to process.  If there are no buffers
 * that can be processed, return -1.
 */
uint8_t  oldest_ready(command_stack* stack, uint8_t* index)
{
	uint8_t i ;
	uint8_t j = (stack->stack_index + 1 == STACK_LEN) ? 0 : stack->stack_index + 1;

	//iterate over each element in the stack, checking to see if the status flag
	//is marked as READY.  If it is, return success, else advance to the next element.
	for (i = 0 ; i < STACK_LEN; i++ )
	{
		*index =  j;
		if (stack->status[j] & READY ) return 1;
		j = (j == STACK_LEN - 1) ? 0 : j+1;
	}

    return 0;
}

//Read the string stored in the current buffer
uint8_t read_current_buffer(command_stack* stack, char* string)
{
	string = stack->buffer[stack->stack_index];
	return strlen(string);
}


uint8_t read_buffer(command_stack* stack, char* string, uint8_t buffer)
{

	int i = 0;

	//buffer_index -1 marks the end of the data that was last written to
	//the buffer, so only read up to that point
	for (i = 0; i < stack->buffer_index[buffer]; i++)
	{
           string[i] = stack->buffer[buffer][i];
	}
	string[i] = '\0';
	return i;
}

/* Functions that provide maintenance to the stack ---------------------------------*/

//reset all elements of the buffer to 0 and reset the write pointet
void flush_buffer(command_stack* stack, uint8_t index)
{
	uint8_t i = 0;
	for (i = 0; i < BUFFER_LEN ; i++) stack->buffer[index][i] = 0;
	stack->buffer_index[index] = 0;
	stack->buffer[index][0]    = '\0';
}


//flush the entire stack
void flush_stack(command_stack* stack)
{
	uint8_t i = 0;
	for (i = 0; i < STACK_LEN ; i ++)
	{
		flush_buffer(stack, i);
		stack->buffer_index[i] = 0;
		stack->status[i]       = 0;
	}
	stack->stack_index  = 0;

}


