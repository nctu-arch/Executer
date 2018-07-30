#include "Inc.h"
#include "cache.h"
#include "mcore_module.h"
#include "cache_module.h"
#include "coherence_module.h"
#include <vector>

int Cache_module::cache_count = 0;
std::vector< Cache_module *> Cache_module::cache_list;

extern "C" {
	norm_cache *cache_lines_init( int, int, int);
	void cache_print(norm_cache *, int);
	coherence_type cache_ref_rd(norm_cache * cptr, unsigned int addr);
	coherence_type cache_ref_wr(norm_cache * cptr, unsigned int addr);
};

int Cache_module::Coherence_connect(Coherence_module * bus)
{
	cache_id = bus->getid(cache_lines);
	mcore_module::connect(bus);
}
Cache_module::Cache_module(int latency, int block_bits, int index_bits, int way) 
{   
	cache_list.push_back(this);
	//cache_id = cache_count++;
	cache_latency = latency;
	cache_lines = cache_lines_init(block_bits, index_bits, way);
}   
int Cache_module::module_recv_rq(Request *rq)
{   
	coherence_type type;
	cache_start_time = mcore_GetCurTime();
	if(rq->type == Rq_Lock_bus) {
		assert("Rq_Lock_bus is failed");
		mcore_rq.type = C_Lock_bus;
        mcore_rq.args[0] = rq->args[0];
		mcore_rq.args[1] = cache_id;
		mcore_send_rq(&mcore_rq);
		return 0;
	}
    else if(rq->type == Rq_Unlock_bus) {
		assert("Rq_Unlock_bus is failed");
        mcore_rq.type = C_Unlock_bus;
        mcore_rq.args[0] = rq->args[0];
        mcore_rq.args[1] = cache_id;
        mcore_send_rq(&mcore_rq);
        return 0;
    }
	else if (rq->type == Rq_Read) {
		type = cache_ref_rd(cache_lines,rq->args[0]);
	} else {
		type = cache_ref_wr(cache_lines,rq->args[0]);
	}
	cache_lines->type_4_cycle = type;
	if (type == C_hit ) {
		cache_lines->total_ref_time += cache_latency;
		cache_lines->local_hit_cycle += cache_latency;
		cache_lines->local_hit_ref += 1;
		mcore_return_rq(cache_latency);
	} else {
		mcore_rq.type = type;
		mcore_rq.args[0] = rq->args[0];
		//assert(cache_id == (int)(rq->args[1]));
		mcore_rq.args[1] = cache_id;
		mcore_rq.args[2] = 0;
		mcore_SetAdvanceTime(cache_latency);
		mcore_send_rq(&mcore_rq);
		cache_lines->coherence++;
	}
return 0;
}   
int Cache_module::module_rq_done(Request *rq)
{   
	int curtime = mcore_GetCurTime();
	if( cache_lines->type_4_cycle != WR_hit) {
		cache_lines->miss_cycle  += (curtime - cache_start_time);
	} else {
		cache_lines->hit_snoop_cycle += (curtime - cache_start_time);
	}
	cache_lines->total_ref_time += (curtime - cache_start_time);
	mcore_return_rq(0);
}   

int Cache_module::module_stats()
{
	cache_print(cache_lines, cache_id);
}

