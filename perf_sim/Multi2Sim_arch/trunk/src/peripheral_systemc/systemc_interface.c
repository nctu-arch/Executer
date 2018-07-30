#include <lib/util/debug.h>
#include "systemc_interface.h"

struct ext memory_interface;

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
)
{
	memory_interface.class_ptr=class_ptr;
	memory_interface.read_up=upr;
	memory_interface.write_up=upw;

}



void generic_rd(unsigned int  addr)
{
	if (!memory_interface.read_up){
		panic("%s: invalid access to_systemc_interface", __FUNCTION__);
	}
	unsigned long int  wordaddr = (unsigned long int) addr;
	unsigned char      mask[4];
	unsigned char      res[4];

	memset (mask, 0xff, sizeof (mask));
	memory_interface.read_up (memory_interface.class_ptr, wordaddr, mask, res,4);
	/*if(addr != 
		((unsigned int) res[3] << 24 |
		(unsigned int) res[2] << 16 |
		(unsigned int) res[1] <<  8 |
		(unsigned int) res[0] )){
		panic("%s: wrong result on read", __FUNCTION__);
	}*/ //no data return now
}

void generic_wr(unsigned int  addr)
{
	if (!memory_interface.write_up){
		panic("%s: invalid access to_systemc_interface", __FUNCTION__);
	}
	unsigned long int  wordaddr = (unsigned long int) addr;
	unsigned char      mask[4];
	unsigned char      val[4];
	
	memset (mask, 0xff, sizeof (mask));
	val[3] = (unsigned char) (addr >> 24);
	val[2] = (unsigned char) (addr >> 16);
	val[1] = (unsigned char) (addr >>  8);
	val[0] = (unsigned char) (addr      );
	memory_interface.write_up (memory_interface.class_ptr, wordaddr, mask, val,4);
	
}

void systemc_interface_wr(unsigned int  addr)
{
	return generic_wr (addr);
}
void systemc_interface_rd(unsigned int  addr)
{
	return generic_rd (addr);	
}
