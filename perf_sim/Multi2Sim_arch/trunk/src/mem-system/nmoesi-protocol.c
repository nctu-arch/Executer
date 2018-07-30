/*
	long long local_degree_pos1;
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
 *  You should have received stack copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>

#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <network/network.h>
#include <network/node.h>

#include "cache.h"
#include "directory.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "prefetcher.h"
#include "throttle.h"
#include "dram.h"
#include "address-analysis.h"
#include "dram-handler.h"

/* Events */

int EV_MOD_NMOESI_LOAD;
int EV_MOD_NMOESI_LOAD_LOCK;
int EV_MOD_NMOESI_LOAD_ACTION;
int EV_MOD_NMOESI_LOAD_MISS;
int EV_MOD_NMOESI_LOAD_UNLOCK;
int EV_MOD_NMOESI_LOAD_FINISH;

int EV_MOD_NMOESI_STORE;
int EV_MOD_NMOESI_STORE_LOCK;
int EV_MOD_NMOESI_STORE_ACTION;
int EV_MOD_NMOESI_STORE_UNLOCK;
int EV_MOD_NMOESI_STORE_FINISH;

int EV_MOD_NMOESI_PREFETCH;
int EV_MOD_NMOESI_PREFETCH_LOCK;
int EV_MOD_NMOESI_PREFETCH_ACTION;
int EV_MOD_NMOESI_PREFETCH_MISS;
int EV_MOD_NMOESI_PREFETCH_UNLOCK;
int EV_MOD_NMOESI_PREFETCH_FINISH;
int EV_MOD_NMOESI_PREFETCH_IDEAL;//cjc_20121225 ideal pf event
int EV_MOD_NMOESI_PREFETCH_IDEAL_INSERT;//cjc_20130107 ideal pf insertion event
int EV_MOD_NMOESI_PREFETCH_IDEAL_SEND;//cjc_20130121 ideal pf msg-sending
int EV_MOD_NMOESI_PREFETCH_IDEAL_RECV;//cjc_20130121 ideal pf msg-receiving
int EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY;//cjc_20130121 ideal pf latency-adding
int EV_MOD_NMOESI_PREFETCH_IDEAL_RETURN;
//IC_S_2013_11_13
int EV_MOD_NMOESI_L1DCUHINT;
int EV_MOD_NMOESI_L1DCUHINT_LOCK;
int EV_MOD_NMOESI_L1DCUHINT_ACTION;
//IC_E

int EV_MOD_NMOESI_NC_STORE;
int EV_MOD_NMOESI_NC_STORE_LOCK;
int EV_MOD_NMOESI_NC_STORE_WRITEBACK;
int EV_MOD_NMOESI_NC_STORE_ACTION;
int EV_MOD_NMOESI_NC_STORE_MISS;
int EV_MOD_NMOESI_NC_STORE_UNLOCK;
int EV_MOD_NMOESI_NC_STORE_FINISH;

int EV_MOD_NMOESI_FIND_AND_LOCK;
int EV_MOD_NMOESI_FIND_AND_LOCK_PORT;
int EV_MOD_NMOESI_FIND_AND_LOCK_ACTION;
int EV_MOD_NMOESI_FIND_AND_LOCK_FINISH;

int EV_MOD_NMOESI_EVICT;
int EV_MOD_NMOESI_EVICT_INVALID;
int EV_MOD_NMOESI_EVICT_ACTION;
int EV_MOD_NMOESI_EVICT_RECEIVE;
int EV_MOD_NMOESI_EVICT_PROCESS;
int EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT;
int EV_MOD_NMOESI_EVICT_REPLY;
int EV_MOD_NMOESI_EVICT_REPLY_RECEIVE;
int EV_MOD_NMOESI_EVICT_FINISH;

int EV_MOD_NMOESI_WRITE_REQUEST;
int EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_ACTION;
int EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN;
int EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP;
int EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_WRITE_REQUEST_REPLY;
int EV_MOD_NMOESI_WRITE_REQUEST_FINISH;

int EV_MOD_NMOESI_READ_REQUEST;
int EV_MOD_NMOESI_READ_REQUEST_RECEIVE;
int EV_MOD_NMOESI_READ_REQUEST_ACTION;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS;
int EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS;
int EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH;
int EV_MOD_NMOESI_READ_REQUEST_REPLY;
int EV_MOD_NMOESI_READ_REQUEST_FINISH;

int EV_MOD_NMOESI_INVALIDATE;
int EV_MOD_NMOESI_INVALIDATE_FINISH;

int EV_MOD_NMOESI_PEER_SEND;
int EV_MOD_NMOESI_PEER_RECEIVE;
int EV_MOD_NMOESI_PEER_REPLY;
int EV_MOD_NMOESI_PEER_FINISH;

int EV_MOD_NMOESI_MESSAGE;
int EV_MOD_NMOESI_MESSAGE_RECEIVE;
int EV_MOD_NMOESI_MESSAGE_ACTION;
int EV_MOD_NMOESI_MESSAGE_REPLY;
int EV_MOD_NMOESI_MESSAGE_FINISH;





/* NMOESI Protocol */

