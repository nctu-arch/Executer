/*
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

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/repos.h>
#include <network/network.h>
#include <network/node.h>
#include <config/config.h>
#include <arch/x86/timing/uop.h> //porshe_20131025: include this head file is for virtual address check

#include "cache.h"
#include "directory.h"
#include "local-mem-protocol.h"
#include "mem-system.h"
#include "mod-stack.h"
#include "nmoesi-protocol.h"
#include "prefetcher.h"//cjc_20130208
#include "dram.h"//cjc_20130315


/* String map for access type */
struct str_map_t mod_access_kind_map =
{
	//4, {    //CIC  change 3 -> 4 
	//	{ "Load", mod_access_load },
	//	{ "Store", mod_access_store },
	//	{ "NCStore", mod_access_nc_store },
	//	{ "Prefetch", mod_access_prefetch }
	//}
	//IC_S_2013_11_13
	5, {     
		{ "Load", mod_access_load },
		{ "Store", mod_access_store },
		{ "NCStore", mod_access_nc_store },
		{ "Prefetch", mod_access_prefetch },
		{ "L1DCUHint", mod_access_L1DCUHint}
	}
	//IC_E_

};




/*
 * Public Functions
 */

struct mod_t *mod_create(char *name, enum mod_kind_t kind, int num_ports,
	int block_size, int latency)
{
	struct mod_t *mod;

	/* Initialize */
	mod = xcalloc(1, sizeof(struct mod_t));
	mod->name = xstrdup(name);
	mod->kind = kind;
	mod->latency = latency;

	/* Ports */
	mod->num_ports = num_ports;
	mod->ports = xcalloc(num_ports, sizeof(struct mod_port_t));

	/* Lists */
	mod->low_mod_list = linked_list_create();
	mod->high_mod_list = linked_list_create();

	/* Block size */
	mod->block_size = block_size;
	assert(!(block_size & (block_size - 1)) && block_size >= 4);
	mod->log_block_size = log_base2(block_size);

	mod->client_info_repos = repos_create(sizeof(struct mod_client_info_t), mod->name);
	
	mod->R_hit_on_self_l1=0;
        mod->R_hit_on_remote_l1=0;
        mod->R_hit_on_l2=0;
        mod->R_hit_on_memory=0;
        mod->W_hit_on_self_l1=0;
        mod->W_hit_on_remote_l1=0;
        mod->W_hit_on_l2=0;
        mod->W_hit_on_memory=0;
	mod->acc_waiting_oi_num = 0;
	mod->acc_waiting_oi_cycle = 0;

	mod->cache_dead_block_demand_caused_by_pf=0;
	return mod;
}


void mod_free(struct mod_t *mod)
{
	linked_list_free(mod->low_mod_list);
	linked_list_free(mod->high_mod_list);
	if (mod->cache)
	{
		cache_free(mod->cache);
		if(*prefetch_stride_table_ref_output)
		{
			prefetcher_dump_stride_table(mod,ref_file,-1,1);
		}
	}
	if(mod->pf_cache)//cjc_20121225 free pf_cache
		cache_free(mod->pf_cache);
	if (mod->dir)
		dir_free(mod->dir);
	free(mod->ports);
	repos_free(mod->client_info_repos);
	
	if(mod->dram_controller)
		dram_free(mod->dram_controller);
	
	free(mod->name);
	free(mod);
}


void mod_dump(struct mod_t *mod, FILE *f)
{
}


/* Access a memory module.
 * Variable 'witness', if specified, will be increased when the access completes.
 * The function returns a unique access ID.
 */
