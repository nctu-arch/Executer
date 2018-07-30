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
#include "../target-arm/helper.h"
#define DEBUG_HOST_RELAY

#ifdef DEBUG_HOST_RELAY
#define DPRINTF(fmt, ...) \
do { printf("host-relay: " fmt , ## __VA_ARGS__); } while (0)
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
extern int      start_tracing;
extern int      *shm_ptr;
extern FILE 	*trace_log_file;
extern uint64_t sim_time;
typedef struct {
    SysBusDevice busdev;
    uint8_t read_buffer[READ_BUF_SIZE];
    uint8_t write_buffer[WRITE_BUF_SIZE];
} host_relay_state;
int *shm_A5_int;
int shm_A5_int_id;
key_t shm_A5_int_key;
int *shm_M4_int;
int shm_M4_int_id;
key_t shm_M4_int_key;
int I2C_read;
int I2C_write;

static uint32_t host_relay_readb(void *opaque, target_phys_addr_t offset)
{
    host_relay_state *s = (host_relay_state *)opaque;

    if (offset < READ_BUF_BASE || offset >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_relay_readb: Bad offset 0x%x\n", (int)offset);
        return 0;
    }

    return ldub_p(&s->read_buffer[offset]);
}

static uint32_t host_relay_readw(void *opaque, target_phys_addr_t offset)
{
    host_relay_state *s = (host_relay_state *)opaque;

    if (offset < READ_BUF_BASE || offset + 1 >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_relay_readw: Bad offset 0x%x\n", (int)offset);
        return 0;
    }

    return lduw_be_p(&s->read_buffer[offset]);
}

static uint32_t host_relay_readl(void *opaque, target_phys_addr_t offset)
{
    host_relay_state *s = (host_relay_state *)opaque;

    if (offset < READ_BUF_BASE || offset + 3 >= READ_BUF_BASE + READ_BUF_SIZE) {
        hw_error("host_relay_readl: Bad offset 0x%x\n", (int)offset);
        return 0;
    }

    return ldl_be_p(&s->read_buffer[offset]);
}

static void host_relay_writeb(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    host_relay_state *s = (host_relay_state *)opaque;
    static clock_t begin,end;
    static double tv_start, tv_end;
    double time_spent;
    struct timeval tv;
   
#ifdef TARGET_X86_64
	//ctrl06
//	offset = offset - 0x6ff00000;
#endif

	//share memory for each cpu
	//int head, tail, i;    
	
	if (offset < WRITE_BUF_BASE || offset >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_relay_writeb: Bad offset 0x%x\n", (int)offset);
        return;
    }
    if(offset==3){
	I2C_read+=value;
	printf("%d times of I2C read\n", value);
	printf("I2C read total : %d\n",I2C_read);
	return;
    }
    else if(offset==4){
	I2C_write+=value;
	printf("%d times of I2C write\n", value);
	printf("I2C write total: %d\n",I2C_write);
	return;
	    
    }

    s->write_buffer[offset] = (uint8_t)value;
    //if ((uint8_t)value == '\0') {//sue7785
        DPRINTF("%s\n", s->write_buffer);
        if(s->write_buffer[0]=='a'  &&  s->write_buffer[1]=='\0')
        {
		begin = clock();
		gettimeofday(&tv, NULL);
		tv_start = (double)tv.tv_sec + (1.0/1000000)*tv.tv_usec;
		printf("start time: %f s,  %f s\n",begin, tv_start);
			CPUState *env;
			for(env=first_cpu;env;env=env->next_cpu)tb_flush(env);
			start_tracing = 1;
            DPRINTF("start trace log....\n");
#ifdef TRACE_TO_FILE
			printf("trace to file init\n");
			TRACE_out_init("my_qemu_trace.trc"); //ctrl06
#endif
        }
	else if(s->write_buffer[0]=='b'  &&  s->write_buffer[1]=='\0')
	{
		start_tracing = 0;
		CPUState *env;
		for(env=first_cpu;env;env=env->next_cpu)tb_flush(env);


		my_trace_end();
		tv_end = (double)tv.tv_sec * 1000000 + tv.tv_usec;
		time_spent = (double)(end-begin)/CLOCKS_PER_SEC;
		//printf("tb total cycle cnt = %d\n",tb_total_cycle_cnt);
		printf("end simulation time: %f s,  %f s\n",end-begin, tv_end);

		for(env=first_cpu;env;env=env->next_cpu){
			DPRINTF("instruction count : %lu\nsim cycle: %lu\n",env->inst_count,sim_time);
			env->inst_count=0;
			sim_time=0;
		}

#ifdef TRACE_TO_FILE
		TRACE_out_done(); //ctrl06
#endif
	}
	else if(s->write_buffer[0]=='c'  &&  s->write_buffer[1]=='\0')
	{
		my_trace_init();
	}
	else if(s->write_buffer[0]=='k'  &&  s->write_buffer[1]=='\0')
	{
#if 1
		begin = clock();
		gettimeofday(&tv, NULL);
		tv_start = (double)tv.tv_sec + (1.0/1000000)*tv.tv_usec;
#endif
	}
	else if(s->write_buffer[0]=='q'  &&  s->write_buffer[1]=='\0')
	{
#if 1
		end = clock();
		gettimeofday(&tv, NULL);
		tv_end = (double)tv.tv_sec * 1000000 + tv.tv_usec;
		time_spent = (double)(end-begin)/CLOCKS_PER_SEC;
		//printf("tb total cycle cnt = %d\n",tb_total_cycle_cnt);
		printf("simulation time: %f s,  %f s\n",time_spent, tv_end);
#endif
	}
	else if(s->write_buffer[0]=='p'  &&  s->write_buffer[1]=='\0')
	{ // set M0 interrupt
#if 1
		*shm_A5_int = 1;
		printf("trigger interrupt\n");
#endif
	}
	else if(s->write_buffer[0]=='s'  &&  s->write_buffer[1]=='\0')
	{  //clear M4 interrupt
#if 1
		*shm_M4_int = 0;
		printf("clear interrupt flag\n");
#endif
	}
	else if(s->write_buffer[0]=='z'  &&  s->write_buffer[1]=='\0')
	{ // set M4 interrupt
#if 1
		*shm_M4_int = 1;
		printf("trigger interrupt\n");
#endif
	}
	else if(s->write_buffer[0]=='j'  &&  s->write_buffer[1]=='\0')
	{  //clear M0 interrupt
#if 1
		*shm_A5_int = 0;
		printf("clear interrupt flag\n");
#endif
	}
    //}
}

static void host_relay_writew(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    int index;
    host_relay_state *s = (host_relay_state *)opaque;

    if (offset < WRITE_BUF_BASE || offset + 1 >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_relay_writeb: Bad offset 0x%x\n", (int)offset);
        return;
    }

    stw_be_p(&s->write_buffer[offset], value);
    for(index = 0; index <= 1; index++) {
        if(s->write_buffer[offset+index] == '\0') {
            DPRINTF("%s\n", s->write_buffer);
			if(s->write_buffer[0]=='a'  &&  s->write_buffer[1]=='\0')
	        {
				start_tracing = 1;
        	    DPRINTF("start trace log....\n");
#ifdef TRACE_TO_FILE
				TRACE_out_init("my_qemu_trace.trc"); //ctrl06
#endif
     	   	}
       		else if(s->write_buffer[0]=='b'  &&  s->write_buffer[1]=='\0')
        	{
				start_tracing = 0;
			
				//share memory
				while(1){
		 			if( shm_ptr[shm_head]!= shm_ptr[shm_tail] ){
						shm_ptr[ shm_ptr[shm_tail] ] = -1;
						DPRINTF(" End shm[%d] = %d\n",shm_ptr[shm_tail],shm_ptr[ shm_ptr[shm_tail] ]);
						shm_ptr[ shm_tail ]=shm_ptr[ shm_tail ]+2;
						shm_ptr[ shm_tail ]%= shm_max;
						break;
					}
				}

				//share memory for each cpu
				/*for(i = 0; i < smp_cpus; i++)
				{
					head = shm_head + (i * 2);
					tail = shm_tail + (i * 2);

					shm_ptr[tail] = (shm_ptr[tail] + 1) % ((shm_max / smp_cpus) * (i + 1));
					if(shm_ptr[tail] < (shm_max / smp_cpus) * i)
						shm_ptr[tail] += (shm_max / smp_cpus) * i;

					while(1){
						if(shm_ptr[tail] != shm_ptr[head]){
							shm_ptr[shm_ptr[tail]] = -1;
                    		DPRINTF(" End shm[%d] = %d\n",shm_ptr[tail],shm_ptr[shm_ptr[tail]]);
                    		break;
						}
					}
				}*/
				DPRINTF("end trace log....\n");
#ifdef TRACE_TO_FILE
				TRACE_out_done(); //ctrl06
#endif
        	}
			else if(s->write_buffer[0]=='c'  &&  s->write_buffer[1]=='\0')
			{
				my_trace_init();
			}
            return;
        }
    }
}

static void host_relay_writel(void * opaque, target_phys_addr_t offset,
                        uint32_t value)
{
    int index;
    host_relay_state *s = (host_relay_state *)opaque;

    if (offset < WRITE_BUF_BASE || offset + 3 >= WRITE_BUF_BASE + WRITE_BUF_SIZE) {
        hw_error("host_relay_writeb: Bad offset 0x%x\n", (int)offset);
        return;
    }

    stl_be_p(&s->write_buffer[offset], value);
    for(index = 0; index <= 3; index++) {
        if(s->write_buffer[offset+index] == '\0') {
            DPRINTF("%s\n", s->write_buffer);
    		if(s->write_buffer[0]=='a'  &&  s->write_buffer[1]=='\0')
	        {
				start_tracing = 1;
        	    DPRINTF("start trace log....\n");
#ifdef TRACE_TO_FILE
				TRACE_out_init("my_qemu_trace.trc"); //ctrl06
#endif
     	   	}
       		else if(s->write_buffer[0]=='b'  &&  s->write_buffer[1]=='\0')
        	{
				start_tracing = 0;
			
				//share memory
				while(1){
		 			if( shm_ptr[shm_head]!= shm_ptr[shm_tail] ){
						shm_ptr[ shm_ptr[shm_tail] ] = -1;
						DPRINTF(" End shm[%d] = %d\n",shm_ptr[shm_tail],shm_ptr[ shm_ptr[shm_tail] ]);
						shm_ptr[ shm_tail ]=shm_ptr[ shm_tail ]+2;
						shm_ptr[ shm_tail ]%= shm_max;
						break;
					}
				}

				//share memory for each cpu
				/*for(i = 0; i < smp_cpus; i++)
				{
					head = shm_head + (i * 2);
					tail = shm_tail + (i * 2);

					shm_ptr[tail] = (shm_ptr[tail] + 1) % ((shm_max / smp_cpus) * (i + 1));
					if(shm_ptr[tail] < (shm_max / smp_cpus) * i)
						shm_ptr[tail] += (shm_max / smp_cpus) * i;

					while(1){
						if(shm_ptr[tail] != shm_ptr[head]){
							shm_ptr[shm_ptr[tail]] = -1;
                    		DPRINTF(" End shm[%d] = %d\n",shm_ptr[tail],shm_ptr[shm_ptr[tail]]);
                    		break;
						}
					}
				}*/
				DPRINTF("end trace log....\n");
#ifdef TRACE_TO_FILE
				TRACE_out_done(); //ctrl06
#endif
        	}
			else if(s->write_buffer[0]=='c'  &&  s->write_buffer[1]=='\0')
			{
				my_trace_init();
			}
            return;
        }
    }
}

static CPUWriteMemoryFunc * const  host_relay_writefn[] = {
    host_relay_writeb,
    host_relay_writew,
    host_relay_writel
};

static CPUReadMemoryFunc * const  host_relay_readfn[] = {
    host_relay_readb,
    host_relay_readw,
    host_relay_readl
};

static int host_relay_init(SysBusDevice *dev)
{
    int iomemtype;
    host_relay_state *s = FROM_SYSBUS(host_relay_state, dev);

    iomemtype = cpu_register_io_memory(host_relay_readfn, host_relay_writefn, s,DEVICE_NATIVE_ENDIAN);
    if (iomemtype == -1) {
        hw_error("host_relay_init: Can't register I/O memory\n");
    }
    shm_A5_int_key = 311;
    shm_A5_int_id  = shmget(shm_A5_int_key,sizeof(int),IPC_CREAT|0666);
    if((shm_A5_int = (int*)shmat(shm_A5_int_id,NULL,0))==-1){
        perror("shmat");
        shmctl(shm_A5_int_id,IPC_RMID,0);
        exit(1);
    }
    shm_M4_int_key = 312;
    shm_M4_int_id  = shmget(shm_M4_int_key,sizeof(int),IPC_CREAT|0666);
    if((shm_M4_int = (int*)shmat(shm_M4_int_id,NULL,0))==-1){
        perror("shmat");
        shmctl(shm_M4_int_id,IPC_RMID,0);
        exit(1);
    }

    sysbus_init_mmio(dev, 0x1000, iomemtype);
    printf("host-relay init\n");
    return 0;
}

static void host_relay_register_devices(void)
{
    sysbus_register_dev("host_relay", sizeof(host_relay_state), host_relay_init);
}

device_init(host_relay_register_devices)
