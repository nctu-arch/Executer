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

#ifndef MEM_SYSTEM_MODULE_H
#define MEM_SYSTEM_MODULE_H

#include <stdio.h>


/* Port */
struct mod_port_t
{
	/* Port lock status */
	int locked;
	long long lock_when;  /* Cycle when it was locked */
	struct mod_stack_t *stack;  /* Access locking port */

	/* Waiting list */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;
};

/* String map for access type */
extern struct str_map_t mod_access_kind_map;

/* Access type */
enum mod_access_kind_t
{
	mod_access_invalid = 0,
	mod_access_load,
	mod_access_store,
	mod_access_nc_store,
	mod_access_prefetch,
	mod_access_L1DCUHint //IC_S_2013_11_13
};

/* Module types */
enum mod_kind_t
{
	mod_kind_invalid = 0,
	mod_kind_cache,
	mod_kind_main_memory,
	mod_kind_local_memory
};

/* Any info that clients (cpu/gpu) can pass
 * to the memory system when mod_access() 
 * is called. */
struct mod_client_info_t
{
	/* This field is for use by the prefetcher. It is set
	 * to the PC of the instruction accessing the module */
	unsigned int prefetcher_eip;
	int lower_level_insertion;//cjc_20130608
	int stream_entry_id;
};

/* Type of address range */
enum mod_range_kind_t
{
	mod_range_invalid = 0,
	mod_range_bounds,
	mod_range_interleaved
};

#define MOD_ACCESS_HASH_TABLE_SIZE  17

/* Memory module */
struct mod_t
{
	/* Parameters */
	enum mod_kind_t kind;
	char *name;
	int block_size;
	int log_block_size;
	int latency;
	int dir_latency;
	int mshr_size;
	int mod_id;//cjc_20130207
	int data_mod;//cjc_20130207	
	/* Module level starting from entry points */
	int level;

	/* Address range served by module */
	enum mod_range_kind_t range_kind;
	union
	{
		/* For range_kind = mod_range_bounds */
		struct
		{
			unsigned int low;
			unsigned int high;
		} bounds;

		/* For range_kind = mod_range_interleaved */
		struct
		{
			unsigned int mod;
			unsigned int div;
			unsigned int eq;
		} interleaved;
	} range;

	/* Ports */
	struct mod_port_t *ports;
	int num_ports;
	int num_locked_ports;

	/* Accesses waiting to get a port */
	struct mod_stack_t *port_waiting_list_head;
	struct mod_stack_t *port_waiting_list_tail;
	int port_waiting_list_count;
	int port_waiting_list_max;

	/* Directory */
	struct dir_t *dir;
	int dir_size;
	int dir_assoc;
	int dir_num_sets;

	/* Waiting list of events */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;

	/* Cache structure */
	struct cache_t *cache;
	struct cache_t *tlb;	//porshe_20131025: create a simple TLB in L1 module for generating TLB miss info

	//IC_S_2013_11_05
	//struct cache_t *L1_hint_history;
	//IC_E

	//IC_S_2013_12_03
	struct cache_t *L1hint;
	struct list_t *L1hint_list0; //4 cores
	struct list_t *L1hint_list1; //4 cores
	struct list_t *L1hint_list2; //4 cores
	struct list_t *L1hint_list3; //4 cores
	//IC_E
	
	/* Low and high memory modules */
	struct linked_list_t *high_mod_list;
	struct linked_list_t *low_mod_list;

	/* Smallest block size of high nodes. When there is no high node, the
	 * sub-block size is equal to the block size. */
	int sub_block_size;
	int num_sub_blocks;  /* block_size / sub_block_size */

	/* Interconnects */
	struct net_t *high_net;
	struct net_t *low_net;
	struct net_node_t *high_net_node;
	struct net_node_t *low_net_node;

	/* Access list */
	struct mod_stack_t *access_list_head;
	struct mod_stack_t *access_list_tail;
	int access_list_count;
	int access_list_max;

