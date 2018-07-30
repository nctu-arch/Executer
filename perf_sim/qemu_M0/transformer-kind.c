#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include "transformer-kind.h"
#define SHM_SIZE 1//256*256
int *shm_flag;
int *shm_buf;
int *shm_M;
static int current_ptr;
int shm_id_flag;
int shm_id_buf;
int flag4M_on=0;
int seg_cycle_cnt=0;
int *trans_cycle;
int *M4_cycle;
extern FILE * result;
extern FILE* insn_list;
void create_flag(){
	key_t shm_key=347;
	shm_id_flag = shmget(shm_key, sizeof(int),IPC_CREAT|0666);
	if((shm_flag = (int *)shmat(shm_id_flag, NULL, 0)) == -1){
		printf("in create flag\n");
		printf("shmatl error:  %x \n", shm_flag);
		perror("shmat");
		shmctl(shm_id_flag,IPC_RMID,0);
		exit(-1);
	}
	shm_flag[0] = 0;
}
void set_flag(){
	shm_flag[0] = 0;

}
void clear_flag(){
	shm_flag[0] = 1;

}
int check_flag(){ // 0 A move; 1 M move
	return shm_flag[0];	
}
void delete_flag(){
	shmdt(shm_flag);
	shmctl(shm_id_flag, IPC_RMID, NULL);
}
void create_buf(){ // first column is size latest put inside
	key_t shm_key = 934;
	int i;
	shm_id_buf = shmget(shm_key, SHM_SIZE, IPC_CREAT|0666);
        shm_buf = (int*)shmat(shm_id_buf, NULL, 0);
	if(shm_buf == -1){
		printf("in create buffer\n");
		printf("shmatl error: %x \n", shm_buf);
		perror("shmat");
		shmctl(shm_id_buf, IPC_RMID, 0);
		exit(-1);
	}
	/*for(i=0;i<(256*256/sizeof(int));i++){
		shm_buf[i] = 0;	
	}*/
	current_ptr = 1;
}
void write_buf(int in){
	shm_buf[current_ptr] = in;
	current_ptr += 1;
	if(current_ptr> (256*256/sizeof(int)))current_ptr = 1;
}
void write_buf_size(int size){
	shm_buf[0] = size;	
}
int read_buf(int size){
	
	
}
void delete_buf(){
	shmdt(shm_buf);
	shmctl(shm_id_buf, IPC_RMID, NULL);
}
void create_flag4M(){
	key_t shm_key = 751;
	int shm_id_M;
	shm_id_M = shmget(shm_key, sizeof(int), IPC_CREAT|0666);
	shm_M = (int*)shmat(shm_id_M, NULL, 0);
	if(shm_M == -1){
		printf("in create flag4M\n");
		printf("shmatl error: %x \n", shm_M);
		perror("shmat");
		shmctl(shm_id_M, IPC_RMID, 0);
		exit(-1);
	}


	
}
void set_flag4M(){
	shm_M[0] = 1;
	seg_cycle_cnt = 0;
}

/*void clear_flag4M(){
	shm_M[0] = 0;
	if(get_global_cycle()>seg_cycle_cnt){
	      printf("seg_cycle_cnt = %d + %d = %d\n",seg_cycle_cnt, get_global_cycle()-seg_cycle_cnt, get_global_cycle());
	      seg_cycle_cnt = get_global_cycle();
	}
	else set_global_cycle(seg_cycle_cnt);
	printf("seg_cycle_cnt = %d\n",seg_cycle_cnt);

}*/
void create_global_timer(){
	key_t shm_key = 263;
	int trans_cycle_id;
	int M4_cycle_id;
	trans_cycle_id = shmget(shm_key, sizeof(int), IPC_CREAT|0666);
	if((trans_cycle = (int *)shmat(trans_cycle_id, NULL, 0)) == -1){
		printf("in create global timer\n");
		printf("shmatl error:  %x \n", trans_cycle);
		perror("shmat");
		shmctl(trans_cycle,IPC_RMID,0);
		exit(-1);
	}
	trans_cycle[0] = 0;
	 shm_key = 233;
	
	M4_cycle_id = shmget(shm_key, sizeof(int), IPC_CREAT|0666);
	if((M4_cycle = (int *)shmat(M4_cycle_id, NULL, 0)) == -1){
		printf("in create global timer\n");
		printf("shmatl error:  %x \n", trans_cycle);
		perror("shmat");
		shmctl(M4_cycle,IPC_RMID,0);
		exit(-1);
	}
	M4_cycle[0] = 0;
	result = fopen("/home/cws/perf_sim/tracerlog/resultM","w");
	if(!result){
		printf("failed to open file\n");
		exit(1);
	}
	insn_list = fopen("/home/cws/perf_sim/tracerlog/insn_list","w");
	if(!insn_list){
		printf("failed to open file\n");
		exit(1);
	}
}
int get_A5_cycle(){
	return trans_cycle[0];	
}
void set_global_cycle(int in_cycle){
	//trans_cycle[0] = in_cycle;
	//M4_cycle[0] = in_cycle;
	//printf("in cycle %d\n",in_cycle);
	memcpy(M4_cycle,&in_cycle,sizeof(int));
}
