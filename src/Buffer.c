/*Buffer - Circular FIFO Buffer implementation*/

#include "Buffer.h"
#include <stdlib.h>

// Adds an element to the end of the queue
extern void Buffer_add(Buffer* b, char* str){
	// Check if buffer is full
	if((b->idx_to_load - b->idx_to_pop) >= MAX_BUFFER_SIZE)
	{
		// Remove oldest element
		b->idx_to_pop++;
		b->overflow_cnt++;
	}

	// Insert element
	strcpy(b->data[b->idx_to_load % MAX_BUFFER_SIZE], str);
	b->idx_to_load++;
}

// Removes an element from the front of the queue
extern void Buffer_pop(Buffer* b, char* data) {
	// Check if the buffer has anything to pop
	if(b->idx_to_load > b->idx_to_pop)
	{
		// Pop oldest element and store it in data
		strcpy(data, b->data[b->idx_to_pop % MAX_BUFFER_SIZE]);
		b->idx_to_pop++;
	}
}

// Reset all variables of the buffer
extern void Buffer_init(Buffer* b){
	b->idx_to_load = 0;
	b->idx_to_pop = 0;
	b->overflow_cnt = 0;
}

// Get the size of the buffer
extern int Buffer_size(Buffer* b){
	return (b->idx_to_load - b->idx_to_pop);
}

// Get the number of overflows that have occurred
extern int Buffer_overflow(Buffer* b){
	return b->overflow_cnt;
}
