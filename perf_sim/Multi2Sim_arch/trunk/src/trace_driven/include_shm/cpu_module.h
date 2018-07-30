#ifndef CPU_MODULE_H
#define CPU_MODULE_H

#include "mcore_module.h"

class CPU_module:public mcore_module
{
public:
	CPU_module(int id);
	void read(unsigned int addr,int size);
	void write(unsigned int addr,int size);
	void translated_block(int instr);
	void sim_cycle(unsigned int sim_tick);
	int module_recv_rq(Request *rq);                      
	int module_rq_done(Request *rq);                  
	int module_stats(); 
private:
	int cpuid;
	int TB_count;
	unsigned long instr_count;
	unsigned long cycle_count;
};

#endif
