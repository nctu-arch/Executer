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

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <lib/util/file.h>
#include <lib/esim/esim.h>//cjc_20121217 in order to use esim_schedule_event()
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include "throttle.h"

#include "mem-system.h"
#include "prefetcher.h"
#include "cache.h"
#include "mod-stack.h"
#include "mmu.h"

#include <lib/Arch_stat/sim-stat.h>

#include <arch/x86/timing/cpu.h>

#define stride_link_start (1)
#define stride_used_count_start (1)
#define MAX_STRIDE_CONFIDENCE (7)
#define MAX_LINK_STRENGTH (7)

#define STREAM_TRAINING_TABLE_SIZE (64)
#define STREAM_STREAM_TABLE_SIZE (32)
#define STREAM_MAX_NEIGHBOR_DISTANCE (16)
#define STREAM_IS_NEIGHBOR(medium,target) ((target)>=(medium-STREAM_MAX_NEIGHBOR_DISTANCE)&&(target)<=(medium+STREAM_MAX_NEIGHBOR_DISTANCE))?1:0
#define STREAM_SET_REGION_START(entry,addr,log_size) (entry)->region_start_baddr_dist=((entry)->ascending>0)?(((entry)->region_end_addr>>log_size)-addr):(addr-((entry)->region_end_addr>>log_size))
#define STREAM_SET_LAST_ACCESS(entry,addr) (entry)->last_access_addr_dist=((entry)->ascending>0)?((entry)->region_end_addr-addr):(addr-(entry)->region_end_addr)

#define DCU_TABLE_SIZE (16)


int EV_PF_TIME_INTERVAL;//cjc_20130513
const int DEGREE_ARRAY[3]={1,2,4};

char *prefetch_stride_table_ref_input = "";
char *prefetch_stride_table_ref_output = "";

FILE *ref_file;
char *prefetch_stride_sequence_out ="";
FILE *seq_file;

//IC_S_2013_09_24
//FILE *fp;
//IC_E

static void prefetcher_ghb_pc_dc_mg(struct mod_t *mod, struct mod_stack_t *stack, int it_index);
int pf_debug_category;
struct str_map_t prefetcher_type_map =
{
	7, {
		{ "GHB_PC_CS", prefetcher_type_ghb_pc_cs },
		{ "GHB_PC_DC", prefetcher_type_ghb_pc_dc },
		{ "STRIDE", prefetcher_type_stride },
		{ "GHB_PC_DC_MG", prefetcher_type_ghb_pc_dc_mg },
		{ "DCU", prefetcher_type_dcu },
		{ "STREAM", prefetcher_type_stream},
		{ "INTEL_IP", prefetcher_type_intel_ip}
	}
};


struct prefetcher_reference_table ref_table;


//IC_S_2014_04_08
static int pf_hint_priority_compare(const void *a, const void *b)
{
	const struct pf_hint_t *pf_hint_a = a;
	const struct pf_hint_t *pf_hint_b = b;
	if (pf_hint_a->priority < pf_hint_b->priority)
		return 1;
	else if (pf_hint_a->priority > pf_hint_b->priority)
		return -1;
	else
		return 0;
	
}
//IC_E

struct prefetcher_t *prefetcher_create(int prefetcher_ghb_size, int prefetcher_it_size,
		int prefetcher_lookup_depth, enum prefetcher_type_t type)
{
	struct prefetcher_t *pref;
	/* Initialize */
	/* The global history buffer and index table cannot be 0
	 * if the prefetcher object is created. */
	assert(prefetcher_ghb_size >= 1 && prefetcher_it_size >= 1);
	pref = xcalloc(1, sizeof(struct prefetcher_t));
	pref->ghb_size = prefetcher_ghb_size;
	pref->it_size = prefetcher_it_size;
	pref->lookup_depth = prefetcher_lookup_depth;
	pref->type = type;
	pref->is_counting = 0;
	if(type==prefetcher_type_stride||type == prefetcher_type_intel_ip)	//cjc_20121210 create stride prefetcher
	{
		int i;
		//if(prefetcher_lookup_depth<0)
		//	fatal("Unsupported pf_degree");
		//pf_debug("degree=%d, table_entry=%d\n",lookup_depth,prefetcher_ghb_size);
		pref->spt.pc = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.addr = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.stride = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.confidence = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.timing_counter = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.degree = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.distance = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.lateness = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.used_count = xcalloc(prefetcher_ghb_size,sizeof(long long));
		pref->spt.reused_interval = xcalloc(prefetcher_ghb_size,sizeof(long long));
		pref->spt.last_used_esim = xcalloc(prefetcher_ghb_size,sizeof(long long));
		pref->spt.stride_hit_count = xcalloc(prefetcher_ghb_size,sizeof(long long));
		pref->spt.cross_page_stride_hit = xcalloc(prefetcher_ghb_size,sizeof(long long));
		pref->spt.next_entry = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.prev_entry = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.link_strength = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.last_entry_used = xcalloc(x86_cpu_num_cores,sizeof(int));
		pref->spt.core_id = xcalloc(prefetcher_ghb_size,sizeof(int));
		pref->spt.state_machine = xcalloc(prefetcher_ghb_size,sizeof(int));
		
		for(i = 0;i < x86_cpu_num_cores;i++)
		{
			pref->spt.last_entry_used[i] = -1;
		}
		pref->prefetch_on_hit = 1;
		pref->evicted_entry_count = 0;
		pref->evicted_useful_entry_count = 0;
		//pref->fine_grained_ctrl = 0;
		//pref->pf_distance = prefetcher_it_size;
		//assert(0);
		
		return pref;
	}

	else if (type==prefetcher_type_stream)
	{
		int i;
		pref->train_table = xcalloc(1,sizeof(struct stream_table));
		pref->train_table->train_entry = xcalloc(STREAM_TRAINING_TABLE_SIZE,sizeof(struct stream_training_entry));
		pref->train_table->type = stream_type_train;
		pref->train_table->entry_count = STREAM_TRAINING_TABLE_SIZE;
		for( i = 0; i < STREAM_TRAINING_TABLE_SIZE; i++)
		{
			pref->train_table->train_entry[i].id = i;
			stream_clear_entry(stream_type_train,&pref->train_table->train_entry[i],NULL);
			linked_list_add(&pref->train_table->free_list,&pref->train_table->train_entry[i]);
		}	
		
		assert(linked_list_count(&pref->train_table->free_list)==STREAM_TRAINING_TABLE_SIZE);	
			
		pref->stream_table = xcalloc(1,sizeof(struct stream_table));
		pref->stream_table->stream_entry = xcalloc(STREAM_STREAM_TABLE_SIZE,sizeof(struct stream_streaming_entry));
		pref->stream_table->type = stream_type_stream;
		pref->stream_table->entry_count = STREAM_STREAM_TABLE_SIZE;	
	
		for( i = 0; i < STREAM_STREAM_TABLE_SIZE; i++)
		{
			pref->stream_table->stream_entry[i].id = i;
			stream_clear_entry(stream_type_stream,&pref->stream_table->stream_entry[i],NULL);
			linked_list_add(&pref->stream_table->free_list,&pref->stream_table->stream_entry[i]);
		}	
		
		assert(linked_list_count(&pref->stream_table->free_list)==STREAM_STREAM_TABLE_SIZE);


		return pref;
	}
	else if (type == prefetcher_type_dcu)
	{
		int i;
		pref->dcu_table = xcalloc(1,sizeof(struct stream_table));
		//pref->dcu_table->dcu_entry = xcalloc(DCU_TABLE_SIZE ,sizeof(struct dcu_entry));
		//IC_S_2014_07_15
		pref->dcu_table->dcu_entry = xcalloc(prefetcher_ghb_size ,sizeof(struct dcu_entry));
		//IC_E
		pref->dcu_table->type = stream_type_dcu;
		//pref->dcu_table->entry_count = DCU_TABLE_SIZE;	
		//IC_S_2014_07_15
		pref->dcu_table->entry_count = prefetcher_ghb_size;	
		//IC_E
	
		//for( i = 0; i < DCU_TABLE_SIZE; i++)
		//IC_S_2014_07_15
		for( i = 0; i < prefetcher_ghb_size; i++)
		//IC_E
                {
                        pref->dcu_table->dcu_entry[i].id = i;
                        stream_clear_entry(stream_type_dcu,&pref->dcu_table->dcu_entry[i],NULL);
                        linked_list_add(&pref->dcu_table->free_list,&pref->dcu_table->dcu_entry[i]);
                }
		
		//assert(linked_list_count(&pref->dcu_table->free_list)==DCU_TABLE_SIZE);
		//IC_S_2014_07_15
		assert(linked_list_count(&pref->dcu_table->free_list) == prefetcher_ghb_size);
		//IC_E
	
		return pref;
	}
	pref->ghb = xcalloc(prefetcher_ghb_size, sizeof(struct prefetcher_ghb_t));
	pref->index_table = xcalloc(prefetcher_it_size, sizeof(struct prefetcher_it_t));
	pref->ghb_head = -1;

	pref->ghb_last_it_index = -1;
	
	//prefetcher_pollution_bloom_filter_reset(pref);
	/* Return */
	return pref;
}

void prefetcher_free(struct prefetcher_t *pref)
{
	if(pref->type==prefetcher_type_stride || pref->type==prefetcher_type_intel_ip)//cjc_20121210 free stride prefetcher
	{
		free(pref->spt.pc);
		free(pref->spt.addr);
		free(pref->spt.stride);
		free(pref->spt.confidence);
		free(pref->spt.timing_counter);
		free(pref->spt.degree);
		free(pref->spt.distance);
		free(pref->spt.lateness);
		free(pref->spt.used_count);
		free(pref->spt.reused_interval);
		free(pref->spt.last_used_esim);
		free(pref->spt.cross_page_stride_hit);
		free(pref->spt.stride_hit_count);
		free(pref->spt.next_entry);
		free(pref->spt.prev_entry);
		free(pref->spt.link_strength);
		free(pref->spt.core_id);
		free(pref->spt.state_machine);
		free(pref);
	}
	else if (pref->type==prefetcher_type_stream)
        {
		linked_list_clear(&pref->train_table->LRU_list);
		linked_list_clear(&pref->train_table->free_list);
		free(pref->train_table->train_entry);
		free(pref->train_table);
		linked_list_clear(&pref->stream_table->LRU_list);
                linked_list_clear(&pref->stream_table->free_list);
		free(pref->stream_table->stream_entry);
		free(pref->stream_table);
		free(pref);
        }
	else if (pref->type == prefetcher_type_dcu)
	{
		linked_list_clear(&pref->dcu_table->LRU_list);
                linked_list_clear(&pref->dcu_table->free_list);
		free(pref->dcu_table->dcu_entry);
                free(pref->dcu_table);
		free(pref);
	}	
	else
	{
		free(pref->ghb);
		free(pref->index_table);
		free(pref);
	}
}

static void get_it_index_tag(struct prefetcher_t *pref, struct mod_stack_t *stack, 
		int *it_index, unsigned *tag)
{
	if (stack->client_info)
	{
		*it_index = stack->client_info->prefetcher_eip % pref->it_size;
		*tag = stack->client_info->prefetcher_eip;
	}
	else
	{
		*it_index = -1;
		*tag = 0;
	}
}

/* Returns it_index >= 0 if any valid update is made, negative otherwise. */
static int prefetcher_update_tables(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	struct prefetcher_t *pref = target_mod->cache->prefetcher;
	int ghb_index;
	unsigned int addr = stack->addr;
	int it_index, prev;
	unsigned it_tag;

	if(target_mod->cache->prefetcher->type == prefetcher_type_stride || target_mod->cache->prefetcher->type == prefetcher_type_intel_ip)//cjc_20121210 stride prefetcher
	{
		return -1;
	}
	assert(pref);

	/* Get the index table index */
	get_it_index_tag(pref, stack, &it_index, &it_tag);

	if (it_index < 0)
		return -1;

	assert(it_index < pref->it_size);

	ghb_index = (++(pref->ghb_head)) % pref->ghb_size;

	/* Remove the current entry in ghb_index, if its valid */
	if (pref->ghb[ghb_index].addr > 0)
	{
		prev = pref->ghb[ghb_index].prev;

		/* The prev field may point to either index table or ghb. */
		if (pref->ghb[ghb_index].prev_it_ghb == prefetcher_ptr_ghb)
		{
			/* prev_it_gb == 0 implies the previous element is in the GHB */
			assert(prev >= 0 && prev < pref->ghb_size);
			assert(pref->ghb[prev].next == ghb_index);
			pref->ghb[prev].next = -1;
		}
		else
		{
			assert(pref->ghb[ghb_index].prev_it_ghb == prefetcher_ptr_it);

			/* The element in index_table may have been replaced since this
			 * entry was put into the ghb. */
			if (prev >= 0)
			{
				assert(pref->index_table[prev].ptr == ghb_index);
				pref->index_table[prev].ptr = -1;
			}
		}
	}

	pref->ghb[ghb_index].addr = 0;
	pref->ghb[ghb_index].next = -1;
	pref->ghb[ghb_index].prev = -1;

	if (pref->index_table[it_index].tag > 0)
	{
		/* Replace entry in index_table if necessary. */
		if (pref->index_table[it_index].tag != it_tag)
		{
			mem_debug("  %lld it_index = %d, old_tag = 0x%x, new_tag = 0x%x" 
					"prefetcher: replace index_table entry\n", stack->id, 
					it_index, pref->index_table[it_index].tag, it_tag);

			prev = pref->index_table[it_index].ptr;

			/* The element in the ghb may have gone out by now. */
			if (prev >= 0)
			{
				/* The element that this is pointing to must be pointing back. */
				assert(pref->ghb[prev].prev_it_ghb == prefetcher_ptr_it && 
						pref->ghb[prev].prev == it_index);
				pref->ghb[prev].prev = -1;
			}

			pref->index_table[it_index].tag = 0;
			pref->index_table[it_index].ptr = -1;
			pref->index_table[it_index].valid_next_it=0;//cjc_20130310
			pref->index_table[it_index].link_strength = 0;

			//cjc_20130722
			pref->index_table[it_index].degree = 0;
			pref->index_table[it_index].distance = 0;
			pref->index_table[it_index].lateness = pref->pf_degree -1;
			pref->index_table[it_index].timing_counter = FINE_GRAINED_COUNT_MAX;
			
		}
		else//cjc_20130722
		{
			if(pref->index_table[it_index].timing_counter==0)
				pref->index_table[it_index].degree -= (pref->index_table[it_index].degree>-2&&pref->index_table[it_index].degree<=0)?1:0;	
			if(pref->is_counting==0)
			{
				pref->is_counting=1;
				if(pref->fine_grained_ctrl)
					esim_schedule_event(EV_PF_TIME_INTERVAL, pref,FINE_GRAINED_INTERVAL);
			}

		}
	}
	else
	{
		/* Just an initialization. Tag == 0 implies the entry has never been used. */
		pref->index_table[it_index].ptr = -1;
		pref->index_table[it_index].valid_next_it=0;//cjc_20130310
	}

	/* Add new element into ghb. */
	pref->ghb[ghb_index].addr = addr;
	pref->ghb[ghb_index].next = pref->index_table[it_index].ptr;
	if (pref->index_table[it_index].ptr >= 0)
	{
		prev = pref->index_table[it_index].ptr;
		assert(pref->ghb[prev].prev_it_ghb == prefetcher_ptr_it &&
				pref->ghb[prev].prev == it_index);
		pref->ghb[prev].prev_it_ghb = prefetcher_ptr_ghb;
		pref->ghb[prev].prev = ghb_index;
	}
	pref->ghb[ghb_index].prev_it_ghb = prefetcher_ptr_it;
	pref->ghb[ghb_index].prev = it_index;

	/* Make the index table entries point to current ghb_index. */
	pref->index_table[it_index].tag = it_tag;
	pref->index_table[it_index].ptr = ghb_index;

	/* Update pref->ghb_head so that its in the range possible. */
	pref->ghb_head = ghb_index;

	return it_index;
}

