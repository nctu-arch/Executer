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
 *                              */

#include <assert.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>

#include "prefetcher.h"
#include "throttle.h"
#include "mem-system.h"
#include "module.h"


int EV_THROTTLE_START;
int EV_THROTTLE_SCHEDULE;
int EV_THROTTLE_CYCLE_FIRST;
int EV_THROTTLE_CYCLE_REPEAT;
int EV_THROTTLE_EVENT_TRIGGER;//cjc_20130124

int EV_THROTTLE_NEW_COUNTER;


int throttle_debug_category;

#define FDP_AGG_MAX 4
int FDP_TABLE[12] = {1,1,0,-1,1,-1,0,-1,-1,-1,0,-1}; //cjc_20130716 FDP table2 case1~12
int FDP_DEGREE_TABLE[5] = {1,1,2,4,4};
int FDP_DISTANCE_TABLE[5] = {4,8,16,32,64};

const int HPAC_pollution_threshold = 90;
const int HPAC_BWC_threshold = 50000;
const int HPAC_BWNO_threshold = 75000;
int HPAC_GLOBAL_DECISION[16] = {1,0,-1-1,1,1,1,1,-1,-1,-1,-1,1,0,0,-1};

struct str_map_t throttle_type_map =
{
        3, {
                { "NONE", throttle_triggle_type_none },
                { "CYCLE", throttle_triggle_type_cycle },
                { "EVENT", throttle_triggle_type_event },
        }
};

struct str_map_t throttle_desicion_map =
{
	3, {
		{"PAM", throttle_desicion_type_pam},
		{"FDP",throttle_desicion_type_fdp},
		{"HPAC",throttle_desicion_hpac},
	}
};

struct throttle_t* throttle_create(char *name,struct prefetcher_t *pref,enum throttle_triggle_type type ,int period,struct monitor_counter * hw_counter)
{
	struct throttle_t* thr;
	int i;
	thr = xcalloc(1, sizeof(struct throttle_t));
	thr->pref = pref;
	thr->type = type;
	thr->period = period;
	thr->name=xstrdup(name);

	thr->FDP_aggressiveness = 2;
	thr->first = 1;
	thr->pending = 0;

	thr->last_dead_block_ratio = 1.0;	
	thr->dead_acc_counter = 0;

	thr->monitor_counter.pf_hit=hw_counter->pf_hit;
       	thr->monitor_counter.pf_issued=hw_counter->pf_issued;
        thr->monitor_counter.dead_block=hw_counter->dead_block;
	thr->monitor_counter.dead_block_demand_caused_by_pf=hw_counter->dead_block_demand_caused_by_pf;
	thr->monitor_counter.FDP_pollution = hw_counter->FDP_pollution;
	thr->monitor_counter.pf_late = hw_counter->pf_late;
	thr->monitor_counter.pf_pollution_HPAC =  hw_counter->pf_pollution_HPAC;
	thr->monitor_counter.HPAC_BWNO =  hw_counter->HPAC_BWNO;
	thr->monitor_counter.HPAC_BWC =  hw_counter->HPAC_BWC;
	
	thr->monitor_counter_older.pf_hit=xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.pf_issued=xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.dead_block=xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.dead_block_demand_caused_by_pf=xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.FDP_pollution = xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.pf_late = xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.pf_pollution_HPAC = xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.HPAC_BWNO = xcalloc(1, sizeof(long long));
	thr->monitor_counter_older.HPAC_BWC = xcalloc(1, sizeof(long long));

