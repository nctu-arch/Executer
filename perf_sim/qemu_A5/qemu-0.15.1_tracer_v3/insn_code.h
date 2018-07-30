#include <stdint.h>
#include <stdio.h>
struct dep{
	int code;
	int affect;
	int vartype;
	uint8_t Rd, Rn, Rm, Rs, Ra, Rt, Rt2;
	uint8_t RdHi, RdLo;
	uint16_t Rlist;
	int cycle;
};
int checkLast(struct dep insn);
void regs(struct dep insn, struct dep insn_t);
void regVal(int type, int n, struct dep i_code);
int varVal(int type,struct dep i_code_t);
void regimmVal(/*int type, */uint32_t insn, struct dep i_code);
extern FILE* result;
extern uint8_t reg_record[300];
extern unsigned long long cycle_cnt;
void oneloop(int n);
