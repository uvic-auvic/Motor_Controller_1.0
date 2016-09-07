#ifndef MOTORS_H_
#define MOTORS_H_

typedef enum{
	right_side,
	left_side,
	top_right,
	top_left,
	middle
} motor;

typedef enum{
	Reverse = -1,
	Forward = 1
} motor_direction;

//Public functions ------------------------------

extern void Motors_init(void);
extern void Motor_PWM(motor m, unsigned int percent_10000);
extern void Motor_Speed(motor m, unsigned int percent_10000, motor_direction dir);

//-----------------------------------------------



#endif