long long mod_access(struct mod_t *mod, enum mod_access_kind_t access_kind, 
	unsigned int addr, int *witness_ptr, struct linked_list_t *event_queue,
	void *event_queue_item, struct mod_client_info_t *client_info)
{
	struct mod_stack_t *stack;
	int event;
	
	//porshe_20131025: TLB access on L1 D$
	unsigned int vlt_address;
	int set, way, state, hit, tag;
	set = way = state = 0;
	if(event_queue_item && mod->tlb)
	{
		vlt_address = (*(struct x86_uop_t *)event_queue_item).uinst->address;
		mod->tlb_access++;
		hit = cache_find_block(mod->tlb, vlt_address, &set, &way, &state); //TLB lookup
		if(hit)
		{
			cache_access_block(mod->tlb, set, way); //update LRU of hit
			//IC_S_2013_10_29
			//if (stack->client_info)
			//{
			//	mem_debug_2("  %lld %lld PC 0x%x virtual_address 0x%x phiscal_address 0x%x %s tlb_hit\n",
			//			esim_cycle, stack->id, stack->pc, vlt_address, addr, mod->name);
			//}
			//else if(stack->prefetch)
			//{
			//	mem_debug_2("  %lld %lld PC 0x%x virtual_address 0x%x phiscal_address 0x%x %s tlb_hit\n",
			//			esim_cycle, stack->id, stack->pc, vlt_address, addr, mod->name);
			//}
			//IC_E

		} else {
			way = cache_replace_block(mod->tlb, set); //update LRU of miss
			cache_decode_address(mod->tlb, vlt_address, NULL, &tag, NULL);//calculate the tag
			cache_set_block(mod->tlb, set, way, tag, cache_block_exclusive);//update tag to block
			mod->tlb_miss++;
			//IC_S_2013_10_29 if you do not profiling, mark it
			//if (stack->client_info)
			//{
			//	mem_debug_2("  %lld %lld PC 0x%x virtual_address 0x%x phiscal_address 0x%x %s tlb_miss\n",
			//			esim_cycle, stack->id, stack->pc, vlt_address, addr, mod->name);
			//
			//}
			//else if (stack->prefetch)
			//{
			//	mem_debug_2("  %lld %lld PC 0x%x virtual_address 0x%x phiscal_address 0x%x %s tlb_miss\n",
			//			esim_cycle, stack->id, stack->pc, vlt_address, addr, mod->name);
			//}
			//IC_E

		}
	}
	//end
	
	/* Create module stack with new ID */
	mod_stack_id++;
        stack = mod_stack_create(mod_stack_id,
               mod, addr, ESIM_EV_NONE, NULL);
	/*
	//cjc_20130208_start
	{
		struct mod_t *target_mod;
		target_mod=pf_find_destination(addr , mod);

		mod_stack_id++;
		assert(target_mod);
		stack = mod_stack_create(mod_stack_id,
				target_mod, addr, ESIM_EV_NONE, NULL);

		if(target_mod->data_mod!=1)
			stack->pf_source = mod->mod_id;
		else
			stack->pf_source = -1;
	}
	//cjc_20130208_end
	*/


	/* Initialize */
	stack->witness_ptr = witness_ptr;
	stack->event_queue = event_queue;
	stack->event_queue_item = event_queue_item;
	stack->client_info = client_info;

	/* Select initial CPU/GPU event */
	if (mod->kind == mod_kind_cache || mod->kind == mod_kind_main_memory)
	{
		if (access_kind == mod_access_load)
		{
			event = EV_MOD_NMOESI_LOAD;
		}
		else if (access_kind == mod_access_store)
		{
			event = EV_MOD_NMOESI_STORE;
		}
		else if (access_kind == mod_access_nc_store)
		{
			event = EV_MOD_NMOESI_NC_STORE;
		}
		else if (access_kind == mod_access_prefetch)
		{
			event = EV_MOD_NMOESI_PREFETCH;
			stack->pc = (client_info)?client_info->prefetcher_eip:0;
			stack->stream_entry_id = (client_info)?client_info->stream_entry_id:0;
			stack->low_insert = (client_info)?client_info->lower_level_insertion:0;
			stack->client_info = NULL;
		}

		//IC_S_2013_11_13
		else if (access_kind == mod_access_L1DCUHint)
		{
			//just like mod_access_prefetch
			//but only give hint to L2 prefetcher
			event = EV_MOD_NMOESI_L1DCUHINT;
			stack->pc = (client_info)?client_info->prefetcher_eip:0;
			stack->stream_entry_id = (client_info)?client_info->stream_entry_id:0;
			stack->low_insert = (client_info)?client_info->lower_level_insertion:0;
			stack->client_info = NULL;
		}
		//IC_E
		else 
		{
			panic("%s: invalid access kind", __FUNCTION__);
		}
	}
	else if (mod->kind == mod_kind_local_memory)
	{
		if (access_kind == mod_access_load)
		{
			event = EV_MOD_LOCAL_MEM_LOAD;
		}
		else if (access_kind == mod_access_store)
		{
			event = EV_MOD_LOCAL_MEM_STORE;
		}
		else
		{
			panic("%s: invalid access kind", __FUNCTION__);
		}
	}
	else
	{
		panic("%s: invalid mod kind", __FUNCTION__);
	}

	/* Schedule */
	esim_execute_event(event, stack);

	/* Return access ID */
	return stack->id;
}


/* Return true if module can be accessed. */
int mod_can_access(struct mod_t *mod, unsigned int addr)
{
	int non_coalesced_accesses;

	/* There must be a free port */
	assert(mod->num_locked_ports <= mod->num_ports);
	if (mod->num_locked_ports == mod->num_ports)
		return 0;

	/* If no MSHR is given, module can be accessed */
	if (!mod->mshr_size)
		return 1;

	/* Module can be accessed if number of non-coalesced in-flight
	 * accesses is smaller than the MSHR size. */
	non_coalesced_accesses = mod->access_list_count -
		mod->access_list_coalesced_count;

	if(non_coalesced_accesses >= mod->mshr_size) mod->mshr_block++;

	return non_coalesced_accesses < mod->mshr_size;
}

//IC_S_2014_07_15
/* Return true if module can be accessed. */
int mod_prefetch_can_access(struct mod_t *mod, unsigned int addr)
{
	int non_coalesced_accesses;

	/* There must be a free port */
	assert(mod->num_locked_ports <= mod->num_ports);
	if (mod->num_locked_ports == mod->num_ports)
		return 0;

	/* If no MSHR is given, module can be accessed */
	if (!mod->mshr_size)
		return 1;

	/* Module can be accessed if number of non-coalesced in-flight
	 * accesses is smaller than the MSHR size. */
	non_coalesced_accesses = mod->access_list_count -
		mod->access_list_coalesced_count;

	if(non_coalesced_accesses >= mod->mshr_size) mod->prefetch_mshr_block++;
	
	if ( (non_coalesced_accesses < mod->mshr_size) && (mod->prefetch_inflight_list_count <= 8) )
	{
		mod->prefetch_inflight_list_count++;
	}

	return ( (non_coalesced_accesses < mod->mshr_size) && (mod->prefetch_inflight_list_count <= 8) );
}
//IC_E

