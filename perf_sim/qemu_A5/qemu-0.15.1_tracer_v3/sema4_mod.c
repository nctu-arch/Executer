#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/shm.h>
#include "sema4_mod.h"


int shm_id_gate;
int shm_id_cp0ine;
int shm_id_cp1ine;
int shm_id_cp0ntf;
int shm_id_cp1ntf;

//uint8_t SEMA4_CP0NTF[16] = {0};
//uint8_t SEMA4_CP1NTF[16] = {0};
//uint8_t SEMA4_CP0INE[16] = {0};
//uint8_t SEMA4_CP1INE[16] = {0};
uint8_t* SEMA4_CP0NTF;
uint8_t* SEMA4_CP1NTF;
uint8_t* SEMA4_CP0INE;
uint8_t* SEMA4_CP1INE;
#define SEMA4_INE 0
#define cp0_semaphore_init  SEMA4_CP0INE[0] & SEMA4_CP0NTF[0] \
                   | SEMA4_CP0INE[1] & SEMA4_CP0NTF[1] 	\
                   | SEMA4_CP0INE[2] & SEMA4_CP0NTF[2]	\
                   | SEMA4_CP0INE[3] & SEMA4_CP0NTF[3]	\
                   | SEMA4_CP0INE[4] & SEMA4_CP0NTF[4]	\
                   | SEMA4_CP0INE[5] & SEMA4_CP0NTF[5]	\
                   | SEMA4_CP0INE[6] & SEMA4_CP0NTF[6]	\
                   | SEMA4_CP0INE[7] & SEMA4_CP0NTF[7]	\
                   | SEMA4_CP0INE[8] & SEMA4_CP0NTF[8]	\
                   | SEMA4_CP0INE[9] & SEMA4_CP0NTF[9]	\
                   | SEMA4_CP0INE[10] & SEMA4_CP0NTF[10]	\
                   | SEMA4_CP0INE[11] & SEMA4_CP0NTF[11]	\
                   | SEMA4_CP0INE[12] & SEMA4_CP0NTF[12]	\
                   | SEMA4_CP0INE[13] & SEMA4_CP0NTF[13]	\
                   | SEMA4_CP0INE[14] & SEMA4_CP0NTF[14]	\
                   | SEMA4_CP0INE[15] & SEMA4_CP0NTF[15]	
#define cp1_semaphore_init  SEMA4_CP1INE[0] & SEMA4_CP1NTF[0]	\
                   | SEMA4_CP1INE[1] & SEMA4_CP1NTF[1]	\
                   | SEMA4_CP1INE[2] & SEMA4_CP1NTF[2]	\
                   | SEMA4_CP1INE[3] & SEMA4_CP1NTF[3]	\
                   | SEMA4_CP1INE[4] & SEMA4_CP1NTF[4]	\
                   | SEMA4_CP1INE[5] & SEMA4_CP1NTF[5]	\
                   | SEMA4_CP1INE[6] & SEMA4_CP1NTF[6]	\
                   | SEMA4_CP1INE[7] & SEMA4_CP1NTF[7]	\
                   | SEMA4_CP1INE[8] & SEMA4_CP1NTF[8]	\
                   | SEMA4_CP1INE[9] & SEMA4_CP1NTF[9]	\
                   | SEMA4_CP1INE[10] & SEMA4_CP1NTF[10]	\
                   | SEMA4_CP1INE[11] & SEMA4_CP1NTF[11]	\
                   | SEMA4_CP1INE[12] & SEMA4_CP1NTF[12]	\
                   | SEMA4_CP1INE[13] & SEMA4_CP1NTF[13]	\
                   | SEMA4_CP1INE[14] & SEMA4_CP1NTF[14]	\
                   | SEMA4_CP1INE[15] & SEMA4_CP1NTF[15]

 /* SEMA4 - Peripheral instance base addresses */
//#define SEMA4_BASE                              (0x4001D000u)
//#define SEMA4                                    ((SEMA4_Type *)SEMA4_BASE)

#define reset 0

#define UNLOCK      0x00
#define IDLE        0x00
#define CP0_LOCK  0x01
#define CP1_LOCK  0x02

/* IRQ Notification State Machine */
#define idle             0x00 
#define wait4_cp0_unlock 0x01
#define wait4_cp1_unlock 0x02
#define wait4_cp1_lock   0x05
#define wait4_cp0_lock   0x06

#define WRITE_BUF_BASE 0x00
#define WRITE_BUF_SIZE 256

#define READ_BUF_BASE (WRITE_BUF_BASE + WRITE_BUF_SIZE)
#define READ_BUF_SIZE  256

GATE_TYPE* qGATE;

