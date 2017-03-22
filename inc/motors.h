#ifndef MOTORS_H_
#define MOTORS_H_

#define SCALE (10000)
#define NEUTRAL (900)
#define FWD_MAX (1200)
#define REV_MAX (600)

typedef enum{
	motor1,
	motor2,
	motor3
} motor;

typedef enum{
	Reverse = -1,
	Forward = 1
} motor_direction;

//Public functions ------------------------------

extern void Motors_init(void);
extern void Motor_PWM(motor m, unsigned int percent_10000);
extern void Motors_Stop(void);
extern void Motor_Speed(motor m, unsigned int percent_10000, motor_direction dir);

//-----------------------------------------------



#endif