//This function is checking if MSHR have empty slots for access or not
//beware of port checking if you want to use this function independently
//return 1 if mshr is free return 0 if full
int mod_mshr_can_access(struct mod_t *mod, unsigned int addr)
{
	int non_coalesced_accesses;

	if (!mod->mshr_size)
		return 1;

	/* Module can be accessed if number of non-coalesced in-flight
	 * accesses is smaller than the MSHR size. */
	non_coalesced_accesses = mod->access_list_count - mod->access_list_coalesced_count;
	return non_coalesced_accesses < mod->mshr_size;
}


/* Return {set, way, tag, state} for an address.
 * The function returns TRUE on hit, FALSE on miss. */
int mod_find_block(struct mod_t *mod, unsigned int addr, int *set_ptr,
		int *way_ptr, unsigned int *tag_ptr, int *state_ptr)
{
	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	struct dir_lock_t *dir_lock;

	int set;
	int way;
	unsigned int tag;

	/* A transient tag is considered a hit if the block is
	 * locked in the corresponding directory. */
	tag = addr & ~cache->block_mask;
	if (mod->range_kind == mod_range_interleaved)
	{
		unsigned int num_mods = mod->range.interleaved.mod;
		set = ((tag >> cache->log_block_size) / num_mods) % cache->num_sets;
	}
	else if (mod->range_kind == mod_range_bounds)
	{
		//printf("log_block_size = %d, num_sets= %d\n",cache->log_block_size,cache->num_sets);
		set = (tag >> cache->log_block_size) % cache->num_sets;
	}
	else 
	{
		panic("%s: invalid range kind (%d)", __FUNCTION__, mod->range_kind);
	}

	for (way = 0; way < cache->assoc; way++)
	{
		blk = &cache->sets[set].blocks[way];
		if (blk->tag == tag && blk->state)
			break;
		if (blk->transient_tag == tag)
		{
			dir_lock = dir_lock_get(mod->dir, set, way);
			if (dir_lock->lock)
				break;
		}
	}

	PTR_ASSIGN(set_ptr, set);
	PTR_ASSIGN(tag_ptr, tag);

	/* Miss */
	if (way == cache->assoc)
	{
		/*
		   PTR_ASSIGN(way_ptr, 0);
		   PTR_ASSIGN(state_ptr, 0);
		 */
		return 0;
	}

	/* Hit */
	PTR_ASSIGN(way_ptr, way);
	PTR_ASSIGN(state_ptr, cache->sets[set].blocks[way].state);
	return 1;
}

void mod_block_set_prefetched(struct mod_t *mod, unsigned int addr, int val)
{
	int set, way;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	//if (mod->cache->prefetcher && mod_find_block(mod, addr, &set, &way, NULL, NULL))
	if (mod_find_block(mod, addr, &set, &way, NULL, NULL))//cjc_20121211 remove condition "mod->cache->prefetcher"
	{
		mod->cache->sets[set].blocks[way].prefetched = val;
	}
}

int mod_block_get_prefetched(struct mod_t *mod, unsigned int addr)
{
	int set, way;

	assert(mod->kind == mod_kind_cache && mod->cache != NULL);
	//if (mod->cache->prefetcher && mod_find_block(mod, addr, &set, &way, NULL, NULL))
	if (mod_find_block(mod, addr, &set, &way, NULL, NULL))//cjc_20121211 remove condition "mod->cache->prefetcher"
	{
		return mod->cache->sets[set].blocks[way].prefetched;
	}

	return 0;
}

/* Lock a port, and schedule event when done.
 * If there is no free port, the access is enqueued in the port
 * waiting list, and it will retry once a port becomes available with a
 * call to 'mod_unlock_port'. */
void mod_lock_port(struct mod_t *mod, struct mod_stack_t *stack, int event)
{
	struct mod_port_t *port = NULL;
	int i;

	/* No free port */
	if (mod->num_locked_ports >= mod->num_ports)
	{
		assert(!DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));

		/* If the request to lock the port is down-up, give it priority since 
		 * it is possibly holding up a large portion of the memory hierarchy */
		if (stack->request_dir == mod_request_down_up)
		{
			DOUBLE_LINKED_LIST_INSERT_HEAD(mod, port_waiting, stack);
		}
		else 
		{
			DOUBLE_LINKED_LIST_INSERT_TAIL(mod, port_waiting, stack);
		}
		stack->port_waiting_list_event = event;
		return;
	}

	/* Get free port */
	for (i = 0; i < mod->num_ports; i++)
	{
		port = &mod->ports[i];
		if (!port->stack)
			break;
	}

	/* Lock port */
	assert(port && i < mod->num_ports);
	port->stack = stack;
	stack->port = port;
	mod->num_locked_ports++;

	/* Debug */
	mem_debug("  %lld stack %lld %s port %d locked\n", esim_cycle, stack->id, mod->name, i);

	/* Schedule event */
	esim_schedule_event(event, stack, 0);
}


