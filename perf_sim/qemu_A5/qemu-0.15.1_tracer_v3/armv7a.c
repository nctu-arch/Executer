//ISA armv7-a cortex-a8 cortex-a9
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "insn_code.h"
struct dep i_code;
struct dep i_code_t;
//FILE *fptr;
//FILE *fptr2;
extern FILE *result;
char buff[50];
unsigned long long cycle_cnt = 0, cycle_last = 0;
uint32_t insn_op1;
int insn_cnt = 0;
uint8_t reg_record[300]={0};
int in_table = 0;
//char* filename = "/home/yhchiang/tracerlog/insn_trace_A.log";
#define QEMU_TEST
void table_17(uint32_t insn){
}
void table_18(uint32_t insn){
	uint8_t op2 =(insn>5)&0x3;
	uint8_t imm5 = (insn>>7)&0x1F;
	if(op2==0){
		if(imm5 ==0){
			fprintf(result,"MOV reg ARM\n");
			i_code.vartype = 3;
			regimmVal(insn, i_code);
			cycle_cnt +=1;
		}
		else {
			fprintf(result,"LSL imm \n");
			i_code.vartype = 3;
			regimmVal(insn, i_code);
			cycle_cnt +=2;
		}
	}
	else if(op2==1){
		fprintf(result,"LSR imm\n");
			cycle_cnt +=1;
		i_code.vartype = 3;
		regimmVal(insn, i_code);
	}
	else if(op2==2){
		fprintf(result,"ASR imm\n");
			cycle_cnt +=1;
		i_code.vartype = 3;
		regimmVal(insn, i_code);
	}
	else if(op2==3){
		if(imm5 == 0){
			fprintf(result,"RRX \n");
			cycle_cnt +=1;
			i_code.vartype = 3;
			regimmVal(insn, i_code);
		}
		else {
			fprintf(result,"ROR imm\n");
			cycle_cnt +=1;
			i_code.vartype = 3;
			regimmVal(insn, i_code);
		}
	}
	else fprintf(result,"error\n");
}
void table_3(uint32_t insn){//Data-processing (register)
	uint8_t op = (insn>>20)&0x1F;
	uint8_t op_1 = op>>1;
	if(op_1 == 0){
		fprintf(result,"AND reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 1){
		fprintf(result,"EOR reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 2){
		fprintf(result,"SUB reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 3){
		fprintf(result,"RSB reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 4){
		fprintf(result,"ADD reg ARM\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 5){
		fprintf(result,"ADC reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 6){
		fprintf(result,"SBC reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 7){
		fprintf(result,"RSC reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if((op&0x19)==0x10)table_00(insn);//Data-processing and miscellaneous instructions
	else if(op == 0x11){
		fprintf(result,"TST reg\n");
			cycle_cnt +=1;
		i_code.vartype = 2;
		regimmVal(insn, i_code);
	}
	else if(op == 0x13){
		fprintf(result,"TEQ reg\n");
			cycle_cnt +=1;
		i_code.vartype = 2;
		regimmVal(insn, i_code);
	}
	else if(op == 0x15){
		fprintf(result,"CMP reg\n");
			cycle_cnt +=1;
		i_code.vartype = 2;
		regimmVal(insn, i_code);
	}
	else if(op == 0x17){
		fprintf(result,"CMN reg\n");
			cycle_cnt +=1;
		i_code.vartype = 2;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 0xC){
		fprintf(result,"ORR reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 0xD)table_18(insn);
	else if(op_1 == 0xE){
		fprintf(result,"BIC reg\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op_1 == 0xF){
		fprintf(result,"MVN reg\n");
			cycle_cnt +=1;
		i_code.vartype = 3;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");

}
void table_19(uint32_t insn){
	uint8_t op2 = (insn>>5)&0x3;
	if(op2==0){
		fprintf(result,"LSL reg\n");
			cycle_cnt +=2;
		i_code.vartype = 6;
		regimmVal(insn, i_code);
	}
	else if(op2==1){
		fprintf(result,"LSR reg\n");
			cycle_cnt +=1;
		i_code.vartype = 6;
		regimmVal(insn, i_code);
	}
	else if(op2==2){
		fprintf(result,"ASR reg\n");
			cycle_cnt +=1;
		i_code.vartype = 6;
		regimmVal(insn, i_code);
	}
	else if(op2==3){
		fprintf(result,"ROR reg\n");
			cycle_cnt +=1;
		i_code.vartype = 6;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");
}
void table_4(uint32_t insn){//Data-processing (register-shifted register)
	uint8_t op1 = (insn>>20)&0x1F;

	uint8_t op1_1 = op1>>1;
	if(op1_1 == 0){
		fprintf(result,"AND reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 1){
		fprintf(result,"EOR reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 2){
		fprintf(result,"SUB reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 3){
		fprintf(result,"RSBreg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 4){
		fprintf(result,"ADD reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 5){
		fprintf(result,"ADC reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 6){
		fprintf(result,"SBC reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 7){
		fprintf(result,"RSC reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if((op1&0x19)==0x10)table_00(insn);//Data-processing and miscellaneous instructions
	else if(op1 == 0x11){
		fprintf(result,"TST reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 5;
		regimmVal(insn, i_code);
	}
	else if(op1 == 0x13){
		fprintf(result,"TEQ reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 5;
		regimmVal(insn, i_code);
	}
	else if(op1 == 0x15){
		fprintf(result,"CMP reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 5;
		regimmVal(insn, i_code);
	}
	else if(op1 == 0x17){
		fprintf(result,"CMN reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 5;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 0xC){
		fprintf(result,"ORR reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 0xD)table_19(insn);
	else if(op1_1 == 0xE){
		fprintf(result,"BIC reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 4;
		regimmVal(insn, i_code);
	}
	else if(op1_1 == 0xF){
		fprintf(result,"MVN reg-shift reg\n");
			cycle_cnt +=1;
		i_code.vartype = 7;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");

}
void table_20(uint32_t insn){//Saturating addition and subtraction
	uint8_t op = (insn>>21)&0x3;
	if(op==0){
		fprintf(result,"QADD\n");
			cycle_cnt +=2;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op==1){
		fprintf(result,"QSUB\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op==2){
		fprintf(result,"QDADD\n");
			cycle_cnt +=1;
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else if(op==3){
		fprintf(result,"QDSUB\n");
		i_code.vartype = 1;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");
}
void table_5(uint32_t insn){//Miscellaneous instructions on page
	uint8_t op2 = (insn>>4)&0x7;
	uint8_t B = (insn>>9)&0x1; // //B?
	uint8_t op = (insn>>21)&0x3;
	uint8_t op1 = (insn>>16)&0xf;
	if(op2==0){
		if(B==1){
			if(op&1==0){
				fprintf(result,"MRS banked reg\n");
			cycle_cnt +=1;
				i_code.vartype = 8;
				regimmVal(insn, i_code);
			}
			else if(op&1==1){
				fprintf(result,"MSR banked reg\n");
			cycle_cnt +=1;
				i_code.vartype = 9;
				regimmVal(insn, i_code);
			}
		}
		else if(B==0){
			if(op&1==0){
				fprintf(result,"MRS MRS\n");
			cycle_cnt +=1;
				i_code.vartype = 8;
				regimmVal(insn, i_code);
			}
			else if(op==0x1){
				if((op1&0x3)==0){
					fprintf(result,"MSR reg\n");
			cycle_cnt +=1;
					i_code.vartype = 9;
					regimmVal(insn, i_code);
				}
				else if((op1&0x3)==1 || (op1&0x2)==0x2){
					fprintf(result,"MSR reg\n");
			cycle_cnt +=1;
					i_code.vartype = 9;
					regimmVal(insn, i_code);
				}
			}
			else if(op==0x3){
				fprintf(result,"MSR reg\n");
			cycle_cnt +=1;
				i_code.vartype = 9;
				regimmVal(insn, i_code);
			}
		}
	}
	else if(op2==0x1){
		if(op==0x1){
			fprintf(result,"BX\n");
			cycle_cnt +=1;
			i_code.vartype = 10;
			regimmVal(insn, i_code);
		}
		else if(op==0x3){
			fprintf(result,"CLZ\n");
			cycle_cnt +=1;
			i_code.vartype = 3;
			regimmVal(insn, i_code);
		}
	}
	else if(op2==0x2 && op==0x1){
		fprintf(result,"BXJ\n");
			cycle_cnt +=1;
		i_code.vartype = 10;
		regimmVal(insn, i_code);
	}
	else if(op2==0x3 && op==0x1){
		fprintf(result,"BLX reg\n");
			cycle_cnt +=1;
		i_code.vartype = 10;
		regimmVal(insn, i_code);
	}
	else if(op2==0x5)table_20(insn);//Saturating addition and subtraction
	else if(op2==0x6 && op==0x3){
		fprintf(result,"ERET\n");
			cycle_cnt +=1;
		i_code.affect==0;
	}
	else if(op2==0x7){
		if(op==1){
			fprintf(result,"BKPT\n");
			cycle_cnt +=1;
			i_code.affect==0;
		}
		else if(op==2){
			fprintf(result,"HVC\n");
			cycle_cnt +=1;
			i_code.affect==0;
		}
		else if(op==3){
			fprintf(result,"SMC (pre SMI)\n");
			cycle_cnt +=1;
			i_code.affect==0;
		}
	}
}
void table_6(uint32_t insn){//Halfword multiply and multiply accumulate
	uint8_t op1 = (insn>>21)&0x3;
	uint8_t op = (insn>>5)&0x1;
	if(op1==0){
		fprintf(result,"SMLABB BT TB TT\n");
			cycle_cnt +=3; //check
		i_code.vartype = 11;
		regimmVal(insn, i_code);
	}
	else if(op1==1 && op==0){
		fprintf(result,"SMLAWB SMLAWT\n");
			cycle_cnt +=3; //check
		i_code.vartype = 11;
		regimmVal(insn, i_code);
	}
	else if(op1==1 && op==1){
		fprintf(result,"SMULWB SMULWT\n");
			cycle_cnt +=3; //check
		i_code.vartype = 12;
		regimmVal(insn, i_code);
	}
	else if(op1==2){
		fprintf(result,"SMLALBB SMLALBT SMLALTB SMLALT\n");
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else if(op1==3){
		fprintf(result,"SMULBB SMULBT SMULTB SMULTT\n");
			cycle_cnt +=3; //check
		i_code.vartype = 12;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");
}
void table_7(uint32_t insn){//Multiply and multiply accumulate
	uint8_t op = (insn>>20)&0xF;
	if(op>>1==0){
		fprintf(result,"MUL\n");
			cycle_cnt +=3;//check
		i_code.vartype = 14;
		regimmVal(insn, i_code);
	}
	else if(op>>1==1){
		fprintf(result,"MLA\n");
			cycle_cnt +=2; //check
		i_code.vartype = 11;
		regimmVal(insn, i_code);
	}
	else if(op==0x4){
		fprintf(result,"UMAAL\n");
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else if(op==0x5){
		fprintf(result,"undefined\n");
	}
	else if(op==0x6){
		fprintf(result,"MLS\n"); 
			cycle_cnt +=2;//check
		i_code.vartype = 11;
		regimmVal(insn, i_code);
	}
	else if(op==0x7){
		fprintf(result,"undefined\n");
	}
	else if(op>>1==0x4){
		fprintf(result,"UMULL\n"); 
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else if(op>>1==0x5){
		fprintf(result,"UMLAL\n");
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else if(op>>1==0x6){
		fprintf(result,"SMULL\n");
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else if(op>>1==0x7){
		fprintf(result,"SMLAL\n");
			cycle_cnt +=3; //check
		i_code.vartype = 13;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error");
}
void table_8(uint32_t insn){//Synchronization primitives
	uint8_t op = (insn>>20)&0xF;
	if((op&0xB)==0){
		fprintf(result,"SWP SWPB\n");
			cycle_cnt +=1;
		i_code.vartype = 15;
		regimmVal(insn, i_code);
	}
	else if(op==0x8){
		fprintf(result,"STREX\n");
			cycle_cnt +=2;
		i_code.vartype = 16;
		regimmVal(insn, i_code);
	}
	else if(op==0x9){
		fprintf(result,"LDREX\n");
			cycle_cnt +=3;
		i_code.vartype = 17;
		regimmVal(insn, i_code);
	}
	else if(op==0xA){
		fprintf(result,"STREXD\n");
			cycle_cnt +=2;
		i_code.vartype = 16;
		regimmVal(insn, i_code);
	}
	else if(op==0xB){
		fprintf(result,"LDREXD\n");
			cycle_cnt +=3;
		i_code.vartype = 17;
		regimmVal(insn, i_code);
	}
	else if(op==0xC){
		fprintf(result,"STREXB\n");
			cycle_cnt +=2;
		i_code.vartype = 16;
		regimmVal(insn, i_code);
	}
	else if(op==0xD){
		fprintf(result,"LDREXB\n");
			cycle_cnt +=3;
		i_code.vartype = 17;
		regimmVal(insn, i_code);
	}
	else if(op==0xE){
		fprintf(result,"STREXH\n");
			cycle_cnt +=2;
		i_code.vartype = 16;
		regimmVal(insn, i_code);
	}
	else if(op==0xF){
		fprintf(result,"LDREXH\n");
			cycle_cnt +=3;
		i_code.vartype = 17;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");
}
void table_9(uint32_t insn){//Extra load/store instructions
//op1!= 0xx1x  op2=1 [01] 1
	uint8_t op1 = ((insn>>20)&0x1F)&0x5;
	uint8_t Rn = (insn>>16)&0xf;
	if(op1==0){
		fprintf(result,"STRH reg\n");
			cycle_cnt +=2;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else if(op1==1){
		fprintf(result,"LDRH reg\n");
			cycle_cnt +=3;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else if(op1==4){
		fprintf(result,"STRH imm ARM\n");
			cycle_cnt +=2;
		i_code.vartype = 17;
		regimmVal(insn, i_code);
	}
	else if(op1==5){
		if(Rn==0xf){
			fprintf(result,"LDRH imm ARM\n");
			cycle_cnt +=3;
			i_code.vartype = 17;
			regimmVal(insn, i_code);
		}
		else {
			fprintf(result,"LDRH literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
			i_code.vartype = 19;
			regimmVal(insn, i_code);
		}
	}
	else fprintf(result,"error\n");
}
void table_10(uint32_t insn){//Extra load/store instructions
//op2=1 [1x] 1
	uint8_t op1 = ((insn>>20)&0x1F)&0x5;
	uint8_t Rn = (insn>>16)&0xf;
	uint8_t op2 = (insn>>5)&0x3;
	if(op2==2){
		if(op1==0){
			fprintf(result,"LDRD reg\n");
			cycle_cnt +=3;
			i_code.vartype = 18;
			regimmVal(insn, i_code);
		}
		else if(op1==1){
			fprintf(result,"LDRSB reg\n");
			cycle_cnt +=3;
			i_code.vartype = 18;
			regimmVal(insn, i_code);
		}
		else if(op1==4){
			if(Rn==0xf){
				fprintf(result,"LDRD imm\n");
			cycle_cnt +=3;
				i_code.vartype = 17;
				regimmVal(insn, i_code);
			}
			else {
				fprintf(result,"LDRD literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
				i_code.vartype = 19;
				regimmVal(insn, i_code);
			}
		}
		else if(op1==5){
			if(Rn==0xf){
				fprintf(result,"LDRSB imm\n");
			cycle_cnt +=3;
				i_code.vartype = 17;
				regimmVal(insn, i_code);
			}
			else {
				fprintf(result,"LDRSB literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
				i_code.vartype = 19;
				regimmVal(insn, i_code);
			}
		}
	}
	else if(op2==0x3){
		if(op1==0){
			fprintf(result,"STRD reg\n");
			cycle_cnt +=2;
			i_code.vartype = 18;
			regimmVal(insn, i_code);
		}
		else if(op1==1){
			fprintf(result,"LDRSH reg\n");
			cycle_cnt +=3;
			i_code.vartype = 18;
			regimmVal(insn, i_code);
		}
		else if(op1==4){
			fprintf(result,"STRD imm\n");
			cycle_cnt +=2;
			i_code.vartype = 17;
			regimmVal(insn, i_code);
		}
		else if(op1==5){
			if(Rn==0xF){
				fprintf(result,"LDRSH imm\n");
			cycle_cnt +=3;
				i_code.vartype = 17;
				regimmVal(insn, i_code);
			}
			else {
				fprintf(result,"LDRSH literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
				i_code.vartype = 19;
				regimmVal(insn, i_code);
			}
		}
	}
	else fprintf(result,"error\n");
}
void table_11(uint32_t insn){//Extra load/store instructions, unprivileged
	uint8_t op2 = (insn>>5)&0x3;
	uint8_t op = (insn>>20)&0x1;
	if(op2==1 && op==0){
		fprintf(result,"STRHT\n");
			cycle_cnt +=2;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else if(op2==1 && op==1){
		fprintf(result,"LDRHT\n");
			cycle_cnt +=3;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else if(op2==2 && op==1){
		fprintf(result,"LDRSBT\n");
			cycle_cnt +=3;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else if(op2==3 && op==1){
		fprintf(result,"LDRSHT\n");
			cycle_cnt +=3;
		i_code.vartype = 18;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");

}
void table_12(uint32_t insn){//Extra load/store instructions
	uint8_t op1 = (insn>>20)&0x1F;
	uint8_t Rn = (insn>>16)&0xf;
	uint8_t op2 = (insn>>5)&0x3;
	if(op2==0x0 || (op1&0x13)==3 ||((op1&0x13)==0x2 && (op2&0x2)==0))table_00(insn);
	else fprintf(result,"error\n");
}
void table_13(uint32_t insn){//Data-processing (immediate) //check
	uint8_t op = (insn>>20)&0x1f;
	uint8_t Rn = (insn>>16)&0xf;
	uint8_t op_1 = op>>1;
	if(op_1==0){
		fprintf(result,"AND imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==1){
		fprintf(result,"EOR imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==2){
		if(Rn!=0xF){
			fprintf(result,"SUB imm ARM\n");
			cycle_cnt +=2;
			i_code.vartype = 20;
			regimmVal(insn, i_code);
		}
		else {
			fprintf(result,"ADR PC\n");//adr pc
			cycle_cnt +=2;
			//cycle_cnt +=30; // adr as branch
			i_code.vartype = 8;
			regimmVal(insn, i_code);
		}
	}
	else if(op_1==3){
		fprintf(result,"RSB imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==4){
		if(Rn!=0xF){
			fprintf(result,"ADD imm ARM\n");
			cycle_cnt +=1;
			i_code.vartype = 20;
			regimmVal(insn, i_code);
		}
		else {
			fprintf(result,"ADR\n");
			cycle_cnt +=2;
			//cycle_cnt +=30; // adr as branch
			i_code.vartype = 8;
			regimmVal(insn, i_code);
		}
	}
	else if(op_1==5){
		fprintf(result,"ADC imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==6){
		fprintf(result,"SBC imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==7){
		fprintf(result,"RSC imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if((op&0x19)==0x10)table_00(insn);
	else if(op==0x11){
		fprintf(result,"TST imm\n");
			cycle_cnt +=2;
		i_code.vartype = 21;
		regimmVal(insn, i_code);
	}
	else if(op==0x13){
		fprintf(result,"TEQ imm\n");
			cycle_cnt +=2;
		i_code.vartype = 21;
		regimmVal(insn, i_code);
	}
	else if(op==0x15){
		fprintf(result,"CMP imm\n");
			cycle_cnt +=2;
		i_code.vartype = 21;
		regimmVal(insn, i_code);
	}
	else if(op==0x17){
		fprintf(result,"CMN imm\n");
			cycle_cnt +=2;
		i_code.vartype = 21;
		regimmVal(insn, i_code);
	}
	else if(op_1==0xC){
		fprintf(result,"ORR imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==0xD){
		fprintf(result,"MOV imm\n");
			cycle_cnt +=1;
		i_code.vartype = 8;
		regimmVal(insn, i_code);
	}
	else if(op_1==0xE){
		fprintf(result,"BIC imm\n");
			cycle_cnt +=2;
		i_code.vartype = 20;
		regimmVal(insn, i_code);
	}
	else if(op_1==0xF){
		fprintf(result,"MVN imm\n");
			cycle_cnt +=2;
		i_code.vartype = 8;
		regimmVal(insn, i_code);
	}
	else fprintf(result,"error\n");

}
void table_14(uint32_t insn){//16-bit immediate load
	fprintf(result,"MOV imm\n");
			cycle_cnt +=1;
	i_code.vartype = 8;
	regimmVal(insn, i_code);
}
void table_15(uint32_t insn){//High halfword 16-bit immediate load
	fprintf(result,"MOVT \n");
			cycle_cnt +=1;
	i_code.vartype = 8;
	regimmVal(insn, i_code);
}
void table_16(uint32_t insn){//MSR (immediate), and hints
	uint8_t op = (insn>>22)&0x1;
	uint8_t op1 = (insn>>16)&0xf;
	uint8_t op2 = insn & 0xFF;
	if(op==0){
		if(op1==0){
			if(op2==0x00){
				fprintf(result,"NOP\n");
			cycle_cnt +=1;
			}
			else if(op2==0x01){
				fprintf(result,"YIELD\n");
			cycle_cnt +=1;
			}
			else if(op2==0x02){
				fprintf(result,"WFE\n");
			cycle_cnt +=1;
			}
			else if(op2==0x3){
				fprintf(result,"WFI\n");
			cycle_cnt +=1;
			}
			else if(op2==0x4){
				fprintf(result,"SEV\n");
			cycle_cnt +=1;
			}
			else if((op2>>4)==0xf){
				fprintf(result,"DBG\n");
			cycle_cnt +=1;
			}
		}
		else if(op1==0x4 || (op1&0xB)==0x8){
			fprintf(result,"MSR(A) imm\n");
			cycle_cnt +=1;
		}
		else if((op1&0x3)==0x1 || (op1&0x2)==0x2){
			fprintf(result,"MSR(B) imm\n");
			cycle_cnt +=1;
		}
	}
	else if(op==1){
		fprintf(result,"MSR(B) imm\n");
			cycle_cnt +=1;
	}
	else fprintf(result,"error\n");
	i_code.affect==0;
}
void table_1(uint32_t insn){
	uint8_t op1 = (insn>>20)&0x1F;
	uint8_t op2 = (insn>>4)&0xF;
	if((op1&0x19)!=0x10){
		if((op2&0x1)==0)table_3(insn);//Data-processing (register)
		else if((op2&0x9)==1)table_4(insn);//Data-processing (register-shifted register)
	}
	else if((op1&0x19)==0x10){
		if((op2&0x8)==0)table_5(insn);//Miscellaneous instructions on page
		else if((op2&0x9)==0x8)table_6(insn);//Halfword multiply and multiply accumulate
	}
	else if((op1&0x10)==0 && op2==0x9)table_7(insn);//Multiply and multiply accumulate
	else if((op1&0x10)==1 && op2==0x9)table_8(insn);//Synchronization primitives
	else if((op1&0x12)!=2){
		if(op2==0xB)table_9(insn);//Extra load/store instructions
		else if((op2&0xD)==0xD)table_10(insn);//Extra load/store instructions
	}
	else if((op1&0x12)==2){
		if(op2==0xB)table_11(insn);//Extra load/store instructions, unprivileged
		else if((op2&0xD)==0xD)table_12(insn);//Extra load/store instructions
	}
	else fprintf(result,"error\n");
}
void table_2(uint32_t insn){
	uint8_t op1 = (insn>>20)&0x1F;
	if((op1&0x19)!=0x10)table_13(insn); //Data-processing (immediate)
	else if(op1==0x10)table_14(insn); //16-bit immediate load, MOV (immediate)
	else if(op1==0x14)table_15(insn); //High halfword 16-bit immediate load, MOVT
	else if((op1&0x1B)==0x12)table_16(insn); //MSR (immediate), and hints
}
void table_00(uint32_t insn){//Data-processing and miscellaneous instructions
	uint8_t op = (insn>>25)&1;

	if(op==0)table_1(insn);
	else if(op==1)table_2(insn);
	else fprintf(result,"error\n");
}
void table_01(uint32_t insn){//Load/store word and unsigned byte
//op1[25:27] = 010
//A=0
	uint8_t op1 = (insn>>20)&0x1F;
	uint8_t Rn = (insn>>16)&0xF;
	uint8_t op1_1 = op1&0x5;
	uint8_t op1_2 = op1&0x17;
	if(op1_1==0 && op1_2!=2){
        fprintf(result,"STR imm ARM\n");
			cycle_cnt +=2;
        i_code.vartype = 17;
        regimmVal(insn, i_code);
	}
	else if(op1_2==2){
        fprintf(result,"STRT \n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==1 && op1_2!=3 && Rn!=0xF){
        fprintf(result,"LDR imm ARM\n");
			cycle_cnt +=3;
        i_code.vartype = 17;
        regimmVal(insn, i_code);
	}
	else if(op1_1==1 && op1_2!=3 && Rn==0xF){
        fprintf(result,"LDR literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
        i_code.vartype = 19;
        regimmVal(insn, i_code);
	}
	else if(op1_2==3){
        fprintf(result,"LDRT\n");
			cycle_cnt +=3;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==4 && op1_2!=6){
        fprintf(result,"STRB imm ARM\n");
			cycle_cnt +=2;
        i_code.vartype = 17;
        regimmVal(insn, i_code);
	}
	else if(op1_2==6){
        fprintf(result,"STRBT\n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==5 && op1_2!=7 && Rn!=0xF){
        fprintf(result,"LDRB imm ARM\n");
			cycle_cnt +=3;
        i_code.vartype = 17;
        regimmVal(insn, i_code);
	}
	else if(op1_1==5 && op1_2!=7 && Rn==0xF){
        fprintf(result,"LDRB literal\n");
			cycle_cnt +=3;
			//cycle_cnt +=120; // literal as branch
        i_code.vartype = 19;
        regimmVal(insn, i_code);
	}
	else if(op1_2==7){
        fprintf(result,"LDRBT\n");
			cycle_cnt +=3;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
}
void table_02(uint32_t insn){//Load/store word and unsigned byte
//op1[25:27] = 011
//A=1
    if((insn>>25)&1==1 && (insn>>4)&1==1){
        table_03(insn);
        return;
    }
    uint8_t op1 = (insn>>20)&0x1F;
    uint8_t op1_1 = op1&0x5;
	uint8_t op1_2 = op1&0x17;
    if(op1_1==0 && op1_2!=2){
        fprintf(result,"STR reg\n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_2==2){
        fprintf(result,"STRT \n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==1 && op1_2!=3){
        fprintf(result,"LDR reg ARM\n");
			cycle_cnt +=3;
			//cycle_cnt +=10;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_2==3){
        fprintf(result,"LDRT\n");
			cycle_cnt +=3;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==4 && op1_2!=6){
        fprintf(result,"STRB reg\n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_2==6){
        fprintf(result,"STRBT\n");
			cycle_cnt +=2;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_1==5 && op1_2!=7){
        fprintf(result,"LDRB reg\n");
			cycle_cnt +=3;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
	else if(op1_2==7){
        fprintf(result,"LDRBT\n");
			cycle_cnt +=3;
        i_code.vartype = 18;
        regimmVal(insn, i_code);
	}
}
void table_25(uint32_t insn){//Saturating instructions
    uint8_t op2 = (insn>>5)&0x7;
    if(op2==0){
        fprintf(result,"QADD16\n");
			cycle_cnt +=2;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==1){
        fprintf(result,"QASX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==2){
        fprintf(result,"QSAX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==3){
        fprintf(result,"QSUB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==4){
        fprintf(result,"QADD8\n");
			cycle_cnt +=2;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==7){
        fprintf(result,"QSUB8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");
}
void table_26(uint32_t insn){//Halving instructions
    uint8_t op2 = (insn>>5)&0x7;
    if(op2==0){
        fprintf(result,"SHADD16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==1){
        fprintf(result,"SHASX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==2){
        fprintf(result,"SHSAX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==3){
        fprintf(result,"SHSUB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==4){
        fprintf(result,"SHADD8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==7){
        fprintf(result,"SHSUB8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");
}
void table_21(uint32_t insn){//Parallel addition and subtraction, signed
    uint8_t op1 = (insn>>20)&0x3;
    uint8_t op2 = (insn>>5)&0x7;
    if(op1==2)table_25(insn);//Saturating instructions
    else if(op1==3)table_26(insn);//Halving instructions
    else if(op1==1){
        if(op2==0){
            fprintf(result,"SADD16\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==1){
            fprintf(result,"SASX\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==2){
            fprintf(result,"SSAX\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==3){
            fprintf(result,"SSUB16\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==4){
            fprintf(result,"SADD8\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==7){
            fprintf(result,"SSUB8\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
    }
    else fprintf(result,"error\n");
}
void table_27(uint32_t insn){//Saturating instructions
    uint8_t op2 = (insn>>5)&0x7;
    if(op2==0){
        fprintf(result,"UQADD16\n");
			cycle_cnt +=2;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==1){
        fprintf(result,"UQASX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==2){
        fprintf(result,"UQSAX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==3){
        fprintf(result,"UQSUB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==4){
        fprintf(result,"UQADD8\n");
			cycle_cnt +=2;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==7){
        fprintf(result,"UQSUB8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");
}
void table_28(uint32_t insn){//Halving instructions
    uint8_t op2 = (insn>>5)&0x7;
    if(op2==0){
        fprintf(result,"UHADD16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==1){
        fprintf(result,"UHASX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==2){
        fprintf(result,"UHSAX\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==3){
        fprintf(result,"UHSUB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==4){
        fprintf(result,"UHADD8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op2==7){
        fprintf(result,"UHSUB8\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");
}
void table_22(uint32_t insn){//Parallel addition and subtraction, unsigned
    uint8_t op1 = (insn>>20)&0x3;
    uint8_t op2 = (insn>>5)&0x7;
    if(op1==2)table_25(insn);//Saturating instructions
    else if(op1==3)table_26(insn);//Halving instructions
    else if(op1==1){
        if(op2==0){
            fprintf(result,"UADD16\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==1){
            fprintf(result,"UASX\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==2){
            fprintf(result,"USAX\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==3){
            fprintf(result,"USUB16\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==4){
            fprintf(result,"UADD8\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
        else if(op2==7){
            fprintf(result,"USUB8\n");
			cycle_cnt +=1;
            i_code.vartype = 1;
            regimmVal(insn, i_code);
        }
    }
    else fprintf(result,"error\n");
}
void table_23(uint32_t insn){//Packing, unpacking, saturation, and reversal
    uint8_t op1 = (insn>>20)&0x7;
    uint8_t op2 = (insn>>5)&0x7;
    uint8_t A = (insn>>16)&0xF;
    if(op1==0 && op2&1==0){
        fprintf(result,"PKH\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==0 && op2==3 && A!=0xF){
        fprintf(result,"SXTAB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==0 && op2==3 && A==0xF){
        fprintf(result,"SXTB16\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==0 && op2==5 ){
        fprintf(result,"SEL\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==1 && op2&1==0){
        fprintf(result,"SSAT\n");
			cycle_cnt +=1;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1==2 && op2==1){
        fprintf(result,"SSAT16\n");
			cycle_cnt +=1;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1==2 && op2==3 && A!=0xF){
        fprintf(result,"SXTAB\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==2 && op2==3 && A==0xF){
        fprintf(result,"SXTB\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==3 && op2==1){
        fprintf(result,"REV\n");
			cycle_cnt +=2;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==3 && op2==3 && A!=0xF){
        fprintf(result,"SXTAH\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==3 && op2==3 && A==0xF){
        fprintf(result,"SXTH\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==3 && op2==5){
        fprintf(result,"REV16\n");
			cycle_cnt +=2;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==4 && op2==3 && A!=0xF){
        fprintf(result,"UXTAB16\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==4 && op2==3 && A==0xF){
        fprintf(result,"UXTB16\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==3 && op2&1==0){
        fprintf(result,"USAT\n");
			cycle_cnt +=1;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1==6 && op2==1){
        fprintf(result,"USAT16\n");
			cycle_cnt +=1;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1==6 && op2==3 && A!=0xF){
        fprintf(result,"UXTAB\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==6 && op2==3 && A==0xF){
        fprintf(result,"UXTB\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==7 && op2==1){
        fprintf(result,"RBIT\n");
			cycle_cnt +=2;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==7 && op2==3 && A!=0xF){
        fprintf(result,"UXTAH\n");
			cycle_cnt +=1;
        i_code.vartype = 1;
        regimmVal(insn, i_code);
    }
    else if(op1==7 && op2==3 && A==0xF){
        fprintf(result,"UXTH\n");
			cycle_cnt +=1;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else if(op1==7 && op2==5){
        fprintf(result,"REVSH\n");
			cycle_cnt +=2;
        i_code.vartype = 3;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");

}
void table_24(uint32_t insn){//Signed multiply, signed and unsigned divide
    uint8_t op1 = (insn>>20)&0x7;
    uint8_t op2 = (insn>>5)&0x7;
    uint8_t A = (insn>>16)&0xF;
    if(op1==0){
        if(op2>>1==0 && A!=0xF){
            fprintf(result,"SMLAD\n");
			cycle_cnt +=3;
            i_code.vartype = 11;
            regimmVal(insn, i_code);
        }
        else if(op2>>1==0 && A==0xF){
            fprintf(result,"SMUAD\n");
			cycle_cnt +=3;
            i_code.vartype = 12;
            regimmVal(insn, i_code);
        }
        else if(op2>>1==1 && A!=0xF){
            fprintf(result,"SMLSD\n");
			cycle_cnt +=1;
            i_code.vartype = 11;
            regimmVal(insn, i_code);
        }
        else if(op2>>1==1 && A==0xF){
            fprintf(result,"SMUSD\n");
			cycle_cnt +=1;
            i_code.vartype = 12;
            regimmVal(insn, i_code);
        }
    }
    else if(op1==1 && op2==0){
        fprintf(result,"SDIV\n");
			cycle_cnt +=6;//-22;1
        i_code.vartype = 12;
        regimmVal(insn, i_code);
    }
    else if(op1==3 && op2==0){
        fprintf(result,"UDIV\n");
			cycle_cnt +=5;//-21;1
        i_code.vartype = 12;
        regimmVal(insn, i_code);
    }
    else if(op1==4 && op2>>1==0){
        fprintf(result,"SMLALD\n");
			cycle_cnt +=3;
        i_code.vartype = 13;
        regimmVal(insn, i_code);
    }
    else if(op1==4 && op2>>1==1){
        fprintf(result,"SMLSLD\n");
			cycle_cnt +=2;
        i_code.vartype = 13;
        regimmVal(insn, i_code);
    }
    else if(op1==5 && op2>>1==0 && A!=0xF){
        fprintf(result,"SMMLA\n");
			cycle_cnt +=3;
        i_code.vartype = 11;
        regimmVal(insn, i_code);
    }
    else if(op1==5 && op2>>1==0 && A==0xF){
        fprintf(result,"SMMUL\n");
			cycle_cnt +=3;
        i_code.vartype = 12;
        regimmVal(insn, i_code);
    }
    else if(op1==5 && op2>>1==3){
        fprintf(result,"SMMLS\n");
			cycle_cnt +=1;
        i_code.vartype = 11;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");

}
void table_03(uint32_t insn){//Media instructions
    uint8_t op1 = (insn>>20)&0x1F;
    uint8_t op2 = (insn>>5)&0x7;
    uint8_t Rn = insn&0xF;
    uint8_t Rd = (insn>>12)&0xF;
    uint8_t cond = (insn>>28)&0xF;
    if(op1>>2==0)table_21(insn);//Parallel addition and subtraction, signed
    else if(op1>>2==1)table_22(insn);//Parallel addition and subtraction, unsigned
    else if(op1>>3==1)table_23(insn);//Packing, unpacking, saturation, and reversal
    else if(op1>>3==2)table_24(insn);//Signed multiply, signed and unsigned divide on
    else if(op1==0x18 && op2==0 && Rd==0xF){
        fprintf(result,"USAD8\n");
			cycle_cnt +=1;
        i_code.vartype = 14;
        regimmVal(insn, i_code);
    }
    else if(op1==0x18 && op2==0 && Rd!=0xF){
        fprintf(result,"USADA8\n");
			cycle_cnt +=1;
        i_code.vartype = 11;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==0xD && op2&0x3==2){
        fprintf(result,"SBFX\n");
			cycle_cnt +=2;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==0xE && op2&0x3==0 && Rn==0xF){
        fprintf(result,"BFC\n");
			cycle_cnt +=2;
        i_code.vartype = 8;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==0xE && op2&0x3==0 && Rn!=0xF){
        fprintf(result,"BFI\n");
			cycle_cnt +=2;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1>>1==0xF && op2&0x3==2){
        fprintf(result,"UBFX\n");
			cycle_cnt +=1;
        i_code.vartype = 22;
        regimmVal(insn, i_code);
    }
    else if(op1==0x1F && op2==0x7 && cond==0xE){
        fprintf(result,"UDF\n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else if(op1==0x1F && op2==0x7 && cond!=0xE){
        fprintf(result,"unconditional\n");
    }
    else fprintf(result,"error\n");
}
void table_04(uint32_t insn){//Branch, branch with link, and block data transfer
    uint8_t op = (insn>>20)&0x3F;
    uint8_t op_1 = op&0x3D;
    uint8_t Rn = (insn>>16)&0xF;
    uint8_t R = (insn>>15)&1;
    uint16_t Rlist;
    if(op_1==0){
        fprintf(result,"STMDA(STMED)\n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op_1==1){
        fprintf(result,"LDMDA/LDMFA\n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op_1==8){
        fprintf(result,"STM(STMIA,STMEA)\n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op==9){
        fprintf(result,"LDM/LDMIA/LDMFD (ARM)\n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op==0xB && Rn!=0xD){
        fprintf(result,"LDM/LDMIA/LDMFD (ARM)\n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op==0xB && Rn==0xD){
        fprintf(result,"POP (ARM)\n");
			cycle_cnt +=1;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
	//cycle_cnt += 120; // pop as branch
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(op==0x10){
        fprintf(result,"STMDB (STMFD) \n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op==0x12 && Rn!=0xD){
        fprintf(result,"STMDB (STMFD) \n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op==0x12 && Rn==0xD){
        fprintf(result,"PUSH\n");
			cycle_cnt +=1;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(op_1==0x11){
        fprintf(result,"LDMDB/LDMEA\n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op_1==0x18){
        fprintf(result,"STMIB (STMFA)\n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op_1==0x19){
        fprintf(result,"LDMIB/LDMED\n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op&0x25==4){
        fprintf(result,"STM (User registers)\n");
			cycle_cnt +=2;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 23;
        regimmVal(insn, i_code);
    }
    else if(op&0x25==5 && R==0){
        fprintf(result,"LDM (User registers) \n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 24;
        regimmVal(insn, i_code);
    }
    else if(op&0x25==5 && R==1){
        fprintf(result,"LDM (exception return) \n");
			cycle_cnt +=3;
        Rlist = insn & 0xffff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
        i_code.vartype = 24;
        regimmVal(insn, i_code);
    }
    else if(op>>4==2){
        fprintf(result,"B label\n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else if(op>>4==3){
        fprintf(result,"BL, BLX imm\n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else fprintf(result,"error\n");
}
void table_29(uint32_t insn){//Extension register load/store instructions
    uint8_t opcode = (insn>>20)&0x1F;
    uint8_t Rn = (insn>>16)&0xF;
    uint8_t op = opcode&0x1B;
    if(opcode>>1==2)table_30(insn);
    else if(op==0x8){
        fprintf(result,"VSTM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0xA){
        fprintf(result,"VSTM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(opcode&0x13==0x10){
        fprintf(result,"VSTR\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0x12 && Rn!=0xF){
        fprintf(result,"VSTM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0x12 && Rn==0xF){
        fprintf(result,"VPUSH\n");
        i_code.affect==0;
    }
    else if(op==0x9){
        fprintf(result,"VLDM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0xB && Rn!=0xF){
        fprintf(result,"VLDM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0xB && Rn==0xF){
        fprintf(result,"VPOP\n");
        i_code.affect==0;
    }
    else if(opcode&0x13==0x11){
        fprintf(result,"VLDR\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op==0x13){
        fprintf(result,"VLDM\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");


}
void table_30(uint32_t insn){//64-bit transfers between ARM core and extension registers
    uint8_t C = (insn>>8)&1;
    uint8_t op = (insn>>4)&0xF;
    if(C==0 && op&0xD==1){
        fprintf(result,"VMOV (between two ARM core registers and two single-precision registers)\n");
        i_code.vartype = 25;
        regimmVal(insn, i_code);
    }
    else if(C==1 && op&0xD==1){
        fprintf(result,"VMOV (between two ARM core registers and a doubleword extension register) \n");
        i_code.vartype = 25;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");
}
void table_42(uint32_t insn){//Vreg
    uint8_t opc2 = (insn>>16)&0xF;
    uint8_t opc3 = (insn>>6)&0x3;
    uint8_t op3 = opc3&0x1;
    if(op3==0){
        fprintf(result,"VMOV (immediate) \n");
    }
    else if(opc2==0 &&opc3==1){
        fprintf(result,"VMOV (reg) \n");
    }
    else if(opc2==0 &&opc3==3){
        fprintf(result,"VABS\n");
    }
    else if(opc2==1 &&opc3==1){
        fprintf(result,"VNEG\n");
    }
    else if(opc2==1 &&opc3==3){
        fprintf(result,"VSQRT\n");
    }
    else if(opc2>>1==1 &&op3==1){
        fprintf(result,"VCVTB, VCVTT\n");
    }
    else if(opc2>>1==2 &&op3==1){
        fprintf(result,"VCMP, VCMPE\n");
    }
    else if(opc2==7 &&opc3==3){
        fprintf(result,"VCVT (between double-precision and single-precision)\n");
    }
    else if(opc2==8 &&op3==1){
        fprintf(result,"VCVT, VCVTR (between floating-point and integer, Floating-point)\n");
    }
    else if(opc2>>1==5 &&op3==1){
        fprintf(result,"VCVT (between floating-point and fixed-point, Floating-point)\n");
    }
    else if(opc2>>1==6 &&op3==1){
        fprintf(result,"VCVT, VCVTR (between floating-point and integer, Floating-point)\n");
    }
    else if(opc2>>1==7 &&op3==1){
        fprintf(result,"VCVT (between floating-point and fixed-point, Floating-point)\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;

}
void table_31(uint32_t insn){//Floating-point data-processing instructions  Vreg
    uint8_t opc1 = (insn>>20)&0xF;
    uint8_t opc3 = (insn>>6)&0x3;
    uint8_t op = opc1&0xB;
    uint8_t op3 = opc3&0x1;
    if(op==0xB)table_42(insn);
    else if(op==0){
        fprintf(result,"VMLA, VMLS (floating-point)\n");
    }
    else if(op==1){
        fprintf(result,"VNMLA, VNMLS, VNMUL\n");
    }
    else if(op==2 && op3==1){
        fprintf(result,"VNMLA, VNMLS, VNMUL\n");
    }
    else if(op==2 && op3==0){
        fprintf(result,"VMUL (floating-point)\n");
    }
    else if(op==3 && op3==0){
        fprintf(result,"VADD (floating-point)\n");
    }
    else if(op==3 && op3==1){
        fprintf(result,"VSUB (floating-point) \n");
    }
    else if(op==8 && op3==0){
        fprintf(result,"VDIV\n");
    }
    else if(op==9){
        fprintf(result,"VFNMA, VFNMS\n");
    }
    else if(op==0xA){
        fprintf(result,"VFMA, VFMS\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;
}
void table_32(uint32_t insn){//8, 16, and 32-bit transfer between ARM core and extension registers
    uint8_t L = (insn>>20)&0x1;
    uint8_t C = (insn>>8)&0x1;
    uint8_t A = (insn>>21)&0x7;
    uint8_t B = (insn>>5)&0x3; // //B?
    if(L==0 && C==0 && A==0){
        fprintf(result,"VMOV (between ARM core register and single-precision register)\n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==0 && C==0 && A==0x7){
        fprintf(result,"VMSR VMSR(System level view)\n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==0 && C==1 && A>>2==0){
        fprintf(result,"VMOV (ARM core register to scalar) \n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==0 && C==1 && A>>2==1 && B>1==0){
        fprintf(result,"VDUP (ARM core register) \n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==1 && C==0 && A==0){
        fprintf(result,"VMOV (between ARM core register and single-precision register) \n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==1 && C==0 && A==0x7){
        fprintf(result,"VMSR VMSR(System level view)\n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else if(L==1 && C==1){
        fprintf(result,"VMOV (scalar to ARM core register)\n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
    }
    else fprintf(result,"error\n");


}
void table_05(uint32_t insn){//Coprocessor instructions, and Supervisor Call
    uint8_t coproc = (insn>>8)&0xF;
    uint8_t op1 = (insn>>20)&0x3F;
    uint8_t op = (insn>>4)&1;
    uint8_t Rn = (insn>>16)&0xF;
    if(op1>>1 == 0)fprintf(result,"undefined\n");
    else if(op1>>4==3){
        fprintf(result,"SVC (previously SWI) \n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else if(coproc>>1!=5){
        if(op1&0x21==0 && op1&0x3B!=0){
            fprintf(result,"STC, STC2\n");
			cycle_cnt +=1;
            i_code.vartype = 21;
            regimmVal(insn, i_code);
        }
        else if(op1&0x21==1 && op1&0x3B!=1 && Rn!=0xF){
            fprintf(result,"LDC, LDC2 (immediate) \n");
			cycle_cnt +=1;
            i_code.vartype = 21;
            regimmVal(insn, i_code);
        }
        else if(op1&0x21==1 && op1&0x3B!=1 && Rn==0xF){
            fprintf(result,"LDC, LDC2 (literal) \n");
			cycle_cnt +=1;
			//cycle_cnt +=120; // literal as branch
            i_code.affect==0;
        }
        else if(op1==0x4){
            fprintf(result,"MCRR, MCRR2\n");
			cycle_cnt +=1;
            i_code.vartype = 25;
            regimmVal(insn, i_code);
        }
        else if(op1==0x5){
            fprintf(result,"MRRC, MRRC2\n");
			cycle_cnt +=1;
            i_code.vartype = 25;
            regimmVal(insn, i_code);
        }
        else if(op1>>4==2 && op==0){
            fprintf(result,"CDP, CDP2\n");
			cycle_cnt +=1;
            i_code.affect==0;
        }
        else if(op1&0x31==0x20 && op==1){
            fprintf(result,"MCR, MCR2\n");
			cycle_cnt +=1;
            i_code.vartype = 19;
            regimmVal(insn, i_code);
        }
        else if(op1&0x31==0x21 && op==1){
            fprintf(result,"MRC, MRC2 \n");
			cycle_cnt +=1;
            i_code.vartype = 19;
            regimmVal(insn, i_code);
        }
    }
    else if(coproc>>1==5){
        if(op1>>5==0 && op1&0x3A!=0)table_29(insn);//Extension register load/store instructions
        else if(op1>>1==2)table_30(insn);//64-bit transfers between ARM core and extension registers
        else if(op1>>4==2 && op==0)table_31(insn);//Floating-point data-processing instructions
        else if(op1>>4==2 && op==1)table_32(insn);//8, 16, and 32-bit transfer between ARM core and extension registers
    }
    else fprintf(result,"error\n");
}
void table_36(uint32_t insn){//Three registers of the same length
    uint8_t U = (insn>>24)&1;
    uint8_t C = (insn>>20)&0x3;
    uint8_t A = (insn>>8)&0xF;
    uint8_t B1 = (insn>>4)&0x1;
    if(A==0 && B1==0){
        fprintf(result,"VHADD, VHSUB \n");
    }
    else if(A==0 && B1==1){
        fprintf(result,"VQADD\n");
    }
    else if(A==1 && B1==0){
        fprintf(result,"VRHADD\n");
    }
    else if(A==1 && B1==1 && U==0 && C==0){
        fprintf(result,"VAND (register) \n");
    }
    else if(A==1 && B1==1 && U==0 && C==1){
        fprintf(result,"VBIC (register) \n");
    }
    else if(A==1 && B1==1 && U==0 && C==2){
        fprintf(result,"VORR (register)  if source registers differ\n");
    }
    else if(A==1 && B1==1 && U==0 && C==2){
        fprintf(result,"VMOV (register) if source registers identical\n");
    }
    else if(A==1 && B1==1 && U==0 && C==3){
        fprintf(result,"VORN (register)\n");
    }
    else if(A==1 && B1==1 && U==1 && C==0){
        fprintf(result,"VEOR \n");
    }
    else if(A==1 && B1==1 && U==1 && C==1){
        fprintf(result,"VBIF, VBIT, VBSL\n");
    }
    else if(A==1 && B1==1 && U==1 && C==2){
        fprintf(result,"VBIF, VBIT, VBSL\n");
    }
    else if(A==1 && B1==1 && U==1 && C==3){
        fprintf(result,"VBIF, VBIT, VBSL\n");
    }
    else if(A==2 && B1==0){
        fprintf(result,"VHADD, VHSUB\n");
    }
    else if(A==2 && B1==1){
        fprintf(result,"VQSUB\n");
    }
    else if(A==3 && B1==0){
        fprintf(result,"VCGT (register)\n");
    }
    else if(A==3 && B1==1){
        fprintf(result,"VCGE (register) \n");
    }
    else if(A==4 && B1==0){
        fprintf(result,"VSHL (register)\n");
    }
    else if(A==4 && B1==1){
        fprintf(result,"VQSHL (register)\n");
    }
    else if(A==5 && B1==0){
        fprintf(result,"VRSHL\n");
    }
    else if(A==5 && B1==1){
        fprintf(result,"VQRSHL\n");
    }
    else if(A==6){
        fprintf(result,"VMAX, VMIN (integer) \n");
    }
    else if(A==7 && B1==0){
        fprintf(result,"VABD, VABDL (integer)\n");
    }
    else if(A==7 && B1==1){
        fprintf(result,"VABA, VABAL\n");
    }
    else if(A==8 && B1==0 && U==0){
        fprintf(result,"VADD (integer)\n");
    }
    else if(A==8 && B1==0 && U==1){
        fprintf(result,"VSUB (integer)\n");
    }
    else if(A==8 && B1==1 && U==0){
        fprintf(result,"VTST\n");
    }
    else if(A==8 && B1==1 && U==1){
        fprintf(result,"VCEQ (register)\n");
    }
    else if(A==9 && B1==0){
        fprintf(result,"VMLA, VMLAL, VMLS, VMLSL (integer)\n");
    }
    else if(A==9 && B1==1){
        fprintf(result,"VMUL, VMULL (integer and polynomial)\n");
    }
    else if(A==0xA){
        fprintf(result,"VPMAX, VPMIN (integer) \n");
    }
    else if(A==0xB && B1==0 && U==0){
        fprintf(result,"VQDMULH\n");
    }
    else if(A==0xB && B1==0 && U==1){
        fprintf(result,"VQRDMULH \n");
    }
    else if(A==0xB && B1==1 && U==0){
        fprintf(result,"VPADD (integer) \n");
    }
    else if(A==0xC && B1==1 && U==0){
        fprintf(result,"VFMA, VFMS\n");
    }
    else if(A==0xD && B1==0 && U==0 && C>>1==0){
        fprintf(result,"VADD (floating-point) \n");
    }
    else if(A==0xD && B1==0 && U==0 && C>>1==1){
        fprintf(result,"VSUB (floating-point) \n");
    }
    else if(A==0xD && B1==0 && U==1 && C>>1==0){
        fprintf(result,"VPADD (floating-point) \n");
    }
    else if(A==0xD && B1==0 && U==1 && C>>1==1){
        fprintf(result,"VABD (floating-point) \n");
    }
    else if(A==0xD && B1==1 && U==0 ){
        fprintf(result,"VMLA, VMLS (floating-point)\n");
    }
    else if(A==0xD && B1==1 && U==1 && C>>1==0){
        fprintf(result,"VMUL (floating-point) \n");
    }
    else if(A==0xE && B1==0 && U==0 && C>>1==0){
        fprintf(result,"VCEQ (register) \n");
    }
    else if(A==0xE && B1==0 && U==1 && C>>1==0){
        fprintf(result,"VCGE (register)\n");
    }
    else if(A==0xE && B1==0 && U==1 && C>>1==1){
        fprintf(result,"VCGT (register)\n");
    }
    else if(A==0xE && B1==1 && U==1 ){
        fprintf(result,"VACGE, VACGT, VACLE, VACLT\n");
    }
    else if(A==0xF && B1==0 && U==0 ){
        fprintf(result,"VMAX, VMIN (floating-point)\n");
    }
    else if(A==0xF && B1==0 && U==1 ){
        fprintf(result,"VPMAX, VPMIN (floating-point)\n");
    }
    else if(A==0xF && B1==1 && U==0 && C>>1==0){
        fprintf(result,"VRECPS \n");
    }
    else if(A==0xF && B1==1 && U==0 && C>>1==1){
        fprintf(result,"VRSQRTS\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;

}
void table_37(uint32_t insn){//One register and a modified immediate value Vreg
    uint8_t op = (insn>>5)&1;
    uint8_t cmode = (insn>>8)&0xF;
    uint8_t c0 = cmode&0x9;
    if(op==0){
        if(c0==0){
            fprintf(result,"VMOV (immediate)\n");
        }
        else if(c0==1){
            fprintf(result,"VORR (immediate) \n");
        }
        else if(cmode&0xD==8){
            fprintf(result,"VMOV (immediate) \n");
        }
        else if(cmode&0xD==9){
            fprintf(result,"VORR (immediate)\n");
        }
        else if(cmode>>2==3){
            fprintf(result,"VMOV (immediate) \n");
        }
    }
    else if(op==1){
        if(c0==0){
            fprintf(result,"VMVN (immediate) \n");
        }
        else if(c0==1){
            fprintf(result,"VBIC (immediate) \n");
        }
        else if(cmode&0xD==8){
            fprintf(result,"VMVN (immediate)\n");
        }
        else if(cmode&0xD==9){
            fprintf(result,"VBIC (immediate) \n");
        }
        else if(cmode>>1==6){
            fprintf(result,"VMVN (immediate)\n");
        }
        else if(cmode==0xE){
            fprintf(result,"VMOV (immediate) \n");
        }
        else if(cmode==0xF){
            fprintf(result,"undefined\n");
        }
    }
    else fprintf(result,"error\n");
    i_code.affect==0;
}
void table_38(uint32_t insn){//Two registers and a shift amount  Vreg
    uint8_t A = (insn>>8)&0xF;
    uint8_t U = (insn>>24)&0x1;
    uint8_t L = (insn>>7)&1;
    uint8_t B1 = (insn>>6)&0x1;
    if(A==0){
        fprintf(result,"VSHR\n");
    }
    else if(A==1){
        fprintf(result,"VSRA\n");
    }
    else if(A==2){
        fprintf(result,"VRSHR\n");
    }
    else if(A==3){
        fprintf(result,"VRSRA\n");
    }
    else if(A==4 && U==1){
        fprintf(result,"VSRI\n");
    }
    else if(A==5 && U==0){
        fprintf(result,"VSHL (immediate) \n");
    }
    else if(A==5 && U==1){
        fprintf(result,"VSLI\n");
    }
    else if(A>>1==3){
        fprintf(result,"VQSHL, VQSHLU (immediate) \n");
    }
    else if(A==8 && U==0 && B1==0 && L==0){
        fprintf(result,"VSHRN\n");
    }
    else if(A==8 && U==0 && B1==1 && L==0){
        fprintf(result,"VRSHRN\n");
    }
    else if(A==8 && U==1 && B1==0 && L==0){
        fprintf(result,"VQSHRN, VQSHRUN\n");
    }
    else if(A==8 && U==1 && B1==1 && L==0){
        fprintf(result,"VQRSHRN, VQRSHRUN\n");
    }
    else if(A==9 && B1==0 && L==0){
        fprintf(result,"VQSHRN, VQSHRUN\n");
    }
    else if(A==9 && B1==1 && L==0){
        fprintf(result,"VQRSHRN, VQRSHRUN\n");
    }
    else if(A==0xA && B1==0 && L==0){
        fprintf(result,"VSHLL\n");
    }
    else if(A==0xA && B1==0 && L==0){
        fprintf(result,"VMOVL\n");
    }
    else if(A>>1==0x7 && L==0){
        fprintf(result,"VCVT (between floating-point and fixed-point, Advanced SIMD) \n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;
}
void table_39(uint32_t insn){//Three registers of different lengths Vreg
    uint8_t A = (insn>>8)&0xF;
    uint8_t U = (insn>>24)&0x1;
    if(A>>1==0){
        fprintf(result,"VADDL, VADDW\n");
    }
    else if(A>>1==1){
        fprintf(result,"VSUBL, VSUBW\n");
    }
    else if(A==4 && U==0){
        fprintf(result,"VADDHN\n");
    }
    else if(A==4 && U==1){
        fprintf(result,"VRADDHN\n");
    }
    else if(A==5){
        fprintf(result,"VABA, VABAL\n");
    }
    else if(A==6 && U==0){
        fprintf(result,"VSUBHN\n");
    }
    else if(A==6 && U==1){
        fprintf(result,"VRSUBHN\n");
    }
    else if(A==7){
        fprintf(result,"VABD, VABDL (integer)\n");
    }
    else if(A&0xD==8){
        fprintf(result,"VMLA, VMLAL, VMLS, VMLSL (integer) \n");
    }
    else if(A&0xD==9 && U==0){
        fprintf(result,"VQDMLAL, VQDMLSL\n");
    }
    else if(A==0xC){
        fprintf(result,"VMUL, VMULL (integer and polynomial)\n");
    }
    else if(A==0xD && U==0){
        fprintf(result,"VQDMULL\n");
    }
    else if(A==0xE){
        fprintf(result,"VMUL, VMULL (integer and polynomial)\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;
}
void table_40(uint32_t insn){//Two registers and a scalar Vreg
    uint8_t A = (insn>>8)&0xF;
    uint8_t U = (insn>>24)&0x1;
    if(A&0xA==0){
        fprintf(result,"VMLA, VMLAL, VMLS, VMLSL (by scalar)\n");
    }
    else if(A&0xB==2){
        fprintf(result,"VMLA, VMLAL, VMLS, VMLSL (by scalar)\n");
    }
    else if(A&0xB==3 && U==0){
        fprintf(result,"VQDMLAL, VQDMLSL\n");
    }
    else if(A>>1==4){
        fprintf(result,"VMUL, VMULL (by scalar) \n");
    }
    else if(A==0xA){
        fprintf(result,"VMUL, VMULL (by scalar) \n");
    }
    else if(A==0xB && U==0){
        fprintf(result,"VQDMULL\n");
    }
    else if(A==0xC){
        fprintf(result,"VQDMULH\n");
    }
    else if(A==0xD){
        fprintf(result,"VQRDMULH\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;
}
void table_41(uint32_t insn){//Two registers, miscellaneous   Vreg
    uint8_t A = (insn>>16)&0x3;
    uint8_t B0 = (insn>>6)&0x1F; //B
    uint8_t B1 = B0>>1;
    uint8_t B3 = B0&0x11; //B2
    if(A==0){
        if(B1==0){
            fprintf(result,"VREV16, VREV32, VREV64\n");
        }
        else if(B1==1){
            fprintf(result,"VREV16, VREV32, VREV64\n");
        }
        else if(B1==2){
            fprintf(result,"VREV16, VREV32, VREV64\n");
        }
        else if(B1>>1==2){
            fprintf(result,"VPADDL\n");
        }
        else if(B1==8){
            fprintf(result,"VCLS\n");
        }
        else if(B1==9){
            fprintf(result,"VCLZ\n");
        }
        else if(B1==0xA){
            fprintf(result,"VCNT\n");
        }
        else if(B1==0xB){
            fprintf(result,"VMVN (register) \n");
        }
        else if(B1>>1==6){
            fprintf(result,"VPADAL\n");
        }
        else if(B1==0xE){
            fprintf(result,"VQABS\n");
        }
        else if(B1==0xF){
            fprintf(result,"VQNEG\n");
        }
    }
    else if(A==1){
        if(B3==0){
            fprintf(result,"VCGT (immediate #0)\n");
        }
        else if(B3==2){
            fprintf(result,"VCGE (immediate #0) \n");
        }
        else if(B3==4){
            fprintf(result,"VCEQ (immediate #0)\n");
        }
        else if(B3==6){
            fprintf(result,"VCLE (immediate #0)\n");
        }
        else if(B3==8){
            fprintf(result,"VCLT (immediate #0)\n");
        }
        else if(B3==0xC){
            fprintf(result,"VABS\n");
        }
        else if(B3==0xE){
            fprintf(result,"VNEG\n");
        }
    }
    else if(A==2){
        if(B1==0){
            fprintf(result,"VSWP\n");
        }
        else if(B1==1){
            fprintf(result,"VTRN\n");
        }
        else if(B1==2){
            fprintf(result,"VUZP\n");
        }
        else if(B1==3){
            fprintf(result,"VZIP\n");
        }
        else if(B0==8){
            fprintf(result,"VMOVN\n");
        }
        else if(B0==9){
            fprintf(result,"VQMOVN, VQMOVUN \n");
        }
        else if(B1==5){
            fprintf(result,"VQMOVN, VQMOVUN\n");
        }
        else if(B0==0xC){
            fprintf(result,"VSHLL\n");
        }
        else if(B0&0x1B==0x18){
            fprintf(result,"VCVT (between half-precision and single-precision, Advanced SIMD)\n");
        }
    }
    else if(A==3){
        if(B0&0x1A==0x10){
            fprintf(result,"VRECPE\n");
        }
        else if(B0&0x1A==0x12){
            fprintf(result,"VRSQRTE\n");
        }
        else if(B0>>3==3){
            fprintf(result,"VCVT (between floating-point and integer, Advanced SIMD)\n");
        }
    }
    else fprintf(result,"error\n");
    i_code.affect==0;

}
void table_34(uint32_t insn){//Advanced SIMD data-processing instructions
    uint8_t U = (insn>>24)&1;
    uint8_t A = (insn>>19)&0x1F;
    uint8_t B0 = (insn>>8)&0xF;
    uint8_t C = (insn>>4)&0xF;
    if(A>>4==0)table_36(insn);//Three registers of the same length
    else if(A&0x17==0x10 && C&0x9==1)table_37(insn);//One register and a modified immediate value
    else if(A&0x17==0x11 && C&0x9==1)table_38(insn);//Two registers and a shift amount
    else if(A&0x16==0x12 && C&0x9==1)table_38(insn);//Two registers and a shift amount
    else if(A&0x14==0x14 && C&0x9==1)table_38(insn);//Two registers and a shift amount
    else if(A&0x10==0x10 && C&0x9==9)table_38(insn);//Two registers and a shift amount
    else if(A&0x14==0x10 && C&0x5==0)table_39(insn);//Three registers of different lengths
    else if(A&0x16==0x14 && C&0x5==0)table_39(insn);//Three registers of different lengths
    else if(A&0x14==0x10 && C&0x5==4)table_40(insn);//Two registers and a scalar
    else if(A&0x16==0x14 && C&0x5==4)table_40(insn);//Two registers and a scalar
    else if(U==0 && A&0x16==0x16 && C&1==0){
        fprintf(result,"VEXT\n");
    }
    else if(U==1 && A&0x16==0x16 && B0>>3==0 && C&1==0)table_41(insn);//Two registers, miscellaneous
    else if(U==1 && A&0x16==0x16 && B0&0xC==8 && C&1==0){
        fprintf(result,"VTBL VTBX\n");
    }
    else if(U==1 && A&0x16==0x16 && B0==0xC && C&0x9==0){
        fprintf(result,"VDUP (scalar)\n");
    }
    else fprintf(result,"error\n");
    i_code.affect==0;

}
void table_35(uint32_t insn){//Advanced SIMD element or structure load/store instructions
    uint8_t A = (insn>>23)&1;
    uint8_t B0 = (insn>>8)&0xF;
    uint8_t L = (insn>>21)&1;
    if(A==0 &&L==0){
        if(B0==2 || B0>>1==3 || B0==0xA){
            fprintf(result,"VST1 (multiple single elements) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==3 || B0>>1==4){
            fprintf(result,"VST2 (multiple 2-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0>>1==2){
            fprintf(result,"VST3 (multiple 3-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0>>1==0){
            fprintf(result,"VST4 (multiple 4-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
    }
    else if(A==0 &&L==1){
        if(B0==2 || B0>>1==3 || B0==0xA){
            fprintf(result,"VLD1 (multiple single elements) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==3 || B0>>1==4){
            fprintf(result,"VLD2 (multiple 2-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0>>1==2){
            fprintf(result,"VLD3 (multiple 3-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0>>1==0){
            fprintf(result,"VLD4 (multiple 4-element structures) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
    }
    else if(A==1 &&L==0){
        if(B0&0xB==0 || B0==8){
            fprintf(result,"VST1 (single element from one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==1 || B0==9){
            fprintf(result,"VST2 (single 2-element structure from one lane)\n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==2 || B0==0xA){
            fprintf(result,"VST3 (single 3-element structure from one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==3 || B0==0xB){
            fprintf(result,"VST4 (single 4-element structure from one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
    }
    else if(A==1 &&L==1){
        if(B0&0xB==0 || B0==8){
            fprintf(result,"VLD1 (single element to one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==0xC){
            fprintf(result,"VLD1 (single element to all lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==1 || B0==9){
            fprintf(result,"VLD2 (single 2-element structure to one lane)\n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==0xD){
            fprintf(result,"VLD2 (single 2-element structure to all lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==2 || B0==0xA){
            fprintf(result,"VLD3 (single 3-element structure to one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==0xE){
            fprintf(result,"VLD3 (single 3-element structure to all lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0&0xB==3 || B0==0xB){
            fprintf(result,"VLD4 (single 4-element structure to one lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
        else if(B0==0xF){
            fprintf(result,"VLD4 (single 4-element structure to all lane) \n");
            i_code.vartype = 2;
            regimmVal(insn, i_code);
        }
    }
    fprintf(result,"error\n");

}
void table_33(uint32_t insn){//Memory hints, Advanced SIMD instructions, and miscellaneous instructions
    uint8_t op1 = (insn>>20)&0x7F;
    uint8_t Rn = (insn>>16)&0xF;
    uint8_t op2 = (insn>>4)&0xF;
    if(op1==0x10 && op2&0x2==0 && Rn&1==0){
        fprintf(result,"CPS (ARM)\n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else if(op1==0x10 && op2==0 && Rn&1==1){
        fprintf(result,"SETEND\n");
			cycle_cnt +=1;
        i_code.affect==0;
    }
    else if(op1>>5==1)table_34(insn);//Advanced SIMD data-processing instructions
    else if(op1&0x71==0x40)table_35(insn);//Advanced SIMD element or structure load/store instructions
    else if(op1&0x77==0x41){
        fprintf(result,"unallocated (NOP)\n");
        i_code.affect==0;
    }
    else if(op1&0x77==0x45){
        fprintf(result,"PLI imm literal\n");
			cycle_cnt +=1;
			//cycle_cnt +=120; // literal as branch
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op1&0x73==0x43){
        fprintf(result,"unpredictable\n");
    }
    else if(op1&0x77==0x51 && Rn!=0xF){
        fprintf(result,"PLD PLDW imm\n");
			cycle_cnt +=1;
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op1&0x77==0x51 && Rn==0xF){
        fprintf(result,"unpredictable\n");
    }
    else if(op1&0x77==0x55 && Rn!=0xF){
        fprintf(result,"PLD PLDW imm\n");
			cycle_cnt +=1;
        i_code.vartype = 21;
        regimmVal(insn, i_code);
    }
    else if(op1&0x77==0x55 && Rn==0xF){
        fprintf(result,"PLD literal\n");
        i_code.affect==0;
			//cycle_cnt +=120; // literal as branch
			cycle_cnt +=1;
    }
    else if(op1==0x53){
        fprintf(result,"unpredictable\n");
    }
    else if(op1==0x57){
        if(op2==0 || op2>>1==1 || op2==7 || op2>>3==1){
            fprintf(result,"unpredictable\n");
        }
        else if(op2==1){
            fprintf(result,"CLREX\n");
            i_code.affect==0;
			cycle_cnt +=1;
        }
        else if(op2==4){
            fprintf(result,"DSB\n");
            i_code.affect==0;
			cycle_cnt +=1;
        }
        else if(op2==5){
            fprintf(result,"DMB\n");
            i_code.affect==0;
			cycle_cnt +=1;
        }
        else if(op2==6){
            fprintf(result,"ISB\n");
            i_code.affect==0;
			cycle_cnt +=1;
        }
    }
    else if(op1&0x7B==0x5B){
        fprintf(result,"unpredictable\n");
    }
    else if(op1&0x77==0x61 && op2&1==0){
        fprintf(result,"unallocated (NOP)\n");
			cycle_cnt +=1;
    }
    else if(op1&0x77==0x65 && op2&1==0){
        fprintf(result,"PLI reg\n");
        i_code.vartype = 2;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1&0x77==0x71 && op2&1==0){
        fprintf(result,"PLD PLDW reg\n");
        i_code.vartype = 2;
        regimmVal(insn, i_code);
    }
    else if(op1&0x77==0x75 && op2&1==0){
        fprintf(result,"PLD PLDW reg\n");
        i_code.vartype = 2;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1&0x77==0x63 && op2&1==0){
        fprintf(result,"unpredictable\n");
    }
    else if(op1==0x7F && op2==0xF){
        fprintf(result,"permanently undefined\n");
    }
    else fprintf(result,"error\n");

}
void table_06(uint32_t insn){//Unconditional instructions
    uint8_t op1 = (insn>>20)&0xFF;
    uint8_t Rn = (insn>>16)&0xF;
    uint8_t op = (insn>>4)&1;
    if(op1>>7==0)table_33(insn);//Memory hints, Advanced SIMD instructions, and miscellaneous instructions
    else if(op1&0xE5==0x84){
        fprintf(result,"SRS (ARM)\n");
        i_code.affect==0;
			cycle_cnt +=1;
    }
    else if(op1&0xE5==0x81){
        fprintf(result,"RFE\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1>>5==0x5){
        fprintf(result,"BL, BLX (immediate) \n");
        i_code.affect==0;
			cycle_cnt +=1;
    }
    else if(op1&0xE1==0xC0 && op1&0xFB==0xC0){
        fprintf(result,"STC, STC2\n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1&0xE1==0xC1 && op1&0xFB==0xC1 && Rn!=0xF){
        fprintf(result,"LDC, LDC2 (immediate) \n");
        i_code.vartype = 21;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1&0xE1==0xC1 && op1&0xFB==0xC1 && Rn==0xF){
        fprintf(result,"LDC, LDC2 (literal)\n");
        i_code.affect==0;
			cycle_cnt +=1;
			//cycle_cnt +=120; // literal as branch
    }
    else if(op1==0xC4){
        fprintf(result,"MCRR, MCRR2\n");
        i_code.vartype = 25;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1==0xC5){
        fprintf(result,"MRRC, MRRC2 \n");
        i_code.vartype = 25;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1>>4==0xE){
        fprintf(result,"CDP, CDP2 \n");
        i_code.affect==0;
			cycle_cnt +=1;
    }
    else if(op1&0xF1==0xE0){
        fprintf(result,"MCR, MCR2\n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else if(op1&0xF1==0xE1){
        fprintf(result,"MRC, MRC2 \n");
        i_code.vartype = 19;
        regimmVal(insn, i_code);
			cycle_cnt +=1;
    }
    else fprintf(result,"error\n");
}
void checkinsn(uint32_t insn){
	uint8_t cond = (insn>>28)&0xF;
	uint8_t op1 = (insn>>25)&0x7;
	uint8_t op = (insn>>4)&0x1;
	if(cond!=0xF){
		if(op1>>1 == 0)table_00(insn);//Data-processing and miscellaneous instructions
		else if(op1==0x2)table_01(insn);//Load/store word and unsigned byte
		else if(op1==0x3 && op==0)table_02(insn);//Load/store word and unsigned byte
		else if(op1==0x3 && op==1)table_03(insn);//Media instructions
		else if(op1>>1==0x2)table_04(insn);//Branch, branch with link, and block data transfer
		else if(op1>>1==0x3)table_05(insn);//Coprocessor instructions, and Supervisor Call
	}
	else if(cond==0xF)table_06(insn);//Unconditional instructions
}
int readinsn(){
	uint32_t insn;
	strcpy(buff,"");
	//fgets(buff,sizeof(buff),fptr);
#if 1
#ifdef QEMU_TEST
    //fprintf(result,"%s\n",buff);
    //while(strcmp(buff,"branch taken\n")==0||strcmp(buff,"branch not taken\n")==0){
    while(strncmp(buff,"branch taken",12)==0||strncmp(buff,"branch not taken",16)==0){
        if(strncmp(buff,"branch taken",12)==0){
            cycle_cnt += 2;
            fprintf(result,"branch taken\n");
            fprintf(result,"Cycle: %d\n",cycle_cnt);
	    fprintf(result,"===================\n");
        }
        if(strncmp(buff,"branch not taken",16)==0){
            fprintf(result,"branch not taken\n");
        }
        strcpy(buff,"");
        //fgets(buff,sizeof(buff),fptr);
    }
    sscanf(buff,"%x",&insn);
    fprintf(result,"op = %X ",insn);
    insn_op1 = insn;
#else
    sscanf(buff,"%x,%x\n",&tmp,&insn);
    fprintf(result,"opcode = %X ",insn);

    insn_op1 = insn;
    //fprintf(result,"op1 = %X ",insn_op1);
#endif
#endif // 1
    return 1;
}
void oneloop(int n){
	int i;
	for(i=0;i<16;i++){
	     if(n==1 && reg_record[i]>0)reg_record[i]--;
	     fprintf(result, "r[%d] = %d ",i,reg_record[i]);
	}
	fprintf(result,"\n");
}

int read_table(uint32_t insn){
	//uint32_t insn = 0xe8bd8810;
	//i_code.affect=1;
	//fptr = fopen("Trace.csv","r");
	int temp=0;
	i_code.affect = 1;
//#ifdef QEMU_TEST
	//fptr = fopen(filename,"r");
	//result = fopen("test2","w");
//#else
	//result = fopen("result2","w");
//#endif
	/*if(!fptr){
		printf("failed to open file\n");
		exit(1);
	}*/
	//fprintf(result,"=========================\n");
	//while(!feof(fptr)){
		//readinsn();
		//insn = insn_op1;
		fprintf(result,"insn = %x\n",insn);
		checkinsn(insn);
		/*if(i_code.affect == 1 && checkLast(i_code_t)){
        		//printf("type = %d\n",i_code.vartype);
        		fprintf(result,"type = %d\n",i_code.vartype);
        		regs(i_code, i_code_t);
		}*/
#if 1
		oneloop(0);
		if(i_code.affect == 1){
			fprintf(result,"affect\n");
        		//printf("type = %d\n",i_code.vartype);
        		fprintf(result,"type = %d\n",i_code.vartype);
        		regs(i_code, i_code_t);
		}
		if(in_table>3)oneloop(1);
		else in_table++;
		varVal(i_code.vartype,i_code);
#endif
		i_code_t = i_code;
		fprintf(result,"type = %d\n",i_code.vartype);
            	fprintf(result,"Cycle: %d\n",cycle_cnt-cycle_last);
	        temp = cycle_cnt - cycle_last;
		/*if((++insn_cnt % 8) ==0){
			temp+=36;
			insn_cnt = 0;
		}*/
		//if(insn_cnt%2==0)temp+=1;
		cycle_last = cycle_cnt;
		fprintf(result,"=========================\n");
		//printf(" %d\n",temp);
		
	//}
	return temp;
}
