
#ifndef COHERENCE_MODULEH
#define COHERENCE_MODULEH
#include "mcore_module.h"

#include "cache.h"
/*
namespace std {
	template<typename _Tp> class allocator;
	template<typename _Tp, typename _Alloc> class deque ;
	template<typename _Tp, typename _Sequence> class queue;
};
*/

class Coherence_module:public mcore_module {
public:
	int coherence_num;
	int latency;
	Coherence_module();
	int module_recv_rq(Request *rq);
	int module_rq_done(Request *rq);
	int module_stats();
	int getid(norm_cache *);
private:
	Cache_array cache_array;
/*	void bus_finish();
	void bus_finish_for_lock();
	bool check_bus_is_not_finish();
	bool flag_bus_finish;
	void print_lock_queue();
	int bus_owner;
	 std::queue< tick *,std::deque<tick *, std::allocator<tick*> > > *tick_queue;
	 std::queue< tick *,std::deque<tick *, std::allocator<tick*> > > *tick_conflict_queue;
*/	 
};
#endif