static void prefetcher_do_prefetch(struct mod_t *mod, struct mod_stack_t *stack,
		unsigned int prefetch_addr)
{
	int set1, set2 ;
	unsigned int tag1,tag2;
	assert(prefetch_addr > 0);

	/* Predicted prefetch_addr can go horribly wrong
	 * sometimes. Since prefetches aren't supposed to
	 * cause any kind of faults/exceptions, return. */
	if (!mod_serves_address(mod, prefetch_addr))
	{
		mem_debug("  miss_addr 0x%x, prefetch_addr 0x%x, %s : illegal prefetch\n", stack->addr,
				prefetch_addr, mod->name);
		return;
	}

	cache_decode_address(mod->cache, stack->addr, &set1, &tag1, NULL);
	cache_decode_address(mod->cache, prefetch_addr, &set2, &tag2, NULL);

	/* If the prefetch_addr is in the same block as the missed address
	 * there is no point in prefetching. One scenario where this may
	 * happen is when we see a stride smaller than block size because
	 * of an eviction between the two accesses. */
	if (set1 == set2 && tag1 == tag2)
		return;

	if(stack->is_stride_link==1)
	{
		if((stack->stride_link_addr&~mmu_page_mask)!=(prefetch_addr&~mmu_page_mask))
		{
			mod->pf_ignored_cross_page_access++;
			return;
		}
	}
	else{
		if((stack->addr&~mmu_page_mask)!=(prefetch_addr&~mmu_page_mask))  //cjc_20130605 two addresses must to present in the same page
		{
			mod->pf_ignored_cross_page_access++;
			return;
		}
	}
	/* I'm not passing back the mod_client_info structure. If this needs to be 
	 * passed in the future, make sure a copy is made (since the one that is
	 * pointed to by stack->client_info may be freed early. */
	mem_debug("  miss_addr 0x%x, prefetch_addr 0x%x, %s : prefetcher\n", stack->addr,
			prefetch_addr, mod->name);
	
	//IC_S_2013_10_29
	mem_debug_2("  %lld %lld PC 0x%x prefetch_address 0x%x trigger_address 0x%x %s prefetcher\n",
			esim_cycle, stack->id, stack->client_info->prefetcher_eip, prefetch_addr, stack->addr, mod->name);
	//IC_E
	
	if(esim_warm_up_finish)
	{
		mod->pf_indentified++;

		//IC_S_2014_07_15
		if( mod_prefetch_can_access(mod,prefetch_addr) == 1 )
			mod_access(mod, mod_access_prefetch, prefetch_addr, NULL, NULL, NULL, stack->client_info);
		//IC_E

		//if( mod_can_access(mod,prefetch_addr) == 1 )
		//	mod_access(mod, mod_access_prefetch, prefetch_addr, NULL, NULL, NULL, stack->client_info);


		//IC_S_2013_11_13
		//if (mod->cache->prefetcher->L1_DCU_hint == 1)
		//{
		//	mod_access(mod, mod_access_L1DCUHint, prefetch_addr, NULL, NULL, NULL, stack->client_info);
		//}
		//else
		//{
		//	mod_access(mod, mod_access_prefetch, prefetch_addr, NULL, NULL, NULL, stack->client_info);
		//}
		//IC_E

		//mod_access(mod, mod_access_prefetch, prefetch_addr, NULL, NULL, NULL, NULL);
	}
}

/* This function implements the GHB based PC/CS prefetching as described in the
 * 2005 paper by Nesbit and Smith. The index table lookup is based on the PC
 * of the instruction causing the miss. The GHB entries are looked at for finding
 * constant stride accesses. Based on this, prefetching is done. */
static void prefetcher_ghb_pc_cs(struct mod_t *mod, struct mod_stack_t *stack, int it_index)
{
	struct prefetcher_t *pref;
	int chain, stride, i;
	unsigned int prev_addr, cur_addr, prefetch_addr = 0;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

	chain = pref->index_table[it_index].ptr;

	/* The lookup depth must be at least 2 - which essentially means
	 * two strides have been seen so far, prefetch for the next. 
	 * It doesn't really help to prefetch on a lookup of depth 1.
	 * It is too low an accuracy and leads to lot of illegal and
	 * redundant prefetches. Hence keeping the minimum at 2. */
	assert(pref->lookup_depth >= 2);

	/* The table should've been updated before calling this function. */
	assert(pref->ghb[chain].addr == stack->addr);

	/* If there's only one element in this linked list, nothing to do. */
	if (pref->ghb[chain].next == -1)
		return;

	prev_addr = pref->ghb[chain].addr;
	chain = pref->ghb[chain].next;
	cur_addr = pref->ghb[chain].addr;
	stride = prev_addr - cur_addr;

	for (i = 2; i <= pref->lookup_depth; i++)
	{
		prev_addr = cur_addr;
		chain = pref->ghb[chain].next;

		/* The linked list (history) is smaller than the lookup depth */
		if (chain == -1)
			break;

		cur_addr = pref->ghb[chain].addr;

		/* The stride changed, can't prefetch */
		if (stride != prev_addr - cur_addr)
			break;

		/* If this is the last iteration (we've seen as much history as
		 * the lookup depth specified), then do a prefetch. */
		if (i == pref->lookup_depth)
			prefetch_addr = stack->addr + stride;
	}

	if (prefetch_addr > 0)
		prefetcher_do_prefetch(mod, stack, prefetch_addr);
}

/* This function implements the GHB based PC/DC prefetching as described in the
 * 2005 paper by Nesbit and Smith. The index table lookup is based on the PC
 * of the instruction causing the miss. The last three accesses are looked at
 * to find the last two strides (deltas). The list is then looked up backwards
 * to see if this pair of strides occurred earlier, if yes, the next stride
 * is obtained from the history there. This stride decides the new prefetch_addr. */
static void prefetcher_ghb_pc_dc(struct mod_t *mod, struct mod_stack_t *stack, int it_index)
{
	struct prefetcher_t *pref;
	int chain, chain2, stride[PREFETCHER_LOOKUP_DEPTH_MAX], i, pref_stride;
	int delta_buffer[PREFETCHER_DELTA_BUFFER_MAX];//cjc_20130304 use to store delta between ghb entries
	int delta_count=0;
	unsigned int prev_addr, cur_addr, prefetch_addr = 0;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

	chain = pref->index_table[it_index].ptr;

	/* The lookup depth must be at least 2 - which essentially means
	 * two strides have been seen so far, predict the next stride. */ 
	assert(pref->lookup_depth >= 2 && pref->lookup_depth <= PREFETCHER_LOOKUP_DEPTH_MAX);

	/* The table should've been updated before calling this function. */
	assert(pref->ghb[chain].addr == stack->addr);

	/* Collect "lookup_depth" number of strides (deltas).
	 * This doesn't really make sense for a depth > 2, but
	 * I'll just have the code here for generality. */
	for (i = 0; i < pref->lookup_depth; i++)
	{
		prev_addr = pref->ghb[chain].addr;
		chain = pref->ghb[chain].next;

		/* The chain isn't long enough */
		if (chain == -1)
			return;

		cur_addr = pref->ghb[chain].addr;
		stride[i] = prev_addr - cur_addr;
	}

	delta_count = 1;
	delta_buffer [0] = stride[0];

	chain = pref->index_table[it_index].ptr;
	chain = pref->ghb[chain].next;
	assert(chain != -1);

	/* "chain" now points to the second element of the list.
	 * Try to match the stride array starting from here. */
	while (chain != -1)
	{
		/* This really doesn't look realistic to implement in 
		 * hardware. Too much time consuming I feel. */
		chain2 = chain;
		for (i = 0; i < pref->lookup_depth; i++)
		{
			prev_addr = pref->ghb[chain2].addr;
			chain2 = pref->ghb[chain2].next;

			/* The chain isn't long enough and we
			 * haven't found a match till now. */
			if (chain2 == -1)
				return;

			cur_addr = pref->ghb[chain2].addr;

			delta_buffer[delta_count+i] = prev_addr - cur_addr;

			if (stride[i] != prev_addr - cur_addr)
				break;
		}

		delta_count ++;//cjc_20130304


		/* If we traversed the above loop full, we have a match. */
		if (i == pref->lookup_depth)
		{
			cur_addr = pref->ghb[chain].addr;
			assert(pref->ghb[chain].prev != -1 && 
					pref->ghb[chain].prev_it_ghb == prefetcher_ptr_ghb);
			chain = pref->ghb[chain].prev;
			prev_addr = pref->ghb[chain].addr;
			pref_stride = prev_addr - cur_addr;
			prefetch_addr = stack->addr + pref_stride;
			break;
		}

		chain = pref->ghb[chain].next;
	}

	if (prefetch_addr > 0)
	{
		//cjc_20130304_start
		pref_stride = 0;
		int degree;
		int delta_buffer_ptr = delta_count -2;

		/*if(it_index%3==0)
		  degree=4;
		  else if(it_index%3==1)
		  degree=1;
		  else
		  degree=2;*/
		//degree = pref->pf_degree;
		degree = calc_degree(pref->pf_degree,pref->index_table[it_index].degree,0);
		prefetcher_calc_local_degree(mod,pref->index_table[it_index].degree);
		if(pref->index_table[it_index].degree==-2)
                	stack->client_info->lower_level_insertion=1;
		for(i = 0; i < degree; i++)
		{
			assert(delta_buffer_ptr>=0);
			pref_stride += delta_buffer[delta_buffer_ptr];
			prefetch_addr = stack->addr + pref_stride;
			if(((pref_stride>0&&prefetch_addr>stack->addr)||(pref_stride<0&&prefetch_addr<stack->addr))&&prefetch_addr>0)
			{
				prefetcher_do_prefetch(mod, stack, prefetch_addr);
			}
			delta_buffer_ptr--;
			if(delta_buffer_ptr<0)
			{
				delta_buffer_ptr = delta_count -2;
			}
		}
		//cjc_20130304_end

		pref->index_table[it_index].lateness = 0;

		//prefetcher_do_prefetch(mod, stack, prefetch_addr);   //cjc_20130304 original code
	}
}

void prefetcher_access_miss(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	int it_index;
	struct mod_t *dest_mod;
	if (target_mod->kind != mod_kind_cache ) 
		return;
	assert(stack->set>=0);
	//assert(stack->tag>=0);
	mod_cal_pf_pollution(target_mod,stack->set,stack->tag);

	prefetcher_pollution_bloom_filter_calc(stack,((long)target_mod==(long)stack->mod)?0:((long)target_mod==(long)stack->target_mod)?1:2);
	if(!target_mod->cache->prefetcher)
		return;
	if(target_mod->cache->prefetcher->type==prefetcher_type_stride)
	{
		int pc,tag,stride,i,pf_addr,distance,degree;
		struct fine_graind_parameter fine;

		if (!stack->client_info)
			//if (!stack->client_info||target_mod->cache->prefetcher->pf_degree==0)//cjc_20121210 it has no pc
			return;
		//if(stack->prefetch)
		//	return;
		pc = stack->client_info->prefetcher_eip;
		tag = (stack->addr )&( ~(target_mod->cache->block_mask));
		stride = prefetcher_find_stride(target_mod,pc,tag,0,&fine,1,stack->core_id);

		if(target_mod->cache->prefetcher->stride_chain)
		{

			struct nxt_link_parameter *prediction;
			int *entry,*use_count;
			int i,count,j;
			int loop = 0;
			if(stride!=-1)
			{
				degree = target_mod->cache->prefetcher->pf_degree;
				entry = xcalloc(degree+2,sizeof(int));
				use_count = xcalloc(degree+2,sizeof(int));
				prediction = xcalloc(degree+1,sizeof( struct nxt_link_parameter));

				for(i = 1;i <= degree;i++)
					entry[i] = -1;
				entry[0] = stride;
				use_count[0] = stride_used_count_start;
				dest_mod = target_mod;
				count = 1;
				//nxt_entry = prefetcher_get_nxt_entry(target_mod->cache->prefetcher, &first,i);

				for(i = 0; i <= degree;i++)
				{
					entry[i+1] = prefetcher_get_nxt_entry(target_mod->cache->prefetcher, &prediction[i],entry[i]);
					for(j = 0; j < count; j++)
					{
						if(entry[i+1]==entry[j])
						{
							entry[i+1]=-1;
							loop = 1;
						}
					}
					if(entry[i+1]!=-1)
					{
						count ++;
						use_count[i+1] = stride_used_count_start;
					}
					else
						break;
				}
				//for( i = 1; i <=count&&loop;i++)
				for( i = stride_link_start; i < degree + stride_link_start;i++)
				{
					if(count==1)
					{
						pf_addr = ((prediction[0].stride*i)+stack->addr);
						prefetcher_do_prefetch(dest_mod, stack, pf_addr);
					}
					else
					{
						int index = i % count;
						stack->stride_link_addr = (prediction[index].addr-prediction[0].addr+stack->addr);
						pf_addr = ((prediction[index].stride*use_count[index])+stack->stride_link_addr);
						//stack->is_stride_link = 1;
						use_count[index]++;
						prefetcher_do_prefetch(dest_mod, stack, pf_addr);
					}
				}
				stack->stride_link_addr = 0;
				free(entry);
				free(use_count);
				free(prediction);
			}
		}
		else
		{
			if(stride==0)
				return;
			distance = target_mod->cache->prefetcher->pf_distance;
			//degree = target_mod->cache->prefetcher->pf_degree;
			//degree = (degree+fine.degree>2)?4:degree+fine.degree;
			degree = calc_degree(target_mod->cache->prefetcher->pf_degree,fine.degree,pc);
			prefetcher_calc_local_degree(target_mod,fine.degree);
			//dest_mod = pf_find_destination(stack,target_mod);
			if(fine.degree==-2)
				stack->client_info->lower_level_insertion=1;
			dest_mod = target_mod;
			assert(dest_mod);
			for(i = 1; i <= degree;i++)
			{
				pf_addr = ((stride*i)*distance+stack->addr);
				if(!((tag>=0&&pf_addr>=0)||(tag<0&&pf_addr<0)))
					continue;

				prefetcher_do_prefetch(dest_mod, stack, pf_addr);

				//IC_S_2013_09_24
				//record stride
				//if (prefetch_result == 0 && fp)//0:hit, 1:miss
				//{
				//	fprintf(fp, "prefetch_hit_on_cache_miss %d\n", stride);
				//	fflush(fp);
                                //
				//}
				//else if (prefetch_result == 1 && fp)
				//{
				//	fprintf(fp, "prefetch_miss_on_cache_miss %d\n", stride);
				//	fflush(fp);
				//}
				//IC_E
			}
		}
		return;
	}
	else if (target_mod->cache->prefetcher->type == prefetcher_type_stream)
	{
		int lateness;
		struct stream_streaming_entry * entry;
		
		entry = stream_access_stream_table(stack,target_mod,&lateness);
		if(!entry)
			stream_access_train_entry(stack,target_mod);
		else
		{
			STREAM_SET_LAST_ACCESS(entry,(stack->tag>>target_mod->cache->prefetcher->log_block_size));
			if(lateness==0)
			{
				stream_issue_prefetching(entry,stack,target_mod);
			}
		}
		return;	
	}
	else if ( target_mod->cache->prefetcher->type ==prefetcher_type_dcu)
	{
		prefetcher_access_dcu(stack,target_mod);
		return;
	}
	else if ( target_mod->cache->prefetcher->type ==prefetcher_type_intel_ip)
        {
                prefetcher_access_intel_ip(stack,target_mod,1);
                return;
        }
	it_index = prefetcher_update_tables(stack, target_mod);

	if (it_index < 0)
		return;

	if (target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_cs)
	{
		/* Perform ghb based PC/CS prefetching
		 * (Program Counter based index, Constant Stride) */
		prefetcher_ghb_pc_cs(target_mod, stack, it_index);
	}
	else if(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc)
	{
		assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc);
		/* Perform ghb based PC/DC prefetching
		 * (Program Counter based index, Delta Correlation) */
		prefetcher_ghb_pc_dc(target_mod, stack, it_index);
	}
	else if(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc_mg)//cjc_20130305
	{
		assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc_mg);
		prefetcher_ghb_pc_dc_mg(target_mod, stack, it_index);
	}

}