uint8_t GATEnStateMachine(GATE_TYPE gate, uint8_t cur_state){
	uint8_t next_state = IDLE;
	if(reset){
	   next_state = IDLE;	
	}
	else{
	   switch(cur_state){
	   case IDLE:
	   	if(~((gate.master == cp0)&&(gate.wdata == wcp0_lock)) 
			&& ~((gate.master == cp1)&&(gate.wdata == wcp1_lock)))
		{  
			next_state = IDLE;
		}
		else if(gate.master == cp0 && gate.wdata == wcp0_lock)
		{
			next_state = CP0_LOCK;
		}
		else if(gate.master == cp1 && gate.wdata == wcp1_lock)
		{
			next_state = CP1_LOCK;
		}
	   break;
	   case CP0_LOCK:
	   	if(gate.master!=cp0 || gate.wdata!=wunlock)
		{
			next_state = CP0_LOCK;
		}
		else if(gate.master==cp0 && gate.wdata==wunlock)
		{
			next_state = IDLE;	
		}
	   break;
	   case CP1_LOCK:
	   	if(gate.master!=cp1 || gate.wdata!=wunlock)
		{
			next_state = CP1_LOCK;
		}
		else if(gate.master==cp1 && gate.wdata==wunlock)
		{
			next_state = IDLE;	
		}
	   break;
	   default:
	       next_state = cur_state;
		   
		   
 	   }	
	}
	gate.state = next_state;
	return next_state;	
}
uint8_t NTFStateMachine(GATE_TYPE gate, uint8_t cur_state){
	uint8_t next_state = idle;
	if(reset){
		next_state = idle;	
	}		
	else{
		switch(cur_state){
		case idle:
	   		if(~((gate.master == cp0)&&(gate.wdata == wcp0_lock)) 
				&& ~((gate.master == cp1)&&(gate.wdata == wcp1_lock)))
			{  
				next_state = idle;
			}
			else if(gate.master == cp1 && gate.wdata == wcp1_lock && gate.motion == cp0_lock)
			{
				next_state = wait4_cp0_unlock;
			}
			else if(gate.master == cp0 && gate.wdata == wcp0_lock && gate.motion == cp1_lock)
			{
				next_state = wait4_cp1_unlock;
			}
		break;
		case wait4_cp0_unlock:
			if(gate.motion!=unlock)
			{
				next_state = wait4_cp0_unlock;
			}
			else if(gate.motion == unlock)
			{
				next_state = wait4_cp1_lock;	
			}
		break;
		case wait4_cp1_unlock:
			if(gate.motion!=unlock)
			{
				next_state = wait4_cp1_unlock;
			}
			else if(gate.motion == unlock)
			{
				next_state = wait4_cp0_lock;	
			}
		break;
		case wait4_cp1_lock:
			if(gate.motion!=cp0_lock && gate.motion!=cp1_lock)
			{
				next_state = wait4_cp1_lock;	
			}
			else if(gate.motion == cp0_lock)
			{
				next_state = wait4_cp0_unlock;
			}
			else if(gate.motion == cp1_lock)
			{
				next_state = idle;	
			}
		break;
		case wait4_cp0_lock:
			if(gate.motion!=cp0_lock && gate.motion!=cp1_lock)
			{
				next_state = wait4_cp1_lock;	
			}
			else if(gate.motion == cp1_lock)
			{
				next_state = wait4_cp0_unlock;
			}
			else if(gate.motion == cp0_lock)
			{
				next_state = idle;	
			}
		break;
		default:
			next_state = cur_state;
		break;
		}	
	}
	gate.state = next_state;
	return next_state;
}


void set_reg(uint8_t GNn, int cur_state){
	if(NTFStateMachine(qGATE[GNn], cur_state) == wait4_cp0_lock){
		SEMA4_CP0NTF[GNn] = 1;	
	}	
	else SEMA4_CP0NTF[GNn] = 0;
	if(NTFStateMachine(qGATE[GNn], cur_state) == wait4_cp1_lock){
		SEMA4_CP1NTF[GNn] = 1;	
	}	
	else SEMA4_CP1NTF[GNn] = 0;
}
int gateLock(uint8_t CPn, uint8_t GNn){
	int current_value;
	int locked_value;
	if(CPn==0)locked_value = CP0_LOCK;
	else locked_value = CP1_LOCK;
	do{
		current_value = qGATE[GNn].state;
	}while(current_value != UNLOCK);

	do{
		qGATE[GNn].state = locked_value;
		current_value = qGATE[GNn].state;
	}while(current_value!=locked_value);
	return 1;
	
}
void gateUnlock(uint8_t GNn){
	qGATE[GNn].wdata = wunlock;	
}
void gateInit(){
	key_t shm_key1 = 204;
	key_t shm_key2 = 212;
	key_t shm_key3 = 232;
	key_t shm_key4 = 278;
	key_t shm_key5 = 299;
	int i = 0;
	
	shm_id_gate = shmget(shm_key1, 16*sizeof(GATE_TYPE), IPC_CREAT|0666);
	qGATE = (GATE_TYPE*)shmat(shm_id_gate, NULL, 0);


#ifdef SEMA4_INE	
	shm_id_cp0ine = shmget(shm_key2, 16*sizeof(uint8_t), IPC_CREAT|0666);
	shm_id_cp1ine = shmget(shm_key3, 16*sizeof(uint8_t), IPC_CREAT|0666);
	shm_id_cp0ntf = shmget(shm_key4, 16*sizeof(uint8_t), IPC_CREAT|0666);
	shm_id_cp1ntf = shmget(shm_key5, 16*sizeof(uint8_t), IPC_CREAT|0666);
	SEMA4_CP1INE = (GATE_TYPE*)shmat(shm_id_cp0ine, NULL, 0);
	SEMA4_CP0INE = (GATE_TYPE*)shmat(shm_id_cp1ine, NULL, 0);
	SEMA4_CP1NTF = (GATE_TYPE*)shmat(shm_id_cp0ntf, NULL, 0);
	SEMA4_CP0NTF = (GATE_TYPE*)shmat(shm_id_cp1ntf, NULL, 0);

	for(i = 0; i< 16; i++){
		qGATE[i].state = IDLE;
		SEMA4_CP1INE[i] = 0;
		SEMA4_CP0INE[i] = 0;
		SEMA4_CP1NTF[i] = 0;
		SEMA4_CP0NTF[i] = 0;
	}
#endif	
}
//if(==wdata)?wdata_eq_unlock:wdata_eq_cp0_lock;
//if(==wdata)?wdata_eq_unlock:wdata_eq_cp1_lock;
//if(==master)?master_eq_cp0:master_eq_cp1;
