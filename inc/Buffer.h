#ifndef BUFFER_H_
#define BUFFER_H_

#include<string.h>


#define MAX_BUFFER_DATA (8)
#define MAX_BUFFER_SIZE (8)

typedef struct Buffer{
	char data[MAX_BUFFER_SIZE][MAX_BUFFER_DATA];
	unsigned short idx_to_load;
	unsigned short idx_to_pop;
	unsigned short overflow_cnt;
}Buffer;

//Public functions ------------------------------

extern void Buffer_add(Buffer* b, char* str);
extern void Buffer_pop(Buffer* b, char* data);
extern int Buffer_size(Buffer* b);
extern void Buffer_init();

//-----------------------------------------------

#endif
