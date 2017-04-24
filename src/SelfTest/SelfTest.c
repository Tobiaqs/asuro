/*******************************************************************************
*
* File Name:   SelfTest.c
* Project  :   SelfTest
*
* Description: 
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
#include "test.h"

void SelfTest (void)
{
	unsigned int i;
	Init();
	for (;;) {
		SerWrite("\n\r-- ASURO Testing --\n\r",23);

		SerWrite("LED Test\n\r",10);
		LEDTest();
		
		SerWrite("Line Test\n\r",11);
		StatusLED(GREEN);
		for (i = 0; i < 0xFE00; i++)
			LineTest();
		StatusLED(OFF);

		SerWrite("Switch Test\n\r",13);
		for (i = 0; i < 0xFE00; i++) 
			SwitchTest();
		Init(); StatusLED(OFF);

		SerWrite("Odometrie Test\n\r",16);
		FrontLED(ON);
		for (i = 0; i < 0xFFF0; i++)
			OdometrieTest();
		StatusLED(OFF); FrontLED(OFF);

		SerWrite("Motor Test\n\r",12);
		BackLED(ON,ON);
		MotorTest();
		BackLED(OFF,OFF);

		SerWrite("Serial Test\n\r",13);
		SerialTest(); 
  }
}