void mod_handler_nmoesi_load(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_LOAD)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s load\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"load\" "
				"state=\"%s:load\" addr=0x%x\n",
				stack->id, mod->name, stack->addr);

		stack->core_id = mod->mod_id;//cjc_20130408
		stack->pf_wait_cycle = 0;//cjc_20130107
		stack->fetch_level=0;  //porshe_20121125 calculate fetch level initial
        stack->issue_cycle = esim_cycle; //porshe_201125 used for fetch latency
	    stack->load_start = esim_cycle;//CIC for break down cyclei ujmyhn
	    /* Record access */
		mod_access_start(mod, stack, mod_access_load);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_load, stack->addr, stack);
		if (master_stack)
		{
			mod->reads++;
			stack->read = 1;
			//stack->tag = stack->addr & ~mod->pf_cache->block_mask;
			stack->tag = stack->addr & ~0x3f;
		  //  printf("%lld  0x%x %s load  %x\n", esim_cycle,master_stack->addr>>6, mod->name,stack->addr);
    		//if(mod->data_mod)prefetcher_access_hit(stack, mod); //before coalesce training prefetcher
    		if(mod->data_mod && (mod->cache->prefetcher != NULL))prefetcher_access_dcu(stack, mod); //before coalesce training prefetcher
//			prefetcher_access_dcu(master_stack, mod); //CIC 2013/12/30
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_LOAD_FINISH);
			if(master_stack->access_kind==mod_access_prefetch)//cjc_20121207 calculate request waiting time
			{
				stack->pf_wait_cycle = esim_cycle;
                                mod->pf_late_load++;
				mod->pf_late++;
				
				stack->read = 1;
				//prefetcher_access_dcu(stack,mod);//cjc_20130814


				if(master_stack->pf_master==0&&master_stack->pf_issued)
				{
					//stack->pf_wait_cycle = esim_cycle;
					//mod->pf_late_load++;
					master_stack->pf_master = 1;

				}
				if((stack->client_info&&master_stack->pc==stack->client_info->prefetcher_eip)||master_stack->stream_entry_id)
				{
					mod->pf_late_load_same_pc++;
					stack->stream_entry_id = master_stack->stream_entry_id;
					if(esim_warm_up_finish)
						prefetcher_access_hit_MSHR(stack, mod);
				}
			}
			return;
		}

		/* Next event */
		esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, 0);

		//cjc_20121217 start throttle if it is in idle status
		if(esim_warm_up_finish&&mod->cache->throttle->status==throttle_working_status_idle)
			esim_schedule_event(EV_THROTTLE_START,mod->cache->throttle,0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s load lock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_lock\"\n",
				stack->id, mod->name);
        stack->load_lock = esim_cycle; //CIC  
		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
					stack->id, older_stack->id);
			stack->waiting_oldwrite_inflight_num++;//CIC
			stack->old_write = 1;
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
					stack->id, older_stack->id);

			stack->waiting_oldwrite_inflight_num++;// CIC
			stack->inflight_ad = 1;                // CIC

			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
			/*if(older_stack->access_kind==mod_access_prefetch)//cjc_20121207 calculate request waiting time
			  {
			  stack->pf_wait_cycle = esim_cycle;
			  mod->pf_late_load++;
			  }*/
			return;
		}
		//record only looping waiting list; not even try find n lock 2013_10_29 
		if((stack->find_n_lock_issue_count == 0 )&(stack->waiting_oldwrite_inflight_num!=0))
				stack->waiting_oldwrite_inflight_cycle = esim_cycle - stack->issue_cycle;
        
		stack->issue_cycle = esim_cycle; //porshe_20131023 update issue cycle for eliminating the blocking cycles
		

		//cjc_20121226 ideal pf_cache_hit
		int set,way,state,tag;
		if(esim_warm_up_finish&&cache_find_block (mod->pf_cache,stack->addr,&set,&way,&state))
		{
			if(state==cache_blcok_transient)
			{
				assert(stack->ideal_wait_MSHR!=1);
				older_stack = cache_get_stack(mod->pf_cache, set, way);
				assert(older_stack);
				mod->pf_late_cycle_load+=esim_cycle-cache_get_insert_cycle(mod->pf_cache, set, way);
				mod->pf_late_load++;
				mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_LOAD_LOCK);
				stack->ideal_wait_MSHR = 1;	
				return;
			}
			else if(state==cache_block_exclusive)
			{
				stack->ideal_access = 1;
				mod->pf_ideal_access++;	
				tag = stack->addr & ~mod->pf_cache->block_mask;
				cache_set_block(mod->pf_cache, set, way, tag,cache_block_invalid);
				cache_set_invalid_to_tail(mod->pf_cache, set, way);
				stack->ideal_start_cycle = esim_cycle;	
				mem_debug("%lld %lld ideal access start\n", esim_cycle, stack->id);
			}
			else
				fatal("Invalid state of pf_cache");
		}
		else
		{
			stack->ideal_access = 0;
		}

        stack->load_find_n_lock = esim_cycle;

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
				EV_MOD_NMOESI_LOAD_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->read = 1;
		stack->find_n_lock_issue_count++;
		new_stack->demand_access = 1;//cjc_20130305
		new_stack->fetch_level = stack->fetch_level; //porshe_20121126 for fetch level calculate
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load action\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_action\"\n",
				stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);

			if(stack->ideal_access)//cjc_20121227 retry statistic
				mod->pf_ideal_l1_retry++;
			return;
		}
		
		stack->ending_find_n_lock_cycle = esim_cycle; // 20131025
		stack->read = 1;//cjc_20130814 identify load access
        stack->load_action = esim_cycle;//CIC
		/* Hit */
		if (stack->state)
		{
			esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			//printf("mod name: %s  , %x \n",mod->name,stack->addr);
			prefetcher_access_hit(stack, mod); 
			//cjc_20121215 clear dead-block flag
			//cache_set_dead_block_flag(mod->cache, stack->set, stack->way, 0);
			//IC_S_2012_12_27
			cache_set_dead_block_flag(mod->cache, stack->set, stack->way, reused);
			//IC_E
			if(stack->ideal_access)//cjc_20130104
                                mod->pf_hits++;
			return;
		}

		/* Miss */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_LOAD_MISS, stack);
		new_stack->read = 1; //IC_S_2013_11_13
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		new_stack->demand_access = 1;//cjc_20130311
		new_stack->peer_to_peer_trans_flag = 0; //porshe_20121128 init $2$ transfer flag
		new_stack->fetch_level = stack->fetch_level + 1; //porshe_20121125 increas count to next level
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

		/* The prefetcher may be interested in this miss */
		prefetcher_access_miss(stack, mod); 

		//printf(" miss mod name: %s  , %x \n",mod->name,stack->addr);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s load miss\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_miss\"\n",
				stack->id, mod->name);

		/* Error on read request. Unlock block and retry load. */
		if (stack->err)
		{
			mod->read_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			stack->fetch_level = 0;  //porshe_20121128_retry and reset fetch_level
			esim_schedule_event(EV_MOD_NMOESI_LOAD_LOCK, stack, retry_lat);
			if(stack->ideal_access)//cjc_20121227 retry statistic
				mod->pf_ideal_l2_retry++;
			return;
		}
        stack->load_miss_get = esim_cycle;
		//IC_S_2013_10_29
		if (stack->client_info)
		{
			mem_debug_2("  %lld %lld PC 0x%x miss_address 0x%x %s load_miss\n", esim_cycle, stack->id, stack->client_info->prefetcher_eip,
				stack->addr, mod->name);
		}
		else if(stack->prefetch)
		{
			mem_debug_2("  %lld %lld PC 0x%x miss_address 0x%x %s load_miss\n", esim_cycle, stack->id, stack->pc,
				stack->addr, mod->name);
		}
		else
		{
			//mem_debug_2("  %lld %lld PC 0x-1 0x%x %s load_miss\n", esim_cycle, stack->id, 
			//	stack->addr, mod->name);
		}
		//IC_E

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
				stack->shared ? cache_block_shared : cache_block_exclusive);

		//cjc_20121215 set dead block flags
		//cache_set_dead_block_flag(mod->cache, stack->set, stack->way, 1);
		//IC_S_2012_12_27
		cache_set_dead_block_flag(mod->cache, stack->set, stack->way, demand);
		//IC_E
		/* Continue */
		//mod_block_set_owner(mod, stack->addr, stack->core_id);
		esim_schedule_event(EV_MOD_NMOESI_LOAD_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s load unlock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_unlock\"\n",
				stack->id, mod->name);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);
        stack->load_unlock = esim_cycle;
		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_LOAD_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_LOAD_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s load finish\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		 mem_debug("fetch_level:%d, fetch_lat: %lld\n", stack->fetch_level, ((stack->coalesced) ? mod->latency : (esim_cycle-stack->issue_cycle))); //porshe_20121126 fetch latency debug

		mem_trace("mem.access name=\"A-%lld\" state=\"%s:load_finish\"\n",
				stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
				stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);


		if(stack->ideal_start_cycle)//cjc_20121227 ideal_access latency
		{
			mod->pf_ideal_wait_cycle+=(esim_cycle-(stack->ideal_start_cycle));
			mem_debug("%lld %lld ideal access finish\n", esim_cycle, stack->id);
		}
		if(stack->pf_wait_cycle)//cjc_20121207 statistic
			mod->pf_late_cycle_load+=(esim_cycle-(stack->pf_wait_cycle));

		/*porshe_20121125 fetch latency increase*/
		assert((esim_cycle-stack->issue_cycle != 0||stack->ideal_access )|| (esim_cycle-stack->issue_cycle == 0 && stack->coalesced));
		assert(stack->fetch_level < 3);
		if(stack->fetch_level == 0) {
			mod->R_hit_on_self_l1 += (stack->coalesced) ? mod->latency : (esim_cycle-stack->issue_cycle);
			//if(esim_cycle-stack->issue_cycle == 1 || stack->coalesced)
			if(stack->coalesced) //porshe_20131025: fix the bug with L1 latency
				mod->R_l1_w_depend_hit_num ++;
			else
				mod->R_l1_wo_depend_hit_num ++;
		}
		else if (stack->fetch_level == 1)
		{
			if(stack->peer_to_peer_trans_flag==1)
			{
				mod->R_hit_on_remote_l1 += (esim_cycle-stack->issue_cycle);
				mod->R_l1_remote_hit_num ++;
			}
			else
			{
				mod->R_hit_on_l2 += (esim_cycle-stack->issue_cycle);
				mod->R_l2_hit_num ++;
			}
		}
		else if (stack->fetch_level == 2)
		{
			mod->R_hit_on_memory += (esim_cycle-stack->issue_cycle);
			mod->R_mm_hit_num ++;
		}
        
		stack->load_finish = esim_cycle;
		
		mod->acc_waiting_oi_num    += stack->waiting_oldwrite_inflight_num;
		mod->acc_waiting_oi_cycle  += stack->waiting_oldwrite_inflight_cycle;
		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s store\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
				"state=\"%s:store\" addr=0x%x\n",
				stack->id, mod->name, stack->addr);

		stack->core_id = mod->mod_id;//cjc_20130408
		stack->pf_wait_cycle = 0;//cjc_20130107
		stack->fetch_level=0;  //porshe_20121125 calculate fetch level initial
		stack->issue_cycle = esim_cycle; //porshe_201125 used for fetch latency
		stack->store_start = esim_cycle;
		/* Record access */
		mod_access_start(mod, stack, mod_access_store);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_store, stack->addr, stack);
		if (master_stack)
		{
			mod->writes++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_STORE_FINISH);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		stack->pf_wait_cycle = 0;//cjc_20121207
		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, 0);

		//cjc_20121217 start throttle if it is in idle status
		if(esim_warm_up_finish&&mod->cache->throttle->status==throttle_working_status_idle)
			esim_schedule_event(EV_THROTTLE_START,mod->cache->throttle,0);
		return;
	}


	if (event == EV_MOD_NMOESI_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s store lock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_lock\"\n",
				stack->id, mod->name);
		stack->store_lock = esim_cycle;
		if(stack->pf_wait_cycle!=0)
		{
			mod->pf_late_cycle_store+=(esim_cycle-(stack->pf_wait_cycle));//cjc_20121207
			stack->pf_wait_cycle=0;
		}
		/* If there is any older access, wait for it */
		older_stack = stack->access_list_prev;
		if (older_stack)
		{
			mem_debug("    %lld wait for access %lld\n",
					stack->id, older_stack->id);
			if(older_stack->access_kind==mod_access_prefetch)
			{
				int mask = ~mod->cache->block_mask;
				if((older_stack->addr&mask)==(stack->addr&mask))
				{
					
					mod->pf_late_store++;
					mod->pf_late++;
					stack->pf_wait_cycle = esim_cycle;    //cjc_20121207 use to  calculate pf_late_cycle
					if(older_stack->pf_master==0&&older_stack->pf_issued)
					{
						older_stack->pf_master = 1;
					}
					if(stack->client_info&&older_stack->pc==stack->client_info->prefetcher_eip)
					{	
						mod->pf_late_store_same_pc++;
						if(esim_warm_up_finish)
							prefetcher_access_hit_MSHR(stack, mod);
					}
				}
			}
			stack->waiting_oldwrite_inflight_num++;// CIC
			stack->wait_store = 1;
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_STORE_LOCK);
			return;
		}

		if((stack->find_n_lock_issue_count == 0 )&(stack->waiting_oldwrite_inflight_num!=0))
				stack->waiting_oldwrite_inflight_cycle = esim_cycle - stack->issue_cycle;
        
		
		stack->issue_cycle = esim_cycle; //porshe_20131023 update issue cycle for eliminating the blocking cycles

		//cjc_20121226 ideal pf_cache_hit
		int set,way,state,tag;
		if(esim_warm_up_finish&&cache_find_block (mod->pf_cache,stack->addr,&set,&way,&state))
		{
			if(state==cache_blcok_transient)
			{
				assert(stack->ideal_wait_MSHR!=1);
				older_stack = cache_get_stack(mod->pf_cache, set, way);
				assert(older_stack);
				mod->pf_late_cycle_store+=esim_cycle-cache_get_insert_cycle(mod->pf_cache, set, way);
				mod->pf_late_store++;
				mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_STORE_LOCK);
				stack->ideal_wait_MSHR = 1;
				return;
			}
			else if(state==cache_block_exclusive)
			{
				stack->ideal_access = 1;
				mod->pf_ideal_access++;
				tag = stack->addr & ~mod->pf_cache->block_mask;
				cache_set_block(mod->pf_cache, set, way, tag,cache_block_invalid);
				cache_set_invalid_to_tail(mod->pf_cache, set, way);
				stack->ideal_start_cycle = esim_cycle;
				mem_debug("%lld %lld ideal access start\n", esim_cycle, stack->id);
			}
			else
				fatal("Invalid state of pf_cache");

		}
		else
		{
			stack->ideal_access = 0;
		}
        stack->store_find_n_lock = esim_cycle;
		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
				EV_MOD_NMOESI_STORE_ACTION, stack);
		new_stack->blocking = 1;
		new_stack->write = 1;
		new_stack->demand_access = 1;//cjc_20130305
		new_stack->retry = stack->retry;
		new_stack->witness_ptr = stack->witness_ptr;
        stack->find_n_lock_issue_count++; // CIC 20131025 
		//new_stack->ideal_access=1;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);

		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store action\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_action\"\n",
				stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			if(stack->ideal_access)//cjc_20121227 retry statistic
				mod->pf_ideal_l1_retry++;
			return;
		}
		stack->ending_find_n_lock_cycle = esim_cycle; // 20131025
        stack->store_find_n_lock = esim_cycle;
		//cjc_20121215 clear dead-block flag
		if(stack->state!=cache_block_invalid)
		{
			cache_set_dead_block_flag(mod->cache, stack->set, stack->way, 0);
		}

		/* Hit - state=M/E */
		if (stack->state == cache_block_modified ||
				stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_NMOESI_STORE_UNLOCK, stack, 0);

			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. */
			prefetcher_access_hit(stack, mod); 
			if(stack->ideal_access)//cjc_20130104
				mod->pf_hits++;
			return;
		}


		/* Miss - state=O/S/I/N */
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_STORE_UNLOCK, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->peer_to_peer_trans_flag = 0; //porshe_20121128 init $2$ transfer flag
		new_stack->fetch_level = stack->fetch_level + 1; //porshe_20121125 increas count to next level
		new_stack->demand_access = 1;//cjc_20130311

		new_stack->request_dir = mod_request_up_down;

		//IC_S_2013_10_29
		if (stack->client_info)
		{
			mem_debug_2("  %lld %lld PC 0x%x miss_address 0x%x %s store_miss\n", esim_cycle, stack->id, stack->client_info->prefetcher_eip,
				stack->addr, mod->name);
		}
		else if (stack->prefetch)
		{
			mem_debug_2("  %lld %lld PC 0x%x miss_address 0x%x %s store_miss\n", esim_cycle, stack->id, stack->pc,
				stack->addr, mod->name);
		}
		else
		{
			//mem_debug_2("  %lld %lld PC 0x-1 0x%x %s store_miss\n", esim_cycle, stack->id,
			//	stack->addr, mod->name);
		}

		//IC_E

		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);



		//cjc_20121215 set dead block flags
		if(stack->state==cache_block_invalid)
			//cache_set_dead_block_flag(mod->cache, stack->set, stack->way, 1);
			//IC_S_2012_12_27
			cache_set_dead_block_flag(mod->cache, stack->set, stack->way, demand);
			//IC_E

		/* The prefetcher may be interested in this miss */
		prefetcher_access_miss(stack, mod); 

		return;
	}

	if (event == EV_MOD_NMOESI_STORE_UNLOCK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s store unlock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_unlock\"\n",
				stack->id, mod->name);
        stack->store_miss_get = esim_cycle;
		/* Error in write request, unlock block and retry store. */
		if (stack->err)
		{
			mod->write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			stack->fetch_level = 0;  //porshe_20121128_retry and reset fetch_level
			esim_schedule_event(EV_MOD_NMOESI_STORE_LOCK, stack, retry_lat);
			if(stack->ideal_access)//cjc_20121227 retry statistic
				mod->pf_ideal_l2_retry++;
			return;
		}
        stack->store_unlock = esim_cycle;
		/* Update tag/state and unlock */
		cache_set_block(mod->cache, stack->set, stack->way,
				stack->tag, cache_block_modified);
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Continue */
		//mod_block_set_owner(mod, stack->addr, stack->core_id);
		esim_schedule_event(EV_MOD_NMOESI_STORE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s store finish\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_debug("fetch_level:%d, fetch_lat: %lld\n", stack->fetch_level, ((stack->coalesced) ? mod->latency : (esim_cycle-stack->issue_cycle))); //porshe_20121126 fetch latency debug
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:store_finish\"\n",
				stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
				stack->id);
        stack->store_finish = esim_cycle;
		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		if(stack->ideal_start_cycle)//cjc_20121227 ideal_access latency
		{
			mod->pf_ideal_wait_cycle+=(esim_cycle-(stack->ideal_start_cycle));
			mem_debug("%lld %lld ideal access finish\n", esim_cycle, stack->id);
		}
		/*porshe_20121125 fetch latency increase*/
		assert((esim_cycle-stack->issue_cycle != 0 ||stack->ideal_access)|| (esim_cycle-stack->issue_cycle == 0 && stack->coalesced));
		if(stack->fetch_level == 0) {
			mod->W_hit_on_self_l1 += (stack->coalesced) ? mod->latency : (esim_cycle-stack->issue_cycle);
			//if(esim_cycle-stack->issue_cycle == 1 || stack->coalesced)
			if(stack->coalesced)  //porshe_20131025 fix the bug with L1 latency
				mod->W_l1_w_depend_hit_num ++;
			else
				mod->W_l1_wo_depend_hit_num ++;
		}
		else if (stack->fetch_level == 1)
		{
			if(stack->peer_to_peer_trans_flag==1)
			{
				mod->W_hit_on_remote_l1 += (esim_cycle-stack->issue_cycle);
				mod->W_l1_remote_hit_num ++;
			} else
			{
				mod->W_hit_on_l2 += (esim_cycle-stack->issue_cycle);
				mod->W_l2_hit_num ++;
			}
		}
		else if (stack->fetch_level == 2)
		{
			mod->W_hit_on_memory += (esim_cycle-stack->issue_cycle);
			mod->W_mm_hit_num ++;
		}
        mod->acc_waiting_oi_num    += stack->waiting_oldwrite_inflight_num;
		mod->acc_waiting_oi_cycle  += stack->waiting_oldwrite_inflight_cycle;
		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_nc_store(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_NC_STORE)
	{
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s nc store\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"nc_store\" "
				"state=\"%s:nc store\" addr=0x%x\n", stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_nc_store);

		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_nc_store, stack->addr, stack);
		if (master_stack)
		{
			mod->nc_writes++;
			mod_coalesce(mod, master_stack, stack);
			mod_stack_wait_in_stack(stack, master_stack, EV_MOD_NMOESI_NC_STORE_FINISH);
			return;
		}

		/* Next event */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s nc store lock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_lock\"\n",
				stack->id, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}

		/* If there is any older access to the same address that this access could not
		 * be coalesced with, wait for it. */
		older_stack = mod_in_flight_address(mod, stack->addr, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n", stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_NC_STORE_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
				EV_MOD_NMOESI_NC_STORE_WRITEBACK, stack);
		new_stack->blocking = 1;
		new_stack->nc_write = 1;
		new_stack->demand_access = 1;//cjc_20130305
		new_stack->retry = stack->retry;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_WRITEBACK)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store writeback\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_writeback\"\n",
				stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* If the block has modified data, evict it so that the lower-level cache will
		 * have the latest copy */
		if (stack->state == cache_block_modified || stack->state == cache_block_owned)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod, 0,
					EV_MOD_NMOESI_NC_STORE_ACTION, stack);
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);
			//IC_S_2012_12_27
			mod->cache_evict_block++;
			//IC_E
			return;
		}

		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_ACTION)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store action\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_action\"\n",
				stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* N/S are hit */
		if (stack->state == cache_block_shared || stack->state == cache_block_noncoherent)
		{
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		}
		/* E state must tell the lower-level module to remove this module as an owner */
		else if (stack->state == cache_block_exclusive)
		{
			new_stack = mod_stack_create(stack->id, mod, stack->tag,
					EV_MOD_NMOESI_NC_STORE_MISS, stack);
			new_stack->message = message_clear_owner;
			new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE, new_stack, 0);
		}
		/* Modified and Owned states need to call read request because we've already
		 * evicted the block so that the lower-level cache will have the latest value
		 * before it becomes non-coherent */
		else
		{
			new_stack = mod_stack_create(stack->id, mod, stack->tag,
					EV_MOD_NMOESI_NC_STORE_MISS, stack);
			new_stack->peer = mod_stack_set_peer(mod, stack->state);
			new_stack->nc_write = 1;
                        new_stack->demand_access = 1;//cjc_20130311
			new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_MISS)
	{
		int retry_lat;

		mem_debug("  %lld %lld 0x%x %s nc store miss\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_miss\"\n",
				stack->id, mod->name);

		/* Error on read request. Unlock block and retry nc store. */
		if (stack->err)
		{
			mod->nc_write_retries++;
			retry_lat = mod_get_retry_latency(mod);
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, retrying in %d cycles\n", retry_lat);
			stack->retry = 1;
			esim_schedule_event(EV_MOD_NMOESI_NC_STORE_LOCK, stack, retry_lat);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_UNLOCK, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s nc store unlock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_unlock\"\n",
				stack->id, mod->name);

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
				cache_block_noncoherent);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_NC_STORE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_NC_STORE_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s nc store finish\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:nc_store_finish\"\n",
				stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
				stack->id);

		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_prefetch(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;   

	int cache_level = mod->level;
	assert(mod->kind==mod_kind_cache);
	
	stack->fetch_level = cache_level - 1;
	
	if (event == EV_MOD_NMOESI_PREFETCH)
	{

		stack->core_id = mod->mod_id;//cjc_20130408
		
		//cjc_20130208_start
		{
			stack->higher_mod = stack->mod;
			
			stack->mod=pf_find_destination(stack->addr , stack->higher_mod,(stack->low_insert)?1:-1);
			
			mod=stack->mod;
			if(mod==stack->higher_mod)
				stack->pf_source = -1;
			else if(mod->data_mod!=1&&stack->higher_mod->level==1)
				stack->pf_source = stack->higher_mod->mod_id;
			else
				stack->pf_source = -1;

		}
		//cjc_20130208_end

		if(mod->ideal_pf_cache&&mod->ideal_pf_cache_pollution==0)
		{
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL, stack, 0);
			return;
		}
		else if (mod->ideal_pf_cache&&mod->ideal_pf_cache_pollution&&mod->ideal_pf_latency_hiding&&mod->ideal_pf_cache_contention==0)
			stack->ideal_access = 1;
		struct mod_stack_t *master_stack;

		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
				"state=\"%s:prefetch\" addr=0x%x\n",
				stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_prefetch);
		stack->prefetch = 1;
		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_prefetch, stack->addr, stack);
		if (master_stack)
		{
			/* Doesn't make sense to prefetch as the block is already being fetched */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - already being fetched\n",
					esim_cycle, stack->id, stack->addr, mod->name);

			mod->useless_prefetches++;
			mod->pf_already_in_MSHR++;//cjc_20121211
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);

			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;

			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_LOCK, stack, 0);
		return;
	}
	
	//IC_S_2013_11_13
	if (event == EV_MOD_NMOESI_L1DCUHINT)
	{

		stack->core_id = mod->mod_id;//cjc_20130408
		
		//cjc_20130208_start
		//{
		//	stack->higher_mod = stack->mod;
		//	
		//	stack->mod=pf_find_destination(stack->addr , stack->higher_mod,(stack->low_insert)?1:-1);
		//	
		//	mod=stack->mod;
		//	if(mod==stack->higher_mod)
		//		stack->pf_source = -1;
		//	else if(mod->data_mod!=1&&stack->higher_mod->level==1)
		//		stack->pf_source = stack->higher_mod->mod_id;
		//	else
		//		stack->pf_source = -1;
		//
		//}
		//cjc_20130208_end
		//
		//if(mod->ideal_pf_cache&&mod->ideal_pf_cache_pollution==0)
		//{
		//	esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL, stack, 0);
		//	return;
		//}
		//else if (mod->ideal_pf_cache&&mod->ideal_pf_cache_pollution&&mod->ideal_pf_latency_hiding&&mod->ideal_pf_cache_contention==0)
		//	stack->ideal_access = 1;
		struct mod_stack_t *master_stack;
		
		mem_debug("%lld %lld 0x%x %s L1DCU hint prefetch\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.new_access name=\"A-%lld\" type=\"store\" "
				"state=\"%s:prefetch\" addr=0x%x\n",
				stack->id, mod->name, stack->addr);

		/* Record access */
		mod_access_start(mod, stack, mod_access_L1DCUHint);
		stack->prefetch = 1;
		/* Coalesce access */
		master_stack = mod_can_coalesce(mod, mod_access_L1DCUHint, stack->addr, stack);
		if (master_stack)
		{
			/* Doesn't make sense to prefetch as the block is already being fetched */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - already being fetched\n",
					esim_cycle, stack->id, stack->addr, mod->name);
		
			mod->useless_prefetches++;
			mod->pf_already_in_MSHR++;//cjc_20121211
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
		
			/* Increment witness variable */
			if (stack->witness_ptr)
				(*stack->witness_ptr)++;
		
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_L1DCUHINT_LOCK, stack, 0);
		return;
	}
	//IC_E
	
	if (event == EV_MOD_NMOESI_PREFETCH_LOCK)
	{
		struct mod_stack_t *older_stack;

		mem_debug("  %lld %lld 0x%x %s prefetch lock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_lock\"\n",
				stack->id, mod->name);

		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
					stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_PREFETCH_LOCK);
			return;
		}

		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
				EV_MOD_NMOESI_PREFETCH_ACTION, stack);
		//IC_S_2013_11_13
		//if (L1_DCU_hint == 1)
		//{
		//	new_stack = mod_stack_create(stack->id, mod, stack->addr,
		//		EV_MOD_NMOESI_L1DCUHINT_ACTION, stack);
		//}
		//else
		//{
		//	new_stack = mod_stack_create(stack->id, mod, stack->addr,
		//		EV_MOD_NMOESI_PREFETCH_ACTION, stack);
		//}
		//IC_E
		new_stack->blocking = 0;
		new_stack->prefetch = 1;
		new_stack->retry = 0;
		new_stack->witness_ptr = stack->witness_ptr;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}

	//IC_S_2013_11_13
	if (event == EV_MOD_NMOESI_L1DCUHINT_LOCK)
	{
		struct mod_stack_t *older_stack;
		
		mem_debug("  %lld %lld 0x%x %s L1DCU hint prefetch lock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_lock\"\n",
				stack->id, mod->name);
		
		/* If there is any older write, wait for it */
		older_stack = mod_in_flight_write(mod, stack);
		if (older_stack)
		{
			mem_debug("    %lld wait for write %lld\n",
					stack->id, older_stack->id);
			mod_stack_wait_in_stack(stack, older_stack, EV_MOD_NMOESI_PREFETCH_LOCK);
			return;
		}
		
		/* Call find and lock */
		new_stack = mod_stack_create(stack->id, mod, stack->addr,
				EV_MOD_NMOESI_L1DCUHINT_ACTION, stack);
		new_stack->blocking = 0;
		new_stack->prefetch = 1;
		new_stack->retry = 0;
		new_stack->witness_ptr = stack->witness_ptr;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		/* Set witness variable to NULL so that retries from the same
		 * stack do not increment it multiple times */
		stack->witness_ptr = NULL;

		return;
	}
	//IC_E

	if (event == EV_MOD_NMOESI_PREFETCH_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch action\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_action\"\n",
				stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if getting a lock failed. 
			   Effectively this means that prefetches are of low priority.
			   This can be improved to not retry only when the current lock
			   holder is writing to the block. */
			mod->prefetch_aborts++;
			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Hit */
		if (stack->state)
		{
			/* block already in the cache */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - cache hit\n",
					esim_cycle, stack->id, stack->addr, mod->name);
			mod->pf_already_in_cache++;
			mod->useless_prefetches++;
			//IC_S_2013_09_24
			//0:hit 1:miss
			//prefetch_result = 0;
			//IC_E
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
			return;
		}

		/* Miss */
		if(stack->pf_source!=-1)
		{
			stack->higher_mod->pf_issued++;
			mod_block_set_pf_owner(mod, stack->addr,stack->pf_source);
		}	
		
		mod->pf_issued++;//cjc_20121211
		esim_schedule_event(EV_THROTTLE_NEW_COUNTER, mod->cache->throttle,0);//cjc_20121217 notify throttle
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_PREFETCH_MISS, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		new_stack->prefetch = cache_level;
		new_stack->fetch_level = stack->fetch_level+1;
		stack->pf_issued = 1;
		//IC_S_2013_09_24
		//0:hit 1:miss
		//prefetch_result = 1;
		//IC_E
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		if(stack->pf_master)
			mod_block_set_prefetched(mod, stack->addr, 0);
		else
			mod_block_set_prefetched(mod, stack->addr, cache_level);//cjc_20121219 inorder to calc pollution
		cache_set_insert_cycle(mod->cache,stack->set,stack->way,esim_cycle);
		return;
	}

	//IC_S_2013_11_13
	if (event == EV_MOD_NMOESI_L1DCUHINT_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s L1DCU hint prefetch action\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		//mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_action\"\n",
		//		stack->id, mod->name);

		/* Error locking */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if getting a lock failed. 
			   Effectively this means that prefetches are of low priority.
			   This can be improved to not retry only when the current lock
			   holder is writing to the block. */
			mod->prefetch_aborts++;
			mem_debug("    lock error, aborting L1DCU hint prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Hit */
		if (stack->state)
		{
			/* block already in the cache */
			mem_debug("  %lld %lld 0x%x %s useless L1DCU hint prefetch - cache hit\n",
					esim_cycle, stack->id, stack->addr, mod->name);
			mod->pf_already_in_cache++;
			mod->useless_prefetches++;
			//IC_S_2013_09_24
			//0:hit 1:miss
			//prefetch_result = 0;
			//IC_E
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);
			return;
		}

		/* Miss */
		//if(stack->pf_source!=-1)
		//{
		//	stack->higher_mod->pf_issued++;
		//	mod_block_set_pf_owner(mod, stack->addr,stack->pf_source);
		//}	
		
		mod->pf_issued++;//cjc_20121211
		//esim_schedule_event(EV_THROTTLE_NEW_COUNTER, mod->cache->throttle,0);//cjc_20121217 notify throttle
		new_stack = mod_stack_create(stack->id, mod, stack->tag,
				EV_MOD_NMOESI_PREFETCH_UNLOCK, stack);
		new_stack->peer = mod_stack_set_peer(mod, stack->state);
		new_stack->target_mod = mod_get_low_mod(mod, stack->tag);
		new_stack->request_dir = mod_request_up_down;
		new_stack->prefetch = cache_level;
		new_stack->fetch_level = stack->fetch_level+1;
		stack->pf_issued = 1;
		//IC_S_2013_09_24
		//0:hit 1:miss
		//prefetch_result = 1;
		//IC_E
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		//if(stack->pf_master)
		//	mod_block_set_prefetched(mod, stack->addr, 0);
		//else
		//	mod_block_set_prefetched(mod, stack->addr, cache_level);//cjc_20121219 inorder to calc pollution
		//IC_S_2013_11_26
		mod_block_set_prefetched(mod, stack->addr, cache_level);
		//IC_E
		cache_set_insert_cycle(mod->cache,stack->set,stack->way,esim_cycle);
		return;
	}
	//IC_E

	if (event == EV_MOD_NMOESI_PREFETCH_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch miss\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_miss\"\n",
				stack->id, mod->name);

		/* Error on read request. Unlock block and abort. */
		if (stack->err)
		{
			/* Don't want to ever retry prefetches if read request failed. 
			 * Effectively this means that prefetches are of low priority.
			 * This can be improved depending on the reason for read request fail */
			mod->prefetch_aborts++;
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mem_debug("    lock error, aborting prefetch\n");
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on return var 'shared'.
		 * Also set the tag of the block. */
		cache_set_block(mod->cache, stack->set, stack->way, stack->tag,
				stack->shared ? cache_block_shared : cache_block_exclusive);

		//IC_S_2012_12_27
		cache_set_dead_block_flag(mod->cache, stack->set, stack->way, prefetch);
		//IC_E

		//cache_set_insert_cycle(mod->cache,stack->set,stack->way,esim_cycle);
		/* Mark the prefetched block as prefetched. This is needed to let the 
		 * prefetcher know about an actual access to this block so that it
		 * is aware of all misses as they would be without the prefetcher. 
		 * TODO: The lower caches that will be filled because of this prefetch
		 * do not know if it was a prefetch or not. Need to have a way to mark
		 * them as prefetched too. */
		//mod_block_set_prefetched(mod, stack->addr, cache_level);//cjc_20121206 change $3 from  1  to  cache_level

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_UNLOCK, stack, 0);

		prefetcher_pollution_bloom_filter_set(mod->cache,stack->tag,0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_UNLOCK)
	{
		mem_debug("  %lld %lld 0x%x %s prefetch unlock\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_unlock\"\n",
				stack->id, mod->name);

		/* Unlock directory entry */
		dir_entry_unlock(mod->dir, stack->set, stack->way);

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_PREFETCH_FINISH)
	{
		mem_debug("%lld %lld 0x%x %s prefetch\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:prefetch_finish\"\n",
				stack->id, mod->name);
		mem_trace("mem.end_access name=\"A-%lld\"\n",
				stack->id);
		assert(stack->prefetch!=0);
		/* Increment witness variable */
		if (stack->witness_ptr)
			(*stack->witness_ptr)++;

		/* Return event queue element into event queue */
		if (stack->event_queue && stack->event_queue_item)
			linked_list_add(stack->event_queue, stack->event_queue_item);

		/* Free the mod_client_info object, if any */
		if (stack->client_info)
			mod_client_info_free(mod, stack->client_info);

		if(stack->pf_master)//cjc_20130207
			mod_block_set_prefetched(mod, stack->addr, 0);
		/* Finish access */
		mod_access_finish(mod, stack);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	if(event == EV_MOD_NMOESI_PREFETCH_IDEAL)//cjc_20121225 handle ideal pf event
	{
		struct mod_t *target_mod ;
		int hit,latency;
		if (mod_in_flight_address(mod, stack->addr, stack))
		{
			/* Doesn't make sense to prefetch as the block is already being fetched */
			mem_debug("  %lld %lld 0x%x %s useless prefetch - already being fetched(ideal)\n",
					esim_cycle, stack->id, stack->addr, mod->name);
			mod->pf_already_in_MSHR++;
			mod_stack_return(stack);
			return;
		}
		target_mod = mod;

		//only search current level
		hit=mod_find_block(target_mod, stack->addr, &stack->set,
				&stack->way, &stack->tag, &stack->state);
		if(hit)
		{
			mem_debug("  %lld %lld 0x%x already_in_cache %s\n",esim_cycle, stack->id, stack->addr,target_mod->name);
			target_mod ->pf_already_in_cache++;
			mod_stack_return(stack);
			return;
		}	
		//only search current level

		//search all of lower level cache
		/*while(target_mod)
		  {	
		  hit=mod_find_block(target_mod, stack->addr, &stack->set,
		  &stack->way, &stack->tag, &stack->state);
		  if(hit)
		  {
		  mem_debug("  %lld %lld 0x%x already_in_cache %s\n",esim_cycle, stack->id, stack->addr,target_mod->name);
		  target_mod ->pf_already_in_cache++;
		  mod_stack_return(stack);
		  return;
		  }
		  target_mod = mod_get_low_mod(target_mod, stack->addr);	
		  if(target_mod->kind==mod_kind_main_memory)
		  break;
		  }*/
		//search all of lower level cache	

		assert(mod->pf_cache);
		hit = cache_find_block (mod->pf_cache,stack->addr,&stack->set,NULL,NULL);
		if(hit)
		{
			mod->useless_prefetches++; //hit in pf_cache
			mem_debug("  %lld %lld 0x%x already_in_pf_cache\n",esim_cycle, stack->id, stack->addr);
			mod_stack_return(stack);
			return;
		}
		else
		{
			mod->pf_issued++;
			stack->way = cache_replace_block(mod->pf_cache, stack->set);
			cache_get_block(mod->pf_cache, stack->set, stack->way, &stack->tag, &stack->state);
			if(stack->state!=cache_block_invalid)
				mod->pf_ideal_cache_victim++;
			stack->tag = stack->addr & ~mod->pf_cache->block_mask;
			cache_set_block(mod->pf_cache, stack->set, stack->way, stack->tag,cache_blcok_transient);
			cache_set_stack(mod->pf_cache, stack->set, stack->way, stack);	
			cache_set_insert_cycle(mod->pf_cache, stack->set, stack->way,esim_cycle);
			stack->issue_cycle = esim_cycle;	
			latency=0;
			if(mod->ideal_pf_latency_hiding==0)
			{
				if(mod->ideal_pf_cache_contention)
				{
					new_stack = mod_stack_create(stack->id, mod, stack->addr,
							EV_MOD_NMOESI_PREFETCH_IDEAL_INSERT, stack);
					new_stack->request_dir = mod_request_up_down;
					new_stack->target_mod = mod_get_low_mod(mod, stack->addr);
					esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_SEND, new_stack, 0);
					return;
				}
				target_mod = mod;
				while(target_mod)
				{
					target_mod = mod_get_low_mod(target_mod, stack->addr);
					hit=mod_find_block(target_mod, stack->addr,NULL,NULL,NULL,NULL);
					latency += target_mod->latency;
					if(target_mod->kind==mod_kind_main_memory)
					{
						mod->pf_ideal_mem_access++;
						break;
					}
					if(hit)
						break;
				}
			}
			mem_debug("  %lld %lld 0x%x ideal prefetching_start, latency =%d\n",esim_cycle, stack->id, stack->addr,latency);
			//assert(latency>0);
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_INSERT, stack, latency);
			return;
		}
	}
	if(event == EV_MOD_NMOESI_PREFETCH_IDEAL_INSERT)//cjc_20130107 handle ideal pf insertion event
	{
		mem_debug("  %lld %lld 0x%x ideal prefetching_end\n",esim_cycle, stack->id, stack->addr);
		cache_set_block(mod->pf_cache, stack->set, stack->way, stack->tag,cache_block_exclusive);
		cache_set_stack(mod->pf_cache, stack->set, stack->way, NULL);
		mod->pf_ideal_latency+=esim_cycle-(stack->issue_cycle);
		mod_stack_return(stack);
		return;
	}

	if(event == EV_MOD_NMOESI_PREFETCH_IDEAL_SEND)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;
		struct mod_t *target_mod = stack->target_mod;
		int size;	
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = mod->low_net_node;
			dst_node = target_mod->high_net_node;
			size = 8;
		}
		else
		{	
			net = mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = mod->low_net_node;
			size = 8 + target_mod->block_size;
		}
		stack->msg = net_try_send_ev(net, src_node, dst_node, size,
				EV_MOD_NMOESI_PREFETCH_IDEAL_RECV, stack, event, stack);
		return;
	}

	if(event == EV_MOD_NMOESI_PREFETCH_IDEAL_RECV)
	{
		struct mod_t *target_mod = stack->target_mod;
		int hit;
		if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
		{	
			net_receive(mod->low_net, mod->low_net_node, stack->msg);	
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_RETURN, stack, 0);	
			return;
		}	
		hit=mod_find_block(target_mod, stack->addr,NULL,NULL,NULL,NULL);
		if(target_mod->kind==mod_kind_main_memory)
		{
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY, stack, 0);
			mod->pf_ideal_mem_access++;
		}
		else if(hit)
		{
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY, stack, 0);
		}
		else
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
					EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY, stack);
			new_stack->request_dir = mod_request_up_down;
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->addr);
			esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_SEND, new_stack, 0);
		}
		return;
	}

	if( event == EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY )
	{
		struct mod_t *target_mod = stack->target_mod;
		assert(stack->request_dir == mod_request_up_down);
		stack->request_dir = mod_request_down_up;
		esim_schedule_event(EV_MOD_NMOESI_PREFETCH_IDEAL_SEND, stack, target_mod->latency);	
		return;
	}

	if(event == EV_MOD_NMOESI_PREFETCH_IDEAL_RETURN)
	{
		assert(stack->request_dir == mod_request_down_up);
		mod_stack_return(stack);
		return;
	}
	abort();
}

