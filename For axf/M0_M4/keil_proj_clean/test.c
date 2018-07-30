//------------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2010 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : 2010-08-03 14:15:21 +0100 (Tue, 03 Aug 2010)
//
//      Revision            : 144883
//
//      Release Information : AT510-MN-80001-r0p0-00rel0
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Cortex-M0 DesignStart C program example
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include standard C library and RealView Compiler header.
//------------------------------------------------------------------------------

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


//------------------------------------------------------------------------------
// Implement the minimum number of functions required to support
// standard C input/output operations without a debugger attached.
//------------------------------------------------------------------------------

// Define the location of the output console in the DesignStart testbench.
volatile unsigned char *console = (volatile unsigned char *) 0x40000000U;

// Implement a simple structure for C's FILE handle.
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;



// Implement file IO handling, only console output is supported.
time_t time(time_t* t)        { static time_t clock = 0; return clock++; }
int fputc(int ch, FILE *f)    { *console = ch; return ch; }
int ferror(FILE *f)           { return 0; }
int fgetc(FILE *f)            { return -1; }
int __backspace(FILE *stream) { return 0; }
void _ttywrch(int ch)         { fputc(ch,&__stdout); }



// Writing 0xD to the console causes the DesignStart testbench to finish.
void _sys_exit(void)          { fputc(0x0D,&__stdout); while(1); }
//------------------------------------------------------------------------------
// 														ROW DATA
//------------------------------------------------------------------------------

double row_data[100] = {
1.079     ,1.14562   ,1.0625    ,1.0638    ,1.14206   ,1.07026   ,1.11716   ,1.11393   ,
1.04924   ,1.07479   ,1.07382   ,1.05409   ,1.02434   ,1.05215   ,1.02531   ,1.05409   ,
1.11231   ,1.10713   ,1.11328   ,1.04633   ,1.04989   ,1.03825   ,1.08029   ,1.09937   ,
1.12201   ,1.11781   ,1.10746   ,1.0211    ,1.04213   ,1.0211    ,1.0127    ,1.08708   ,
1.0942    ,1.08029   ,1.10099   ,1.02531   ,1.02822   ,1.11942   ,1.11651   ,1.10843   ,
1.10843   ,1.10778   ,1.09064   ,1.02822   ,1.08191   ,1.01011   ,1.08094   ,1.00655   ,
1.03048   ,1.09323   ,1.07932   ,1.01755   ,1.08741   ,1.06218   ,1.10713   ,1.0211    ,
1.09323   ,1.03889   ,1.09905   ,1.00784   ,1.05409   ,1.03242   ,9.97495   ,1.00655   ,
1.00558   ,1.01981   ,1.08935   ,1.04827   ,1.09905   ,1.02693   ,1.01787   ,1.10293   ,
1.0528    ,1.08482   ,1.03663   ,1.08094   ,1.09064   ,1.08644   ,1.06994   ,1.00881   ,
1.09032   ,1.00914   ,1.0072    ,1.02854   ,1.0997    ,1.09775   ,1.10843   ,1.01884   ,
1.10196   ,1.01916   ,1.0473    ,1.09905   ,1.07512   ,1.08352   ,1.00623   ,1.01431   ,
1.08126   ,1.09129   ,1.02337   ,1.09258 };
//------------------------------------------------------------------------------
// 														YOUR DESIGN
//------------------------------------------------------------------------------
#define INPUT_LENGTH 100
#define CHECK_DISTANCE 100
#define MAX_buffer (INPUT_LENGTH+CHECK_DISTANCE)

int loop=0;
double buffer[MAX_buffer] = {0};
double max=0;

int peak_detection(double input_t[INPUT_LENGTH]){
	int peak = 0;
	double max =0 ;
	double min = 0;
	double check = 0;
	
	int i =0;
	int j= 0;
	
	
	//find max
	for(i=0;i<INPUT_LENGTH;i++){
		if(input_t[i]>max) max=input_t[i];
	}
	
	min = max;
	
	//find min
	for(i=0;i<INPUT_LENGTH;i++){
		if(input_t[i]<min) min = input_t[i];
	}
	check = max - min;
	
	if(check>0.07) {
				   peak = 1;
				   }
	else peak = 0;
	
	return peak;
	

}

int main(){
	double input[INPUT_LENGTH]={0};
	int i;
	int index;
	int peaks=0;	
	int data_length = 0;

	data_length =100;

	
	//while(fscanf(fp,"%lf",&buffer[index++])!=EOF){
	while(loop<data_length){

	peaks += peak_detection(row_data);
	loop++;
	}
	
	printf("loop = %d\n",loop);
	printf("peaks = %d in %d seconds\n",peaks-1,(loop)/4);

	return 0;
}