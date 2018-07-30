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

#ifndef MEM_SYSTEM_DRAM_H
#define MEM_SYSTEM_DRAM_H

#define DRAM_ROWBUFFER_LOG_SIZE (14)

#define DRAM_BUFFER_SIZE (256)

extern int EV_DRAM_ACTIVATE;
extern int EV_DRAM_PRECHARGE;
extern int EV_DRAM_COL_ACCESS;
extern int EV_DRAM_BANK_SCHEDULE;
extern int EV_DRAM_CHANNEL_SCHEDULE;
extern int EV_DRAM_BANK_STATUS;
extern int EV_DRAM_DATA_BUS;

enum dram_schedule
{
	dram_sche_fcfs = 0,
	dram_sche_frfcfs,

};
enum dram_banks_status
{
        dram_bank_status_idle = 0,
        dram_bank_status_precharge,
        dram_bank_status_activate,
	dram_bank_status_col_access,
};

//extern int dram_debug_category;

void  dram_set_drb_entry(struct mod_stack_t *stack);
struct dram_ctrl* dram_init(struct mod_t * memory);
void dram_free(struct dram_ctrl *dram_ctrl);
struct dram_buffer_entry
{
	int valid;
	unsigned int row, column;
        struct mod_stack_t *stack;
	
	long long inserted_cycle;

};
struct dram_buffer
{
	//struct dram_buffer_entry *head;
	int new_request;
	int cur_request;
	int cur_requester_id;
	enum dram_banks_status status;
	unsigned int row_buffer_id;
	long long  avail_cycle;
	struct dram_buffer_entry *entry;
	int *requester_counter;	
	int waiting_request;
};

struct dram_ctrl
{
	int dram_num_banks;
	int dram_num_channel;
	int dram_bank_per_channel;

	unsigned int bid_msk,cid_msk;
	unsigned int bid_shift,rid_shift;
		
	struct dram_buffer *buffer;

	long long *channel_avail;
	long long *data_bus_avail;
	int *bw_consumed;
	int *bw_needed;
	int HPAC_enable;
	int HPAC_BWN;
	long long **HPAC_BWC;
	long long **HPAC_BWNO;
	// latency
	int dram_cycle;
	int dram_burst_length;	
	int dram_activate_latency; /**< activate latency */
	int dram_precharge_latency; /**< precharge latency */
	int dram_column_latency; /**< column access latency */
	long long esim_dram_cycles;
};

void dram_handler(int event, void *data);
void dram_calc_bw_needed(struct dram_ctrl *dram_ctrl);
#endif  

