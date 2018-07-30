/*
 * Version          : $Revision: 1.0 $
 * Last Modified On : $Date: 2012/12/14
 * Last Modified By : $Author: cjc
 *----------------------------------------------------------------------
 * Description      :
 *                    For throttling L1-prefetcher
 *----------------------------------------------------------------------
 * Modification List:
 *
 *				*/

#ifndef MEM_SYSTEM_THROTTLE_H
#define MEM_SYSTEM_THROTTLE_H	


#define MAX_DEGREE 4

// --- 1.0 --

extern int EV_THROTTLE_START;
extern int EV_THROTTLE_SCHEDULE;
extern int EV_THROTTLE_CYCLE_FIRST;
extern int EV_THROTTLE_CYCLE_REPEAT;
extern int EV_THROTTLE_NEW_COUNTER;

extern int EV_THROTTLE_EVENT_TRIGGER;

extern struct str_map_t throttle_type_map;

extern struct str_map_t throttle_desicion_map;
enum throttle_triggle_type
{
	throttle_triggle_type_none = 0,
	throttle_triggle_type_cycle, 
	throttle_triggle_type_event,
};
enum throttle_desicion_type
{
	throttle_desicion_type_pam = 0,
	throttle_desicion_type_fdp,
	throttle_desicion_hpac,
};

enum throttle_working_status
{
	throttle_working_status_idle = 0,
	throttle_working_status_busy,
	throttle_working_status_suspend,
};

struct monitor_counter
{
	long long *pf_hit;
	long long *pf_issued;
	long long *dead_block;
	long long *dead_block_demand_caused_by_pf;
	long long *FDP_pollution;
	long long *pf_late;	
	long long *pf_pollution_HPAC;
	long long *HPAC_BWC;
	long long *HPAC_BWNO;
	int dead_block_threshold;
	double pf_accuracy_ratio;
	double pf_accuracy_ratio_low;
	double pf_dead_block_ratio;
	double pollution_ratio;
	double lateness_ratio;
};
struct throttle_t
{
	char *name;
	struct prefetcher_t *pref;
	int period;
	int pending;
	int first;
	int FDP_aggressiveness;
	struct monitor_counter monitor_counter;
	struct monitor_counter monitor_counter_older;
	struct monitor_counter current_counter;
	enum throttle_triggle_type type;
	enum throttle_working_status status;
	
	////cjc_20130220
	double last_dead_block_ratio;
	int dead_acc_counter;	
	////
	enum throttle_desicion_type decision_method;	
	
	//statistic
	long long thr_degree_distribution[MAX_DEGREE];
	long long thr_dest_distribution[2];
};
struct throttle_t* throttle_create(char *name,struct prefetcher_t *pref,enum throttle_triggle_type type ,int period,struct monitor_counter * hw_counter);
void throttle_free(struct throttle_t* thr);
void throttle_handler(int event, void *data);
void throttle_counter_assign(struct monitor_counter *lhs,struct monitor_counter *rhs);
void throttle_throttling_prefetcher(struct throttle_t* thr);
void throttle_update_counter(struct monitor_counter * lhs,struct monitor_counter * operand1,struct monitor_counter * operand2);
void throttle_print_counter(struct monitor_counter * target);
void throttle_dump_report(FILE *f,struct throttle_t* thr);
// --- 1.0 ---
#endif
