#ifndef MEM_MODULEH
#define MEM_MODULEH
#include "mcore_module.h"

//class LockBus;
class Memory_module:public mcore_module {
public:
	int mem_hit;
	Memory_module() ;
	Memory_module(int latency) ;
	int module_recv_rq(Request *rq);
	int module_rq_done(Request *rq);
	int module_stats();
private:
	unsigned int mem_latency;
//	LockBus *lockbus;
};
#endif
