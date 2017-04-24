#include "run.h"

unsigned char state;

int main(void)
{
	Init();

	transition(STATE_DRIVE);

	// The direction for the right motor should be reversed
	while (TRUE) {
		switch (state) {
			case STATE_DRIVE:
				if (PollSwitch() > 0) {
					StatusLED(RED);
					transition(STATE_WAIT1);
				}
				break;

			case STATE_WAIT1:
				if (GetTimerValue() >= 36000) {
					transition(STATE_REVERSE);
				}
				break;

			case STATE_REVERSE:
				if (GetTimerValue() >= 72000) {
					transition(STATE_WAIT2);
				}
				break;

			case STATE_WAIT2:
				if (GetTimerValue() >= 36000) {
					transition(STATE_TURN);
				}
				break;

			case STATE_TURN:
				if (GetTimerValue() >= 72000) {
					transition(STATE_WAIT3);
				}
				break;

			case STATE_WAIT3:
				if (GetTimerValue() >= 36000) {
					transition(STATE_DRIVE);
				}
				break;
		}
	}
	return 0;
}

void transition(unsigned char newState) {
	ResetTimer();
	
	switch (newState) {
		case STATE_DRIVE:
			StatusLED(GREEN);
			MotorDir(FWD, RWD);
			MotorSpeed(192, 255);
			break;

		case STATE_REVERSE:
			MotorDir(RWD, FWD);
			MotorSpeed(136, 180);
			break;

		case STATE_TURN:
			MotorDir(RWD, RWD);
			MotorSpeed(136, 180);
			break;

		case STATE_WAIT1:
		case STATE_WAIT2:
		case STATE_WAIT3:
			MotorDir(BREAK, BREAK);
			break;
	}

	state = newState;
}
