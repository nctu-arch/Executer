#ifndef CACHEH
#define CACHEH
#define MEMORY_LATENCY 20

#include "Inc.h"

//porshe
typedef enum{
	MESI_INVALID = 0x0, MESI_EXCLUSIVE, MESI_SHARED, MESI_MODIFY, MESI_Error=0xff   
} MESI_state;

typedef enum {
	C_hit = 0x0,	RD_miss,	WR_hit, WR_miss,C_Unlock_bus=0x7,C_Lock_bus=0x8, hit_miss_error=0xff
} coherence_type;

//for cache record
typedef struct ndr {
	unsigned int tag;
	int lru;
	int owner;
	MESI_state state;
} norm_rec;



typedef struct cache_local{
	int size, setnum, assoc, blksize;
	int blkbit;
	int setbit;
	int set_mask;
	int ref;
	int share_ref;
	int local_hit_ref;
	int hit,miss, hit_on_shared;
	int share_hit,share_miss;
	int invalidation_miss;
	int start;
	unsigned total_ref_time;
	int coherence;
	unsigned local_hit_cycle, hit_snoop_cycle, miss_cycle;
	coherence_type type_4_cycle;
	norm_rec *tag_tab, *recent;
	unsigned int rqWrite, rqRead;
	unsigned int read_miss, write_miss;
	unsigned int read_hit, write_hit;
} norm_cache;

//**************************************


#define MAX_CACHE 256

typedef struct {
	norm_cache *cache_lines[MAX_CACHE];
	int cache_count;
} Cache_array;

/*
typedef struct Cache_rec {
	Object obj;
	Port *out_port;
	int latency;
	int start;
	norm_cache *cache_lines;   //porshe 
} Cache;

typedef struct Coherence_mod {
	Object obj;
	Port *out_port;
	int latency;
	int free_time;
	int conflict;
	int next_profile;
} Coherence_Bus ;

typedef struct Mem_rec {
	Object obj;
	int latency;
	int free_time;
	int conflict;
} Memory;
*/

#ifdef __cplusplus
extern "C" {
#endif
        void Coherence_Bus_entry();
        void Coherence_Bus_stats();

        void perform_coherence(coherence_type co_type,unsigned int addr, int self, Cache_array *cache_line);
        void perform_coherence_v2(coherence_type co_type,unsigned int addr, int self, Cache_array *cache_line);
        norm_cache * cache_lines_init(int blk_shift, int setbit, int assoc);
        void cache_print(norm_cache * cptr, int id);
        coherence_type cache_ref_rd(norm_cache * cptr,unsigned int addr);
        coherence_type cache_ref_wr(norm_cache * cptr,unsigned int addr);
#ifdef __cplusplus
}
#endif

#endif
