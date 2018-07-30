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

#include <iostream>
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
#define shm_line 131070//174762
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

typedef struct trace_format{
	int cpu_id;
	int RW;
	unsigned long long addr;
	int size;
	int TS;//yhchiang 20160328
}trace_format;
#if 0
//share memory var
int shmid;
key_t key;
int *shm;
int log_size,log_num,log_instr;
unsigned int trace_counter;
int size;
unsigned int tmp_data;
unsigned int record[4];
int total_instr;
int shm_flag;
int init_flag;
#endif
int total_instr;
int shmid, shmid2, shmid3, shmid4,shmid5;
key_t key, key2, key3, key4, key5;
struct trace_format *shm_ptr;//yhchiang   //800
trace_format in_data;
int *shm_head_index;
int *shm_tail_index;
unsigned long long *shm_head_ptr;//yhchiang  799
unsigned long long *shm_tail_ptr;//yhchiang  798
int *shm_empty_flag;//yhchiang 700
int *shm;
int *shm_end;
static int last_cycle_cnt;
static int trace_cnt = 0;
int first_getin=1, already_in=0;
extern unsigned long start_timestamp;
//int log_size,log_num[8]={0},log_instr; //yh comment hchung
int log_size,log_num,log_instr; //yh
unsigned int record[4]; //yh
//int trace_count;
int size;
unsigned int tmp_data;
//struct trace_format record[8]; //yh comment hchung
int exceed[8]={0};
int count[8]={0};
//unsigned int record[4];
//int flag;
int i;
int max_bound;
long long cnt;
double cnt_i;
//int cpu_id;
unsigned int LS_num;
CPU_module *CPU[256];
Memory_module *Mem;
/*int *trans_cycle;
void trans_cycle_init()
{
	key_t shm_key = 263;
	int trans_cycle_id;
	trans_cycle_id = shmget(shm_key, sizeof(int), 0666);
	if((trans_cycle = (int *)shmat(trans_cycle_id, NULL, 0)) == -1){
	      printf("shmatl error:  %x \n", trans_cycle);
	      perror("shmat");
	      shmctl(trans_cycle,IPC_RMID,0);
	      exit(-1);
	}
	//trans_cycle[0] = 0;

	
}*/
int check_cycle_matching(int mem_timestamp){ // 1: match;  0: not match
	/*if(mem_timestamp<=esim_cycle)return 1;
	else return 0;*/
	//printf("TS = %d , esim_cycle = %u, last = %u\n", mem_timestamp, esim_cycle, last_cycle_cnt);
	if(mem_timestamp<= (esim_cycle-last_cycle_cnt)){
		last_cycle_cnt = esim_cycle;
		return 1;
	}
	else {
		
		return 0;	
	}
}
int* shm_L1readhit;
int* shm_L1writehit;
int* shm_L2readhit;
int* shm_L2writehit;
int L1readhit_id;
int L1writehit_id;
int L2readhit_id;
int L2writehit_id;
void shm_mem_init()
{
	//flag = 1;
	max_bound = 0;
	cnt = 0;
	cnt_i = 0;
	total_instr = 0;
	//log_size = 0;
	//log_num  = 0;
	log_instr = 0;//1-> tb_exit , 2->Ld, 3->St
	//trace_count = 0;
	//cpu_id = 0;
	LS_num = 0;
#if 1
	key = 800;//ftok(SHMPATH,0);
	//key2 = 700; //for empty flag
	//key3 = 799; //for head 
	//key4 = 798; //for tail
	//key5 = 347; //for shm end
	key2 = 700;//head
	key3 = 799;//tail
#endif
#if 0  //yh comment hchung 0323
#if 1
	key = 850;//ftok(SHMPATH,0);
	key2 = 830; //for empty flag
	key3 = 829; //for head 
	key4 = 828; //for tail
	key5 = 827; //for shm end
#endif
#endif
	/*
	 * Locate the segment.
	 */
	while(1){
		/*if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}*/
		if ((shmid = shmget(key, 100*sizeof(trace_format), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
		/*else if((shmid2 = shmget(key2, 8*sizeof(int), 0666)) < 0){
			//perror("shmget");
		}
		else if((shmid3 = shmget(key3, 8*sizeof(unsigned long long), 0666)) < 0){}
		else if((shmid4 = shmget(key4, 8*sizeof(unsigned long long), 0666)) < 0){}
		else if((shmid5 = shmget(key5, 8*sizeof(int), 0666)) < 0){}*/
		else if ((shmid2 = shmget(700, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
		else if ((shmid3 = shmget(799, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
#if 1
		else if ((L1readhit_id = shmget(721, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
		else if ((L1writehit_id = shmget(722, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
		else if ((L2readhit_id = shmget(723, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
		else if ((L2writehit_id = shmget(724, sizeof(int), 0666)) < 0) {
			//perror("shmget");
			//exit(1);
		}
#endif
		else break;

	}
	printf("shm init successfully\n");
	perror("shmget");

	shm_ptr = (trace_format *)shmat(shmid, NULL, 0);
		
		
	
	if ((shm_head_index = (int *)shmat(shmid2, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}
	if ((shm_tail_index = (int *)shmat(shmid3, NULL, 0)) == (int *) -1) {
		perror("shmat");
		exit(1);
	}
#if 1

	if ((shm_L1readhit = (int *)shmat(L1readhit_id, NULL, 0)) == (int *) -1) {
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
	//shm_ptr = (trace_format *)shmat(shmid, NULL, 0);
	
	/*shm_empty_flag = (int *)shmat(shmid2, NULL, 0);
	
	shm_head_ptr = (unsigned long long *)shmat(shmid3, NULL, 0);
	
	shm_tail_ptr = (unsigned long long *)shmat(shmid4, NULL, 0);
	
	shm_end = (int *)shmat(shmid5, NULL, 0);*/


	//D-cache initail
	config_init(CPU_NUM);
	// Now read what the server put in the memory.
	printf("key:%d  id:%d  ptr:%x\n",key,shmid,shm);
}
#if 0
//one loop, one trace
int shm_mem_trace()
{

	//int put_trace = 0;//the flag about whether the trace put in mod_access
	
	//record[0]=record[1]=record[2]=record[3]=0;

	while(1){
		//for(cpu_id=0;cpu_id < CPU_NUM;cpu_id++){
			if(shm_end[0]&shm_end[1]&shm_end[2]&shm_end[3]==1){//check for tasks of all cores complete
				config_print(CPU_NUM);
				printf("total instruction count:%d\n",total_instr);	
				printf("cnt = %d, cnt_i = %f, ratio = %f\n",cnt,cnt_i,cnt_i/cnt);
				shmctl(shmid,IPC_RMID,0);
				perror("shmctl");
				return 0;
				printf("The total instruction count of load and store: %u",LS_num);
				//break;
			}
			if(shm_empty_flag[cpu_id]==0 && exceed[cpu_id]!=1 ){//check the buffer until which is full, then go next
				if(shm_tail_ptr[cpu_id]<1000){
					exceed[cpu_id]=0;
					shm_empty_flag[cpu_id]=1;	
					if(shm_tail_ptr[cpu_id]!=1)count[cpu_id]++;
				}	
				else{ 
					exceed[cpu_id]=1;
					if(shm_tail_ptr[cpu_id]>max_bound){
						max_bound = shm_tail_ptr[cpu_id];
						printf("max_bound = %d\n",max_bound);
					}
					cnt++;
					if(shm_tail_ptr[cpu_id]>10000)cnt_i++;
					//printf("shm_tail_ptr[%d]=%d\n",cpu_id,shm_tail_ptr[cpu_id]);
				}

			}
			/*if(shm_head_ptr[cpu_id]==shm_tail_ptr[cpu_id]){
				//cpu_id++;
				shm_empty_flag[cpu_id]=1;
			}*/
			//if(shm_empty_flag[cpu_id]==0){
				if((shm_head_ptr[cpu_id]+1==shm_tail_ptr[cpu_id])/*||(shm_head_ptr[cpu_id]==0 && shm_tail_ptr[cpu_id]==1)*/){
					//printf("shm_tail_ptr[%d]= %d\n",cpu_id,shm_tail_ptr[cpu_id]);
					if(exceed[cpu_id]==1){
						
						shm_head_ptr[cpu_id]=0;
						shm_tail_ptr[cpu_id]=1;
						shm_empty_flag[cpu_id]=1;
						exceed[cpu_id]=0;
						//printf("count[%d]=%d\n",cpu_id, count[cpu_id]);
						count[cpu_id]=0;
					}
					//printf("empty flag 1\n");
					cpu_id++;
					if(cpu_id > 3)
						cpu_id = 0;
					continue;
				}
				record[cpu_id] = shm_ptr[shm_line*cpu_id+shm_head_ptr[cpu_id]];
				//printf("%d\n",cpu_id);
				switch(record[cpu_id].RW/*log_instr*/){
					case 1:	//tb_exit
						CPU[cpu_id]->translated_block(record[cpu_id].size);//tb_ic
						total_instr+=record[cpu_id].size;
						//put_trace = 0;
						break;
					case 2:	//Ld
						//read(record[cpu_id].addr,record[cpu_id].size,cpu_id);
						mod_access( (*((*x86_cpu).core[cpu_id]).thread).data_mod, mod_access_load, record[cpu_id].addr, NULL, NULL , NULL , NULL);
						//put_trace = 1;
						LS_num++;
						break;
					case 3:	//st
						//write(record[cpu_id].addr,record[cpu_id].size,cpu_id);
						mod_access( (*((*x86_cpu).core[cpu_id]).thread).data_mod, mod_access_store, record[cpu_id].addr, NULL, NULL , NULL , NULL);
						//put_trace = 1;
						LS_num++;
						break;
					case 4:	
						CPU[cpu_id]->sim_cycle(record[cpu_id].addr);//tb_pc
						//put_trace = 0;
						break;
					case -1:
						config_print(CPU_NUM);
						printf("total instruction count:%d\n",total_instr);
						shmctl(shmid,IPC_RMID,0);
						perror("shmctl");
						return 0;
						break;
					default:
						//printf("This instruction:%d is not recognized.a\n",log_instr);
						break;
				}//switch
				//shm_num[cpu_id]--;
				shm_head_ptr[cpu_id]++;
				//shm_head_ptr[cpu_id] = shm_head_ptr[cpu_id] % 1000 ;
				/*if(put_trace == 1)
					return 1;
				else 
					return 0;*/
			//}
		//}//for
		cpu_id++;
		if(cpu_id > 3)
			cpu_id = 0;
		return 1;
		//cpu_id++;
		//cpu_id%=4;
		//if(record[cpu_id].RW==-1)break;
	}//while


	exit(0);
}
#endif
//one loop, five trace
#if 1  //yhchiang
int shm_mem_trace()
{

	//int put_trace = 0;//the flag about whether the trace put in mod_access
	//int cpu_id;	
	//record[0]=record[1]=record[2]=record[3]=0;
	int trace_skipped = 0;

	//cws
	FILE *trace_file=NULL;
	char mywrite = 1;
	char myread = 0;

	
	while(1){
		if((shm_head_index[0]+1)%100!=shm_tail_index[0])//not empty
		{
			//trace_cnt++;
			//printf("trace_cnt = %d\n",trace_cnt);
			//fflush(stdout);
			if(first_getin){
				start_timestamp = esim_cycle;
				printf("start_timestamp %u\n",start_timestamp);
				already_in = 1;
				first_getin = 0;
			}
				//record[cpu_id] = shm_ptr[shm_line*cpu_id+shm_head_ptr[cpu_id]];
				in_data = shm_ptr[shm_head_index[0]];
				printf("%d %d %u\n",in_data.cpu_id, in_data.RW, in_data.addr);
				//printf("%d\n",cpu_id);
				switch(in_data.RW/*log_instr*/){
					case 1:	//tb_exit
						CPU[in_data.cpu_id]->translated_block(in_data.size);//tb_ic
						total_instr+=in_data.size;
						//put_trace = 0;
						break;
					case 2:	//Ld
						//read(record[cpu_id].addr,record[cpu_id].size,cpu_id);
					if(mod_can_access((*((*x86_cpu).core[in_data.cpu_id]).thread).data_mod, in_data.addr)&&check_cycle_matching(in_data.TS)){ //porshe_20160324
						printf("LD %x\n",in_data.addr);
						mod_access( (*((*x86_cpu).core[in_data.cpu_id]).thread).data_mod, mod_access_load, in_data.addr, NULL, NULL , NULL , NULL); 
						LS_num++;
						//trace_cnt++;
						//printf("trace_cnt = %d\n",trace_cnt);

						trace_file = fopen(cws_str,"a");
						if(trace_file!=NULL){
						fwrite(&in_data.TS,sizeof(unsigned long long),1,trace_file);
						fwrite(&myread,sizeof(char),1,trace_file);
						fwrite(&in_data.size,sizeof(char),1,trace_file);
						fwrite(&in_data.addr,sizeof(unsigned int),1,trace_file);
						fclose(trace_file);
						}
					}
					else //porshe_20160324
					{	trace_skipped = 1;}
						//put_trace = 1;
						break;
					case 3:	//st
						//write(record[cpu_id].addr,record[cpu_id].size,cpu_id);
					if(mod_can_access((*((*x86_cpu).core[in_data.cpu_id]).thread).data_mod, in_data.addr)&&check_cycle_matching(in_data.TS)){ //porshe_20160324
						printf("ST %x\n",in_data.addr);
						mod_access( (*((*x86_cpu).core[in_data.cpu_id]).thread).data_mod, mod_access_store, in_data.addr, NULL, NULL , NULL , NULL);
						LS_num++;
						//trace_cnt++;
						//printf("trace_cnt = %d\n",trace_cnt);
						
					
						trace_file = fopen(cws_str,"a");
						if(trace_file!=NULL){
						fwrite(&in_data.TS,sizeof(unsigned long long),1,trace_file);
						fwrite(&mywrite,sizeof(char),1,trace_file);
						fwrite(&in_data.size,sizeof(char),1,trace_file);
						fwrite(&in_data.addr,sizeof(unsigned int),1,trace_file);
						fclose(trace_file);
						}
					}
					else //porshe_20160324
					{	trace_skipped = 1;}
						//put_trace = 1;
						break;
					case 4:	
						CPU[in_data.cpu_id]->sim_cycle(in_data.addr);//tb_pc
						//put_trace = 0;
						break;
					case -1:
						config_print(CPU_NUM);
						printf("total instruction count:%d\n",total_instr);
						fprintf(stderr,"shm_end !!\n");
						shmctl(shmid,IPC_RMID,0);
						perror("shmctl");
						return 0;
						break;
					default:
						//printf("This instruction:%d is not recognized.a\n",log_instr);
						break;
				}//switch
				if(!trace_skipped)
				{
					shm_head_index[0]++;
					shm_head_index[0]%=100;
				}
				else{ break;}
		}
		/*else if(shm_head_index[0]==shm_tail_index[0]){
			break;
		}*/
		else 
		{
			break;
		}
	}//while

	return 1;
	exit(0);
}
#endif
#if 0 //yhchiang new shm_mem_trace
int shm_mem_trace(){
	int cpu_id;
	int trace_skipped = 0;  //porshe_20160324
	while(1){
		if((shm[shm_head]+1)%shm_max != shm[shm_tail]){
			if(log_num==log_size){//new instr starting
				//----------one instr end-------------//
				//printf("CPU:%d tb:0x%08x addr:0x%08x size:%d\n",record[0],record[1],record[2],record[3]);
				switch(log_instr){
					case 1:	//tb_exit
						CPU[record[0]]->translated_block(record[2]);
						total_instr+=record[2];
						//sim_time+=record[3];
						break;
					case 2:	//Ld
						//read(record[2],record[3],record[0]);
						if(mod_can_access((*((*x86_cpu).core[record[0]]).thread).data_mod, record[2])) //porshe_20160324
							mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_load, record[2], NULL, NULL , NULL , NULL); 
						else //porshe_20160324
							trace_skipped = 1;
						break;
					case 3:	//st
						//write(record[2],record[3],record[0]);
						if(mod_can_access((*((*x86_cpu).core[record[0]]).thread).data_mod, record[2])) //porshe_20160324
							mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_store, record[2], NULL, NULL , NULL , NULL);
						else //porshe_20160324
							trace_skipped = 1;
						break;
					case 4:	CPU[record[0]]->sim_cycle(record[1]);
						break;
					default:
						//printf("This instruction:%d is not recognized.a\n",log_instr);
						break;
				}//switch

				//---------new instr start------------//
				log_instr = shm[shm[shm_head]];
				switch(log_instr){
					case 1:	log_size = 3;
						log_num  = 0;
						break;
					case 2: log_size = 4;
						log_num  = 0;
						break;
					case 3:	log_size = 4;
						log_num  = 0;
						break;
					case 4:	log_size = 2;//tick instrction cycle
						log_num = 0;
						break;
					case -1:
						config_print(CPU_NUM);
						printf("total instruction count:%d\n",total_instr);
						shmctl(shmid,IPC_RMID,0);
						perror("shmctl");
						return 0;
						break;
					default:
						//printf("This instruction:%d is not recognized.\n",log_instr);
						break;

				}
			}else{//instr trace
				record[log_num] = shm[shm[shm_head]];
				log_num++;
			}
			if(!trace_skipped)
			{	
				shm[shm_head]++;
				shm[shm_head]%=shm_max;
			}
		}else{
			//cout<<shm[shm_head]<<" "<<shm[shm[shm_head]]<<endl;
		}
	}//while

	*shm = '*';

		return 1;
	exit(0);
}
#endif
#if 0    //yh comment hchung 0323
#if 1
int shm_mem_trace()
{

	//int put_trace = 0;//the flag about whether the trace put in mod_access
	int cpu_id;	
	//record[0]=record[1]=record[2]=record[3]=0;

	while(1){
		for(cpu_id=0;cpu_id < CPU_NUM;cpu_id++){
			if(shm_end[0]&shm_end[1]&shm_end[2]&shm_end[3]==1){//check for tasks of all cores complete
				config_print(CPU_NUM);
				printf("total instruction count:%d\n",total_instr);	
				printf("cnt = %d, cnt_i = %f, ratio = %f\n",cnt,cnt_i,cnt_i/cnt);
				shmctl(shmid,IPC_RMID,0);
				perror("shmctl");
				printf("The total instruction count of load and store: %u",LS_num);
				return 0;
				//break;
			}
			if(shm_empty_flag[cpu_id]==0 && exceed[cpu_id]!=1 ){//check the buffer until which is full, then go next
				if(shm_tail_ptr[cpu_id]<1000){
					exceed[cpu_id]=0;
					shm_empty_flag[cpu_id]=1;	
					if(shm_tail_ptr[cpu_id]!=1)count[cpu_id]++;
				}	
				else{ 
					exceed[cpu_id]=1;
					if(shm_tail_ptr[cpu_id]>max_bound){
						max_bound = shm_tail_ptr[cpu_id];
						printf("max_bound = %d\n",max_bound);
					}
					cnt++;
					if(shm_tail_ptr[cpu_id]>10000)cnt_i++;
					//printf("shm_tail_ptr[%d]=%d\n",cpu_id,shm_tail_ptr[cpu_id]);
				}

			}
			/*if(shm_head_ptr[cpu_id]==shm_tail_ptr[cpu_id]){
				//cpu_id++;
				shm_empty_flag[cpu_id]=1;
			}*/
			//if(shm_empty_flag[cpu_id]==0){
				if((shm_head_ptr[cpu_id]+1==shm_tail_ptr[cpu_id])/*||(shm_head_ptr[cpu_id]==0 && shm_tail_ptr[cpu_id]==1)*/){
					//printf("shm_tail_ptr[%d]= %d\n",cpu_id,shm_tail_ptr[cpu_id]);
					if(exceed[cpu_id]==1){
						
						shm_head_ptr[cpu_id]=0;
						shm_tail_ptr[cpu_id]=1;
						shm_empty_flag[cpu_id]=1;
						exceed[cpu_id]=0;
						//printf("count[%d]=%d\n",cpu_id, count[cpu_id]);
						count[cpu_id]=0;
					}
					//printf("empty flag 1\n");
					continue;
				}
				record[cpu_id] = shm_ptr[shm_line*cpu_id+shm_head_ptr[cpu_id]];
				//printf("%d\n",cpu_id);
				switch(record[cpu_id].RW/*log_instr*/){
					case 1:	//tb_exit
						CPU[cpu_id]->translated_block(record[cpu_id].size);//tb_ic
						total_instr+=record[cpu_id].size;
						//put_trace = 0;
						break;
					case 2:	//Ld
						//read(record[cpu_id].addr,record[cpu_id].size,cpu_id);
						mod_access( (*((*x86_cpu).core[cpu_id]).thread).data_mod, mod_access_load, record[cpu_id].addr, NULL, NULL , NULL , NULL);
						LS_num++;
						//put_trace = 1;
						break;
					case 3:	//st
						//write(record[cpu_id].addr,record[cpu_id].size,cpu_id);
						mod_access( (*((*x86_cpu).core[cpu_id]).thread).data_mod, mod_access_store, record[cpu_id].addr, NULL, NULL , NULL , NULL);
						LS_num++;
						//put_trace = 1;
						break;
					case 4:	
						CPU[cpu_id]->sim_cycle(record[cpu_id].addr);//tb_pc
						//put_trace = 0;
						break;
					case -1:
						config_print(CPU_NUM);
						printf("total instruction count:%d\n",total_instr);
						shmctl(shmid,IPC_RMID,0);
						perror("shmctl");
						return 0;
						break;
					default:
						//printf("This instruction:%d is not recognized.a\n",log_instr);
						break;
				}//switch
				//shm_num[cpu_id]--;
				shm_head_ptr[cpu_id]++;
				//shm_head_ptr[cpu_id] = shm_head_ptr[cpu_id] % 1000 ;
				/*if(put_trace == 1)
					return 1;
				else 
					return 0;*/
			//}
		}//for
		return 1;
		//cpu_id++;
		//cpu_id%=4;
		//if(record[cpu_id].RW==-1)break;
	}//while


	exit(0);
}
#endif
#endif
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

void trace_driven_init()
{
			
	mem_access_trace_f_C0  = file_open_for_read(mem_trace_driven_file_name_C0);
#if 1 
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
#if 1
	if(mem_access_trace_f_C1 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C1 fail !!!!!!!!!!!!\n");
		//fatal("trace driven open file fail!");	
		printf("trace driven open file fail!");
	}
	if(mem_access_trace_f_C2 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C2 fail !!!!!!!!!!!!\n");
		//fatal("trace driven open file fail!");
		printf("trace driven open file fail!");
	}
	if(mem_access_trace_f_C3 == NULL )// fatal("trace driven open file fail!");
	{
		printf("trace driven open file mem C3 fail !!!!!!!!!!!!\n");
		//fatal("trace driven open file fail!");
		printf("trace driven open file fail!");
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
#if 1
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
#if 1
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
	if( read_c0 <= 0 && read_c1 <= 0 && read_c2 <= 0 && read_c3 <= 0   ) return 0;
	//if(read_c0 <= 0) return 0;
	else return 1;
}





void trace_driven_done()
{
		printf("close mem trace FILE * \n");
		fclose( mem_access_trace_f_C0 );
#if 1
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

#if 0
void shm_mem_init()
{
    
    	//key = 800;
    	key = 850;
	total_instr = 0;
	log_size = 0;
	log_num = 0;
	log_instr = 0;//1:tb_exit 2:Ld 3:St
	trace_counter = 0;
	record[0]=record[1]=record[2]=record[3]=0;
	init_flag = 1;
	printf("enter shm function\n");
    	/*
     	* Locate the segment.
     	*/
	while(1){
        	if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {//wait until the shared memory space be created
        		//perror("shmget1");
			//exit(1);
    		}
		else{
			printf("shmget success");
			break;
		}
	}
	//perror("shmget2");
    	/*
     	* Now we attach the segment to our data space.
     	*/
    	if ((shm = (int *)shmat(shmid, NULL, 0)) == (int *) -1) {
        	perror("shmat");
        	exit(1);
    	}
	shm_flag = 1;
	//D-chche initial?
	config_init(CPU_NUM);//2015/1/16 hchung: delete part of removeable code
    	/*
     	* Now read what the server put in the memory.
     	*/
	printf("key:%d  id:%d  ptr:%x\n",key,shmid,shm);
}
#endif
#if 0
int shm_mem_trace()//one loop read all trace into mod_access
{
        //int flag=1;
        int i;


	while(1){
                if((shm[shm_head]+1)%shm_max != shm[shm_tail]){
                        #if 0
			cout<<"shm[shm_head]= "<<shm[shm_head]<<endl;
			cout<<"shm[shm[shm_head]]= "<<shm[shm[shm_head]]<<endl;
                        cout<<"shm[shm_tail]= "<<shm[shm_head]<<endl;
			#endif
                        //fprintf(model_log_file,"shm[shm_head] = %i, <shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
                        if(log_num==log_size){//new instr starting
				//trace_counter++;
                                //----------one instr end-------------//
                                //printf("CPU:%d tb:0x%08x addr:0x%08x size:%d\n",record[0],record[1],record[2],record[3]);
                                switch(log_instr){
                                        case 1: //tb_exit
                                                CPU[record[0]]->translated_block(record[2]);
                                                total_instr+=record[2];
                                                //sim_time+=record[3];
                                                break;
                                        case 2: //Ld
						//20150304_hchung_Put the information into m2s memory module
                                                //read_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_load, record[2], NULL, NULL , NULL , NULL);
                                                break;
                                        case 3: //st
						//20150304_hchung_Put the information into m2s memory module
                                                //write_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_store, record[2], NULL, NULL , NULL , NULL);
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
                                                //config_print(CPU_NUM);//20150304_hchung_Not neccessary
                                                //printf("total instruction count:%d\n",total_instr);
						if(shm_flag == 1)// hchung 2015_4_22 add a flag for solving duplicated shmctl
                                                {
							shmctl(shmid,IPC_RMID,0);
                                                	perror("shmctl");
							printf("shm_mem_trace over!");
							shm_flag = 0;
							trace_counter--;
                                                	return 1;//  hchung 2014_12_10
						}
						else
						{
							if(trace_counter > 0)
							{
								trace_counter--;
								return 1;
							}
							else
								return 0;
						}
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

    	//exit(0);
}
#endif
#if 0
int shm_mem_trace()//one loop read one trace
{
        //int flag=1;
        int i;
	int put_trace = 0;//the flag about whether the trace put in mod_access
	int new_instr = 1;

	log_size = 0;
	log_num = 0;
#if 0
	while(1){
                if((shm[shm_head]+1)%shm_max != shm[shm_tail]){
                        //if(log_num==log_size){//new instr starting
			if(new_instr == 1){
                        	shm[shm_head]++;
	                        shm[shm_head]%=shm_max;
                                log_instr = shm[shm[shm_head]];
				new_instr = 0;
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
						if(shm_flag == 1)// hchung 2015_4_22 add a flag for solving duplicated shmctl
                                                {
							shmctl(shmid,IPC_RMID,0);
                                                	perror("shmctl");
							printf("shm_mem_trace over!");
							shm_flag = 0;
                                                	return 0;
						}
						else//won't do shmctl
							return 0;
                                                break;
                                        default:
                                                break;

                                }
			}
			else if(log_num == log_size)
			{
                                switch(log_instr){
                                        case 1: //tb_exit
                                                CPU[record[0]]->translated_block(record[2]);
                                                total_instr+=record[2];
                                                //sim_time+=record[3];
						put_trace = 0;
                                                break;
                                        case 2: //Ld
						//20150304_hchung_Put the information into m2s memory module
                                                //read_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_load, record[2], NULL, NULL , NULL , NULL);
						put_trace = 1;
                                                break;
                                        case 3: //st
						//20150304_hchung_Put the information into m2s memory module
                                                //write_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_store, record[2], NULL, NULL , NULL , NULL);
						put_trace = 1;
                                                break;
                                        case 4: CPU[record[0]]->sim_cycle(record[1]);
						put_trace = 0;
                                                break;
                                        default:
                                                //printf("This instruction:%d is not recognized.a\n",log_instr);
                                                break;
                                }//switch
				if(put_trace == 1)
					return 1;
				else
					return 0;
                        }
			else
			{
                        	record[log_num] = shm[shm[shm_head]];
                        	log_num++;
				if(log_num == log_size)
					continue;//avoid the below shm[shm_head]++
			}

                        shm[shm_head]++;
                        shm[shm_head] %= shm_max;
                
                }
		else{
                        //cout<<"else "<<shm[shm_head]<<" "<<shm[shm[shm_head]]<<endl;
                        //fprintf(model_log_file,"in  else shm[shm_head] = %i, shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
                        //printf("%i , %i\n",shm[shm_head],shm[shm[shm_head]]);
                }
        }//while
#endif
#if 1
	while(1){
                if((shm[shm_head]+1)%shm_max != shm[shm_tail]){
                        #if 0
			cout<<"shm[shm_head]= "<<shm[shm_head]<<endl;
			cout<<"shm[shm[shm_head]]= "<<shm[shm[shm_head]]<<endl;
                        cout<<"shm[shm_tail]= "<<shm[shm_head]<<endl;
			#endif
                        //fprintf(model_log_file,"shm[shm_head] = %i, <shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
			if(init_flag == 1)
			{
      	                	shm[shm_head]++;
	                        shm[shm_head]%=shm_max;
			}
                        if(log_num==log_size){//new instr starting
				//trace_counter++;
                                //----------one instr end-------------//
                                //printf("CPU:%d tb:0x%08x addr:0x%08x size:%d\n",record[0],record[1],record[2],record[3]);
                                switch(log_instr){
                                        case 1: //tb_exit
                                                CPU[record[0]]->translated_block(record[2]);
                                                total_instr+=record[2];
                                                //sim_time+=record[3];
						put_trace = 0;
                                                break;
                                        case 2: //Ld
						//20150304_hchung_Put the information into m2s memory module
                                                //read_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_load, record[2], NULL, NULL , NULL , NULL);
						put_trace = 1;
                                                break;
                                        case 3: //st
						//20150304_hchung_Put the information into m2s memory module
                                                //write_mem(record[2],record[3],record[0]);
						mod_access( (*((*x86_cpu).core[record[0]]).thread).data_mod, mod_access_store, record[2], NULL, NULL , NULL , NULL);
						put_trace = 1;
                                                break;
                                        case 4: CPU[record[0]]->sim_cycle(record[1]);
						put_trace = 0;
                                                break;
                                        default:
                                                printf("This instruction:%d is not recognized.a\n",log_instr);	
                                                break;
                                }//switch
				if(log_size != 0){
					if(put_trace == 1)
						return 1;
					else
						return 0;
				}
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
                                                //config_print(CPU_NUM);//20150304_hchung_Not neccessary
                                                //printf("total instruction count:%d\n",total_instr);
						if(shm_flag == 1)// hchung 2015_4_22 add a flag for solving duplicated shmctl
                                                {
							shmctl(shmid,IPC_RMID,0);
                                                	perror("shmctl");
							printf("shm_mem_trace over!");
							shm_flag = 0;
                                                	return 1;//  hchung 2014_12_10
						}
						else
						{
							return 0;
						}
                                                break;
                                        default:
                                                printf("This instruction:%d is not recognized.\n",log_instr);
                                                break;

                                }//switch

                        }
			else{//instr trace
                                record[log_num] = shm[shm[shm_head]];
                                //printf("record[log_num] = %d, shm[shm[shm_head]] = %d\n",record[log_num],shm[shm[shm_head]]);
                                //fprintf(model_log_file,"record[log_num] = %d, shm[shm[shm_head]] = %d\n",record[log_num],shm[shm[shm_head]]);
                                log_num++;
                        }
			if(init_flag == 0){
                        	shm[shm_head]++;
	                        shm[shm_head]%=shm_max;
			}
			init_flag = 0;
                }else{
                        //cout<<"else "<<shm[shm_head]<<" "<<shm[shm[shm_head]]<<endl;
                        //fprintf(model_log_file,"in  else shm[shm_head] = %i, shm[shm[shm_head]]=%i\n",shm[shm_head],shm[shm[shm_head]]);
                        //printf("%i , %i\n",shm[shm_head],shm[shm[shm_head]]);
                }
        }//while
#endif
    	/*
     	* Finally, change the first character of the 
     	* segment to '*', indicating we have read 
     	* the segment.
     	*/
    	*shm = '*';

    	//exit(0);
}
#endif
