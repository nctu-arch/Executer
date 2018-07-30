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

#ifndef MEM_SYSTEM_MOD_STACK_H
#define MEM_SYSTEM_MOD_STACK_H

#include "module.h"


/* Current identifier for stack */
extern long long mod_stack_id;

/* Read/write request direction */
enum mod_request_dir_t
{
	mod_request_invalid = 0,
	mod_request_up_down,
	mod_request_down_up
};

/* ACK types */
enum mod_reply_type_t
{
	reply_none = 0,
	reply_ack ,
	reply_ack_data,
	reply_ack_data_sent_to_peer,
	reply_ack_error
};

/* Message types */
enum mod_message_type_t
{
	message_none = 0,
	message_clear_owner
};

//IC_S_2014_04_08
struct pf_hint_t
{
	int hint_address;
	unsigned int priority;
	//0: default
	//1: coherence state (S)
	//Priority: 1 > 0

};
//IC_S

/* Stack */
struct mod_stack_t
{
	long long id;
	enum mod_access_kind_t access_kind;
	int *witness_ptr;

	struct linked_list_t *event_queue;
	void *event_queue_item;
	struct mod_client_info_t *client_info;

	struct mod_t *mod;
	struct mod_t *target_mod;
	struct mod_t *except_mod;
	struct mod_t *peer;

	struct mod_port_t *port;

	unsigned int addr;
	unsigned int tag;
	int set;
	int way;
	int state;

	int src_set;
	int src_way;
	int src_tag;

	enum mod_request_dir_t request_dir;
	enum mod_message_type_t message;
	enum mod_reply_type_t reply;
	int reply_size;
	int retain_owner;
	int pending;

	/* Linked list of accesses in 'mod' */
	struct mod_stack_t *access_list_prev;
	struct mod_stack_t *access_list_next;

	/* Linked list of write accesses in 'mod' */
	struct mod_stack_t *write_access_list_prev;
	struct mod_stack_t *write_access_list_next;

	struct mod_stack_t *prefetch_access_list_prev;
	struct mod_stack_t *prefetch_access_list_next;
	/* Bucket list of accesses in hash table in 'mod' */
	struct mod_stack_t *bucket_list_prev;
	struct mod_stack_t *bucket_list_next;

	int fetch_level; //porshe_20121125 0 is L1 hit, 1 is L2 hit, and 2 is memory fetch
        long long  issue_cycle; //porshe_20121125 for fetch latency count
        int peer_to_peer_trans_flag; //porshe_20121128 $2$ transfer flag
		
	/* Flags */
	int hit : 1;
	int err : 1;
	int shared : 1;
	int read : 1;
	int write : 1;
	int nc_write : 1;
	int prefetch ;
	int blocking : 1;
	int writeback : 1;
	int eviction : 1;
	int retry : 1;
	int coalesced : 1;
	int port_locked : 1;
	
	int ideal_access :1;//cjc_20121220 this access will have no access latency
	int ideal_wait_MSHR:1;//cjc_20130107
	int demand_access:1;//cjc_20130305
	int skip_dram:1;//cjc_20130328
	int write_back:1;
	int pf_master:1;//cjc_20130402
	int low_insert:1;//cjc_20130608		
	int pf_issued:1;
	
	//IC_S_2014_04_08
	struct pf_hint_t *pf_hint_priority;
	//IC_E

	/* Message sent through interconnect */
	struct net_msg_t *msg;

	/* Linked list for waiting events */
	int waiting_list_event;  /* Event to schedule when stack is waken up */
	struct mod_stack_t *waiting_list_prev;
	struct mod_stack_t *waiting_list_next;

	/* Waiting list for locking a port. */
	int port_waiting_list_event;
	struct mod_stack_t *port_waiting_list_prev;
	struct mod_stack_t *port_waiting_list_next;

	/*porshe_20130813: waiting event for DRAMSIM2 access registeration*/
	int dramsim2_waiting_list_event;