	thr->current_counter.pf_hit=xcalloc(1, sizeof(long long));
        thr->current_counter.pf_issued=xcalloc(1, sizeof(long long));
        thr->current_counter.dead_block=xcalloc(1, sizeof(long long));
	thr->current_counter.dead_block_demand_caused_by_pf=xcalloc(1, sizeof(long long));
	thr->current_counter.FDP_pollution = xcalloc(1, sizeof(long long));
	thr->current_counter.pf_accuracy_ratio=hw_counter->pf_accuracy_ratio;
	thr->current_counter.pf_accuracy_ratio_low=hw_counter->pf_accuracy_ratio_low;
	thr->current_counter.dead_block_threshold=hw_counter->dead_block_threshold;
	thr->current_counter.pf_dead_block_ratio=hw_counter->pf_dead_block_ratio;
	thr->current_counter.pollution_ratio =hw_counter->pollution_ratio;
	thr->current_counter.lateness_ratio = hw_counter->lateness_ratio;
	thr->current_counter.pf_late = xcalloc(1, sizeof(long long));
	thr->current_counter.pf_pollution_HPAC = xcalloc(1, sizeof(long long));
	thr->current_counter.HPAC_BWNO = xcalloc(1, sizeof(long long));
	thr->current_counter.HPAC_BWC = xcalloc(1, sizeof(long long));
	throttle_debug("Throttle %s created : pf_accuracy_ratio = %lf pf_accuracy_ratio_low = %lf dead_block_threshold = %d pf_dead_block_ratio=%lf pollution_ratio =%lf lateness_ratio=%lf\n"			,thr->name,thr->current_counter.pf_accuracy_ratio,thr->current_counter.pf_accuracy_ratio_low,thr->current_counter.dead_block_threshold
		,thr->current_counter.pf_dead_block_ratio,thr->current_counter.pollution_ratio,thr->current_counter.lateness_ratio);

	for(i=0;i<MAX_DEGREE;i++)
		thr->thr_degree_distribution[i]=0;
	thr->thr_dest_distribution[0]=0;
	thr->thr_dest_distribution[1]=0;

	if(type==throttle_triggle_type_cycle)
	{
		throttle_debug(" type=timing\n");
		thr->status = throttle_working_status_idle;
	}
	else if(type==throttle_triggle_type_event)
	{
		throttle_debug(" type=event\n");
		thr->status = throttle_working_status_idle;
	}
	else if(type==throttle_triggle_type_none)
	{
		//disable throttle
		throttle_debug(" type=none\n");
		thr->status = throttle_working_status_suspend;
	}
	else
		fatal("Undefined type of throttle");

