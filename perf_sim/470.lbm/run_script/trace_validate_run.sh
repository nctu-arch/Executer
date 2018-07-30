#/bin/bash



echo Benchmark $1.C$2

#trace='trace/lu_cb'
#dir='trace_d3/dcu'
trace='/home/hchung/Trace'
dir='/home/hchung/Report'

MULTI2SIMROOT=/home/hchung/Multi2Sim_arch/trunk_shm/BIN/bin
#X86_MAX_INST_AFTER_FF=1200000000
#X86_MAX_INST_AFTER_FF=300000000
#X86_MAX_INST=1200000000
X86_MAX_INST=120000
#CTX_CONFIG=/nishome/ic/Benchmarks/Multi2Sim/Multi2Sim_Office/m2s-bench-spec2006
CTX_CONFIG=/home/hchung/Benchmark/m2s-benchmarks-splash2
CONFIG_REPORT_ROOT=`pwd | sed -e 's/\/run_script//ig'`
X86_CONFIG=$CONFIG_REPORT_ROOT/config
X86_REPORT=$CONFIG_REPORT_ROOT/result
MEM_CONFIG=$CONFIG_REPORT_ROOT/config
MEM_REPORT=$CONFIG_REPORT_ROOT/result
DRAM_LOG=$CONFIG_REPORT_ROOT/result
NET_CONFIG=$CONFIG_REPORT_ROOT/config
NET_REPORT=$CONFIG_REPORT_ROOT/result
ADDR_ANALYSIS=$CONFIG_REPORT_ROOT/result
TERMINAL_OUTPUT=$CONFIG_REPORT_ROOT/result

#( \
#/home/yichengc/m2s/Multi2Sim_arch_1/Multi2Sim_arch/trunk/bin/m2s \
#--x86-sim detailed \
#--ctx-config /home/yichengc/splash2x/m2s/$1/runspec_C$2_simsmall \
#--x86-config ./m5config_dram/cpu-config_core$2T1 \
#--mem-config ./m5config_dram/mem-config_net_C$2_PD$3Pd$4d20 \
#--mem-report ./result_dram/EXP/$dir/$1_C$2_PD$3_Pd$4_MEM_REPORT \
#--mem-trace-driven_C0 ./result_dram/EXP/$trace/test_traceC0 \
#--mem-trace-driven_C1 ./result_dram/EXP/$trace/test_traceC1 \
#--mem-trace-driven_C2 ./result_dram/EXP/$trace/test_traceC2 \
#--mem-trace-driven_C3 ./result_dram/EXP/$trace/test_traceC3 \
#--inst-trace-driven_C0 ./result_dram/EXP/$trace/test_traceIC0 \
#--inst-trace-driven_C1 ./result_dram/EXP/$trace/test_traceIC1 \
#--inst-trace-driven_C2 ./result_dram/EXP/$trace/test_traceIC2 \
#--inst-trace-driven_C3 ./result_dram/EXP/$trace/test_traceIC3 \
#--net-config ./m5config_dram/net-config_C$2 \
#--net-report ./result_dram/EXP/$dir/$1_C$2_PD$3_Pd$4_NET_REPORT \
#> ./result_dram/EXP/$dir/$1_C$2_PD$3_Pd$4 2>&1 \
#)

#normal execute
( \
$MULTI2SIMROOT/m2s \
--x86-sim detailed \
--x86-config $X86_CONFIG/cpu-config_C$2T1 \
--mem-config $MEM_CONFIG/mem-config_net_C$2 \
--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT_ref \
--net-config $NET_CONFIG/net-config_C$2 \
--net-report $NET_REPORT/$1_C$2_NET_REPORT_ref \
--mem-trace-driven_C0 $trace/test_traceC0 \
--mem-trace-driven_C1 $trace/test_traceC1 \
--mem-trace-driven_C2 $trace/test_traceC2 \
--mem-trace-driven_C3 $trace/test_traceC3 \
> $dir/$1_C$2_ref 2>&1 \
)


#for validate data created by myself
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--ctx-config $CTX_CONFIG/$1/runspec \
#--x86-config $X86_CONFIG/cpu-config_validate_C1 \
#--mem-config $MEM_CONFIG/mem-config_validate_C1 \
#--net-config $NET_CONFIG/net-config_validate_C1 \
#--mem-report $dir/validate_memory_report \
#--mem-trace-driven_C0 $trace/test_traceC0 \
#> $dir/$1_C$2_ref 2>&1 \
#)


#for validate data created by myself
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--ctx-config $CTX_CONFIG/$1/runspec \
#--x86-config /home/hchung/Multi2Sim_arch/trunk_validate/samples/memory/example-1/x86-config_validate_C1 \
#--mem-config /home/hchung/Multi2Sim_arch/trunk_validate/samples/memory/example-1/mem-config_validate_C1 \
#--mem-report $dir/validate_memory_report \
#--mem-debug $dir/validate_memory_dbg-report \
#--mem-trace-driven_C0 $trace/test_traceC0 \
#> $dir/$1_C$2_ref 2>&1 \
#)

echo "\
$MULTI2SIMROOT/m2s \
--x86-sim detailed \
--ctx-config $CTX_CONFIG/$1/runspec \
--x86-config $X86_CONFIG/cpu-config_C$2T1 \
--mem-config $MEM_CONFIG/mem-config_net_C$2 \
--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT_ref \
--net-config $NET_CONFIG/net-config_C$2 \
--net-report $NET_REPORT/$1_C$2_NET_REPORT_ref \
--mem-trace-driven_C0 $trace/test_traceC0 \
--mem-trace-driven_C1 $trace/test_traceC1 \
--mem-trace-driven_C2 $trace/test_traceC2 \
--mem-trace-driven_C3 $trace/test_traceC3 \
> $dir/$1_C$2_ref 2>&1 \
"


