extern "C"{
#include "mem-system/module.h"
#include "lib/util/file.h"
}
//for share memory
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "include_shm/Inc.h"
#include "include_shm/mcore_module.h"
#include "include_shm/cpu_module.h"

//for config shared memory
#include "include_shm/cache.h"
#include "include_shm/cache_module.h"
//#include "coherence_module.h"
#include "include_shm/memory_module.h"
#define Short_Latency 2

#include <lib/esim/esim.h>
#include <lib/util/file.h>
#include <mem-system/module.h>
#include <arch/x86/timing/cpu.h>
#include <cstdio>
#include <cstring>
#include "trace_driven.h"

#define SHMSZ     1024*1024*12
#define shm_head 3145701
#define shm_tail 3145702
#define shm_max  3145700 

#define CPU_NUM 4

using namespace std;


char * mem_trace_driven_file_name_C0="";
char * mem_trace_driven_file_name_C1="";
char * mem_trace_driven_file_name_C2="";
char * mem_trace_driven_file_name_C3="";
FILE *mem_access_trace_f_C0; 
FILE *mem_access_trace_f_C1; 
FILE *mem_access_trace_f_C2; 
FILE *mem_access_trace_f_C3; 

char * inst_trace_driven_file_name_C0="";
char * inst_trace_driven_file_name_C1="";
char * inst_trace_driven_file_name_C2="";
char * inst_trace_driven_file_name_C3="";
FILE *inst_access_trace_f_C0; 
FILE *inst_access_trace_f_C1; 
FILE *inst_access_trace_f_C2; 
FILE *inst_access_trace_f_C3; 

char bufferC0[80000]; 
char bufferC1[80000]; 
char bufferC2[80000]; 
char bufferC3[80000];
typedef struct  {
	unsigned int type:8;
	unsigned int time_difference:24;
	unsigned int address:32;
}basic_trace;

char bufferIC0[4000]; 
char bufferIC1[4000]; 
char bufferIC2[4000]; 
char bufferIC3[4000];
typedef struct  {
	unsigned int time_difference:32;
	unsigned int address:32;
}basic_trace_I;

int total_instr;
CPU_module *CPU[256];
Memory_module *Mem;

void trace_driven_init()
{
			
	mem_access_trace_f_C0  = file_open_for_read(mem_trace_driven_file_name_C0);
#if 0 
	mem_access_trace_f_C1  = file_open_for_read(mem_trace_driven_file_name_C1); 
	mem_access_trace_f_C2  = file_open_for_read(mem_trace_driven_file_name_C2); 
	mem_access_trace_f_C3  = file_open_for_read(mem_trace_driven_file_name_C3); 
#endif
	//inst_access_trace_f_C0  = file_open_for_read(inst_trace_driven_file_name_C0); 
	//inst_access_trace_f_C1  = file_open_for_read(inst_trace_driven_file_name_C1); 
	//inst_access_trace_f_C2  = file_open_for_read(inst_trace_driven_file_name_C2); 
	//inst_access_trace_f_C3  = file_open_for_read(inst_trace_driven_file_name_C3); 
	
	//hchung 2014_11_26 for Qemu trace
	if(mem_access_trace_f_C0 == NULL )
	{
		printf("trace driven open file mem C0  fail !!!!!!!!!!!!\n");
		//fatal("trace driven open file fail!");
		printf("trace driven open file fail!");
	}
#if 0
	if(mem_access_trace_f_C1 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C1 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");
	}
	if(mem_access_trace_f_C2 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C2 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");
	}
	if(mem_access_trace_f_C3 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C3 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");
	}
#endif
	//hchung 2014_11_26 for Qemu trace
	/*if(inst_access_trace_f_C0 == NULL )
	{
		printf("trace driven open file inst C0  fail  %s !!!!!!!!!!!!\n",inst_trace_driven_file_name_C0 );
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C1 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C1 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C2 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C2 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}
	if(inst_access_trace_f_C3 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file inst C3 fail !!!!!!!!!!!!\n");
		fatal("trace driven open file fail!");		
	}*/
	printf("openfile successful!!!!!\n");
}