	return thr;	
	
}
void throttle_free(struct throttle_t* thr)
{
	free(thr->monitor_counter_older.pf_hit);
	free(thr->monitor_counter_older.pf_issued);
	free(thr->monitor_counter_older.dead_block);
	free(thr->monitor_counter_older.dead_block_demand_caused_by_pf);
	free(thr->monitor_counter_older.FDP_pollution);
	free(thr->monitor_counter_older.pf_late);
	free(thr->monitor_counter_older.pf_pollution_HPAC);
	free(thr->monitor_counter_older.HPAC_BWNO);
	free(thr->monitor_counter_older.HPAC_BWC);

	free(thr->current_counter.pf_hit);
        free(thr->current_counter.pf_issued);
        free(thr->current_counter.dead_block);
	free(thr->current_counter.dead_block_demand_caused_by_pf);
	free(thr->current_counter.FDP_pollution);
	free(thr->current_counter.pf_late);
	free(thr->current_counter.pf_pollution_HPAC);
	free(thr->current_counter.HPAC_BWNO);
	free(thr->current_counter.HPAC_BWC);
	free(thr->name);
	free(thr);
}
void throttle_handler(int event, void *data)
{
	struct throttle_t * thr;
	thr = (struct throttle_t *)data;
	if(event==EV_THROTTLE_START)
	{	
		if(thr->pref->type!=prefetcher_type_stride||thr->status==throttle_working_status_busy)
			return;
		//assert(thr->status!=throttle_working_status_busy);
		assert(thr->type==throttle_triggle_type_cycle||thr->type==throttle_triggle_type_event);
		
		if(thr->status==throttle_working_status_idle)
			throttle_debug("%lld Throttle %s start\n",mod_current_esim_cycle(),thr->name);
		else if(thr->status==throttle_working_status_suspend)
                        throttle_debug("%lld Throttle %s restart\n",mod_current_esim_cycle(),thr->name);
		
		thr->status=throttle_working_status_busy;
		//thr->first = 0;	
		throttle_debug("prefetch status: Degree=%d, Destination=L%d\n",thr->pref->pf_degree,((thr->pref->lower_level_insertion==0)?1:2));	
		if(thr->type==throttle_triggle_type_cycle)
		{
			assert(thr->period);
			assert(thr->pref);
			esim_schedule_event(EV_THROTTLE_CYCLE_FIRST,thr,thr->period);
		}
		else if (thr->type==throttle_triggle_type_event)
		{	
			assert(thr->pref);
			//esim_schedule_event(EV_THROTTLE_EVENT_TRIGGER,thr,0);
		}
		return;
	}
	else if (event==EV_THROTTLE_CYCLE_FIRST)
	{
		throttle_counter_assign(&thr->monitor_counter_older,NULL);
		throttle_counter_assign(&thr->current_counter,&thr->monitor_counter);
		throttle_debug("%lld %s throttle cycle first, period = %d\n",mod_current_esim_cycle(),thr->name,thr->period);	
		assert(thr->pref);
		throttle_debug("%s ",thr->name);	
		throttle_print_counter(&thr->monitor_counter);
		throttle_throttling_prefetcher(thr);	
		
		thr->pending=1;
		//esim_schedule_event(EV_THROTTLE_CYCLE_REPEAT,thr,thr->period);
		return;
	}
	else if (event==EV_THROTTLE_CYCLE_REPEAT)
        {
		throttle_debug("%lld %s throttle cycle repeat, period = %d\n",mod_current_esim_cycle(),thr->name,thr->period);
		//throttle_print_counter(&thr->monitor_counter);

		throttle_update_counter(&thr->current_counter,&thr->monitor_counter,&thr->monitor_counter_older);	
                assert(thr->pref);
               	throttle_debug("%s ",thr->name);	
		throttle_print_counter(&thr->current_counter);
		
		//if(thr->status==throttle_working_status_busy)
			throttle_throttling_prefetcher(thr);
		
		throttle_counter_assign(&thr->monitor_counter_older,&thr->monitor_counter);
               
		if(thr->pending==0)
		{
		//	esim_schedule_event(EV_THROTTLE_CYCLE_REPEAT,thr,thr->period);
			thr->pending=1;
		}
		return;
        }
	else if (event==EV_THROTTLE_NEW_COUNTER)
	{
		if(thr->pending==0)
			return;
		if(thr->type==throttle_triggle_type_cycle)
                {
			thr->pending=0;
                        assert(thr->period);
                        esim_schedule_event(EV_THROTTLE_CYCLE_REPEAT,thr,thr->period);
                }
                else //if (thr->type==throttle_triggle_type_event)
                        fatal("FIX ME, UNSUPPORTED TYPE");

                return;
	}

	else if (event==EV_THROTTLE_EVENT_TRIGGER)
	{
		if(thr->status!=throttle_working_status_busy||thr->type!=throttle_triggle_type_event)
			return;
		if(thr->first==1)
		{
			throttle_counter_assign(&thr->monitor_counter_older,NULL);
			throttle_counter_assign(&thr->current_counter,&thr->monitor_counter);
			throttle_debug("%lld %s throttle event first \n",mod_current_esim_cycle(),thr->name);
			assert(thr->pref);

			throttle_debug("%s ",thr->name);
			throttle_print_counter(&thr->monitor_counter);
			throttle_throttling_prefetcher(thr);
			thr->first = 0;
			return;
		}
		else
		{
			assert(thr->first == 0);
			throttle_debug("%lld %s throttle event repeat \n",mod_current_esim_cycle(),thr->name);
			//throttle_print_counter(&thr->monitor_counter);

			throttle_update_counter(&thr->current_counter,&thr->monitor_counter,&thr->monitor_counter_older);
			assert(thr->pref);

			throttle_debug("%s ",thr->name);
			throttle_print_counter(&thr->current_counter);

			throttle_throttling_prefetcher(thr);

			throttle_counter_assign(&thr->monitor_counter_older,&thr->monitor_counter);

			return;	
		}
	}

	else
		fatal("UNDEFINED THROTTLE EVENT");
}
void throttle_counter_assign(struct monitor_counter *lhs,struct monitor_counter *rhs)
{
	if(rhs)
	{
		*(lhs->pf_hit)=*(rhs->pf_hit);
		*(lhs->pf_issued)=*(rhs->pf_issued);
		*(lhs->dead_block)=*(rhs->dead_block);
		*(lhs->dead_block_demand_caused_by_pf)=*(rhs->dead_block_demand_caused_by_pf);
	}
	else
	{
		*(lhs->pf_hit)=0;
		*(lhs->pf_issued)=0;
		*(lhs->dead_block)=0;
		*(lhs->dead_block_demand_caused_by_pf)=0;
	}
}
void throttle_throttling_prefetcher(struct throttle_t* thr) 
{
	assert(thr->pref);
	double accuracy,pf_accuracy_ratio,pf_accuracy_ratio_low,lateness,pollution;
	double dead_block_ratio,dead_block_ratio_std;
	double pollution_ratio;
        double lateness_ratio;
	long long dead_block_count,dead_block_threshold;
	int degree,destination;	
	int fdp_case;
	int hpac_case;

	degree = thr->pref->pf_degree;
	destination=thr->pref->lower_level_insertion;
	dead_block_count = *(thr->current_counter.dead_block);
	pf_accuracy_ratio =  thr->current_counter.pf_accuracy_ratio;
	pf_accuracy_ratio_low =  thr->current_counter.pf_accuracy_ratio_low;
	dead_block_threshold = thr->current_counter.dead_block_threshold;
	dead_block_ratio_std = thr->current_counter.pf_dead_block_ratio;
	pollution_ratio = thr->current_counter.pollution_ratio;
	lateness_ratio = thr->current_counter.lateness_ratio;

	if(*(thr->current_counter.pf_issued)==0)
	{
		accuracy = 0;
		dead_block_ratio = 0;
		lateness = 0;
		pollution = 0;
		
	}
	else
	{
		accuracy = (double)*(thr->current_counter.pf_hit)/(double)*(thr->current_counter.pf_issued);
		dead_block_ratio = (double)*(thr->current_counter.dead_block_demand_caused_by_pf)/(double)*(thr->current_counter.pf_issued);
		pollution = (double)*(thr->current_counter.FDP_pollution)/(double)*(thr->current_counter.pf_issued);
		lateness  = (double)*(thr->current_counter.pf_late)/(double)*(thr->current_counter.pf_issued);
		
	}
	throttle_debug("%s pf_accuracy=%lf dead_block_count=%lld dead_block_ratio=%lf pollution_ratio=%lf lateness_ratio=%lf \n",thr->name,accuracy,dead_block_count,dead_block_ratio,pollution_ratio,lateness_ratio);
	
	//FDP decision
	if(thr->decision_method == throttle_desicion_type_fdp)	
	{	
		if(accuracy>pf_accuracy_ratio)
			fdp_case = 0;
		else if(accuracy<pf_accuracy_ratio_low)
			fdp_case = 8;
		else
			fdp_case = 4;
		if(lateness<lateness_ratio)
			fdp_case +=2;
		if(pollution>pollution_ratio)
			fdp_case +=1;
		assert(fdp_case>=0&&fdp_case<12);		

		thr->FDP_aggressiveness +=FDP_TABLE[fdp_case];

		if(thr->FDP_aggressiveness<0)
			thr->FDP_aggressiveness = 0;
		else if(thr->FDP_aggressiveness>FDP_AGG_MAX)
			thr->FDP_aggressiveness=FDP_AGG_MAX;

		thr->pref->pf_degree = FDP_DEGREE_TABLE[thr->FDP_aggressiveness];
		if(thr->pref->type == prefetcher_type_stream)
			thr->pref->pf_distance = FDP_DISTANCE_TABLE[thr->FDP_aggressiveness];
	}
	else if (thr->decision_method ==throttle_desicion_type_pam)
	{
		if(thr->first==1)
		{
			thr->last_dead_block_ratio = dead_block_ratio;
		}
		else
		{
			if(dead_block_ratio>thr->last_dead_block_ratio)
				thr->dead_acc_counter = (thr->dead_acc_counter==1)?2:1;
			else
				thr->dead_acc_counter = (thr->dead_acc_counter==-1)?-2:-1;	
			if(thr->dead_acc_counter==2)
			{
				thr->dead_acc_counter = 1;
				thr->pref->pf_degree = (degree<=1)?1:degree>>1;
			}		
			else if (thr->dead_acc_counter==-2)
			{
				thr->dead_acc_counter = -1;
				thr->pref->pf_degree = (degree>=MAX_DEGREE)?MAX_DEGREE:degree<<1;
			}
			thr->last_dead_block_ratio = dead_block_ratio;
			if(dead_block_ratio<dead_block_ratio_std)
				thr->pref->lower_level_insertion = (destination==0)?0:destination-1;
			else
				thr->pref->lower_level_insertion = (destination>=1)?1:destination+1;
		}
	}
	
	else if (thr->decision_method == throttle_desicion_hpac)
	{
		if(accuracy>pf_accuracy_ratio)
                        fdp_case = 0;
                else if(accuracy<pf_accuracy_ratio_low)
                        fdp_case = 8;
                else
                        fdp_case = 4;
                if(lateness<lateness_ratio)
                        fdp_case +=2;
                if(pollution>pollution_ratio)
                        fdp_case +=1;
                assert(fdp_case>=0&&fdp_case<12);

                //thr->FDP_aggressiveness +=FDP_TABLE[fdp_case];
	
		//global control
		if(*(thr->current_counter.pf_pollution_HPAC) >HPAC_pollution_threshold)
			hpac_case = 8;
		else 
			hpac_case = 0;
		if(accuracy > 0.6)
			hpac_case +=4;
		if(*(thr->current_counter.HPAC_BWNO) > HPAC_BWNO_threshold)
			hpac_case +=2;
		if(*(thr->current_counter.HPAC_BWC) > HPAC_BWC_threshold)
			hpac_case +=1;
		if(HPAC_GLOBAL_DECISION[hpac_case]==1)
			thr->FDP_aggressiveness +=FDP_TABLE[fdp_case];
		else if (HPAC_GLOBAL_DECISION[hpac_case]==-1)
			thr->FDP_aggressiveness -=1;
		else if (FDP_TABLE[fdp_case]<=0)
			thr->FDP_aggressiveness +=FDP_TABLE[fdp_case];
			
		//global end
                if(thr->FDP_aggressiveness<0)
                        thr->FDP_aggressiveness = 0;
                else if(thr->FDP_aggressiveness>FDP_AGG_MAX)
                        thr->FDP_aggressiveness=FDP_AGG_MAX;

                thr->pref->pf_degree = FDP_DEGREE_TABLE[thr->FDP_aggressiveness];
                if(thr->pref->type == prefetcher_type_stream)
                        thr->pref->pf_distance = FDP_DISTANCE_TABLE[thr->FDP_aggressiveness];
	}
	/*if(accuracy<pf_accuracy_ratio_low)
	  thr->pref->pf_degree = (degree==1)?1:degree>>1;
	  else if(accuracy>pf_accuracy_ratio)
	  thr->pref->pf_degree = (degree>=MAX_DEGREE)?MAX_DEGREE:degree<<1;
	 */
	/*if(dead_block_count<dead_block_threshold)
	  thr->pref->lower_level_insertion = (destination==0)?0:destination-1;
	  else
	  thr->pref->lower_level_insertion = (destination>=1)?1:destination+1;*/


	//throttle_debug("New prefetch status: Degree=%d, Destination=L%d\n",thr->pref->pf_degree,(thr->pref->lower_level_insertion+thr->name[5]-'0'));
	throttle_debug("%s New prefetch status Degree=%d Destination=L%d\n",thr->name,thr->pref->pf_degree,((destination==0)?1:2));
	thr->thr_degree_distribution[degree-1]++;
	thr->thr_dest_distribution[(destination==0)?0:1]++;
}
void throttle_update_counter(struct monitor_counter * lhs,struct monitor_counter * operand1,struct monitor_counter * operand2)
{
	*(lhs->pf_hit)=(*(lhs->pf_hit)+*(operand1->pf_hit)-*(operand2->pf_hit))>>1;
	*(lhs->pf_issued)=(*(lhs->pf_issued)+*(operand1->pf_issued)-*(operand2->pf_issued))>>1;
	*(lhs->dead_block)=(*(lhs->dead_block)+*(operand1->dead_block)-*(operand2->dead_block))>>1;
	*(lhs->dead_block_demand_caused_by_pf)=(*(lhs->dead_block_demand_caused_by_pf)+
			*(operand1->dead_block_demand_caused_by_pf)-*(operand2->dead_block_demand_caused_by_pf))>>1;
	*(lhs->FDP_pollution) = (*(lhs->FDP_pollution)+*(operand1->FDP_pollution)-*(operand2->FDP_pollution))>>1;
	*(lhs->pf_late) = (*(lhs->pf_late)+*(operand1->pf_late)-*(operand2->pf_late))>>1;
	*(lhs->pf_pollution_HPAC) = (*(lhs->pf_pollution_HPAC)+*(operand1->pf_pollution_HPAC)-*(operand2->pf_pollution_HPAC))>>1;
	*(lhs->HPAC_BWC) = (*(lhs->HPAC_BWC)+*(operand1->HPAC_BWC)-*(operand2->HPAC_BWC))>>1;
	*(lhs->HPAC_BWNO) = (*(lhs->HPAC_BWNO)+*(operand1->HPAC_BWNO)-*(operand2->HPAC_BWNO))>>1;
}
void throttle_print_counter(struct monitor_counter * target)
{
	throttle_debug("pf_hit=%lld pf_issued=%lld dead_block=%lld dead_block_by_pf=%lld FDP_pollution=%lld pf_late=%lld\n",*(target->pf_hit),*(target->pf_issued),
			*(target->dead_block),*(target->dead_block_demand_caused_by_pf),*(target->FDP_pollution),*(target->pf_late));
}
void throttle_dump_report(FILE *f,struct throttle_t* thr)
{
	long long total;
	int i;
	total = 0;
	fprintf(f, "%s_Thr_degree_distribution\n",thr->name);
	for(i=0;i<MAX_DEGREE;i++)
	{
		total+=thr->thr_degree_distribution[i];
		fprintf(f, "%s_Thr_degree%d = %lld\n", thr->name,i+1,thr->thr_degree_distribution[i]);	
	}
	fprintf(f, "%s_Thr_degree_total = %lld\n\n", thr->name,total);
	fprintf(f, "%s_Thr_destination_distribution\n",thr->name);
	fprintf(f, "%s_Thr_destination_cur_level = %lld\n",thr->name,thr->thr_dest_distribution[0]);
	fprintf(f, "%s_Thr_destination_lower_level = %lld\n",thr->name,thr->thr_dest_distribution[1]);
	fprintf(f, "%s_Thr_destination_total = %lld\n",thr->name,thr->thr_dest_distribution[1]+thr->thr_dest_distribution[0]); 
	fprintf(f, "\n\n");
}
