/*
 * Version          : $Revision: 1.0 $
 * Last Modified On : $Date: 2013/03/15
 * Last Modified By : $Author: cjc
 *----------------------------------------------------------------------
 * Description      :
 *                   DRAM module  
 *----------------------------------------------------------------------
 * Modification List:
 *
 *                              */

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/misc.h>

#include <arch/x86/timing/cpu.h>
#include "mod-stack.h"
#include "module.h"
#include "dram.h"
#include "cache.h"
#include "mem-system.h"


int EV_DRAM_ACTIVATE;
int EV_DRAM_PRECHARGE;
int EV_DRAM_COL_ACCESS;
int EV_DRAM_BANK_SCHEDULE;
int EV_DRAM_CHANNEL_SCHEDULE;
int EV_DRAM_BANK_STATUS;
int EV_DRAM_DATA_BUS;


int dram_debug_category;
//IC_S_2013_07_29
int dram_log_category;
//IC_E
/*static int dram_insert_request(struct dram_ctrl* dram, unsigned int bank_id, unsigned int row_id,struct mod_stack_t *stack)
  {
  struct dram_buffer *buffer;
  struct dram_buffer_entry *entry;

  buffer = &dram->buffer[bank_id];

  int ptr;

  ptr = buffer->head;
  entry = &buffer->entry[ptr];

  if(buffer->count<DRAM_BUFFER_SIZE)
  {
  buffer->count = buffer->count +1;
//entry->valid = 1;
entry->row = row_id;
entry->stack = stack;
buffer->head = (ptr+1)%DRAM_BUFFER_SIZE;
return 1;
}
else
return 0;
}*/
void dram_set_drb_entry(struct mod_stack_t *stack)
{
	unsigned int bank_id,row_id;//,column_id;

	unsigned int addr;

	int ptr,i;
	int channel_id,core_id;
	struct mod_t *mod;
	struct dram_ctrl* dram;
	struct dram_buffer *buffer;
	struct dram_buffer_entry *entry;
	struct mod_stack_t *new_stack;//,master_stack;
	assert(stack);

	mod  = stack->mod;
	dram = mod->dram_controller;
	addr = stack->addr;

	//assert(mod->kind==mod_kind_main_memory);
	if(!dram)
		return;

	bank_id = (addr>>dram->bid_shift)&(dram->bid_msk);
	row_id  = (addr>>dram->rid_shift);

	buffer = &dram->buffer[bank_id];

	channel_id = bank_id/dram->dram_bank_per_channel;

	stack->bank_id = bank_id;
	stack->row_id = row_id;	
	stack->dram_buffer_insert_cycle = esim_cycle;
	core_id = stack->core_id;
	
	assert(core_id<x86_cpu_num_cores&&core_id>=0);


	//merge request
	if(buffer->cur_request!=buffer->new_request)
	{	
		ptr = buffer->new_request-1;	
		for(i = 0; i < DRAM_BUFFER_SIZE; i++,ptr--)
		{
			if(ptr<0)
				ptr = DRAM_BUFFER_SIZE -1;
			entry = &buffer->entry[ptr];
			if(entry->valid&&entry->stack->addr==stack->addr)
			{
				if(entry->stack->read&&stack->read)
				{
					mod_stack_wait_in_stack(stack,entry->stack,EV_DRAM_DATA_BUS);
					return;	
				}
				else
					break;
			}
			if(ptr == buffer->cur_request)
				break;
		}		
	}

	buffer->requester_counter[core_id]++;
	ptr = buffer->new_request;
	entry = &buffer->entry[ptr];

	dram_debug("%lld new request %d-%d %lld %x\n",esim_cycle,bank_id,ptr,stack->id,stack->addr);
	//IC_S_2013_07_29
	if (warmup_finish != 01)
	{
		dram_log("%lld %d %d %d %d %d %d %d %lld %x\n",
                          esim_cycle, core_id, channel_id, bank_id, row_id, ptr, 
                          buffer->waiting_request, buffer->requester_counter[core_id], stack->id, stack->addr);
	}
	//IC_E
	buffer->new_request = (ptr+1)%DRAM_BUFFER_SIZE;
	if(buffer->cur_request!=buffer->new_request)
	{
		if(entry->valid)
			fatal("FIXME");


		entry->valid = 1;
		entry->row = row_id;
		entry->stack = stack;
		entry->inserted_cycle = esim_cycle;

		buffer->waiting_request++;
		dram_debug("insert bank%d waiting=%d\n",bank_id,buffer->waiting_request);
		if(buffer->avail_cycle <= esim_cycle )//&&dram->channel_avail[channel_id]<=esim_cycle)
		{
			buffer->avail_cycle = esim_cycle + 1;//using this to occupy the bank
			new_stack = mod_stack_create(stack->id, mod, stack->addr,-1, NULL);
			//new_stack->target_mod = mod;
			new_stack->bank_id = bank_id;	
			new_stack->core_id = core_id;
			esim_schedule_event(EV_DRAM_BANK_SCHEDULE,new_stack,0);
		}
		return;
	}
	fatal("DRAM FIXME");
}
void dram_handler(int event, void *data)
{
	struct mod_t *mod;
	struct mod_stack_t *stack = data;
	struct mod_stack_t *new_stack;
	struct dram_ctrl* dram;
	struct dram_buffer *buffer;
	struct dram_buffer_entry *entry;
	unsigned int bank_id,row_id;
	int cur_ptr,banks;//,next_bank_grand;
	int channel_id,core_id;


	mod  = stack->mod;
	dram = mod->dram_controller;
	bank_id = stack->bank_id;
	row_id = stack->row_id;
	if(!dram)
		fatal("DRAM is NULL");

	dram_debug("%lld %lld %x ",esim_cycle,stack->id,stack->addr);
	banks = dram->dram_num_banks; 
	buffer = &dram->buffer[bank_id];
	channel_id = bank_id/dram->dram_bank_per_channel;
	core_id = stack->core_id;
	
	/*if(event == EV_DRAM_BANK_STATUS)
	  {
	  if(buffer->row_buffer_id==~0)
	  {
	  buffer->row_buffer_id = row_id;
	  dram_debug("activate\n" );
	  buffer->avail_cycle = esim_cycle + dram->dram_activate_latency;
	  esim_schedule_event(EV_DRAM_BANK_STATUS,stack,dram->dram_activate_latency);
	  }
	  else if (buffer->row_buffer_id == row_id)
	  {
	  buffer->entry[buffer->cur_request].stack=NULL;
	  buffer->entry[buffer->cur_request].valid=0;
	  dram_debug("access %d fin\n",buffer->cur_request);
	  buffer->avail_cycle = esim_cycle + dram->dram_precharge_latency;
	  buffer->cur_request = (buffer->cur_request +1 )%DRAM_BUFFER_SIZE;
	  mod_stack_return(stack);
	//dram_debug("\n");	
	}
	else
	{
	buffer->row_buffer_id = ~0;
	dram_debug("precharge\n");
	buffer->avail_cycle = esim_cycle + dram->dram_precharge_latency;
	esim_schedule_event(EV_DRAM_BANK_STATUS,stack,dram->dram_precharge_latency);
	}

	}*/

	if(event==EV_DRAM_ACTIVATE)
	{
		if(dram->channel_avail[channel_id]>esim_cycle)
		{
			dram_debug("delay\n" );
			esim_schedule_event(event,stack,dram->channel_avail[channel_id]-esim_cycle);
			buffer->avail_cycle = esim_cycle + dram->dram_cycle;
			return;
		}
		dram->channel_avail[channel_id] = esim_cycle + dram->dram_cycle;
		dram_debug("activate\n" );
		buffer->avail_cycle = esim_cycle + dram->dram_column_latency;	
		esim_schedule_event(EV_DRAM_COL_ACCESS,stack,dram->dram_column_latency);
	}
	else if(event==EV_DRAM_PRECHARGE)
	{
		if(dram->channel_avail[channel_id]>esim_cycle)
		{
			dram_debug("delay\n" );
			esim_schedule_event(event,stack,dram->channel_avail[channel_id]-esim_cycle);
			buffer->avail_cycle = esim_cycle + dram->dram_cycle;
			return;
		}
		dram->channel_avail[channel_id] = esim_cycle + dram->dram_cycle;
		dram_debug("precharge\n");
		buffer->avail_cycle = esim_cycle + dram->dram_activate_latency + dram->dram_cycle;
		esim_schedule_event(EV_DRAM_ACTIVATE,stack,dram->dram_activate_latency);
	}
	else if(event==EV_DRAM_COL_ACCESS)
	{
		dram->channel_avail[channel_id] = esim_cycle + dram->dram_cycle;
		buffer->row_buffer_id = row_id;
		buffer->entry[buffer->cur_request].stack=NULL;
		buffer->entry[buffer->cur_request].valid=0;
		dram_debug("access %d ",buffer->cur_request);

		buffer->cur_request = (buffer->cur_request +1 )%DRAM_BUFFER_SIZE;
		if(buffer->cur_request!=buffer->new_request)//buffer isn't empty
		{
			dram_debug(" next bank_id=%d \n",bank_id);
			new_stack = mod_stack_create(stack->id, mod, stack->addr,-1, NULL);
			new_stack->bank_id = bank_id;
			new_stack->core_id = core_id;
			//new_stack->target_mod = mod;
			esim_schedule_event(EV_DRAM_BANK_SCHEDULE,new_stack,0);
		}
		else
		{
			buffer->cur_requester_id = -1;
			dram_debug("\n");
			dram_calc_bw_needed(dram);
			assert(buffer->waiting_request==0);
		}
		
		//mod->dram_access++;
		
		dram->bw_consumed[core_id]--;
		buffer->requester_counter[core_id]--;			
		assert(dram->bw_consumed[core_id]>=0);			
		esim_schedule_event(EV_DRAM_DATA_BUS,stack,dram->dram_burst_length);
		//mod_stack_return(stack);


	}
	else if(event==EV_DRAM_BANK_SCHEDULE)
	{
		if(dram->channel_avail[channel_id]>esim_cycle)
		{
			dram_debug("delay\n" );
			buffer->avail_cycle = esim_cycle + dram->dram_cycle;
			esim_schedule_event(event,stack,dram->channel_avail[channel_id]-esim_cycle);
			mod->dram_bus_waiting_cycle +=dram->channel_avail[channel_id]-esim_cycle;
			return;
		}
		/*for(i = 0; i < banks; i++)
		  {
		  next_bank_grand = ((i+dram->bank_grand)%banks);
		  if(next_bank_grand == dram->bank_grand)
		  {
		  buffer = &dram->buffer[next_bank_grand];
		  cur_ptr = buffer->cur_request;

		  if(cur_ptr!=buffer->new_request)
		  {
		  entry = &buffer->entry[cur_ptr];


		  break;
		  }
		  }

		  }
		  if(i == banks)
		  fatal("DRAM NO REQUEST");
		 */	
		while(1)
		{
			cur_ptr = buffer->cur_request;
			entry = &buffer->entry[cur_ptr];
			if(entry->valid)
				break;
			buffer->cur_request = (cur_ptr +1 )%DRAM_BUFFER_SIZE;	
			if(buffer->cur_request==buffer->new_request)
				fatal("DRAM EMPTY QUEUE");
		}

		buffer->waiting_request--;
		dram_debug("schedule bank%d waiting=%d ",bank_id,buffer->waiting_request);
		
		dram->bw_consumed[entry->stack->core_id]++;
		buffer->cur_requester_id = entry->stack->core_id;
		if(dram->esim_dram_cycles<(esim_cycle/dram->dram_cycle))
		{
			dram->esim_dram_cycles=(esim_cycle/dram->dram_cycle);
		}
		
		if(entry->stack->demand_access)
			mod->dram_demand_queuing_cycle +=esim_cycle-entry->stack->dram_buffer_insert_cycle;
		else if(entry->stack->prefetch)
			mod->dram_prefetch_queuing_cycle +=esim_cycle-entry->stack->dram_buffer_insert_cycle;
		else
			mod->dram_others_queuing_cycle +=esim_cycle-entry->stack->dram_buffer_insert_cycle;

		dram->channel_avail[channel_id] = esim_cycle + dram->dram_cycle;
		if(buffer->row_buffer_id == entry->row)//row_buffer hit
		{
			dram_debug("rowbuffer hit\n");
			mod->dram_rowbuffer_hit++;
			buffer->avail_cycle = esim_cycle + dram->dram_column_latency;
			esim_schedule_event(EV_DRAM_COL_ACCESS,entry->stack,dram->dram_column_latency);
		}
		else
		{
			dram_debug("\n");
			buffer->avail_cycle = esim_cycle + dram->dram_precharge_latency + dram->dram_cycle;
			esim_schedule_event(EV_DRAM_PRECHARGE,entry->stack,dram->dram_precharge_latency);
		}
		dram_calc_bw_needed(dram);	
		free(stack);

	}

	else if (event == EV_DRAM_DATA_BUS)
	{
		dram_debug("data return ");
		if(dram->data_bus_avail[channel_id]>esim_cycle)
		{
			dram_debug("delay\n" );
			esim_schedule_event(event,stack,dram->data_bus_avail[channel_id]-esim_cycle);
			return;
		}
		dram->data_bus_avail[channel_id] = esim_cycle + dram->dram_burst_length;
		
		//mod->dram_access_latency+=dram->data_bus_avail[channel_id]-stack->dram_buffer_insert_cycle;
		
		if(stack->demand_access)
		{
			assert(stack->prefetch==0&&stack->write_back==0);
			mod->dram_access_demand++;
			mod->dram_access_latency_demand+=dram->data_bus_avail[channel_id]-stack->dram_buffer_insert_cycle;
		}
		else if (stack->prefetch)
		{
			assert(stack->write_back==0);
			mod->dram_access_prefetch++;
                        mod->dram_access_latency_prefetch+=dram->data_bus_avail[channel_id]-stack->dram_buffer_insert_cycle;
		}
		else
		{	
			if(stack->write_back==0)
				fatal("Not write back");
			mod->dram_access_others++;
                        mod->dram_access_latency_others+=dram->data_bus_avail[channel_id]-stack->dram_buffer_insert_cycle;
		}		
	
		dram_debug("\n");

		mod_stack_return(stack);
	}
	else if (event == EV_DRAM_CHANNEL_SCHEDULE)
	{
		fatal ("FIXME_CHANNEL_SCHEDULE");
	}
	else
	{
		fatal ("Unknown EVENT");
	}
}
struct dram_ctrl* dram_init(struct mod_t * memory)
{
	struct dram_ctrl* dram;
	//int log_block_size;
	int banks,channel,bank_per_channel;
	int i;
	//banks = memory->dram_num_banks;
	channel = memory->dram_num_channel;
	//bank_per_channel = banks/channel;
	bank_per_channel = memory->dram_num_banks;
	banks = bank_per_channel * channel;
	if(banks==0||channel==0||(bank_per_channel*channel!=banks))
	{
		fprintf(stderr,"Error in Dram-creating\n");
		return NULL;
	}
	//dram =NULL;

