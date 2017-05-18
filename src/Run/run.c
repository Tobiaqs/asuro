#include "run.h"

// State variable
unsigned char state;

// Left/right motor speeds
unsigned char speedLeft = 0;
unsigned char speedRight = 0;

// Count pulses on both wheels
unsigned char odometryCounterLeft = 0;
unsigned char odometryCounterRight = 0;

// Whenever the motor speed is set by the program, this variable
// is set to true. It causes the robot to first drive a short while
// before it starts modifying the speed
bool odometryWait = true;

bool odometryDarkLeft = false;
bool odometryDarkRight = false;
unsigned int odometry[2];

int main(void)
{
	// Counts loops in a row, in which there is a collision detected
	unsigned char collisionCounter = 0;

	// Init device
	Init();

	// Reset the secondary timer
	ResetTimer();

	// Activate the first state
	transition(STATE_DRIVE);

	// The main loop of the program
	while (TRUE) {
		switch (state) {
			case STATE_DRIVE:
				updateMotorSpeed();

				if (PollSwitch() > 0) {
					if (collisionCounter < 255)
						collisionCounter ++;
				} else
					collisionCounter = 0;

				if (collisionCounter >= 5) {
					collisionCounter = 0;
					transition(STATE_WAIT1);
				}
				break;

			case STATE_WAIT1:
				if (GetTimer() >= TICKS_WAIT) {
					transition(STATE_REVERSE);
				}
				break;

			case STATE_REVERSE:
				updateMotorSpeed();

				if (GetTimer() >= TICKS_REVERSE) {
					transition(STATE_WAIT2);
				}
				break;

			case STATE_WAIT2:
				if (GetTimer() >= TICKS_WAIT) {
					transition(STATE_TURN);
				}
				break;

			case STATE_TURN:
				if (GetTimer() >= TICKS_TURN) {
					transition(STATE_WAIT3);
				}
				break;

			case STATE_WAIT3:
				if (GetTimer() >= TICKS_WAIT) {
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
			setMotorSpeed(200);
			break;

		case STATE_REVERSE:
			StatusLED(RED);
			MotorDir(RWD, FWD);
			setMotorSpeed(180);
			break;

		case STATE_TURN:
			StatusLED(RED);
			MotorDir(RWD, RWD);
			setMotorSpeed(180);
			break;

		case STATE_WAIT1:
		case STATE_WAIT2:
		case STATE_WAIT3:
			StatusLED(YELLOW);
			MotorDir(BREAK, BREAK);
			setMotorSpeed(0);
			break;
	}

	state = newState;
}

/**
 * Sets the motor speed for both motors.
 */
void setMotorSpeed(unsigned char speed) {
	// Approximation for the speed of the left motor
	speedLeft = speed / 4 * 3;
	speedRight = speed;
	// Wait for robot to get up to speed after setting the new speed
	odometryWait = true;
	// Reset odometry counters
	odometryCounterLeft = 0;
	odometryCounterRight = 0;
	// Set motor speed
	MotorSpeed(speedLeft, speedRight);
}

void updateMotorSpeed() {
	// One of the motors has speed zero? Then don't run
	if (speedLeft == 0 || speedRight == 0)
		return;

	// Read odometry data
	OdometrieData(odometry);

	// Left sensor is detecting dark while most recent scan revealed bright, or other way around
	if ((odometry[0] > ODO_THRESHOLD && !odometryDarkLeft) || (odometry[0] <= ODO_THRESHOLD && odometryDarkLeft)) {
		// Toggle "previous scan result" flag
		odometryDarkLeft = !odometryDarkLeft;
		odometryCounterLeft ++;
	}
	// Right sensor is detecting dark while most recent scan revealed bright, or other way around
	if ((odometry[1] > ODO_THRESHOLD && !odometryDarkRight) || (odometry[1] <= ODO_THRESHOLD && odometryDarkRight)) {
		odometryDarkRight = !odometryDarkRight;
		odometryCounterRight ++;
	}

	// Are we still waiting for the robot to get up to speed?
	if (odometryWait) {
		// See if one of the pulse counters has reached the target amount of pulses
		if (odometryCounterLeft >= ODO_WAITPULSES || odometryCounterRight >= ODO_WAITPULSES) {
			// Reset the wait flag
			odometryWait = false;
			// Reset the counters
			odometryCounterLeft = 0;
			odometryCounterRight = 0;
		}
	}

	// See if we have enough pulses to modify the speed
	if (odometryCounterLeft >= ODO_INTERVAL || odometryCounterRight >= ODO_INTERVAL) {
		// Left wheel turning faster than right wheel?
		if (odometryCounterLeft > odometryCounterRight) {
			// Decrease speed on the right
			speedRight += ODO_SPEEDSTEP;
			// Increase speed on the left
			speedLeft -= ODO_SPEEDSTEP;

		// Right wheel turning faster than left wheel?
		} else if (odometryCounterRight > odometryCounterLeft) {
			// Increase speed on the left
			speedLeft += ODO_SPEEDSTEP;
			// Decrease speed on the right
			speedRight -= ODO_SPEEDSTEP;
		}
		// Reset counters
		odometryCounterLeft = 0;
		odometryCounterRight = 0;
	}

	// Failsafe
	if (speedLeft < 75 || speedRight < 100) {
		speedLeft = 75;
		speedRight = 100;
		// Enable front LED to indicate that the failsafe was triggered
		FrontLED(ON);
	}

	// Set the motor speeds
	MotorSpeed(speedLeft, speedRight);
}