void prefetcher_access_hit(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	int it_index;

	//cjc_20121206
	struct mod_t *dest_mod;
	int cache_level = target_mod->level;
	int pf_bit;
	if(target_mod->kind != mod_kind_cache)
		return;
	//cjc_20121207 statistic of useful pf_reques
	pf_bit=mod_block_get_prefetched(target_mod, stack->addr);
//printf("cache_level = %d, pf_bit = %d\n", cache_level, pf_bit);
	if(pf_bit==cache_level)
	{
		if(stack->mod->data_mod==1&&stack->mod->mod_id==mod_block_get_pf_owner(target_mod, stack->addr))
		{
			stack->mod->pf_hits++;
		}
		target_mod->pf_early_cycle+=(mod_current_esim_cycle()-cache_get_insert_cycle(target_mod->cache,stack->set,stack->way));
		target_mod->pf_hits++;
		esim_schedule_event(EV_THROTTLE_NEW_COUNTER, target_mod->cache->throttle,0);//cjc_20121217 notify throttle
	}
	else if(pf_bit<cache_level&&pf_bit!=0)
	{
		target_mod->pf_hit_by_upper_level++;
		target_mod->pf_early_cycle_upper_level+=(mod_current_esim_cycle()-cache_get_insert_cycle(target_mod->cache,stack->set,stack->way));
	}
	if(pf_bit)
	{
		mod_block_set_prefetched(target_mod, stack->addr, 0);	
	}
	if (!target_mod->cache->prefetcher)
		return;	


	if(target_mod->cache->prefetcher->type==prefetcher_type_stride)
	{
		int pc,tag,stride,i,pf_addr,distance,degree,j;
		struct fine_graind_parameter fine;

		if (!stack->client_info)
			//if (!stack->client_info||target_mod->cache->prefetcher->pf_degree==0)//cjc_20121210 it has no pc
			return;
		//if(stack->prefetch)
		//      return;
		pc = stack->client_info->prefetcher_eip;
		tag = (stack->addr )&( ~(target_mod->cache->block_mask));
		stride = prefetcher_find_stride(target_mod,pc,tag,1,&fine,1,stack->core_id);


		if(target_mod->cache->prefetcher->stride_chain)
		{
			struct nxt_link_parameter *prediction;
			int *entry,*use_count;
			int i,count;
			int loop = 0;
			if(stride!=-1)
			{
				degree = target_mod->cache->prefetcher->pf_degree;
				entry = xcalloc(degree+2,sizeof(int));
				use_count = xcalloc(degree+2,sizeof(int));
				prediction = xcalloc(degree+1,sizeof( struct nxt_link_parameter));

				for(i = 1;i <= degree;i++)
					entry[i] = -1;
				entry[0] = stride;
				use_count[0] = stride_used_count_start;
				dest_mod = target_mod;
				count = 1;
				//nxt_entry = prefetcher_get_nxt_entry(target_mod->cache->prefetcher, &first,i);

				for(i = 0; i <= degree;i++)
				{
					entry[i+1] = prefetcher_get_nxt_entry(target_mod->cache->prefetcher, &prediction[i],entry[i]);

					for(j = 0; j < count; j++)
					{
						if(entry[i+1]==entry[j])
						{
							entry[i+1]=-1;
							loop = 1;
						}
					}
					if(entry[i+1]!=-1)
					{
						count ++;
						use_count[i+1] = stride_used_count_start;

					}
					else
						break;
				}
				//for( i = 1; i <=count&&loop;i++)
				for( i = stride_link_start; i < degree + stride_link_start;i++)
				{
					if(count==1)
					{
						pf_addr = ((prediction[0].stride*i)+stack->addr);
						prefetcher_do_prefetch(dest_mod, stack, pf_addr);
					}
					else
					{
						int index = i % count;
						stack->stride_link_addr = (prediction[index].addr-prediction[0].addr+stack->addr);
						pf_addr = ((prediction[index].stride*use_count[index])+stack->stride_link_addr);
						//stack->is_stride_link = 1;
						use_count[index]++;
						prefetcher_do_prefetch(dest_mod, stack, pf_addr);
					}
				}
				stack->stride_link_addr = 0;
				free(entry);
				free(use_count);
				free(prediction);
			}
		}
		else
		{
			if(stride==0)
				return;
			distance = target_mod->cache->prefetcher->pf_distance;
			//degree = target_mod->cache->prefetcher->pf_degree;
			//degree = (degree+fine.degree>2)?4:degree+fine.degree;
			degree = calc_degree(target_mod->cache->prefetcher->pf_degree,fine.degree,pc);
			prefetcher_calc_local_degree(target_mod,fine.degree);
			//dest_mod = pf_find_destination(stack,target_mod);
			if(fine.degree==-2)
				stack->client_info->lower_level_insertion=1;
			dest_mod = target_mod;
			assert(dest_mod);
			for(i = 1; i <= degree;i++)
			{
				pf_addr = (stride*i*distance+stack->addr);
				if(!((tag>=0&&pf_addr>=0)||(tag<0&&pf_addr<0)))
					continue;	
				prefetcher_do_prefetch(dest_mod, stack, pf_addr);
				//IC_S_2013_09_24
				//record stride
				//if (prefetch_result == 0 && fp)//0:hit, 1:miss
				//{
				//	fprintf(fp, "prefetch_hit_on_cache_hit %d\n", stride);
				//	fflush(fp);
				//}
				//else if (prefetch_result == 1 && fp)
				//{
				//	fprintf(fp, "prefetch_miss_on_cache_hit %d\n", stride);
				//	fflush(fp);
				//}
				//IC_E
			}
		}
		return;
	}
	else if (target_mod->cache->prefetcher->type == prefetcher_type_stream)
        {
                int lateness;
                struct stream_streaming_entry * entry;

                entry = stream_access_stream_table(stack,target_mod,&lateness);
                if(!entry)
			return;
                else
                {
                        STREAM_SET_LAST_ACCESS(entry,(stack->tag>>target_mod->cache->prefetcher->log_block_size));
                        if(lateness==0)
                        {
                                stream_issue_prefetching(entry,stack,target_mod);
                        }
                }
                return;
        }

	else if ( target_mod->cache->prefetcher->type ==prefetcher_type_dcu)
        {
                prefetcher_access_dcu(stack,target_mod);
                return;
        }
	else if ( target_mod->cache->prefetcher->type ==prefetcher_type_intel_ip)
	{
		prefetcher_access_intel_ip(stack,target_mod,1);
		return;
	}

	if (pf_bit)
	{
		/* This block was prefetched. Now it has a real access. For the purposes
		 * of the prefetcher heuristic, this is still a miss. Hence, update
		 * the prefetcher tables. */
		//it_index = prefetcher_update_tables(stack, target_mod);

		/* Clear the prefetched flag since we have a real access now */
		mem_debug ("  addr 0x%x %s : clearing \"prefetched\" flag\n", 
				stack->addr, target_mod->name);
		mod_block_set_prefetched(target_mod, stack->addr, 0);

		it_index = prefetcher_update_tables(stack, target_mod);
		if (it_index < 0)
			return;

		if (target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_cs)
		{
			/* Perform ghb based PC/CS prefetching
			 * (Program Counter based index, Constant Stride) */
			prefetcher_ghb_pc_cs(target_mod, stack, it_index);
		}
		else if(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc)
		{
			assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc);
			/* Perform ghb based PC/DC prefetching
			 * (Program Counter based index, Delta Correlation) */
			prefetcher_ghb_pc_dc(target_mod, stack, it_index);
		}
		else //cjc_20130305
		{
			assert(target_mod->cache->prefetcher->type == prefetcher_type_ghb_pc_dc_mg);
			prefetcher_ghb_pc_dc_mg(target_mod, stack, it_index);
		}
	}
}
int prefetcher_find_stride(struct mod_t *mod,int pc, int addr, int cache_hit, struct fine_graind_parameter *fine_graind,int clear_late,int core_id)
{
	struct prefetcher_t *pref;
	int i, end, lowest,low_cond,last,cur,stride_chain=-1;
	int new_stride = 0;
	int stride_match=0;
	int cross_page_stride_match=0;
	int hit=0;

	pref = mod->cache->prefetcher;
	if(!pref)
		return 0;
	end = (pref->spt).count;
	last = (pref->spt).last;
	low_cond = (pref->spt).confidence[last];
	lowest = last;
	if((!(pref->prefetch_on_hit))&&cache_hit)
		return 0;
	pf_debug(" %lld %s: pc=%x\taddr=%x\tcache_hit=%d\n",mod_current_esim_cycle(),mod->cache->name,pc,addr,cache_hit);
	for(i=0;i<end;i++)
	{
		if((pref->spt).pc[i]==pc )//&& (pref->spt).core_id[i]!=core_id)
		{
			if(pref->stride_chain&&(pref->spt).core_id[i]!=core_id)
				return i;

			//IC_S_2013_10_15
			//Must check core_id, then calculate stride
			//If core_ic does not match, return (table miss)
			if ((pref->spt).core_id[i]!=core_id && pref->stride_coreid_match)
			{
				continue;
			}

			//IC_E
			stride_chain = i;
			pf_debug("\ttable hit @%d",i);
			hit =1;
			new_stride=addr-((pref->spt).addr[i]);
			stride_match=new_stride==(pref->spt).stride[i];

			cross_page_stride_match = ((!stride_match)&&(((addr - (pref->spt).stride[i])&mmu_page_mask)==((pref->spt).addr[i]&mmu_page_mask)))?1:0;


			(pref->spt).addr[i]=addr;
			if(clear_late)
				(pref->spt).lateness[i]=0;
			else if((pref->spt).lateness[i])
				(pref->spt).lateness[i]-=1;

			if(esim_warm_up_finish)
			{
				(pref->spt).used_count[i]++;
				//(pref->spt).reused_interval[i] += esim_cycle - (pref->spt).last_used_esim[i] ;
			}
			//(pref->spt).last_used_esim[i] = esim_cycle;



			if (new_stride!=0&&(stride_match||cross_page_stride_match))
			{
				(pref->spt).confidence[i]+=1;
				if((pref->spt).confidence[i]>MAX_STRIDE_CONFIDENCE)
					(pref->spt).confidence[i]=MAX_STRIDE_CONFIDENCE;
				if(cache_hit)
					pf_debug("\t(cache hit)");
				pf_debug("\tstride_match stride=%d\n",new_stride);

				if(cross_page_stride_match)
				{
					new_stride = (pref->spt).stride[i];
				}
				if(esim_warm_up_finish)
				{
					(pref->spt).stride_hit_count[i]++;
					//if((pref->spt).timing_counter[i] == 0)
					//	(pref->spt).degree[i] -= ((pref->spt).degree[i]>-2&&(pref->spt).degree[i]<=0)?1:0;
					(pref->spt).timing_counter[i] = ((pref->spt).degree[i]>=0)?FINE_GRAINED_COUNT_MAX:(FINE_GRAINED_COUNT_MAX<<1);
					(pref->spt).reused_interval[i] += esim_cycle - (pref->spt).last_used_esim[i] ;
					(pref->spt).cross_page_stride_hit[i] +=cross_page_stride_match;
					if(pref->is_counting==0)
					{
						pref->is_counting=1;
						if(pref->fine_grained_ctrl)
							esim_schedule_event(EV_PF_TIME_INTERVAL, pref,FINE_GRAINED_INTERVAL);		
					}
				}
				(pref->spt).last_used_esim[i] = esim_cycle;

				if(pref->stride_chain)
				{
					prefetcher_construct_link(pref,i,core_id);
				}
			}
			/*else if(new_stride==((pref->pf_degree+1)*(pref->spt).stride[i]))//2012/11/23 last number(mod->degree) of address is already cache
			  {
			  (pref->spt).confidence[i]+=1;
			  new_stride=(pref->spt).stride[i];
			  pf_debug("\tstride_match stride=%d (last %d is in cache)\n",new_stride,pref->pf_degree);
			  }*/
			else if(new_stride==0)
			{
				pf_debug("\tsame addr\n");
			}
			else
			{
				pf_debug("\tstride_mismatch new_stride=%d, old_stride=%d\n",new_stride,(pref->spt).stride[i]);
				if((pref->spt).confidence[i]<4)//cjc_20130611 keep old stride
				{
					(pref->spt).stride[i]=new_stride;
					if(esim_warm_up_finish)
					{
						(pref->spt).timing_counter[i] = ((pref->spt).degree[i]>=0)?FINE_GRAINED_COUNT_MAX:(FINE_GRAINED_COUNT_MAX<<1);	
					}
				}
				else if((pref->spt).confidence[i]>=7)
				{
					if(esim_warm_up_finish)
					{	
						(pref->spt).timing_counter[i] = ((pref->spt).degree[i]>=0)?FINE_GRAINED_COUNT_MAX:(FINE_GRAINED_COUNT_MAX<<1);
						(pref->spt).degree[i] = ((pref->spt).degree[i]<0)?0:(pref->spt).degree[i];
					}

				}
				(pref->spt).confidence[i]=(pref->spt).confidence[i]>>1;
				new_stride = 0;
				(pref->spt).last_used_esim[i] = esim_cycle;
			}

			fine_graind->distance = (pref->spt).distance[i]; 
			fine_graind->degree =   (pref->spt).degree[i];
			if(fine_graind->degree)
				mod->pf_late_stride_used++;

		}
		//find lowest confidence as a victim
		cur=(i+last)%(pref->ghb_size);
		if((pref->spt).confidence[cur]<low_cond)
		{
			lowest=cur;
			low_cond = (pref->spt).confidence[cur];
		}

	}
	if(hit)
		return (pref->stride_chain)?stride_chain:new_stride;

	else if(cache_hit)
		return (pref->stride_chain)?stride_chain:0;
	//table is not full
	else if(end<(pref->ghb_size)){

		(pref->spt).pc[end] = pc;
		(pref->spt).addr[end]=addr;
		(pref->spt).stride[end]=0;
		(pref->spt).confidence[end]=0;
		(pref->spt).count=end+1;
		(pref->spt).degree[end]=0;
		(pref->spt).distance[end]=0;
		(pref->spt).lateness[end]=0;
		(pref->spt).used_count[end]=0;	
		(pref->spt).reused_interval[end]=0;
		(pref->spt).stride_hit_count[end]=0;
		(pref->spt).last_used_esim[end]=esim_cycle;
		(pref->spt).cross_page_stride_hit[end]=0;
		(pref->spt).next_entry[end] = -1;
		(pref->spt).prev_entry[end] = -1;
		(pref->spt).link_strength[end] = 0;
		(pref->spt).core_id[end] = core_id;
		(pref->spt).timing_counter[end] = FINE_GRAINED_COUNT_MAX;
		pf_debug("\ttable miss:insert new element. current size=%d\n",end+1);
		return  (pref->stride_chain)?-1:0;
	}
	else
	{
		/*end=(mod->spt).last;
		  low_cond = (mod->spt).confidence[end];
		  lowest = end;
		  for(i=0;i<PF_BUF_SIZE;i++)
		  {
		  cur=(i+end)%PF_BUF_SIZE;
		  if((mod->spt).confidence[cur]<low_cond)
		  {
		  lowest=cur;
		  low_cond = (mod->spt).confidence[cur];
		  }
		  }*/
		pf_debug("\ttable miss:full replace=%d, pc=%x, last_addr=%x, stride=%d\n",lowest,(pref->spt).pc[lowest],(pref->spt).addr[lowest],(pref->spt).stride[lowest]);
		if(*prefetch_stride_table_ref_output)
		{
			prefetcher_dump_stride_table(mod,ref_file,lowest,1);	
		}
		(pref->spt).last=(last+1)%(pref->ghb_size);
		(pref->spt).pc[lowest] = pc;
		(pref->spt).addr[lowest]=addr;
		(pref->spt).stride[lowest]=0;
		(pref->spt).confidence[lowest]=0;
		(pref->spt).degree[lowest]=0;
		(pref->spt).distance[lowest]=0;
		(pref->spt).lateness[lowest]=0;
		(pref->spt).used_count[lowest]=0;
		(pref->spt).reused_interval[lowest]=0;

		(pref->spt).last_used_esim[lowest]=esim_cycle;

		(pref->spt).timing_counter[lowest] = FINE_GRAINED_COUNT_MAX;
		if(esim_warm_up_finish)
		{
			pref->evicted_entry_count++;
			if((pref->spt).stride_hit_count[lowest])
				pref->evicted_useful_entry_count++;
		}
		(pref->spt).stride_hit_count[lowest]=0;
		(pref->spt).cross_page_stride_hit[lowest]=0;
		(pref->spt).next_entry[lowest] = -1;
		(pref->spt).prev_entry[lowest] = -1;
		(pref->spt).link_strength[lowest] = 0;
		(pref->spt).core_id[lowest] = core_id;
		return (pref->stride_chain)?-1:0;

	}
}

