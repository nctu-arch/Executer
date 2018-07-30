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

#define DEBUG_HOST_PERF

#ifdef DEBUG_HOST_PERF
#define DPRINTF(fmt, ...) \
do { printf("host-perf: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#endif

/* Write buffer */
#define WRITE_BUF_BASE   0x00
#define WRITE_BUF_SIZE   256

/* Read buffer */
#define READ_BUF_BASE    (WRITE_BUF_BASE + WRITE_BUF_SIZE)
#define READ_BUF_SIZE    256

typedef struct {
    SysBusDevice busdev;
    uint8_t read_buffer[READ_BUF_SIZE];
    uint8_t write_buffer[WRITE_BUF_SIZE];
} host_perf_state;

static uint32_t host_perf_readb(void *opaque, target_phys_addr_t offset)
{
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < READ_BUF_BASE || offset >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_perf_readb: Bad offset 0x%x\n", (int)offset);
    	return 0;
    }

    return ldub_p(&s->read_buffer[offset]);
}

static uint32_t host_perf_readw(void *opaque, target_phys_addr_t offset)
{
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < READ_BUF_BASE || offset + 1 >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_perf_readw: Bad offset 0x%x\n", (int)offset);
    	return 0;
    }

    return lduw_be_p(&s->read_buffer[offset]);
}

static uint32_t host_perf_readl(void *opaque, target_phys_addr_t offset)
{
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < READ_BUF_BASE || offset + 3 >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_perf_readl: Bad offset 0x%x\n", (int)offset);
    	return 0;
    }

    return ldl_be_p(&s->read_buffer[offset]);
}

static void host_perf_writeb(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < WRITE_BUF_BASE || offset >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_perf_writeb: Bad offset 0x%x\n", (int)offset);
    	return;
    }

    s->write_buffer[offset] = (uint8_t)value;
    if ((uint8_t)value == '\0') {
        DPRINTF("%s\n", s->write_buffer);
        system((const char*)s->write_buffer);
    }
}

static void host_perf_writew(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    int index;
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < WRITE_BUF_BASE || offset + 1 >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_perf_writeb: Bad offset 0x%x\n", (int)offset);
    	return;
    }

    stw_be_p(&s->write_buffer[offset], value);
    for(index = 0; index <= 1; index++) {
    	if(s->write_buffer[offset+index] == '\0') {
            DPRINTF("%s\n", s->write_buffer);
            system((const char*)s->write_buffer);
            return;	
        }
    }
}

static void host_perf_writel(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    int index;
    host_perf_state *s = (host_perf_state *)opaque;

    if (offset < WRITE_BUF_BASE || offset + 3 >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_perf_writeb: Bad offset 0x%x\n", (int)offset);
    	return;
    }

    stl_be_p(&s->write_buffer[offset], value);
    for(index = 0; index <= 3; index++) {
    	if(s->write_buffer[offset+index] == '\0') {
            DPRINTF("%s\n", s->write_buffer);
            system((const char*)s->write_buffer);
            return;	
        }
    }
}

static CPUWriteMemoryFunc * const  host_perf_writefn[] = {
    host_perf_writeb,
    host_perf_writew,
    host_perf_writel
};

static CPUReadMemoryFunc * const  host_perf_readfn[] = {
    host_perf_readb,
    host_perf_readw,
    host_perf_readl
};

static int host_perf_init(SysBusDevice *dev)
{
    int iomemtype;
    host_perf_state *s = FROM_SYSBUS(host_perf_state, dev);

    iomemtype = cpu_register_io_memory(host_perf_readfn, host_perf_writefn, s,DEVICE_NATIVE_ENDIAN);
    if (iomemtype == -1) {
        hw_error("host_perf_init: Can't register I/O memory\n");
    }

    sysbus_init_mmio(dev, 0x1000, iomemtype);
    return 0;
}

static void host_perf_register_devices(void)
{
    sysbus_register_dev("host_perf", sizeof(host_perf_state), host_perf_init);
}

device_init(host_perf_register_devices)
