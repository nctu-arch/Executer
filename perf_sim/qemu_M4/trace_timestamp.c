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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "cpu.h"
#include "exec-all.h"
#include "trace_timestamp.h"

//sue7785 2013.1.9 Modify from orig tracer
#include <sys/ipc.h>
#include <sys/shm.h>
#include "sysemu.h"  //to get smp_cpus

/* the following array is used to deal with def-use register interlocks, which we
 * can compute statically (ignoring conditions), very fortunately.
 *
 * the idea is that interlock_base contains the number of cycles "executed" from
 * the start of a basic block. It is set to 0 in trace_bb_start, and incremented
 * in each call to get_insn_ticks_arm.
 *
 * interlocks[N] correspond to the value of interlock_base after which a register N
 * can be used by another operation, it is set each time an instruction writes to
 * the register in get_insn_ticks()
 */
#ifdef TRANSFORMER_KIND
#include "insn_code.h"
#endif
typedef struct trace_format{
	int cpu_id;
	int RW;
	unsigned long long addr;
	unsigned int size;
	int TS; //yhchiang 20160328
}trace_format;
int   start_tracing =0;
int trace_log_flag;
key_t shm_key;
int   shm_id;
int   shm_id_t;
int shm_id_head, shm_id_tail;
//unsigned int   *shm_ptr; //yhchiang comment 20160324
/* yhchiang 20160324 */
trace_format *shm_ptr;
trace_format print_out;
int* shm_head_index;
int* shm_tail_index;
volatile int mem_access = 0;
uint64_t sim_time = 0;
uint64_t inst_count =0;//wswu
extern uint32_t* shm2_ptr;

int* shm_L1readhit;
int* shm_L1writehit;
int* shm_L2readhit;
int* shm_L2writehit;
int L1readhit_id;
int L1writehit_id;
int L2readhit_id;
int L2writehit_id;
int ld_cnt=0;
int st_cnt=0;
FILE                    *trace_log_file;
FILE  * insn_trace_file;
FILE  * insn_trace_file_A;
char                    trace_log_buffer[8192];

static int interlocks[16];
static int interlock_base;
int mode = 0;
//FILE *result;
static void
_interlock_def(int  reg, int  delay)
{
    if (reg >= 0)
        interlocks[reg] = interlock_base + delay;
}

static int
_interlock_use(int  reg)
{
    int  delay = 0;

    if (reg >= 0)
    {
        delay = interlocks[reg] - interlock_base;
        if (delay < 0)
            delay = 0;
    }
    return delay;
}
void delay_t(){
	int i=1000;
	for(;i>0;i--){};
}
void my_trace_log(/*unsigned long long item*/int cpu_id, int rw, unsigned long long addr_t, int size_h, int ts ) //sue7785
{

	/*volatile int i,k,j=9;
	//printf(" ");
	for(i=0;i<1000;i++){
		//for(k=0;k<1000;k++)
		j^=1;	
	}
	return ;*/
	//mem_access++;
#if 1
        print_out.cpu_id = cpu_id;
	print_out.RW = rw;
	print_out.addr = addr_t;
	print_out.size = size_h;
	print_out.TS = ts;
    if(rw==2)ld_cnt++;
    else if(rw==3)st_cnt++;
	int volatile head,tail;
	tail = *shm_tail_index;
	head = *shm_head_index;
	if((tail+1)%100!=head/*(*shm_tail_index+1)!=*shm_head_index*/)//not full
	{
		//shm_ptr[*shm_tail_index] = print_out;
		//*shm_tail_index+=1;
		//*shm_tail_index%=100;
		//delay_t();
		shm_ptr[tail] = print_out;
		tail +=1;
		tail%=100;
		*shm_tail_index = tail;
		//printf("head %d\n",*shm_head_index);
		//printf("tail %d\n",*shm_tail_index);
	}
	else 
	{
		//while((*shm_tail_index+1)==*shm_head_index);
		while((tail+1)%100==head){
			//tail = *shm_tail_index;
			//head = *shm_head_index;
			memcpy(&head,shm_head_index,sizeof(int));
		}
		shm_ptr[tail] = print_out;
		tail +=1;
		tail%=100;
		*shm_tail_index = tail;

	}

	return;	
#endif	
}