struct mod_t *pf_find_destination(unsigned int tag , struct mod_t *mod,int ref)
{
	struct prefetcher_t *pref;
	struct mod_t * target;
	struct mod_t * tmp;
	int i,lower_level;


	pref = mod->cache->prefetcher;
	assert(pref);
	if(ref<0)
		lower_level = pref->lower_level_insertion;
	else
		lower_level = ref;
	target =mod;
	for(i=0;i<lower_level;i++)
	{
		tmp=mod_get_low_mod(target, tag);
		if(tmp->kind==mod_kind_cache)
			target=tmp;
		else
			break;
	}
	return target;
}

static void prefetcher_ghb_pc_dc_mg(struct mod_t *mod, struct mod_stack_t *stack, int it_index)
{
	struct prefetcher_t *pref;
	int chain, chain2, stride[PREFETCHER_MAX_STREAM_TRAVERSE][PREFETCHER_LOOKUP_DEPTH_MAX], i, pref_stride[PREFETCHER_MAX_STREAM_TRAVERSE], j;
	int delta_buffer[PREFETCHER_MAX_STREAM_TRAVERSE][PREFETCHER_DELTA_BUFFER_MAX];//cjc_20130304 use to store delta between ghb entries
	int delta_count[PREFETCHER_MAX_STREAM_TRAVERSE];
	unsigned int prev_addr, cur_addr, prefetch_addr = 0,enable_pref = 0;

	int traverse_count = 0;
	int traverse_buffer[PREFETCHER_MAX_STREAM_TRAVERSE];
	unsigned int base_addr[PREFETCHER_MAX_STREAM_TRAVERSE];
	int delta_buffer_ptr[PREFETCHER_MAX_STREAM_TRAVERSE];
	int last_it,cur_it;//cjc_20130308


	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

	last_it = pref->ghb_last_it_index;
	chain = pref->index_table[it_index].ptr;

	/* The lookup depth must be at least 2 - which essentially means
	 * two strides have been seen so far, predict the next stride. */
	assert(pref->lookup_depth >= 2 && pref->lookup_depth <= PREFETCHER_LOOKUP_DEPTH_MAX);

	/* The table should've been updated before calling this function. */
	assert(pref->ghb[chain].addr == stack->addr);

	if(last_it!=-1&&last_it!=it_index)
	{
		if(pref->index_table[last_it].valid_next_it==0)
		{
			pref->index_table[last_it].next_it = it_index;
			pref->index_table[last_it].link_strength =1;
			pref->index_table[last_it].valid_next_it=1;
		}
		else
		{
			if(pref->index_table[last_it].next_it == it_index)
			{
				pref->index_table[last_it].link_strength = (pref->index_table[last_it].link_strength>=7)?7:pref->index_table[last_it].link_strength+1;
			}
			else
			{
				pref->index_table[last_it].link_strength--;
			}

			assert (pref->index_table[last_it].link_strength>=0);
			if(pref->index_table[last_it].link_strength == 0)
			{
				pref->index_table[last_it].next_it = it_index;
				pref->index_table[last_it].link_strength =1;
			}
		}
	}



	/* Collect "lookup_depth" number of strides (deltas).
	 * This doesn't really make sense for a depth > 2, but
	 * I'll just have the code here for generality. */

	cur_it =  it_index ;
	traverse_buffer[0] = it_index;

	for (i = 0; i < PREFETCHER_MAX_STREAM_TRAVERSE-1; i++)
	{
		chain = pref->index_table[cur_it].ptr;

		if(chain==-1)
			continue;

		base_addr[traverse_count] = pref->ghb[chain].addr;		

		for (j = 0; j < pref->lookup_depth; j++)
		{
			prev_addr = pref->ghb[chain].addr;
			chain = pref->ghb[chain].next;

			/* The chain isn't long enough */
			if (chain == -1)
				break;

			cur_addr = pref->ghb[chain].addr;
			stride[i][j] = prev_addr - cur_addr;
		}

		if (pref->index_table[cur_it].valid_next_it && pref->index_table[cur_it].link_strength>=3)
		{
			traverse_count ++;
			cur_it = pref->index_table[cur_it].next_it;
			traverse_buffer[traverse_count] = cur_it;
		}
		else
			break;
	}

	for (j = 0; j <= traverse_count; j++)
	{
		delta_count[j] = 1;
		delta_buffer [j][0] = stride[j][0];
		pref_stride[j] = 0;


		chain = pref->index_table[traverse_buffer[j]].ptr;
		//assert(chain<256);
		if(chain ==-1)
			continue;
		chain = pref->ghb[chain].next;
		//assert(chain<256);
		if(chain ==-1)
			continue;
		/* "chain" now points to the second element of the list.
		 * Try to match the stride array starting from here. */
		while (chain != -1)
		{
			/* This really doesn't look realistic to implement in
			 * hardware. Too much time consuming I feel. */
			chain2 = chain;
			for (i = 0; i < pref->lookup_depth; i++)
			{
				prev_addr = pref->ghb[chain2].addr;
				chain2 = pref->ghb[chain2].next;

				/* The chain isn't long enough and we
				 * haven't found a match till now. */
				if (chain2 == -1)
					break;

				cur_addr = pref->ghb[chain2].addr;

				delta_buffer[j][delta_count[j]+i] = prev_addr - cur_addr;

				if (stride[j][i] != prev_addr - cur_addr)
					break;
			}

			delta_count[j] ++;//cjc_20130304


			/* If we traversed the above loop full, we have a match. */
			if (i == pref->lookup_depth)
			{
				cur_addr = pref->ghb[chain].addr;
				assert(pref->ghb[chain].prev != -1 &&
						pref->ghb[chain].prev_it_ghb == prefetcher_ptr_ghb);
				chain = pref->ghb[chain].prev;
				enable_pref =1;
				break;
			}

			chain = pref->ghb[chain].next;
		}
	}
	if (enable_pref)
	{
		int degree = 0,last_degree,final_degree;
		assert (stack->addr == base_addr[0]);
		for(i = 0;i <=traverse_count; i++)
			delta_buffer_ptr[i] = delta_count[i] -2;
		final_degree = calc_degree(pref->pf_degree,pref->index_table[it_index].degree,0);
		prefetcher_calc_local_degree(mod,pref->index_table[it_index].degree);
		if(pref->index_table[it_index].degree==-2)
                        stack->client_info->lower_level_insertion=1;
		while(degree < final_degree)
		{
			last_degree = degree;
			for(i = 0;i <=traverse_count; i++)
			{
				if(delta_buffer_ptr[i]<0)
					continue;
				pref_stride[i] += delta_buffer[i][delta_buffer_ptr[i]];
				prefetch_addr = base_addr[i] + pref_stride[i];
				if(((pref_stride[i]>0&&prefetch_addr>base_addr[i])||(pref_stride[i]<0&&prefetch_addr<base_addr[i]))&&prefetch_addr>0)
				{
					prefetcher_do_prefetch(mod, stack, prefetch_addr);
				}
				degree ++;
				delta_buffer_ptr[i]--;
				if(delta_buffer_ptr[i]<0)
				{
					delta_buffer_ptr[i] = delta_count[i] -2;
				}
			}
			if(last_degree==degree)
				fatal("Error in Miss Graph");
		}
		pref->index_table[it_index].lateness = 0;


	}

	pref->ghb_last_it_index = it_index;
}
/*struct mod_t *pf_find_destination(struct mod_stack_t *stack, struct mod_t *mod)
  {
  struct prefetcher_t *pref;
  struct mod_t * target;
  struct mod_t * tmp;
  int i,lower_level;


  pref = mod->cache->prefetcher;
  assert(pref);
  lower_level = pref->lower_level_insertion;
  target =mod;
  for(i=0;i<lower_level;i++)
  {
  tmp=mod_get_low_mod(target, stack->tag);
  if(tmp->kind==mod_kind_cache)
  target=tmp;
  else
  break;
  }
  return target;
  }*/

void prefetcher_handler(int event, void *data)
{
	struct prefetcher_t *pref;
	pref = (struct prefetcher_t *)data;

	if(event == EV_PF_TIME_INTERVAL)
	{
		assert(pref->is_counting==1);
		if(prefetcher_calc_spt_timing_counter(pref))
		{
			esim_schedule_event(EV_PF_TIME_INTERVAL, pref,FINE_GRAINED_INTERVAL);	
		}
		else
		{
			pref->is_counting = 0;
		}
	}
	else
		abort();
}

int prefetcher_calc_spt_timing_counter(struct prefetcher_t *pref)
{
	int i,non_zero_count=0;
	if(pref->type==prefetcher_type_stride)
	{
		for(i = 0; i < pref->spt.count; i++)
		{
			pref->spt.timing_counter[i]=(pref->spt.timing_counter[i])?pref->spt.timing_counter[i]-1:0;
			if(pref->spt.timing_counter[i])
			{
				non_zero_count = 1;
			}
			else if(pref->spt.timing_counter[i]==0&&pref->spt.confidence[i]>0)
			{
				if(pref->spt.degree[i]>-2&&pref->spt.degree[i]<=0)
				{
					pref->spt.timing_counter[i] = (FINE_GRAINED_COUNT_MAX<<1);
					pref->spt.degree[i]--;	
				}
			}
		}
	}

	//cjc_20130722
	else if (pref->type==prefetcher_type_ghb_pc_dc||pref->type==prefetcher_type_ghb_pc_dc_mg)
	{
		for(i = 0; i < pref->it_size; i++)
		{
			pref->index_table[i].timing_counter -= (pref->index_table[i].timing_counter>0)?1:0;
			if(pref->index_table[i].timing_counter)
			{
				non_zero_count = 1;
			}
		}
	}
	else if (pref->type==prefetcher_type_stream)
	{
		struct stream_streaming_entry *entry;
		LINKED_LIST_FOR_EACH(&pref->stream_table->LRU_list)
        	{
			entry = linked_list_get(&pref->stream_table->LRU_list);	
			entry->timing_counter -=(entry->timing_counter>0)?1:0;
			if(entry->timing_counter)
			{
				non_zero_count = 1;
			}
			/*else 
			{
				if(entry->degree>-2&&entry->degree<=0)
				{
					entry->degree--;
					entry->timing_counter = (FINE_GRAINED_COUNT_MAX<<1);
				}
			}*/
		}
	}
	return non_zero_count;
}