	dram = xcalloc(1, sizeof(struct dram_ctrl));	
	dram_debug("dram create\n");

	dram->dram_num_banks = 	banks;
	dram->dram_num_channel = channel;
	dram->dram_bank_per_channel = bank_per_channel;

	//dram->bank_grand = 0;
	//dram->cur_used_banks = 0;

	dram->cid_msk = (1<<DRAM_ROWBUFFER_LOG_SIZE)-1;
	dram->rid_shift = DRAM_ROWBUFFER_LOG_SIZE + log_base2(banks);

	dram->bid_shift = DRAM_ROWBUFFER_LOG_SIZE  ;
	dram->bid_msk = (banks)-1;	

	dram->dram_cycle	 	= 3;
	dram->dram_activate_latency 	= 10 * dram->dram_cycle;
	dram->dram_precharge_latency	= 10 * dram->dram_cycle;
	dram->dram_column_latency   	= 10 * dram->dram_cycle;

	dram->dram_burst_length 	= dram->dram_cycle * ((memory->block_size>>4)?(memory->block_size>>4):1);
	
	dram->channel_avail = xcalloc(channel,sizeof(long long ));
	dram->data_bus_avail= xcalloc(channel,sizeof(long long ));
	dram->buffer = xcalloc(banks, sizeof(struct dram_buffer));
	
