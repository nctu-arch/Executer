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

#include <stdio.h>
#include <time.h>

//------------------------------------------------------------------------------
// Implement the minimum number of functions required to support
// standard C input/output operations without a debugger attached.
//------------------------------------------------------------------------------

// Define the location of the output console in the DesignStart testbench.
volatile unsigned char *console = (volatile unsigned char *) 0x60000U;

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

int main(void) {
	
	int a;
	
  /*printf("Hello World!\n");

  printf("This message was printed from helloworld.c provided\n");
  printf("with the ARM Cortex-M0 DesignStart processor\n");
*/
	a=1+2;
	
  return 0;
}



/*#include <stdio.h>

#define TEST_ADDR 0x0000FFFF

int main(){
	
    int i = 2, j, k;
    int q;
	
    q = 10*5+23-7;
	
    i = i * q;
	
    j = i<<15;
	
    for(k = 0;k<5;k++){
        j+=15;
    }
    j = j>>3;
		

		printf("s");
    return 0;
	

	int x, y, z;
	unsigned int   test_load;
	
	test_load = *(unsigned int*)(0x00000000);
  unsigned int volatile * const test = (unsigned int *) TEST_ADDR;
	
	*test = 123;
	//x = *test;
	
	__asm{
		//MOVE
		MOVS     x, #2
		MOVS		 y, x
		MOV      z, x
		//ADD
		ADDS     x, y, x		
		MULS     z, x, y
		
	}
	x = test_load;
}

*/