	/* Waiting list. Contains other stacks waiting for this one to finish.
	 * Waiting stacks corresponds to slave coalesced accesses waiting for
	 * the current one to finish. */
	struct mod_stack_t *waiting_list_head;
	struct mod_stack_t *waiting_list_tail;
	int waiting_list_count;
	int waiting_list_max;

	/* Master stack that the current access has been coalesced with.
	 * This field has a value other than NULL only if 'coalesced' is TRUE. */
	struct mod_stack_t *master_stack;

	/* Events waiting in directory lock */
	int dir_lock_event;
	struct mod_stack_t *dir_lock_next;

	/* Return stack */
	struct mod_stack_t *ret_stack;
	int ret_event;
	
	//cjc_20121207
        long long  pf_wait_cycle;//esim_cycle when it begin to wait pf_request

	//cjc_20121227
	long long ideal_start_cycle;//esim_cycle when a ideal access started

	//cjc_20130315
	long long dram_buffer_insert_cycle;//esim_cycle

	int pf_source;//cjc_20130208
	struct mod_t *higher_mod;

	//cjc_20130315 dram parameter
	unsigned int bank_id,row_id,column_id;

	int core_id;//cjc_20130408
	int pc;//cjc_20130514
    ///////////////////CIC/////////////////////////////////////////////////
	int waiting_oldwrite_inflight_num;
	long long waiting_oldwrite_inflight_cycle;
    long long first_find_n_lock_cycle;
	long long ending_find_n_lock_cycle;
	int find_n_lock_issue_count;
//	int waiting_oi_num_find_n_lock;
//	int waiting_oi_prev;
//  long long waiting_oi_cycle_find_n_lock;

	long long mod_start_access_cycle; //record mod access time
	long long mod_finish_access_cycle;//

	int mshr_access_count;

	long long lock_port_start;//CIC
	long long lock_port_end;//CIC

	long long load_start; 
	long long load_lock; 
	long long load_find_n_lock; 
	long long load_action;
	long long load_miss_get;
	long long load_unlock;
	long long load_finish;


	long long store_start; 
	long long store_lock; 
	long long store_find_n_lock; 
	long long store_action;
	long long store_miss_get;
	long long store_unlock;
	long long store_finish;

	long long read_rq;
	long long read_rq_receive;
	long long read_rq_action; 
//	            |
//	            v
	long long read_rq_upd;             long long read_rq_dup;
	long long read_rq_upd_miss;        long long read_rq_wait_for_rqs;
	long long read_rq_upd_finish;      long long read_rq_dup_finish;
	long long read_dram;
	long long read_rq_reply;
	long long read_rq_finish;

	long long write_rq;
	long long write_rq_receive;
	long long write_rq_action;
	long long write_rq_exclusive;       long long write_rq_dup;
	long long write_rq_upd;             long long write_rq_dup_finish;
	long long write_rq_upd_finish;
	long long write_rq_reply;
	long long write_rq_finish;

	int old_write : 1;
	int inflight_ad : 1;
	int wait_store:1;
	///////////////////////////////////////////////////////////////////////////
	
	int is_stride_link;
	unsigned int stride_link_addr;
	int stream_entry_id;
	
};

struct mod_stack_t *mod_stack_create(long long id, struct mod_t *mod,
		unsigned int addr, int ret_event, struct mod_stack_t *ret_stack);
void mod_stack_return(struct mod_stack_t *stack);

void mod_stack_wait_in_mod(struct mod_stack_t *stack,
	struct mod_t *mod, int event);
void mod_stack_wakeup_mod(struct mod_t *mod);

void mod_stack_wait_in_port(struct mod_stack_t *stack,
	struct mod_port_t *port, int event);
void mod_stack_wakeup_port(struct mod_port_t *port);

void mod_stack_wait_in_stack(struct mod_stack_t *stack,
	struct mod_stack_t *master_stack, int event);
void mod_stack_wakeup_stack(struct mod_stack_t *master_stack);


#endif