void mod_unlock_port(struct mod_t *mod, struct mod_port_t *port,
		struct mod_stack_t *stack)
{
	int event;

	/* Checks */
	assert(mod->num_locked_ports > 0);
	assert(stack->port == port && port->stack == stack);
	assert(stack->mod == mod);

	/* Unlock port */
	stack->port = NULL;
	port->stack = NULL;
	mod->num_locked_ports--;

	/* Debug */
	mem_debug("  %lld %lld %s port unlocked\n", esim_cycle,
			stack->id, mod->name);

	/* Check if there was any access waiting for free port */
	if (!mod->port_waiting_list_count)
		return;

	/* Wake up one access waiting for a free port */
	stack = mod->port_waiting_list_head;
	event = stack->port_waiting_list_event;
	assert(DOUBLE_LINKED_LIST_MEMBER(mod, port_waiting, stack));
	DOUBLE_LINKED_LIST_REMOVE(mod, port_waiting, stack);
	mod_lock_port(mod, stack, event);

}


void mod_access_start(struct mod_t *mod, struct mod_stack_t *stack,
		enum mod_access_kind_t access_kind)
{
	int index;
	//CIC  
    stack->mod_start_access_cycle = esim_cycle;
	stack->mshr_access_count = mod->access_list_count - mod->access_list_coalesced_count; 
	/* Record access kind */
	stack->access_kind = access_kind;

	/* Insert in access list */
	DOUBLE_LINKED_LIST_INSERT_TAIL(mod, access, stack);

	/* Insert in write access list */
	if (access_kind == mod_access_store)
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, write_access, stack);
	if (access_kind == mod_access_prefetch )
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, prefetch_access,stack);
	//IC_S_2013_11_13
	if (access_kind == mod_access_L1DCUHint )
		DOUBLE_LINKED_LIST_INSERT_TAIL(mod, prefetch_access,stack);
	//IC_E

	/* Insert in access hash table */
	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_INSERT_TAIL(&mod->access_hash_table[index], bucket, stack);
}


