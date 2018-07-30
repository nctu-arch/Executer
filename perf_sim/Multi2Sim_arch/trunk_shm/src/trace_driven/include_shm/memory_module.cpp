
#include "memory_module.h"
//#include "LockBus.h"
#define MEMORY_LATENCY 20

Memory_module::Memory_module() 
{
	mem_hit=0;

//	lockbus = new("shmem") LockBus;
	mem_latency = MEMORY_LATENCY;
}
Memory_module::Memory_module(int latency) 
{
	mem_hit=0;

//	lockbus = new("shmem") LockBus;
	mem_latency = latency;
}
int Memory_module::module_recv_rq(Request *rq)
{
/*	if( rq->type == Rq_Lock_bus || rq->type==Rq_Unlock_bus){
		if (lockbus->check_owner(rq, this) ) {
			mcore_return_rq(0);
		} else {
			mcore_suspend_rq();
		}
	} else {
		mem_hit++;
		mcore_return_rq(mem_latency);
	}
*/
	mem_hit++;
	mcore_return_rq(mem_latency);
	return 0;
}
int Memory_module::module_rq_done(Request *rq)
{
	// memory should no send request to others
}
int Memory_module::module_stats()
{
	printf("================= Memory ==========================\n");
//	printf("memory conflict: %i\n",mcore_GetConflict());
	printf("memory hit time: %i\n",mem_hit);
//	printf("memory Lock conflict: %i\n",lockbus->log_Lock_conflict);
}
