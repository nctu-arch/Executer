
/home/cws/perf_sim/qemu_M0/arm-softmmu/qemu-system-arm \
-cpu cortex-m3 -device host_create \
-nographic -serial null -monitor null \
-semihosting -kernel ${1}