	/* Write access list */
	struct mod_stack_t *write_access_list_head;
	struct mod_stack_t *write_access_list_tail;
	int write_access_list_count;
	int write_access_list_max;

	/* prefetch access list */
	struct mod_stack_t *prefetch_access_list_head;
	struct mod_stack_t *prefetch_access_list_tail;
	int prefetch_access_list_count;
	int prefetch_access_list_max;

	//IC_S_2014_07_15
	int prefetch_inflight_list_count;
	//IC_E
	
	/* Number of in-flight coalesced accesses. This is a number
	 * between 0 and 'access_list_count' at all times. */
	int access_list_coalesced_count;

	/* Clients (CPU/GPU) that use this module can fill in some
	 * optional information in the mod_client_info_t structure.
	 * Using a repos_t memory allocator for these structures. */
	struct repos_t *client_info_repos;

	/* Hash table of accesses */
	struct
	{
		struct mod_stack_t *bucket_list_head;
		struct mod_stack_t *bucket_list_tail;
		int bucket_list_count;
		int bucket_list_max;
	} access_hash_table[MOD_ACCESS_HASH_TABLE_SIZE];

	/* Architecture accessing this module. For versions of Multi2Sim where it is
	 * allowed to have multiple architectures sharing the same subset of the
	 * memory hierarchy, the field is used to check this restriction. */
	struct arch_t *arch;

	int ideal_pf_cache;//cjc_20121219
	int ideal_pf_cache_contention;//cjc_20130121
	int ideal_pf_cache_pollution;//cjc_20130121
	int ideal_pf_latency_hiding;//cjc_20130123
	int one_fourth_block_mask;//cjc_20130128	
	struct cache_t *pf_cache;//cjc_20121219
	
	struct dram_ctrl* dram_controller;//cjc_20130315
	int dram_num_banks;//cjc_20130315
	int dram_num_channel;//cjc_20130319
	
	/* Statistics */
	long long accesses;
	long long hits;

	long long reads;
	long long effective_reads;
	long long effective_read_hits;
	long long writes;
	long long effective_writes;
	long long effective_write_hits;
	long long nc_writes;
	long long effective_nc_writes;
	long long effective_nc_write_hits;
	long long prefetches;
	long long prefetch_aborts;
	long long useless_prefetches;
	long long evictions;

	long long blocking_reads;
	long long non_blocking_reads;
	long long read_hits;
	long long blocking_writes;
	long long non_blocking_writes;
	long long write_hits;
	long long blocking_nc_writes;
	long long non_blocking_nc_writes;
	long long nc_write_hits;

	//cjc_20121206 prefetch statistic
        long long pf_already_in_cache;
	long long pf_already_in_MSHR;
        long long pf_indentified;
	long long pf_issued;
        long long pf_hits;
        long long pf_pollution;
        long long pf_hit_by_other_core;
        long long pf_early_cycle;
	long long pf_early_cycle_upper_level;
        long long pf_hit_by_upper_level;
        long long pf_late_load;
        long long pf_late_store;
        long long pf_late_cycle_load;
        long long pf_late_cycle_store;

	//cjc_20121215 statistic for throttle
	long long cache_dead_block;
	//IC_S_2012_12_27
	long long cache_dead_block_demand;
	long long cache_dead_block_prefetch;
	long long cache_evict_block;
	//IC_E

	//cjc_20121226 ideal pf_cache statistic
	long long pf_ideal_cache_victim;
	long long pf_ideal_wait_cycle;
	long long pf_ideal_l1_retry;
	long long pf_ideal_l2_retry;
	long long pf_ideal_access;//cjc_20130104
	long long pf_ideal_skipped_mem_access;//cjc_20130104
	long long pf_ideal_hit_by_itself;//cjc_20130107
	long long pf_ideal_hit_by_other_core;//cjc_20130107
        long long pf_ideal_mem_access;//cjc_20130107
	long long pf_ideal_latency;//cjc_20130107
	long long pf_ideal_MSHR_hit;