void mod_access_finish(struct mod_t *mod, struct mod_stack_t *stack)
{
	int index;
#ifdef PRINT_DETAIL
    printf("name: %s  kind: %d  start:  %-6lld  ending: %-6lld  access t:  %-4lld  level:%d  mshr: %-2d  addr: %-3x  ",mod->name,stack->access_kind,stack->mod_start_access_cycle,esim_cycle,esim_cycle - stack->mod_start_access_cycle,stack->fetch_level,stack->mshr_access_count,(stack->addr >> 6));
   
   
	if (stack->access_kind != mod_access_load &&stack->access_kind != mod_access_store )printf("\n");
	if (stack->coalesced) printf(" [ ");
#endif
	if (stack->access_kind == mod_access_load){
#ifdef PRINT_DETAIL
			printf("star: %-5ld load_lock: %-5ld find_lock: %-5ld action: %-5ld miss_rt:%-5ld  ulock:%-5ld fish:%-5ld wait:%-5ld",stack->load_start,stack->load_lock,stack->load_find_n_lock,stack->load_action,stack->load_miss_get,stack->load_unlock,stack->load_finish,stack->waiting_oldwrite_inflight_cycle);
#endif	
	mod->acc_load_access_time += esim_cycle - stack->mod_start_access_cycle;
	mod->acc_load_find_n_lock += stack->load_find_n_lock - stack->store_lock;
	mod->acc_load_L1_L2_net   += stack->read_rq_receive - stack->read_rq;
	mod->acc_load_L2_access   += stack->read_rq_reply - stack->read_rq_upd_finish;
	mod->acc_load_reply       += stack->read_rq_finish - stack->read_rq_reply;
	if (stack->old_write ||stack->inflight_ad)
	{
			mod->acc_load_wait_old_write += 1;
			mod->acc_load_wait_old_inflight_cycle += stack->waiting_oldwrite_inflight_cycle;
	}
	if(stack->fetch_level == 2)
		mod->acc_load_MM_access +=stack->read_rq_upd_miss - stack->read_rq_upd;
	if(stack->peer_to_peer_trans_flag == 1) mod->acc_load_peer_to_peer += stack->read_rq_upd_finish - stack->read_rq_upd;
	}
	else
	{
#ifdef PRINT_DETAIL
			printf("star: %-5ld store_lock: %-5ld store_lock: %-5ld action: %-5ld miss_rt:%-5ld  ulock:%-5ld fish:%-5ld wait:%-5ld",stack->store_start,stack->store_lock,stack->store_find_n_lock,stack->store_action,stack->store_miss_get,stack->store_unlock,stack->store_finish,stack->waiting_oldwrite_inflight_cycle);
#endif
		mod->acc_store_access_time += esim_cycle - stack->mod_start_access_cycle;
		mod->acc_store_find_n_lock += stack->store_find_n_lock - stack->store_lock;
		mod->acc_store_L1_L2_net   += stack->write_rq_receive - stack->write_rq;
		mod->acc_store_L2_access   += stack->write_rq_reply - stack->write_rq_upd_finish;
		mod->acc_store_exclusive   += stack->write_rq_exclusive - stack->write_rq_action;
		mod->acc_store_reply       += stack->write_rq_finish - stack->write_rq_reply;
		if (stack->wait_store)
		{
			mod->acc_store_wait_old_access += 1;
			mod->acc_store_wait_old_inflight_cycle += stack->waiting_oldwrite_inflight_cycle;
		}
		if(stack->fetch_level == 2)
		mod->acc_store_MM_access +=stack->write_rq_upd_finish - stack->write_rq_upd;
		
//	if(stack->peer_to_peer_trans_flag == 1) acc_load_peer_to_peer += stack->read_rq_upd_finish - stack->read_rq_upd;
	
	}
#ifdef PRINT_DETAIL
	if (stack->old_write) printf(" old_write"); 
	if (stack->wait_store) printf(" old_access"); 
	if (stack->inflight_ad) printf(" inflight_ad"); 
	if (stack->coalesced) printf(" ] \n");
	else printf("\n");

#endif
  //  if(stack->fetch_level >= 1 && stack->access_kind == 1)
//	{
	//		if(stack->peer_to_peer_trans_flag == 0)
//			printf("name: %s  kind: %d  start:  %-6lld Read_rq: %-5lld rq_rece: %-5lld rq_action: %-5lld rq_upd: %-5lld rq_upd_miss: %-5lld rq_upd_fin: %-5lld rq_reply: %-5lld rq_fin: %-5lld   \n",mod->name,stack->access_kind,stack->mod_start_access_cycle,stack->read_rq,stack->read_rq_receive,stack->read_rq_action,stack->read_rq_upd,stack->read_rq_upd_miss,stack->read_rq_upd_finish,stack->read_rq_reply,stack->read_rq_finish);
//			else
//			printf("name: %s  kind: %d  start:  %-6lld Read_rq: %-5lld rq_rece: %-5lld rq_action: %-5lld rq_dup: %-5lld rq_wait_f_rqs: %-5lld rq_dup_fin: %-5lld rq_reply: %-5lld rq_fin: %-5lld\n",mod->name,stack->access_kind,stack->mod_start_access_cycle,stack->read_rq,stack->read_rq_receive,stack->read_rq_action,stack->read_rq_dup,stack->read_rq_wait_for_rqs,stack->read_rq_dup_finish,stack->read_rq_reply,stack->read_rq_finish);
//			if(stack->peer_to_peer_trans_flag == 1) printf("peer to peer" );
//	printf("-------------------------------------------------------------------------------------------\n");
//	}
  //  if(stack->fetch_level >= 1 && stack->access_kind == 2)
//	{
	//		if(stack->peer_to_peer_trans_flag == 0)
//			printf("name: %s  kind: %d  start:  %-6lld wrt_rq: %-5lld wrt_rece: %-5lld wrt_action: %-5lld wrt_exclu:%-5lld  wrt_upd: %-5lld wrt_upd_fin: %-5lld wrt_reply: %-5lld wrt_fin: %-5lld\n",mod->name,stack->access_kind,stack->mod_start_access_cycle,stack->write_rq,stack->write_rq_receive,stack->write_rq_action,stack->write_rq_exclusive,stack->write_rq_upd,stack->write_rq_upd_finish,stack->write_rq_reply,stack->write_rq_finish);
//			else
//			printf("name: %s  kind: %d  start:  %-6lld wrt_rq: %-5lld wrt_rece: %-5lld rq_action: %-5lld wrt_rq_exclu:%-5lld wrt_dup: %-5lld rq_dup_fin: %-5lld rq_reply: %-5lld rq_fin: %-5lld\n",mod->name,stack->access_kind,stack->mod_start_access_cycle,stack->write_rq,stack->write_rq_receive,stack->write_rq_action,stack->write_rq_exclusive,stack->write_rq_dup,stack->write_rq_dup_finish,stack->write_rq_reply,stack->write_rq_finish);
//			if(stack->peer_to_peer_trans_flag == 1) printf("peer to peer" );
//	printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
//	}
	
	/* Remove from access list */
	DOUBLE_LINKED_LIST_REMOVE(mod, access, stack);

	/* Remove from write access list */
	assert(stack->access_kind);
	if (stack->access_kind == mod_access_store)
		DOUBLE_LINKED_LIST_REMOVE(mod, write_access, stack);
	if (stack->access_kind == mod_access_prefetch )
		DOUBLE_LINKED_LIST_REMOVE(mod,prefetch_access,stack);
	//IC_S_2013_11_13
	if (stack->access_kind == mod_access_L1DCUHint )
		DOUBLE_LINKED_LIST_REMOVE(mod,prefetch_access,stack);
	//IC_E




    //stack->mod_start_access_cycle = esim_cycle;
/* Remove from hash table */

	index = (stack->addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	DOUBLE_LINKED_LIST_REMOVE(&mod->access_hash_table[index], bucket, stack);

	/* If this was a coalesced access, update counter */
	if (stack->coalesced)
	{
		assert(mod->access_list_coalesced_count > 0);
		mod->access_list_coalesced_count--;
	}

	//IC_S_2014_07_15
	if (stack->prefetch == 1)
	{
		mod->prefetch_inflight_list_count--;
	}
	//IC_E
}


/* Return true if the access with identifier 'id' is in flight.
 * The address of the access is passed as well because this lookup is done on the
 * access truth table, indexed by the access address.
 */
int mod_in_flight_access(struct mod_t *mod, long long id, unsigned int addr)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for access */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack; stack = stack->bucket_list_next)
		if (stack->id == id)
			return 1;

	/* Not found */
	return 0;
}


