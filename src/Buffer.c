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
	struct str_node* temp = b->front;
	if(b->front == NULL) {
		return;
	}
	strcpy(data, b->front->data);
	if(b->front == b->rear) {
		b->front = b->rear = NULL;
	}
	else {
		b->front = b->front->next;
	}
	free(temp);
	b->size = b->size -1;
}

extern Buffer* Buffer_init(){
	Buffer* b = (Buffer*) malloc(sizeof(Buffer));
	b->size = 0;
	b->front = NULL;
	b->rear = NULL;
	return b;
}

extern int Buffer_size(Buffer* b){
	return b->size;
}
