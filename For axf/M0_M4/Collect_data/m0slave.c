/*
 * @brief Multicore blinky example (Master or slave
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2014
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */



#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "Sensors_M0_batch.h"
/** @defgroup PERIPH_M0SLAVE_5410X Multicore blinky example (M0 as slave)
 * @ingroup EXAMPLES_CORE_5410X
 * @include "multicore_m0slave_blinky\readme.txt"
 */

/**
 * @}
 */
#define TICKRATE_HZ 100 //Hz
#define SAMPLE_RATE 5 //ms
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
int wait_M4 = 0;


extern uint32_t __Vectors;

char sequenceComplete = 0;

int SENSOR_SIGN[9] = {-1,1,-1,1,1,1,-1,-1,-1};  //Correct directions x,y,z - gyros, accels, magnetormeter
int AN[6] = {0,0,0,0,0,0}; //array that store the 3 ADC filtered data (gyros)
int AN_OFFSET[6]={0,0,0,0,0,0}; //Array that stores the Offset of the sensors
int Sensor_raw_data[9]={0};//acc,gyro,mag
uint8_t tx_buff[I2C_BUFF_SZ]={0};
uint8_t rx_buff[I2C_BUFF_SZ]={0};
int rawADC = 0;
int counter=0;
int LEDstate=0;
int loop = 1;	/* Fixes unreachable statement warning */


//sensor batch
int volatile batch[201][9]={0};
int pbatch=0;
int bfull=0;


/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{

		memcpy((char*)0x70000000,"a",sizeof(char));
		//clear_int_flag();

		//sema_lock();
		Read_Accel(pbatch);
		Read_Gyro(pbatch);

		Read_Compass(pbatch);

		//sema_unlock();


		//restore to 96Mhz before wake M4


		//set_int_flag();
		//__WFI();

		memcpy((char*)0x70000000,"b",sizeof(char));

	return 0;
}