void prefetcher_access_hit_MSHR(struct mod_stack_t *stack, struct mod_t *target_mod)
{
	/*int pc;

	  assert(stack->client_info);

	  if (!target_mod->cache->prefetcher)
	  return;


	  if(target_mod->cache->prefetcher->type==prefetcher_type_stride)
	  {
	  pc = stack->client_info->prefetcher_eip;
	  prefetcher_set_stride_lateness(target_mod,pc);
	  }*/
	//struct mod_t *dest_mod;
	struct fine_graind_parameter fine;
	int pc;
	int tag,stride;//,i,pf_addr,distance,degree;
	struct prefetcher_t *pref;

	if(target_mod->kind != mod_kind_cache)
		return;

	if (!target_mod->cache->prefetcher)
		return;
	pc = stack->client_info->prefetcher_eip;
	pref = target_mod->cache->prefetcher;
	if(pref->type==prefetcher_type_stride)
	{


		if(esim_warm_up_finish)
		{
			if(pref->fine_grained_ctrl)		
				prefetcher_set_stride_lateness(target_mod,pc);
		}

		tag = (stack->addr )&( ~(target_mod->cache->block_mask));
		stride = prefetcher_find_stride(target_mod,pc,tag,1,&fine,0,stack->core_id);
		/*if(stride==0)
		  return;
		  dest_mod = target_mod;
		//degree = target_mod->cache->prefetcher->pf_degree;
		//degree = (degree+fine.degree>2)?4:degree+fine.degree;
		degree = calc_degree(target_mod->cache->prefetcher->pf_degree,fine.degree,pc);
		distance = target_mod->cache->prefetcher->pf_distance;
		assert(dest_mod);
		for(i = 1; i <= degree;i++)
		{
		pf_addr = (stride*i*distance+stack->addr);
		if(!((tag>=0&&pf_addr>=0)||(tag<0&&pf_addr<0)))
		continue;
		prefetcher_do_prefetch(dest_mod, stack, pf_addr);
		}*/
	}
	else if (pref->type==prefetcher_type_intel_ip)
	{
		prefetcher_access_intel_ip(stack,target_mod, 0);
	}
	else if (pref->type==prefetcher_type_ghb_pc_dc||pref->type==prefetcher_type_ghb_pc_dc_mg)
	{
		int it_index;
		unsigned it_tag;	
		if(esim_warm_up_finish)
		{
			if(pref->fine_grained_ctrl)
			{
				get_it_index_tag(target_mod->cache->prefetcher,stack,&it_index,&it_tag);
				if((unsigned int)pc == it_tag)
				{
					if(pref->index_table[it_index].lateness == 0)
					{
						target_mod->pf_late_stride_found++;
						if(pref->index_table[it_index].degree == -2)
							pref->index_table[it_index].degree =0;
						else
							pref->index_table[it_index].degree +=(pref->index_table[it_index].degree<2)?1:0;
						pref->index_table[it_index].lateness = calc_degree(pref->pf_degree,pref->index_table[it_index].degree,pc)-1;
					}
					pref->index_table[it_index].lateness -=1;			
				}
			}
		}
	}	
	else if (pref->type==prefetcher_type_stream)
	{
		struct stream_streaming_entry *entry;
		if(esim_warm_up_finish)
                {
			if(pref->fine_grained_ctrl)
			{
				assert(stack->stream_entry_id<=pref->stream_table->entry_count);
				entry = &pref->stream_table->stream_entry[stack->stream_entry_id-1];
				if(entry->degree<0)
					entry->degree = 0;
				else if (entry->degree<2)
					entry->degree++;
			}
		}
	}
}

void prefetcher_set_stride_lateness(struct mod_t *mod, int pc)
{
	struct prefetcher_t *pref;
	int i, end;
	pref = mod->cache->prefetcher;
	if(pref->type==prefetcher_type_stride)
	{		
		end = (pref->spt).count;	
		for(i=0;i<end;i++)
		{
			if((pref->spt).pc[i]==pc)
			{	
				if(pref->spt.lateness[i]==0&&pref->spt.confidence[i]>=pref->pf_degree)
				{
					mod->pf_late_stride_found++;
					if(pref->spt.degree[i]==-2)
						pref->spt.degree[i] = 0;
					else if(pref->spt.degree[i]==2)
						pref->spt.degree[i] = 2;
					else
						pref->spt.degree[i] = pref->spt.degree[i] + 1;


					pref->spt.lateness[i]=calc_degree(pref->pf_degree,pref->spt.degree[i],pc)-1;
				}		
				return;
			}
		}
	}
	//if(i==end)
	//	fatal("Evicted stride entry\n");
}

void prefetcher_dump_stride_table(struct mod_t *mod,FILE *f, int which,int dump_useful)
{
	struct prefetcher_t *pref;
	int i,end;
	if(!mod||!mod->cache)
		return;
	pref = mod->cache->prefetcher;

	if(!pref||pref->type!=prefetcher_type_stride)
		return;	
	end = (pref->spt).count;
	if(which==-1)
	{	
		if(dump_useful==0)
		{
			fprintf(f, "\n");
			fprintf(f, "%s_evicted_entry_count = %lld\n",mod->name,pref->evicted_entry_count);
			fprintf(f, "%s_evicted_useful_entry_count = %lld\n",mod->name,pref->evicted_useful_entry_count);
		}
		for(i = 0; i <end; i++)
		{
			if(ref_file&&((long)f==(long)ref_file))
			{
				if((pref->spt).stride_hit_count[i]>=dump_useful&&(pref->spt).stride_hit_count[i])
				{	
					int cycle = ((pref->spt).reused_interval[i]/(pref->spt).stride_hit_count[i]);
					fprintf(f,"%d %d\n",(pref->spt).pc[i] ,(cycle<(FINE_GRAINED_INTERVAL/2))?4:(cycle>=FINE_GRAINED_INTERVAL*FINE_GRAINED_COUNT_MAX)?1:2);
				}
			}
			else
			{
				if((pref->spt).stride_hit_count[i]>=dump_useful)
					fprintf(f, "%s pc %d stride %d degree %d distance %d used_count %lld reused_interval %lld stride_hit_count %lld cross_page_stride_hit %lld\n"
							,mod->name,(pref->spt).pc[i],(pref->spt).stride[i],(pref->spt).degree[i],(pref->spt).distance[i],(pref->spt).used_count[i],
							(pref->spt).reused_interval[i],(pref->spt).stride_hit_count[i],(pref->spt).cross_page_stride_hit[i]);
			}
		}
		if(dump_useful==0)
			fprintf(f, "\n\n");
	}
	else if(which<end)
	{
		i = which;
		if(ref_file&&(((long )f)==((long )ref_file)))
		{
			if((pref->spt).stride_hit_count[i]>=dump_useful&&(pref->spt).stride_hit_count[i])
			{
				int cycle = ((pref->spt).reused_interval[i]/(pref->spt).stride_hit_count[i]);
				fprintf(f,"%d %d\n",(pref->spt).pc[i] ,(cycle<(FINE_GRAINED_INTERVAL/2))?4:(cycle>=FINE_GRAINED_INTERVAL*FINE_GRAINED_COUNT_MAX)?1:2);
			}
		}
		else
		{
			if((pref->spt).stride_hit_count[i]>=dump_useful)
				fprintf(f, "%s pc %d stride %d degree %d distance %d used_count %lld reused_interval %lld stride_hit_count %lld cross_page_stride_hit %lld\n"
						,mod->name,(pref->spt).pc[i],(pref->spt).stride[i],(pref->spt).degree[i],(pref->spt).distance[i],(pref->spt).used_count[i],
						(pref->spt).reused_interval[i],(pref->spt).stride_hit_count[i],(pref->spt).cross_page_stride_hit[i]);
		}
	}
	else
		fatal("%s Unreachable index\n",mod->name);
}

int calc_degree(int global,int local, int pc)
{
	if(*prefetch_stride_table_ref_input)
		return prefetcher_ref_table_get(pc,global);


	if(local>2||local<-2)
		fatal("Out-range pf degree");
	if(local>=0)
	{
		return ((global<<local)>=4)?4:(global<<local);
	}
	else
	{
		return ((global>>(-local))==0)?1:(global>>(-local));
	}

}

void prefetcher_ref_table_insert(int pc, int degree)
{
	int i;	
	if(degree<0||degree>4)
		return;
	for (i = 0; i < ref_table.count; i++)
	{
		if(ref_table.valid[i]&&ref_table.pc[i]==pc)
		{
			if(degree < ref_table.degree[i])
				ref_table.degree[i] = degree;
			return;
		}	
	}

	if( i== REF_TABLE_SIZE)
		fatal("Out of range\n");
	ref_table.valid[i] = 1;
	ref_table.pc[i] = pc;
	ref_table.degree[i] = degree;
	ref_table.count ++;
}

int prefetcher_ref_table_get(int pc, int ori_degree)
{
	int i;

	for(i = 0; i<ref_table.count;i++)
	{
		if(ref_table.valid[i]&&ref_table.pc[i]==pc)
		{
			return ref_table.degree[i];
		}
	}

	return ori_degree;
}

void prefetcher_construct_link(struct prefetcher_t *pref,int cur,int core_id)
{
	int prev , prev_nxt;
	prev = pref->spt.last_entry_used[core_id];
	if(pref->type!=prefetcher_type_stride||prev == cur)
		return;		
	if(prev!=-1&&prev<pref->spt.count)
	{
		prev_nxt = pref->spt.next_entry[prev];

		if(prev_nxt == -1)
		{
			pref->spt.next_entry[prev] = cur;
			pref->spt.link_strength[prev] = 1;
			pref->spt.prev_entry[cur] = prev;
		}
		else
		{
			if(prev_nxt==cur)
			{
				pref->spt.link_strength[prev] = (pref->spt.link_strength[prev]>MAX_LINK_STRENGTH)?MAX_LINK_STRENGTH:pref->spt.link_strength[prev]+1;
				pref->spt.prev_entry[cur] = prev;
			}
			else if (pref->spt.link_strength[prev])
			{
				pref->spt.link_strength[prev]--;
			}
			if(pref->spt.link_strength[prev]==0)
			{
				pref->spt.prev_entry[cur] = prev;
				pref->spt.next_entry[prev] = cur;
				pref->spt.link_strength[prev] = 1;
			}
		}
	}

	pref->spt.last_entry_used[core_id] = cur;
}

int prefetcher_get_nxt_entry(struct prefetcher_t *pref,struct nxt_link_parameter *nxt, int index)
{
	int next_index;
	if(pref->type!=prefetcher_type_stride||pref->spt.count<index)
		return -1;

	nxt->addr = pref->spt.addr[index];
	nxt->stride = pref->spt.stride[index];
	nxt->core_id = pref->spt.core_id[index];
	next_index = pref->spt.next_entry[index];
	if(pref->spt.next_entry[index]!=-1&&nxt->core_id!=pref->spt.core_id[next_index])
		return -1;
	else if(pref->spt.link_strength[index]<4||pref->spt.confidence[next_index]<4)
		return -1;
	else if (pref->spt.prev_entry[next_index]!=index)
		return -1;
	else
		return next_index;
}

void prefetcher_streamer(struct mod_t *mod, struct mod_stack_t *stack)
{
	struct prefetcher_t *pref;
	//        unsigned int prefetch_addr = 0;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	pref = mod->cache->prefetcher;

}
void prefetcher_pollution_bloom_filter_set(struct cache_t *cache,unsigned int tag,int is_set)
{
	int index;
	int mask;
	if(!cache)
		return;
	assert(FDP_FILTER_POW_SIZE<16);
	mask = (1<<FDP_FILTER_POW_SIZE) - 1;

	index = (tag&mask)^((tag>>FDP_FILTER_POW_SIZE)&mask);
	assert(index<=mask&&index>=0);

	cache->FDP_filter.pollution_filter[index] = (is_set)?'1':'0';
}
void prefetcher_pollution_bloom_filter_reset(struct cache_t *cache)
{
	int i;
	for(i = 0; i<(1<<FDP_FILTER_POW_SIZE);i++)
		cache->FDP_filter.pollution_filter[i] = '0';
}
void prefetcher_pollution_bloom_filter_calc(struct mod_stack_t *stack,int used_mod)
{
	struct cache_t *cache;
	int index;
	int mask;
	char value;
	struct mod_t *mod;	
	if(used_mod==0)
		mod=stack->mod;
	else if(used_mod==1)
		mod=stack->target_mod;
	else
		fatal("Unknowed module\n");

	cache = mod->cache;
	if(!cache)
		return;
	assert(FDP_FILTER_POW_SIZE<16);
	mask = (1<<FDP_FILTER_POW_SIZE) - 1;

	index = (stack->tag&mask)^((stack->tag>>FDP_FILTER_POW_SIZE)&mask);
	assert(index<=mask&&index>=0);
	value = cache->FDP_filter.pollution_filter[index];
	if(value=='0'||value=='1')
		mod->pf_pollution_FDP +=(value=='1')?1:0;
	else
		fatal("Undefined value for FDP\n");
	cache->FDP_filter.pollution_filter[index]='0';
}

void stream_LRU_replace(struct stream_table *table, void *data , enum stream_replace_type_t type)
{
	int index,lru_count,free_count;

	lru_count = linked_list_count(&table->LRU_list);
	free_count =linked_list_count(&table->free_list);
	
	assert(lru_count+free_count==table->entry_count);



	if(type == stream_type_to_MRU)
	{
		if(table->type == stream_type_train)
		{
			index = ((struct stream_training_entry *)linked_list_get(&table->LRU_list))->id;
			assert(*(int *)data == index);
			linked_list_remove(&table->LRU_list);
			linked_list_add(&table->LRU_list,&(table->train_entry[index]));
		}
		else if(table->type == stream_type_stream)
		{
			index = ((struct stream_streaming_entry *)linked_list_get(&table->LRU_list))->id;
			assert(*(int *)data == index);
			linked_list_remove(&table->LRU_list);
			if(table->stream_entry[index].timing_counter==0)
				table->stream_entry[index].degree -= (table->stream_entry[index].degree<=0&&table->stream_entry[index].degree>-2)?1:0;
			table->stream_entry[index].timing_counter = (table->stream_entry[index].degree<0)?(FINE_GRAINED_COUNT_MAX<<1):FINE_GRAINED_COUNT_MAX; 
			linked_list_add(&table->LRU_list,&(table->stream_entry[index]));

		}
		else if (table->type ==stream_type_dcu)
		{
			index = ((struct dcu_entry *)linked_list_get(&table->LRU_list))->id;
                        assert(*(int *)data == index);
                        linked_list_remove(&table->LRU_list);
			linked_list_add(&table->LRU_list,&(table->dcu_entry[index]));
		}
		return;
	}
	else if(type == stream_type_to_free)
	{
		if(table->type == stream_type_train)
		{
			index = ((struct stream_training_entry *)linked_list_get(&table->LRU_list))->id;
			assert(*(int *)data == index);
			linked_list_remove(&table->LRU_list);	
			stream_clear_entry(stream_type_train,&(table->train_entry[index]),NULL);	
			linked_list_add(&table->free_list,&(table->train_entry[index]));
		}
		else if(table->type == stream_type_stream)
		{
			fatal("unimplement stream_type_to_free streaming");
		}
		else if (table->type ==stream_type_dcu)
		{
			index = ((struct dcu_entry *)linked_list_get(&table->LRU_list))->id;
			assert(*(int *)data == index);
                        linked_list_remove(&table->LRU_list);
                        stream_clear_entry(stream_type_dcu,&(table->dcu_entry[index]),NULL);
			linked_list_add(&table->free_list,&(table->dcu_entry[index]));
		}
	}
	else if(  lru_count < table->entry_count && type == stream_type_to_replace)
	{
		linked_list_head(&table->free_list);
		if(table->type == stream_type_train)
		{
			index = ((struct stream_training_entry *)linked_list_get(&table->free_list))->id;
			linked_list_remove(&table->free_list);
			stream_clear_entry(stream_type_train,&(table->train_entry[index]),NULL);
			table->train_entry[index].start_addr = ((struct stream_training_entry *)data)->start_addr;		
			linked_list_add(&table->LRU_list,&(table->train_entry[index]));
		}
		else if(table->type == stream_type_stream)
		{
			index = ((struct stream_streaming_entry *)linked_list_get(&table->free_list))->id;
			linked_list_remove(&table->free_list);
			stream_clear_entry(stream_type_stream,&(table->stream_entry[index]),data);
			linked_list_add(&table->LRU_list,&(table->stream_entry[index]));
		}
		else if (table->type ==stream_type_dcu)
                {
                        index = ((struct dcu_entry *)linked_list_get(&table->free_list))->id;
                        linked_list_remove(&table->free_list);
                        stream_clear_entry(stream_type_dcu,&(table->dcu_entry[index]),data);
                        linked_list_add(&table->LRU_list,&(table->dcu_entry[index]));
                }
		
	}
	else if (lru_count == table->entry_count && type == stream_type_to_replace)
	{
		linked_list_head(&table->LRU_list);
		if(table->type == stream_type_train)
		{
			index = ((struct stream_training_entry *)linked_list_get(&table->LRU_list))->id;
			linked_list_remove(&table->LRU_list);
			stream_clear_entry(stream_type_train,&(table->train_entry[index]),NULL);
			table->train_entry[index].start_addr = ((struct stream_training_entry *)data)->start_addr;
			linked_list_add(&table->LRU_list,&(table->train_entry[index]));
		}
		else if(table->type == stream_type_stream)
		{
			index = ((struct stream_streaming_entry *)linked_list_get(&table->LRU_list))->id;
			linked_list_remove(&table->LRU_list);
			stream_clear_entry(stream_type_stream,&(table->stream_entry[index]),data);
			linked_list_add(&table->LRU_list,&(table->stream_entry[index]));
		}
		else if (table->type ==stream_type_dcu)
		{
			index = ((struct dcu_entry *)linked_list_get(&table->LRU_list))->id;
                        linked_list_remove(&table->LRU_list);
                        stream_clear_entry(stream_type_dcu,&(table->dcu_entry[index]),data);
                        linked_list_add(&table->LRU_list,&(table->dcu_entry[index]));
		}
	}
	else
	{
		fatal("Stream Table out of range\n");
	}
}

