#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum {unlock, lock, cp0_lock, cp1_lock} motion_type;
typedef enum {wcp0_lock, wcp1_lock, wunlock} wdata_type;
typedef enum {cp0, cp1} cp_type;
typedef struct {
	cp_type master;
	wdata_type wdata;
	uint8_t state;
	motion_type motion;
}GATE_TYPE;
//GATE_TYPE gate[16];
extern GATE_TYPE *qGATE;
/*typedef struct {
    SysBusDevice busdev;
    uint8_t GATE[256];
}SEMA4_mod_state;*/



//GATE_TYPE* gate;

uint8_t GATEnStateMachine(GATE_TYPE gate, uint8_t cur_state);
uint8_t NTFStateMachine(GATE_TYPE gate, uint8_t cur_state);


void set_reg(uint8_t GNn, int cur_state);
int gateLock(uint8_t CPn, uint8_t GNn);
void gateUnlock(uint8_t GNn);
void gateInit();
