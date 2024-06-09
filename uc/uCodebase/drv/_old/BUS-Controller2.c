/*
 * BUS-Controller2.c
 *
 * Created: 12.01.2019 21:38:44
 *  Author: Christian
 */ 

#include "BUS-Controller2.h"

void init_BusController2(void)
{
		setLedGreen(false);
		setLedBlue(false);
		setLedRed(false);
		
		PORT->Group[0].DIRSET.reg = 0x08000003;
}