void mod_handler_nmoesi_find_and_lock(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;


	if (event == EV_MOD_NMOESI_FIND_AND_LOCK)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock (blocking=%d)\n",
				esim_cycle, stack->id, stack->addr, mod->name, stack->blocking);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock\"\n",
				stack->id, mod->name);

		/* Default return values */
		ret->err = 0;

		/* If this stack has already been assigned a way, keep using it */
		stack->way = ret->way;

		/* Get a port */
        stack->lock_port_start = esim_cycle;

		mod_lock_port(mod, stack, EV_MOD_NMOESI_FIND_AND_LOCK_PORT);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_PORT)
	{
		struct mod_port_t *port = stack->port;
		struct dir_lock_t *dir_lock;

		assert(stack->port);
		mem_debug("  %lld %lld 0x%x %s find and lock port\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_port\"\n",
				stack->id, mod->name);

		/* Set parent stack flag expressing that port has already been locked.
		 * This flag is checked by new writes to find out if it is already too
		 * late to coalesce. */
		ret->port_locked = 1;

		/* Look for block. */
		stack->hit = mod_find_block(mod, stack->addr, &stack->set,
				&stack->way, &stack->tag, &stack->state);
        //mod->cache->num_of_access_on_set[stack->set]++; //porshe:20130219 profiling cache accesses distribution of cache lines
		/* Debug */
		if (stack->hit)
			mem_debug("    %lld 0x%x %s hit: set=%d, way=%d, state=%s\n", stack->id,
					stack->tag, mod->name, stack->set, stack->way,
					str_map_value(&cache_block_state_map, stack->state));

		/* Statistics */
		mod->accesses++;
		if (stack->hit)
			mod->hits++;

		if(stack->demand_access)
		{
			mod->demand_accesses++;
			if (stack->hit)
                        	mod->demand_hits++;
			if (stack->read)
				mod->demand_reads++;
			else if(stack->write)
				mod->demand_writes++;
		}
	
		if (stack->read)
		{
			mod->reads++;
			mod->effective_reads++;
			stack->blocking ? mod->blocking_reads++ : mod->non_blocking_reads++;
			if (stack->hit)
				mod->read_hits++;
		}
		else if (stack->prefetch)
		{
			mod->prefetches++;
		}
		else if (stack->nc_write)  /* Must go after read */
		{
			mod->nc_writes++;
			mod->effective_nc_writes++;
			stack->blocking ? mod->blocking_nc_writes++ : mod->non_blocking_nc_writes++;
			if (stack->hit)
				mod->nc_write_hits++;
		}
		else if (stack->write)
		{
			mod->writes++;
			mod->effective_writes++;
			stack->blocking ? mod->blocking_writes++ : mod->non_blocking_writes++;

			/* Increment witness variable when port is locked */
			if (stack->witness_ptr)
			{
				(*stack->witness_ptr)++;
				stack->witness_ptr = NULL;
			}

			if (stack->hit)
				mod->write_hits++;
		}
		else if (stack->message)
		{
			/* FIXME */
		}
		else 
		{
			fatal("Unknown memory operation type");
		}

		if (!stack->retry)
		{
			mod->no_retry_accesses++;
			if (stack->hit)
				mod->no_retry_hits++;

			if (stack->read)
			{
				mod->no_retry_reads++;
				if (stack->hit)
					mod->no_retry_read_hits++;
			}
			else if (stack->nc_write)  /* Must go after read */
			{
				mod->no_retry_nc_writes++;
				if (stack->hit)
					mod->no_retry_nc_write_hits++;
			}
			else if (stack->write)
			{
				mod->no_retry_writes++;
				if (stack->hit)
					mod->no_retry_write_hits++;
			}
			else if (stack->prefetch)
			{
				/* No retries currently for prefetches */
			}
			else if (stack->message)
			{
				/* FIXME */
			}
			else 
			{
				fatal("Unknown memory operation type");
			}
		}

		if (!stack->hit)
		{
			/* Find victim */
			if (stack->way < 0) 
			{
				stack->way = cache_replace_block(mod->cache, stack->set);
			}
		}
		assert(stack->way >= 0);

		/* If directory entry is locked and the call to FIND_AND_LOCK is not
		 * blocking, release port and return error. */
		dir_lock = dir_lock_get(mod->dir, stack->set, stack->way);
		if (dir_lock->lock && !stack->blocking)
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - aborting\n",
					stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			ret->err = 1;
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;

			//IC_S_2013_10_23
			if (stack->demand_access)
			{
				mod->Demand_Access--;
				if (stack->hit)
				{
					mod->Demand_Hit--;
				}
				if (stack->read)
				{
					mod->Demand_Read--;
				}
				else if (stack->write)
				{
					mod->Demand_Write--;
				}
			}
			//IC_E

			mod_stack_return(stack);
			return;
		}

		/* Lock directory entry. If lock fails, port needs to be released to prevent 
		 * deadlock.  When the directory entry is released, locking port and 
		 * directory entry will be retried. */
		if (!dir_entry_lock(mod->dir, stack->set, stack->way, EV_MOD_NMOESI_FIND_AND_LOCK, 
					stack))
		{
			mem_debug("    %lld 0x%x %s block locked at set=%d, way=%d by A-%lld - waiting\n",
					stack->id, stack->tag, mod->name, stack->set, stack->way, dir_lock->stack_id);
			mod_unlock_port(mod, port, stack);
			ret->port_locked = 0;
			return;
		}

		//IC_S_2013_10_23
		//XXX
		if (stack->demand_access)
		{
			mod->Demand_Access++;
			if (stack->hit)
			{
				mod->Demand_Hit++;
			}
			if (stack->read)
			{
				mod->Demand_Read++;
			}
			else if (stack->write)
			{
				mod->Demand_Write++;
			}
		}
	
		//IC_E
		//
		/* Miss */

		//if (!stack->hit)
		//{
		//	/* Find victim */
		//	cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
            	//	mod->cache->num_of_access_on_set[stack->set]++; //porshe:20130222 profiling cache accesses distribution of cache lines
		//	assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
		//				stack->set, stack->way));
		//	mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
		//			stack->id, stack->tag, mod->name, stack->set, stack->way,
		//			str_map_value(&cache_block_state_map, stack->state));

		//	if(stack->prefetch&&mod->kind==mod_kind_cache)
		//	{
		//		unsigned int tag;
		//		tag = cache_set_victim_tag(mod->cache, stack->set, stack->way);
		//		prefetcher_pollution_bloom_filter_set(mod->cache,tag,1);
		//	}
		//}


		///* Entry is locked. Record the transient tag so that a subsequent lookup
		// * detects that the block is being brought.
		// * Also, update LRU counters here. */
		//cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
		//cache_access_block(mod->cache, stack->set, stack->way);

		///* Access latency */
		//int latency;
		//latency = (stack->ideal_access)?0:mod->dir_latency;      //cjc_20121225 ideal access have no access latency
		//esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_ACTION, stack, latency);
		//return;

		//IC_S_2013_11_13
		if (mod->cache->prefetcher == 1 && mod->cache->prefetcher->L1_DCU_hint == 1)
		{
			/* Entry is locked. Record the transient tag so that a subsequent lookup
		 	*  * detects that the block is being brought.
		 	*   * Also, update LRU counters here. */
			cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
			cache_access_block(mod->cache, stack->set, stack->way);
			/* Access latency */
			int latency;
			latency = (stack->ideal_access)?0:mod->dir_latency;      //cjc_20121225 ideal access have no access latency
			esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack, latency);
			return;
		}
		else
		{
			if (!stack->hit)
			{
				/* Find victim */
				cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
            			mod->cache->num_of_access_on_set[stack->set]++; //porshe:20130222 profiling cache accesses distribution of cache lines
				assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
							stack->set, stack->way));
				mem_debug("    %lld 0x%x %s miss -> lru: set=%d, way=%d, state=%s\n",
						stack->id, stack->tag, mod->name, stack->set, stack->way,
						str_map_value(&cache_block_state_map, stack->state));

				if(stack->prefetch&&mod->kind==mod_kind_cache)
				{
					unsigned int tag;
					tag = cache_set_victim_tag(mod->cache, stack->set, stack->way);
					prefetcher_pollution_bloom_filter_set(mod->cache,tag,1);
				}
			}


			/* Entry is locked. Record the transient tag so that a subsequent lookup
			 * detects that the block is being brought.
			 * Also, update LRU counters here. */
			cache_set_transient_tag(mod->cache, stack->set, stack->way, stack->tag);
			cache_access_block(mod->cache, stack->set, stack->way);

			/* Access latency */
			int latency;
			latency = (stack->ideal_access)?0:mod->dir_latency;      //cjc_20121225 ideal access have no access latency
			esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_ACTION, stack, latency);
			return;

		}
		//IC_E
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_ACTION)
	{
		struct mod_port_t *port = stack->port;

		assert(port);
		mem_debug("  %lld %lld 0x%x %s find and lock action\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_action\"\n",
				stack->id, mod->name);

		/* Release port */
		mod_unlock_port(mod, port, stack);
		ret->port_locked = 0;

		/* On miss, evict if victim is a valid block. */
		if (!stack->hit && stack->state)
		{
			stack->eviction = 1;
			new_stack = mod_stack_create(stack->id, mod, 0,
					EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack);
			new_stack->set = stack->set;
			new_stack->way = stack->way;
			new_stack->prefetch = stack->prefetch;//cjc_20130502
			if(mod->kind==mod_kind_cache)
			{
				mod_block_set_prefetched(mod, stack->addr, 0);
				mod_block_set_pf_owner (mod, stack->addr,-1);//cjc_20130208 clear owner

				if(mod->level==2&&stack->prefetch&&stack->core_id!=-1)
				{
					if(mod_block_get_owner(mod, stack->addr)!=stack->core_id)
					{
						 struct mod_t * high_mod;
						 LINKED_LIST_FOR_EACH(mod->high_mod_list)
						 {
							high_mod = linked_list_get(mod->high_mod_list);
							assert(high_mod->level==1);
							if(high_mod->data_mod==1&&high_mod->mod_id == stack->core_id)
							{	
								high_mod->pf_pollution_HPAC ++;
								break;
							}
						 }	
					}
				}		
				mod_block_set_owner(mod, stack->addr, -1);//cjc_20130418
			}
			esim_schedule_event(EV_MOD_NMOESI_EVICT, new_stack, 0);

			//IC_S_2012_12_27
			mod->cache_evict_block++;
			//IC_E


			if(mod->level==2&&((mod->cache_evict_block %mod->one_fourth_block_mask)==0))//cjc_20130128 for event-throttling
			{
				struct mod_t * high_mod;
				LINKED_LIST_FOR_EACH(mod->high_mod_list)
				{
					high_mod = linked_list_get(mod->high_mod_list);
					//if(high_mod->cache->throttle)
					assert(high_mod->level==1);
					if(high_mod->cache->prefetcher)
						esim_schedule_event(EV_THROTTLE_EVENT_TRIGGER, high_mod->cache->throttle,0);
				}
			}

			/*if(mod->level==1&&mod->cache->prefetcher&&((mod->cache_evict_block %mod->one_fourth_block_mask)==0))//cjc_20130717 for event-throttling
                        {
				esim_schedule_event(EV_THROTTLE_EVENT_TRIGGER, mod->cache->throttle,0);
			}*/



			//cjc_20121215 this block has no reuse by core or upper level
			//if( cache_get_dead_block_flag(mod->cache, stack->set, stack->way)==1)
			//IC_S_2012_12_27
			if( cache_get_dead_block_flag(mod->cache, stack->set, stack->way)==demand)
				//IC_E
			{
				//cache_set_dead_block_flag(mod->cache, stack->set, stack->way, 0);
				//IC_S_2012_12_27
				cache_set_dead_block_flag(mod->cache, stack->set, stack->way, reused);
				//IC_E
				mod->cache_dead_block_demand++;
				if(mod->kind==mod_kind_cache)//cjc_20121217 notify throttle
					esim_schedule_event(EV_THROTTLE_NEW_COUNTER, mod->cache->throttle,0);
				if(stack->prefetch)
					mod->cache_dead_block_demand_caused_by_pf++;
			}
			//IC_S_2012_12_27
			else if (cache_get_dead_block_flag(mod->cache, stack->set, stack->way)==prefetch)
			{
				cache_set_dead_block_flag(mod->cache, stack->set, stack->way, reused);
				mod->cache_dead_block_prefetch++;
				if(mod->kind==mod_kind_cache)
					esim_schedule_event(EV_THROTTLE_NEW_COUNTER, mod->cache->throttle,0);
			}
			//IC_E
			else if(cache_get_dead_block_flag(mod->cache, stack->set, stack->way)==reused)
			{

			}
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_FIND_AND_LOCK_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s find and lock finish (err=%d)\n", esim_cycle, stack->id,
				stack->tag, mod->name, stack->err);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:find_and_lock_finish\"\n",
				stack->id, mod->name);

		/* If evict produced err, return err */
		if (stack->err)
		{
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(stack->state);
			assert(stack->eviction);
			ret->err = 1;
			dir_entry_unlock(mod->dir, stack->set, stack->way);
			mod_stack_return(stack);
			return;
		}

		/* Eviction */
		if (stack->eviction)
		{
			mod->evictions++;
			cache_get_block(mod->cache, stack->set, stack->way, NULL, &stack->state);
			assert(!stack->state);
		}

		/* If this is a main memory, the block is here. A previous miss was just a miss
		 * in the directory. */
		if (mod->kind == mod_kind_main_memory && !stack->state)
		{
			stack->state = cache_block_exclusive;
			cache_set_block(mod->cache, stack->set, stack->way,
					stack->tag, stack->state);
		}


			
		
		/* Return */
		ret->err = 0;
		ret->set = stack->set;
		ret->way = stack->way;
		ret->state = stack->state;
		ret->tag = stack->tag;
		

		//cjc_20130317
		if(mod->kind == mod_kind_main_memory && mod->dram_controller && stack->skip_dram==0)
		{
			dram_set_drb_entry(stack);
		}	
		else
			mod_stack_return(stack);
		if(*addr_anal_file_name && esim_warm_up_finish && mod->kind == mod_kind_main_memory && (stack->demand_access||stack->prefetch))
		{
			int pc;
			if(stack->client_info)
			{
				pc = stack->client_info->prefetcher_eip;
				addr_anal_insert(stack->core_id,stack->addr,pc);
			}
			else if(stack->prefetch)
			{
				pc = stack->pc;
				addr_anal_insert(stack->core_id,stack->addr,pc);
			}
			else 
			{
				pc = -1;
				//fprintf(stderr,"no pc %lld\n",stack->id);
			}
			//addr_anal_insert(stack->core_id,stack->addr,pc);
		}

		if(*prefetch_stride_sequence_out&&stack->prefetch &&esim_warm_up_finish && mod->kind == mod_kind_main_memory)
		{
			fprintf(seq_file, "core %d cycles %lld pc %u addr %u\n",stack->core_id,esim_cycle,stack->pc,stack->addr);	

		}
		return;
	}

	abort();
}


