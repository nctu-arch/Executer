#ifndef LIB_SYSTEMC_INTERFACE_H
#define LIB_SYSTEMC_INTERFACE_H

#include <stdio.h>
#include <string.h>

struct ext
{                             /* External linkage for SystemC */
	void  *class_ptr;
   	int  (*read_up) (void              *class_ptr,
		unsigned long int  addr,
     		unsigned char      mask[],
      		unsigned char      rdata[],
       		int                data_len);
    	int  (*write_up) (void              *class_ptr,
     		unsigned long int  addr,
      		unsigned char      mask[],
       		unsigned char      wdata[],
       		int                data_len);
};

extern struct ext memory_interface;

//public function
void systemc_init(void *class_ptr,
	int (*upr) (void *class_ptr,
	           unsigned long int  addr,
		   unsigned char      mask[],
		   unsigned char      rdata[],
		   int                data_len),
	int (*upw) (void              *class_ptr,
		   unsigned long int  addr,
		   unsigned char      mask[],
		   unsigned char      wdata[],
		   int                data_len)
);
void systemc_interface_wr(unsigned int  addr);
void systemc_interface_rd(unsigned int  addr);


//private function
void generic_rd(unsigned int  addr);
void generic_wr(unsigned int  addr);
#endif
