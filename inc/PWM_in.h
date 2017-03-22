#ifndef PWM_IN_H_
#define PWM_IN_H_

#define CLOCK_PWM_PRESCALE		(960)
#define MAX_PWM_FREQUENCY		(2000)
#define CLOCK_PERIOD			(50000)
#define MOTOR_HZ_RATIO			((CLOCK_PERIOD) / (MAX_PWM_FREQUENCY))
#define CYCLES_PER_REV			(7)

#include "motors.h"

//Public functions
extern void pwm_in_init(void);
extern int read_duty_cycle_percent_1000(motor m);
extern int read_frequency_hz(motor m);

#endif
