/*
 * Version          : $Revision: 1.0 $
 * Last Modified On : $Date: 2013/08/12
 * Last Modified By : $Author: porshe
 *----------------------------------------------------------------------
 * Description      :
 *                   DRAM access handler module  
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
#include <lib/util/list.h>

#include <arch/x86/timing/cpu.h>

#include <peripheral_systemc/systemc_interface.h> //porshe_20130819

#include "mod-stack.h"
#include "module.h"
#include "mem-system.h"
#include "dram-handler.h"


int EV_DRAM_ACCESS;
int EV_DRAM_WAIT;
int EV_DRAM_DONE;

static int dram_mod_id = 0;
struct list_t *dram_mod_list;

int in_flight_in_dram_count; //porshe_20130819: counter for evaluating the in-flight access 

void dram_handler_dramsim2(int event, void *data)
{
	struct mod_stack_t *stack = data;
	if(event==EV_DRAM_ACCESS)
	{//parse info to DRAMSIM2
		stack->dramsim2_waiting_list_event = EV_DRAM_DONE;
		mod_access_start(stack->mod, stack, stack->read? mod_access_load: mod_access_store);
		//porshe_20130819: pass to systemc_if to call DRAMSIM2
		/* //porshe_20150507_S: skip the DRAMSim simulation
		if(stack->read)
			systemc_interface_rd(stack->addr);
		else
			systemc_interface_wr(stack->addr);
		*/ //porshe_20150507_E
		//porshe_20130891:end
		esim_schedule_event(EV_DRAM_DONE, stack, 100); //porshe_20150507: just add the constant delay
		in_flight_in_dram_count ++;
	}
	/*else if(event==EV_DRAM_WAIT)
	{//Wait for DRAMSIM2 to finish
		if( list_count(command_list) && ((index = list_index_of(command_list, data)) != -1) )
		{
			list_remove_at(command_list, index);
			esim_schedule_event(EV_DRAM_DONE,stack,0);
			return;
		}

		esim_schedule_event(EV_DRAM_WAIT,stack,1);
	}*/
	else if(event==EV_DRAM_DONE)
	{//go back to M2S
		assert(in_flight_in_dram_count);
		in_flight_in_dram_count --;
		mod_access_finish(stack->mod, stack); //porshe_20150507: finish for constant latency
		mod_stack_return(stack);
	}
}

void dram_init_dramsim2 (struct mod_t *mod)
{
	if(!dram_mod_id)
	{
		dram_mod_list = list_create(); //list for mapping main memory module with index(dram_mod_id)
		in_flight_in_dram_count = 0;
	}
	list_insert(dram_mod_list, dram_mod_id, mod);
	dram_mod_id++;
	
}
void dram_done ()
{
	list_free(dram_mod_list);
}

void dramsim2_access_finish(unsigned int addr, int module_num)
{
	struct mod_t *mod = list_get(dram_mod_list, module_num); //lookup the mapping memory module
	struct mod_stack_t *stack = mod_in_flight_oldest_address(mod, addr);
	assert(stack); //if NULL, it's not an in-flight access
	mod_access_finish(mod, stack);

	int event = stack->dramsim2_waiting_list_event;
	esim_schedule_event(event,stack,0);
}
