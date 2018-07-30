/*
 * Version          : $Revision: 1.0 $
 * Last Modified On : $Date: 2013/08/09
 * Last Modified By : $Author: porshe
 *----------------------------------------------------------------------
 * Description      :
 *                   DRAM module for DRAMSIM2
 *----------------------------------------------------------------------
 * Modification List:
 *
 *                              */

#ifndef MEM_SYSTEM_DRAMSIM2_H
#define MEM_SYSTEM_DRAMSIM2_H


extern int EV_DRAM_ACCESS;
extern int EV_DRAM_WAIT;
extern int EV_DRAM_DONE;

extern int in_flight_in_dram_count;

void dram_handler_dramsim2(int event, void *data);
void dram_init_dramsim2();
void dram_done();
void dramsim2_access_finish(unsigned int add, int module_num);

#endif  

