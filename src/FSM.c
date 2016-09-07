#include "FSM.h"
#include "motors.h"
#include <string.h>
#include "PWM_in.h"

#define MIN_COMMAND_LENGTH (4)
#define MAX_COMMAND_LENGTH (5)

//Local Global Variables
Buffer* input_buff;
Buffer* output_buff;

static void comm_exec(char* comm, unsigned int arg){
	if( strcmp(comm, "M1F") == 0){
		Motor_Speed(right_side, arg, Forward);
	}
	else if(strcmp(comm, "M1R") == 0){
		Motor_Speed(right_side, arg, Reverse);
	}
	else if(strcmp(comm, "RV1") == 0){
		char temp[] = {read_revoultions(right_side),'\r','\n', '\0'};
		Send_to_Odroid(temp);
	}
	else if(strcmp(comm, "DT1") == 0){
			char temp[] = {read_duty_cycle(right_side),'\r','\n', '\0'};
			Send_to_Odroid(temp);
	}
	else{
		Send_to_Odroid("ERR: ");
		Send_to_Odroid(comm);
		Send_to_Odroid("  \r\n");
	}
}

extern void FSM_do(){
	if(Buffer_size(input_buff)){
		char data[MAX_COMMAND_LENGTH];
		Buffer_pop(input_buff, data);
		char comm[4] = {data[0], data[1], data[2], '\0'};
		int arg = (unsigned int) data[3];
		comm_exec(comm, arg);
	}
	//Just polling for now as it is easiest
	if(Buffer_size(output_buff)){
		if(check_UART_busy() == 0){
			char mesg[MAX_COMMAND_LENGTH];
			Buffer_pop(output_buff, mesg);
			UART_push_out(mesg);
		}
	}
}


void command_recv(char *comm, int comm_length){
	//Light to show that the command was received
	GPIOC->ODR ^= GPIO_ODR_8;
	//Buffer the data so that we can exit the interrupt
	//buffer_add(comm, comm_length);
	//need to implement
	Buffer_add(input_buff, comm);
}

//Message currently must be under five characters counting \0
//Check to see if we can send directly at this point
extern void Send_to_Odroid(char* mesg){
	Buffer_add(output_buff, mesg);
}

extern void init_FSM(void){
	input_buff = Buffer_init();
	output_buff = Buffer_init();
}
