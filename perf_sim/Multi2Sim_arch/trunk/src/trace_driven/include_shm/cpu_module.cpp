#include "Inc.h"
#include "cpu_module.h"

CPU_module::CPU_module(int id)
{
	cpuid = id;
	instr_count = 0;
	TB_count = 0;
	cycle_count = 0;
}

void CPU_module::read(unsigned int addr,int size)
{
	mcore_rq.type    = Rq_Read;
	mcore_rq.args[0] = addr;
	mcore_rq.args[1] = cpuid;
	mcore_rq.args[2] = size;
	mcore_send_rq(&mcore_rq);
}

void CPU_module::write(unsigned int addr,int size)
{
	mcore_rq.type    = Rq_Write;
	mcore_rq.args[0] = addr;
	mcore_rq.args[1] = cpuid;
	mcore_rq.args[2] = size;
	mcore_send_rq(&mcore_rq);
}
void CPU_module::translated_block(int instr)
{
	instr_count = instr_count + instr;
	//cycle_count += sim_tick;
	TB_count ++;
	mcore_return_rq(0);
}
void CPU_module::sim_cycle(unsigned int sim_tick)
{
        cycle_count += sim_tick;
}
int CPU_module::module_recv_rq(Request *rq)
{

}

int CPU_module::module_rq_done(Request *rq)
{

}
int CPU_module::module_stats()
{
	printf("================= CPU %i ==========================\n",cpuid);
	printf("TB count         : %i\n",TB_count);
	printf("instruction count: %i\n",instr_count);
	printf("cycle count 	 : %i\n",cycle_count);
}