void mod_handler_nmoesi_evict(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_NMOESI_EVICT)
	{
		/* Default return value */
		ret->err = 0;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		assert(stack->state || !dir_entry_group_shared_or_owned(mod->dir,
					stack->set, stack->way));
		mem_debug("  %lld %lld 0x%x %s evict (set=%d, way=%d, state=%s)\n", esim_cycle, stack->id,
				stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict\"\n",
				stack->id, mod->name);

		/* Save some data */
		stack->src_set = stack->set;
		stack->src_way = stack->way;
		stack->src_tag = stack->tag;
		stack->target_mod = mod_get_low_mod(mod, stack->tag);

		/* Send write request to all sharers */
		new_stack = mod_stack_create(stack->id, mod, 0, EV_MOD_NMOESI_EVICT_INVALID, stack);
		new_stack->except_mod = NULL;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		new_stack->prefetch = stack->prefetch;//cjc_20130502
		stack->prefetch = 0;//cjc_20130502
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_INVALID)
	{
		mem_debug("  %lld %lld 0x%x %s evict invalid\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_invalid\"\n",
				stack->id, mod->name);

		/* If module is main memory, we just need to set the block as invalid, 
		 * and finish. */
		if (mod->kind == mod_kind_main_memory)
		{
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
					0, cache_block_invalid);
			esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* Continue */
		esim_schedule_event(EV_MOD_NMOESI_EVICT_ACTION, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_ACTION)
	{
		struct mod_t *low_mod;
		struct net_node_t *low_node;
		int msg_size;

		mem_debug("  %lld %lld 0x%x %s evict action\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_action\"\n",
				stack->id, mod->name);

		/* Get low node */
		low_mod = stack->target_mod;
		low_node = low_mod->high_net_node;
		assert(low_mod != mod);
		assert(low_mod == mod_get_low_mod(mod, stack->tag));
		assert(low_node && low_node->user_data == low_mod);

		/* State = I */
		if (stack->state == cache_block_invalid)
		{
			esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
			return;
		}

		/* If state is M/O/N, data must be sent to lower level mod */
		if (stack->state == cache_block_modified || stack->state == cache_block_owned ||
				stack->state == cache_block_noncoherent)
		{
			/* Need to transmit data to low module */
			msg_size = 8 + mod->block_size;
			stack->reply = reply_ack_data;
		}
		/* If state is E/S, just an ack needs to be sent */
		else 
		{
			msg_size = 8;
			stack->reply = reply_ack;
		}
		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_EVICT_RECEIVE, stack, 0);
		else
		{
			/* Send message */
			stack->msg = net_try_send_ev(mod->low_net, mod->low_net_node,
					low_node, msg_size, EV_MOD_NMOESI_EVICT_RECEIVE, stack, event, stack);
		}
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict receive\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_receive\"\n",
				stack->id, target_mod->name);
		if(stack->ideal_access==0)//cjc_20121227 skip network-receiving
			/* Receive message */
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Find and lock */
		if (stack->state == cache_block_noncoherent)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
					EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT, stack);
		}
		else 
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->src_tag,
					EV_MOD_NMOESI_EVICT_PROCESS, stack);
		}

		/* FIXME It's not guaranteed to be a write */
		new_stack->blocking = 0;
		new_stack->write = 1;
		new_stack->retry = 0;
		new_stack->write_back =1;
		new_stack->skip_dram = (stack->reply == reply_ack)? 1:0;//cjc_20130328 this request need not to access dram
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS)
	{
		mem_debug("  %lld %lld 0x%x %s evict process\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process\"\n",
				stack->id, target_mod->name);

		/* Error locking block */
		if (stack->err)
		{
			ret->err = 1;
			esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack)
		{
			/* Nothing to do */
		}
		else if (stack->reply == reply_ack_data)
		{
			if (stack->state == cache_block_exclusive)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_modified);
			}
			else if (stack->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
						stack->state); //IC_S_2013_11_26
			}
		}
		else 
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
					stack->state); //IC_S_2013_11_26
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size || dir_entry_tag ==stack->tag);
			if (dir_entry_tag < stack->src_tag || 
					dir_entry_tag >= stack->src_tag + mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, stack->set, stack->way, z, 
					mod->low_net_node->index);
			if (dir_entry->owner == mod->low_net_node->index)
			{
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT)
	{
		mem_debug("  %lld %lld 0x%x %s evict process noncoherent\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_process_noncoherent\"\n",
				stack->id, target_mod->name);

		/* Error locking block */
		if (stack->err)
		{
			ret->err = 1;
			esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
			return;
		}

		/* If data was received, set the block to modified */
		if (stack->reply == reply_ack_data)
		{
			if (stack->state == cache_block_exclusive)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_modified);
			}
			else if (stack->state == cache_block_owned || 
					stack->state == cache_block_modified)
			{
				/* Nothing to do */
			}
			else if (stack->state == cache_block_shared ||
					stack->state == cache_block_noncoherent)
			{
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_noncoherent);
			}
			else
			{
				fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
						stack->state); //IC_S_2013_11_26
			}
		}
		else 
		{
			fatal("%s: Invalid cache block state: %d\n", __FUNCTION__, 
					stack->state); //IC_S_2013_11_26
		}

		/* Remove sharer and owner */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			/* Skip other subblocks */
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size || dir_entry_tag ==stack->tag);
			if (dir_entry_tag < stack->src_tag || 
					dir_entry_tag >= stack->src_tag + mod->block_size)
			{
				continue;
			}

			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_clear_sharer(dir, stack->set, stack->way, z, 
					mod->low_net_node->index);
			if (dir_entry->owner == mod->low_net_node->index)
			{
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
			}
		}

		/* Unlock the directory entry */
		dir = target_mod->dir;
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply\"\n",
				stack->id, target_mod->name);

		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_EVICT_REPLY_RECEIVE, stack, 0);
		else
		{
			/* Send message */
			stack->msg = net_try_send_ev(target_mod->high_net, target_mod->high_net_node,
					mod->low_net_node, 8, EV_MOD_NMOESI_EVICT_REPLY_RECEIVE, stack,
					event, stack);
		}
		return;

	}

	if (event == EV_MOD_NMOESI_EVICT_REPLY_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s evict reply receive\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_reply_receive\"\n",
				stack->id, mod->name);


		if(stack->ideal_access==0)//cjc_20121227 skip network-receiving
			/* Receive message */
			net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Invalidate block if there was no error. */
		if (!stack->err)
		{
			cache_set_block(mod->cache, stack->src_set, stack->src_way,
					0, cache_block_invalid);
			//cache_set_invalid_to_tail(mod->cache, stack->src_set, stack->src_way);//cjc_20130102		
		}	
		assert(!dir_entry_group_shared_or_owned(mod->dir, stack->src_set, stack->src_way));
		esim_schedule_event(EV_MOD_NMOESI_EVICT_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_EVICT_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s evict finish\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:evict_finish\"\n",
				stack->id, mod->name);

		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_read_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;
 	void *ret_stack = stack->ret_stack;//CIC 2013/11/11
	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag, z;

	if (event == EV_MOD_NMOESI_READ_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request\"\n",
				stack->id, mod->name);
		/* Default return values*/
		ret->shared = 0;
		ret->err = 0;
        //CIC  2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq = esim_cycle;
		//
		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
				stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == mod ||
				stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			if(mod->kind==mod_kind_main_memory)//cjc_20121221 disable memory coherence
			{
				mod_stack_return(stack);
				return;
			}
			net = mod->high_net;
			src_node = mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}

		if(stack->ideal_access)		//cjc_20121225 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_RECEIVE, stack, 0);
		else
		{
			/* Send message */
			stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
					EV_MOD_NMOESI_READ_REQUEST_RECEIVE, stack, event, stack);
		}	
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s read request receive\n", esim_cycle, stack->id,
				stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_receive\"\n",
				stack->id, target_mod->name);

		if(stack->ideal_access);         //cjc_20121225 skip network-receiving
		/* Receive message */
		else if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);
        
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_receive = esim_cycle;

		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
				EV_MOD_NMOESI_READ_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->read = 1;
		new_stack->retry = 0;
		if(stack->demand_access)
			new_stack->demand_access = 1;//cjc_20130311
		new_stack->prefetch = stack->prefetch;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s read request action\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_action\"\n",
				stack->id, target_mod->name);

		/* Check block locking error. If read request is down-up, there should not
		 * have been any error while locking. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_action = esim_cycle;

		esim_schedule_event(stack->request_dir == mod_request_up_down ?
				EV_MOD_NMOESI_READ_REQUEST_UPDOWN : EV_MOD_NMOESI_READ_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request updown\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown\"\n",
				stack->id, target_mod->name);

		stack->pending = 1;
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_upd = esim_cycle;

		/* Set the initial reply message and size.  This will be adjusted later if
		 * a transfer occur between peers. */
		stack->reply_size = mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);

		if (stack->state)
		{

			/* Status = M/O/E/S/N
			 * Check: address is a multiple of requester's block_size
			 * Check: no sub-block requested by mod is already owned by mod */
			assert(stack->addr % mod->block_size == 0);
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size|| dir_entry_tag ==stack->tag);
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
					continue;
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				assert(dir_entry->owner != mod->low_net_node->index);
				//IC_S_2013_11_13
				//if (mod->cache->prefetcher)
				//{
				//	if (mod->cache->prefetcher->L1_DCU_hint == 1)
				//	{
				//		;
				//	}
				//	else
				//	{
				//		assert(dir_entry->owner != mod->low_net_node->index);
				//	}
				//}
				//else
				//{
				//	assert(dir_entry->owner != mod->low_net_node->index);
				//}
				//IC_E
			}


			//cjc_20121211
			/*only works when  target_mod->block_size == target_mod->sub_block_size
eg: L1 blk size == L2 blk size,  L2 blk size == L3 blk size*/	
			if(target_mod->kind==mod_kind_cache)
				if(mod_block_get_prefetched(target_mod,stack->addr))
				{
					dir_entry = dir_entry_get(dir, stack->set, stack->way, 0);
					if(DIR_ENTRY_VALID_OWNER(dir_entry))
						target_mod->pf_hit_by_other_core++;
				}




			/* TODO If there is only sharers, should one of them
			 *      send the data to mod instead of having target_mod do it? */

			/* Send read request to owners other than mod for all sub-blocks. */
			for (z = 0; z < dir->zsize; z++)
			{
				struct net_node_t *node;

				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;

				/* No owner */
				if (!DIR_ENTRY_VALID_OWNER(dir_entry))
					continue;

				/* Owner is mod */
				if (dir_entry->owner == mod->low_net_node->index)
					continue;

				/* Get owner mod */
				node = list_get(target_mod->high_net->node_list, dir_entry->owner);
				assert(node->kind == net_node_end);
				owner = node->user_data;
				assert(owner);

				/* Not the first sub-block */
				if (dir_entry_tag % owner->block_size)
					continue;

				/* Send read request */
				stack->pending++;
				new_stack = mod_stack_create(stack->id, target_mod, dir_entry_tag,
						EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack);
				/* Only set peer if its a subblock that was requested */
				if (dir_entry_tag >= stack->addr && 
						dir_entry_tag < stack->addr + mod->block_size)
				{
					new_stack->peer = mod_stack_set_peer(mod, stack->state);
				}
				new_stack->target_mod = owner;
				new_stack->request_dir = mod_request_down_up;
				esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
			}
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);

			if (stack->prefetch&&target_mod->kind!=mod_kind_main_memory)
                        {
                                if(mod_block_get_owner(target_mod, stack-> addr)==stack->core_id)
                                {
                                        if(mod_block_get_prefetched(target_mod, stack->addr))
                                                target_mod->pf_in_cache_get_by_its_own_pf++;
                                        else
                                                target_mod->pf_in_cache_get_by_itself++;
                                }
                                else
                                {
                                        if(mod_block_get_prefetched(target_mod, stack->addr))
                                                target_mod->pf_in_cache_get_by_other_pf++;
                                        else
                                                target_mod->pf_in_cache_get_by_others++;
                                }
                        }
			
			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may have been a miss. 
			 * TODO: I'm not sure how relavant this is here for all states. */
			prefetcher_access_hit(stack, target_mod);
			//IC_S_2013_11_13
			//if (mod->cache->prefetcher)
			//{	
			//	if (mod->cache->prefetcher->L1_DCU_hint == 1)
			//	{
			//		;
			//	}
			//	else
			//	{
			//		prefetcher_access_hit(stack, target_mod);
			//	}
			//}
			//else
			//{
			//	prefetcher_access_hit(stack, target_mod);
			//}
			//IC_E

			//cjc_20121215 reuse
			//cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, 0);
			//IC_S_2012_12_27
			cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, reused);
			//IC_E

			//cjc_20130103
			if(stack->ideal_access)
				target_mod->pf_hits++;
			
		}
		else
		{
			//cjc_20121226 skip memory access
			if(stack->ideal_access&&((mod_get_low_mod(target_mod, stack->tag))->kind==mod_kind_main_memory))
			{
				esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS, stack, 0);
				stack->fetch_level=stack->fetch_level + 1;
				mod->pf_ideal_skipped_mem_access++;//cjc_20130104
				return;
			}
			/* State = I */
			assert(!dir_entry_group_shared_or_owned(target_mod->dir,
						stack->set, stack->way));
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS, stack);
			/* Peer is NULL since we keep going up-down */
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			new_stack->request_dir = mod_request_up_down;
			new_stack->prefetch = stack->prefetch;//cjc_20121207 bypass stack->prefetch
			if(stack->demand_access)
	                        new_stack->demand_access = 1;//cjc_20130311
			new_stack->fetch_level = stack->fetch_level + 1; //porshe_20121125 increas count to next level
			//new_stack->ideal_access=1;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);

			/* The prefetcher may be interested in this miss */
			prefetcher_access_miss(stack, target_mod);
			//IC_S_2013_11_13
			//if (mod->cache->prefetcher)
			//{
			//	if (mod->cache->prefetcher->L1_DCU_hint == 1)
			//	{
			//		;
			//	}
			//	else
			//	{
			//		prefetcher_access_miss(stack, target_mod);
			//	}
			//	
			//}
			//else
			//{
			//	prefetcher_access_miss(stack, target_mod);
			//}
			//IC_E

			//cjc_20121215 set dead-block flag
			//cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, 1);
			//IC_S_2012_12_27
			if (stack->prefetch)
			{
				cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, prefetch);

				mod->pf_issued_get_from_mem++;
			}
			else
			{
				cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, demand);
			}
			//IC_E

		}
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS)
	{
		mem_debug("  %lld %lld 0x%x %s read request updown miss\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_miss\"\n",
				stack->id, target_mod->name);
        ((struct mod_stack_t *)ret_stack)->read_rq_upd_miss = esim_cycle;
		/* Check error */
		if (stack->err)
		{
			dir_entry_unlock(target_mod->dir, stack->set, stack->way);
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, 0);
			return;
		}

		/* Set block state to excl/shared depending on the return value 'shared'
		 * that comes from a read request into the next cache level.
		 * Also set the tag of the block. */
		cache_set_block(target_mod->cache, stack->set, stack->way, stack->tag,
				stack->shared ? cache_block_shared : cache_block_exclusive);
		if(stack->prefetch)//cjc_20121207 insert current cycle
			cache_set_insert_cycle(target_mod->cache,stack->set,stack->way,esim_cycle);	
		if(target_mod->kind==mod_kind_cache)//cjc_20121207 set prefetched bit
		{
			mod_block_set_prefetched(target_mod, stack->addr,stack->prefetch);
			mod_block_set_pf_owner (target_mod, stack->addr,(mod->data_mod)?mod->mod_id:-1);//cjc_20130208 need not to keep owner
			mod_block_set_owner(target_mod, stack->addr, stack->core_id);//cjc_20130418
		}
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH)
	{
		int shared;

		/* Ensure that a reply was received */
		assert(stack->reply);
        
		
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_upd_finish = esim_cycle;
		//
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		/* Trace */
		mem_debug("  %lld %lld 0x%x %s read request updown finish\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_updown_finish\"\n",
				stack->id, target_mod->name);

		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data or simply ack */
		if (stack->reply_size == 8) 
		{
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->reply_size > 8)
		{
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else 
		{
			fatal("Invalid reply size: %d", stack->reply_size);
		}

		dir = target_mod->dir;

		shared = 0;
		/* With the Owned state, the directory entry may remain owned by the sender */
		if (!stack->retain_owner)
		{
			/* Set owner to 0 for all directory entries not owned by mod. */
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				if (dir_entry->owner != mod->low_net_node->index)
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
							DIR_ENTRY_OWNER_NONE);
			}
		}

		/* For each sub-block requested by mod, set mod as sharer, and
		 * check whether there is other cache sharing it. */
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			if (dir_entry->num_sharers > 1 || stack->nc_write || stack->shared)
				shared = 1;

			/* If the block is owned, non-coherent, or shared,  
			 * mod (the higher-level cache) should never be exclusive */
			if (stack->state == cache_block_owned || 
					stack->state == cache_block_noncoherent ||
					stack->state == cache_block_shared )
				shared = 1;
		}

		/* If no sub-block requested by mod is shared by other cache, set mod
		 * as owner of all of them. Otherwise, notify requester that the block is
		 * shared by setting the 'shared' return value to true. */
		ret->shared = shared;
		if (!shared)
		{
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
					continue;
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_set_owner(dir, stack->set, stack->way, z, mod->low_net_node->index);
			}
		}

		dir_entry_unlock(dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		latency = (stack->ideal_access)?0:latency;      //cjc_20121225 have no access latency
		//esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
                
		/*porshe_20130812: add for DRAMSIM2 module, go to dram access here.*/
		if(target_mod->kind == mod_kind_main_memory)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->addr, EV_MOD_NMOESI_READ_REQUEST_REPLY, stack);
			new_stack->read = 1;
			esim_schedule_event(EV_DRAM_ACCESS,new_stack,0);
		}
		else //end
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);

		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP)
	{
		struct mod_t *owner;

		mem_debug("  %lld %lld 0x%x %s read request downup\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup\"\n",
				stack->id, target_mod->name);


		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_dup = esim_cycle;

		/* Check: state must not be invalid or shared.
		 * By default, only one pending request.
		 * Response depends on state */
		assert(stack->state != cache_block_invalid);
		assert(stack->state != cache_block_shared);
		assert(stack->state != cache_block_noncoherent);
		stack->pending = 1;

		/* Send a read request to the owner of each subblock. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			struct net_node_t *node;

			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size|| dir_entry_tag ==stack->tag);
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);

			/* No owner */
			if (!DIR_ENTRY_VALID_OWNER(dir_entry))
				continue;

			/* Get owner mod */
			node = list_get(target_mod->high_net->node_list, dir_entry->owner);
			assert(node && node->kind == net_node_end);
			owner = node->user_data;

			/* Not the first sub-block */
			if (dir_entry_tag % owner->block_size)
				continue;

			stack->pending++;
			new_stack = mod_stack_create(stack->id, target_mod, dir_entry_tag,
					EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack);
			new_stack->target_mod = owner;
			new_stack->request_dir = mod_request_down_up;
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST, new_stack, 0);
		}

		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS)
	{
		/* Ignore while pending requests */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_wait_for_rqs = esim_cycle;
		
		mem_debug("  %lld %lld 0x%x %s read request downup wait for reqs\n", 
				esim_cycle, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_wait_for_reqs\"\n",
				stack->id, target_mod->name);

		if (stack->peer)
		{
			stack->peer_to_peer_trans_flag = 1; //porshe_20121128 init $2$ transfer flag
			/* Send this block (or subblock) to the peer */
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack);
			new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
			new_stack->target_mod = stack->target_mod;
			esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
		}
		else 
		{
			/* No data to send to peer, so finish */
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH, stack, 0);
		}

		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request downup finish\n", 
				esim_cycle, stack->id, stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_downup_finish\"\n",
				stack->id, target_mod->name);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_dup_finish = esim_cycle;
		
		if (stack->reply == reply_ack_data)
		{
			/* If data was received, it was owned or modified by a higher level cache.
			 * We need to continue to propagate it up until a peer is found */

			if (stack->peer) 
			{
				/* Peer was found, so this directory entry should be changed 
				 * to owned */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_owned);

				/* Higher-level cache changed to shared, set owner of 
				 * sub-blocks to NONE. */
				dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size || dir_entry_tag ==stack->tag);
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
							DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);

				/* Let the lower-level cache know not to delete the owner */
				ret->retain_owner = 1;
			}
			else
			{
				/* Set state to shared */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_shared);

				/* State is changed to shared, set owner of sub-blocks to 0. */
				dir = target_mod->dir;
				for (z = 0; z < dir->zsize; z++)
				{
					dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
					assert(dir_entry_tag < stack->tag + target_mod->block_size|| dir_entry_tag ==stack->tag);
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
							DIR_ENTRY_OWNER_NONE);
				}

				stack->reply_size = target_mod->block_size + 8;
				mod_stack_set_reply(ret, reply_ack_data);
			}
		}
		else if (stack->reply == reply_ack)
		{
			/* Higher-level cache was exclusive with no modifications above it */
			stack->reply_size = 8;

			/* Set state to shared */
			cache_set_block(target_mod->cache, stack->set, stack->way, 
					stack->tag, cache_block_shared);

			/* State is changed to shared, set owner of sub-blocks to 0. */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
				assert(dir_entry_tag < stack->tag + target_mod->block_size|| dir_entry_tag ==stack->tag);
				dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
				dir_entry_set_owner(dir, stack->set, stack->way, z, 
						DIR_ENTRY_OWNER_NONE);
			}

			if (stack->peer)
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->block_size;
				assert(ret->reply_size >= 8);
			}
			else
			{
				mod_stack_set_reply(ret, reply_ack);
				stack->reply_size = 8;
			}
		}
		else if (stack->reply == reply_none)
		{
			/* This block is not present in any higher level caches */

			if (stack->peer) 
			{
				stack->reply_size = 8;
				mod_stack_set_reply(ret, reply_ack_data_sent_to_peer);

				/* Decrease the amount of data that mod will have to send back
				 * to its higher level cache */
				ret->reply_size -= target_mod->sub_block_size;
				assert(ret->reply_size >= 8);

				if (stack->state == cache_block_modified || 
						stack->state == cache_block_owned)
				{
					/* Let the lower-level cache know not to delete the owner */
					ret->retain_owner = 1;

					/* Set block to owned */
					cache_set_block(target_mod->cache, stack->set, stack->way, 
							stack->tag, cache_block_owned);
				}
				else 
				{
					/* Set block to shared */
					cache_set_block(target_mod->cache, stack->set, stack->way, 
							stack->tag, cache_block_shared);
				}
			}
			else 
			{
				if (stack->state == cache_block_exclusive || 
						stack->state == cache_block_shared)
				{
					stack->reply_size = 8;
					mod_stack_set_reply(ret, reply_ack);

				}
				else if (stack->state == cache_block_owned ||
						stack->state == cache_block_modified || 
						stack->state == cache_block_noncoherent)
				{
					/* No peer exists, so data is returned to mod */
					stack->reply_size = target_mod->sub_block_size + 8;
					mod_stack_set_reply(ret, reply_ack_data);
				}
				else 
				{
					fatal("Invalid cache block state: %d\n", stack->state); //IC_S_2013_11_26
				}

				/* Set block to shared */
				cache_set_block(target_mod->cache, stack->set, stack->way, 
						stack->tag, cache_block_shared);
			}
		}
		else 
		{
			fatal("Unexpected reply type: %d\n", stack->reply);
		}


		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		latency = (stack->ideal_access)?0:latency;      //cjc_20121225 have no access latency
		esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s read request reply\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_reply\"\n",
				stack->id, target_mod->name);

		
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_reply = esim_cycle;
		
		/* Checks */
		assert(stack->reply_size);
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
				mod_get_low_mod(target_mod, stack->addr) == mod);

		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = mod->low_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = mod->high_net_node;
		}

		if(stack->ideal_access)         //cjc_20121225 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_READ_REQUEST_FINISH, stack, 0);	
		else
		{
			/* Send message */
			stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
					EV_MOD_NMOESI_READ_REQUEST_FINISH, stack, event, stack);
		}
		return;
	}

	if (event == EV_MOD_NMOESI_READ_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s read request finish\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:read_request_finish\"\n",
				stack->id, mod->name);
       
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->read_rq_finish = esim_cycle;
		
		if(stack->ideal_access);         //cjc_20121225 skip network-receiving
		/* Receive message */
		else if (stack->request_dir == mod_request_up_down)
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
		else
			net_receive(mod->high_net, mod->high_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_write_request(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

 	void *ret_stack = stack->ret_stack;//CIC 2013/11/13

	uint32_t dir_entry_tag, z;


	if (event == EV_MOD_NMOESI_WRITE_REQUEST)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request\n", esim_cycle, stack->id,
				stack->addr, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request\"\n",
				stack->id, mod->name);

		/* Default return values */
		ret->err = 0;

		/* For write requests, we need to set the initial reply size because
		 * in updown, peer transfers must be allowed to decrease this value
		 * (during invalidate). If the request turns out to be downup, then 
		 * these values will get overwritten. */
		stack->reply_size = mod->block_size + 8;
		mod_stack_set_reply(stack, reply_ack_data);
        
		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq = esim_cycle;

		/* Checks */
		assert(stack->request_dir);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
				stack->request_dir == mod_request_down_up);
		assert(mod_get_low_mod(target_mod, stack->addr) == mod ||
				stack->request_dir == mod_request_up_down);

		/* Get source and destination nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = mod->low_net_node;
			dst_node = target_mod->high_net_node;
		}
		else
		{
			if(mod->kind==mod_kind_main_memory)//cjc_20121221 disable memory coherence
			{	
				mod_stack_return(stack);
				return;
			}
			net = mod->high_net;
			src_node = mod->high_net_node;
			dst_node = target_mod->low_net_node;
		}
		if(stack->ideal_access)         //cjc_20121225 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE, stack, 0);	
		else
		{		
			/* Send message */

			stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
					EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE, stack, event, stack);
		}
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request receive\n", esim_cycle, stack->id,
				stack->addr, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_receive\"\n",
				stack->id, target_mod->name);

		if(stack->ideal_access);         //cjc_20121225 skip network-receviving
		/* Receive message */
		else if (stack->request_dir == mod_request_up_down)
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);
		else
			net_receive(target_mod->low_net, target_mod->low_net_node, stack->msg);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_receive = esim_cycle;
		
		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
				EV_MOD_NMOESI_WRITE_REQUEST_ACTION, stack);
		new_stack->blocking = stack->request_dir == mod_request_down_up;
		new_stack->write = 1;
		new_stack->retry = 0;
		if(stack->demand_access)
                        new_stack->demand_access = 1;//cjc_20130311
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s write request action\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_action\"\n",
				stack->id, target_mod->name);

		/* Check lock error. If write request is down-up, there should
		 * have been no error. */
		if (stack->err)
		{
			assert(stack->request_dir == mod_request_up_down);
			ret->err = 1;
			stack->reply_size = 8;
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_action = esim_cycle;
		
		/* Invalidate the rest of upper level sharers */
		new_stack = mod_stack_create(stack->id, target_mod, 0,
				EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE, stack);
		new_stack->except_mod = mod;
		new_stack->set = stack->set;
		new_stack->way = stack->way;
		new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE)
	{
		mem_debug("  %lld %lld 0x%x %s write request exclusive\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_exclusive\"\n",
				stack->id, target_mod->name);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_exclusive = esim_cycle;
		
		if (stack->request_dir == mod_request_up_down)
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN, stack, 0);
		else
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown\"\n",
				stack->id, target_mod->name);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_upd = esim_cycle;
		
		
		/* state = M/E */
		if (stack->state == cache_block_modified ||
				stack->state == cache_block_exclusive)
		{
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);

			//cjc_20130103
			if(stack->ideal_access)
				target_mod->pf_hits++;
		}
		/* state = O/S/I/N */
		else if (stack->state == cache_block_owned || stack->state == cache_block_shared ||
				stack->state == cache_block_invalid || stack->state == cache_block_noncoherent)
		{

			//cjc_20121226 skip memory access
			if(stack->ideal_access&&(mod_get_low_mod(target_mod, stack->tag))->kind==mod_kind_main_memory)
			{
				stack->fetch_level=stack->fetch_level + 1;
				esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack, 0);
				mod->pf_ideal_skipped_mem_access++;//cjc_20130104
				return;
			}
			new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
					EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH, stack);
			new_stack->peer = mod_stack_set_peer(mod, stack->state);
			new_stack->target_mod = mod_get_low_mod(target_mod, stack->tag);
			if(stack->demand_access)
                        	new_stack->demand_access = 1;//cjc_20130311
			new_stack->fetch_level = stack->fetch_level + 1; //porshe_20121125 increas count to next level
			new_stack->request_dir = mod_request_up_down;
			//new_stack->ideal_access=1;
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);

			if (stack->state == cache_block_invalid)
			{
				/* The prefetcher may be interested in this miss */
				prefetcher_access_miss(stack, target_mod);
				//IC_S_2013_11_13
				//if (mod->cache->prefetcher)
				//{
				//	if (mod->cache->prefetcher->L1_DCU_hint == 1)
				//	{
				//		;
				//	}
				//	else
				//	{
				//		prefetcher_access_miss(stack, target_mod);
				//	}
				//}
				//else
				//{
				//	prefetcher_access_miss(stack, target_mod);
				//}
				//IC_E

				//20121215
				//cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, 1);
				//IC_S_2012_12_27
				cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, demand);
				//IC_E
			}
		}
		else 
		{
			fatal("Invalid cache block state: %d\n", stack->state); //IC_S_2013_11_26
		}

		if (stack->state != cache_block_invalid)
		{
			/* The prefetcher may have prefetched this earlier and hence
			 * this is a hit now. Let the prefetcher know of this hit
			 * since without the prefetcher, this may been a miss. 
			 * TODO: I'm not sure how relavant this is here for all states. */
			prefetcher_access_hit(stack, target_mod);
			//IC_S_2013_11_13
			//if (mod->cache->prefetcher)
			//{
			//	if (mod->cache->prefetcher->L1_DCU_hint == 1)
			//	{
			//		;
			//	}
			//	else
			//	{
			//		prefetcher_access_hit(stack, target_mod);
			//	}
			//}
			//else
			//{
			//	prefetcher_access_hit(stack, target_mod);
			//}
			//IC_E

			//cjc_20121215
			//cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, 0);
			//IC_S_2012_12_27
			cache_set_dead_block_flag(target_mod->cache, stack->set, stack->way, reused);
			//IC_E
		}

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request updown finish\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_updown_finish\"\n",
				stack->id, target_mod->name);

		/* Ensure that a reply was received */
		assert(stack->reply);

		/* Error in write request to next cache level */
		if (stack->err)
		{
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			stack->reply_size = 8;
			dir_entry_unlock(target_mod->dir, stack->set, stack->way);
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, 0);
			return;
		}

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_upd_finish = esim_cycle;
		
		
		/* Check that addr is a multiple of mod.block_size.
		 * Set mod as sharer and owner. */
		dir = target_mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			assert(stack->addr % mod->block_size == 0);
			dir_entry_tag = stack->tag + z * target_mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + target_mod->block_size|| dir_entry_tag ==stack->tag);
			if (dir_entry_tag < stack->addr || dir_entry_tag >= stack->addr + mod->block_size)
				continue;
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			dir_entry_set_sharer(dir, stack->set, stack->way, z, mod->low_net_node->index);
			dir_entry_set_owner(dir, stack->set, stack->way, z, mod->low_net_node->index);
			assert(dir_entry->num_sharers == 1);
		}

		/* Set state to exclusive */
		cache_set_block(target_mod->cache, stack->set, stack->way,
				stack->tag, cache_block_exclusive);
		mod_block_set_owner(target_mod, stack->addr, stack->core_id);//cjc_20130418
		
		/* If blocks were sent directly to the peer, the reply size would
		 * have been decreased.  Based on the final size, we can tell whether
		 * to send more data up or simply ack */
		if (stack->reply_size == 8) 
		{
			mod_stack_set_reply(ret, reply_ack);
		}
		else if (stack->reply_size > 8)
		{
			mod_stack_set_reply(ret, reply_ack_data);
		}
		else 
		{
			fatal("Invalid reply size: %d", stack->reply_size);
		}

		/* Unlock, reply_size is the data of the size of the requester's block. */
		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		latency = (stack->ideal_access)?0:latency;      //cjc_20121225 have no access latency
		/*porshe_20130812: add for DRAMSIM2 module, go to dram access here.*/
		if(target_mod->kind == mod_kind_main_memory)
		{
			new_stack = mod_stack_create(stack->id, target_mod, stack->addr, EV_MOD_NMOESI_READ_REQUEST_REPLY, stack);
			new_stack->write = 1;
			esim_schedule_event(EV_DRAM_ACCESS,new_stack,0);
		}
		else //end
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);

		//esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup\"\n",
				stack->id, target_mod->name);

		assert(stack->state != cache_block_invalid);
		assert(!dir_entry_group_shared_or_owned(target_mod->dir, stack->set, stack->way));

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_dup = esim_cycle;
		
		
		/* Compute reply size */	
		if (stack->state == cache_block_exclusive || 
				stack->state == cache_block_shared) 
		{
			/* Exclusive and shared states send an ack */
			stack->reply_size = 8;
			mod_stack_set_reply(stack, reply_ack);
		}
		else if (stack->state == cache_block_noncoherent)
		{
			/* Non-coherent state sends data */
			stack->reply_size = target_mod->block_size + 8;
			mod_stack_set_reply(stack, reply_ack_data);
		}
		else if (stack->state == cache_block_modified || 
				stack->state == cache_block_owned)
		{
			if (stack->peer) 
			{
				stack->peer_to_peer_trans_flag = 1; //porshe_20121128 init $2$ transfer flag
				/* Modified or owned entries send data directly to peer 
				 * if it exists */
				mod_stack_set_reply(stack, reply_ack_data_sent_to_peer);
				stack->reply_size = 8;

				/* This control path uses an intermediate stack that disappears, so 
				 * we have to update the return stack of the return stack */
				ret->ret_stack->reply_size -= target_mod->block_size;
				assert(ret->ret_stack->reply_size >= 8);

				/* Send data to the peer */
				new_stack = mod_stack_create(stack->id, target_mod, stack->tag,
						EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack);
				new_stack->peer = mod_stack_set_peer(stack->peer, stack->state);
				new_stack->target_mod = stack->target_mod;

				esim_schedule_event(EV_MOD_NMOESI_PEER_SEND, new_stack, 0);
				return;
			}	
			else 
			{
				/* If peer does not exist, data is returned to mod */
				mod_stack_set_reply(stack, reply_ack_data);
				stack->reply_size = target_mod->block_size + 8;
			}
		}
		else 
		{
			fatal("Invalid cache block state: %d\n", stack->state); //IC_S_2013_11_26
		}

		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request downup complete\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_downup_finish\"\n",
				stack->id, target_mod->name);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_dup_finish = esim_cycle;
		
		/* Set state to I, unlock*/
		cache_set_block(target_mod->cache, stack->set, stack->way, 0, cache_block_invalid);
		cache_set_invalid_to_tail(target_mod->cache, stack->set, stack->way);//cjc_20130102
		dir_entry_unlock(target_mod->dir, stack->set, stack->way);

		int latency = stack->reply == reply_ack_data_sent_to_peer ? 0 : target_mod->latency;
		latency = (stack->ideal_access)?0:latency;	//cjc_20121225 have no access latency
		esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_REPLY, stack, latency);
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s write request reply\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_reply\"\n",
				stack->id, target_mod->name);

		/* Checks */
		assert(stack->reply_size);
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
				mod_get_low_mod(target_mod, stack->addr) == mod);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_reply = esim_cycle;
		
		/* Get network and nodes */
		if (stack->request_dir == mod_request_up_down)
		{
			net = mod->low_net;
			src_node = target_mod->high_net_node;
			dst_node = mod->low_net_node;
		}
		else
		{
			net = mod->high_net;
			src_node = target_mod->low_net_node;
			dst_node = mod->high_net_node;
		}
		if(stack->ideal_access)         //cjc_20121225 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST_FINISH, stack, 0);	
		else
		{
			stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
					EV_MOD_NMOESI_WRITE_REQUEST_FINISH, stack, event, stack);
		}
		return;
	}

	if (event == EV_MOD_NMOESI_WRITE_REQUEST_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s write request finish\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:write_request_finish\"\n",
				stack->id, mod->name);

		//CIC 2013/11/11
		((struct mod_stack_t *)ret_stack)->write_rq_finish = esim_cycle;
		
		if(stack->ideal_access);         //cjc_20121225 skip network-reveiving
		/* Receive message */
		else if (stack->request_dir == mod_request_up_down)
			net_receive(mod->low_net, mod->low_net_node, stack->msg);
		else
			net_receive(mod->high_net, mod->high_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_peer(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_t *src = stack->target_mod;
	struct mod_t *peer = stack->peer;

	if (event == EV_MOD_NMOESI_PEER_SEND) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer send\n", esim_cycle, stack->id,
				stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer\"\n",
				stack->id, src->name);

		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_PEER_RECEIVE, stack, 0);
		else
		{
			/* Send message from src to peer */
			stack->msg = net_try_send_ev(src->low_net, src->low_net_node, peer->low_net_node, 
					src->block_size + 8, EV_MOD_NMOESI_PEER_RECEIVE, stack, event, stack);
		}
		return;
	}

	if (event == EV_MOD_NMOESI_PEER_RECEIVE) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer receive\n", esim_cycle, stack->id,
				stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_receive\"\n",
				stack->id, peer->name);

		if(stack->ideal_access==0)//cjc_20121227 skip network-receiving
		{
			/* Receive message from src */
			net_receive(peer->low_net, peer->low_net_node, stack->msg);
		}
		esim_schedule_event(EV_MOD_NMOESI_PEER_REPLY, stack, 0);

		return;
	}

	if (event == EV_MOD_NMOESI_PEER_REPLY) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer reply ack\n", esim_cycle, stack->id,
				stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_reply_ack\"\n",
				stack->id, peer->name);
		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_PEER_FINISH, stack, 0);
		else
		{
			/* Send ack from peer to src */
			stack->msg = net_try_send_ev(peer->low_net, peer->low_net_node, src->low_net_node, 
					8, EV_MOD_NMOESI_PEER_FINISH, stack, event, stack); 
		}
		return;
	}

	if (event == EV_MOD_NMOESI_PEER_FINISH) 
	{
		mem_debug("  %lld %lld 0x%x %s %s peer finish\n", esim_cycle, stack->id,
				stack->tag, src->name, peer->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:peer_finish\"\n",
				stack->id, src->name);

		if(stack->ideal_access==0)//cjc_20121227 skip network-receiving
		{
			/* Receive message from src */
			net_receive(src->low_net, src->low_net_node, stack->msg);
		}
		mod_stack_return(stack);
		return;
	}

	abort();
}


