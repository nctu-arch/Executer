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


#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <lib/util/linked-list.h>
#include <network/network.h>

#include "cache.h"
#include "command.h"
#include "config.h"
#include "local-mem-protocol.h"
#include "mem-system.h"
#include "dram-handler.h" //porshe_20130812 dram handler
#include "module.h"
#include "nmoesi-protocol.h"
#include "throttle.h"//cjc_20121217
#include "dram.h"//cjc_20130317
#include "prefetcher.h"//cjc_20130513

#include "address-analysis.h"
/*
 * Global Variables
 */

int mem_debug_category;
//IC_S_2013_10_29
int mem_debug_category_2;
int mem_debug_2_start;
//IC_E
int mem_trace_category;
int mem_system_peer_transfers;

struct mem_system_t *mem_system;

char *mem_report_file_name = "";
char *mem_profiling_file_name = ""; //porshe:20130220 mem profile report

struct mod_t * memory_mod;
/*
 * Public Functions
 */

void mem_system_init(void)
{
	//IC_S_2013_09_24
	//fp = file_open_for_write("./strideresult");
	//IC_E
	
	/* Trace */
	mem_trace_category = trace_new_category();

	/* Try to open report file */
	if (*mem_report_file_name && !file_can_open_for_write(mem_report_file_name))
		fatal("%s: cannot open GPU cache report file",
			mem_report_file_name);

	if (*mem_profiling_file_name && !file_can_open_for_write(mem_profiling_file_name))
		fatal("%s: cannot open GPU cache report file",
			mem_profiling_file_name);

	if (*addr_anal_file_name && !file_can_open_for_write(addr_anal_file_name))
		fatal("%s: cannot open CPU address report file",
                        addr_anal_file_name);

	/* Initialize */
	mem_system = xcalloc(1, sizeof(struct mem_system_t));
	mem_system->net_list = list_create();
	mem_system->mod_list = list_create();

	/* Event handler for memory hierarchy commands */
	EV_MEM_SYSTEM_COMMAND = esim_register_event_with_name(mem_system_command_handler, "mem_system_command");
	EV_MEM_SYSTEM_END_COMMAND = esim_register_event_with_name(mem_system_end_command_handler, "mem_system_end_command");

	/* NMOESI memory event-driven simulation */

	EV_MOD_NMOESI_LOAD = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load");
	EV_MOD_NMOESI_LOAD_LOCK = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load_lock");
	EV_MOD_NMOESI_LOAD_ACTION = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load_action");
	EV_MOD_NMOESI_LOAD_MISS = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load_miss");
	EV_MOD_NMOESI_LOAD_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load_unlock");
	EV_MOD_NMOESI_LOAD_FINISH = esim_register_event_with_name(mod_handler_nmoesi_load, "mod_nmoesi_load_finish");

	EV_MOD_NMOESI_STORE = esim_register_event_with_name(mod_handler_nmoesi_store, "mod_nmoesi_store");
	EV_MOD_NMOESI_STORE_LOCK = esim_register_event_with_name(mod_handler_nmoesi_store, "mod_nmoesi_store_lock");
	EV_MOD_NMOESI_STORE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_store, "mod_nmoesi_store_action");
	EV_MOD_NMOESI_STORE_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_store, "mod_nmoesi_store_unlock");
	EV_MOD_NMOESI_STORE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_store, "mod_nmoesi_store_finish");
	
	EV_MOD_NMOESI_NC_STORE = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store");
	EV_MOD_NMOESI_NC_STORE_LOCK = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_lock");
	EV_MOD_NMOESI_NC_STORE_WRITEBACK = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_writeback");
	EV_MOD_NMOESI_NC_STORE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_action");
	EV_MOD_NMOESI_NC_STORE_MISS= esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_miss");
	EV_MOD_NMOESI_NC_STORE_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_unlock");
	EV_MOD_NMOESI_NC_STORE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_nc_store, "mod_nmoesi_nc_store_finish");

	EV_MOD_NMOESI_PREFETCH = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch");
	EV_MOD_NMOESI_PREFETCH_LOCK = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_lock");
	EV_MOD_NMOESI_PREFETCH_ACTION = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_action");
	EV_MOD_NMOESI_PREFETCH_MISS = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_miss");
	EV_MOD_NMOESI_PREFETCH_UNLOCK = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_unlock");
	EV_MOD_NMOESI_PREFETCH_FINISH = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_finish");
	EV_MOD_NMOESI_PREFETCH_IDEAL  = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal");//cjc_20121225 ideal pf event
	EV_MOD_NMOESI_PREFETCH_IDEAL_INSERT = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal_insert");//cjc_20130107 ideal pf insertion event
	EV_MOD_NMOESI_PREFETCH_IDEAL_SEND = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal_send");//cjc_20130121 ideal pf msg-sending
	EV_MOD_NMOESI_PREFETCH_IDEAL_RECV = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal_recv");//cjc_20130121 ideal pf msg-receiving
	EV_MOD_NMOESI_PREFETCH_IDEAL_LATENCY = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal_latency");//cjc_20130121 ideal pf latency-adding
	EV_MOD_NMOESI_PREFETCH_IDEAL_RETURN = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_prefetch_ideal_return");	

	//IC_S_2013_11_13
	EV_MOD_NMOESI_L1DCUHINT = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_l1dcuhint");
	EV_MOD_NMOESI_L1DCUHINT_LOCK = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_l1dcuhint_lock");
	EV_MOD_NMOESI_L1DCUHINT_ACTION = esim_register_event_with_name(mod_handler_nmoesi_prefetch, "mod_nmoesi_l1dcuhint_action");

	//IC_E
	
	EV_MOD_NMOESI_FIND_AND_LOCK = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock, "mod_nmoesi_find_and_lock");
	EV_MOD_NMOESI_FIND_AND_LOCK_PORT = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock, "mod_nmoesi_find_and_lock_port");
	EV_MOD_NMOESI_FIND_AND_LOCK_ACTION = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock, "mod_nmoesi_find_and_lock_action");
	EV_MOD_NMOESI_FIND_AND_LOCK_FINISH = esim_register_event_with_name(mod_handler_nmoesi_find_and_lock, "mod_nmoesi_find_and_lock_finish");

	EV_MOD_NMOESI_EVICT = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict");
	EV_MOD_NMOESI_EVICT_INVALID = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_invalid");
	EV_MOD_NMOESI_EVICT_ACTION = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_action");
	EV_MOD_NMOESI_EVICT_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_receive");
	EV_MOD_NMOESI_EVICT_PROCESS = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_process");
	EV_MOD_NMOESI_EVICT_PROCESS_NONCOHERENT = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_process_noncoherent");
	EV_MOD_NMOESI_EVICT_REPLY = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_reply");
	EV_MOD_NMOESI_EVICT_REPLY = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_reply");
	EV_MOD_NMOESI_EVICT_REPLY_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_reply_receive");
	EV_MOD_NMOESI_EVICT_FINISH = esim_register_event_with_name(mod_handler_nmoesi_evict, "mod_nmoesi_evict_finish");

	EV_MOD_NMOESI_WRITE_REQUEST = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request");
	EV_MOD_NMOESI_WRITE_REQUEST_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_receive");
	EV_MOD_NMOESI_WRITE_REQUEST_ACTION = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_action");
	EV_MOD_NMOESI_WRITE_REQUEST_EXCLUSIVE = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_exclusive");
	EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_updown");
	EV_MOD_NMOESI_WRITE_REQUEST_UPDOWN_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_updown_finish");
	EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_downup");
	EV_MOD_NMOESI_WRITE_REQUEST_DOWNUP_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_downup_finish");
	EV_MOD_NMOESI_WRITE_REQUEST_REPLY = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_reply");
	EV_MOD_NMOESI_WRITE_REQUEST_FINISH = esim_register_event_with_name(mod_handler_nmoesi_write_request, "mod_nmoesi_write_request_finish");

	EV_MOD_NMOESI_READ_REQUEST = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request");
	EV_MOD_NMOESI_READ_REQUEST_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_receive");
	EV_MOD_NMOESI_READ_REQUEST_ACTION = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_action");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_updown");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN_MISS = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_updown_miss");
	EV_MOD_NMOESI_READ_REQUEST_UPDOWN_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_updown_finish");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_downup");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP_WAIT_FOR_REQS = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_downup_wait_for_reqs");
	EV_MOD_NMOESI_READ_REQUEST_DOWNUP_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_downup_finish");
	EV_MOD_NMOESI_READ_REQUEST_REPLY = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_reply");
	EV_MOD_NMOESI_READ_REQUEST_FINISH = esim_register_event_with_name(mod_handler_nmoesi_read_request, "mod_nmoesi_read_request_finish");

	EV_MOD_NMOESI_INVALIDATE = esim_register_event_with_name(mod_handler_nmoesi_invalidate, "mod_nmoesi_invalidate");
	EV_MOD_NMOESI_INVALIDATE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_invalidate, "mod_nmoesi_invalidate_finish");

	EV_MOD_NMOESI_PEER_SEND = esim_register_event_with_name(mod_handler_nmoesi_peer, "mod_nmoesi_peer_send");
	EV_MOD_NMOESI_PEER_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_peer, "mod_nmoesi_peer_receive");
	EV_MOD_NMOESI_PEER_REPLY = esim_register_event_with_name(mod_handler_nmoesi_peer, "mod_nmoesi_peer_reply");
	EV_MOD_NMOESI_PEER_FINISH = esim_register_event_with_name(mod_handler_nmoesi_peer, "mod_nmoesi_peer_finish");

	EV_MOD_NMOESI_MESSAGE = esim_register_event_with_name(mod_handler_nmoesi_message, "mod_nmoesi_message");
	EV_MOD_NMOESI_MESSAGE_RECEIVE = esim_register_event_with_name(mod_handler_nmoesi_message, "mod_nmoesi_message_receive");
	EV_MOD_NMOESI_MESSAGE_ACTION = esim_register_event_with_name(mod_handler_nmoesi_message, "mod_nmoesi_message_action");
	EV_MOD_NMOESI_MESSAGE_REPLY = esim_register_event_with_name(mod_handler_nmoesi_message, "mod_nmoesi_message_reply");
	EV_MOD_NMOESI_MESSAGE_FINISH = esim_register_event_with_name(mod_handler_nmoesi_message, "mod_nmoesi_message_finish");

	/* Local memory event driven simulation */

	EV_MOD_LOCAL_MEM_LOAD = esim_register_event_with_name(mod_handler_local_mem_load, "mod_local_mem_load");
	EV_MOD_LOCAL_MEM_LOAD_LOCK = esim_register_event_with_name(mod_handler_local_mem_load, "mod_local_mem_load_lock");
	EV_MOD_LOCAL_MEM_LOAD_FINISH = esim_register_event_with_name(mod_handler_local_mem_load, "mod_local_mem_load_finish");

	EV_MOD_LOCAL_MEM_STORE = esim_register_event_with_name(mod_handler_local_mem_store, "mod_local_mem_store");
	EV_MOD_LOCAL_MEM_STORE_LOCK = esim_register_event_with_name(mod_handler_local_mem_store, "mod_local_mem_store_lock");
	EV_MOD_LOCAL_MEM_STORE_FINISH = esim_register_event_with_name(mod_handler_local_mem_store, "mod_local_mem_store_finish");

	EV_MOD_LOCAL_MEM_FIND_AND_LOCK = esim_register_event_with_name(mod_handler_local_mem_find_and_lock, "mod_local_mem_find_and_lock");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_PORT = esim_register_event_with_name(mod_handler_local_mem_find_and_lock, "mod_local_mem_find_and_lock_port");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_ACTION = esim_register_event_with_name(mod_handler_local_mem_find_and_lock, "mod_local_mem_find_and_lock_action");
	EV_MOD_LOCAL_MEM_FIND_AND_LOCK_FINISH = esim_register_event_with_name(mod_handler_local_mem_find_and_lock, "mod_local_mem_find_and_lock_finish");

	//cjc_20121217 register throttle event
	EV_THROTTLE_START	=esim_register_event_with_name(throttle_handler,"throttle_handler_start");
	EV_THROTTLE_SCHEDULE	=esim_register_event_with_name(throttle_handler,"throttle_handler_schedule");
	EV_THROTTLE_CYCLE_FIRST =esim_register_event_with_name(throttle_handler,"throttle_handler_cycle_first");
	EV_THROTTLE_CYCLE_REPEAT=esim_register_event_with_name(throttle_handler,"throttle_handler_cycle_repeat");
	EV_THROTTLE_NEW_COUNTER =esim_register_event_with_name(throttle_handler,"throttle_handler_new_counter");
	EV_THROTTLE_EVENT_TRIGGER =esim_register_event_with_name(throttle_handler,"throttle_handler_event_trigger");
	
	//cjc_20130317

	EV_DRAM_ACTIVATE	=esim_register_event_with_name(dram_handler,"dram_activate");
	EV_DRAM_PRECHARGE	=esim_register_event_with_name(dram_handler,"dram_precharge");
	EV_DRAM_COL_ACCESS	=esim_register_event_with_name(dram_handler,"dram_col_access");
	EV_DRAM_BANK_SCHEDULE	=esim_register_event_with_name(dram_handler,"dram_bank_schedule");
	EV_DRAM_CHANNEL_SCHEDULE=esim_register_event_with_name(dram_handler,"dram_channel_schedule");
	EV_DRAM_BANK_STATUS	=esim_register_event_with_name(dram_handler,"dram_bank_status");
	EV_DRAM_DATA_BUS	=esim_register_event_with_name(dram_handler,"dram_data_bus");


	//cjc)20130513 register pf event
	EV_PF_TIME_INTERVAL	=esim_register_event_with_name(prefetcher_handler,"pf_time_interval");

	//porshe_20130812: event handler for dram module
	EV_DRAM_ACCESS = esim_register_event_with_name(dram_handler_dramsim2,"dram_access");
	EV_DRAM_WAIT = esim_register_event_with_name(dram_handler_dramsim2,"dram_wait");
	EV_DRAM_DONE = esim_register_event_with_name(dram_handler_dramsim2,"dram_done");

	/* Read configuration */
	mem_config_read();
	if(*prefetch_stride_table_ref_input)
	{
		int i;
		int pc,degree;
		ref_table.count = 0;
		for(i = 0; i < REF_TABLE_SIZE; i++)
			ref_table.valid[i] = 0;

		ref_file = file_open_for_read(prefetch_stride_table_ref_input);
		if(!ref_file)
			fatal("can not open file ");
		while(!feof(ref_file))
		{
			i = fscanf(ref_file,"%d%d",&pc,&degree);
			prefetcher_ref_table_insert(pc,degree);
		}
		
	}
	else if(*prefetch_stride_table_ref_output)
	{
		ref_file = file_open_for_write(prefetch_stride_table_ref_output);
	}
	else if (*prefetch_stride_table_ref_input&&*prefetch_stride_table_ref_output)
	{
		fatal("can not open those file together\n");
	}
	
	if(*prefetch_stride_sequence_out)
	{
		seq_file = file_open_for_write(prefetch_stride_sequence_out);
		if(!seq_file)
                        fatal("can not open file seq");
	}
}


