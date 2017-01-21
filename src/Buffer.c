/*Buffer - Linked List implementation*/

#include "Buffer.h"
#include <stdlib.h>

extern void Buffer_add(Buffer* b, char* str){





	struct str_node* temp = (struct str_node*) malloc(sizeof(struct str_node));
	strcpy(temp->data, str);
	temp->next = NULL;

	if(b->size == 0){
		b->front = temp;
		b->rear = temp;
		b->size = b->size + 1;
		return;
	}
	b->rear->next = temp;
	b->rear = temp;
	b->size = b->size + 1;
}

// To DeBuffer an integer.
//Pass in the address to ensure variable scope
extern void Buffer_pop(Buffer* b, char* data) {
	if(b->head > b->tail) {
		strcpy(data, b->data[b->tail % MAX_BUFFER_SIZE]);
		b->tail++;
	}
}

extern void Buffer_init(Buffer* b){
	b->head = 0;
	b->tail = 0;
}

extern int Buffer_size(Buffer* b){
	return b->size;
}
