#include "mcore_module.h"
#include "cache.h"
#include <vector>

class Coherence_module;
class Cache_module:public mcore_module {
public:
	static std::vector< Cache_module *> cache_list;
	static int cache_count;
	int cache_start_time;
	int cache_id, cache_latency;
	norm_cache *cache_lines;
	Cache_module(int latency, int block_bits, int index_bits, int way);
	int module_recv_rq(Request *rq);
	int module_rq_done(Request *rq);
	int module_stats();
	int Coherence_connect(Coherence_module * bus);

};
extern "C" {
	norm_cache *Cache_module_cache_lines(int);
}