void mem_system_done(void)
{
	/* Dump report */

	FILE *f;
	f = file_open_for_write(mem_report_file_name);
	if(f)
	{
		mem_system_dump_report(f);
		fclose(f);
	}
	//porshe:20130220 profiling report dump
	f = file_open_for_write(mem_profiling_file_name);
	if(f)
	{
		mem_system_profiling_report(f);
		fclose(f);
	}
	//end

	//cjc_20130517 dump address-analysis
	f = file_open_for_write(addr_anal_file_name);
	if(f)
	{
		addr_anal_dump(f);
		fclose(f);
		addr_anal_free();
	}

	struct mod_t *mod;
	int count = 0;

        /*porshe_20130812: Free dram handler*/
	dram_done();

	/* Free memory modules */
	while (list_count(mem_system->mod_list))
	{
		mod=list_pop(mem_system->mod_list);
		if(count)//cjc_20121225 each pf_cache of mod are the same, so only free it once
			mod->pf_cache = NULL;
		mod_free(mod);
		count = 1;
	}
	list_free(mem_system->mod_list);

	if(ref_file)
		fclose(ref_file);
	if(seq_file)
		fclose(seq_file);
	//IC_S_2013_09_24
	//if(fp)
	//	fclose(fp);
	//IC_E
	
	/* Free networks */
	while (list_count(mem_system->net_list))
		net_free(list_pop(mem_system->net_list));
	list_free(mem_system->net_list);

	/* Free memory system */
	free(mem_system);
}