void stream_access_train_entry(struct mod_stack_t *stack,struct mod_t * mod)
{
	unsigned int address,start;
	struct prefetcher_t *pref;
	struct stream_table *table;	
	struct stream_training_entry *train_entry;
	struct stream_training_entry data;
	int direction;
	int count = 0;

	pref = mod->cache->prefetcher;
	address = stack->tag>>pref->log_block_size;
	table = pref->train_table;

	if(!pref||pref->type!=prefetcher_type_stream)
		return ;
	assert(table->type==stream_type_train);

	LINKED_LIST_FOR_EACH(&table->LRU_list)
	{
		train_entry = linked_list_get(&table->LRU_list);
		start = train_entry->start_addr;
		if(!(STREAM_IS_NEIGHBOR(start,address))||address==start)
			continue;
		if(count)
			fatal("Second Time");
		//stream_LRU_replace(table,&train_entry->id,stream_type_to_MRU);
		count = 1;
		if(train_entry->second_addr_dist)
		{
			train_entry->end_addr_dist = (address-start);
			if(train_entry->end_addr_dist == train_entry->second_addr_dist)
				return ;
			direction = (train_entry->end_addr_dist-train_entry->second_addr_dist)*train_entry->ascending;
			if(direction>0)//same direction
			{
				train_entry->trained = 1;
				stream_move_to_stream_table(pref,train_entry);
				stream_LRU_replace(table,&train_entry->id,stream_type_to_free);

				mod->stream_trained_stream ++;
				return ;
			}
			else if (direction<0)
			{	

				if(STREAM_IS_NEIGHBOR((start+train_entry->second_addr_dist),address))
				{
					train_entry->start_addr = start+train_entry->second_addr_dist;
					train_entry->second_addr_dist = train_entry->end_addr_dist - train_entry->second_addr_dist;
					train_entry->end_addr_dist = 0;
					train_entry->ascending = (train_entry->second_addr_dist==0)?0:(train_entry->second_addr_dist>0)?1:-1;
					mod->stream_renew_stream++;
				}
				else
				{	
					stream_clear_entry(stream_type_train,train_entry,NULL);
					train_entry->start_addr = address;
					mod->stream_cleared_stream ++;
				}
				stream_LRU_replace(table,&train_entry->id,stream_type_to_MRU);		
				return ;
			}
			else
				fatal("Stream Unknown Direction");


		}
		else
		{
			if(address>start)
			{
				train_entry->ascending = 1;
				//train_entry->second_addr_dist = (address-start)>>pref->log_block_size;
			}
			else
			{
				train_entry->ascending = -1;
				//train_entry->second_addr_dist = (start-address)>>pref->log_block_size;

			}
			train_entry->second_addr_dist = (address-start);
			assert((start+train_entry->second_addr_dist)==address);
			stream_LRU_replace(table,&train_entry->id,stream_type_to_MRU);
		}	
		return ;
	}		

	stream_clear_entry(stream_type_train,&data,NULL);
	data.start_addr = address;

	stream_LRU_replace(table,&data,stream_type_to_replace);
	mod->stream_table_replace ++;
	return ;
}

void stream_clear_entry(enum stream_table_type_t type,void *data, void *assignment)
{
	if(type == stream_type_train)
	{	
		if(assignment)
		{

		}
		else
		{
			((struct stream_training_entry*)data)->start_addr = 0;
			((struct stream_training_entry*)data)->	second_addr_dist = 0;
			((struct stream_training_entry*)data)->end_addr_dist = 0;
			((struct stream_training_entry*)data)->ascending = 0;
			((struct stream_training_entry*)data)->trained = 0;
		}
	}
	else if(type == stream_type_stream)
	{
		if(assignment)
		{
			((struct stream_streaming_entry*)data)->original_addr= ((struct stream_streaming_entry*)assignment)->original_addr;
			((struct stream_streaming_entry*)data)->region_start_baddr_dist=((struct stream_streaming_entry*)assignment)->region_start_baddr_dist;
			((struct stream_streaming_entry*)data)->region_end_addr=((struct stream_streaming_entry*)assignment)->region_end_addr;
			((struct stream_streaming_entry*)data)->last_access_addr_dist=((struct stream_streaming_entry*)assignment)->last_access_addr_dist;
			((struct stream_streaming_entry*)data)->ascending=((struct stream_streaming_entry*)assignment)->ascending;
			((struct stream_streaming_entry*)data)->stride=((struct stream_streaming_entry*)assignment)->stride;
			((struct stream_streaming_entry*)data)->csd1=((struct stream_streaming_entry*)assignment)->csd1;
			((struct stream_streaming_entry*)data)->csd2=((struct stream_streaming_entry*)assignment)->csd2;
			((struct stream_streaming_entry*)data)->isrepeat=((struct stream_streaming_entry*)assignment)->isrepeat;
			((struct stream_streaming_entry*)data)->degree = 0;
			((struct stream_streaming_entry*)data)->distance = 0;
			((struct stream_streaming_entry*)data)->lateness = 0;
			((struct stream_streaming_entry*)data)->timing_counter = FINE_GRAINED_COUNT_MAX;
		}
		else
		{
			((struct stream_streaming_entry*)data)->original_addr= 0;
			((struct stream_streaming_entry*)data)->region_start_baddr_dist= 0;
			((struct stream_streaming_entry*)data)->region_end_addr= 0;
			((struct stream_streaming_entry*)data)->last_access_addr_dist= 0;
			((struct stream_streaming_entry*)data)->stride= 0;
			((struct stream_streaming_entry*)data)->ascending= 0;
			((struct stream_streaming_entry*)data)->stride_confirm= 0;
			((struct stream_streaming_entry*)data)->csd1= 0;
			((struct stream_streaming_entry*)data)->csd2= 0;
			((struct stream_streaming_entry*)data)->isrepeat= 0;
			((struct stream_streaming_entry*)data)->degree = 0;
			((struct stream_streaming_entry*)data)->distance = 0;
			((struct stream_streaming_entry*)data)->lateness = 0;
			((struct stream_streaming_entry*)data)->timing_counter = FINE_GRAINED_COUNT_MAX;
		}
	}	
	else if (type == stream_type_dcu)
	{
		if(assignment)
		{
			((struct dcu_entry*)data)->address = ((struct dcu_entry*)assignment)->address;
                        ((struct dcu_entry*)data)->access_count = 0;
                        ((struct dcu_entry*)data)->had_prefetched = 0;
		}
		else
		{
			((struct dcu_entry*)data)->address = 0;
			((struct dcu_entry*)data)->access_count = 0;
			((struct dcu_entry*)data)->had_prefetched = 0;
		}
	}
}
void stream_move_to_stream_table(struct prefetcher_t *pref,struct stream_training_entry * moved_entry)
{
	struct stream_streaming_entry trained_entry;
	assert(pref);
	stream_clear_entry(stream_type_stream,&trained_entry,NULL);

	trained_entry.original_addr = moved_entry->start_addr;

	trained_entry.ascending = moved_entry->ascending;
	assert(trained_entry.ascending);
	trained_entry.region_end_addr = (moved_entry->start_addr+moved_entry->end_addr_dist+(moved_entry->end_addr_dist-moved_entry->second_addr_dist))<<(pref->log_block_size);

	STREAM_SET_REGION_START(&trained_entry,trained_entry.original_addr,pref->log_block_size);
	STREAM_SET_LAST_ACCESS(&trained_entry,trained_entry.region_end_addr);

	stream_LRU_replace(pref->stream_table,&trained_entry,stream_type_to_replace);
	if(esim_warm_up_finish&&pref->is_counting==0)
        {
                pref->is_counting=1;
                if(pref->fine_grained_ctrl)
                        esim_schedule_event(EV_PF_TIME_INTERVAL, pref,FINE_GRAINED_INTERVAL);
        }
}

struct stream_streaming_entry * stream_access_stream_table(struct mod_stack_t *stack,struct mod_t * mod, int *lateness)
{
	unsigned int address,region_start,region_end,late_region;
	struct prefetcher_t *pref;
	struct stream_table *table;
	struct stream_streaming_entry *entry;
	int log_size;

	pref = mod->cache->prefetcher;
	log_size = pref->log_block_size;
	address = stack->tag>>log_size;
	table = pref->stream_table;

	if(!pref||pref->type!=prefetcher_type_stream)
		return NULL;
	assert(table->type==stream_type_stream);	

	assert(pref->pf_degree>=0);

	*lateness = 0;
	LINKED_LIST_FOR_EACH(&table->LRU_list)
	{
		entry = linked_list_get(&table->LRU_list);
		region_start = (entry->region_end_addr>>log_size)-entry->region_start_baddr_dist;
		region_end = entry->region_end_addr>>log_size;
		if(entry->ascending>0)
		{
			late_region = (region_start-(unsigned int)pref->pf_degree);
			if(address>=region_start&&address<=region_end)
			{
				stream_LRU_replace(table,&entry->id,stream_type_to_MRU);
				return entry;
			}
			/*else if(address<region_start&&address>late_region)
			{
				if(entry->timing_counter==0)
                                	entry->degree -= (entry->degree<=0&&entry->degree>-2)?1:0;
                        	entry->timing_counter = (entry->degree<0)?(FINE_GRAINED_COUNT_MAX<<1):FINE_GRAINED_COUNT_MAX;
				*lateness = 1;
				return entry;
			}*/
		}
		else if (entry->ascending<0)
		{
			late_region = (region_start+(unsigned int)pref->pf_degree);
			if(address<=region_start&&address>=region_end)
			{
				stream_LRU_replace(table,&entry->id,stream_type_to_MRU);
				return entry;
			}
			/*else if(address>region_start&&address<late_region)
			{
				if(entry->timing_counter==0)
                                        entry->degree -= (entry->degree<=0&&entry->degree>-2)?1:0;
                                entry->timing_counter = (entry->degree<0)?(FINE_GRAINED_COUNT_MAX<<1):FINE_GRAINED_COUNT_MAX;
				*lateness = 1;
				return entry;
			}*/
		}
		else
		{
			fatal("Unknow direction of stream table");
		}
	}
	return NULL;
}

void stream_issue_prefetching(struct stream_streaming_entry *entry,struct mod_stack_t *stack,struct mod_t * mod)
{
	unsigned int pf_addr,region_start;
	struct prefetcher_t *pref;
	int degree,distance,i;
	unsigned int block_size;
	unsigned int last_pf_addr,maxaddr,minaddr,num_stride,last_access;

	block_size = mod->cache->block_size;
	pref = mod->cache->prefetcher;
	//degree = pref->pf_degree;
	degree = calc_degree(pref->pf_degree,entry->degree,0);
	distance = pref->pf_distance;

	//prefetcher_calc_local_degree(mod,entry->degree);	

	if(esim_warm_up_finish&&pref->is_counting==0)
        {
                pref->is_counting=1;
                if(pref->fine_grained_ctrl)
                        esim_schedule_event(EV_PF_TIME_INTERVAL, pref,FINE_GRAINED_INTERVAL);
        }
	if(entry->ascending>0)
	{
		last_access = entry->region_end_addr-entry->last_access_addr_dist;
		maxaddr = (entry->region_end_addr)|(block_size-1);
		num_stride = (maxaddr -last_access)/block_size;
		last_pf_addr = last_access + num_stride*block_size;
		if((last_pf_addr + degree*block_size) > entry->region_end_addr)
			entry->region_end_addr = last_pf_addr + degree*block_size;
	}
	else if (entry->ascending<0)
	{
		last_access = entry->region_end_addr+entry->last_access_addr_dist;
		minaddr = ((entry->region_end_addr>>pref->log_block_size)<<pref->log_block_size);
		num_stride = (last_access-minaddr)/block_size;

		last_pf_addr = last_access - num_stride*block_size;	

		if(entry->region_end_addr > (last_pf_addr -degree*block_size))
			entry->region_end_addr = last_pf_addr -degree*block_size;
	}
	else
	{
		fatal("unkwnon direction");
	}

	STREAM_SET_LAST_ACCESS(entry,last_access);	
	region_start = (entry->region_end_addr>>pref->log_block_size)-entry->region_start_baddr_dist;

	if(entry->degree==-2)
		stack->client_info->lower_level_insertion=1;
	if(mod->level==1)
		stack->client_info->stream_entry_id = entry->id+1;
	for( i = 1; i<=degree; i++ )
	{
		pf_addr = last_pf_addr + i*block_size*entry->ascending;
		prefetcher_do_prefetch(mod, stack, pf_addr);	
		prefetcher_calc_local_degree(mod,entry->degree);
	}



	if(entry->ascending>0)
	{
		num_stride = (entry->region_end_addr - (region_start<<pref->log_block_size))/block_size;
		if(num_stride>distance)
			region_start = (entry->region_end_addr - distance*block_size)>>pref->log_block_size;
		entry->region_start_baddr_dist = (entry->region_end_addr >>pref->log_block_size)-region_start;
	}
	else
	{
		num_stride = ((region_start<<pref->log_block_size) - entry->region_end_addr)/block_size;
		if(num_stride>distance)
			region_start = (entry->region_end_addr + distance*block_size)>>pref->log_block_size;
		entry->region_start_baddr_dist = region_start - (entry->region_end_addr >>pref->log_block_size);
	}	
}
void prefetcher_calc_local_degree(struct mod_t * mod, int degree)
{
	assert(degree>=-2&&degree<=2);
	if(esim_warm_up_finish)	
		mod->local_degree[degree+2]++;
}

