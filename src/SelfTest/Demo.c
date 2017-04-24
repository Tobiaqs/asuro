/*******************************************************************************
*
* File Name:   Demo.c
* Project  :   Demo
*
* Description: This file contains Demo main routine
*
* Ver.     Date         Author           Comments
* -------  ----------   --------------   ------------------------------
* 1.00	   14.08.2003   Jan Grewe		 build
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
#include "LineDemo.h"
#include "RechteckDemo.h"
#include "PCDemo.h"
#include "IRDemo.h"

void Demo (void)
{
	unsigned char sw;
	Init();
	for (;;) {
		sw = PollSwitch();
		if (sw == 0x01) LineDemo();
		if (sw == 0x02) RechteckDemo();
		if (sw == 0x04) PCDemo();
		if (sw == 0x08) IRDemo();
	}
}
