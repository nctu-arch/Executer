/*
 * @brief Multicore blinky example (Master)
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
#include "AHRS.h"
#include "api.h"
/** @defgroup PERIPH_M4MASTER_5410X Multicore blinky example (M4 core as master)
 * @ingroup EXAMPLES_CORE_5410X
 * @include "multicore_m4master_blinky\readme.txt"
 */

/**
 * @}
 */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TICKRATE_HZ (50)	/* 50 ticks per second */



#ifdef __CODE_RED
extern uint32_t __core_m0slave_START__[];
#define M0_BOOT_STACKADDR &__core_m0slave_START__[0]
#define M0_BOOT_ENTRYADDR &__core_m0slave_START__[1]

#else
/* The M0 slave core's boot code is loaded to execute at address 0x2000.
   These address contain the needed M0 boot entry point and the M0 stack
     pointer that the M4 master core sets up for boot. */
/* When booting from RAM this will be overriden by project setting */
#ifndef M0_BOOT_STACKADDR
#define M0_BOOT_STACKADDR           0x20000
#endif

/* When booting from RAM this will be overriden by project setting */
#ifndef M0_BOOT_ENTRYADDR
#define M0_BOOT_ENTRYADDR           ((uint32_t) (M0_BOOT_STACKADDR) +4)
#endif
#endif

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/******************SHARED*********************/
volatile uint32_t sharedLEDStates;
volatile uint32_t shared_address;
int Sensor_raw_data[9]={0};//acc,gyro,mag
//**********************************************//
extern void *_pvHeapStart;
extern void *_pvHeapLimit;
extern void *_vStackTop;
extern void *__user_heap_base;


extern unsigned int __data_section_table;
extern unsigned int __data_section_table_end;
extern unsigned int __bss_section_table;
extern unsigned int __bss_section_table_end;

char sequenceComplete = 0;

int sensors[3] = {1,2,0};  // Map the ADC channels gyro_x, gyro_y, gyro_z
int SENSOR_SIGN[9] = {-1,1,-1,1,1,1,-1,-1,-1};  //Correct directions x,y,z - gyros, accels, magnetormeter

extern float G_Dt=0.02;    // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

long timer=0;   //general purpuse timer
long timer_old;
long timer24=0; //Second timer used to print values
int AN[6] = {0,0,0,0,0,0}; //array that store the 3 ADC filtered data (gyros)
int AN_OFFSET[6]={0,0,0,0,0,0}; //Array that stores the Offset of the sensors
int ACC[3] = {0,0,0};          //array that store the accelerometers data

int accel_x = 0;
int accel_y = 0;
int accel_z = 0;
int magnetom_x = 0;
int magnetom_y = 0;
int magnetom_z = 0;
int gyro_x = 0;
int gyro_y = 0;
int gyro_z = 0;
float MAG_Heading = 0;

float Accel_Vector[3]= {0,0,0}; //Store the acceleration in a vector
float Gyro_Vector[3]= {0,0,0};//Store the gyros turn rate in a vector
float Omega_Vector[3]= {0,0,0}; //Corrected Gyro_Vector data
float Omega_P[3]= {0,0,0};//Omega Proportional correction
float Omega_I[3]= {0,0,0};//Omega Integrator
float Omega[3]= {0,0,0};

// Euler angles
float roll = 0;
float pitch = 0;
float yaw = 0;

float errorRollPitch[3]= {0,0,0};
float errorYaw[3]= {0,0,0};

unsigned int counter=0;
int gyro_sat=0;

float DCM_Matrix[3][3]= {
  {
    1,0,0  }
  ,{
    0,1,0  }
  ,{
    0,0,1  }
};
float Update_Matrix[3][3]={{0,1,2},{3,4,5},{6,7,8}}; //Gyros here


float Temporary_Matrix[3][3]={
  {
    0,0,0  }
  ,{
    0,0,0  }
  ,{
    0,0,0  }
};

int main_loop_counter=0;
int LEDstate=0;
int loop = 1;	/* Fixes unreachable statement warning */

int main(void)
{
		 int i;
		 memcpy((char*)0x70000000,"a",sizeof(char));
		 //clear_int_flag();

		 //sema_lock();
	
		 for(i=0;i<2;i++){
		 accel_x = shm_read(0);
		 accel_y = shm_read(1);
		 accel_z = shm_read(2);
		 gyro_x = shm_read(3);
		 gyro_y = shm_read(4);
		 gyro_z = shm_read(5);
		 magnetom_x = shm_read(6);
		 magnetom_y = shm_read(7);
		 magnetom_z = shm_read(8);

		 //sema_unlock();

			
			Compass_Heading();

			Matrix_update();

			Normalize();

			Drift_correction();

			Euler_angles();
			}

		//set_int_flag();


			//__asm__("WFI");
			//__WFI();

			memcpy((char*)0x70000000,"b",sizeof(char));

		//clear_int_flag();


	return 0;
}
