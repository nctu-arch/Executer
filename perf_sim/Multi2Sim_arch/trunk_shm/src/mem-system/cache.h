/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MEM_SYSTEM_CACHE_H
#define MEM_SYSTEM_CACHE_H

#define FDP_FILTER_POW_SIZE 12

extern struct str_map_t cache_policy_map;
extern struct str_map_t cache_block_state_map;

struct prefetcher_pollution_bloom_filter        //cjc_20130715 Feedback Directed Prefetching: Improving the Performance and Bandwidth-Efficiency of Hardware Prefetchers, HPCA'2007
{
        char pollution_filter[(1<<FDP_FILTER_POW_SIZE)];
};
enum cache_policy_t
{
	cache_policy_invalid = 0,
	cache_policy_lru,
	cache_policy_fifo,
	cache_policy_random
};

enum cache_block_state_t
{
	cache_block_invalid = 0,
	cache_block_noncoherent,
	cache_block_modified,
	cache_block_owned,
	cache_block_exclusive,
	cache_block_shared,
	cache_blcok_transient	//cjc_20130107 identify the block is in mshr (used in ideal pf_cache)
};

//IC_S_2012_12_27
enum dead_block_state_t
{
	demand = 0, //the block comes from demand request
	prefetch,   //the block comes from prefetch request
	reused     //the block is re-reference, clear, i.e. the block is not dead block 

};
//IC_E

struct cache_block_t
{
	struct cache_block_t *way_next;
	struct cache_block_t *way_prev;

	unsigned int tag;
	int transient_tag;
	int way;
	int prefetched;

	int flag_dead_blck;		//cjc_20121215 identify whether it has been used by core
	unsigned int victim_tag;                 //cjc_20121207 use to calculate cache poolution
	long long inserted_cycle;       //cjc_20121207 esim_cycle of insertion
	struct mod_stack_t *stack;	//cjc_20130107 for ideal pf_cache	
	int pf_owner;			//cjc_20130208 identify which core carry this block into cache

	enum cache_block_state_t state;
	//IC_S_2012_12_27
	enum dead_block_state_t dead_block_state;
	//IC_E

	int owner;//cc_20130418
};

struct cache_set_t
{
	struct cache_block_t *way_head;
	struct cache_block_t *way_tail;
	struct cache_block_t *blocks;
};

struct cache_t
{
	char *name;

	unsigned int num_sets;
	unsigned int block_size;
	unsigned int assoc;
	enum cache_policy_t policy;

	struct cache_set_t *sets;
	unsigned int block_mask;
	int log_block_size;

	struct prefetcher_t *prefetcher;
	struct throttle_t * throttle;

	long long  *num_of_access_on_set; //porshe:20130219 profiling cache accesses distribution of cache lines
	struct prefetcher_pollution_bloom_filter FDP_filter;
};


struct cache_t *cache_create(char *name, unsigned int num_sets, unsigned int block_size,
		unsigned int assoc, enum cache_policy_t policy);
void cache_free(struct cache_t *cache);

void cache_decode_address(struct cache_t *cache, unsigned int addr,
		int *set_ptr, unsigned int *tag_ptr, unsigned int *offset_ptr);
int cache_find_block(struct cache_t *cache, unsigned int addr, int *set_ptr, int *pway, 
		int *state_ptr);
void cache_set_block(struct cache_t *cache, int set, int way, unsigned int tag, int state);
void cache_get_block(struct cache_t *cache, int set, int way, unsigned int *tag_ptr, int *state_ptr);

void cache_access_block(struct cache_t *cache, int set, int way);
int cache_replace_block(struct cache_t *cache, int set);
void cache_set_transient_tag(struct cache_t *cache, int set, int way, unsigned int tag);

unsigned int cache_set_victim_tag(struct cache_t *cache, int set, int way);
void cache_set_insert_cycle(struct cache_t *cache, int set, int way,long long cycle_time);
long long  cache_get_insert_cycle(struct cache_t *cache, int set, int way);

//cjc_20121215 get and set dead blck flags
int cache_get_dead_block_flag(struct cache_t *cache, int set, int way);
//void cache_set_dead_block_flag(struct cache_t *cache, int set, int way, int flag);
//IC_S_2012_12_27
void cache_set_dead_block_flag(struct cache_t *cache, int set, int way, enum dead_block_state_t flag);
//IC_E
void cache_set_invalid_to_tail(struct cache_t *cache, int set, int way);//cjc_20130102
void cache_set_stack(struct cache_t *cache, int set, int way, struct mod_stack_t *stack);//cjc_20130107
struct mod_stack_t * cache_get_stack(struct cache_t *cache, int set, int way);//cjc_20130107
void cache_dump_coherence_state(struct cache_t *cache);//cjc_20130411
void prefetcher_pollution_bloom_filter_set(struct cache_t *cache,unsigned int tag,int is_set);//cjc_20130715
void prefetcher_pollution_bloom_filter_reset(struct cache_t *cache);//cjc_20130715
void prefetcher_pollution_bloom_filter_calc(struct mod_stack_t *stack,int used_mod);//cjc_20130715
#endif


