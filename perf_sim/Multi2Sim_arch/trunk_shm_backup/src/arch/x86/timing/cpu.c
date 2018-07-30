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

#include <assert.h>

#include <config/config.h>
#include <arch/common/arch.h>
#include <arch/x86/emu/context.h>
#include <arch/x86/emu/emu.h>
#include <lib/esim/esim.h>
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/file.h>
#include <lib/util/linked-list.h>
#include <lib/util/timer.h>
#include <mem-system/memory.h>
#include <mem-system/mmu.h>
#include <mem-system/prefetch-history.h>
//IC_S_2013_01_29
#include <mem-system/config.h>
#include <mem-system/mem-system.h>
#include <mem-system/module.h>
#include <mem-system/mmu.h>
#include <network/network.h>
#include <lib/util/list.h>
//IC_E

#include "bpred.h"
#include "cpu.h"
#include "event-queue.h"
#include "fetch-queue.h"
#include "fu.h"
#include "inst-queue.h"
#include "load-store-queue.h"
#include "mem-config.h"
#include "reg-file.h"
#include "rob.h"
#include "trace-cache.h"
#include "uop-queue.h"


/*
 * Global variables
 */


/* Help message */

char *x86_config_help =
	"The x86 CPU configuration file is a plain text INI file, defining\n"
	"the parameters of the CPU model used for a detailed (architectural) simulation.\n"
	"This configuration file is passed to Multi2Sim with option '--x86-config <file>,\n"
	"which must be accompanied by option '--x86-sim detailed'.\n"
	"\n"
	"The following is a list of the sections allowed in the CPU configuration file,\n"
	"along with the list of variables for each section.\n"
	"\n"
	"Section '[ General ]':\n"
	"\n"
	"  Cores = <num_cores> (Default = 1)\n"
	"      Number of cores.\n"
	"  Threads = <num_threads> (Default = 1)\n"
	"      Number of hardware threads per core. The total number of computing nodes\n"
	"      in the CPU model is equals to Cores * Threads.\n"
	"  FastForward = <num_inst> (Default = 0)\n"
	"      Number of x86 instructions to run with a fast functional simulation before\n"
	"      the architectural simulation starts.\n"
	//IC_S_2013_01_29
	"  WarmUp = <num_inst> (Default = 0)\n"
	"      Number of x86 instructions to warm up before\n"
	"      the architectural simulation starts.\n"
	//IC_E
	//IC_S_2013_10_12
	"  FinishOnOneThread = (1 or 0) (Default = 0)\n"
	"      If set, the simulator will stop when context thread decreases to one\n"
	"      Use this with fastforward to parallel\n"
	//IC_E
	"  ContextSwitch = {t|f} (Default = t)\n"
	"      Allow context switches in computing nodes. If this option is set to false,\n"
	"      the maximum number of contexts that can be run is limited by the number of\n"
	"      computing nodes; if a contexts spawns a child that cannot be allocated to\n"
	"      a free hardware thread, the simulation will stop with an error message.\n"
	"  ContextQuantum = <cycles> (Default = 100k)\n"
	"      If ContextSwitch is true, maximum number of cycles that a context can occupy\n"
	"      a CPU hardware thread before it is replaced by other pending context.\n"
	"  ThreadQuantum = <cycles> (Default = 1k)\n"
	"      For multithreaded processors (Threads > 1) configured as coarse-grain multi-\n"
	"      threading (FetchKind = SwitchOnEvent), number of cycles in which instructions\n"
	"      are fetched from the same thread before switching.\n"
	"  ThreadSwitchPenalty = <cycles> (Default = 0)\n"
	"      For coarse-grain multithreaded processors (FetchKind = SwitchOnEvent), number\n"
	"      of cycles that the fetch stage stalls after a thread switch.\n"
	"  RecoverKind = {Writeback|Commit} (Default = Writeback)\n"
	"      On branch misprediction, stage in the execution of the mispredicted branch\n"
	"      when processor recovery is triggered.\n"
	"  RecoverPenalty = <cycles> (Default = 0)\n"
	"      Number of cycles that the fetch stage gets stalled after a branch\n"
	"      misprediction.\n"
	"  PageSize = <size> (Default = 4kB)\n"
	"      Memory page size in bytes.\n"
	"  DataCachePerfect = {t|f} (Default = False)\n"
	"  ProcessPrefetchHints = {t|f} (Default = True)\n"
	"      If specified as false, the cpu will ignore any prefetch hints/instructions.\n"
	"  PrefetchHistorySize = <size> (Default = 10)\n"
	"      Number of past prefetches to keep track of, so as to avoid redundant prefetches\n"
	"      from being issued from the cpu to the cache module.\n"
	"  InstructionCachePerfect = {t|f} (Default = False)\n"
	"      Set these options to true to simulate a perfect data/instruction caches,\n"
	"      respectively, where every access results in a hit. If set to false, the\n"
	"      parameters of the caches are given in the memory configuration file\n"
	"\n"
	"Section '[ Pipeline ]':\n"
	"\n"
	"  FetchKind = {Shared|TimeSlice|SwitchOnEvent} (Default = TimeSlice)\n"
	"      Policy for fetching instruction from different threads. A shared fetch stage\n"
	"      fetches instructions from different threads in the same cycle; a time-slice\n"
	"      fetch switches between threads in a round-robin fashion; option SwitchOnEvent\n"
	"      switches thread fetch on long-latency operations or thread quantum expiration.\n"
	"  DecodeWidth = <num_inst> (Default = 4)\n"
	"      Number of x86 instructions decoded per cycle.\n"
	"  DispatchKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
	"      Policy for dispatching instructions from different threads. If shared,\n"
	"      instructions from different threads are dispatched in the same cycle. Otherwise,\n"
	"      instruction dispatching is done in a round-robin fashion at a cycle granularity.\n"
	"  DispatchWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions dispatched per cycle.\n"
	"  IssueKind = {Shared|TimeSlice} (Default = TimeSlice)\n"
	"      Policy for issuing instructions from different threads. If shared, instructions\n"
	"      from different threads are issued in the same cycle; otherwise, instruction issue\n"
	"      is done round-robin at a cycle granularity.\n"
	"  IssueWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions issued per cycle.\n"
	"  CommitKind = {Shared|TimeSlice} (Default = Shared)\n"
	"      Policy for committing instructions from different threads. If shared,\n"
	"      instructions from different threads are committed in the same cycle; otherwise,\n"
	"      they commit in a round-robin fashion.\n"
	"  CommitWidth = <num_inst> (Default = 4)\n"
	"      Number of microinstructions committed per cycle.\n"
	"  OccupancyStats = {t|f} (Default = False)\n"
	"      Calculate structures occupancy statistics. Since this computation requires\n"
	"      additional overhead, the option needs to be enabled explicitly. These statistics\n"
	"      will be attached to the CPU report.\n"
	"\n"
	"Section '[ Queues ]':\n"
	"\n"
	"  FetchQueueSize = <bytes> (Default = 64)\n"
	"      Size of the fetch queue given in bytes.\n"
	"  UopQueueSize = <num_uops> (Default = 32)\n"
	"      Size of the uop queue size, given in number of uops.\n"
	"  RobKind = {Private|Shared} (Default = Private)\n"
	"      Reorder buffer sharing among hardware threads.\n"
	"  RobSize = <num_uops> (Default = 64)\n"
	"      Reorder buffer size in number of microinstructions (if private, per-thread size).\n"
	"  IqKind = {Private|Shared} (Default = Private)\n"
	"      Instruction queue sharing among threads.\n"
	"  IqSize = <num_uops> (Default = 40)\n"
	"      Instruction queue size in number of uops (if private, per-thread IQ size).\n"
	"  LsqKind = {Private|Shared} (Default = Private)\n"
	"      Load-store queue sharing among threads.\n"
	"  LsqSize = <num_uops> (Default = 20)\n"
	"      Load-store queue size in number of uops (if private, per-thread LSQ size).\n"
	"  RfKind = {Private|Shared} (Default = Private)\n"
	"      Register file sharing among threads.\n"
	"  RfIntSize = <entries> (Default = 80)\n"
	"      Number of integer physical register (if private, per-thread).\n"
	"  RfFpSize = <entries> (Default = 40)\n"
	"      Number of floating-point physical registers (if private, per-thread).\n"
	"  RfXmmSize = <entries> (Default = 40)\n"
	"      Number of XMM physical registers (if private, per-thread).\n"
	"\n"
	"Section '[ TraceCache ]':\n"
	"\n"
	"  Present = {t|f} (Default = False)\n"
	"      If true, a trace cache is included in the model. If false, the rest of the\n"
	"      options in this section are ignored.\n"
	"  Sets = <num_sets> (Default = 64)\n"
	"      Number of sets in the trace cache.\n"
	"  Assoc = <num_ways> (Default = 4)\n"
	"      Associativity of the trace cache. The product Sets * Assoc is the total\n"
	"      number of traces that can be stored in the trace cache.\n"
	"  TraceSize = <num_uops> (Default = 16)\n"
	"      Maximum size of a trace of uops.\n"
	"  BranchMax = <num_branches> (Default = 3)\n"
	"      Maximum number of branches contained in a trace.\n"
	"  QueueSize = <num_uops> (Default = 32)\n"
	"      Size of the trace queue size in uops.\n"
	"\n"
	"Section '[ FunctionalUnits ]':\n"
	"\n"
	"  The possible variables in this section follow the format\n"
	"      <func_unit>.<field> = <value>\n"
	"  where <func_unit> refers to a functional unit type, and <field> refers to a\n"
	"  property of it. Possible values for <func_unit> are:\n"
	"      IntAdd      Integer adder\n"
	"      IntMult     Integer multiplier\n"
	"      IntDiv      Integer divider\n"
	"      EffAddr     Operator for effective address computations\n"
	"      Logic       Operator for logic operations\n"
	"      FpSimple    Simple floating-point operations\n"
	"      FpAdd       Floating-point adder\n"
	"      FpMult      Floating-point multiplier\n"
	"      FpDiv       Floating-point divider\n"
	"      FpComplex   Operator for complex floating-point computations\n"
	"      XMMInt      XMM integer unit\n"
	"      XMMFloat    XMM floating-point unit\n"
	"      XMMLogic    XMM logic unit\n"
	"  Possible values for <field> are:\n"
	"      Count       Number of functional units of a given kind.\n"
	"      OpLat       Latency of the operator.\n"
	"      IssueLat    Latency since an instruction was issued until the functional\n"
	"                  unit is available for the next use. For pipelined operators,\n"
	"                  IssueLat is smaller than OpLat.\n"
	"\n"
	"Section '[ BranchPredictor ]':\n"
	"\n"
	"  Kind = {Perfect|Taken|NotTaken|Bimodal|TwoLevel|Combined} (Default = TwoLevel)\n"
	"      Branch predictor type.\n"
	"  BTB.Sets = <num_sets> (Default = 256)\n"
	"      Number of sets in the BTB.\n"
	"  BTB.Assoc = <num_ways) (Default = 4)\n"
	"      BTB associativity.\n"
	"  Bimod.Size = <entries> (Default = 1024)\n"
	"      Number of entries of the bimodal branch predictor.\n"
	"  Choice.Size = <entries> (Default = 1024)\n"
	"      Number of entries for the choice predictor.\n"
	"  RAS.Size = <entries> (Default = 32)\n"
	"      Number of entries of the return address stack (RAS).\n"
	"  TwoLevel.L1Size = <entries> (Default = 1)\n"
	"      For the two-level adaptive predictor, level 1 size.\n"
	"  TwoLevel.L2Size = <entries> (Default = 1024)\n"
	"      For the two-level adaptive predictor, level 2 size.\n"
	"  TwoLevel.HistorySize = <size> (Default = 8)\n"
	"      For the two-level adaptive predictor, level 2 history size.\n"
	"\n";


