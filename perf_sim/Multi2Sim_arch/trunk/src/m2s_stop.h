#ifndef M2S_STOP_H
#define M2S_STOP_H

#include <stdio.h>
#include <string.h>

struct m2s_stop_st
{                             /* External linkage for SystemC */
     void  *class_ptr;
   	 void  (*stop_function) (void              *class_ptr);

};

extern struct m2s_stop_st m2s_stop_interface;

//public function
void m2s_stop_init(void *class_ptr,
                   void (*upstop_function) (void *class_ptr);
);


void systemc_interface_m2s_stop(void);

//private function
void generic_m2s_stop(void);

#endif
