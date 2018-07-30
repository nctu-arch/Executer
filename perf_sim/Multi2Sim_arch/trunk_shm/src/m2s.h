#ifndef M2S_H
#define M2S_H

void m2s_init(int *argc_ptr, 
	char **argv,
	void *class_ptr,
	int (*upr) (void *class_ptr,
	           unsigned long int  addr,
		   unsigned char      mask[],
		   unsigned char      rdata[],
		   int                data_len),
	int (*upw) (void              *class_ptr,
		   unsigned long int  addr,
		   unsigned char      mask[],
		   unsigned char      wdata[],
		   int                data_len),
	void (*stop_function) (void              *class_ptr) //IC_S_2013_08_01
);

void m2s_loop(void);
void m2s_end();

#endif
