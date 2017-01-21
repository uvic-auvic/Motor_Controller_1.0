/*Buffer - Linked List implementation*/

#include "Buffer.h"
#include <stdlib.h>

extern void Buffer_add(Buffer* b, char* str){
	if((b->idx_to_load - b->idx_to_pop) >= MAX_BUFFER_SIZE)
	{
		b->idx_to_pop++;
		b->overflow_cnt++;
	}

	strcpy(b->data[b->idx_to_load % MAX_BUFFER_SIZE], str);
	b->idx_to_load++;
}

// To DeBuffer an integer.
//Pass in the address to ensure variable scope
extern void Buffer_pop(Buffer* b, char* data) {
	if(b->idx_to_load > b->idx_to_pop)
	{
		strcpy(data, b->data[b->idx_to_pop % MAX_BUFFER_SIZE]);
		b->idx_to_pop++;
	}
}

extern void Buffer_init(Buffer* b){
	b->idx_to_load = 0;
	b->idx_to_pop = 0;
}

extern int Buffer_size(Buffer* b){
	return (b->idx_to_load - b->idx_to_pop);
}