void mod_handler_nmoesi_invalidate(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;

	uint32_t dir_entry_tag;
	uint32_t z;

	if (event == EV_MOD_NMOESI_INVALIDATE)
	{
		struct mod_t *sharer;
		int i;

		/* Get block info */
		cache_get_block(mod->cache, stack->set, stack->way, &stack->tag, &stack->state);
		mem_debug("  %lld %lld 0x%x %s invalidate (set=%d, way=%d, state=%s)\n", esim_cycle, stack->id,
				stack->tag, mod->name, stack->set, stack->way,
				str_map_value(&cache_block_state_map, stack->state));
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate\"\n",
				stack->id, mod->name);

		/* At least one pending reply */
		stack->pending = 1;

		/* Send write request to all upper level sharers except 'except_mod' */
		dir = mod->dir;
		for (z = 0; z < dir->zsize; z++)
		{
			dir_entry_tag = stack->tag + z * mod->sub_block_size;
			assert(dir_entry_tag < stack->tag + mod->block_size || dir_entry_tag ==stack->tag);
			dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
			for (i = 0; i < dir->num_nodes; i++)
			{
				struct net_node_t *node;

				/* Skip non-sharers and 'except_mod' */
				if (!dir_entry_is_sharer(dir, stack->set, stack->way, z, i))
					continue;

				node = list_get(mod->high_net->node_list, i);
				sharer = node->user_data;
				
				if (sharer == stack->except_mod)
					continue;

				/* Clear sharer and owner */
				dir_entry_clear_sharer(dir, stack->set, stack->way, z, i);
				if (dir_entry->owner == i)
					dir_entry_set_owner(dir, stack->set, stack->way, z, DIR_ENTRY_OWNER_NONE);

				/* Send write request upwards if beginning of block */
				if (dir_entry_tag % sharer->block_size)
					continue;
				new_stack = mod_stack_create(stack->id, mod, dir_entry_tag,
						EV_MOD_NMOESI_INVALIDATE_FINISH, stack);
				new_stack->target_mod = sharer;
				
				new_stack->request_dir = mod_request_down_up;

				esim_schedule_event(EV_MOD_NMOESI_WRITE_REQUEST, new_stack, 0);
				stack->pending++;


			}
		}
		if(stack->pending>1&&stack->prefetch&&mod->level==2)//cjc_20130502 this eviction caused by prefetch trigger an invalidation 
		{
			struct net_node_t *node;
			for (i = 0; i < dir->num_nodes; i++)
			{
				node = list_get(mod->high_net->node_list, i);
				sharer = node->user_data;
				if(!sharer)
					continue;
				if(sharer->data_mod&&sharer->mod_id==stack->core_id)
				{
					sharer->pf_downup_invalidation++;
					break;
				}
			}
		}
		esim_schedule_event(EV_MOD_NMOESI_INVALIDATE_FINISH, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_INVALIDATE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s invalidate finish\n", esim_cycle, stack->id,
				stack->tag, mod->name);
		mem_trace("mem.access name=\"A-%lld\" state=\"%s:invalidate_finish\"\n",
				stack->id, mod->name);

		/* Ignore while pending */
		assert(stack->pending > 0);
		stack->pending--;
		if (stack->pending)
			return;
		mod_stack_return(stack);
		return;
	}

	abort();
}

