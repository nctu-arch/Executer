#ifndef GLOBALS_H
#define GLOBALS_H
	
#define DIV(x,y)  (((float) (x))/((float) (y)))

typedef enum { 
	 Rq_Read = 0x01, Rq_Write,Rq_Unlock_bus,Rq_Lock_bus
} Request_type;

#define TRUE	1
#define FALSE	0

#endif