/* Main Processor Global Variable */
struct x86_cpu_t *x86_cpu;

/* Trace */
int x86_trace_category;


/* Configuration file and parameters */

char *x86_config_file_name = "";
char *x86_cpu_report_file_name = "";

//#ifdef dump_trace   CIC 2013/12/17
char *mem_trace_file_name = "";
char *inst_trace_file_name = "";
FILE *trace_f;
FILE *trace_instf;
//#endif

char *x86_cpu_mpki_file_name ;//cjc_20130124
FILE *MPKI;
int x86_cpu_forward_to_parallel = 0;
int x86_cpu_forward_to_parallel_finish = 0;

int x86_cpu_num_cores = 1;
int x86_cpu_num_threads = 1;

long long x86_cpu_fast_forward_count;

//IC_S_2013_01_29
long long x86_cpu_warmup_count; 
//IC_E
long long x86_cpu_warmup_cycle_count;//cjc_20130207

long long x86_mpki_interval ;	//cjc_20130208
long long x86_mem_dump_interval; //cjc_20130208
long long x86_training_instruction;//cjc_20130208
long long x86_max_instr_after_fastforward;//cjc_20130507

int finish_on_one_thread; //IC_S_2013_10_12

int x86_cpu_context_quantum;
int x86_cpu_context_switch;

int x86_cpu_thread_quantum;
int x86_cpu_thread_switch_penalty;

char *x86_cpu_recover_kind_map[] = { "Writeback", "Commit" };
enum x86_cpu_recover_kind_t x86_cpu_recover_kind;
int x86_cpu_recover_penalty;

char *x86_cpu_fetch_kind_map[] = { "Shared", "TimeSlice", "SwitchOnEvent" };
enum x86_cpu_fetch_kind_t x86_cpu_fetch_kind;

int x86_cpu_decode_width;

char *x86_cpu_dispatch_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_dispatch_kind_t x86_cpu_dispatch_kind;
int x86_cpu_dispatch_width;

char *x86_cpu_issue_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_issue_kind_t x86_cpu_issue_kind;
int x86_cpu_issue_width;

char *x86_cpu_commit_kind_map[] = { "Shared", "TimeSlice" };
enum x86_cpu_commit_kind_t x86_cpu_commit_kind;
int x86_cpu_commit_width;

int x86_cpu_occupancy_stats;


//IC_S_2013_01_29
int warmup_finish = 0;
//IC_E


/*
 * Private Functions
 */

static char *x86_cpu_err_fast_forward =
	"\tThe number of instructions specified in the x86 CPU configuration file\n"
	"\tfor fast-forward (functional) execution has caused all contexts to end\n"
	"\tbefore the timing simulation could start. Please decrease the number\n"
	"\tof fast-forward instructions and retry.\n";


/* Check CPU configuration file */
static void x86_cpu_config_check(void)
{
	struct config_t *config;
	char *section;

	/* Open file */
	config = config_create(x86_config_file_name);
	if (*x86_config_file_name)
		config_load(config);

	
	/* General configuration */

	section = "General";

	x86_cpu_num_cores = config_read_int(config, section, "Cores", x86_cpu_num_cores);
	x86_cpu_num_threads = config_read_int(config, section, "Threads", x86_cpu_num_threads);

	x86_cpu_fast_forward_count = config_read_llint(config, section, "FastForward", 0);
        
	//IC_S_2013_01_29
	x86_cpu_warmup_count = config_read_llint(config, section, "WarmUp", 0);
	//IC_E
	//IC_S_2013_10_12
	finish_on_one_thread = config_read_llint(config, section, "FinishOnOneThread", 0);
	//IC_E
	x86_cpu_warmup_cycle_count = config_read_llint(config, section, "WarmUpCycle", 0);
	if(x86_cpu_warmup_count||x86_cpu_warmup_cycle_count)//cjc_20130131
		esim_warm_up_finish = 0;
	
	x86_mpki_interval = config_read_llint(config, section, "MpkiInterval", 1000000);
	x86_mem_dump_interval = config_read_llint(config, section, "MemDumpInterval", 250000000);
	x86_training_instruction = config_read_llint(config, section, "TrainingInstr", 0);
		
	x86_cpu_context_switch = config_read_bool(config, section, "ContextSwitch", 1);
	x86_cpu_context_quantum = config_read_int(config, section, "ContextQuantum", 100000);

	x86_cpu_thread_quantum = config_read_int(config, section, "ThreadQuantum", 1000);
	x86_cpu_thread_switch_penalty = config_read_int(config, section, "ThreadSwitchPenalty", 0);

	x86_cpu_recover_kind = config_read_enum(config, section, "RecoverKind", x86_cpu_recover_kind_writeback, x86_cpu_recover_kind_map, 2);
	x86_cpu_recover_penalty = config_read_int(config, section, "RecoverPenalty", 0);

	mmu_page_size = config_read_int(config, section, "PageSize", mmu_page_size);

	x86_emu_process_prefetch_hints = config_read_bool(config, section, "ProcessPrefetchHints", 1);
	prefetch_history_size = config_read_int(config, section, "PrefetchHistorySize", 10);

	x86_cpu_forward_to_parallel = config_read_int(config, section, "ForwardToParallel", 0);//cjc_20130130

	/* Section '[ Pipeline ]' */

	section = "Pipeline";

	x86_cpu_fetch_kind = config_read_enum(config, section, "FetchKind", x86_cpu_fetch_kind_timeslice, x86_cpu_fetch_kind_map, 3);

	x86_cpu_decode_width = config_read_int(config, section, "DecodeWidth", 4);

	x86_cpu_dispatch_kind = config_read_enum(config, section, "DispatchKind", x86_cpu_dispatch_kind_timeslice, x86_cpu_dispatch_kind_map, 2);
	x86_cpu_dispatch_width = config_read_int(config, section, "DispatchWidth", 4);

	x86_cpu_issue_kind = config_read_enum(config, section, "IssueKind", x86_cpu_issue_kind_timeslice, x86_cpu_issue_kind_map, 2);
	x86_cpu_issue_width = config_read_int(config, section, "IssueWidth", 4);

	x86_cpu_commit_kind = config_read_enum(config, section, "CommitKind", x86_cpu_commit_kind_shared, x86_cpu_commit_kind_map, 2);
	x86_cpu_commit_width = config_read_int(config, section, "CommitWidth", 4);

	x86_cpu_occupancy_stats = config_read_bool(config, section, "OccupancyStats", 0);


	/* Section '[ Queues ]' */

	section = "Queues";

	x86_fetch_queue_size = config_read_int(config, section, "FetchQueueSize", 64);

	x86_uop_queue_size = config_read_int(config, section, "UopQueueSize", 32);

	x86_rob_kind = config_read_enum(config, section, "RobKind", x86_rob_kind_private, x86_rob_kind_map, 2);
	x86_rob_size = config_read_int(config, section, "RobSize", 64);

	x86_iq_kind = config_read_enum(config, section, "IqKind", x86_iq_kind_private, x86_iq_kind_map, 2);
	x86_iq_size = config_read_int(config, section, "IqSize", 40);

	x86_lsq_kind = config_read_enum(config, section, "LsqKind", x86_lsq_kind_private, x86_lsq_kind_map, 2);
	x86_lsq_size = config_read_int(config, section, "LsqSize", 20);

	x86_reg_file_kind = config_read_enum(config, section, "RfKind", x86_reg_file_kind_private, x86_reg_file_kind_map, 2);
	x86_reg_file_int_size = config_read_int(config, section, "RfIntSize", 80);
	x86_reg_file_fp_size = config_read_int(config, section, "RfFpSize", 40);
	x86_reg_file_xmm_size = config_read_int(config, section, "RfXmmSize", 40);


	/* Functional Units */

	section = "FunctionalUnits";

	x86_fu_res_pool[x86_fu_intadd].count = config_read_int(config, section, "IntAdd.Count", 4);
	x86_fu_res_pool[x86_fu_intadd].oplat = config_read_int(config, section, "IntAdd.OpLat", 2);
	x86_fu_res_pool[x86_fu_intadd].issuelat = config_read_int(config, section, "IntAdd.IssueLat", 1);

	x86_fu_res_pool[x86_fu_intmult].count = config_read_int(config, section, "IntMult.Count", 1);
	x86_fu_res_pool[x86_fu_intmult].oplat = config_read_int(config, section, "IntMult.OpLat", 3);
	x86_fu_res_pool[x86_fu_intmult].issuelat = config_read_int(config, section, "IntMult.IssueLat", 3);

	x86_fu_res_pool[x86_fu_intdiv].count = config_read_int(config, section, "IntDiv.Count", 1);
	x86_fu_res_pool[x86_fu_intdiv].oplat = config_read_int(config, section, "IntDiv.OpLat", 20);
	x86_fu_res_pool[x86_fu_intdiv].issuelat = config_read_int(config, section, "IntDiv.IssueLat", 20);

	x86_fu_res_pool[x86_fu_effaddr].count = config_read_int(config, section, "EffAddr.Count", 4);
	x86_fu_res_pool[x86_fu_effaddr].oplat = config_read_int(config, section, "EffAddr.OpLat", 2);
	x86_fu_res_pool[x86_fu_effaddr].issuelat = config_read_int(config, section, "EffAddr.IssueLat", 1);

	x86_fu_res_pool[x86_fu_logic].count = config_read_int(config, section, "Logic.Count", 4);
	x86_fu_res_pool[x86_fu_logic].oplat = config_read_int(config, section, "Logic.OpLat", 1);
	x86_fu_res_pool[x86_fu_logic].issuelat = config_read_int(config, section, "Logic.IssueLat", 1);

	x86_fu_res_pool[x86_fu_fpsimple].count = config_read_int(config, section, "FpSimple.Count", 2);
	x86_fu_res_pool[x86_fu_fpsimple].oplat = config_read_int(config, section, "FpSimple.OpLat", 2);
	x86_fu_res_pool[x86_fu_fpsimple].issuelat = config_read_int(config, section, "FpSimple.IssueLat", 2);

	x86_fu_res_pool[x86_fu_fpadd].count = config_read_int(config, section, "FpAdd.Count", 2);
	x86_fu_res_pool[x86_fu_fpadd].oplat = config_read_int(config, section, "FpAdd.OpLat", 5);
	x86_fu_res_pool[x86_fu_fpadd].issuelat = config_read_int(config, section, "FpAdd.IssueLat", 5);

	x86_fu_res_pool[x86_fu_fpmult].count = config_read_int(config, section, "FpMult.Count", 1);
	x86_fu_res_pool[x86_fu_fpmult].oplat = config_read_int(config, section, "FpMult.OpLat", 10);
	x86_fu_res_pool[x86_fu_fpmult].issuelat = config_read_int(config, section, "FpMult.IssueLat", 10);

	x86_fu_res_pool[x86_fu_fpdiv].count = config_read_int(config, section, "FpDiv.Count", 1);
	x86_fu_res_pool[x86_fu_fpdiv].oplat = config_read_int(config, section, "FpDiv.OpLat", 20);
	x86_fu_res_pool[x86_fu_fpdiv].issuelat = config_read_int(config, section, "FpDiv.IssueLat", 20);

	x86_fu_res_pool[x86_fu_fpcomplex].count = config_read_int(config, section, "FpComplex.Count", 1);
	x86_fu_res_pool[x86_fu_fpcomplex].oplat = config_read_int(config, section, "FpComplex.OpLat", 40);
	x86_fu_res_pool[x86_fu_fpcomplex].issuelat = config_read_int(config, section, "FpComplex.IssueLat", 40);

	x86_fu_res_pool[x86_fu_xmm_int].count = config_read_int(config, section, "XMMInt.Count", 1);
	x86_fu_res_pool[x86_fu_xmm_int].oplat = config_read_int(config, section, "XMMInt.OpLat", 2);
	x86_fu_res_pool[x86_fu_xmm_int].issuelat = config_read_int(config, section, "XMMInt.IssueLat", 2);

	x86_fu_res_pool[x86_fu_xmm_float].count = config_read_int(config, section, "XMMFloat.Count", 1);
	x86_fu_res_pool[x86_fu_xmm_float].oplat = config_read_int(config, section, "XMMFloat.OpLat", 10);
	x86_fu_res_pool[x86_fu_xmm_float].issuelat = config_read_int(config, section, "XMMFloat.IssueLat", 10);

	x86_fu_res_pool[x86_fu_xmm_logic].count = config_read_int(config, section, "XMMLogic.Count", 1);
	x86_fu_res_pool[x86_fu_xmm_logic].oplat = config_read_int(config, section, "XMMLogic.OpLat", 1);
	x86_fu_res_pool[x86_fu_xmm_logic].issuelat = config_read_int(config, section, "XMMLogic.IssueLat", 1);


	/* Branch Predictor */

	section = "BranchPredictor";

	x86_bpred_kind = config_read_enum(config, section, "Kind", x86_bpred_kind_twolevel, x86_bpred_kind_map, 6);
	x86_bpred_btb_sets = config_read_int(config, section, "BTB.Sets", 256);
	x86_bpred_btb_assoc = config_read_int(config, section, "BTB.Assoc", 4);
	x86_bpred_bimod_size = config_read_int(config, section, "Bimod.Size", 1024);
	x86_bpred_choice_size = config_read_int(config, section, "Choice.Size", 1024);
	x86_bpred_ras_size = config_read_int(config, section, "RAS.Size", 32);
	x86_bpred_twolevel_l1size = config_read_int(config, section, "TwoLevel.L1Size", 1);
	x86_bpred_twolevel_l2size = config_read_int(config, section, "TwoLevel.L2Size", 1024);
	x86_bpred_twolevel_hist_size = config_read_int(config, section, "TwoLevel.HistorySize", 8);

	/* Trace Cache */
	x86_trace_cache_config_check(config);

	/* Close file */
	config_check(config);
	config_free(config);
}


