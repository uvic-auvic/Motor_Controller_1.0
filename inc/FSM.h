#ifndef FSM_H_
#define FSM_H_

#include "UART_Receiver.h"
#include "stm32f0xx.h"
#include "Buffer.h"


/* Public function prototypes -----------------------------------------------*/

/*
 * Once the a command has been received of the proper length with a \n it is
 * sent to this function to be dealt in the system
 */
extern void command_recv(char *comm, int comm_length);

/*
 *
 */
extern void init_FSM(void);

extern void FSM_do(void);

extern void Send_to_Odroid(char* mesg);
// ----------------------------------------------------------------------------



#endif // FSM_H_

/*
enum commands{
	Motor_1_Forward,		//M1F
	Motor_2_Forward,		//M2F
	Motor_3_Forward,		//M3F
	Motor_4_Forward,		//M4F
	Motor_5_Forward,		//M5F
	Motor_1_Reverse,		//M1R
	Motor_2_Reverse,		//M2R
	Motor_3_Reverse,		//M3R
	Motor_4_Reverse,		//M4R
	Motor_5_Reverse,		//M5R
	Stop_All, 				//STP
	Stop_Motor_1,			//SM1
	Stop_Motor_2,			//SM2
	Stop_Motor_3,			//SM3
	Stop_Motor_4,			//SM4
	Stop_Motor_5,			//SM5
	Revoultion_on_Motor_1,	//RV1
	Revoultion_on_Motor_2,	//RV2
	Revoultion_on_Motor_3,	//RV3
	Revoultion_on_Motor_4,	//RV4
	Revoultion_on_Motor_5,	//RV5
	Pulse_Width_Modulate_1,	//PW1
	Pulse_Width_Modulate_2,	//PW2
	Pulse_Width_Modulate_3,	//PW3
	Pulse_Width_Modulate_4,	//PW4
	Pulse_Width_Modulate_5,	//PW5
	Forward,				//FWD
	Reverse,				//REV
	Up,						//UUP
	Down,					//DWN
	More_Commands_Coming,	//MCC
	Calibrate_Motor_1,		//CL1
	Calibrate_Motor_2,		//CL2
	Calibrate_Motor_3,		//CL3
	Calibrate_Motor_4,		//CL4
	Calibrate_Motor_5		//CL5
};*/
