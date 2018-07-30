#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "sim-stat.h"

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}

main()
{
	int i,j;
	long long start, end;
	
	j=0;
	for (i=0; i<10000; i++, j+=2) {
		start = rdtsc();
		stat_tick(1, Awalys);   // 
		stat_tick((i%2)==0, MOD2);  // only if i is even
		stat_tick((i%4)==0, MOD4a);  
		stat_tick((j%4)==0, MOD4b);  // j = 2*i
		stat_tick((i%4)==0, MOD4);   // C compiler will give the same addr for "MOD4"
		stat_tick((random()%7)!=0, MOD7);
		stat_tick((i%4)==0, MOD4);   // C compiler will give the same addr for "MOD4"
// do something here ....
		end = rdtsc();
    
		stat_variable((i%2)==0, Cycle_clocks, end-start);
	}
	
	stat_summary(1);
}
