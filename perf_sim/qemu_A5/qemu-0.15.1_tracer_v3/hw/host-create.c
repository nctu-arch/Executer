#include <stdlib.h>
#include "sysbus.h"
#include "../cpu-all.h"
#include "../trace_timestamp.h"
#include "../Qemu_trace.h"
#include "sysemu.h"

typedef struct {
    SysBusDevice busdev;
    uint8_t read_buf[256];
    uint8_t write_buffer[256];
}host_create_state;
static CPUWriteMemoryFunc * const host_create_writefn[]={};
static CPUReadMemoryFunc * const host_create_readfn[]={};

static int host_create_init(SysBusDevice *dev){
    int iomemtype;
    host_create_state *s = FROM_SYSBUS(host_create_state, dev);
    iomemtype = cpu_register_io_memory(host_create_readfn, host_create_writefn, s, DEVICE_NATIVE_ENDIAN);
    if(iomemtype == -1){
        hw_error("host_create_init: can't register I/O memory\n");
    }

    sysbus_init_mmio(dev, 0x100, iomemtype);
    sysbus_create_simple("host_relay",0x70000000 , NULL);
    sysbus_create_simple("host_TS", 0x20000000, NULL);
    sysbus_create_simple("SEMA4_mod", 0x70050000, NULL);
    sysbus_create_simple("host_shm", 0x20050000, NULL);
    printf("host-create init\n");
    my_trace_init();
    return 0;
}
static void host_create_register_devices(void){
    sysbus_register_dev("host_create",sizeof(host_create_state), host_create_init);	
}
device_init(host_create_register_devices)


