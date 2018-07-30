/*
 * Guest Command Realy - forward to host machine
 *
 * Copyright (c) 2010 pcchang
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <stdlib.h>
#include "sysbus.h"
#include "../cpu-all.h"
#include "../trace_timestamp.h"
#include "../Qemu_trace.h"
#include "sysemu.h"

#define DEBUG_HOST_TS

#ifdef DEBUG_HOST_TS
#define DPRINTF(fmt, ...) \
do { printf("host-trigger-signal: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#endif

/* Write buffer */
#define WRITE_BUF_BASE   0x00
#define WRITE_BUF_SIZE   256

/* Read buffer */
#define READ_BUF_BASE    (WRITE_BUF_BASE + WRITE_BUF_SIZE)
#define READ_BUF_SIZE    256

//ctrl06
//#define TRACE_TO_FILE
typedef struct {
    SysBusDevice busdev;
    uint8_t read_buffer[READ_BUF_SIZE];
    uint8_t write_buffer[WRITE_BUF_SIZE];
} host_TS_state;

static uint32_t host_TS_readb(void *opaque, target_phys_addr_t offset)
{
	printf("trigger interrupt read\n");
}

static void host_TS_writeb(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
	printf("M4 signaling A5 ...\n");
	host_TS_state *s = (host_TS_state *)opaque;
	s->write_buffer[offset] = (uint8_t)value;
	/*if ((uint8_t)value == '\0') {
		printf("quququ\n");
		DPRINTF("%s\n", s->write_buffer);
		if(s->write_buffer[0]=='c'  &&  s->write_buffer[1]=='\0')
		{
			printf("hohoho\n");
		}
	}*/
	//system("/home/yhchiang/arm_qemu/run_M.sh /home/yhchiang/arm_qemu/cortexM_test/test_cm0 \r");
	send_SSH();
}

static CPUWriteMemoryFunc * const  host_TS_writefn[] = {
    host_TS_writeb
};

static CPUReadMemoryFunc * const  host_TS_readfn[] = {
    host_TS_readb
};

static int host_TS_init(SysBusDevice *dev)
{
    int iomemtype;
    host_TS_state *s = FROM_SYSBUS(host_TS_state, dev);

    iomemtype = cpu_register_io_memory(host_TS_readfn, host_TS_writefn, s,DEVICE_NATIVE_ENDIAN);
    if (iomemtype == -1) {
        hw_error("host_TS_init: Can't register I/O memory\n");
    }

    sysbus_init_mmio(dev, 0x1000, iomemtype);
    printf("host-TS init\n");
    return 0;
}

static void host_TS_register_devices(void)
{
    sysbus_register_dev("host_TS", sizeof(host_TS_state), host_TS_init);
}

device_init(host_TS_register_devices)