	dram->bw_consumed = xcalloc(x86_cpu_num_cores,sizeof(int));
	dram->bw_needed = xcalloc(x86_cpu_num_cores,sizeof(int));

	dram->HPAC_BWC = xcalloc(x86_cpu_num_cores,sizeof(long long *));
	dram->HPAC_BWNO= xcalloc(x86_cpu_num_cores,sizeof(long long *));
	dram->HPAC_enable = 0;
	
	dram->esim_dram_cycles = 0;
	for( i = 0; i < banks; i++)
	{
		dram->buffer[i].new_request = 0;
		dram->buffer[i].cur_request = 0;
		dram->buffer[i].row_buffer_id = ~0;
		dram->buffer[i].avail_cycle = 0;
		dram->buffer[i].entry = xcalloc(DRAM_BUFFER_SIZE, sizeof(struct dram_buffer_entry));
		dram->buffer[i].requester_counter = xcalloc(x86_cpu_num_cores,sizeof(int));
		dram->buffer[i].waiting_request = 0;
	}
	return dram;
}

void dram_free(struct dram_ctrl *dram_ctrl)
{
	int i;
	for( i = 0; i < dram_ctrl->dram_num_banks; i++)
	{
		free(dram_ctrl->buffer[i].entry);
		free(dram_ctrl->buffer[i].requester_counter);

	}
	free(dram_ctrl->buffer);
	free(dram_ctrl->channel_avail);
	free(dram_ctrl->data_bus_avail);
	free(dram_ctrl->bw_consumed);
	free(dram_ctrl->bw_needed);
	free(dram_ctrl);
}

void dram_calc_bw_needed(struct dram_ctrl *dram_ctrl)
{
	int i,j;
	struct dram_buffer *buffer;
	//int *bw_needed;
	//bw_needed = xcalloc(x86_cpu_num_cores,sizeof(int));
	int bw_needed[32];
	if(x86_cpu_num_cores>=32||dram_ctrl->HPAC_enable==0)
		return;
	dram_ctrl->HPAC_BWN = 0;
	for( i = 0; i < dram_ctrl->dram_num_banks; i++)
	{
		buffer = &dram_ctrl->buffer[i];
		for ( j = 0; j < x86_cpu_num_cores; j++)
		{
			if( j == buffer->cur_requester_id )
				continue;
			if( buffer->requester_counter[j] )
				bw_needed[j]++;
		}
	}
	
	for ( j = 0; j < x86_cpu_num_cores; j++)
        {
		dram_ctrl->bw_needed[j]=bw_needed[j];
		dram_ctrl->HPAC_BWN += bw_needed[j];
		dram_debug("core=%d bw=%d  ",j,bw_needed[j]);
	}
	dram_debug("\n");
	//free(bw_needed);
}