void prefetcher_access_dcu(struct mod_stack_t *stack,struct mod_t * mod)
{
	struct prefetcher_t *pref;
	struct stream_table *table;
	struct dcu_entry *entry,data;
	int i = 0; //loop constant

	//IC_S_2014_04_08
	int tmp_i;
	struct pf_hint_t *victim_tmp;
	int get_lowest_priority_index;
	int get_second_priority_index;
	//IC_E
	
	unsigned int address;
	unsigned int dcu_prefetch_address; 
	unsigned int *dcu_prefetch_address_ptr; //IC_S_2014_04_08

	//IC_S_2014_04_08
	struct pf_hint_t *pf_hint_pri;
	//IC_E

	//IC_S_2013_12_03
	int set, way, state, hit, tag;
	set = way = state = 0;
	struct mod_t * target_mod;
	unsigned int pre_dcu_prefetch_address_0;
	unsigned int pre_dcu_prefetch_address_1;
	unsigned int pre_dcu_prefetch_address_2;
	unsigned int pre_dcu_prefetch_address_3;
	static int distance0 = 0;
	static int distance1 = 0;
	static int distance2 = 0;
	static int distance3 = 0;
	int direction;
	//IC_E
	
	//IC_S_2013_11_05
	//unsigned int prefetch_address;
	//int set = 0;
	//int way = 0;
	//int state = 0;
	//int hit;
	//int tag;
	//IC_E
	
	//if(stack->read == 0)
	//	return;
	pref = mod->cache->prefetcher;
	table = pref->dcu_table;
	address = stack->tag;
	dcu_prefetch_address = address;
	dcu_prefetch_address_ptr = &(stack->tag);

    	//IC_S_2014_04_08
	pf_hint_pri = stack->pf_hint_priority;
	//pf_hint_pri->hint_address = stack->tag;
	//pf_hint_pri->priority = stack->state; //default

	stat_tick(stack->state==cache_block_invalid, access_dcu_state_I);
	stat_tick(stack->state==cache_block_noncoherent, access_dcu_state_NON);
	stat_tick(stack->state==cache_block_modified, access_dcu_state_M);
	stat_tick(stack->state==cache_block_owned, access_dcu_state_O);
	stat_tick(stack->state==cache_block_exclusive, access_dcu_state_E);
	stat_tick(stack->state==cache_block_shared, access_dcu_state_S);
	//IC_E
	
	if(pref->type !=prefetcher_type_dcu || stack->read == 0)
		return;
	
	LINKED_LIST_FOR_EACH(&table->LRU_list)
    {
        entry = linked_list_get(&table->LRU_list);

		//IC_S_2014_03_25
		if (mod->cache->prefetcher->L2_DCU_ActbyHint == 1)
		{
			address = entry->address;
		}
		//IC_E

		if( (entry->address>> (pref->log_block_size)) == (address>>(pref->log_block_size )) )
		{
			entry->history[0] = entry->address;	
			entry->access_count+=(entry->access_count>=2)?0:1;// fisrt access will not be counted, actual access count equals to access count + 1
			entry->history[entry->access_count]=stack->addr;	

			//IC_S_2014_03_25
			if (mod->cache->prefetcher->L2_DCU_ActbyHint == 1)
			{
				entry->access_count = 2;
			}
			//IC_E
			
			if(entry->access_count>=2)
			{
				if(entry->had_prefetched==0)
				{
					entry->had_prefetched = 1;
					
					//IC_S_2014_03_25
					if (mod->cache->prefetcher->L2_DCU_ActbyHint == 1)
					{
						entry->had_prefetched = 0;
					}
					//IC_E
					if( (entry->history[1] - entry->history[0]) < 0 &&  (  entry->history[2] - entry->history[     0]) < 0 ) direction = -1;
					else direction = 1;
//					printf("%d addr: %x %x %x : %d\n",stack->core_id,entry->history[0],entry->history[1],entry->history[2],direction);
                   			
					for( i = 0 ; i < pref->pf_degree ; i++ )
					{
						//dcu_prefetch_address = dcu_prefetch_address + ((pref->pf_distance)<<pref->log_block_size);
						//IC_S_2013_11_15
						if (mod->cache->prefetcher->L1_DCU_hint == 1)
						{
							//dcu_prefetch_address = dcu_prefetch_address + ((pref->L1_DCU_hint_prefetch_distance)<<pref->log_block_size); 

							//IC_S_2013_12_03
							target_mod = mod_get_low_mod(mod, stack->tag);
							//printf("target_mod = %d\n", target_mod->mod_id);
							//hit = cache_find_block(target_mod->L1hint, dcu_prefetch_address, &set, &way, &state);
							if (mod->cache->prefetcher->L1_DCU_hint_coreid == 1)
							{
								switch (stack->core_id)
								{
									case 0:
										//if (list_count(target_mod->L1hint_list0) == (target_mod->L1hint->block_size/4))
										//{
										//	//list_dequeue(target_mod->L1hint_list0);

										//	//IC_S_2014_04_08
										//	get_lowest_priority_index = -1;
										//	get_second_priority_index = -1;

										//	for (tmp_i=0; tmp_i<list_count(target_mod->L1hint_list0); tmp_i++)
										//	{
										//		victim_tmp = list_get(target_mod->L1hint_list0, tmp_i);

										//		if (victim_tmp->priority == 2)
										//		{
										//			get_lowest_priority_index = tmp_i;
										//			break;
										//		}
										//		if (victim_tmp->priority == 1 && get_second_priority_index < 0)
										//		{
										//			get_second_priority_index = tmp_i;
										//		}

										//	}
										//	if (get_lowest_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_Iother);
										//		list_remove_at(target_mod->L1hint_list0, get_lowest_priority_index);
										//	}
										//	else if (get_second_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_ME);
										//		list_remove_at(target_mod->L1hint_list0, get_second_priority_index);
										//	}
										//	else
										//	{
										//		stat_tick(1, hint_discard_state_OS);
										//		list_dequeue(target_mod->L1hint_list0);
										//	}

										//	stat_tick(1, hint_discard_state_Total);

										//	list_sort(target_mod->L1hint_list0, pf_hint_priority_compare);
										//	//IC_E

										//}

										//pre_dcu_prefetch_address_0 = (unsigned int)list_top(target_mod->L1hint_list0);
										
										//list_enqueue(target_mod->L1hint_list0, dcu_prefetch_address_ptr);
										//IC_S_2014_04_08
										pf_hint_pri->hint_address = stack->tag;

										if (stack->state == cache_block_owned || stack->state == cache_block_shared)
										{
											pf_hint_pri->priority = 0; //Highest priority
										}
										else if (stack->state == cache_block_modified || stack->state == cache_block_exclusive)
										{
											pf_hint_pri->priority = 1; //Second priority
										}
										else
										{
											pf_hint_pri->priority = 2; //Lowest priority
										}

										target_mod->hint_miss_trigger0 = 1;

										if (list_index_of(target_mod->L1hint_list0, pf_hint_pri) == -1)
										{

											stat_tick(stack->state==cache_block_invalid, hint_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_dcu_state_S);
											stat_tick(1, hint_dcu_state_Total);

											list_enqueue(target_mod->L1hint_list0, pf_hint_pri);

											prefetcher_access_dcu(stack, target_mod);
										}

										else
										{
											stat_tick(stack->state==cache_block_invalid, hint_already_in_buffer_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_already_in_buffer_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_already_in_buffer_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_already_in_buffer_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_already_in_buffer_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_already_in_buffer_dcu_state_S);
											stat_tick(1, hint_already_in_buffer_dcu_state_Total);

										}

										
										////IC_E

										//if (list_count(target_mod->L1hint_list0) > 1)
										//{
										//	;
										//}
										//if (list_count(target_mod->L1hint_list0) == (target_mod->L1hint->block_size/4))
										//{
										//	target_mod->hint_miss_trigger0 = 1;
										//	target_mod->hint_full++;
										//	//IC_S_2014_04_08
										//	list_sort(target_mod->L1hint_list0, pf_hint_priority_compare);
										//	//IC_E
										//}
										//else if (list_count(target_mod->L1hint_list0) < (target_mod->L1hint->block_size/4))
										//{
										//	//target_mod->hint_miss_trigger0 = 0;
										//}
										
										//IC_S_2014_04_22
										//prefetcher_access_dcu(stack, target_mod);
										//IC_E
										
										break;
									case 1:
										//if (list_count(target_mod->L1hint_list1) == (target_mod->L1hint->block_size/4))
										//{
										//	//list_dequeue(target_mod->L1hint_list1);

										//	//IC_S_2014_04_08
										//	get_lowest_priority_index = -1;
										//	get_second_priority_index = -1;

										//	for (tmp_i=0; tmp_i<list_count(target_mod->L1hint_list1); tmp_i++)
										//	{
										//		victim_tmp = list_get(target_mod->L1hint_list1, tmp_i);

										//		if (victim_tmp->priority == 2)
										//		{
										//			get_lowest_priority_index = tmp_i;
										//			break;
										//		}
										//		if (victim_tmp->priority == 1 && get_second_priority_index < 0)
										//		{
										//			get_second_priority_index = tmp_i;
										//		}

										//	}
										//	if (get_lowest_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_Iother);
										//		list_remove_at(target_mod->L1hint_list1, get_lowest_priority_index);
										//	}
										//	else if (get_second_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_ME);
										//		list_remove_at(target_mod->L1hint_list1, get_second_priority_index);
										//	}
										//	else
										//	{
										//		stat_tick(1, hint_discard_state_OS);
										//		list_dequeue(target_mod->L1hint_list1);
										//	}

										//	stat_tick(1, hint_discard_state_Total);

										//	list_sort(target_mod->L1hint_list1, pf_hint_priority_compare);
										//	//IC_E
										//}

										//pre_dcu_prefetch_address_1 = (unsigned int)list_top(target_mod->L1hint_list1);

										//list_enqueue(target_mod->L1hint_list1, dcu_prefetch_address_ptr);
										//IC_S_2014_04_08
										pf_hint_pri->hint_address = stack->tag;

										if (stack->state == cache_block_owned || stack->state == cache_block_shared)
										{
											pf_hint_pri->priority = 0; //Highest priority
										}
										else if (stack->state == cache_block_modified || stack->state == cache_block_exclusive)
										{
											pf_hint_pri->priority = 1; //Second priority
										}
										else
										{
											pf_hint_pri->priority = 2; //Lowest priority
										}

										target_mod->hint_miss_trigger1 = 1;

										if (list_index_of(target_mod->L1hint_list1, pf_hint_pri) == -1)
										{

											stat_tick(stack->state==cache_block_invalid, hint_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_dcu_state_S);
											stat_tick(1, hint_dcu_state_Total);

											list_enqueue(target_mod->L1hint_list1, pf_hint_pri);
											prefetcher_access_dcu(stack, target_mod);
										}
										else
										{
											stat_tick(stack->state==cache_block_invalid, hint_already_in_buffer_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_already_in_buffer_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_already_in_buffer_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_already_in_buffer_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_already_in_buffer_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_already_in_buffer_dcu_state_S);
											stat_tick(1, hint_already_in_buffer_dcu_state_Total);
										}
										//IC_E

										//if (list_count(target_mod->L1hint_list1) > 1)
										//{
										//	;
										//}
										//if (list_count(target_mod->L1hint_list1) == (target_mod->L1hint->block_size/4))
										//{
										//	target_mod->hint_miss_trigger1 = 1;
										//	target_mod->hint_full++;
										//	//IC_S_2014_04_08
										//	list_sort(target_mod->L1hint_list1, pf_hint_priority_compare);
										//	//IC_E

										//}
										//else if (list_count(target_mod->L1hint_list1) < (target_mod->L1hint->block_size/4))
										//{
										//	//target_mod->hint_miss_trigger1 = 0;
										//}

										//IC_S_2014_04_22
										//prefetcher_access_dcu(stack, target_mod);
										//IC_E

										break;
									case 2: 
										//if (list_count(target_mod->L1hint_list2) == (target_mod->L1hint->block_size/4))
										//{
										//	//list_dequeue(target_mod->L1hint_list2);

										//	//IC_S_2014_04_08
										//	get_lowest_priority_index = -1;
										//	get_second_priority_index = -1;

										//	for (tmp_i=0; tmp_i<list_count(target_mod->L1hint_list2); tmp_i++)
										//	{
										//		victim_tmp = list_get(target_mod->L1hint_list2, tmp_i);

										//		if (victim_tmp->priority == 2)
										//		{
										//			get_lowest_priority_index = tmp_i;
										//			break;
										//		}
										//		if (victim_tmp->priority == 1 && get_second_priority_index < 0)
										//		{
										//			get_second_priority_index = tmp_i;
										//		}

										//	}
										//	if (get_lowest_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_Iother);
										//		list_remove_at(target_mod->L1hint_list2, get_lowest_priority_index);
										//	}
										//	else if (get_second_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_ME);
										//		list_remove_at(target_mod->L1hint_list2, get_second_priority_index);
										//	}
										//	else
										//	{
										//		stat_tick(1, hint_discard_state_OS);
										//		list_dequeue(target_mod->L1hint_list2);
										//	}

										//	stat_tick(1, hint_discard_state_Total);

										//	list_sort(target_mod->L1hint_list2, pf_hint_priority_compare);
										//	//IC_E

										//}

										//pre_dcu_prefetch_address_2 = (unsigned int)list_top(target_mod->L1hint_list2);

										//list_enqueue(target_mod->L1hint_list2, dcu_prefetch_address_ptr);
										//IC_S_2014_04_08
										pf_hint_pri->hint_address = stack->tag;

										if (stack->state == cache_block_owned || stack->state == cache_block_shared)
										{
											pf_hint_pri->priority = 0; //Highest priority
										}
										else if (stack->state == cache_block_modified || stack->state == cache_block_exclusive)
										{
											pf_hint_pri->priority = 1; //Second priority
										}
										else
										{
											pf_hint_pri->priority = 2; //Lowest priority
										}

										target_mod->hint_miss_trigger2 = 1;

										if (list_index_of(target_mod->L1hint_list2, pf_hint_pri) == -1)
										{
											stat_tick(stack->state==cache_block_invalid, hint_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_dcu_state_S);
											stat_tick(1, hint_dcu_state_Total);

											list_enqueue(target_mod->L1hint_list2, pf_hint_pri);

											prefetcher_access_dcu(stack, target_mod);
										}
										else
										{
											stat_tick(stack->state==cache_block_invalid, hint_already_in_buffer_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_already_in_buffer_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_already_in_buffer_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_already_in_buffer_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_already_in_buffer_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_already_in_buffer_dcu_state_S);
											stat_tick(1, hint_already_in_buffer_dcu_state_Total);

										}
										//IC_E

										//if (list_count(target_mod->L1hint_list2) > 1)
										//{
										//	;
										//}

										//if (list_count(target_mod->L1hint_list2) == (target_mod->L1hint->block_size/4))
										//{
										//	target_mod->hint_miss_trigger2 = 1;
										//	target_mod->hint_full++;
										//	//IC_S_2014_04_08
										//	list_sort(target_mod->L1hint_list2, pf_hint_priority_compare);
										//	//IC_E

										//}
										//else if (list_count(target_mod->L1hint_list2) < (target_mod->L1hint->block_size/4))
										//{
										//	//target_mod->hint_miss_trigger2 = 0;
										//}
										
										//IC_S_2014_04_22
										//prefetcher_access_dcu(stack, target_mod);
										//IC_E
										break;
									case 3:
										//if (list_count(target_mod->L1hint_list3) == (target_mod->L1hint->block_size/4))
										//{
										//	//list_dequeue(target_mod->L1hint_list3);
										//	
										//	//IC_S_2014_04_08
										//	get_lowest_priority_index = -1;
										//	get_second_priority_index = -1;

										//	for (tmp_i=0; tmp_i<list_count(target_mod->L1hint_list3); tmp_i++)
										//	{
										//		victim_tmp = list_get(target_mod->L1hint_list3, tmp_i);

										//		if (victim_tmp->priority == 2)
										//		{
										//			get_lowest_priority_index = tmp_i;
										//			break;
										//		}
										//		if (victim_tmp->priority == 1 && get_second_priority_index < 0)
										//		{
										//			get_second_priority_index = tmp_i;
										//		}

										//	}
										//	if (get_lowest_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_Iother);
										//		list_remove_at(target_mod->L1hint_list3, get_lowest_priority_index);
										//	}
										//	else if (get_second_priority_index >= 0)
										//	{
										//		stat_tick(1, hint_discard_state_ME);
										//		list_remove_at(target_mod->L1hint_list3, get_second_priority_index);
										//	}
										//	else
										//	{
										//		stat_tick(1, hint_discard_state_OS);
										//		list_dequeue(target_mod->L1hint_list3);
										//	}

										//	stat_tick(1, hint_discard_state_Total);

										//	list_sort(target_mod->L1hint_list3, pf_hint_priority_compare);
										//	//IC_E

										//}

										//pre_dcu_prefetch_address_3 = (unsigned int)list_top(target_mod->L1hint_list3);

										//list_enqueue(target_mod->L1hint_list3, dcu_prefetch_address_ptr);
										//IC_S_2014_04_08
										pf_hint_pri->hint_address = stack->tag;

										if (stack->state == cache_block_owned || stack->state == cache_block_shared)
										{
											pf_hint_pri->priority = 0; //Highest priority
										}
										else if (stack->state == cache_block_modified || stack->state == cache_block_exclusive)
										{
											pf_hint_pri->priority = 1; //Second priority
										}
										else
										{
											pf_hint_pri->priority = 2; //Lowest priority
										}

										target_mod->hint_miss_trigger3 = 1;

										if (list_index_of(target_mod->L1hint_list3, pf_hint_pri) == -1)
										{
											stat_tick(stack->state==cache_block_invalid, hint_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_dcu_state_S);
											stat_tick(1, hint_dcu_state_Total);

											list_enqueue(target_mod->L1hint_list3, pf_hint_pri);

											prefetcher_access_dcu(stack, target_mod);
										}
										else
										{
											stat_tick(stack->state==cache_block_invalid, hint_already_in_buffer_dcu_state_I);
											stat_tick(stack->state==cache_block_noncoherent, hint_already_in_buffer_dcu_state_NON);
											stat_tick(stack->state==cache_block_modified, hint_already_in_buffer_dcu_state_M);
											stat_tick(stack->state==cache_block_owned, hint_already_in_buffer_dcu_state_O);
											stat_tick(stack->state==cache_block_exclusive, hint_already_in_buffer_dcu_state_E);
											stat_tick(stack->state==cache_block_shared, hint_already_in_buffer_dcu_state_S);
											stat_tick(1, hint_already_in_buffer_dcu_state_Total);
										}
										//IC_E

										//if (list_count(target_mod->L1hint_list3) > 1)
										//{
										//	;
										//}

										//if (list_count(target_mod->L1hint_list3) == (target_mod->L1hint->block_size/4))
										//{
										//	target_mod->hint_miss_trigger3 = 1;
										//	target_mod->hint_full++;
										//	//IC_S_2014_04_08
										//	list_sort(target_mod->L1hint_list3, pf_hint_priority_compare);
										//	//IC_E

										//}
										//else if (list_count(target_mod->L1hint_list3) < (target_mod->L1hint->block_size/4))
										//{
										//	//target_mod->hint_miss_trigger3 = 0;
										//}

										//IC_S_2014_04_22
										//prefetcher_access_dcu(stack, target_mod);
										//IC_E
										break;

								}
							}
						}
						else
						{
							if (mod->cache->prefetcher->L2_DCU_ActbyHint == 0)
							{
								if( mod->cache->prefetcher->dcu_bidirection == 0){
									dcu_prefetch_address = dcu_prefetch_address + ((pref->pf_distance)<<(pref->log_block_size));
								}
								else{
									if(direction == 1)
											dcu_prefetch_address = dcu_prefetch_address + ((pref->pf_distance)<<(pref->log_block_size));		
									else if(direction == -1)
											dcu_prefetch_address = dcu_prefetch_address - ((pref->pf_distance)<<(pref->log_block_size));		
								}
							//	printf("issue: %x \n",dcu_prefetch_address);
								prefetcher_do_prefetch(mod, stack, dcu_prefetch_address );
							}
							else
							{

								//IC_S_2013_12_03
								//if (mod->hint_miss_trigger0 == 1 || mod->hint_change_trigger0 == 1)
								if (mod->hint_miss_trigger0 == 1)
								{
									//way = cache_replace_block(mod->L1hint, 1);
									//cache_decode_address(mod->L1hint, dcu_prefetch_address, NULL, &tag, NULL);
									//cache_set_block(mod->L1hint, set, way, tag, cache_block_exclusive);
									
									mod->hint_trigger_num0++;
									//dcu_prefetch_address_ptr = (unsigned int)list_dequeue(mod->L1hint_list0);
									//IC_S_2014_04_08
									pf_hint_pri  = list_dequeue(mod->L1hint_list0);
									//IC_E

									//IC_S_2014_03_25
									//dcu_prefetch_address = (*(dcu_prefetch_address_ptr)) + ((pref->pf_distance)<<pref->log_block_size);
									//IC_E

									//IC_S_2014_04_08
									if (pf_hint_pri != NULL)
									{
										stat_tick(pf_hint_pri->priority==0, hint_prefetch_issue_state_OS);
										stat_tick(pf_hint_pri->priority==1, hint_prefetch_issue_state_ME);
										stat_tick(pf_hint_pri->priority==2, hint_prefetch_issue_state_Iother);
										stat_tick((pf_hint_pri->priority==0 || pf_hint_pri->priority==1 || pf_hint_pri->priority==2), hint_prefetch_issue_state_Total);

										dcu_prefetch_address = (pf_hint_pri->hint_address) + ((pref->pf_distance)<<pref->log_block_size);

										prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
									}
									
									

									//prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
								}
								//if (mod->hint_miss_trigger1 == 1 || mod->hint_change_trigger1 == 1)
								if (mod->hint_miss_trigger1 == 1)
								{
									//way = cache_replace_block(mod->L1hint, 1);
									//cache_decode_address(mod->L1hint, dcu_prefetch_address, NULL, &tag, NULL);
									//cache_set_block(mod->L1hint, set, way, tag, cache_block_exclusive);
									
									mod->hint_trigger_num1++;
									//dcu_prefetch_address_ptr = (unsigned int)list_dequeue(mod->L1hint_list1);
									//IC_S_2014_04_08
									pf_hint_pri = list_dequeue(mod->L1hint_list1);
									//IC_E

									//IC_S_2014_03_25
									//dcu_prefetch_address = (*(dcu_prefetch_address_ptr)) + ((pref->pf_distance)<<pref->log_block_size);
									//IC_E
									
									//IC_S_2014_04_08
									if (pf_hint_pri != NULL)
									{
										stat_tick(pf_hint_pri->priority==0, hint_prefetch_issue_state_OS);
										stat_tick(pf_hint_pri->priority==1, hint_prefetch_issue_state_ME);
										stat_tick(pf_hint_pri->priority==2, hint_prefetch_issue_state_Iother);
										stat_tick((pf_hint_pri->priority==0 || pf_hint_pri->priority==1 || pf_hint_pri->priority==2), hint_prefetch_issue_state_Total);

										dcu_prefetch_address = (pf_hint_pri->hint_address) + ((pref->pf_distance)<<pref->log_block_size);

										prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
									}
									//IC_E

									//prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
								}
								//if (mod->hint_miss_trigger2 == 1 || mod->hint_change_trigger2 == 1)
								if (mod->hint_miss_trigger2 == 1)
								{
									//way = cache_replace_block(mod->L1hint, 1);
									//cache_decode_address(mod->L1hint, dcu_prefetch_address, NULL, &tag, NULL);
									//cache_set_block(mod->L1hint, set, way, tag, cache_block_exclusive);
									
									mod->hint_trigger_num2++;
									//dcu_prefetch_address_ptr = (unsigned int)list_dequeue(mod->L1hint_list2);
									//IC_S_2014_04_08
									pf_hint_pri = list_dequeue(mod->L1hint_list2);
									//IC_E

									//IC_S_2014_03_25
									//dcu_prefetch_address = (*(dcu_prefetch_address_ptr)) + ((pref->pf_distance)<<pref->log_block_size);
									//IC_E

									//IC_S_2014_04_08
									if (pf_hint_pri != NULL)
									{
										stat_tick(pf_hint_pri->priority==0, hint_prefetch_issue_state_OS);
										stat_tick(pf_hint_pri->priority==1, hint_prefetch_issue_state_ME);
										stat_tick(pf_hint_pri->priority==2, hint_prefetch_issue_state_Iother);
										stat_tick((pf_hint_pri->priority==0 || pf_hint_pri->priority==1 || pf_hint_pri->priority==2), hint_prefetch_issue_state_Total);

										dcu_prefetch_address = (pf_hint_pri->hint_address) + ((pref->pf_distance)<<pref->log_block_size);

										prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
									}
									//IC_E
									//prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
								}
								//if (mod->hint_miss_trigger3 == 1 || mod->hint_change_trigger3 == 1)
								if (mod->hint_miss_trigger3 == 1)
								{
									//way = cache_replace_block(mod->L1hint, 1);
									//cache_decode_address(mod->L1hint, dcu_prefetch_address, NULL, &tag, NULL);
									//cache_set_block(mod->L1hint, set, way, tag, cache_block_exclusive);
									
									mod->hint_trigger_num3++;
									//dcu_prefetch_address_ptr = (unsigned int)list_dequeue(mod->L1hint_list3);
									//IC_S_2014_04_08
									pf_hint_pri = list_dequeue(mod->L1hint_list3);
									//IC_E

									//IC_S_2014_03_25
									//dcu_prefetch_address = (*(dcu_prefetch_address_ptr)) + ((pref->pf_distance)<<pref->log_block_size);
									//IC_E

									//IC_S_2014_04_08
									if (pf_hint_pri != NULL)
									{
										stat_tick(pf_hint_pri->priority==0, hint_prefetch_issue_state_OS);
										stat_tick(pf_hint_pri->priority==1, hint_prefetch_issue_state_ME);
										stat_tick(pf_hint_pri->priority==2, hint_prefetch_issue_state_Iother);
										stat_tick((pf_hint_pri->priority==0 || pf_hint_pri->priority==1 || pf_hint_pri->priority==2), hint_prefetch_issue_state_Total);

										dcu_prefetch_address = (pf_hint_pri->hint_address) + ((pref->pf_distance)<<pref->log_block_size);

										prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
									}
									//IC_E
									//prefetcher_do_prefetch(mod, stack, dcu_prefetch_address);
								}
								//IC_E
							}
						}
						//IC_E

						//prefetcher_do_prefetch(mod, stack, dcu_prefetch_address ); //IC_S_2013_11_15 marked


					}	
					return;
				}
				return;
			}
			else
			{
				stream_LRU_replace(table,&entry->id,stream_type_to_MRU);
				return;
			}
		}
	}		

	//miss
	data.address = stack->addr;	
	stream_LRU_replace(table,&data,stream_type_to_replace);
}

