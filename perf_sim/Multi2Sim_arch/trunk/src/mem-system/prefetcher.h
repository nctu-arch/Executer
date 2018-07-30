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

#ifndef MEM_SYSTEM_PREFETCHER_H
#define MEM_SYSTEM_PREFETCHER_H


extern int EV_PF_TIME_INTERVAL;//cjc_20130513

extern char *prefetch_stride_table_ref_input ;
extern char *prefetch_stride_table_ref_output;
extern FILE *ref_file;

extern char *prefetch_stride_sequence_out ;
extern FILE *seq_file;

extern struct prefetcher_reference_table ref_table;
/* 
 * This file implements a global history buffer
 * based prefetcher. Refer to the 2005 paper by
 * Nesbit and Smith. 
 */

//IC_S_2013_09_24
//char prefetch_result;
//IC_E

extern struct str_map_t prefetcher_type_map;
enum prefetcher_type_t
{
	prefetcher_type_invalid = 0,
	prefetcher_type_ghb_pc_cs,
	prefetcher_type_ghb_pc_dc,
	prefetcher_type_stride,			//cjc_20121210
	prefetcher_type_ghb_pc_dc_mg,		//cjc_20130305  
	prefetcher_type_dcu,		//cjc_20130709  intel DCU (streamer) prefetcher, have no detailed information about the hardware design of this, 
						//		I implement the function I think 
						//              (When detecting multiple loads from the same line within a time limit, prefetches the next line)

	prefetcher_type_stream,
	prefetcher_type_intel_ip,
};

enum ip_prefetcher_state
{
	ip_state_init = 0,
	ip_state_steady,
	ip_state_trasient,
	ip_state_no_pred,
};

/* Doesn't really make sense to have a big lookup depth */
#define PREFETCHER_LOOKUP_DEPTH_MAX 4


#define PREFETCHER_DELTA_BUFFER_MAX 256

//cjc_20130308 maximum number of link-traversing in pc_dc_mg
#define PREFETCHER_MAX_STREAM_TRAVERSE 16

#define FINE_GRAINED_INTERVAL 200
#define FINE_GRAINED_COUNT_MAX 3

#define REF_TABLE_SIZE 512


struct fine_graind_parameter
{
	int degree;
	int distance;
};

struct nxt_link_parameter
{
	int addr;
	int stride;
	int core_id;
};

enum stream_table_type_t
{
	stream_type_train =0,
	stream_type_stream,
	stream_type_dcu
};
enum stream_replace_type_t
{
	stream_type_to_LRU = 0,
	stream_type_to_MRU,
	stream_type_to_replace,
	stream_type_to_free
};
struct stream_training_entry
{
	int id;
	unsigned int start_addr;
	int second_addr_dist;
	int end_addr_dist;
	int ascending;
	int trained;
};

struct stream_streaming_entry
{
	int id;
	unsigned int original_addr;
	int region_start_baddr_dist;
	int region_end_addr;
	int last_access_addr_dist;
	int stride;
	int ascending;
	int stride_confirm;
	int csd1;
	int csd2;
	int isrepeat;

	int degree;
	int distance;
	int timing_counter;
	int lateness;
};

struct dcu_entry
{	
	int id;
	unsigned int address;
	int access_count;	
	int had_prefetched;
	int history[10];
};

struct stream_table
{
	struct stream_training_entry *train_entry;
	struct stream_streaming_entry *stream_entry;
	struct dcu_entry *dcu_entry;
	enum stream_table_type_t type;
		
	int entry_count;	
	//int pf_distance;
	struct linked_list_t free_list;
	struct linked_list_t LRU_list;
};

/* Global history buffer. */
struct prefetcher_ghb_t
{
	/* The global address miss this entry corresponds to */
	unsigned int addr;
	/* Next element in the linked list : -1 implies none */
	int next;
	/* Previous element in the linked list : -1 implies none */
	int prev;
	/* Whether the previous element is a GHB entry or a pointer
	 * to the index table. */
	enum prefetcher_ptr_t
	{
		prefetcher_ptr_ghb = 0,
		prefetcher_ptr_it,
	} prev_it_ghb;
};

/* Index table. */
struct prefetcher_it_t
{
	/* The tag to compare to before indexing into the table */
	unsigned int tag;
	/* Pointer into the GHB. -1 implies no entry in GHB. */
   	int ptr;

	int next_it;//cjc_20130308
	int valid_next_it;//cjc_20130308
	int link_strength;//cjc_20130308
	int timing_counter;//cjc_20130722
	int degree;//cjc_20130722
	int distance;//cjc_20130722
	int lateness;//cjc_20130722
};

//cjc_20121210 stride_table
struct stride_table
{
        int *pc;
        int *addr;
        int *stride;
        int *confidence;
	int *timing_counter;
	int *degree;
	int *distance;
	int *lateness;
	int *next_entry;
	int *prev_entry;
	int *link_strength;
	int *core_id;
        int count;
        int last;	
	int *last_entry_used;	
	int *state_machine;
	
