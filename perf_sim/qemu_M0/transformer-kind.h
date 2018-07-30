#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

void create_flag();
void set_flag();
int check_flag();
void create_buf(); // first column is size latest put inside
void write_buf(int in);
void write_buf_size(int size);
int read_buf(int size);
extern int* shm_flag;
extern int* shm_buf;
extern int flag4M_on;
void create_flag4M();
int check_flag4M();
extern int seg_cycle_cnt;
extern int* trans_cycle;
void create_global_timer();
int get_A5_cycle();
void set_global_cycle(int in_cycle);
