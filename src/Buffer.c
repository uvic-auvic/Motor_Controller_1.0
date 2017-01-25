/*Buffer - Circular FIFO Buffer implementation*/

#include "Buffer.h"
#include <stdlib.h>

// Adds an element to the end of the queue
//  - str (the element) must be NULL terminated ('\0') for strcpy
extern void Buffer_add(Buffer* b, const char* str){
	// Insert element
	strcpy(b->data[b->idx_to_load], str);
	b->idx_to_load++;
	b->idx_to_load %= MAX_BUFFER_SIZE;
	b->size++;

	// Check if buffer is full
	if(b->size > MAX_BUFFER_SIZE)
	{
		// Remove oldest element
		b->idx_to_pop++;
		b->idx_to_pop %= MAX_BUFFER_SIZE;
		b->size--;
		b->overflow_cnt++;
	}
}

// Removes an element from the front of the queue
extern void Buffer_pop(Buffer* b, char* data) {
	// Check if the buffer has anything to pop
	if(b->size)
	{
		// Pop oldest element and store it in data
		strcpy(data, b->data[b->idx_to_pop]);
		b->idx_to_pop++;
		b->idx_to_pop %= MAX_BUFFER_SIZE;
		b->size--;
	}
}

// Reset all variables of the buffer
extern void Buffer_init(Buffer* b){
	b->idx_to_load = 0;
	b->idx_to_pop = 0;
	b->size = 0;
	b->overflow_cnt = 0;
}

// Get the size of the buffer
extern int Buffer_size(Buffer* b){
	return b->size;
}

// Get the number of overflows that have occurred
extern int Buffer_overflow(Buffer* b){
	return b->overflow_cnt;
}
