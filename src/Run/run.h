#ifndef RUN_H
#define RUN_H

#include "asuro.h"

#define 	STATE_DRIVE		1
#define 	STATE_WAIT1		2
#define 	STATE_REVERSE	3
#define 	STATE_WAIT2		4
#define 	STATE_TURN		5
#define 	STATE_WAIT3		6

void transition(unsigned char newState);

#endif