/* Dump the CPU configuration */
static void x86_cpu_config_dump(FILE *f)
{
	/* General configuration */
	fprintf(f, "[ Config.General ]\n");
	fprintf(f, "Cores = %d\n", x86_cpu_num_cores);
	fprintf(f, "Threads = %d\n", x86_cpu_num_threads);
	fprintf(f, "FastForward = %lld\n", x86_cpu_fast_forward_count);
	//IC_S_2013_01_29
	fprintf(f, "WarmUp = %lld\n", x86_cpu_warmup_count);
	//IC_E
	//IC_S_2013_10_12
	fprintf(f, "FinishOnOneThread = %d\n", finish_on_one_thread);
	//IC_E
	fprintf(f, "ContextSwitch = %s\n", x86_cpu_context_switch ? "True" : "False");
	fprintf(f, "ContextQuantum = %d\n", x86_cpu_context_quantum);
	fprintf(f, "ThreadQuantum = %d\n", x86_cpu_thread_quantum);
	fprintf(f, "ThreadSwitchPenalty = %d\n", x86_cpu_thread_switch_penalty);
	fprintf(f, "RecoverKind = %s\n", x86_cpu_recover_kind_map[x86_cpu_recover_kind]);
	fprintf(f, "RecoverPenalty = %d\n", x86_cpu_recover_penalty);
	fprintf(f, "PageSize = %d\n", mmu_page_size);
	fprintf(f, "ProcessPrefetchHints = %d\n", x86_emu_process_prefetch_hints);
	fprintf(f, "PrefetchHistorySize = %d\n", prefetch_history_size);
	fprintf(f, "\n");

	/* Pipeline */
	fprintf(f, "[ Config.Pipeline ]\n");
	fprintf(f, "FetchKind = %s\n", x86_cpu_fetch_kind_map[x86_cpu_fetch_kind]);
	fprintf(f, "DecodeWidth = %d\n", x86_cpu_decode_width);
	fprintf(f, "DispatchKind = %s\n", x86_cpu_dispatch_kind_map[x86_cpu_dispatch_kind]);
	fprintf(f, "DispatchWidth = %d\n", x86_cpu_dispatch_width);
	fprintf(f, "IssueKind = %s\n", x86_cpu_issue_kind_map[x86_cpu_issue_kind]);
	fprintf(f, "IssueWidth = %d\n", x86_cpu_issue_width);
	fprintf(f, "CommitKind = %s\n", x86_cpu_commit_kind_map[x86_cpu_commit_kind]);
	fprintf(f, "CommitWidth = %d\n", x86_cpu_commit_width);
	fprintf(f, "OccupancyStats = %s\n", x86_cpu_occupancy_stats ? "True" : "False");
	fprintf(f, "\n");

	/* Queues */
	fprintf(f, "[ Config.Queues ]\n");
	fprintf(f, "FetchQueueSize = %d\n", x86_fetch_queue_size);
	fprintf(f, "UopQueueSize = %d\n", x86_uop_queue_size);
	fprintf(f, "RobKind = %s\n", x86_rob_kind_map[x86_rob_kind]);
	fprintf(f, "RobSize = %d\n", x86_rob_size);
	fprintf(f, "IqKind = %s\n", x86_iq_kind_map[x86_iq_kind]);
	fprintf(f, "IqSize = %d\n", x86_iq_size);
	fprintf(f, "LsqKind = %s\n", x86_lsq_kind_map[x86_lsq_kind]);
	fprintf(f, "LsqSize = %d\n", x86_lsq_size);
	fprintf(f, "RfKind = %s\n", x86_reg_file_kind_map[x86_reg_file_kind]);
	fprintf(f, "RfIntSize = %d\n", x86_reg_file_int_size);
	fprintf(f, "RfFpSize = %d\n", x86_reg_file_fp_size);
	fprintf(f, "\n");

	/* Trace Cache */
	fprintf(f, "[ Config.TraceCache ]\n");
	fprintf(f, "Present = %s\n", x86_trace_cache_present ? "True" : "False");
	fprintf(f, "Sets = %d\n", x86_trace_cache_num_sets);
	fprintf(f, "Assoc = %d\n", x86_trace_cache_assoc);
	fprintf(f, "TraceSize = %d\n", x86_trace_cache_trace_size);
	fprintf(f, "BranchMax = %d\n", x86_trace_cache_branch_max);
	fprintf(f, "QueueSize = %d\n", x86_trace_cache_queue_size);
	fprintf(f, "\n");

	/* Functional units */
	fprintf(f, "[ Config.FunctionalUnits ]\n");

	fprintf(f, "IntAdd.Count = %d\n", x86_fu_res_pool[x86_fu_intadd].count);
	fprintf(f, "IntAdd.OpLat = %d\n", x86_fu_res_pool[x86_fu_intadd].oplat);
	fprintf(f, "IntAdd.IssueLat = %d\n", x86_fu_res_pool[x86_fu_intadd].issuelat);

	fprintf(f, "IntMult.Count = %d\n", x86_fu_res_pool[x86_fu_intmult].count);
	fprintf(f, "IntMult.OpLat = %d\n", x86_fu_res_pool[x86_fu_intmult].oplat);
	fprintf(f, "IntMult.IssueLat = %d\n", x86_fu_res_pool[x86_fu_intmult].issuelat);

	fprintf(f, "IntDiv.Count = %d\n", x86_fu_res_pool[x86_fu_intdiv].count);
	fprintf(f, "IntDiv.OpLat = %d\n", x86_fu_res_pool[x86_fu_intdiv].oplat);
	fprintf(f, "IntDiv.IssueLat = %d\n", x86_fu_res_pool[x86_fu_intdiv].issuelat);

	fprintf(f, "EffAddr.Count = %d\n", x86_fu_res_pool[x86_fu_effaddr].count);
	fprintf(f, "EffAddr.OpLat = %d\n", x86_fu_res_pool[x86_fu_effaddr].oplat);
	fprintf(f, "EffAddr.IssueLat = %d\n", x86_fu_res_pool[x86_fu_effaddr].issuelat);

	fprintf(f, "Logic.Count = %d\n", x86_fu_res_pool[x86_fu_logic].count);
	fprintf(f, "Logic.OpLat = %d\n", x86_fu_res_pool[x86_fu_logic].oplat);
	fprintf(f, "Logic.IssueLat = %d\n", x86_fu_res_pool[x86_fu_logic].issuelat);

	fprintf(f, "FpSimple.Count = %d\n", x86_fu_res_pool[x86_fu_fpsimple].count);
	fprintf(f, "FpSimple.OpLat = %d\n", x86_fu_res_pool[x86_fu_fpsimple].oplat);
	fprintf(f, "FpSimple.IssueLat = %d\n", x86_fu_res_pool[x86_fu_fpsimple].issuelat);

	fprintf(f, "FpAdd.Count = %d\n", x86_fu_res_pool[x86_fu_fpadd].count);
	fprintf(f, "FpAdd.OpLat = %d\n", x86_fu_res_pool[x86_fu_fpadd].oplat);
	fprintf(f, "FpAdd.IssueLat = %d\n", x86_fu_res_pool[x86_fu_fpadd].issuelat);

	fprintf(f, "FpMult.Count = %d\n", x86_fu_res_pool[x86_fu_fpmult].count);
	fprintf(f, "FpMult.OpLat = %d\n", x86_fu_res_pool[x86_fu_fpmult].oplat);
	fprintf(f, "FpMult.IssueLat = %d\n", x86_fu_res_pool[x86_fu_fpmult].issuelat);

	fprintf(f, "FpDiv.Count = %d\n", x86_fu_res_pool[x86_fu_fpdiv].count);
	fprintf(f, "FpDiv.OpLat = %d\n", x86_fu_res_pool[x86_fu_fpdiv].oplat);
	fprintf(f, "FpDiv.IssueLat = %d\n", x86_fu_res_pool[x86_fu_fpdiv].issuelat);

	fprintf(f, "FpComplex.Count = %d\n", x86_fu_res_pool[x86_fu_fpcomplex].count);
	fprintf(f, "FpComplex.OpLat = %d\n", x86_fu_res_pool[x86_fu_fpcomplex].oplat);
	fprintf(f, "FpComplex.IssueLat = %d\n", x86_fu_res_pool[x86_fu_fpcomplex].issuelat);

	fprintf(f, "XMMInt.Count = %d\n", x86_fu_res_pool[x86_fu_xmm_int].count);
	fprintf(f, "XMMInt.OpLat = %d\n", x86_fu_res_pool[x86_fu_xmm_int].oplat);
	fprintf(f, "XMMInt.IssueLat = %d\n", x86_fu_res_pool[x86_fu_xmm_int].issuelat);

	fprintf(f, "XMMFloat.Count = %d\n", x86_fu_res_pool[x86_fu_xmm_int].count);
	fprintf(f, "XMMFloat.OpLat = %d\n", x86_fu_res_pool[x86_fu_xmm_int].oplat);
	fprintf(f, "XMMFloat.IssueLat = %d\n", x86_fu_res_pool[x86_fu_xmm_int].issuelat);

	fprintf(f, "XMMLogic.Count = %d\n", x86_fu_res_pool[x86_fu_xmm_logic].count);
	fprintf(f, "XMMLogic.OpLat = %d\n", x86_fu_res_pool[x86_fu_xmm_logic].oplat);
	fprintf(f, "XMMLogic.IssueLat = %d\n", x86_fu_res_pool[x86_fu_xmm_logic].issuelat);

	fprintf(f, "\n");

	/* Branch Predictor */
	fprintf(f, "[ Config.BranchPredictor ]\n");
	fprintf(f, "Kind = %s\n", x86_bpred_kind_map[x86_bpred_kind]);
	fprintf(f, "BTB.Sets = %d\n", x86_bpred_btb_sets);
	fprintf(f, "BTB.Assoc = %d\n", x86_bpred_btb_assoc);
	fprintf(f, "Bimod.Size = %d\n", x86_bpred_bimod_size);
	fprintf(f, "Choice.Size = %d\n", x86_bpred_choice_size);
	fprintf(f, "RAS.Size = %d\n", x86_bpred_ras_size);
	fprintf(f, "TwoLevel.L1Size = %d\n", x86_bpred_twolevel_l1size);
	fprintf(f, "TwoLevel.L2Size = %d\n", x86_bpred_twolevel_l2size);
	fprintf(f, "TwoLevel.HistorySize = %d\n", x86_bpred_twolevel_hist_size);
	fprintf(f, "\n");

	/* End of configuration */
	fprintf(f, "\n");

}