void my_trace_log_init(void)
{
	if((trace_log_file = fopen(TRACE_FILE_PATH, "w+")) == NULL)
        {
                printf("Can't open trace log file\n");
                exit(1);
        }
	
	/*if((insn_trace_file = fopen(INSN_TRACE_FILE_PATH, "w+")) == NULL)
        {
                printf("Can't open insn trace log file\n");
                exit(1);
        }*/
	if((insn_trace_file_A = fopen(INSN_TRACE_FILE_PATH_A, "w+")) == NULL)
        {
                printf("Can't open insn trace A log file\n");
                exit(1);
        }
	/*if((result = fopen(CYCLE_RESULT, "w+")) == NULL)
	{
		printf("Can't open result2 file\n");
		exit(1);
	}*/
#ifdef TRANSFORMER_KIND
	//result = fopen("/home/yhchiang/tracerlog/testA","w");
#endif
	printf("Start tracing......\n");

}

void my_trace_init(void) //sue7785
{
        //start_tracing = 0;

        /*// 2013.03.14 sue7785 Add write to file
        if((trace_log_file = fopen("trace.log", "w+")) == NULL)
        {
                printf("Can't open\n");
                exit(1);
        }
        setvbuf(trace_log_file, trace_log_buffer, _IOFBF, sizeof(trace_log_buffer));

        int nn;
        interlock_base = 0;
        for (nn = 0; nn < 16; nn++)
                interlocks[nn] = 0;

        
        int c;
        shm_key = 800;
        shm_id  = shmget(shm_key,shm_size,IPC_CREAT|0666);
        perror("shmget");

        if((shm_ptr = (int *)shmat(shm_id,NULL,0)) == -1){
                printf("shmat1 error:%x\n",shm_ptr);
                perror("shmat");
                shmctl(shm_id,IPC_RMID,0);
                exit(-1);
        }*/

#ifdef CONFIG_TRACE
	CPUState *env;
        for(env=first_cpu;env;env=env->next_cpu){
		env->sim_time=0;
		env->inst_count=0;
		env->ld_count=0;
		env->st_count=0;
	}
#endif

#ifdef CONFIG_SHMEM    
	int c;

	shm_key = 800;
        //shm_id  = shmget(shm_key,shm_size,IPC_CREAT|0666);
        shm_id  = shmget(shm_key,100*sizeof(trace_format),IPC_CREAT|0666);
	shm_id_head  = shmget(700,sizeof(int),IPC_CREAT|0666);	
	shm_id_tail  = shmget(799,sizeof(int),IPC_CREAT|0666);	
#if 1
	if ((L1readhit_id = shmget(721, sizeof(int), IPC_CREAT|0666)) < 0) {
		//perror("shmget");
		//exit(1);
	}
	if ((L1writehit_id = shmget(722, sizeof(int), IPC_CREAT|0666)) < 0) {
		//perror("shmget");
		//exit(1);
	}
	if ((L2readhit_id = shmget(723, sizeof(int), IPC_CREAT|0666)) < 0) {
		//perror("shmget");
		//exit(1);
	}
	if ((L2writehit_id = shmget(724, sizeof(int), IPC_CREAT|0666)) < 0) {
		//perror("shmget");
		//exit(1);
	}
#endif
	 perror("shmget");

        /*if((shm_ptr = (unsigned int *)shmat(shm_id,NULL,0)) == -1){
                printf("shmat1 error:%x\n",shm_ptr);
                perror("shmat");
                shmctl(shm_id,IPC_RMID,0);
                exit(-1);
        }*/
        if((shm_ptr = (trace_format *)shmat(shm_id,NULL,0)) == -1){
                printf("shmat1 error:%x\n",shm_ptr);
                perror("shmat");
                shmctl(shm_id,IPC_RMID,0);
                exit(-1);
        }
        if((shm_head_index = (int *)shmat(shm_id_head,NULL,0)) == -1){
                printf("shmat1 error:%x\n",shm_ptr);
                perror("shmat");
                shmctl(shm_id,IPC_RMID,0);
                exit(-1);
        }
        if((shm_tail_index = (int *)shmat(shm_id_tail,NULL,0)) == -1){
                printf("shmat1 error:%x\n",shm_ptr);
                perror("shmat");
                shmctl(shm_id,IPC_RMID,0);
                exit(-1);
        }
#if 1

	if ((shm_L1readhit = (int *)shmat(L1readhit_id, NULL, 0)) == (int *) -1) {
		printf("yoki\n");
		perror("shmat");
		exit(1);
	}
	if ((shm_L1writehit = (int *)shmat(L1writehit_id, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}
	if ((shm_L2readhit = (int *)shmat(L2readhit_id, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}
	if ((shm_L2writehit = (int *)shmat(L2writehit_id, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}
#endif
	/*shm_key = 339;
        shm_id_t  = shmget(shm_key,200,IPC_CREAT|0666);
        if((shm2_ptr = (uint32_t*)shmat(shm_id_t,NULL,0))==-1){
	        perror("shmat");
	        shmctl(shm_id_t,IPC_RMID,0);
	        exit(1);
	}*/

        perror("shmat");
    printf("key:%d  id:%d  ptr:%x\n",shm_key,shm_id,shm_ptr);
#if 0
    for(c=0;c<=100;c++){
                shm_ptr[c] = 0;
        }
#endif
    //share memory
    shm_head_index[0] = 0;
    shm_tail_index[0] = 1;
    #if 0
    shm_ptr[shm_head] = 0;
    shm_ptr[shm_tail] = 1;
    #endif
	printf("finish init shmem\n");
#endif
    //share memory for each cpu
    /*for(i = 0; i < smp_cpus; i++){
          shm_ptr[shm_head + (i * 2)] = (shm_max / smp_cpus) * i;
          shm_ptr[shm_tail + (i * 2)] = (shm_max / smp_cpus) * i;
        }*/
#ifdef CONFIG_TRACEFILE
                        my_trace_log_init();//wswu
#endif
}

void my_trace_end(void)//wswu
{
	//share memory
	int volatile tail,head;
#ifdef CONFIG_SHMEM
	while(shm_tail_index[0]==shm_head_index[0]);

		 while((*shm_head_index+1)%100!=*shm_tail_index){
                 //printf("head %d\n",*shm_head_index);
                 //printf("tail %d\n",*shm_tail_index);
                 memcpy(&head,shm_head_index,sizeof(int));
                 memcpy(&tail,shm_tail_index,sizeof(int));
                 if((head+1)%100==tail)break;
 
         }
         printf("L1readhit %d, L1writehit %d, L2readhit %d, L2writehit %d\n",*shm_L1readhit, *shm_L1writehit, *shm_L2readhit, *shm_L2writehit);
		 //printf("store = %d, load = %d\n",st_cnt,ld_cnt);
		
	print_out.RW = -1;
	shm_ptr[shm_tail_index[0]] = print_out;
	memcpy(&tail,shm_tail_index,sizeof(int));
	tail+=2;
	memcpy(shm_tail_index,&tail,sizeof(int));
         
		 
	//my_trace_init();//yhchiang 20160328	
	/*while(1){
		if( shm_ptr[shm_head]!= shm_ptr[shm_tail] ){
			shm_ptr[ shm_ptr[shm_tail] ] = -1;
			printf(" End shm[%d] = %d\n",shm_ptr[shm_tail],shm_ptr[ shm_ptr[shm_tail] ]);
			shm_ptr[ shm_tail ]=shm_ptr[ shm_tail ]+2;
			shm_ptr[ shm_tail ]%= shm_max;
			break;
		}
	}*/

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
#endif
#ifdef CONFIG_TRACEFILE
	fflush(trace_log_file);
	fclose(trace_log_file);//wswu
	fflush(insn_trace_file_A);
	fclose(insn_trace_file_A);
#endif
	printf("end trace log....\n");
	//fflush(insn_trace_file);
	//fclose(insn_trace_file);
}

// Define the number of clock ticks for some instructions.  Add one to these
// (in some cases) if there is an interlock.  We currently do not check for
// interlocks.
#define TICKS_OTHER	1
#define TICKS_SMULxy	1
#define TICKS_SMLAWy	1
#define TICKS_SMLALxy	2
#define TICKS_MUL	2
#define TICKS_MLA	2
#define TICKS_MULS	4	// no interlock penalty
#define TICKS_MLAS	4	// no interlock penalty
#define TICKS_UMULL	3
#define TICKS_UMLAL	3
#define TICKS_SMULL	3
#define TICKS_SMLAL	3
#define TICKS_UMULLS	5	// no interlock penalty
#define TICKS_UMLALS	5	// no interlock penalty
#define TICKS_SMULLS	5	// no interlock penalty
#define TICKS_SMLALS	5	// no interlock penalty

// Compute the number of cycles that this instruction will take,
// not including any I-cache or D-cache misses.  This function
// is called for each instruction in a basic block when that
// block is being translated.
int get_insn_ticks_arm(uint32_t insn)
{
    int   result   =  1;   /* by default, use 1 cycle */

    /* See Chapter 12 of the ARM920T Reference Manual for details about clock cycles */

    /* first check for invalid condition codes */
    if ((insn >> 28) == 0xf)
    {
        if ((insn >> 25) == 0x7d) {  /* BLX */
            result = 3;
            goto Exit;
        }
        /* XXX: if we get there, we're either in an UNDEFINED instruction     */
        /*      or in co-processor related ones. For now, only return 1 cycle */
        goto Exit;
    }

    /* other cases */
    switch ((insn >> 25) & 7)
    {
        case 0:
            if ((insn & 0x00000090) == 0x00000090)  /* Multiplies, extra load/store, Table 3-2 */
            {
                /* XXX: TODO: Add support for multiplier operand content penalties in the translator */

                if ((insn & 0x0fc000f0) == 0x00000090)   /* 3-2: Multiply (accumulate) */
                {
                    int  Rm = (insn & 15);
                    int  Rs = (insn >> 8) & 15;
                    int  Rn = (insn >> 12) & 15;

                    if ((insn & 0x00200000) != 0) {  /* MLA */
                        result += _interlock_use(Rn);
                    } else {   /* MLU */
                        if (Rn != 0)      /* UNDEFINED */
                            goto Exit;
                    }
                    /* cycles=2+m, assume m=1, this should be adjusted at interpretation time */
                    result += 2 + _interlock_use(Rm) + _interlock_use(Rs);
                }
                else if ((insn & 0x0f8000f0) == 0x00800090)  /* 3-2: Multiply (accumulate) long */
                {
                    int  Rm   = (insn & 15);
                    int  Rs   = (insn >> 8) & 15;
                    int  RdLo = (insn >> 12) & 15;
                    int  RdHi = (insn >> 16) & 15;

                    if ((insn & 0x00200000) != 0) { /* SMLAL & UMLAL */
                        result += _interlock_use(RdLo) + _interlock_use(RdHi);
                    }
                    /* else SMLL and UMLL */

                    /* cucles=3+m, assume m=1, this should be adjusted at interpretation time */
                    result += 3 + _interlock_use(Rm) + _interlock_use(Rs);
                }
                else if ((insn & 0x0fd00ff0) == 0x01000090)  /* 3-2: Swap/swap byte */
                {
                    int  Rm = (insn & 15);
                    int  Rd = (insn >> 8) & 15;

                    result = 2 + _interlock_use(Rm);
                    _interlock_def(Rd, result+1);
                }
                else if ((insn & 0x0e400ff0) == 0x00000090)  /* 3-2: load/store halfword, reg offset */
                {
                    int  Rm = (insn & 15);
                    int  Rd = (insn >> 12) & 15;
                    int  Rn = (insn >> 16) & 15;

                    result += _interlock_use(Rn) + _interlock_use(Rm);
                    if ((insn & 0x00100000) != 0)  /* it's a load, there's a 2-cycle interlock */
                        _interlock_def(Rd, result+2);
                }
                else if ((insn & 0x0e400ff0) == 0x00400090)  /* 3-2: load/store halfword, imm offset */
                {
                    int  Rd = (insn >> 12) & 15;
                    int  Rn = (insn >> 16) & 15;

                    result += _interlock_use(Rn);
                    if ((insn & 0x00100000) != 0)  /* it's a load, there's a 2-cycle interlock */
                        _interlock_def(Rd, result+2);
                }
                else if ((insn & 0x0e500fd0) == 0x000000d0) /* 3-2: load/store two words, reg offset */
                {
                    /* XXX: TODO: Enhanced DSP instructions */
                }
                else if ((insn & 0x0e500fd0) == 0x001000d0) /* 3-2: load/store half/byte, reg offset */
                {
                    int  Rm = (insn & 15);
                    int  Rd = (insn >> 12) & 15;
                    int  Rn = (insn >> 16) & 15;

                    result += _interlock_use(Rn) + _interlock_use(Rm);
                    if ((insn & 0x00100000) != 0)  /* load, 2-cycle interlock */
                        _interlock_def(Rd, result+2);
                }
                else if ((insn & 0x0e5000d0) == 0x004000d0) /* 3-2: load/store two words, imm offset */
                {
                    /* XXX: TODO: Enhanced DSP instructions */
                }
                else if ((insn & 0x0e5000d0) == 0x005000d0) /* 3-2: load/store half/byte, imm offset */
                {
                    int  Rd = (insn >> 12) & 15;
                    int  Rn = (insn >> 16) & 15;

                    result += _interlock_use(Rn);
                    if ((insn & 0x00100000) != 0)  /* load, 2-cycle interlock */
                        _interlock_def(Rd, result+2);
                }
                else
                {
                    /* UNDEFINED */
                }
            }
            else if ((insn & 0x0f900000) == 0x01000000)  /* Misc. instructions, table 3-3 */
            {
                switch ((insn >> 4) & 15)
                {
                    case 0:
                        if ((insn & 0x0fb0fff0) == 0x0120f000) /* move register to status register */
                        {
                            int  Rm = (insn & 15);
                            result += _interlock_use(Rm);
                        }
                        break;

                    case 1:
                        if ( ((insn & 0x0ffffff0) == 0x01200010) ||  /* branch/exchange */
                             ((insn & 0x0fff0ff0) == 0x01600010) )   /* count leading zeroes */
                        {
                            int  Rm = (insn & 15);
                            result += _interlock_use(Rm);
                        }
                        break;

                    case 3:
                        if ((insn & 0x0ffffff0) == 0x01200030)   /* link/exchange */
                        {
                            int  Rm = (insn & 15);
                            result += _interlock_use(Rm);
                        }
                        break;

                    default:
                        /* TODO: Enhanced DSP instructions */
                        ;
                }
            }
            else  /* Data processing */
            {
                int  Rm = (insn & 15);
                int  Rn = (insn >> 16) & 15;

                result += _interlock_use(Rn) + _interlock_use(Rm);
                if ((insn & 0x10)) {   /* register-controlled shift => 1 cycle penalty */
                    int  Rs = (insn >> 8) & 15;
                    result += 1 + _interlock_use(Rs);
                }
            }
            break;

        case 1:
            if ((insn & 0x01900000) == 0x01900000)
            {
                /* either UNDEFINED or move immediate to CPSR */
            }
            else  /* Data processing immediate */
            {
                int  Rn = (insn >> 12) & 15;
                result += _interlock_use(Rn);
            }
            break;

        case 2:  /* load/store immediate */
            {
                int  Rn = (insn >> 16) & 15;

                result += _interlock_use(Rn);
                if (insn & 0x00100000) {  /* LDR */
                    int  Rd = (insn >> 12) & 15;

                    if (Rd == 15)  /* loading PC */
                        result = 5;
                    else
                        _interlock_def(Rd,result+1);
                }
            }
            break;

        case 3:
            if ((insn & 0x10) == 0)  /* load/store register offset */
            {
                int  Rm = (insn & 15);
                int  Rn = (insn >> 16) & 15;

                result += _interlock_use(Rm) + _interlock_use(Rn);

                if (insn & 0x00100000) {  /* LDR */
                    int  Rd = (insn >> 12) & 15;
                    if (Rd == 15)
                        result = 5;
                    else
                        _interlock_def(Rd,result+1);
                }
            }
            /* else UNDEFINED */
            break;

        case 4:  /* load/store multiple */
            {
                int       Rn   = (insn >> 16) & 15;
                uint32_t  mask = (insn & 0xffff);
                int       count;

                for (count = 0; mask; count++)
                    mask &= (mask-1);

                result += _interlock_use(Rn);

                if (insn & 0x00100000)  /* LDM */
                {
                    int  nn;

                    if (insn & 0x8000) {  /* loading PC */
                        result = count+4;
                    } else {  /* not loading PC */
                        result = (count < 2) ? 2 : count;
                    }
                    /* create defs, all registers locked until the end of the load */
                    for (nn = 0; nn < 15; nn++)
                        if ((insn & (1U << nn)) != 0)
                            _interlock_def(nn,result);
                }
                else  /* STM */
                    result = (count < 2) ? 2 : count;
            }
            break;

        case 5:  /* branch and branch+link */
            break;

        case 6:  /* coprocessor load/store */
            {
                int  Rn = (insn >> 16) & 15;

                if (insn & 0x00100000)
                    result += _interlock_use(Rn);

                /* XXX: other things to do ? */
            }
            break;

        default: /* i.e. 7 */
            /* XXX: TODO: co-processor related things */
            ;
    }
Exit:
    interlock_base += result;
    return result;
}

int  get_insn_ticks_thumb(uint32_t  insn)
{
#if 1
    int  result = 1;

    switch ((insn >> 11) & 31)
    {
        case 0:
        case 1:
        case 2:   /* Shift by immediate */
            {
                int  Rm = (insn >> 3) & 7;
                result += _interlock_use(Rm);
            }
            break;

        case 3:  /* Add/Substract */
            {
                int  Rn = (insn >> 3) & 7;
                result += _interlock_use(Rn);

                if ((insn & 0x0400) == 0) {  /* register value */
                    int  Rm = (insn >> 6) & 7;
                    result += _interlock_use(Rm);
                }
            }
            break;

        case 4:  /* move immediate */
            break;

        case 5:
        case 6:
        case 7:  /* add/substract/compare immediate */
            {
                int  Rd = (insn >> 8) & 7;
                result += _interlock_use(Rd);
            }
            break;

        case 8:
            {
                if ((insn & 0x0400) == 0)  /* data processing register */
                {
                    /* the registers can also be Rs and Rn in some cases */
                    /* but they're always read anyway and located at the */
                    /* same place, so we don't check the opcode          */
                    int  Rm = (insn >> 3) & 7;
                    int  Rd = (insn >> 3) & 7;

                    result += _interlock_use(Rm) + _interlock_use(Rd);
                }
                else switch ((insn >> 8) & 3)
                {
                    case 0:
                    case 1:
                    case 2:  /* special data processing */
                        {
                            int  Rn = (insn & 7) | ((insn >> 4) & 0x8);
                            int  Rm = ((insn >> 3) & 15);

                            result += _interlock_use(Rn) + _interlock_use(Rm);
                        }
                        break;

                    case 3:
                        if ((insn & 0xff07) == 0x4700)  /* branch/exchange */
                        {
                            int  Rm = (insn >> 3) & 15;

                            result = 3 + _interlock_use(Rm);
                        }
                        /* else UNDEFINED */
                        break;
                }
            }
            break;

        case 9:  /* load from literal pool */
            {
                int  Rd = (insn >> 8) & 7;
                _interlock_def(Rd,result+1);
            }
            break;

        case 10:
        case 11:  /* load/store register offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;
                int  Rm = (insn >> 6) & 7;

                result += _interlock_use(Rn) + _interlock_use(Rm);

                switch ((insn >> 9) & 7)
                {
                    case 0: /* STR  */
                    case 1: /* STRH */
                    case 2: /* STRB */
                        result += _interlock_use(Rd);
                        break;

                    case 3: /* LDRSB */
                    case 5: /* LDRH */
                    case 6: /* LDRB */
                    case 7: /* LDRSH */
                        _interlock_def(Rd,result+2);
                        break;

                    case 4: /* LDR */
                        _interlock_def(Rd,result+1);
                }
            }
            break;

        case 12:  /* store word immediate offset */
        case 14:  /* store byte immediate offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;

                result += _interlock_use(Rd) + _interlock_use(Rn);
            }
            break;

        case 13:  /* load word immediate offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;

                result += _interlock_use(Rn);
                _interlock_def(Rd,result+1);
            }
            break;

        case 15:  /* load byte immediate offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;

                result += _interlock_use(Rn);
                _interlock_def(Rd,result+2);
            }
            break;

        case 16:  /* store halfword immediate offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;

                result += _interlock_use(Rn) + _interlock_use(Rd);
            }
            break;

        case 17:  /* load halfword immediate offset */
            {
                int  Rd = (insn & 7);
                int  Rn = (insn >> 3) & 7;

                result += _interlock_use(Rn);
                _interlock_def(Rd,result+2);
            }
            break;

        case 18:  /* store to stack */
            {
                int  Rd = (insn >> 8) & 3;
                result += _interlock_use(Rd);
            }
            break;

        case 19:  /* load from stack */
            {
                int  Rd = (insn >> 8) & 3;
                _interlock_def(Rd,result+1);
            }
            break;

        case 20:  /* add to PC */
        case 21:  /* add to SP */
            {
                int  Rd = (insn >> 8) & 3;
                result += _interlock_use(Rd);
            }
            break;

        case 22:
        case 23:  /* misc. instructions, table 6-2 */
            {
                if ((insn & 0xff00) == 0xb000)  /* adjust stack pointer */
                {
                    result += _interlock_use(14);
                }
                else if ((insn & 0x0600) == 0x0400)  /* push pop register list */
                {
                    uint32_t  mask = insn & 0x01ff;
                    int       count, nn;

                    for (count = 0; mask; count++)
                        mask &= (mask-1);

                    result = (count < 2) ? 2 : count;

                    if (insn & 0x0800)  /* pop register list */
                    {
                        for (nn = 0; nn < 9; nn++)
                            if (insn & (1 << nn))
                                _interlock_def(nn, result);
                    }
                    else  /* push register list */
                    {
                        for (nn = 0; nn < 9; nn++)
                            if (insn & (1 << nn))
                                result += _interlock_use(nn);
                    }
                }
                /* else  software breakpoint */
            }
            break;

        case 24:  /* store multiple */
            {
                int  Rd = (insn >> 8) & 7;
                uint32_t  mask = insn & 255;
                int       count, nn;

                for (count = 0; mask; count++)
                    mask &= (mask-1);

                result = (count < 2) ? 2 : count;
                result += _interlock_use(Rd);

                for (nn = 0; nn < 8; nn++)
                    if (insn & (1 << nn))
                        result += _interlock_use(nn);
            }
            break;

        case 25:  /* load multiple */
            {
                int  Rd = (insn >> 8) & 7;
                uint32_t  mask = insn & 255;
                int       count, nn;

                for (count = 0; mask; count++)
                    mask &= (mask-1);

                result  = (count < 2) ? 2 : count;
                result += _interlock_use(Rd);

                for (nn = 0; nn < 8; nn++)
                    if (insn & (1 << nn))
                        _interlock_def(nn, result);
            }
            break;

        case 26:
        case 27:  /* conditional branch / undefined / software interrupt */
            switch ((insn >> 8) & 15)
            {
                case 14: /* UNDEFINED */
                case 15: /* SWI */
                    break;

                default:  /* conditional branch */
                    result = 3;
            }
            break;

        case 28:  /* unconditional branch */
            result = 3;
            break;

        case 29:  /* BLX suffix or undefined */
            if ((insn & 1) == 0)
                result = 3;
            break;

        case 30:  /* BLX/BLX prefix */
            break;

        case 31:  /* BL suffix */
            result = 3;
            break;
    }
    interlock_base += result;
    return result;
#else /* old code */
    if ((insn & 0xfc00) == 0x4340) /* MUL */
        return TICKS_SMULxy;

    return TICKS_OTHER;
#endif
}
