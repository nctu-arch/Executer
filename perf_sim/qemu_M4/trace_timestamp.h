/* Copyright (C) 2006-2007 The Android Open Source Project
**
** This software is licensed under the terms of the GNU General Public
** License version 2, as published by the Free Software Foundation, and
** may be copied, distributed, and modified under those terms.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*/

//sue7785 2013.1.8 Modify from Android trace
#ifndef TRACE_H
#define TRACE_H

#include <inttypes.h>
#include <sys/shm.h> 
#include "trace_common.h"

//sue7785 2013.1.9 Modify from orig tracer

//trace log => shared mem
//#define TRACE_FILE_PATH "/home/wswu/tracerlog/mem.log"
#define SHMKEYPATH "/home/sue7785/QEMU_tracer/with_timestamp/qemu"
#define shm_size   256*256//1024*1024*12
#define shm_max    3145700
#define shm_head   3145701
#define shm_tail   3145702

#ifdef CONFIG_TRACE
#define CPU_TLB_BITS 8
#define CPU_TLB_SIZE (1 << CPU_TLB_BITS)
#endif
/*extern int      start_tracing;

extern key_t    shm_key;
extern int      shm_id;
extern int      *shm_ptr;

extern int      now_pid;
extern int      trace_pid;

extern uint64_t sim_time;
*/
void my_trace_log(/*unsigned long long item*/int cpu_id,int rw, unsigned long long addr, int size, int ts);
void my_trace_log_init(void);
void     my_trace_init(void);
void my_trace_end(void);
//timestamp
int      get_insn_ticks_arm(uint32_t insn);
int      get_insn_ticks_thumb(uint32_t  insn);

extern int ld_cnt;
extern int st_cnt;
//write to file 

#endif /* TRACE_H */
