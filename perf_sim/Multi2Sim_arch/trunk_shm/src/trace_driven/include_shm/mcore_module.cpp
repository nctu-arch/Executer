#include "mcore_module.h"

mcore_module::mcore_module()
{
	timestamp = 0;
	mcore_out = NULL;
	set_return= return_latency = 0;
	set_module_finish = 0;
	mcore_execution_time = 0;
	mcore_select_out = NULL;
}

//connect
int mcore_module::connect(mcore_module *obj)
{
	mcore_out = obj;
	return 0;
}

// provide
int mcore_module::mcore_send_rq(Request *rq)
{
	mcore_select_out = mcore_out;
}

int mcore_module::mcore_return_rq(int latency)
{
	set_return  = 1;
	set_module_finish = 1;
	return_latency = latency;
	mcore_select_out = NULL;
}

int mcore_module::mcore_SetAdvanceTime(int time)
{
	timestamp += time;
	return time;
}

int mcore_module::mcore_GetCurTime()
{
	return timestamp;
}
//private method
int mcore_module::SetCurTime(int time)
{
	return timestamp = time;
}

