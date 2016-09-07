#ifndef BUFFER_H_
#define BUFFER_H_

#include<string.h>


#define MAX_BUFFER_DATA (8)

struct str_node{
	char data[MAX_BUFFER_DATA];
	struct str_node* next;
};

typedef struct Buffer{
	int size;
	struct str_node* front;
	struct str_node* rear;
}Buffer;

//Public functions ------------------------------

extern void Buffer_add(Buffer* b, char* str);
extern void Buffer_pop(Buffer* b, char* data);
extern Buffer* Buffer_init();
extern int Buffer_size(Buffer* b);

//-----------------------------------------------

#endif