/* Return the youngest in-flight access older than 'older_than_stack' to block containing 'addr'.
 * If 'older_than_stack' is NULL, return the youngest in-flight access containing 'addr'.
 * The function returns NULL if there is no in-flight access to block containing 'addr'.
 */
struct mod_stack_t *mod_in_flight_address(struct mod_t *mod, unsigned int addr,
		struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	int index;

	/* Look for address */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack;
			stack = stack->bucket_list_next)
	{
		/* This stack is not older than 'older_than_stack' */
		if (older_than_stack && stack->id >= older_than_stack->id)
			continue;

		/* Address matches */
		if (stack->addr >> mod->log_block_size == addr >> mod->log_block_size)
			return stack;
	}

	/* Not found */
	return NULL;
}

//porshe_20130820
/* Return the oldest in-flight access to block containing 'addr'.
 * The function returns NULL if there is no in-flight access to block containing 'addr'.
 */
struct mod_stack_t *mod_in_flight_oldest_address(struct mod_t *mod, unsigned int addr)
{
	struct mod_stack_t *stack;
	struct mod_stack_t *oldest_stack = NULL;
	int index;
	int min_id = 0;
	/* Look for address */
	index = (addr >> mod->log_block_size) % MOD_ACCESS_HASH_TABLE_SIZE;
	for (stack = mod->access_hash_table[index].bucket_list_head; stack;
		stack = stack->bucket_list_next)
	{

		/* Address matches */
		if (stack->addr >> mod->log_block_size == addr >> mod->log_block_size && 
			(!min_id || (stack->id < min_id)))
		{
			oldest_stack = stack;
			min_id = stack->id;
		}
	}

	return oldest_stack;
}

/* Return the youngest in-flight write older than 'older_than_stack'. If 'older_than_stack'
 * is NULL, return the youngest in-flight write. Return NULL if there is no in-flight write.
 */
struct mod_stack_t *mod_in_flight_write(struct mod_t *mod,
		struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;

	/* No 'older_than_stack' given, return youngest write */
	if (!older_than_stack)
		return mod->write_access_list_tail;

	/* Search */
	for (stack = older_than_stack->access_list_prev; stack;
			stack = stack->access_list_prev)
		if (stack->access_kind == mod_access_store)
			return stack;

	/* Not found */
	return NULL;
}


int mod_serves_address(struct mod_t *mod, unsigned int addr)
{
	/* Address bounds */
	if (mod->range_kind == mod_range_bounds)
		return addr >= mod->range.bounds.low &&
			addr <= mod->range.bounds.high;

	/* Interleaved addresses */
	if (mod->range_kind == mod_range_interleaved)
		return (addr / mod->range.interleaved.div) %
			mod->range.interleaved.mod ==
			mod->range.interleaved.eq;

	/* Invalid */
	panic("%s: invalid range kind", __FUNCTION__);
	return 0;
}


/* Return the low module serving a given address. */
struct mod_t *mod_get_low_mod(struct mod_t *mod, unsigned int addr)
{
	struct mod_t *low_mod;
	struct mod_t *server_mod;

	/* Main memory does not have a low module */
	assert(mod_serves_address(mod, addr));
	if (mod->kind == mod_kind_main_memory)
	{
		assert(!linked_list_count(mod->low_mod_list));
		return NULL;
	}

	/* Check which low module serves address */
	server_mod = NULL;
	LINKED_LIST_FOR_EACH(mod->low_mod_list)
	{
		/* Get new low module */
		low_mod = linked_list_get(mod->low_mod_list);
		if (!mod_serves_address(low_mod, addr))
			continue;

		/* Address served by more than one module */
		if (server_mod)
			fatal("%s: low modules %s and %s both serve address 0x%x",
					mod->name, server_mod->name, low_mod->name, addr);

		/* Assign server */
		server_mod = low_mod;
	}

	/* Error if no low module serves address */
	if (!server_mod)
		fatal("module %s: no lower module serves address 0x%x",
				mod->name, addr);

	/* Return server module */
	return server_mod;
}


int mod_get_retry_latency(struct mod_t *mod)
{
	return random() % mod->latency + mod->latency;
}


/* Check if an access to a module can be coalesced with another access older
 * than 'older_than_stack'. If 'older_than_stack' is NULL, check if it can
 * be coalesced with any in-flight access.
 * If it can, return the access that it would be coalesced with. Otherwise,
 * return NULL. */