	//porshe_20121125 for access latency
        long long  R_hit_on_self_l1;
        long long  W_hit_on_self_l1;
        long long  R_hit_on_remote_l1;
        long long  W_hit_on_remote_l1;
        long long  R_hit_on_l2;
        long long  W_hit_on_l2;
        long long  R_hit_on_memory;
        long long  W_hit_on_memory;
        long long  R_l1_wo_depend_hit_num;
        long long  W_l1_wo_depend_hit_num;
        long long  R_l1_w_depend_hit_num;
        long long  W_l1_w_depend_hit_num;
        long long  R_l1_remote_hit_num;
        long long  W_l1_remote_hit_num;
        long long  R_l2_hit_num;
        long long  W_l2_hit_num;
        long long  R_mm_hit_num;
        long long  W_mm_hit_num;
		
	long long read_retries;
	long long write_retries;
	long long nc_write_retries;

	long long no_retry_accesses;
	long long no_retry_hits;
	long long no_retry_reads;
	long long no_retry_read_hits;
	long long no_retry_writes;
	long long no_retry_write_hits;
	long long no_retry_nc_writes;
	long long no_retry_nc_write_hits;

	long long cache_dead_block_demand_caused_by_pf;//cjc_20130202
	long long pf_issued_get_from_mem;

	long long demand_accesses;//cjc_20130305
        long long demand_hits;//cjc_20130305
	long long demand_reads;//cjc_20130411
	long long demand_writes;//cjc_20130411

	//IC_S_2013_10_23
	//XXX
	long long Demand_Access;
	long long Demand_Hit;
	long long Demand_Read;
	long long Demand_Write;
	//IC_E
	long long dram_access_latency;
	long long dram_access_latency_demand;
	long long dram_access_latency_prefetch;
	long long dram_access_latency_others;
	long long dram_access;
	long long dram_access_demand;
	long long dram_access_prefetch;
	long long dram_access_others;
	long long dram_rowbuffer_hit;
	long long dram_demand_queuing_cycle;
	long long dram_prefetch_queuing_cycle;
	long long dram_others_queuing_cycle;
	long long dram_bus_waiting_cycle;


	long long pf_in_cache_get_by_others;
	long long pf_in_cache_get_by_other_pf;
	long long pf_in_cache_get_by_itself;
	long long pf_in_cache_get_by_its_own_pf;

	long long pf_downup_invalidation;
	long long pf_late_store_same_pc;
	long long pf_late_load_same_pc;
	long long pf_late_stride_found;
	long long pf_late_stride_used;

	long long pf_ignored_cross_page_access;
	long long pf_pollution_FDP;//cjc_20130715  Feedback Directed Prefetching
	long long pf_late;
	
	long long stream_trained_stream;
	long long stream_renew_stream;
	long long stream_cleared_stream;
	long long stream_table_replace;

	long long acc_waiting_oi_num;
	long long acc_waiting_oi_cycle;

	long long acc_load_access_time;
	long long acc_lock_dir;
	long long acc_load_find_n_lock;
	long long acc_load_L1_L2_net;
	long long acc_load_L2_access;
	long long acc_load_reply;
	long long acc_load_MM_access;
	long long acc_load_peer_to_peer;
	long long acc_load_wait_old_write;
	long long acc_load_wait_old_inflight_cycle;
	
	
	long long acc_store_access_time;
	long long acc_store_find_n_lock;
	long long acc_store_L1_L2_net;
	long long acc_store_L2_access;
	long long acc_store_exclusive;
	long long acc_store_reply;
	long long acc_store_MM_access;
	long long acc_store_wait_old_access;
	long long acc_store_wait_old_inflight_cycle;

	long long local_degree[5];
	long long pf_pollution_HPAC;
	long long HPAC_BWC;
	long long HPAC_BWNO;

	//porshe_20131025: TLB statistics
	long long tlb_access;
	long long tlb_miss;
	//end
	//CIC 2013/11/11
	long long mshr_block;
	//
	//IC_S_2014_07_15
	long long prefetch_mshr_block;
	//IC_E
	