static void x86_cpu_dump_uop_report(FILE *f, long long *uop_stats, char *prefix, int peak_ipc)
{
	long long uinst_int_count = 0;
	long long uinst_logic_count = 0;
	long long uinst_fp_count = 0;
	long long uinst_mem_count = 0;
	long long uinst_ctrl_count = 0;
	long long uinst_total = 0;

	char *name;
	enum x86_uinst_flag_t flags;
	int i;

	for (i = 0; i < x86_uinst_opcode_count; i++)
	{
		name = x86_uinst_info[i].name;
		flags = x86_uinst_info[i].flags;

		fprintf(f, "%s.Uop.%s = %lld\n", prefix, name, uop_stats[i]);
		if (flags & X86_UINST_INT)
			uinst_int_count += uop_stats[i];
		if (flags & X86_UINST_LOGIC)
			uinst_logic_count += uop_stats[i];
		if (flags & X86_UINST_FP)
			uinst_fp_count += uop_stats[i];
		if (flags & X86_UINST_MEM)
			uinst_mem_count += uop_stats[i];
		if (flags & X86_UINST_CTRL)
			uinst_ctrl_count += uop_stats[i];
		uinst_total += uop_stats[i];
	}
	fprintf(f, "%s.Integer = %lld\n", prefix, uinst_int_count);
	fprintf(f, "%s.Logic = %lld\n", prefix, uinst_logic_count);
	fprintf(f, "%s.FloatingPoint = %lld\n", prefix, uinst_fp_count);
	fprintf(f, "%s.Memory = %lld\n", prefix, uinst_mem_count);
	fprintf(f, "%s.Ctrl = %lld\n", prefix, uinst_ctrl_count);
	fprintf(f, "%s.WndSwitch = %lld\n", prefix, uop_stats[x86_uinst_call] + uop_stats[x86_uinst_ret]);
	fprintf(f, "%s.Total = %lld\n", prefix, uinst_total);
	fprintf(f, "%s.IPC = %.4g\n", prefix, x86_cpu->cycle ? (double) uinst_total / x86_cpu->cycle : 0.0);
	fprintf(f, "%s.DutyCycle = %.4g\n", prefix, x86_cpu->cycle && peak_ipc ?
		(double) uinst_total / x86_cpu->cycle / peak_ipc : 0.0);
	fprintf(f, "\n");
}