void mod_handler_nmoesi_message(int event, void *data)
{
	struct mod_stack_t *stack = data;
	struct mod_stack_t *ret = stack->ret_stack;
	struct mod_stack_t *new_stack;

	struct mod_t *mod = stack->mod;
	struct mod_t *target_mod = stack->target_mod;

	struct dir_t *dir;
	struct dir_entry_t *dir_entry;
	uint32_t z;

	if (event == EV_MOD_NMOESI_MESSAGE)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message\n", esim_cycle, stack->id,
				stack->addr, mod->name);

		stack->reply_size = 8;
		stack->reply = reply_ack;

		/* Default return values*/
		ret->err = 0;

		/* Checks */
		assert(stack->message);

		/* Get source and destination nodes */
		net = mod->low_net;
		src_node = mod->low_net_node;
		dst_node = target_mod->high_net_node;

		/* Send message */
		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE_RECEIVE, stack, 0);
		else
			stack->msg = net_try_send_ev(net, src_node, dst_node, 8,
					EV_MOD_NMOESI_MESSAGE_RECEIVE, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_RECEIVE)
	{
		mem_debug("  %lld %lld 0x%x %s message receive\n", esim_cycle, stack->id,
				stack->addr, target_mod->name);

		/* Receive message */
		if(stack->ideal_access==0)//cjc_20121227 skip network-receiving
			net_receive(target_mod->high_net, target_mod->high_net_node, stack->msg);

		/* Find and lock */
		new_stack = mod_stack_create(stack->id, target_mod, stack->addr,
				EV_MOD_NMOESI_MESSAGE_ACTION, stack);
		new_stack->message = stack->message;
		new_stack->blocking = 0;
		new_stack->retry = 0;
		esim_schedule_event(EV_MOD_NMOESI_FIND_AND_LOCK, new_stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_ACTION)
	{
		mem_debug("  %lld %lld 0x%x %s clear owner action\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);

		assert(stack->message);

		/* Check block locking error. */
		mem_debug("stack err = %u\n", stack->err);
		if (stack->err)
		{
			ret->err = 1;
			mod_stack_set_reply(ret, reply_ack_error);
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
			return;
		}

		if (stack->message == message_clear_owner)
		{
			/* Remove owner */
			dir = target_mod->dir;
			for (z = 0; z < dir->zsize; z++)
			{
				/* Skip other subblocks */
				if (stack->addr == stack->tag + z * target_mod->sub_block_size)
				{
					/* Clear the owner */
					dir_entry = dir_entry_get(dir, stack->set, stack->way, z);
					assert(dir_entry->owner == mod->low_net_node->index);
					dir_entry_set_owner(dir, stack->set, stack->way, z, 
							DIR_ENTRY_OWNER_NONE);
				}
			}

		}
		else
		{
			fatal("Unexpected message");
		}

		/* Unlock the directory entry */
		dir_entry_unlock(dir, stack->set, stack->way);

		esim_schedule_event(EV_MOD_NMOESI_MESSAGE_REPLY, stack, 0);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_REPLY)
	{
		struct net_t *net;
		struct net_node_t *src_node;
		struct net_node_t *dst_node;

		mem_debug("  %lld %lld 0x%x %s message reply\n", esim_cycle, stack->id,
				stack->tag, target_mod->name);

		/* Checks */
		assert(mod_get_low_mod(mod, stack->addr) == target_mod ||
				mod_get_low_mod(target_mod, stack->addr) == mod);

		/* Get network and nodes */
		net = mod->low_net;
		src_node = target_mod->high_net_node;
		dst_node = mod->low_net_node;

		/* Send message */
		if(stack->ideal_access)//cjc_20121227 skip network-sending
			esim_schedule_event(EV_MOD_NMOESI_MESSAGE_FINISH, stack, 0);
		else
			stack->msg = net_try_send_ev(net, src_node, dst_node, stack->reply_size,
					EV_MOD_NMOESI_MESSAGE_FINISH, stack, event, stack);
		return;
	}

	if (event == EV_MOD_NMOESI_MESSAGE_FINISH)
	{
		mem_debug("  %lld %lld 0x%x %s message finish\n", esim_cycle, stack->id,
				stack->tag, mod->name);


		if(stack->ideal_access==0)
			/* Receive message */
			net_receive(mod->low_net, mod->low_net_node, stack->msg);

		/* Return */
		mod_stack_return(stack);
		return;
	}

	abort();
}