unsigned short int trace_driven_read_trace(int core)
{
	if( core == 0 )return  fread (bufferC0,8,10000,mem_access_trace_f_C0);
	else if( core == 1 )return  fread (bufferC1,8,10000,mem_access_trace_f_C1);
	else if( core == 2 )return  fread (bufferC2,8,10000,mem_access_trace_f_C2);
	else if( core == 3 )return  fread (bufferC3,8,10000,mem_access_trace_f_C3);

	//fatal("Core number error!!??\n");
	printf("Core number error!!??\n");
}

unsigned short int trace_driven_read_trace_I(int core)
{
	if( core == 0 )return  fread (bufferIC0,8,500,inst_access_trace_f_C0);
	else if( core == 1 )return  fread (bufferIC1,8,500,inst_access_trace_f_C1);
	else if( core == 2 )return  fread (bufferIC2,8,500,inst_access_trace_f_C2);
	else if( core == 3 )return  fread (bufferIC3,8,500,inst_access_trace_f_C3);

	//fatal("Core number error!!??\n");
	printf("Core number error!!??\n");
}

int trace_driven_run_inst()
{
	static unsigned int phy_addr_0,phy_addr_1,phy_addr_2,phy_addr_3;
	static int time_diff_0 = 0,time_diff_1 = 0, time_diff_2 = 0, time_diff_3 = 0;
	static int state0 = 0,state1 = 0,state2 = 0, state3 = 0;
	
	static unsigned short int read_c0 = 0, read_c1 = 0,read_c2 = 0,read_c3 = 0;
	static unsigned short int issue_c0 = 0, issue_c1 = 0,issue_c2 = 0,issue_c3 = 0;
	static char * point0,* point1,* point2,* point3;

	basic_trace_I tmp1;

	if(  state0 == 0 )
	{
			state0 =1;
			issue_c0 =read_c0 = trace_driven_read_trace_I(0); 
			time_diff_0=0;
			point0 = bufferIC0; 
	}
	if(  state1 == 0 )
	{
			state1 =1;
		    issue_c1 =	read_c1 = trace_driven_read_trace_I(1); 
			time_diff_1=0;
			point1 = bufferIC1; 
	}
	if(  state2 == 0 )
	{
			state2 =1;
			issue_c2 =read_c2 = trace_driven_read_trace_I(2); 
			time_diff_2=0;
			point2 = bufferIC2; 
	}
	if(  state3 == 0 )
	{
			state3 =1;
			issue_c3 =read_c3 = trace_driven_read_trace_I(3); 
			time_diff_3=0;
			point3 = bufferIC3; 
	}

//	printf("  %d  %d  %d  %d  |  %d  %d %d  %d  \n",issue_c0,issue_c1,issue_c2,issue_c3,read_c0,read_c1,read_c2,read_c3);
//	printf("1  %d  %d  \n",issue_c1,read_c1);
//	printf("2  %d  %d  \n",issue_c2,read_c2);
//	printf("3  %d  %d  \n",issue_c3,read_c3);
	while(read_c0 != 0){
			if(time_diff_0 == 0 && issue_c0 != 0){
					memcpy(&tmp1,point0,8);
		            time_diff_0 = tmp1.time_difference;
		            phy_addr_0 = tmp1.address;
					point0 = point0 + 8;
					if(time_diff_0 != 0 )time_diff_0++; 
				//	time_diff_0++;
		//	printf("C0   %d  %x \n",time_diff_0,phy_addr_0);
			}

			if(time_diff_0 != 0)time_diff_0--;

		if(time_diff_0 == 0 && issue_c0 != 0)
		{
			//printf("C0 esim_cycle: %lld     %x\n", esim_cycle, phy_addr_0);
			mod_access( (*((*x86_cpu).core[0]).thread).inst_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
			issue_c0--;
//			printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
			if(issue_c0 == 0)
			{
				issue_c0 = read_c0 = trace_driven_read_trace_I(0); 
				time_diff_0=0;
				point0 = bufferIC0; 
			}
		}
		else
			break;
	}

	while(read_c1 != 0){
			if(time_diff_1 == 0 && issue_c1 != 0){
					memcpy(&tmp1,point1,8);
		            time_diff_1 = tmp1.time_difference;
		            phy_addr_1 = tmp1.address;
					point1 = point1 + 8;
					if(time_diff_1 != 0 )time_diff_1++; 
				//	time_diff_0++;
	//		printf("C1 %d  %d  %x \n",kind1,time_diff_1,phy_addr_1);
			}
			 if(time_diff_1 != 0)time_diff_1--;

		if(time_diff_1 == 0 && issue_c1 != 0)
		{
			//printf("C1 esim_cycle: %lld     %x\n", esim_cycle, phy_addr_1);
			mod_access( (*((*x86_cpu).core[1]).thread).inst_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
			issue_c1--;
			if(issue_c1 == 0)
			{
				issue_c1 = read_c1 = trace_driven_read_trace_I(1); 
				time_diff_1=0;
				point1 = bufferIC1; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}

	while(read_c2 != 0){
			if(time_diff_2 == 0 && issue_c2 != 0){
					memcpy(&tmp1,point2,8);
		            time_diff_2 = tmp1.time_difference;
		            phy_addr_2 = tmp1.address;
					point2 = point2 + 8;
					if(time_diff_2 != 0 )time_diff_2++; 
	//		printf("C2 %d  %d  %x \n",kind2,time_diff_2,phy_addr_2);
			}
			 if(time_diff_2 != 0)time_diff_2--;

		if(time_diff_2 == 0 && issue_c2 != 0)
		{
			//printf("C2 esim_cycle: %lld     %x\n", esim_cycle, phy_addr_2);
			mod_access( (*((*x86_cpu).core[2]).thread).inst_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
			issue_c2--;
			if(issue_c2 == 0)
			{
				issue_c2 = read_c2 = trace_driven_read_trace_I(2); 
				time_diff_2=0;
				point2 = bufferIC2; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}

	while(read_c3 != 0){
			if(time_diff_3 == 0 && issue_c3 != 0){
					memcpy(&tmp1,point3,8);
		            time_diff_3 = tmp1.time_difference;
		            phy_addr_3 = tmp1.address;
					point3 = point3 + 8;
					if(time_diff_3 != 0 )time_diff_3++; 
			//printf("C3 %d    %x \n",time_diff_3,phy_addr_3);
			}
			 if(time_diff_3 != 0)time_diff_3--;

		if(time_diff_3 == 0 && issue_c3 != 0)
		{
			//printf("C3 esim_cycle: %lld     %x\n", esim_cycle, phy_addr_3);
			mod_access( (*((*x86_cpu).core[3]).thread).inst_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
			issue_c3--;
			if(issue_c3 == 0)
			{
				issue_c3 = read_c3 = trace_driven_read_trace_I(3); 
				time_diff_3=0;
				point3 = bufferIC3; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}

	if( read_c0 <= 0 && read_c1 <= 0 && read_c2 <= 0 && read_c3 <= 0   ) return 0;
	else return 1;
}





int trace_driven_run_mem()
{
	static unsigned int phy_addr_0,phy_addr_1,phy_addr_2,phy_addr_3;
	static int kind0,kind1,kind2,kind3;                                          // 1 load  , 2 store
	static int time_diff_0 = 0,time_diff_1 = 0, time_diff_2 = 0, time_diff_3 = 0;
	static int state0 = 0,state1 = 0,state2 = 0, state3 = 0;
	
	static unsigned short int read_c0 = 0, read_c1 = 0,read_c2 = 0,read_c3 = 0;
	static unsigned short int issue_c0 = 0, issue_c1 = 0,issue_c2 = 0,issue_c3 = 0;
	static char * point0,* point1,* point2,* point3;

	basic_trace tmp1;

	if(  state0 == 0 )
	{
			state0 =1;
			issue_c0 =read_c0 = trace_driven_read_trace(0); 
			time_diff_0=0;
			point0 = bufferC0; 
	}
#if 0
	if(  state1 == 0 )
	{
			state1 =1;
		    issue_c1 =	read_c1 = trace_driven_read_trace(1); 
			time_diff_1=0;
			point1 = bufferC1; 
	}
	if(  state2 == 0 )
	{
			state2 =1;
			issue_c2 =read_c2 = trace_driven_read_trace(2); 
			time_diff_2=0;
			point2 = bufferC2; 
	}
	if(  state3 == 0 )
	{
			state3 =1;
			issue_c3 =read_c3 = trace_driven_read_trace(3); 
			time_diff_3=0;
			point3 = bufferC3; 
	}
#endif
//	printf("  %d  %d  %d  %d  |  %d  %d %d  %d  \n",issue_c0,issue_c1,issue_c2,issue_c3,read_c0,read_c1,read_c2,read_c3);
//	printf("1  %d  %d  \n",issue_c1,read_c1);
//	printf("2  %d  %d  \n",issue_c2,read_c2);
//	printf("3  %d  %d  \n",issue_c3,read_c3);
	while(read_c0 != 0){
			if(time_diff_0 == 0 && issue_c0 != 0){
					memcpy(&tmp1,point0,8);
		            kind0 = tmp1.type;
		            time_diff_0 = tmp1.time_difference;
		            phy_addr_0 = tmp1.address;
					point0 = point0 + 8;
					if(time_diff_0 != 0 )time_diff_0++; 
				//	time_diff_0++;
			printf("C0 %d  %d  %x \n",kind0,time_diff_0,phy_addr_0);//debug
			}

			if(time_diff_0 != 0)time_diff_0--;

		if(time_diff_0 == 0 && issue_c0 != 0)
		{
			printf("C0 esim_cycle: %lld   %d   %x\n", esim_cycle, kind0, phy_addr_0);//debug
			if(kind0 == 1)mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_load,phy_addr_0, NULL, NULL , NULL , NULL);
			else if(kind0 == 2) mod_access( (*((*x86_cpu).core[0]).thread).data_mod  , mod_access_store,phy_addr_0, NULL, NULL , NULL , NULL);
			issue_c0--;
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
			if(issue_c0 == 0)
			{
				issue_c0 = read_c0 = trace_driven_read_trace(0); 
				time_diff_0=0;
				point0 = bufferC0; 
			}
		}
		else
			break;
	}
#if 0
	while(read_c1 != 0){
			if(time_diff_1 == 0 && issue_c1 != 0){
					memcpy(&tmp1,point1,8);
		            kind1 = tmp1.type;
		            time_diff_1 = tmp1.time_difference;
		            phy_addr_1 = tmp1.address;
					point1 = point1 + 8;
					if(time_diff_1 != 0 )time_diff_1++; 
				//	time_diff_0++;
			printf("C1 %d  %d  %x \n",kind1,time_diff_1,phy_addr_1);//debug
			}
			 if(time_diff_1 != 0)time_diff_1--;

		if(time_diff_1 == 0 && issue_c1 != 0)
		{
			printf("C1 esim_cycle: %lld   %d   %x\n", esim_cycle, kind1, phy_addr_1);//debug
			if(kind1 == 1)mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_load,phy_addr_1, NULL, NULL , NULL , NULL);
			else if(kind1 == 2) mod_access( (*((*x86_cpu).core[1]).thread).data_mod  , mod_access_store,phy_addr_1, NULL, NULL , NULL , NULL);
			issue_c1--;
			if(issue_c1 == 0)
			{
				issue_c1 = read_c1 = trace_driven_read_trace(1); 
				time_diff_1=0;
				point1 = bufferC1; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}

	while(read_c2 != 0){
			if(time_diff_2 == 0 && issue_c2 != 0){
					memcpy(&tmp1,point2,8);
		            kind2 = tmp1.type;
		            time_diff_2 = tmp1.time_difference;
		            phy_addr_2 = tmp1.address;
					point2 = point2 + 8;
					if(time_diff_2 != 0 )time_diff_2++; 
			printf("C2 %d  %d  %x \n",kind2,time_diff_2,phy_addr_2);//debug
			}
			 if(time_diff_2 != 0)time_diff_2--;

		if(time_diff_2 == 0 && issue_c2 != 0)
		{
			printf("C2 esim_cycle: %lld   %d   %x\n", esim_cycle, kind2, phy_addr_2);//debug
			if(kind2 == 1)mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_load,phy_addr_2, NULL, NULL , NULL , NULL);
			else if(kind2 == 2) mod_access( (*((*x86_cpu).core[2]).thread).data_mod  , mod_access_store,phy_addr_2, NULL, NULL , NULL , NULL);
			issue_c2--;
			if(issue_c2 == 0)
			{
				issue_c2 = read_c2 = trace_driven_read_trace(2); 
				time_diff_2=0;
				point2 = bufferC2; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}

	while(read_c3 != 0){
			if(time_diff_3 == 0 && issue_c3 != 0){
					memcpy(&tmp1,point3,8);
		            kind3 = tmp1.type;
		            time_diff_3 = tmp1.time_difference;
		            phy_addr_3 = tmp1.address;
					point3 = point3 + 8;
					if(time_diff_3 != 0 )time_diff_3++; 
			printf("C3 %d  %d  %x \n",kind3,time_diff_3,phy_addr_3);//debug
			}
			 if(time_diff_3 != 0)time_diff_3--;

		if(time_diff_3 == 0 && issue_c3 != 0)
		{
			printf("C3 esim_cycle: %lld   %d   %x\n", esim_cycle, kind3, phy_addr_3);//debug
			if(kind3 == 1)mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_load,phy_addr_3, NULL, NULL , NULL , NULL);
			else if(kind3 == 2) mod_access( (*((*x86_cpu).core[3]).thread).data_mod  , mod_access_store,phy_addr_3, NULL, NULL , NULL , NULL);
			issue_c3--;
			if(issue_c3 == 0)
			{
				issue_c3 = read_c3 = trace_driven_read_trace(3); 
				time_diff_3=0;
				point3 = bufferC3; 
			}
			//printf("%d %d %d %d\n",issue_c0,issue_c1,issue_c2,issue_c3);
		}
		else break;
	}
#endif
	//if( read_c0 <= 0 && read_c1 <= 0 && read_c2 <= 0 && read_c3 <= 0   ) return 0;
	if(read_c0 <= 0) return 0;
	else return 1;
}





void trace_driven_done()
{
		printf("close mem trace FILE * \n");
		fclose( mem_access_trace_f_C0 );
#if 0
		fclose( mem_access_trace_f_C1 );
		fclose( mem_access_trace_f_C2 );
		fclose( mem_access_trace_f_C3 );
#endif
		//hchung 2014_11_26 for Qemu memory trace
		/*fclose( inst_access_trace_f_C0 );
		fclose( inst_access_trace_f_C1 );
		fclose( inst_access_trace_f_C2 );
		fclose( inst_access_trace_f_C3 );*/
}

void shm_mem_trace()
{
	int shmid;
    	key_t key;
	int *shm;
        int log_size,log_num,log_instr;
        int trace_count;
        int size;
        unsigned int tmp_data;
        unsigned int record[4];
        int flag=1;
        int i;

    	key = 800;
	total_instr = 0;
	log_size = 0;
	log_num = 0;
	log_instr = 0;//1:tb_exit 2:Ld 3:St
	trace_count = 0;
	record[0]=record[1]=record[2]=record[3]=0;
	printf("enter shm function\n");
    	/*
     	* Locate the segment.
     	*/
	while(1){
    		if ((shmid = shmget(key, SHMSZ, IPC_CREAT|0666)) < 0) {
        		perror("shmget");
		        exit(1);
    		}
		else
			break;
	}
	perror("shmget");
    	/*
     	* Now we attach the segment to our data space.
     	*/
    	if ((shm = (int *)shmat(shmid, NULL, 0)) == (int *) -1) {
        	perror("shmat");
        	exit(1);
    	}

	//D-chche initial?
	config_init(CPU_NUM);

    	/*
     	* Now read what the server put in the memory.
     	*/
	printf("key:%d  id:%d  ptr:%x\n",key,shmid,shm);
	while(1){
                if((shm[shm_head]+1)%shm_max != shm[shm_tail]){
                        //cout<<shm[shm_head]<<" "<<shm[shm[shm_head]]<<"  ";
                        //fprintf(model_log_file,"shm[shm_head] = %i, <shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
                        if(log_num==log_size){//new instr starting
                                //----------one instr end-------------//
                                //printf("CPU:%d tb:0x%08x addr:0x%08x size:%d\n",record[0],record[1],record[2],record[3]);
                                switch(log_instr){
                                        case 1: //tb_exit
                                                CPU[record[0]]->translated_block(record[2]);
                                                total_instr+=record[2];
                                                //sim_time+=record[3];
                                                break;
                                        case 2: //Ld
                                                read_mem(record[2],record[3],record[0]);
                                                break;
                                        case 3: //st
                                                write_mem(record[2],record[3],record[0]);
                                                break;
                                        case 4: CPU[record[0]]->sim_cycle(record[1]);
                                                break;
                                        default:
                                                //printf("This instruction:%d is not recognized.a\n",log_instr);
                                                break;
                                }//switch
			 //---------new instr start------------//
                                log_instr = shm[shm[shm_head]];
                                switch(log_instr){
                                        case 1: log_size = 3;
                                                log_num  = 0;
                                                break;
                                        case 2: log_size = 4;
                                                log_num  = 0;
                                                break;
                                        case 3: log_size = 4;
                                                log_num  = 0;
                                                break;
                                        case 4: log_size = 2;//tick instrction cycle
                                                log_num = 0;
                                                break;
                                        case -1:
                                                config_print(CPU_NUM);
                                                printf("total instruction count:%d\n",total_instr);
                                                shmctl(shmid,IPC_RMID,0);
                                                perror("shmctl");
                                                return;//delete the return value  hchung 2014_12_10
                                                break;
                                        default:
                                                //printf("This instruction:%d is not recognized.\n",log_instr);
                                                break;

                                }
                        }else{//instr trace
                                record[log_num] = shm[shm[shm_head]];
                                //printf("record[log_num] = %d, shm[shm[shm_head]] = %d\n",record[log_num],shm[shm[shm_head]]);
                                //fprintf(model_log_file,"record[log_num] = %d, shm[shm[shm_head]] = %d\n",record[log_num],shm[shm[shm_head]]);
                                log_num++;
                        }
                        shm[shm_head]++;
                        shm[shm_head]%=shm_max;
                }else{
                        //cout<<"else "<<shm[shm_head]<<" "<<shm[shm[shm_head]]<<endl;
                        //fprintf(model_log_file,"in  else shm[shm_head] = %i, shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
                        //printf("%i , %i\n",shm[shm_head],shm[shm[shm_head]]);
                }
        }//while

    	/*
     	* Finally, change the first character of the 
     	* segment to '*', indicating we have read 
     	* the segment.
     	*/
    	*shm = '*';

    	exit(0);
}

void read_mem(unsigned int addr,int size,int cpuid)
{
        mcore_module * mcore_obj;

        CPU[cpuid]->read(addr,size);
        mcore_obj = CPU[cpuid];
        while(1){
                if(mcore_obj->mcore_select_out==NULL){
                        break;
                }else{
                        mcore_obj->mcore_select_out->module_recv_rq(&mcore_obj->mcore_rq);
                        mcore_obj = mcore_obj->mcore_select_out;
                }
        }//while
}
void write_mem(unsigned int addr,int size,int cpuid)
{
        mcore_module * mcore_obj;

        CPU[cpuid]->write(addr,size);
        mcore_obj = CPU[cpuid];
        while(1){
                if(mcore_obj->mcore_select_out==NULL){
                        break;
                }else{
                        mcore_obj->mcore_select_out->module_recv_rq(&mcore_obj->mcore_rq);
                        mcore_obj = mcore_obj->mcore_select_out;
                }
        }//while
}
int config_init(int proc_cnt)
{
	int i;
	Mem = new Memory_module();
	for(i=0;i<proc_cnt;i++) {
		CPU[i] = new CPU_module(i);
		CPU[i]->connect(Mem);
	}
	return 0;
}

void config_print(int proc_cnt)
{
	int i;
	for (i = 0; i < proc_cnt; i++) {
		CPU[i]->module_stats();
	}
	Mem->module_stats();
}