#define DUMP_FU_STAT(NAME, ITEM) { \
	fprintf(f, "fu." #NAME ".Accesses = %lld\n", X86_CORE.fu->accesses[ITEM]); \
	fprintf(f, "fu." #NAME ".Denied = %lld\n", X86_CORE.fu->denied[ITEM]); \
	fprintf(f, "fu." #NAME ".WaitingTime = %.4g\n", X86_CORE.fu->accesses[ITEM] ? \
		(double) X86_CORE.fu->waiting_time[ITEM] / X86_CORE.fu->accesses[ITEM] : 0.0); \
}

#define DUMP_DISPATCH_STAT(NAME) { \
	fprintf(f, "Dispatch.Stall." #NAME " = %lld\n", X86_CORE.dispatch_stall[x86_dispatch_stall_##NAME]); \
}

#define DUMP_CORE_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) x86_##ITEM##_size * x86_cpu_num_threads); \
	if (x86_cpu_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", x86_cpu->cycle ? (double) X86_CORE.ITEM##_occupancy / x86_cpu->cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", X86_CORE.ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", X86_CORE.ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", X86_CORE.ITEM##_writes); \
}

#define DUMP_THREAD_STRUCT_STATS(NAME, ITEM) { \
	fprintf(f, #NAME ".Size = %d\n", (int) x86_##ITEM##_size); \
	if (x86_cpu_occupancy_stats) \
		fprintf(f, #NAME ".Occupancy = %.2f\n", x86_cpu->cycle ? (double) X86_THREAD.ITEM##_occupancy / x86_cpu->cycle : 0.0); \
	fprintf(f, #NAME ".Full = %lld\n", X86_THREAD.ITEM##_full); \
	fprintf(f, #NAME ".Reads = %lld\n", X86_THREAD.ITEM##_reads); \
	fprintf(f, #NAME ".Writes = %lld\n", X86_THREAD.ITEM##_writes); \
}


static void x86_cpu_dump_report(void)
{
	FILE *f;
	int core, thread;

	long long now;

	/* Open file */
	f = file_open_for_write(x86_cpu_report_file_name);
	if (!f)
		return;
	
	/* Get CPU timer value */
	now = m2s_timer_get_value(x86_emu->timer);

	/* Dump CPU configuration */
	fprintf(f, ";\n; CPU Configuration\n;\n\n");
	x86_cpu_config_dump(f);
	
	/* Report for the complete processor */
	fprintf(f, ";\n; Simulation Statistics\n;\n\n");
	fprintf(f, "; Global statistics\n");
	fprintf(f, "[ Global ]\n\n");
	//fprintf(f, "Cycles = %lld\n", x86_cpu->cycle);
	//IC_S_2013_01_30
	fprintf(f, "Cycles = %lld\n", x86_cpu->cycle - x86_cpu->num_warm_up_cycle);
	//IC_e
	fprintf(f, "Time = %.2f\n", (double) now / 1000000);
	//fprintf(f, "CyclesPerSecond = %.0f\n", now ? (double) x86_cpu->cycle / now * 1000000 : 0.0);
	fprintf(f, "CyclesPerSecond = %.0f\n", now ? (double) (x86_cpu->cycle - x86_cpu->num_warm_up_cycle) / now * 1000000 : 0.0);
	fprintf(f, "MemoryUsed = %lu\n", (long) mem_mapped_space);
	fprintf(f, "MemoryUsedMax = %lu\n", (long) mem_max_mapped_space);
	fprintf(f, "\n");

	/* Dispatch stage */
	fprintf(f, "; Dispatch stage\n");
	x86_cpu_dump_uop_report(f, x86_cpu->num_dispatched_uinst_array, "Dispatch", x86_cpu_dispatch_width);

	/* Issue stage */
	fprintf(f, "; Issue stage\n");
	x86_cpu_dump_uop_report(f, x86_cpu->num_issued_uinst_array, "Issue", x86_cpu_issue_width);

	/* Commit stage */
	fprintf(f, "; Commit stage\n");
	x86_cpu_dump_uop_report(f, x86_cpu->num_committed_uinst_array, "Commit", x86_cpu_commit_width);

	/* Committed branches */
	fprintf(f, "; Committed branches\n");
	fprintf(f, ";    Branches - Number of committed control uops\n");
	fprintf(f, ";    Squashed - Number of mispredicted uops squashed from the ROB\n");
	fprintf(f, ";    Mispred - Number of mispredicted branches in the correct path\n");
	fprintf(f, ";    PredAcc - Prediction accuracy\n");
	fprintf(f, "Commit.Branches = %lld\n", x86_cpu->num_branch_uinst);
	fprintf(f, "Commit.Squashed = %lld\n", x86_cpu->num_squashed_uinst);
	fprintf(f, "Commit.Mispred = %lld\n", x86_cpu->num_mispred_branch_uinst);
	fprintf(f, "Commit.PredAcc = %.4g\n", x86_cpu->num_branch_uinst ?
		(double) (x86_cpu->num_branch_uinst - x86_cpu->num_mispred_branch_uinst) / x86_cpu->num_branch_uinst : 0.0);
	fprintf(f, "\n");
	
	/* Report for each core */
	X86_CORE_FOR_EACH
	{
		/* Core */
		fprintf(f, "\n; Statistics for core %d\n", core);
		fprintf(f, "[ c%d ]\n\n", core);
		fprintf(f, "Core_committed = %lld\n", X86_CORE.num_committed_inst);
		/* Functional units */
		fprintf(f, "; Functional unit pool\n");
		fprintf(f, ";    Accesses - Number of uops issued to a f.u.\n");
		fprintf(f, ";    Denied - Number of requests denied due to busy f.u.\n");
		fprintf(f, ";    WaitingTime - Average number of waiting cycles to reserve f.u.\n");
		fprintf(f, "; cycle blocked by mshr = %lld\n",X86_CORE.block_by_mshr_cycle);
		fprintf(f, "; num   blocked by mshr = %lld\n",X86_CORE.block_by_mshr_count);
		DUMP_FU_STAT(IntAdd, x86_fu_intadd);
		DUMP_FU_STAT(IntMult, x86_fu_intmult);
		DUMP_FU_STAT(IntDiv, x86_fu_intdiv);
		DUMP_FU_STAT(Effaddr, x86_fu_effaddr);
		DUMP_FU_STAT(Logic, x86_fu_logic);
		DUMP_FU_STAT(FPSimple, x86_fu_fpsimple);
		DUMP_FU_STAT(FPAdd, x86_fu_fpadd);
		DUMP_FU_STAT(FPMult, x86_fu_fpmult);
		DUMP_FU_STAT(FPDiv, x86_fu_fpdiv);
		DUMP_FU_STAT(FPComplex, x86_fu_fpcomplex);
		fprintf(f, "\n");

		/* Dispatch slots */
		if (x86_cpu_dispatch_kind == x86_cpu_dispatch_kind_timeslice)
		{
			fprintf(f, "; Dispatch slots usage (sum = cycles * dispatch width)\n");
			fprintf(f, ";    used - dispatch slot was used by a non-spec uop\n");
			fprintf(f, ";    spec - used by a mispeculated uop\n");
			fprintf(f, ";    ctx - no context allocated to thread\n");
			fprintf(f, ";    uopq,rob,iq,lsq,rename - no space in structure\n");
			DUMP_DISPATCH_STAT(used);
			DUMP_DISPATCH_STAT(spec);
			DUMP_DISPATCH_STAT(uop_queue);
			DUMP_DISPATCH_STAT(rob);
			DUMP_DISPATCH_STAT(iq);
			DUMP_DISPATCH_STAT(lsq);
			DUMP_DISPATCH_STAT(rename);
			DUMP_DISPATCH_STAT(ctx);
			fprintf(f, "\n");
		}

		/* Dispatch stage */
		fprintf(f, "; Dispatch stage\n");
		x86_cpu_dump_uop_report(f, X86_CORE.num_dispatched_uinst_array, "Dispatch", x86_cpu_dispatch_width);

		/* Issue stage */
		fprintf(f, "; Issue stage\n");
		x86_cpu_dump_uop_report(f, X86_CORE.num_issued_uinst_array, "Issue", x86_cpu_issue_width);

		/* Commit stage */
		fprintf(f, "; Commit stage\n");
		x86_cpu_dump_uop_report(f, X86_CORE.num_committed_uinst_array, "Commit", x86_cpu_commit_width);

		/* Committed branches */
		fprintf(f, "; Committed branches\n");
		fprintf(f, "Commit.Branches = %lld\n", X86_CORE.num_branch_uinst);
		fprintf(f, "Commit.Squashed = %lld\n", X86_CORE.num_squashed_uinst);
		fprintf(f, "Commit.Mispred = %lld\n", X86_CORE.num_mispred_branch_uinst);
		fprintf(f, "Commit.PredAcc = %.4g\n", X86_CORE.num_branch_uinst ?
			(double) (X86_CORE.num_branch_uinst - X86_CORE.num_mispred_branch_uinst) / X86_CORE.num_branch_uinst : 0.0);
		fprintf(f, "\n");

		/* Occupancy stats */
		fprintf(f, "; Structure statistics (reorder buffer, instruction queue,\n");
		fprintf(f, "; load-store queue, and integer/floating-point register file)\n");
		fprintf(f, ";    Size - Available size\n");
		fprintf(f, ";    Occupancy - Average number of occupied entries\n");
		fprintf(f, ";    Full - Number of cycles when the structure was full\n");
		fprintf(f, ";    Reads, Writes - Accesses to the structure\n");
		if (x86_rob_kind == x86_rob_kind_shared)
			DUMP_CORE_STRUCT_STATS(ROB, rob);
		if (x86_iq_kind == x86_iq_kind_shared)
		{
			DUMP_CORE_STRUCT_STATS(IQ, iq);
			fprintf(f, "IQ.WakeupAccesses = %lld\n", X86_CORE.iq_wakeup_accesses);
		}
		if (x86_lsq_kind == x86_lsq_kind_shared)
			DUMP_CORE_STRUCT_STATS(LSQ, lsq);
		if (x86_reg_file_kind == x86_reg_file_kind_shared)
		{
			DUMP_CORE_STRUCT_STATS(RF_Int, reg_file_int);
			DUMP_CORE_STRUCT_STATS(RF_Fp, reg_file_fp);
		}
		fprintf(f, "\n");

		/* Report for each thread */
		X86_THREAD_FOR_EACH
		{
			fprintf(f, "\n; Statistics for core %d - thread %d\n", core, thread);
			fprintf(f, "[ c%dt%d ]\n\n", core, thread);

			/* Dispatch stage */
			fprintf(f, "; Dispatch stage\n");
			x86_cpu_dump_uop_report(f, X86_THREAD.num_dispatched_uinst_array, "Dispatch", x86_cpu_dispatch_width);

			/* Issue stage */
			fprintf(f, "; Issue stage\n");
			x86_cpu_dump_uop_report(f, X86_THREAD.num_issued_uinst_array, "Issue", x86_cpu_issue_width);

			/* Commit stage */
			fprintf(f, "; Commit stage\n");
			x86_cpu_dump_uop_report(f, X86_THREAD.num_committed_uinst_array, "Commit", x86_cpu_commit_width);

			/* Committed branches */
			fprintf(f, "; Committed branches\n");
			fprintf(f, "Commit.Branches = %lld\n", X86_THREAD.num_branch_uinst);
			fprintf(f, "Commit.Squashed = %lld\n", X86_THREAD.num_squashed_uinst);
			fprintf(f, "Commit.Mispred = %lld\n", X86_THREAD.num_mispred_branch_uinst);
			fprintf(f, "Commit.PredAcc = %.4g\n", X86_THREAD.num_branch_uinst ?
				(double) (X86_THREAD.num_branch_uinst - X86_THREAD.num_mispred_branch_uinst) / X86_THREAD.num_branch_uinst : 0.0);
			fprintf(f, "\n");

			/* Occupancy stats */
			fprintf(f, "; Structure statistics (reorder buffer, instruction queue, load-store queue,\n");
			fprintf(f, "; integer/floating-point register file, and renaming table)\n");
			if (x86_rob_kind == x86_rob_kind_private)
				DUMP_THREAD_STRUCT_STATS(ROB, rob);
			if (x86_iq_kind == x86_iq_kind_private)
			{
				DUMP_THREAD_STRUCT_STATS(IQ, iq);
				fprintf(f, "IQ.WakeupAccesses = %lld\n", X86_THREAD.iq_wakeup_accesses);
			}
			if (x86_lsq_kind == x86_lsq_kind_private)
				DUMP_THREAD_STRUCT_STATS(LSQ, lsq);
			if (x86_reg_file_kind == x86_reg_file_kind_private)
			{
				DUMP_THREAD_STRUCT_STATS(RF_Int, reg_file_int);
				DUMP_THREAD_STRUCT_STATS(RF_Fp, reg_file_fp);
			}
			fprintf(f, "RAT.IntReads = %lld\n", X86_THREAD.rat_int_reads);
			fprintf(f, "RAT.IntWrites = %lld\n", X86_THREAD.rat_int_writes);
			fprintf(f, "RAT.FpReads = %lld\n", X86_THREAD.rat_fp_reads);
			fprintf(f, "RAT.FpWrites = %lld\n", X86_THREAD.rat_fp_writes);
			fprintf(f, "BTB.Reads = %lld\n", X86_THREAD.btb_reads);
			fprintf(f, "BTB.Writes = %lld\n", X86_THREAD.btb_writes);
			fprintf(f, "\n");

			/* Trace cache stats */
			if (X86_THREAD.trace_cache)
				x86_trace_cache_dump_report(X86_THREAD.trace_cache, f);
		}
	}

	/* Close */
	fclose(f);
}


static void x86_cpu_thread_init(int core, int thread)
{
}


static void x86_cpu_core_init(int core)
{
	int thread;
	X86_CORE.thread = xcalloc(x86_cpu_num_threads, sizeof(struct x86_thread_t));
	X86_CORE.thread->L1Dcache_Misses = 0;
	X86_CORE.thread->pf_get_from_mem = 0;
	X86_CORE.thread->pf_mshr=0;
	X86_THREAD_FOR_EACH
		x86_cpu_thread_init(core, thread);

	X86_CORE.prefetch_history = prefetch_history_create();
}

static void x86_cpu_core_done(int core)
{
	free(X86_CORE.thread);
	prefetch_history_free(X86_CORE.prefetch_history);
}

/*
 * Public Functions
 */

/* Version of x86 trace producer.
 * See 'src/visual/x86/cpu.c' for x86 trace consumer. */

#define X86_TRACE_VERSION_MAJOR		1
#define X86_TRACE_VERSION_MINOR		671


/* Initialization */
void x86_cpu_init()
{
	int core;

	#ifdef dump_trace  //CIC2013/11/30
	trace_f = file_open_for_write(mem_trace_file_name);
	trace_instf = file_open_for_write(inst_trace_file_name);
	if(trace_f == NULL) printf("open trace file %s fail \n",mem_trace_file_name);
	if(trace_instf == NULL) printf("open trace file %s fail \n",mem_trace_file_name);
	#endif
	/* Trace */
	x86_trace_category = trace_new_category();

	/* Register architecture */
	x86_emu_arch->mem_config_check_func = x86_mem_config_check;
	x86_emu_arch->mem_config_default_func = x86_mem_config_default;
	x86_emu_arch->mem_config_parse_entry_func = x86_mem_config_parse_entry;

	/* Analyze CPU configuration file */
	x86_cpu_config_check();

	/* Initialize */
	x86_cpu = xcalloc(1, sizeof(struct x86_cpu_t));
	x86_cpu->uop_trace_list = linked_list_create();

	/* Initialize cores */
	x86_cpu->core = xcalloc(x86_cpu_num_cores, sizeof(struct x86_core_t));
	//----------------------------------------
	(x86_cpu->core[0]).core_id = 0;//CIC
	(x86_cpu->core[1]).core_id = 1;
	(x86_cpu->core[2]).core_id = 2;
	(x86_cpu->core[3]).core_id = 3;
	//-------------------------------------------
	X86_CORE_FOR_EACH
		x86_cpu_core_init(core);

    #ifndef mem_trace_only   //CIC 2013 11 28
	/* Components of an x86 CPU */
	x86_reg_file_init();
	x86_bpred_init();
	x86_trace_cache_init();
	x86_fetch_queue_init();
	x86_uop_queue_init();
	x86_rob_init();
	x86_iq_init();
	x86_lsq_init();
	x86_event_queue_init();
	x86_fu_init();
    #endif

    	
    	#ifndef shm_trace_mode   //CIC 2013 11 28
	/* Components of an x86 CPU */
	x86_reg_file_init();
	x86_bpred_init();
	x86_trace_cache_init();
	x86_fetch_queue_init();
	x86_uop_queue_init();
	x86_rob_init();
	x86_iq_init();
	x86_lsq_init();
	x86_event_queue_init();
	x86_fu_init();
	#endif
	/* Trace */
	x86_trace_header("x86.init version=\"%d.%d\" num_cores=%d num_threads=%d\n",
		X86_TRACE_VERSION_MAJOR, X86_TRACE_VERSION_MINOR,
		x86_cpu_num_cores, x86_cpu_num_threads);
	//IC_S_2013_01_30
	x86_cpu->num_warm_up_inst = 0;
	x86_cpu->num_warm_up_cycle = 0;
	//IC_E
}


/* Finalization */
void x86_cpu_done()
{
	int core;

	/* Dump CPU report */
	x86_cpu_dump_report();

	/* Uop trace list */
	x86_cpu_uop_trace_list_empty();
	linked_list_free(x86_cpu->uop_trace_list);

	/* Finalize structures */
    #ifndef mem_trace_only   //CIC 2013 11 28
	x86_fetch_queue_done();
	x86_uop_queue_done();
	x86_rob_done();
	x86_iq_done();
	x86_lsq_done();
	x86_event_queue_done();
	x86_bpred_done();
	x86_trace_cache_done();
	x86_reg_file_done();
	x86_fu_done();
	#endif

    #ifndef shm_trace_mode   //CIC 2013 11 28
	x86_fetch_queue_done();
	x86_uop_queue_done();
	x86_rob_done();
	x86_iq_done();
	x86_lsq_done();
	x86_event_queue_done();
	x86_bpred_done();
	x86_trace_cache_done();
	x86_reg_file_done();
	x86_fu_done();
	#endif
	#ifdef dump_trace
	fclose(trace_f);
	#endif
	/* Free processor */
	X86_CORE_FOR_EACH
		x86_cpu_core_done(core);

	free(x86_cpu->core);
	free(x86_cpu);
}


void x86_cpu_dump(FILE *f)
{
	int core;
	int thread;
	
	/* General information */
	fprintf(f, "\n");
	fprintf(f, "sim.last_dump  %lld  # Cycle of last dump\n", x86_cpu->last_dump);
	fprintf(f, "sim.ipc_last_dump  %.4g  # IPC since last dump\n", x86_cpu->cycle - x86_cpu->last_dump > 0 ?
		(double) (x86_cpu->num_committed_uinst - x86_cpu->last_committed) / (x86_cpu->cycle - x86_cpu->last_dump) : 0);
	fprintf(f, "\n");

	/* Cores */
	X86_CORE_FOR_EACH
	{
		fprintf(f, "Core %d:\n", core);
		
		fprintf(f, "eventq:\n");
		x86_uop_linked_list_dump(X86_CORE.event_queue, f);
		fprintf(f, "rob:\n");
		x86_rob_dump(core, f);

		X86_THREAD_FOR_EACH
		{
			fprintf(f, "Thread %d:\n", thread);
			
			fprintf(f, "fetch queue:\n");
			x86_uop_list_dump(X86_THREAD.fetch_queue, f);
			fprintf(f, "uop queue:\n");
			x86_uop_list_dump(X86_THREAD.uop_queue, f);
			fprintf(f, "iq:\n");
			x86_uop_linked_list_dump(X86_THREAD.iq, f);
			fprintf(f, "lq:\n");
			x86_uop_linked_list_dump(X86_THREAD.lq, f);
			fprintf(f, "sq:\n");
			x86_uop_linked_list_dump(X86_THREAD.sq, f);
			x86_reg_file_dump(core, thread, f);
			if (X86_THREAD.ctx)
			{
				fprintf(f, "mapped context: %d\n", X86_THREAD.ctx->pid);
				x86_ctx_dump(X86_THREAD.ctx, f);
			}
			
			fprintf(f, "\n");
		}
	}

	/* Register last dump */
	x86_cpu->last_dump = x86_cpu->cycle;
	x86_cpu->last_committed = x86_cpu->num_committed_uinst;
}


void x86_cpu_dump_summary(FILE *f)
{
	int core;
	double time_in_sec;
	double inst_per_cycle;
	double uinst_per_cycle;
	double branch_acc;
	double cycles_per_sec;

	/* Calculate statistics */
	time_in_sec = (double) m2s_timer_get_value(x86_emu->timer) / 1.0e6;
	//inst_per_cycle = x86_cpu->cycle ? (double) x86_cpu->num_committed_inst / x86_cpu->cycle : 0.0;
	//IC_S_2013_01_30
	inst_per_cycle = (x86_cpu->cycle- x86_cpu->num_warm_up_cycle) ? (double) (x86_cpu->num_committed_inst - x86_cpu->num_warm_up_inst) / (x86_cpu->cycle -x86_cpu->num_warm_up_cycle): 0.0;
	//IC_E
	uinst_per_cycle = x86_cpu->cycle ? (double) x86_cpu->num_committed_uinst / x86_cpu->cycle : 0.0;
	branch_acc = x86_cpu->num_branch_uinst ? (double) (x86_cpu->num_branch_uinst - x86_cpu->num_mispred_branch_uinst) / x86_cpu->num_branch_uinst : 0.0;
	cycles_per_sec = time_in_sec > 0.0 ? (double) x86_cpu->cycle / time_in_sec : 0.0;

	/* Print statistics */
	//fprintf(f, "Cycles = %lld\n", x86_cpu->cycle);
	fprintf(f, "Cycles = %lld\n", x86_cpu->cycle - x86_cpu->num_warm_up_cycle);
	fprintf(f, "CyclesPerSecond = %.0f\n", cycles_per_sec);
	fprintf(f, "FastForwardInstructions = %lld\n", x86_cpu->num_fast_forward_inst);
	//IC_S_2013_01_30
	fprintf(f, "WarmUpInstructions = %lld\n", x86_cpu->num_warm_up_inst);
	fprintf(f, "Cycles at WarmUp finished = %lld\n", x86_cpu->num_warm_up_cycle);
	//IC_E
	//fprintf(f, "CommittedInstructions = %lld\n", x86_cpu->num_committed_inst);
	//IC_S_2013_01_30
	fprintf(f, "CommittedInstructions = %lld\n", x86_cpu->num_committed_inst - x86_cpu->num_warm_up_inst);
	//IC_E
	fprintf(f, "CommittedInstructionsPerCycle = %.4g\n", inst_per_cycle);
	fprintf(f, "CommittedMicroInstructions = %lld\n", x86_cpu->num_committed_uinst);
	fprintf(f, "CommittedMicroInstructionsPerCycle = %.4g\n", uinst_per_cycle);
	fprintf(f, "BranchPredictionAccuracy = %.4g\n", branch_acc);
	X86_CORE_FOR_EACH//cjc_20130206
	{
		fprintf(f, "CommittedInstructions_core%d = %lld\n", core,X86_CORE.num_committed_inst);
		if(x86_cpu->cycle-x86_cpu->num_warm_up_cycle)
			fprintf(f, "CommittedInstructionsPerCorePerCycle_core%d = %.4g\n",core, 
					(double) X86_CORE.num_committed_inst / (x86_cpu->cycle-x86_cpu->num_warm_up_cycle ));
	}
}


#define UPDATE_THREAD_OCCUPANCY_STATS(ITEM) { \
	X86_THREAD.ITEM##_occupancy += X86_THREAD.ITEM##_count; \
	if (X86_THREAD.ITEM##_count == x86_##ITEM##_size) \
	X86_THREAD.ITEM##_full++; \
}


#define UPDATE_CORE_OCCUPANCY_STATS(ITEM) { \
	X86_CORE.ITEM##_occupancy += X86_CORE.ITEM##_count; \
	if (X86_CORE.ITEM##_count == x86_##ITEM##_size * x86_cpu_num_threads) \
	X86_CORE.ITEM##_full++; \
}


void x86_cpu_update_occupancy_stats()
{
	int core, thread;

	X86_CORE_FOR_EACH
	{
		/* Update occupancy stats for shared structures */
		if (x86_rob_kind == x86_rob_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(rob);
		if (x86_iq_kind == x86_iq_kind_shared)
			UPDATE_CORE_OCCUPANCY_STATS(iq);
		if (x86_lsq_kind == x86_lsq_kind_shared)
		{
			UPDATE_CORE_OCCUPANCY_STATS(lsq);
		}
		if (x86_reg_file_kind == x86_reg_file_kind_shared)
		{
			UPDATE_CORE_OCCUPANCY_STATS(reg_file_int);
			UPDATE_CORE_OCCUPANCY_STATS(reg_file_fp);
		}

		/* Occupancy stats for private structures */
		X86_THREAD_FOR_EACH
		{
			if (x86_rob_kind == x86_rob_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(rob);
			if (x86_iq_kind == x86_iq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(iq);
			if (x86_lsq_kind == x86_lsq_kind_private)
				UPDATE_THREAD_OCCUPANCY_STATS(lsq);
			if (x86_reg_file_kind == x86_reg_file_kind_private)
			{
				UPDATE_THREAD_OCCUPANCY_STATS(reg_file_int);
				UPDATE_THREAD_OCCUPANCY_STATS(reg_file_fp);
			}
		}
	}
}


void x86_cpu_uop_trace_list_add(struct x86_uop_t *uop)
{
	assert(x86_tracing());
	assert(!uop->in_uop_trace_list);

	uop->in_uop_trace_list = 1;
	linked_list_add(x86_cpu->uop_trace_list, uop);
}


void x86_cpu_uop_trace_list_empty(void)
{
	struct linked_list_t *uop_trace_list;
	struct x86_uop_t *uop;

	uop_trace_list = x86_cpu->uop_trace_list;
	while (uop_trace_list->count)
	{
		/* Remove from list */
		linked_list_head(uop_trace_list);
		uop = linked_list_get(uop_trace_list);
		linked_list_remove(uop_trace_list);
		assert(uop->in_uop_trace_list);

		/* Trace */
		x86_trace("x86.end_inst id=%lld core=%d\n",
				uop->id_in_core, uop->core);

		/* Free uop */
		uop->in_uop_trace_list = 0;
		x86_uop_free_if_not_queued(uop);
	}
}


void x86_cpu_run_stages()
{
	/* Static scheduler called after any context changed status other than 'sepcmode' */
	if (!x86_cpu_context_switch && x86_emu->context_reschedule)
	{
		x86_cpu_static_schedule();
		x86_emu->context_reschedule = 0;
	}

	/* Dynamic scheduler called after any context changed status other than 'specmode',
	 * or quantum of the oldest context expired, and no context is being evicted. */
	if (x86_cpu_context_switch && !x86_cpu->ctx_dealloc_signals &&
			(x86_emu->context_reschedule || x86_cpu->ctx_alloc_oldest + x86_cpu_context_quantum <= x86_cpu->cycle))
	{
		x86_cpu_dynamic_schedule();
		x86_emu->context_reschedule = 0;
	}

	/* Stages */
	x86_cpu_commit();
	x86_cpu_writeback();
	x86_cpu_issue();
	x86_cpu_dispatch();
	x86_cpu_decode();
	x86_cpu_fetch();

	/* Update stats for structures occupancy */
	if (x86_cpu_occupancy_stats)
		x86_cpu_update_occupancy_stats();
}


/* Run fast-forward simulation */
void x86_cpu_run_fast_forward(void)
{
	/* Fast-forward simulation. Run 'x86_cpu_fast_forward' iterations of the x86
	 * emulation loop until any simulation end reason is detected. */
	while (x86_emu->inst_count < x86_cpu_fast_forward_count && !esim_finish)
		x86_emu_run();

	/* Record number of instructions in fast-forward execution. */
	x86_cpu->num_fast_forward_inst = x86_emu->inst_count;
	/* Output warning if simulation finished during fast-forward execution. */
	if (esim_finish)
		warning("x86 fast-forwarding finished simulation.\n%s",
				x86_cpu_err_fast_forward);
}

void x86_cpu_forward_parallel(void)
{
	int ctx_count = 0;
	/* Fast-forward simulation. Run 'x86_cpu_fast_forward' iterations of the x86
	 * emulation loop until any simulation end reason is detected. */
	//while (ctx_count < x86_cpu_num_cores && !esim_finish)
	while (ctx_count <2  && !esim_finish)
		ctx_count=x86_emu_run();
	/* Record number of instructions in fast-forward execution. */
	x86_cpu->num_fast_forward_inst = x86_emu->inst_count;

	x86_cpu_forward_to_parallel_finish = 1;
	fprintf(stderr,"Forward_to_parallel_finish\n");
	/* Output warning if simulation finished during fast-forward execution. */
	if (esim_finish)
		warning("x86 parallel-forwarding finished simulation.\n%s",
				x86_cpu_err_fast_forward);
}
//IC_S_2013_01_29
void x86_cpu_warmup_finish (void)
{	
	int i;
	//struct net_t *net;
	struct mod_t *mod;
	
	int core,thread;
	//struct cache_t *cache;
	esim_warm_up_finish = 1;
	x86_cpu->num_warm_up_inst = x86_cpu->num_committed_inst;
	x86_cpu->num_warm_up_cycle = x86_cpu->cycle;
    printf("WARM_UP_FINSH : %lld \n",esim_cycle);
	//cjc_20130219 clear pf_bits in cache
	X86_CORE_FOR_EACH
	{
		X86_THREAD_FOR_EACH
		{
			mod_clear_pf_bits(X86_THREAD.data_mod);
		}
	}
	
	for (i = 0; i < list_count(mem_system->mod_list); i++)
	{
		mod = list_get(mem_system->mod_list, i);
		//cache = mod->cache;

		//clear statistics

		mod->accesses 			          = 0;
		mod->hits                                 = 0;

		mod->reads                                = 0;
		mod->effective_reads                      = 0;
		mod->effective_read_hits                  = 0;
		mod->writes                               = 0;
		mod->effective_writes                     = 0;
		mod->effective_write_hits                 = 0;
		mod->nc_writes                            = 0;
		mod->effective_nc_writes                  = 0;
		mod->effective_nc_write_hits              = 0;
		mod->prefetches                           = 0;
		mod->prefetch_aborts                      = 0;
		mod->useless_prefetches                   = 0;
		mod->evictions                            = 0;

		mod->blocking_reads                       = 0;
		mod->non_blocking_reads                   = 0;
		mod->read_hits                            = 0;
		mod->blocking_writes                      = 0;
		mod->non_blocking_writes                  = 0;
		mod->write_hits                           = 0;
		mod->blocking_nc_writes                   = 0;
		mod->non_blocking_nc_writes               = 0;
		mod->nc_write_hits                        = 0;
		if(warmup_finish ==0 )
		{
			mod->pf_already_in_cache                  = 0;
			mod->pf_already_in_MSHR                   = 0;
			mod->pf_indentified                       = 0;
			mod->pf_issued                            = 0;
			mod->pf_hits                              = 0;
			mod->pf_pollution                         = 0;
			mod->pf_hit_by_other_core                 = 0;
			mod->pf_early_cycle                       = 0;
			mod->pf_early_cycle_upper_level           = 0;
			mod->pf_hit_by_upper_level                = 0;
			mod->pf_late_load                         = 0;
			mod->pf_late_store                        = 0;
			mod->pf_late_cycle_load                   = 0;
			mod->pf_late_cycle_store                  = 0;

			mod->cache_dead_block                     = 0;
			mod->cache_dead_block_demand              = 0;
			mod->cache_dead_block_prefetch            = 0;
			mod->cache_evict_block                    = 0;
			
			mod->cache_dead_block_demand_caused_by_pf = 0;
                	mod->pf_issued_get_from_mem               = 0;
			
			mod->pf_pollution_FDP			  = 0;
			mod->pf_late				  = 0;
			
			mod->pf_pollution_HPAC			  = 0;
			mod->HPAC_BWC				  = 0;
			mod->HPAC_BWNO				  = 0;
		}
		mod->pf_ideal_cache_victim                = 0;
		mod->pf_ideal_wait_cycle                  = 0;
		mod->pf_ideal_l1_retry                    = 0;
		mod->pf_ideal_l2_retry                    = 0;
		mod->pf_ideal_access                      = 0;//cjc_20130104
		mod->pf_ideal_skipped_mem_access          = 0;//cjc_20130104
		mod->pf_ideal_hit_by_itself               = 0;//cjc_20130107
		mod->pf_ideal_hit_by_other_core           = 0;//cjc_20130107
		mod->pf_ideal_mem_access                  = 0;//cjc_20130107
		mod->pf_ideal_latency                     = 0;//cjc_20130107
		mod->pf_ideal_MSHR_hit                    = 0;
		
		//mod->mshr_full_block_num                  = 0;
		mod->acc_waiting_oi_num                   = 0;
		mod->acc_waiting_oi_cycle                 = 0;

		mod->acc_load_access_time                 = 0;
		mod->acc_load_find_n_lock                 = 0;
		mod->acc_load_L1_L2_net					  = 0;
		mod->acc_load_L2_access                   = 0;
		mod->acc_load_reply                       = 0;
		mod->acc_load_MM_access                   = 0;
		mod->acc_load_peer_to_peer                = 0;
		mod->acc_load_wait_old_write              = 0;
		mod->acc_load_wait_old_inflight_cycle     = 0;

		mod->acc_store_access_time                = 0;
		mod->acc_store_find_n_lock                = 0;
		mod->acc_store_L1_L2_net                  = 0;
		mod->acc_store_L2_access                  = 0;
		mod->acc_store_exclusive                  = 0;
		mod->acc_store_reply                      = 0;
		mod->acc_store_MM_access                  = 0;
		mod->acc_store_wait_old_access            = 0;
		mod->acc_store_wait_old_inflight_cycle    = 0;

		

		mod->R_hit_on_self_l1                     = 0;
		mod->W_hit_on_self_l1                     = 0;
		mod->R_hit_on_remote_l1                   = 0;
		mod->W_hit_on_remote_l1                   = 0;
		mod->R_hit_on_l2                          = 0;
		mod->W_hit_on_l2                          = 0;
		mod->R_hit_on_memory                      = 0;
		mod->W_hit_on_memory                      = 0;
		mod->R_l1_wo_depend_hit_num               = 0;
		mod->W_l1_wo_depend_hit_num               = 0;
		mod->R_l1_w_depend_hit_num                = 0;
		mod->W_l1_w_depend_hit_num                = 0;
		mod->R_l1_remote_hit_num                  = 0;
		mod->W_l1_remote_hit_num                  = 0;
		mod->R_l2_hit_num                         = 0;
		mod->W_l2_hit_num                         = 0;
		mod->R_mm_hit_num                         = 0;
		mod->W_mm_hit_num                         = 0;
		mod->read_retries                         = 0;
		mod->write_retries                        = 0;
		mod->nc_write_retries                     = 0;

		mod->no_retry_accesses                    = 0;
		mod->no_retry_hits                        = 0;
		mod->no_retry_reads                       = 0;
		mod->no_retry_read_hits                   = 0;
		mod->no_retry_writes                      = 0;
		mod->no_retry_write_hits                  = 0;
		mod->no_retry_nc_writes                   = 0;
		mod->no_retry_nc_write_hits               = 0;
		
		mod->demand_accesses			  = 0;//cjc_20130305
        	mod->demand_hits			  = 0;//cjc_20130305
		mod->demand_reads			  = 0;
		mod->demand_writes			  = 0;
		
		//IC_S_2013_10_25
		//XXX
		mod->Demand_Access                        = 0;
		mod->Demand_Hit                           = 0;
		mod->Demand_Read                          = 0;
		mod->Demand_Write                         = 0;
		//IC_E

		mod->dram_access_latency		  = 0;	
        	mod->dram_access			  = 0;

		mod->dram_access_demand			  = 0;
		mod->dram_access_prefetch		  = 0;
		mod->dram_access_others			  = 0;
		mod->dram_access_latency_demand		  = 0;
		mod->dram_access_latency_prefetch	  = 0;
		mod->dram_access_latency_others 	  = 0;
        	mod->dram_rowbuffer_hit			  = 0;
		mod->dram_demand_queuing_cycle		  = 0;
		mod->dram_prefetch_queuing_cycle	  = 0;
		mod->dram_others_queuing_cycle		  = 0;
		mod->dram_bus_waiting_cycle		  = 0;

		mod->pf_in_cache_get_by_others		  = 0;
		mod->pf_in_cache_get_by_other_pf	  = 0;
		mod->pf_in_cache_get_by_itself		  = 0;
		mod->pf_in_cache_get_by_its_own_pf	  = 0;

		mod->pf_downup_invalidation		  = 0;
		
		mod->pf_late_store_same_pc		  = 0;
		mod->pf_late_load_same_pc		  = 0;
		mod->pf_late_stride_found		  = 0;
		mod->pf_late_stride_used		  = 0;
		
		mod->pf_ignored_cross_page_access	  = 0;
		
		mod->stream_trained_stream		  = 0;
		mod->stream_renew_stream		  = 0;
		mod->stream_cleared_stream		  = 0;
		mod->stream_table_replace		  = 0;
		
		mod->local_degree[0]			  = 0;
		mod->local_degree[1]			  = 0;
		mod->local_degree[2]			  = 0;
		mod->local_degree[3]			  = 0;
		mod->local_degree[4]			  = 0;
	}
        /*
	for (i = 0; i < list_count(mem_system->net_list); i++)
	{
		net = list_get(mem_system->net_list, i);
		//clear statistics
		net->transfers								= 0;
		net->lat_acc                                                            = 0; 
		net->msg_size_acc                                                       = 0; 

		net->msg_size_8B_num                                                    = 0;
		net->msg_size_72B_num                                                   = 0;
		net->msg_size_else_num                                                  = 0;
                
		net->output_buffer_must_wait_num                                        = 0;
		net->src_C0L1D_dst_L2_output_buffer_must_wait_num                       = 0;
		net->src_C1L1D_dst_L2_output_buffer_must_wait_num                       = 0;
		net->src_C2L1D_dst_L2_output_buffer_must_wait_num                       = 0;
		net->src_C3L1D_dst_L2_output_buffer_must_wait_num                       = 0;
		net->src_L2_output_buffer_must_wait_num                                 = 0;

		net->source_output_buffer_busy_num                                      = 0;
		net->src_C0L1D_dst_L2_source_output_buffer_busy_num                     = 0;
		net->src_C1L1D_dst_L2_source_output_buffer_busy_num                     = 0;
		net->src_C2L1D_dst_L2_source_output_buffer_busy_num                     = 0;
		net->src_C3L1D_dst_L2_source_output_buffer_busy_num                     = 0;
		net->src_L2_source_output_buffer_busy_num                               = 0;

		net->link_is_busy_num                                                   = 0;
		net->src_C0L1D_dst_L2_link_is_busy_num                                  = 0;
		net->src_C1L1D_dst_L2_link_is_busy_num                                  = 0;
		net->src_C2L1D_dst_L2_link_is_busy_num                                  = 0;
		net->src_C3L1D_dst_L2_link_is_busy_num                                  = 0;
		net->src_L2_link_is_busy_num                                            = 0;

		net->arbitrator_stall_num                                               = 0;
		net->src_C0L1D_dst_L2_arbitrator_stall_num                              = 0;
		net->src_C1L1D_dst_L2_arbitrator_stall_num                              = 0;
		net->src_C2L1D_dst_L2_arbitrator_stall_num                              = 0;
		net->src_C3L1D_dst_L2_arbitrator_stall_num                              = 0;
		net->src_L2_arbitrator_stall_num                                        = 0;

		net->destination_input_buffer_busy_num                                  = 0;
		net->src_C0L1D_dst_L2_destination_input_buffer_busy_num                 = 0;
		net->src_C1L1D_dst_L2_destination_input_buffer_busy_num                 = 0;
		net->src_C2L1D_dst_L2_destination_input_buffer_busy_num                 = 0;
		net->src_C3L1D_dst_L2_destination_input_buffer_busy_num                 = 0;
		net->src_L2_destination_input_buffer_busy_num                           = 0;

		net->destination_input_buffer_full_num                                  = 0;
		net->src_C0L1D_dst_L2_destination_input_buffer_full_num                 = 0;
		net->src_C1L1D_dst_L2_destination_input_buffer_full_num                 = 0;
		net->src_C2L1D_dst_L2_destination_input_buffer_full_num                 = 0;
		net->src_C3L1D_dst_L2_destination_input_buffer_full_num                 = 0;
		net->src_L2_destination_input_buffer_full_num                           = 0;

		net->input_buffer_must_wait_num                                         = 0;
		net->src_C0L1D_dst_L2_input_buffer_must_wait_num                        = 0;
		net->src_C1L1D_dst_L2_input_buffer_must_wait_num                        = 0;
		net->src_C2L1D_dst_L2_input_buffer_must_wait_num                        = 0;
		net->src_C3L1D_dst_L2_input_buffer_must_wait_num                        = 0;
		net->src_L2_input_buffer_must_wait_num                                  = 0;

		net->source_input_buffer_busy_num                                       = 0;
		net->src_C0L1D_dst_L2_source_input_buffer_busy_num                      = 0;
		net->src_C1L1D_dst_L2_source_input_buffer_busy_num                      = 0;
		net->src_C2L1D_dst_L2_source_input_buffer_busy_num                      = 0;
		net->src_C3L1D_dst_L2_source_input_buffer_busy_num                      = 0;
		net->src_L2_source_input_buffer_busy_num                                = 0;

		net->destination_output_buffer_busy_num                                 = 0;
		net->src_C0L1D_dst_L2_destination_output_buffer_busy_num                = 0;
		net->src_C1L1D_dst_L2_destination_output_buffer_busy_num                = 0;
		net->src_C2L1D_dst_L2_destination_output_buffer_busy_num                = 0;
		net->src_C3L1D_dst_L2_destination_output_buffer_busy_num                = 0;
		net->src_L2_destination_output_buffer_busy_num                          = 0;

		net->destination_output_buffer_full_num                                 = 0;
		net->src_C0L1D_dst_L2_destination_output_buffer_full_num                = 0;
		net->src_C1L1D_dst_L2_destination_output_buffer_full_num                = 0;
		net->src_C2L1D_dst_L2_destination_output_buffer_full_num                = 0;
		net->src_C3L1D_dst_L2_destination_output_buffer_full_num                = 0;
		net->src_L2_destination_output_buffer_full_num                          = 0;
		
		//IC_E
	}
	*/
	warmup_finish ++;
}
//IC_E

/* Run one iteration of the x86 timing simulation loop.
 * The function returns FALSE if there is no more simulation to perform. */
int x86_cpu_run(void)
{
	/* Stop if no context is running */
	if (x86_emu->finished_list_count >= x86_emu->context_list_count)
		return 0;
	/* Fast-forward simulation */
	if (x86_cpu_fast_forward_count && x86_emu->inst_count < x86_cpu_fast_forward_count)
		x86_cpu_run_fast_forward();

	if((x86_cpu_forward_to_parallel&&x86_cpu_forward_to_parallel_finish)||x86_cpu_forward_to_parallel==0)//cjc_20130131
	{
		//IC_S_2013_01_29
		if (x86_cpu_warmup_count && (x86_cpu->num_committed_inst > x86_cpu_warmup_count) && !warmup_finish)
		{
			x86_cpu_warmup_finish();
		}
		//IC_E
		if(x86_cpu_warmup_cycle_count && esim_cycle>x86_cpu_warmup_cycle_count && !warmup_finish)//cjc_20130207
		{
			x86_cpu_warmup_finish();
		}
		
		if((x86_training_instruction&&x86_cpu->num_committed_inst > x86_cpu_warmup_count + x86_training_instruction ) 
			&& ((warmup_finish ==1&&(x86_cpu_warmup_count||x86_cpu_warmup_cycle_count))
			||(x86_cpu_warmup_count==0&&x86_cpu_warmup_cycle_count==0&&warmup_finish==0)))
		{
			x86_cpu_warmup_finish();
		}
	}
	/* Stop if maximum number of CPU instructions exceeded */
	if (x86_emu_max_inst && x86_cpu->num_committed_inst >= x86_emu_max_inst)
		esim_finish = esim_finish_x86_max_inst;
	
	/* Stop if maximum number of CPU instructions exceeded */
	if(x86_max_instr_after_fastforward && x86_cpu->num_committed_inst >=x86_max_instr_after_fastforward)
		esim_finish = esim_finish_x86_max_inst;	

        //IC_S_2013_10_12
        if (finish_on_one_thread && (ctx_number == 1) && !esim_finish && (x86_cpu_forward_to_parallel_finish == 1) && (warmup_finish == 1)) 
	{
            esim_finish = 1;
	    fprintf(stderr,"context thread is only one\n");
	}
        //IC_E
	if(x86_emu_min_inst)
	{	
		int core;
		X86_CORE_FOR_EACH
		{
			if(X86_CORE.num_committed_inst < x86_emu_min_inst)
				break;
			if(core == x86_cpu_num_cores-1)
				esim_finish = esim_finish_x86_max_inst;
		}
	}

	/* Stop if maximum number of cycles exceeded */
	if (x86_emu_max_cycles && x86_cpu->cycle >= x86_emu_max_cycles)
		esim_finish = esim_finish_x86_max_cycles;
	


	if(x86_cpu_forward_to_parallel&&x86_cpu_forward_to_parallel_finish==0)//cjc_20130130
		x86_cpu_forward_parallel();	

	/* Stop if any previous reason met */
	if (esim_finish)
		return 0;

	/* One more cycle of x86 timing simulation */
	x86_cpu->cycle++;

	/* Empty uop trace list. This dumps the last trace line for instructions
	 * that were freed in the previous simulation cycle. */
	x86_cpu_uop_trace_list_empty();

	/* Processor stages */
	x86_cpu_run_stages();

	/* Process host threads generating events */
	x86_emu_process_events();

	/* Return TRUE */
	return 1;
}
void x86_cpu_dump_per_200M(int num)
{
	//int length;
	//char *mem_file_name;

	//FILE *f;
	fprintf(stderr,"\n\n[ x86_%d]\n",num);
	x86_cpu_dump_summary(stderr);
	esim_warm_up_finish++;
	/*length = strlen(mem_report_file_name);
	if(length)
	{

		mem_file_name=xcalloc((length+5),sizeof(char));
		strncpy(mem_file_name,mem_report_file_name,length);
		mem_file_name[length]='_';
		mem_file_name[length+1]=num/100+'0';
		mem_file_name[length+2]=(num/10)%10+'0';
		mem_file_name[length+3]=num%10+'0';
		mem_file_name[length+4]='\0';	
		FILE *f;
		f = file_open_for_write(mem_file_name);
		if(f)
		{
			mem_system_dump_report(f);
			fclose(f);
		}	
		//fprintf(stderr,"\n\n[ %d  %s]\n",length,mem_file_name);
		free(mem_file_name);
	}*/
	
}