void prefetcher_access_intel_ip(struct mod_stack_t *stack,struct mod_t * mod, int do_pf)
{
	struct prefetcher_t *pref;
	unsigned int address;
	unsigned int index;
	int stride;
	int i;
	pref = mod->cache->prefetcher;
	address = stack->addr&((1<<12)-1);

	assert(pref->ghb_size&(pref->ghb_size-1)==0);

	index = (stack->client_info->prefetcher_eip)&(pref->ghb_size-1);
	
	assert (index <pref->ghb_size);

        if(pref->type !=prefetcher_type_intel_ip || stack->read == 0)
                return;
	
	stride = address - pref->spt.addr[index];
	if(stride == 0)
		return;
	/*if(pref->spt.stride[index]==0 && pref->spt.state_machine[index]==ip_state_init && pref->spt.addr[index]==0)
	{
		pref->spt.stride[index] = stride;
		pref->spt.addr[index] = stack->tag;
		
	}*/
	//else 
	
	pref->spt.addr[index] = address;
	if (pref->spt.stride[index] == stride)
	{
		if(pref->spt.state_machine[index] == ip_state_no_pred)
		{
			pref->spt.state_machine[index]=ip_state_trasient;
		}
		else
		{
			pref->spt.state_machine[index]=ip_state_steady;
			////
			if(do_pf)
				for (i = 1; i <= pref->pf_degree; i ++)	
					prefetcher_do_prefetch(mod, stack, stack->addr + (i*stride));	
		}
	}
	else
	{
		if(pref->spt.state_machine[index] ==ip_state_steady)
			pref->spt.state_machine[index] = ip_state_init;

		else if (pref->spt.state_machine[index] ==ip_state_init)
		{
			pref->spt.state_machine[index] = ip_state_trasient;
			pref->spt.stride[index] = stride;
		}
		else if (pref->spt.state_machine[index] == ip_state_trasient)
		{
			pref->spt.state_machine[index] = ip_state_no_pred;
			pref->spt.stride[index] = stride;
		}
		else 
		{
			assert (pref->spt.state_machine[index] == ip_state_no_pred);
			pref->spt.stride[index] = stride;
		}	

	}

}
