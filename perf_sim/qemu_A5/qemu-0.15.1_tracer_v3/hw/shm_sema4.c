#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/shm.h>
#include "sysbus.h"
#include "../cpu-all.h"
#include "../trace_timestamp.h"
#include "../Qemu_trace.h"
#include "sysemu.h"
#include "../sema4_mod.h"
#include <semaphore.h>
#include <sys/ipc.h>
#include <errno.h>
#include <sys/sem.h>
#include "../transformer-kind.h"
#define SNAME "/my_sem"
#define SEM_LOCK 121 //257
#define SEM_UNLOCK 122//258
#define WRITE_BUF_BASE 0x00
#define WRITE_BUF_SIZE 256

//==============
union semun {
	int val;               /* used for SETVAL only */
	struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
	ushort *array;         /* used for GETALL and SETALL */
};
key_t key = 0x753;
int semid;
struct sembuf sb_lock = {0, -1, IPC_NOWAIT}; 
struct sembuf sb_unlock = {0, 1, IPC_NOWAIT}; 
union semun arg;
int first=1;
//==============
extern int start_tracing;
extern int* shm_L1readhit;
extern int* shm_L1writehit;
extern int* shm_L2readhit;
extern int* shm_L2writehit;
extern int* shm_head_index;
extern int* shm_tail_index;
#define READ_BUF_BASE (WRITE_BUF_BASE + WRITE_BUF_SIZE)
#define READ_BUF_SIZE  256
extern unsigned int tb_total_cycle_cnt;
sem_t sem;
typedef struct{ 
    SysBusDevice busdev;
    uint8_t GATE[256];
    //uint8_t read_buffer[READ_BUF_SIZE];
    //uint8_t write_buffer[WRITE_BUF_SIZE];
}SEMA4_mod_state;
static uint32_t SEMA4_mod_readb(void *opaque, target_phys_addr_t offset)
{
	printf("SEMA4 read\n");
	SEMA4_mod_state *s = (SEMA4_mod_state *)opaque;
	//printf("offset = 0x%x\n",offset);
	fflush(stdout);
	if(offset < READ_BUF_BASE || offset >= READ_BUF_BASE + READ_BUF_SIZE){
	    //hw_error("SEMA4_mod_readb: Bad offset 0x%x\n", (int)offset);
	    //return 0;
	}
	
	//printf("%d\n",s->read_buffer[offset]);
	//return s->read_buffer[offset];
	//printf("%d\n", s->GATE[offset]);
	return 0;//s->GATE[offset];
}
static void SEMA4_mod_writeb(void * opaque, target_phys_addr_t offset, uint32_t value)
{
	int ret;
	int head, tail;
	//if(get_M4_cycle()<tb_total_cycle_cnt){}//tb_total_cycle_cnt = get_global_cycle();
	//else set_global_cycle(tb_total_cycle_cnt/3);
	set_global_cycle(tb_total_cycle_cnt/3);
	if((uint8_t)value==1){
	while( (tb_total_cycle_cnt/3)>get_M4_cycle()){
		if((tb_total_cycle_cnt/3)<=get_M4_cycle())break;	
	}
	}
	printf("current cycle %u\n",tb_total_cycle_cnt);
	if(start_tracing){
	while((*shm_head_index+1)%100!=*shm_tail_index){
		//printf("head %d\n",*shm_head_index);
		//printf("tail %d\n",*shm_tail_index);
		memcpy(&head,shm_head_index,sizeof(int));
		memcpy(&tail,shm_tail_index,sizeof(int));
		if((head+1)%100==tail)break;

	}	
	printf("L1readhit %d, L1writehit %d, L2readhit %d, L2writehit %d\n",*shm_L1readhit, *shm_L1writehit, *shm_L2readhit, *shm_L2writehit);
	}
	//printf("SEMA4 write\n");
	//printf("offset = 0x%x\n",offset);
	SEMA4_mod_state *s = (SEMA4_mod_state *)opaque;
	//lock
	if((uint8_t)value==1/*SEM_LOCK*/){
		tb_total_cycle_cnt += 52;
		while((ret = semop(semid, &sb_lock,1))<0){	
			if(errno==EAGAIN){
				
				if(first){
					printf("wait ....\n");
					first = 0;
				}
				tb_total_cycle_cnt += 2;
				//set_global_cycle(get_global_cycle()+2);
			}
			else{
				perror("semop");
				exit(1);
			}
		}
		first=1;
		printf("A5 sema lock --> %u\n",tb_total_cycle_cnt);
	}
	//unclock
	else if((uint8_t)value==0/*SEM_UNLOCK*/){
		tb_total_cycle_cnt += 42;
		while((ret = semop(semid, &sb_unlock,1))<0){	
			if(errno==EAGAIN){
				
				if(first){
					printf("wait ....\n");
					first = 0;
				}
				tb_total_cycle_cnt += 2;
			}
			else{
				perror("semop");
				exit(1);
			}
		}
		first=1;
		printf("A5 sema unlock --> %u\n",tb_total_cycle_cnt);
	}
	//if(get_global_cycle()>tb_total_cycle_cnt)tb_total_cycle_cnt = get_global_cycle();
	set_global_cycle(tb_total_cycle_cnt/3);
	//s->write_buffer[offset] = (uint8_t)value;
	//s->read_buffer[offset] = s->write_buffer[offset];
	//s->GATE[offset] = (uint8_t)value;
	//qGATE[offset].state = (uint8_t)value;
	//GATEnStateMachine(qGATE[offset],qGATE[offset].state);
	//printf("%d\n",value);
}
static CPUWriteMemoryFunc * const SEMA4_mod_writefn[] = {
	SEMA4_mod_writeb
};
static CPUReadMemoryFunc * const SEMA4_mod_readfn[] = {
	SEMA4_mod_readb
};
static int SEMA4_mod_init(SysBusDevice * dev)
{
	int iomemtype;
	SEMA4_mod_state *s = FROM_SYSBUS(SEMA4_mod_state, dev);
	iomemtype = cpu_register_io_memory(SEMA4_mod_readfn, SEMA4_mod_writefn, s,DEVICE_NATIVE_ENDIAN);
	if (iomemtype == -1) {
		hw_error("SEMA4_mod: Can't register I/O memory\n");
	}
	sysbus_init_mmio(dev, 0x1000, iomemtype);
	printf("SEMA4_mod init\n");
	//if (sem_init(&sem, 0, 0) == -1)hw_error("sem_init error");
	//sem_open(SNAME, O_CREAT, 0644, 0);
	//sem_unlink(SNAME);
	/* create a semaphore set with 1 semaphore: */
	if ((semid = semget(key, 1, 0666 | IPC_CREAT)) == -1) {
	      perror("semget");
	      exit(1);
	}
	/* initialize semaphore #0 to 1: */
	arg.val = 1;
	if (semctl(semid, 0, SETVAL, arg) == -1) {
	      perror("semctl");
	      exit(1);
	}
	return 0;
}
static void SEMA4_mod_register_devices(void)
{
     sysbus_register_dev("SEMA4_mod", sizeof(SEMA4_mod_state), SEMA4_mod_init);
}
device_init(SEMA4_mod_register_devices)

