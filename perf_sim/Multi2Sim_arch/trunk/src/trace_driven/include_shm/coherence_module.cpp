
#include "coherence_module.h"
#include "cache.h"
//#include <queue>

Coherence_module::Coherence_module() 
{   
	coherence_num = 0;
	latency = 1;
	//mcore_SetExclusive();
	//mcore_ExclusiveLatency(latency);
	cache_array.cache_count = 0;
/*
	bus_owner = -1;
	tick_queue = new std::queue<tick*>;
	tick_conflict_queue = new std::queue<tick*>;
	flag_bus_finish = true;
*/	
}
int Coherence_module::getid(norm_cache *obj)
{
        int id = cache_array.cache_count++;
        cache_array.cache_lines[id] = obj;
        return id;
}
/*
bool Coherence_module::check_bus_is_not_finish()
{
	if (flag_bus_finish == true) {
		flag_bus_finish = false;
		return false;
	} else {
		tick *tk = mcore_catch_request();
		tick_conflict_queue->push(tk);
		return true;
	}
}
*/
int Coherence_module::module_recv_rq(Request *rq)
{   
/*
	if (check_bus_is_not_finish()) {
		return 0;
	}

	//lock_bus
	if(rq->type == C_Lock_bus ){

		if(bus_owner == -1 ) {		 // first lock
			bus_owner=rq->args[1];	
			mcore_rq.type=Rq_Lock_bus;
			mcore_send_rq(&mcore_rq);
			return 0;
		} else {			  // others lock
			assert(bus_owner != rq->args[1]); // lock twice
			
			tick * tk;
			tk = mcore_catch_request();
			tick_queue->push(tk);
			bus_finish_for_lock();
			return 0;
		}
	}else if( rq->type==C_Unlock_bus){
		assert(bus_owner==rq->args[1]);
		mcore_rq.type=Rq_Unlock_bus;
		mcore_send_rq(&mcore_rq);
		return 0;
	}
*/
	if (rq->type != WR_hit && rq->type != WR_miss && rq->type != RD_miss) {
		fprintf(stderr,"coherence: recv unknown request\n");
		assert(rq->type == WR_hit);
	}

	perform_coherence((coherence_type)rq->type, rq->args[0], rq->args[1], &cache_array);
	coherence_num++;

	if (rq->type == WR_miss) {
		rq->type = Rq_Write;
	} else if (rq->type == RD_miss) {
		rq->type = Rq_Read;
	} else {//write_hit
//		bus_finish();
		return 0;
	}
	mcore_SetAdvanceTime(latency); 
	mcore_send_rq(rq);
	return 0;
}


int Coherence_module::module_rq_done(Request *rq)
{   
	/*if (mcore_rq.type==Rq_Unlock_bus) {
		assert(rq->type == Rq_Unlock_bus);
	}*/
	//if (rq->type == Rq_Unlock_bus) { // 2010.10.18
/*	if (rq->type == C_Unlock_bus) {
		bus_owner = -1;
		while (!tick_queue->empty()) {
			tick *tk;
			tk = tick_queue->front();
			tick_queue->pop();
			mcore_set_tick_time(tk,mcore_GetCurTime());
			mcore_release_request(tk);
		}
	}

	bus_finish();
*/
	return 0;
} 
/*
void Coherence_module::bus_finish()
{
	while (!tick_conflict_queue->empty()) {
		tick *tk;
		tk = tick_conflict_queue->front();
		tick_conflict_queue->pop();
		mcore_set_tick_time(tk,mcore_GetCurTime());
		mcore_release_request(tk);
	}
	mcore_return_rq(0);
	flag_bus_finish = true;
}
void Coherence_module::bus_finish_for_lock()
{
	while (!tick_conflict_queue->empty()) {
		tick *tk;
		tk = tick_conflict_queue->front();
		tick_conflict_queue->pop();
		mcore_set_tick_time(tk,mcore_GetCurTime());
		mcore_release_request(tk);
	}
	flag_bus_finish = true;
}
void Coherence_module::print_lock_queue()
{
	std::queue<tick *> tmp;
	tick *tk;

	fprintf(stderr,"print tick_queue:\n");

	while (tick_queue->empty() == false) {
		tk = tick_queue->front();
		tmp.push(tk);
		tick_queue->pop();

		Request *rq = (Request *)tk->args[0];
		fprintf(stderr,"%p at %d, rq->args[1] = %d\n",tk,tk->time,rq->args[1]);
	}
	fprintf(stderr,"end\n");
	//recover
	while (tmp.empty() == false) {
		tick_queue->push( tmp.front() );
		tmp.pop();
	}

	fprintf(stderr,"print tick_conflict_queue:\n");

	while (tick_conflict_queue->empty() == false) {
		tk = tick_conflict_queue->front();
		tmp.push(tk);
		tick_conflict_queue->pop();

		Request *rq = (Request *)tk->args[0];
		fprintf(stderr,"%p, rq->args[1] = %d\n",tk,tk->time,rq->args[1]);
	}
	fprintf(stderr,"end\n");

	//recover
	while (tmp.empty() == false) {
		tick_conflict_queue->push( tmp.front() );
		tmp.pop();
	}
}
*/
int Coherence_module::module_stats()
{   
	printf("================ Coherence_Bus ============================\n");
//	printf("  bus conflict: %i\n", mcore_GetConflict());
	printf("  Coherence: %i\n", coherence_num);
}   