	long long *used_count;
	long long *reused_interval;
	long long *last_used_esim;
	long long *stride_hit_count;
	long long *cross_page_stride_hit;
};


/* The main prefetcher object */
struct prefetcher_t
{
	int ghb_size;
	int it_size;
	int lookup_depth;
	int prefetch_on_hit;//cjc_20121210 flag of enable prefetching when cache hit
	int pf_distance;//cjc_20130108 stride prefetcher's distance
	int pf_degree;//cjc_20130304
	int fine_grained_ctrl;
		
	int lower_level_insertion;	/*cjc_20121212 let prefetech destination become lower level cache
					 * if pf1 is L1-pfer, destination will be L1 when this value ==0, L2 when ==1, ...
					 * if pf2 is L2-pfer, destination will be L2 when this value ==0, L3 when ==1, ...
					 * destination won't be main memory */
	enum prefetcher_type_t type;

	struct stride_table spt;//cjc_20121210 Stride Prediction Table
	struct prefetcher_ghb_t *ghb;
	struct prefetcher_it_t *index_table;

	int ghb_head;
	
	int ghb_last_it_index;//cjc_20130308 use in pc/dc/mg to record the last used it-index
	int is_counting;//cjc_20130513
	long long evicted_entry_count;//cjc_20130521
	long long evicted_useful_entry_count;//cjc_20130521
	int stride_chain;
	//IC_S_2013_11_05
	int L1_DCU_hint;
	int L1_DCU_hint_coreid;
	int L1_DCU_hint_prefetch_distance;
	//IC_E
	//IC_S_2014_03_25
	int L2_DCU_ActbyHint;
	//IC_E
	//IC_S_2013_10_15
	int stride_coreid_match;
	//IC_E
	//IC_S_2013_11_26
	int L1_DCU_hint_L2_prefetch_only_be_triggered_by_hint;
	//IC_E
	//struct prefetcher_pollution_bloom_filter FDP_filter;//cjc_20130715
	int dcu_bidirection;
	struct stream_table *train_table;
	struct stream_table *stream_table;
	struct stream_table *dcu_table;
	int log_block_size;
};

struct prefetcher_reference_table
{
        int count;
        int valid[REF_TABLE_SIZE];
        int pc[REF_TABLE_SIZE];
        int degree[REF_TABLE_SIZE];
};



struct mod_stack_t;
struct mod_t;

struct prefetcher_t *prefetcher_create(int prefetcher_ghb_size, int prefetcher_it_size,
				       int prefetcher_lookup_depth, enum prefetcher_type_t type);
void prefetcher_free(struct prefetcher_t *pref);

void prefetcher_access_miss(struct mod_stack_t *stack, struct mod_t *mod);
void prefetcher_access_hit(struct mod_stack_t *stack, struct mod_t *mod);

//cjc_20121207
int prefetcher_find_stride(struct mod_t *mod,int pc, int addr, int cache_hit, struct fine_graind_parameter *fine_graind,int clear_late, int core_id);

//cjc_20121212
//struct mod_t *pf_find_destination(struct mod_stack_t *stack, struct mod_t *mod);
struct mod_t *pf_find_destination(unsigned int tag, struct mod_t *mod,int ref);

void prefetcher_handler(int event, void *data);
int prefetcher_calc_spt_timing_counter(struct prefetcher_t *pref);
void prefetcher_access_hit_MSHR(struct mod_stack_t *stack, struct mod_t *target_mod);
void prefetcher_set_stride_lateness(struct mod_t *target_mod, int pc);
void prefetcher_dump_stride_table(struct mod_t *mod,FILE *f,int which,int dump_useful);
int calc_degree(int global,int local,int pc);
void prefetcher_ref_table_insert(int pc, int degree);
int prefetcher_ref_table_get(int pc, int ori_degree);
void prefetcher_construct_link(struct prefetcher_t *pref,int cur,int core_id);
int prefetcher_get_nxt_entry(struct prefetcher_t *pref,struct nxt_link_parameter *nxt, int index);
void prefetcher_streamer(struct mod_t *mod, struct mod_stack_t *stack);
void stream_LRU_replace(struct stream_table *table, void *data, enum stream_replace_type_t type);
void stream_write_table(int index,void *data);
void stream_clear_entry(enum stream_table_type_t type,void *data,void *assignment);
void stream_access_train_entry(struct mod_stack_t *stack,struct mod_t * mod);
void stream_move_to_stream_table(struct prefetcher_t *pref,struct stream_training_entry * moved_entry);
struct stream_streaming_entry * stream_access_stream_table(struct mod_stack_t *stack,struct mod_t * mod, int *lateness);
void stream_issue_prefetching(struct stream_streaming_entry *entry,struct mod_stack_t *stack,struct mod_t * mod);
void prefetcher_calc_local_degree(struct mod_t * mod, int degree);
void prefetcher_access_dcu(struct mod_stack_t *stack,struct mod_t * mod);
void prefetcher_access_intel_ip(struct mod_stack_t *stack,struct mod_t * mod, int do_pf);
#endif
