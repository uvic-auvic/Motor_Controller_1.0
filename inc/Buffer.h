#ifndef BUFFER_H_
#define BUFFER_H_

#include<string.h>


#define MAX_BUFFER_DATA (8)
#define MAX_BUFFER_SIZE (8)

typedef struct Buffer{
	char data[MAX_BUFFER_SIZE][MAX_BUFFER_DATA];
	uint16_t head;
	uint16_t tail;
}Buffer;

//Public functions ------------------------------

extern void Buffer_add(Buffer* b, char* str);
extern void Buffer_pop(Buffer* b, char* data);
extern void Buffer_init();

//-----------------------------------------------

#endif
