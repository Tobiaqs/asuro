/*******************************************************************************
*
* File Name:   IRDemo.c
* Project  :   Demo
*
* Description: This file contains IRDemo features
*
* Ver.     Date         Author           Comments
* -------  ----------   --------------   ------------------------------
* 1.00	   14.08.2003   Jan Grewe		 build
* 2.00     22.10.2003   Jan Grewe        angepasst auf asuro.c Ver.2.10
*
* Copyright (c) 2003 DLR Robotics & Mechatronics
*****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 ***************************************************************************/
#include "asuro.h"
#include <stdlib.h>

#define DIARWD   0xFE01
#define DIAFWD   0xFF00
#define DIALEFT  0xFB04
#define DIARIGHT 0xFA05
#define DIASTOP  0xF30C

#define TUNERRWD   0xF00F
#define TUNERFWD   0xEE11
#define TUNERLEFT  0xE718
#define TUNERRIGHT 0xE51A
#define TUNERSTOP  0xE619

#define OFFSET	0x3F
#define STEP	5

int speedLeft,speedRight;

void IRFwd(void)
{
	speedRight += STEP;
	speedLeft  += STEP; 
	if (speedLeft < 0 && speedLeft >= -OFFSET) speedLeft = 1; 
	if (speedRight < 0 && speedRight >= -OFFSET) speedRight = 1; 
	FrontLED(ON);
	BackLED(OFF,OFF);
}

void IRRwd(void)
{
	speedRight -= STEP;
	speedLeft  -= STEP;
	if (speedRight > 0 && speedRight <= OFFSET)  speedRight = -1; 
	if (speedLeft > 0 && speedLeft <= OFFSET)  speedLeft = -1; 
	FrontLED(OFF);
	BackLED(ON,ON);
}

void IRLeft (void)
{
	speedLeft  -= STEP;
	if (speedLeft > 0 && speedLeft <= OFFSET) speedLeft = -1;
	speedRight += STEP;
	if (speedRight < 0 && speedRight >= -OFFSET) speedRight = 1;
	FrontLED(OFF);
	BackLED(ON,OFF);
}

void IRRight (void)
{
	speedLeft  += STEP;
	if (speedLeft < 0 && speedLeft >= -OFFSET) speedLeft = 1; 
	speedRight -= STEP;
	if (speedRight > 0 && speedRight <= OFFSET) speedRight = -1; 
	FrontLED(OFF);
	BackLED(OFF,ON);
}


void IRStop(void)
{
	speedRight = speedLeft = 0;
	FrontLED(OFF);
	BackLED(OFF,OFF);
}

void IRDemo(void)
{

	unsigned char oldToggle,toggle,i;
	unsigned char data[33],start;
	static unsigned int cmd;
	unsigned char leftDir = FWD, rightDir = FWD;

	Init();
	for (;;) {
		oldToggle = 1;
		start = FALSE;
		i = 0;
		while(PIND & (1 << PD0)); // wait for Sync
		TCNT0 = 0;
		TCCR0 = (1 << CS02) | (1 << CS00); // Start Timer
		do 	{
			toggle = PIND & (1 << PD0);
			while(PIND & (1 << PD0));
		}
		while (toggle != oldToggle); 
		data[0] = TCNT0;
		if (TCNT0 > 0x60 && TCNT0 < 0x72) { start = TRUE; StatusLED(YELLOW); } 
		oldToggle = 0;
		TCNT0 = 0;

		if (start == TRUE) {
			do {
				toggle = PIND & (1 << PD0);
				if (toggle != oldToggle) {
					if (toggle < oldToggle){ //HIGH to LOW
					if ((TCNT0 > 0x10 && TCNT0 < 0x13) || (TCNT0 > 0x06 && TCNT0 < 0x10))
						data[i++] = TCNT0;
						TCNT0 = 0;
					}
					oldToggle = toggle; 
				}
			} while(i < 32 && TCNT0 < 0x60); 

			if (TCNT0 < 0x62) {
				cmd = 0;
				for (i = 16; i < 32; i++) 
					if (data[i] > 0x10) cmd |= 1 << (i - 16);
				StatusLED(GREEN);
			}
		}

		switch (cmd) {
			case TUNERRWD :
			case DIARWD :		IRRwd(); break;
			case TUNERFWD :
			case DIAFWD :		IRFwd(); break;
			case TUNERLEFT :
			case DIALEFT:		IRLeft(); break;
			case TUNERRIGHT :
			case DIARIGHT:		IRRight(); break;
			case TUNERSTOP :
			case DIASTOP :		IRStop(); break;
		}
		if (speedLeft > 0 && speedLeft <  OFFSET) speedLeft += OFFSET;
		if (speedLeft < 0 && speedLeft > -OFFSET) speedLeft -= OFFSET;
		if (speedRight > 0 && speedRight <  OFFSET) speedRight += OFFSET;
		if (speedRight < 0 && speedRight > -OFFSET) speedRight -= OFFSET;

		leftDir = rightDir = FWD;
		if (speedLeft < 0)  leftDir = RWD; 
		if (speedRight < 0) rightDir = RWD; 
			
		if (speedLeft >   255) speedLeft  =  255; 
		if (speedLeft <  -255) speedLeft  = -255;
		if (speedRight >  255) speedRight =  255;
		if (speedRight < -255) speedRight = -255;

		MotorDir(leftDir,rightDir);
		MotorSpeed(abs(speedLeft),abs(speedRight));
	}
}