void mem_system_profiling_report(FILE *f)
{
	struct mod_t *mod;
	struct cache_t *cache;
	int set_num, i, j;
	if (!f)
		return;
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		if(mod->level == 3)
		{
			cache = mod->cache;
			set_num =  cache->num_sets;
			for (j = 0; j < set_num; j ++)
				fprintf(f, "set_%d      = %lld\n", j, cache->num_of_access_on_set[j]);
			return;
		}

	}
	return;
}

void mem_system_dump_report(FILE *f)
{
	struct net_t *net;
	struct mod_t *mod;
	struct cache_t *cache;

	int i,j;

	if (!f)
		return;

	/* Intro */
	fprintf(f, "; Report for caches, TLBs, and main memory\n");
	fprintf(f, ";    Accesses - Total number of accesses\n");
	fprintf(f, ";    Hits, Misses - Accesses resulting in hits/misses\n");
	fprintf(f, ";    HitRatio - Hits divided by accesses\n");
	fprintf(f, ";    Evictions - Invalidated or replaced cache blocks\n");
	fprintf(f, ";    Retries - For L1 caches, accesses that were retried\n");
	fprintf(f, ";    ReadRetries, WriteRetries, NCWriteRetries - Read/Write retried accesses\n");
	fprintf(f, ";    NoRetryAccesses - Number of accesses that were not retried\n");
	fprintf(f, ";    NoRetryHits, NoRetryMisses - Hits and misses for not retried accesses\n");
	fprintf(f, ";    NoRetryHitRatio - NoRetryHits divided by NoRetryAccesses\n");
	fprintf(f, ";    NoRetryReads, NoRetryWrites - Not retried reads and writes\n");
	fprintf(f, ";    Reads, Writes, NCWrites - Total read/write accesses\n");
	fprintf(f, ";    BlockingReads, BlockingWrites, BlockingNCWrites - Reads/writes coming from lower-level cache\n");
	fprintf(f, ";    NonBlockingReads, NonBlockingWrites, NonBlockingNCWrites - Coming from upper-level cache\n");
	fprintf(f, "\n\n");

	/* Report for each cache */
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		cache = mod->cache;
		fprintf(f, "[ %s ]\n", mod->name);
		fprintf(f, "\n");

		/* Configuration */
		if (cache) {
			fprintf(f, "%s_Sets = %d\n", mod->name, cache->num_sets);
			fprintf(f, "%s_Assoc = %d\n", mod->name, cache->assoc);
			fprintf(f, "%s_Policy = %s\n", mod->name, str_map_value(&cache_policy_map, cache->policy));
		}
		fprintf(f, "%s_BlockSize = %d\n", mod->name, mod->block_size);
		fprintf(f, "%s_Latency = %d\n", mod->name, mod->latency);
		fprintf(f, "%s_Ports = %d\n", mod->name, mod->num_ports);
		fprintf(f, "\n");

		/* Statistics */
		fprintf(f, "%s_Accesses = %lld\n", mod->name, mod->accesses);
		fprintf(f, "%s_Hits = %lld\n", mod->name, mod->hits);
		fprintf(f, "%s_Misses = %lld\n", mod->name, mod->accesses - mod->hits);
		fprintf(f, "%s_HitRatio = %.4g\n", mod->name, mod->accesses ?
				(double) mod->hits / mod->accesses : 0.0);
		fprintf(f, "%s_Evictions = %lld\n", mod->name, mod->evictions);
		fprintf(f, "%s_Retries = %lld\n", mod->name, mod->read_retries + mod->write_retries +
				mod->nc_write_retries);
		fprintf(f, "\n");
		fprintf(f, "%s_Demand_Accesses = %lld\n", mod->name, mod->demand_accesses);
		fprintf(f, "%s_Demand_Hits = %lld\n", mod->name, mod->demand_hits);
		fprintf(f, "%s_Demand_Misses = %lld\n", mod->name, mod->demand_accesses - mod->demand_hits);
		fprintf(f, "%s_Demand_HitRatio = %.4g\n", mod->name, mod->demand_accesses ?
				(double) mod->demand_hits / mod->demand_accesses : 0.0);
		fprintf(f, "%s_Demand_Reads = %lld\n", mod->name, mod->demand_reads);
		fprintf(f, "%s_Demand_Writes = %lld\n", mod->name, mod->demand_writes);
		fprintf(f, "\n");

		//IC_S_2013_10_23
		//XXX
		fprintf(f, "%s_Demand_Access_Modified = %lld\n", mod->name, mod->Demand_Access);
		fprintf(f, "%s_Demand_Hit_Modified = %lld\n", mod->name, mod->Demand_Hit);
		fprintf(f, "%s_Demand_Miss_Modified = %lld\n", mod->name, mod->Demand_Access - mod->Demand_Hit);
		fprintf(f, "%s_Demand_HitRatio_Modified = %.4g\n", mod->name, mod->Demand_Access ?
                   		(double) mod->Demand_Hit / mod->Demand_Access : 0.0);
		fprintf(f, "%s_Demand_Read_Modified = %lld\n", mod->name, mod->Demand_Read);
		fprintf(f, "%s_Demand_Write_Modified = %lld\n", mod->name, mod->Demand_Write);
		fprintf(f, "\n");
		//IC_E

		fprintf(f, "%s_Pf_indentified = %lld (number of pf_request identified)\n", mod->name, mod->pf_indentified);	
		fprintf(f, "%s_Pf_already_in_MSHR = %lld (pf_request filter by MSHR)\n", mod->name, mod->pf_already_in_MSHR);
		fprintf(f, "%s_Pf_already_in_cache = %lld (pf_request filter by this cache)\n", mod->name, mod->pf_already_in_cache);
		fprintf(f, "%s_Pf_issued = %lld (number of pf_request issued)\n", mod->name, mod->pf_issued);
		fprintf(f, "%s_Pf_issued_get_from_mem = %lld (useful if mod == L1)\n", mod->name,mod->pf_issued_get_from_mem);
		fprintf(f, "%s_Pf_ignored_cross_page_access = %lld\n",mod->name,mod->pf_ignored_cross_page_access);
		fprintf(f, "%s_Pf_hits = %lld (useful pf)\n", mod->name, mod->pf_hits);
		fprintf(f, "%s_Pf_late = %lld (num of demands which are  waiting for pf)\n", mod->name, (mod->pf_late_load+mod->pf_late_store));
		fprintf(f, "%s_Pf_late_load = %lld (num of load which are  waiting for pf)\n", mod->name, mod->pf_late_load);
		fprintf(f, "%s_Pf_late_store = %lld (num of store which are  waiting for pf)\n", mod->name, mod->pf_late_store);
		fprintf(f, "%s_Pf_late_cycle = %lld (cycles that demands are waiting for pf_request coming back)\n", mod->name, (mod->pf_late_cycle_load+mod->pf_late_cycle_store));
		fprintf(f, "%s_Pf_late_cycle_load = %lld (cycles that all of load are waiting for pf_request coming back)\n", mod->name, mod->pf_late_cycle_load);
		fprintf(f, "%s_Pf_late_cycle_store = %lld (cycles that all of store are waiting for pf_request coming back)\n", mod->name, mod->pf_late_cycle_store);
		fprintf(f, "%s_Pf_late_load_same_pc = %lld \n", mod->name, mod->pf_late_load_same_pc);
		fprintf(f, "%s_Pf_late_store_same_pc = %lld \n", mod->name, mod->pf_late_store_same_pc);
		fprintf(f, "%s_Pf_late_same_pc = %lld \n", mod->name, mod->pf_late_store_same_pc+mod->pf_late_load_same_pc);
		fprintf(f, "%s_Pf_late_stride_found = %lld \n", mod->name, mod->pf_late_stride_found);
		fprintf(f, "%s_Pf_late_stride_used = %lld \n", mod->name, mod->pf_late_stride_used);
		fprintf(f, "%s_Pf_pollution = %lld (miss count of pf-causing)\n", mod->name, mod->pf_pollution);
		fprintf(f, "%s_Pf_hit_by_upper_level = %lld \n", mod->name, mod->pf_hit_by_upper_level);
		fprintf(f, "%s_Pf_hit_by_other_core = %lld (used by other core (useless in private cache))\n", mod->name, mod->pf_hit_by_other_core);
		fprintf(f, "%s_Pf_early_cycle= %lld (every pf_hit cycles record from insertion to use-by-demand)\n", mod->name, mod->pf_early_cycle);
		fprintf(f, "%s_Pf_early_cycle_upper_level= %lld (every pf_hit cycles record from insertion to use-by-upper-level)\n", mod->name, mod->pf_early_cycle_upper_level);
		fprintf(f, "%s_Pf_downup_invalidation = %lld\n", mod->name,mod->pf_downup_invalidation);
		fprintf(f, "%s_Pf_pollution_FDP = %lld\n",mod->name,mod->pf_pollution_FDP);
		fprintf(f, "\n");
		fprintf(f, "%s_Pf_in_cache_get_by_others = %lld \n", mod->name, mod->pf_in_cache_get_by_others);
		fprintf(f, "%s_Pf_in_cache_get_by_other_pf = %lld \n", mod->name, mod->pf_in_cache_get_by_other_pf);
		fprintf(f, "%s_Pf_in_cache_get_by_itself = %lld \n", mod->name, mod->pf_in_cache_get_by_itself);
		fprintf(f, "%s_Pf_in_cache_get_by_its_own_pf = %lld \n", mod->name, mod->pf_in_cache_get_by_its_own_pf);
		fprintf(f, "\n");
		fprintf(f, "%s_Pf_ideal_cache_victim = %lld \n", mod->name, mod->pf_ideal_cache_victim);
		fprintf(f, "%s_Pf_ideal_wait_cycle = %lld \n", mod->name, mod->pf_ideal_wait_cycle);
		fprintf(f, "%s_Pf_ideal_l1_retry = %lld \n", mod->name, mod->pf_ideal_l1_retry);
		fprintf(f, "%s_Pf_ideal_l2_retry = %lld \n", mod->name, mod->pf_ideal_l2_retry);
		fprintf(f, "%s_Pf_ideal_access_count = %lld \n", mod->name, mod->pf_ideal_access);
		fprintf(f, "%s_Pf_ideal_latency = %lld \n", mod->name, mod->pf_ideal_latency);
		fprintf(f, "%s_Pf_ideal_skipped_mem_access = %lld \n", mod->name, mod->pf_ideal_skipped_mem_access);
		fprintf(f, "\n");
		fprintf(f, "%s_Pf_stream_trained_stream = %lld \n", mod->name,mod->stream_trained_stream);
		fprintf(f, "%s_Pf_stream_renew_stream = %lld \n", mod->name,mod->stream_renew_stream);
		fprintf(f, "%s_Pf_stream_cleared_stream = %lld \n", mod->name,mod->stream_cleared_stream);
		fprintf(f, "%s_Pf_stream_table_replace = %lld \n", mod->name,mod->stream_table_replace);
		fprintf(f, "\n");

		
		fprintf(f, "%s mshr_full_block_num = %lld \n", mod->name,mod->mshr_block);
		fprintf(f, "%s prefetch_mshr_full_block_num = %lld \n", mod->name,mod->prefetch_mshr_block);

		fprintf(f, "%s acc_waiting_oi_num = %lld \n", mod->name,mod->acc_waiting_oi_num);
		fprintf(f, "%s acc_waiting_oi_cycle = %lld \n\n", mod->name,mod->acc_waiting_oi_cycle);


		fprintf(f, "%s acc_load_access_time = %lld \n", mod->name,mod->acc_load_access_time);
		fprintf(f, "%s acc_load_find_n_lock = %lld \n", mod->name,mod->acc_load_find_n_lock);
		fprintf(f, "%s acc_load_L1_L2_net = %lld \n", mod->name,mod->acc_load_L1_L2_net);
		fprintf(f, "%s acc_load_L2_access = %lld \n", mod->name,mod->acc_load_L2_access);
		fprintf(f, "%s acc_load_reply = %lld \n", mod->name,mod->acc_load_reply);
		fprintf(f, "%s acc_load_MM_access = %lld \n", mod->name,mod->acc_load_MM_access);
		fprintf(f, "%s acc_load_peer_to_peer = %lld \n", mod->name,mod->acc_load_peer_to_peer);
		fprintf(f, "%s acc_load_wait_old_write = %lld \n", mod->name,mod->acc_load_wait_old_write);
		fprintf(f, "%s acc_load_wait_old_inflight_cycle = %lld \n\n", mod->name,mod->acc_load_wait_old_inflight_cycle);

		
		fprintf(f, "%s acc_store_access_time = %lld \n", mod->name,mod->acc_store_access_time);
		fprintf(f, "%s acc_store_find_n_lock = %lld \n", mod->name,mod->acc_store_find_n_lock);
		fprintf(f, "%s acc_store_L1_L2_net = %lld \n", mod->name,mod->acc_store_L1_L2_net);
		fprintf(f, "%s acc_store_L2_access = %lld \n", mod->name,mod->acc_store_L2_access);
		fprintf(f, "%s acc_store_exclusive = %lld \n", mod->name,mod->acc_store_exclusive);
		fprintf(f, "%s acc_store_reply = %lld \n", mod->name,mod->acc_store_reply);
		fprintf(f, "%s acc_store_MM_access = %lld \n", mod->name,mod->acc_store_MM_access);
		fprintf(f, "%s acc_store_wait_old_access = %lld \n", mod->name,mod->acc_store_wait_old_access);
		fprintf(f, "%s acc_store_wait_old_inflight_cycle = %lld \n", mod->name,mod->acc_store_wait_old_inflight_cycle);

		fprintf(f, "\n");
		fprintf(f, "%s_hint_table_hit = %lld \n", mod->name,mod->hint_hit);		
		fprintf(f, "%s_hint_table_miss = %lld \n", mod->name,mod->hint_miss);		
		fprintf(f, "%s_hint_table_full = %lld \n", mod->name,mod->hint_full);		
		fprintf(f, "%s_hint_table_hint_trigger_num0 = %lld \n", mod->name,mod->hint_trigger_num0);		
		fprintf(f, "%s_hint_table_hint_trigger_num1 = %lld \n", mod->name,mod->hint_trigger_num1);		
		fprintf(f, "%s_hint_table_hint_trigger_num2 = %lld \n", mod->name,mod->hint_trigger_num2);		
		fprintf(f, "%s_hint_table_hint_trigger_num3 = %lld \n", mod->name,mod->hint_trigger_num3);		
		fprintf(f, "%s_hint_table_hint_change0 = %lld \n", mod->name,mod->hint_change0);		
		fprintf(f, "%s_hint_table_hint_change1 = %lld \n", mod->name,mod->hint_change1);		
		fprintf(f, "%s_hint_table_hint_change2 = %lld \n", mod->name,mod->hint_change2);		
		fprintf(f, "%s_hint_table_hint_change3 = %lld \n", mod->name,mod->hint_change3);		
		fprintf(f, "\n");

		for(j = 0; j <5 ;j++)
			fprintf(f, "%s_Pf_local_degree_%d = %lld\n",mod->name,j-2,mod->local_degree[j]);	
		fprintf(f, "\n");
		//porshe_20121127 access distribution
		fprintf(f, "%s_R_hit_on_self_l1_lat = %lld\n", mod->name, mod->R_hit_on_self_l1);
		fprintf(f, "%s_R_hit_on_remote_l1_lat = %lld\n", mod->name, mod->R_hit_on_remote_l1);
		fprintf(f, "%s_R_hit_on_l2_lat = %lld\n", mod->name, mod->R_hit_on_l2);
		fprintf(f, "%s_R_hit_on_memory = %lld\n", mod->name, mod->R_hit_on_memory);
		fprintf(f, "%s_R_l1_wo_depend_hit_num = %lld\n", mod->name, mod->R_l1_wo_depend_hit_num);
		fprintf(f, "%s_R_l1_w_depend_hit_num = %lld\n", mod->name, mod->R_l1_w_depend_hit_num);
		fprintf(f, "%s_R_l1_remote_hit_num = %lld\n", mod->name, mod->R_l1_remote_hit_num);
		fprintf(f, "%s_R_l2_hit_num = %lld\n", mod->name, mod->R_l2_hit_num);
		fprintf(f, "%s_R_mm_hit_num = %lld\n", mod->name, mod->R_mm_hit_num);
		fprintf(f, "\n");

		//IC_S_2013_11_19
		fprintf(f, "%s_R_hit_on_self_l1_lat_per_num = %.4g\n", mod->name, 
				(double)(mod->R_hit_on_self_l1) / 
				(
				(mod->R_l1_wo_depend_hit_num) +
				(mod->R_l1_w_depend_hit_num)
				)
				);
		fprintf(f, "%s_R_hit_on_remote_l1_lat_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_remote_l1) /
				(mod->R_l1_remote_hit_num) 
				);
		fprintf(f, "%s_R_hit_on_l2_lat_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_l2) /
				(mod->R_l2_hit_num) 
				);
		fprintf(f, "%s_R_hit_on_memory_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_memory) /
				(mod->R_mm_hit_num) 
				);
		fprintf(f, "\n");
		//IC_E

		fprintf(f, "%s_W_hit_on_self_l1_lat = %lld\n", mod->name, mod->W_hit_on_self_l1);
		fprintf(f, "%s_W_hit_on_remote_l1_lat = %lld\n", mod->name, mod->W_hit_on_remote_l1);
		fprintf(f, "%s_W_hit_on_l2_lat = %lld\n", mod->name, mod->W_hit_on_l2);
		fprintf(f, "%s_W_hit_on_memory = %lld\n", mod->name, mod->W_hit_on_memory);
		fprintf(f, "%s_W_l1_wo_depend_hit_num = %lld\n", mod->name, mod->W_l1_wo_depend_hit_num);
		fprintf(f, "%s_W_l1_w_depend_hit_num = %lld\n", mod->name, mod->W_l1_w_depend_hit_num);
		fprintf(f, "%s_W_l1_remote_hit_num = %lld\n", mod->name, mod->W_l1_remote_hit_num);
		fprintf(f, "%s_W_l2_hit_num = %lld\n", mod->name, mod->W_l2_hit_num);
		fprintf(f, "%s_W_mm_hit_num = %lld\n", mod->name, mod->W_mm_hit_num);
		fprintf(f, "\n");

		//IC_S_2013_11_19
		fprintf(f, "%s_W_hit_on_self_l1_lat_per_num = %.4g\n", mod->name, 
				(double)(mod->W_hit_on_self_l1) / 
				(
				(mod->W_l1_wo_depend_hit_num) +
				(mod->W_l1_w_depend_hit_num)
				)
				);
		fprintf(f, "%s_W_hit_on_remote_l1_lat_per_num = %.4g\n", mod->name,
				(double)(mod->W_hit_on_remote_l1) /
				(mod->W_l1_remote_hit_num) 
				);
		fprintf(f, "%s_W_hit_on_l2_lat_per_num = %.4g\n", mod->name,
				(double)(mod->W_hit_on_l2) /
				(mod->W_l2_hit_num) 
				);
		fprintf(f, "%s_W_hit_on_memory_per_num = %.4g\n", mod->name,
				(double)(mod->W_hit_on_memory) /
				(mod->W_mm_hit_num) 
				);
		//IC_E

		fprintf(f, "\n");

		//IC_S_2013_11_19
		fprintf(f, "%s_R/W_hit_on_self_l1_lat_per_num = %.4g\n", mod->name, 
				(double)(mod->R_hit_on_self_l1 + mod->W_hit_on_self_l1) / 
				(
				mod->R_l1_wo_depend_hit_num +
				mod->R_l1_w_depend_hit_num + 
				mod->W_l1_wo_depend_hit_num +
				mod->W_l1_w_depend_hit_num
				)
				);
		fprintf(f, "%s_R/W_hit_on_remote_l1_lat_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_remote_l1 + mod->W_hit_on_remote_l1) /
				(
				mod->R_l1_remote_hit_num +
				mod->W_l1_remote_hit_num
				)
				);
		fprintf(f, "%s_R/W_hit_on_l2_lat_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_l2 + mod->W_hit_on_l2) /
				(
				mod->R_l2_hit_num +
				mod->W_l2_hit_num
				)
				);
		fprintf(f, "%s_R/W_hit_on_memory_per_num = %.4g\n", mod->name,
				(double)(mod->R_hit_on_memory + mod->W_hit_on_memory) /
				(
				mod->R_mm_hit_num +
				mod->W_mm_hit_num
				)
				);
		fprintf(f, "\n");
		//IC_E

		fprintf(f, "%s_Cache_dead_block_demand = %lld (demand-data which has no reuse)\n",mod->name,mod->cache_dead_block_demand);
		fprintf(f, "%s_Cache_dead_block_demand_caused_by_pf = %lld \n",mod->name,mod->cache_dead_block_demand_caused_by_pf);
		fprintf(f, "%s_Cache_dead_block_pf = %lld (pf-data which has no reuse)\n",mod->name,mod->cache_dead_block_prefetch);
		fprintf(f, "%s_Cache_evict_block = %lld \n",mod->name,mod->cache_evict_block);
		fprintf(f, "%s_Cache_evict_block_wo_reused = %lld\n",mod->name,mod->cache_dead_block_demand+mod->cache_dead_block_prefetch);
		fprintf(f, "%s_Cache_evict_block_reused= %lld\n",mod->name,mod->cache_evict_block-(mod->cache_dead_block_demand+mod->cache_dead_block_prefetch));
		fprintf(f, "\n");
		
		//IC_S_2013_11_04
		fprintf(f, "\n");
		fprintf(f, "%s_R/W_l1_wo_depend_hit_num = %lld\n", mod->name, (mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num));
		fprintf(f, "%s_R/W_l1_w_depend_hit_num = %lld\n", mod->name, (mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num));
		fprintf(f, "%s_R/W_l1_remote_hit_num = %lld\n", mod->name, (mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num));
		fprintf(f, "%s_R/W_l2_hit_num = %lld\n", mod->name, (mod->R_l2_hit_num + mod->W_l2_hit_num));
		fprintf(f, "%s_R/W_mm_hit_num = %lld\n", mod->name, (mod->R_mm_hit_num + mod->W_mm_hit_num));
		fprintf(f, "%s_R/W_total_access_num = %lld\n", mod->name, 
				(mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num) + 
				(mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num) + 
				(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) + 
				(mod->R_l2_hit_num + mod->W_l2_hit_num) + 
				(mod->R_mm_hit_num + mod->W_mm_hit_num)
				);
		fprintf(f, "%s_R/W_L1_hit_ratio = %.4g\n", mod->name, 
				(double)((mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num) + (mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num)) / 
				((mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num) +
				(mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num) +
				(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) +
				(mod->R_l2_hit_num + mod->W_l2_hit_num) +
				(mod->R_mm_hit_num + mod->W_mm_hit_num)
				)
				);
		//fprintf(f, "%s_R/W_L1_hit_ratio_combine_remote = %.4g\n", mod->name, 
		//		(double)((mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num) + (mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num) + (mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num)) / 
		//		((mod->R_l1_wo_depend_hit_num + mod->W_l1_wo_depend_hit_num) +
		//		(mod->R_l1_w_depend_hit_num + mod->W_l1_w_depend_hit_num) +
		//		(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) +
		//		(mod->R_l2_hit_num + mod->W_l2_hit_num) +
		//		(mod->R_mm_hit_num + mod->W_mm_hit_num)
		//		)
		//		);
		fprintf(f, "%s_R/W_L1_hit_remote_ratio = %.4g\n", mod->name, 
				(double)(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) / 
				(
				(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) +
				(mod->R_l2_hit_num + mod->W_l2_hit_num) +
				(mod->R_mm_hit_num + mod->W_mm_hit_num)
				)
				);
		fprintf(f, "%s_R/W_L2_hit_ratio = %.4g\n", mod->name, 
				(double)(mod->R_l2_hit_num + mod->W_l2_hit_num) / 
				(
				//(mod->R_l1_remote_hit_num + mod->W_l1_remote_hit_num) +  
				(mod->R_l2_hit_num + mod->W_l2_hit_num) +
				(mod->R_mm_hit_num + mod->W_mm_hit_num)
				)
				);
		fprintf(f, "\n");
		//IC_E
		
		//IC_S_2013_11_26
		//AMAT = L1 cache hit time + L1 miss rate*(L2 hit time + L2 miss rate*(memory access time))
		//IC_E

		fprintf(f, "%s_Prefetches = %lld\n",mod->name, mod->prefetches);
		fprintf(f, "%s_PrefetchAborts = %lld\n",mod->name, mod->prefetch_aborts);
		fprintf(f, "%s_UselessPrefetches = %lld\n",mod->name, mod->useless_prefetches);
		fprintf(f, "\n");
		fprintf(f, "%s_Reads = %lld\n", mod->name, mod->reads);
		fprintf(f, "%s_ReadRetries = %lld\n", mod->name, mod->read_retries);
		fprintf(f, "%s_BlockingReads = %lld\n", mod->name, mod->blocking_reads);
		fprintf(f, "%s_NonBlockingReads = %lld\n", mod->name, mod->non_blocking_reads);
		fprintf(f, "%s_ReadHits = %lld\n", mod->name, mod->read_hits);
		fprintf(f, "%s_ReadMisses = %lld\n", mod->name, mod->reads - mod->read_hits);
		fprintf(f, "\n");
		fprintf(f, "%s_Writes = %lld\n", mod->name, mod->writes);
		fprintf(f, "%s_WriteRetries = %lld\n", mod->name, mod->write_retries);
		fprintf(f, "%s_BlockingWrites = %lld\n", mod->name, mod->blocking_writes);
		fprintf(f, "%s_NonBlockingWrites = %lld\n", mod->name, mod->non_blocking_writes);
		fprintf(f, "%s_WriteHits = %lld\n", mod->name, mod->write_hits);
		fprintf(f, "%s_WriteMisses = %lld\n", mod->name, mod->writes - mod->write_hits);
		fprintf(f, "\n");
		fprintf(f, "%s_NCWrites = %lld\n", mod->name, mod->nc_writes);
		fprintf(f, "%s_NCWriteRetries = %lld\n", mod->name, mod->nc_write_retries);
		fprintf(f, "%s_NCBlockingWrites = %lld\n", mod->name, mod->blocking_nc_writes);
		fprintf(f, "%s_NCNonBlockingWrites = %lld\n", mod->name, mod->non_blocking_nc_writes);
		fprintf(f, "%s_NCWriteHits = %lld\n", mod->name, mod->nc_write_hits);
		fprintf(f, "%s_NCWriteMisses = %lld\n", mod->name, mod->nc_writes - mod->nc_write_hits);
		fprintf(f, "\n");
		fprintf(f, "%s_NoRetryAccesses = %lld\n", mod->name, mod->no_retry_accesses);
		fprintf(f, "%s_NoRetryHits = %lld\n", mod->name, mod->no_retry_hits);
		fprintf(f, "%s_NoRetryMisses = %lld\n", mod->name, mod->no_retry_accesses - mod->no_retry_hits);
		fprintf(f, "%s_NoRetryHitRatio = %.4g\n", mod->name, mod->no_retry_accesses ?
				(double) mod->no_retry_hits / mod->no_retry_accesses : 0.0);
		fprintf(f, "%s_NoRetryReads = %lld\n", mod->name, mod->no_retry_reads);
		fprintf(f, "%s_NoRetryReadHits = %lld\n", mod->name, mod->no_retry_read_hits);
		fprintf(f, "%s_NoRetryReadMisses = %lld\n", mod->name, (mod->no_retry_reads -
					mod->no_retry_read_hits));
		fprintf(f, "%s_NoRetryWrites = %lld\n", mod->name, mod->no_retry_writes);
		fprintf(f, "%s_NoRetryWriteHits = %lld\n", mod->name, mod->no_retry_write_hits);
		fprintf(f, "%s_NoRetryWriteMisses = %lld\n", mod->name, mod->no_retry_writes
				- mod->no_retry_write_hits);
		fprintf(f, "%s_NoRetryNCWrites = %lld\n", mod->name, mod->no_retry_nc_writes);
		fprintf(f, "%s_NoRetryNCWriteHits = %lld\n", mod->name, mod->no_retry_nc_write_hits);
		fprintf(f, "%s_NoRetryNCWriteMisses = %lld\n", mod->name, mod->no_retry_nc_writes
				- mod->no_retry_write_hits);

		if(mod->dram_controller)
		{
			fprintf(f, "\n");
			fprintf(f, "%s_DRAM_Access = %lld\n", mod->name, mod->dram_access_demand+mod->dram_access_prefetch+mod->dram_access_others);
			fprintf(f, "%s_DRAM_AccessLatency = %lld\n", mod->name, mod->dram_access_latency_demand+
					mod->dram_access_latency_prefetch+mod->dram_access_latency_others);
			fprintf(f, "%s_DRAM_RowBufferHit = %lld\n", mod->name, mod->dram_rowbuffer_hit);

			fprintf(f, "%s_DRAM_Access_Demand = %lld\n", mod->name, mod->dram_access_demand);
			fprintf(f, "%s_DRAM_Access_Prefetch = %lld\n", mod->name, mod->dram_access_prefetch);
			fprintf(f, "%s_DRAM_Access_Others = %lld\n", mod->name, mod->dram_access_others);
			fprintf(f, "%s_DRAM_AccessLatency_Demand = %lld\n", mod->name, mod->dram_access_latency_demand);
			fprintf(f, "%s_DRAM_AccessLatency_Prefetch = %lld\n", mod->name, mod->dram_access_latency_prefetch);
			fprintf(f, "%s_DRAM_AccessLatency_Others = %lld\n", mod->name, mod->dram_access_latency_others);
			fprintf(f, "%s_DRAM_QueuingCycle_ALL = %lld\n", mod->name, mod->dram_demand_queuing_cycle+mod->dram_prefetch_queuing_cycle+mod->dram_others_queuing_cycle);
			fprintf(f, "%s_DRAM_QueuingCycle_Demand = %lld\n", mod->name, mod->dram_demand_queuing_cycle);
			fprintf(f, "%s_DRAM_QueuingCycle_Prefetch = %lld\n", mod->name, mod->dram_prefetch_queuing_cycle);
			fprintf(f, "%s_DRAM_QueuingCycle_Others = %lld\n", mod->name, mod->dram_others_queuing_cycle);
			fprintf(f, "%s_DRAM_WaitForBus_Cycles = %lld\n", mod->name, mod->dram_bus_waiting_cycle);
		}
		fprintf(f, "\n\n");
		if(mod->cache->prefetcher)
		{
			prefetcher_dump_stride_table(mod,f,-1,0);	
			throttle_dump_report(f,mod->cache->throttle);
		}

		//porshe_20131025: TLB statistics dump
		if(mod->tlb)
		{
			fprintf(f, "%s_TLB_Access = %lld\n", mod->name, mod->tlb_access);	
			fprintf(f, "%s_TLB_Miss = %lld\n", mod->name, mod->tlb_miss);	
		}
		//end
	}

	/* Dump report for networks */
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		net_dump_report(net, f);
	}

	/* Done */
}


struct mod_t *mem_system_get_mod(char *mod_name)
{
	struct mod_t *mod;

	int mod_id;

	/* Look for module */
	LIST_FOR_EACH(mem_system->mod_list, mod_id)
	{
		mod = list_get(mem_system->mod_list, mod_id);
		if (!strcasecmp(mod->name, mod_name))
			return mod;
	}

	/* Not found */
	return NULL;
}


struct net_t *mem_system_get_net(char *net_name)
{
	struct net_t *net;

	int net_id;

	/* Look for network */
	LIST_FOR_EACH(mem_system->net_list, net_id)
	{
		net = list_get(mem_system->net_list, net_id);
		if (!strcasecmp(net->name, net_name))
			return net;
	}

	/* Not found */
	return NULL;
}

