#!/bin/bash

echo Benchmark.$1.C$2

MULTI2SIMROOT=/home/hchung/Multi2Sim_arch/trunk/BIN/bin
#X86_MAX_INST_AFTER_FF=1200000000
#X86_MAX_INST_AFTER_FF=300000000
X86_MAX_INST=1200000000
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

#DEFAULT
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--x86-max-inst-ff $X86_MAX_INST_AFTER_FF \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_core$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_PD$3_Pd$4_MEM_REPORT \
#--dram-log $DRAM_LOG/$1_C$2_PD$3_Pd$4_DRAM_LOG \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_PD$3_Pd$4_NET_REPORT \
#> $TERMINAL_OUTPUT/$1_C$2_PD$3_Pd$4 2>&1 \
#)

##TEST STDOUT
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--x86-max-inst $X86_MAX_INST \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT \
#)
#echo " \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--x86-max-inst $X86_MAX_INST \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT \
#"

##TEST
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--x86-max-inst-ff $X86_MAX_INST_AFTER_FF \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT \
#> $TERMINAL_OUTPUT/$1_C$2 2>&1 \
#)

##TEST
#echo " \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT \
#> $TERMINAL_OUTPUT/$1_C$2 2>&1 \
#"
#
##TEST
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simtest \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT \
#> $TERMINAL_OUTPUT/$1_C$2 2>&1 \
#)

#SMALL
#( \
#$MULTI2SIMROOT/m2s \
#--x86-sim detailed \
#--x86-max-inst $X86_MAX_INST \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simsmall \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT_small \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT_small \
#> $TERMINAL_OUTPUT/$1_C$2_small 2>&1 \
#)

##SMALL
#( \
#$MULTI2SIMROOT/m2s \
#--mem-debug_2 $MEM_REPORT/$1_C$2_MEM_REPORT_DEBUG_2_small \
#--x86-sim detailed \
#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simsmall \
#--x86-config $X86_CONFIG/cpu-config_C$2T1 \
#--mem-config $MEM_CONFIG/mem-config_net_C$2 \
#--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT_small \
#--net-config $NET_CONFIG/net-config_C$2 \
#--net-report $NET_REPORT/$1_C$2_NET_REPORT_small \
#> $TERMINAL_OUTPUT/$1_C$2_small 2>&1 \
#)

#ref
( \
$MULTI2SIMROOT/m2s \
--x86-sim detailed \
--x86-max-inst $X86_MAX_INST \
--ctx-config $CTX_CONFIG/$1/runspec \
--x86-config $X86_CONFIG/cpu-config_C$2T1 \
--x86-report $X86_REPORT/$1_C$2_X86_REPORT_ref \
--mem-config $MEM_CONFIG/mem-config_net_C$2 \
--mem-report $MEM_REPORT/$1_C$2_MEM_REPORT_ref \
--net-config $NET_CONFIG/net-config_C$2 \
--net-report $NET_REPORT/$1_C$2_NET_REPORT_ref \
> $TERMINAL_OUTPUT/$1_C$2_ref 2>&1 \
)

#--ctx-config $CTX_CONFIG/$1/runspec_C$2_simref \
