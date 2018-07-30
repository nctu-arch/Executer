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
#include <string.h>
#include <sys/shm.h>
#include <stdint.h>
#define DEBUG_HOST_TS

#ifdef DEBUG_HOST_TS
#define DPRINTF(fmt, ...) \
do { printf("host-trigger-signal: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#endif
#define SHMSIZE 64	
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
    uint32_t buffer[READ_BUF_SIZE];
    //char write_buffer[WRITE_BUF_SIZE];
} host_shm_state;
uint32_t * shm2_ptr;
int shm_id;
key_t shm_key;
static uint32_t host_shm_readb(void *opaque, target_phys_addr_t offset)
{
	printf("M4 shm_readb\n");
    	host_shm_state *s = (host_shm_state *)opaque;
	printf("out shm %c\n",*shm2_ptr);
	return *shm2_ptr;//s->buffer[offset];
}

static void host_shm_writeb(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
	printf("M4 shm_writeb\n");
	
    	host_shm_state *s = (host_shm_state *)opaque;
	s->buffer[offset] = value;
	*shm2_ptr = value;
	printf("in shm %c\n",value);
	//s->write_buffer[offset] = (uint8_t)value;
	/*if ((uint8_t)value == '\0') {
		printf("quququ\n");
		DPRINTF("%s\n", s->write_buffer);
		if(s->write_buffer[0]=='c'  &&  s->write_buffer[1]=='\0')
		{
			printf("hohoho\n");
		}
	}*/
}

static CPUWriteMemoryFunc * const  host_shm_writefn[] = {
    host_shm_writeb
};

static CPUReadMemoryFunc * const  host_shm_readfn[] = {
    host_shm_readb
};

static int host_shm_init(SysBusDevice *dev)
{
    int iomemtype;
    host_shm_state *s = FROM_SYSBUS(host_shm_state, dev);

    iomemtype = cpu_register_io_memory(host_shm_readfn, host_shm_writefn, s,DEVICE_NATIVE_ENDIAN);
    if (iomemtype == -1) {
        hw_error("host_shm_init: Can't register I/O memory\n");
    }
    shm_key = 339;
    shm_id  = shmget(shm_key,SHMSIZE,IPC_CREAT|0666);
    if((shm2_ptr = (uint32_t*)shmat(shm_id,NULL,0))==-1){
	    printf("oooooo\n");
	    perror("shmat");
	    shmctl(shm_id,IPC_RMID,0);
	    exit(1);
    }

    sysbus_init_mmio(dev, 0x1000, iomemtype);
    printf("host-shm init\n");
    return 0;
}

static void host_shm_register_devices(void)
{
    sysbus_register_dev("host_shm", sizeof(host_shm_state), host_shm_init);
}

device_init(host_shm_register_devices)
