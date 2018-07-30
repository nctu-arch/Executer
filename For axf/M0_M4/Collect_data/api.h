#include <stdint.h>
#define SEMA_BASE 0x70050000
#define SHM_BASE 0x20050000
#define SHM_BUFSIZE 256*256
#define SEMA_LOCK 121
#define SEMA_UNLOCK 122
char* shm = (char*) SHM_BASE;
char* sema = (char*) SEMA_BASE;

void sema_lock(){
	memcpy((void*) SEMA_BASE,(void*) 121,sizeof(char));	
	//*sema=SEMA_LOCK;
}
void sema_unlock(){
	memcpy((void*) SEMA_BASE,(void*) 122,sizeof(char));	
	//*sema=SEMA_BASE;
}
int shm_write(uint32_t in_data, int offset){
	if(offset<0 || offset>=SHM_BUFSIZE/8-8)return -1;
	shm[offset] = in_data;
	return 0;
}
char shm_read(int offset){
	if(offset<0 || offset>=SHM_BUFSIZE/8-8)return -1;
	return shm[offset];
}
void set_int_flag(){
	memcpy((char*)0x70000000,"p",sizeof(char));	
}
void clear_int_flag(){
	memcpy((char*)0x70000000,"s",sizeof(char));	
}