	//IC_S_2013_12_03
	long long hint_hit;
	long long hint_miss;
	long long hint_full;
	long long hint_empty;
	long long hint_trigger_num0;
	long long hint_trigger_num1;
	long long hint_trigger_num2;
	long long hint_trigger_num3;
	long long hint_miss_trigger0;
	long long hint_miss_trigger1;
	long long hint_miss_trigger2;
	long long hint_miss_trigger3;
	long long hint_change_trigger0;
	long long hint_change_trigger1;
	long long hint_change_trigger2;
	long long hint_change_trigger3;
	long long hint_change0;
	long long hint_change1;
	long long hint_change2;
	long long hint_change3;
	//IC_E
};

struct mod_t *mod_create(char *name, enum mod_kind_t kind, int num_ports,
	int block_size, int latency);
void mod_free(struct mod_t *mod);
void mod_dump(struct mod_t *mod, FILE *f);
void mod_stack_set_reply(struct mod_stack_t *stack, int reply);
struct mod_t *mod_stack_set_peer(struct mod_t *peer, int state);

long long mod_access(struct mod_t *mod, enum mod_access_kind_t access_kind, 
	unsigned int addr, int *witness_ptr, struct linked_list_t *event_queue,
	void *event_queue_item, struct mod_client_info_t *client_info);
int mod_can_access(struct mod_t *mod, unsigned int addr);

int mod_mshr_can_access(struct mod_t *mod, unsigned int addr);

int mod_find_block(struct mod_t *mod, unsigned int addr, int *set_ptr, int *way_ptr, 
	unsigned int *tag_ptr, int *state_ptr);

void mod_block_set_prefetched(struct mod_t *mod, unsigned int addr, int val);
int mod_block_get_prefetched(struct mod_t *mod, unsigned int addr);

void mod_lock_port(struct mod_t *mod, struct mod_stack_t *stack, int event);
void mod_unlock_port(struct mod_t *mod, struct mod_port_t *port,
	struct mod_stack_t *stack);

void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack,
	enum mod_access_kind_t access_kind);
void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack);

int mod_in_flight_access(struct mod_t *mod, long long id, unsigned int addr);
struct mod_stack_t *mod_in_flight_address(struct mod_t *mod, unsigned int addr,
	struct mod_stack_t *older_than_stack);
struct mod_stack_t *mod_in_flight_write(struct mod_t *mod,
	struct mod_stack_t *older_than_stack);
struct mod_stack_t *mod_in_flight_oldest_address(struct mod_t *mod, unsigned int addr);

int mod_serves_address(struct mod_t *mod, unsigned int addr);
struct mod_t *mod_get_low_mod(struct mod_t *mod, unsigned int addr);

int mod_get_retry_latency(struct mod_t *mod);

struct mod_stack_t *mod_can_coalesce(struct mod_t *mod,
	enum mod_access_kind_t access_kind, unsigned int addr,
	struct mod_stack_t *older_than_stack);
void mod_coalesce(struct mod_t *mod, struct mod_stack_t *master_stack,
	struct mod_stack_t *stack);

struct mod_client_info_t *mod_client_info_create(struct mod_t *mod);
void mod_client_info_free(struct mod_t *mod, struct mod_client_info_t *client_info);

long long mod_current_esim_cycle(void);
void mod_cal_pf_pollution(struct mod_t *mod, int set, unsigned int tag);

void mod_block_set_pf_owner(struct mod_t *mod, unsigned int addr, int val);//cjc_20130208
int mod_block_get_pf_owner(struct mod_t *mod, unsigned int addr);//cjc_20130208

void mod_block_set_owner(struct mod_t *mod, unsigned int addr, int val);
int mod_block_get_owner(struct mod_t *mod, unsigned int addr);

void mod_clear_pf_bits(struct mod_t *mod);//cjc_20130219
struct mod_t *mod_get_main_memory(struct mod_t *mod); //cjc_20130422
void mod_dump_cache_coherence_state(struct mod_t *mod);
#endif

