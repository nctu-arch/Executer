
#ifndef MCORE_MODULEH
#define MCORE_MODULEH


#include "Inc.h"
#include <pthread.h>

typedef struct {
	int type;
	unsigned int args[3];// 0 => addr
				// 1 => id
				// 2 => size
} Request;

//extern "C" void *shmalloc(unsigned int);
//void *operator new (size_t size, const char *);

class mcore_module {
//friend class LockBus;
public:
	Request mcore_rq;
	mcore_module *mcore_select_out;
	mcore_module();

	// connect
	int connect(mcore_module *);
	// provide
	int mcore_send_rq(Request *);
	int mcore_return_rq(int latency);
//	int mcore_suspend_rq();
	int mcore_SetAdvanceTime(int);//update timestamp
	int mcore_GetCurTime();
	// virtual
	virtual int module_recv_rq(Request *) = 0;
	virtual int module_rq_done(Request *) = 0;
	virtual int module_stats() = 0;
protected:
//	tick *mcore_catch_request();
//	void mcore_release_request(tick *);
//	void mcore_set_tick_time(tick *, int);
private:
	int timestamp;
	mcore_module * mcore_out;
	int set_return, return_latency;
	int set_module_finish;
	int mcore_execution_time;
	int SetCurTime(int time);
};

#endif
