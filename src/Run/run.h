#ifndef RUN_H
#define RUN_H

#include "asuro.h"

#define 	STATE_DRIVE		1
#define 	STATE_WAIT1		2
#define 	STATE_REVERSE	3
#define 	STATE_WAIT2		4
#define 	STATE_TURN		5
#define 	STATE_WAIT3		6

// The threshold the odometry values are compared to
#define		ODO_THRESHOLD	650

// The amount the speed is modified with after ODO_INTERVAL
// pulses have occurred
#define		ODO_SPEEDSTEP	5

// The amount of pulses inbetween speed modifications
#define		ODO_INTERVAL	10

// The amount of pulses the robot should drive before
// speed modifications are enabled
#define		ODO_WAITPULSES	25

typedef enum {false=0, true=1} bool;

void transition(unsigned char newState);
void setMotorSpeed(unsigned char speed);
void updateMotorSpeed(void);
void odoTest(void);

#endif