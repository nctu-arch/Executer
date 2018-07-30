#ifndef TRACE_DRIVEN_H  //hchung 2014/11/26
#define TRACE_DRIVEN_H  



extern char * mem_trace_driven_file_name_C0;
extern char * mem_trace_driven_file_name_C1;
extern char * mem_trace_driven_file_name_C2;
extern char * mem_trace_driven_file_name_C3;

extern char * inst_trace_driven_file_name_C0;
extern char * inst_trace_driven_file_name_C1;
extern char * inst_trace_driven_file_name_C2;
extern char * inst_trace_driven_file_name_C3;

void trace_driven_init();

int trace_driven_run_mem();

int trace_driven_run_inst();

void trace_driven_done();

void shm_mem_trace();
void read_mem(unsigned int addr,int size,int cpuid);
void write_mem(unsigned int addr,int size,int cpuid);
int config_init(int proc_cnt);
void config_print(int proc_cnt);

#endif
