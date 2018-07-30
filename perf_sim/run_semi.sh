
/home/cws/perf_sim/qemu_A5/qemu-0.15.1_tracer_v3/arm-softmmu/qemu-system-arm \
-M versatilepb -device host_create -cpu cortex-a8 \
-nographic -serial null -monitor null \
-semihosting -kernel ${1}&


/home/cws/perf_sim/Multi2Sim_arch/trunk_shm/BIN/bin/m2s --x86-sim detailed --x86-config /home/cws/perf_sim/470.lbm/config/cpu-config_C4T1 --mem-config /home/cws/perf_sim/470.lbm/config/mem-config_net_C4 --mem-report /home/cws/perf_sim/Report/shm_memory_report --net-config /home/cws/perf_sim/470.lbm/config/net-config_C4 --net-report /home/cws/perf_sim/470.lbm/result/test > /home/cws/perf_sim/Report/test_ref
