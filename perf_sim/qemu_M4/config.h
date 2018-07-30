#include "config-host.h"
#include "config-target.h"
#define CONFIG_TRACE    //wswu
#define CONFIG_TRACE_TIME //wswu
//#define CONFIG_TRACE_SOFT
//#define CONFIG_TRACE_SOFT_IO
#define CONFIG_SHMEM
#define data_trace_flag
#define data_trace_flag_tb // yh
#define CONFIG_GPA 	//wswu
#define CONFIG_TRACEFILE //wswu
//#define CONFIG_TRACE_SCHEDULE
#define CPU_TLB_SIZE (1 << 8)
#define TRACE_FILE_PATH "/home/cws/perf_sim/tracerlog/trace-pid.log"
#define INSN_TRACE_FILE_PATH "/home/cws/perf_sim/tracerlog/insn_trace.log"
#define INSN_TRACE_FILE_PATH_A "/home/cws/perf_sim/tracerlog/insn_trace_A.log"
//#define CONFIG_ARM_TRACE
#define CONFIG_HSAPI
#define MODE0   // straight SSH
#define MODE1   // through signal 
#define TRANSFORMER_KIND