struct mod_stack_t *mod_can_coalesce(struct mod_t *mod,
		enum mod_access_kind_t access_kind, unsigned int addr,
		struct mod_stack_t *older_than_stack)
{
	struct mod_stack_t *stack;
	struct mod_stack_t *tail;

	/* For efficiency, first check in the hash table of accesses
	 * whether there is an access in flight to the same block. */
	assert(access_kind);
	if (!mod_in_flight_address(mod, addr, older_than_stack))
		return NULL;

	/* Get youngest access older than 'older_than_stack' */
	tail = older_than_stack ? older_than_stack->access_list_prev :
		mod->access_list_tail;

	/* Coalesce depending on access kind */
	switch (access_kind)
	{

		case mod_access_load:
			{
				for (stack = tail; stack; stack = stack->access_list_prev)
				{
					/* Only coalesce with groups of reads or prefetches at the tail */
					if (stack->access_kind != mod_access_load &&
							stack->access_kind != mod_access_prefetch)
						return NULL;
					//IC_S_2013_11_13
					if (stack->access_kind != mod_access_load &&
							stack->access_kind != mod_access_L1DCUHint)
						return NULL;
					//IC_E

					if (stack->addr >> mod->log_block_size ==
							addr >> mod->log_block_size)
						return stack->master_stack ? stack->master_stack : stack;
				}
				break;
			}

		case mod_access_store:
			{
				/* Only coalesce with last access */
				stack = tail;
				if (!stack)
					return NULL;

				/* Only if it is a write */
				if (stack->access_kind != mod_access_store)
					return NULL;

				/* Only if it is an access to the same block */
				if (stack->addr >> mod->log_block_size != addr >> mod->log_block_size)
					return NULL;

				/* Only if previous write has not started yet */
				if (stack->port_locked)
					return NULL;

				/* Coalesce */
				return stack->master_stack ? stack->master_stack : stack;
			}

		case mod_access_nc_store:
			{
				/* Only coalesce with last access */
				stack = tail;
				if (!stack)
					return NULL;

				/* Only if it is a non-coherent write */
				if (stack->access_kind != mod_access_nc_store)
					return NULL;

				/* Only if it is an access to the same block */
				if (stack->addr >> mod->log_block_size != addr >> mod->log_block_size)
					return NULL;

				/* Only if previous write has not started yet */
				if (stack->port_locked)
					return NULL;

				/* Coalesce */
				return stack->master_stack ? stack->master_stack : stack;
			}
		case mod_access_prefetch:
			/* At this point, we know that there is another access (load/store)
			 * to the same block already in flight. Just find and return it.
			 * The caller may abort the prefetch since the block is already
			 * being fetched. */
			for (stack = tail; stack; stack = stack->access_list_prev)
			{
				if (stack->addr >> mod->log_block_size ==
						addr >> mod->log_block_size)
					             return stack;                
			
			}
			assert(!"Hash table wrongly reported another access to same block.\n");
			break;

		//IC_S_2013_11_13	
		case mod_access_L1DCUHint:
			/* At this point, we know that there is another access (load/store)
			 * to the same block already in flight. Just find and return it.
			 * The caller may abort the prefetch since the block is already
			 * being fetched. */
			for (stack = tail; stack; stack = stack->access_list_prev)
			{
				if (stack->addr >> mod->log_block_size ==
						addr >> mod->log_block_size)
					             return stack;                
			
			}
			assert(!"Hash table wrongly reported another access to same block.\n");
			break;
		//IC_E


		default:
			panic("%s: invalid access type", __FUNCTION__);
			break;
	}

	/* No access found */
	return NULL;
}


void mod_coalesce(struct mod_t *mod, struct mod_stack_t *master_stack,
		struct mod_stack_t *stack)
{
	/* Debug */
	mem_debug("  %lld %lld 0x%x %s coalesce with %lld\n", esim_cycle,
			stack->id, stack->addr, mod->name, master_stack->id);

	/* Master stack must not have a parent. We only want one level of
	 * coalesced accesses. */
	assert(!master_stack->master_stack);

	/* Access must have been recorded already, which sets the access
	 * kind to a valid value. */
	assert(stack->access_kind);

	/* Set slave stack as a coalesced access */
	stack->coalesced = 1;
	stack->master_stack = master_stack;
	assert(mod->access_list_coalesced_count <= mod->access_list_count);

	/* Record in-flight coalesced access in module */
	mod->access_list_coalesced_count++;
}

struct mod_client_info_t *mod_client_info_create(struct mod_t *mod)
{
	struct mod_client_info_t *client_info;

	/* Create object */
	client_info = repos_create_object(mod->client_info_repos);

	/* Return */
	return client_info;
}

void mod_client_info_free(struct mod_t *mod, struct mod_client_info_t *client_info)
{
	repos_free_object(mod->client_info_repos, client_info);
}
//cjc_20121207 return current esim_cycle
long long mod_current_esim_cycle(void)
{
	return esim_cycle;
}
//cjc_20121211
void mod_cal_pf_pollution(struct mod_t *mod, int set, unsigned int tag)
{

	struct cache_t *cache = mod->cache;
	struct cache_block_t *blk;
	int way;
	for (way = 0; way < cache->assoc; way++)
	{
		blk = &cache->sets[set].blocks[way];
		if(blk->prefetched!=0&&blk->victim_tag==tag)
			mod->pf_pollution++;
	}
	return;
}

void mod_block_set_pf_owner(struct mod_t *mod, unsigned int addr, int val)
{
        int set, way;

        assert(mod->kind == mod_kind_cache && mod->cache != NULL);
        if (mod_find_block(mod, addr, &set, &way, NULL, NULL))
        {
                mod->cache->sets[set].blocks[way].pf_owner = val;
        }
}

int mod_block_get_pf_owner(struct mod_t *mod, unsigned int addr)
{
        int set, way;

        assert(mod->kind == mod_kind_cache && mod->cache != NULL);
        if (mod_find_block(mod, addr, &set, &way, NULL, NULL))
        {
                return mod->cache->sets[set].blocks[way].pf_owner;
        }

        return -1;
}

void mod_block_set_owner(struct mod_t *mod, unsigned int addr, int val)
{
        int set, way;

        //assert(mod->kind == mod_kind_cache && mod->cache != NULL);
        if (mod_find_block(mod, addr, &set, &way, NULL, NULL))
        {
                mod->cache->sets[set].blocks[way].owner = val;
        }
}

int mod_block_get_owner(struct mod_t *mod, unsigned int addr)
{
        int set, way;

        //assert(mod->kind == mod_kind_cache && mod->cache != NULL);
        if (mod_find_block(mod, addr, &set, &way, NULL, NULL))
        {
                return mod->cache->sets[set].blocks[way].owner;
        }

        return -1;
}

void mod_clear_pf_bits(struct mod_t *mod)
{
	struct cache_t *cache = mod->cache;
	unsigned int set,way;
	if(!cache)
		return;
	for(set = 0; set < cache->num_sets; set++)
		for(way = 0; way < cache->assoc; way++)
		{
			cache->sets[set].blocks[way].prefetched = 0;
			cache->sets[set].blocks[way].dead_block_state = reused;
		}
}

void mod_dump_cache_coherence_state(struct mod_t *mod)
{
	struct cache_t *cache = mod->cache;
	struct dir_t *dir = mod->dir;
	struct mod_t *first_mod;	

	assert(cache);
	assert(dir);
        int set,way;
        int i,j;
	int sharer,total_sharing,first_sharer,total_sharing_wo_icache;
        //int state;
        int counter[10];
	int sharing_counter[10];
        set = cache->num_sets;
        way = cache->assoc;
	total_sharing = 0;
	total_sharing_wo_icache = 0;
        for(i = 0; i <10;i++)
        {
                counter[i] = 0;
		sharing_counter[i] = 0;
        }

        for(i = 0; i < set;i++)
        {
                for(j = 0;j < way;j++)
                {
			//sharing = 0;
			/*for (z = 0; z < dir->zsize; z++)
			{
				sharing|=dir_entry_is_sharing(dir,i,j,z);
			}*/
			sharer = dir_entry_get_sharer_count(dir,i,j,0);
			//sharing_counter[cache->sets[i].blocks[j].state] += (sharer > 1)?1:0;
			//total_sharing += (sharer > 1)?sharer:0;
                        counter[cache->sets[i].blocks[j].state]++;

			if(sharer > 1)
			{
				first_sharer = dir_entry_get_the_first_sharer (dir,i,j,0);	
				sharing_counter[cache->sets[i].blocks[j].state] += 1;
                        	total_sharing += sharer;
				//linked_list_goto(mod->high_mod_list,first_sharer);
				
				LINKED_LIST_FOR_EACH(mod->high_mod_list)
				{	
					first_mod = linked_list_get(mod->high_mod_list);
					if(first_mod->data_mod&& first_mod->low_net_node->index==first_sharer)//this mod is dcache
					{
						total_sharing_wo_icache +=sharer;
						break;
					}
				}
			}
		}
	}

	cache_debug("%s_(I,N,M,O,E,S,T)= %d %d %d %d %d %d %d\n",mod->name,counter[0],counter[1],counter[2],counter[3],counter[4],counter[5],counter[6]);
	cache_debug("%s_sharing = %d %d %d %d %d %d %d %d %d\n",mod->name,sharing_counter[0],sharing_counter[1],sharing_counter[2],sharing_counter[3],
			sharing_counter[4],sharing_counter[5],sharing_counter[6],total_sharing,total_sharing_wo_icache);

}

struct mod_t *mod_get_main_memory(struct mod_t *mod) //cjc_20130422
{
	//assume we have a centralized main memory (only one memory module)	
	struct mod_t * target;

	target = mod;

	while(target)
	{
		if(target->kind==mod_kind_main_memory)
		{
			return target;
		}
		target = mod_get_low_mod(target, 1);
	}	
	return NULL;
}

/*struct mod_t *mod_get_LLC(struct mod_t *mod)
  {
  assert(mod->kind==mod_kind_main_memory);

  struct mod_t * high_mod;
  }*/
