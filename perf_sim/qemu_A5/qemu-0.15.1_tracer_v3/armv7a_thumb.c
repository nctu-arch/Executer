//armv7m  cortex-m4
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "insn_thumb_code.h"
//uint8_t i_code_thumb.affect = 1;
uint16_t tmp,insn_thumb_op1=0,insn_thumb_op2=0;
//int code = -1;
struct dep i_code_thumb;
struct dep i_code_thumb_t;
FILE* fptr;
int long count=0;
char insn_name[5];
uint16_t imm_thumb=-1,Rm_thumb=-1,Rd_thumb=-1,Rn_thumb=-1,Rt_thumb=-1;
uint16_t Ra_thumb=-1, Rt2_thumb=-1, Rlist_thumb=-1;
uint16_t RdLo_thumb = -1, RdHi_thumb = -1;
unsigned long long cycle_cnt_thumb = 0, cycle_last_thumb = 0;
char buff[50];
int  optype;
//FILE * name;
extern FILE* result;
//FILE* insn_list;
static int final_cnt = 0;
static int insn_cnt=0;
extern int in_table;
extern uint8_t reg_record[300];
void reg_init(){
	imm_thumb=-1;Rm_thumb=-1;Rd_thumb=-1;Rn_thumb=-1;Rt_thumb=-1;
	Ra_thumb=-1; Rt2_thumb=-1; Rlist_thumb=-1;RdLo_thumb = -1; RdHi_thumb = -1;
}
void regimmVal_thumb(int type, uint16_t insn){
    reg_init();
    fprintf(result,"in regimmVal_thumb insn = %X\n", insn);
    uint8_t DN = 0;
    switch(type){
    case 1://imm5 Rm3 Rd3
        Rd_thumb = insn&0x7;
        Rm_thumb = (insn>>3)&0x7;
        imm_thumb = (insn>>6)&0x1f;
        break;
    case 2: //imm8 Rd3
        Rd_thumb = (insn>>8)&0x7;
        imm_thumb = insn&0xff;
        break;
    case 3: //Rm3 Rn3 Rd3
        Rd_thumb = insn&0x7;
        Rn_thumb = (insn>>3)&0x7;
        Rm_thumb = (insn>>6)&0x7;
        break;
    case 4: //imm3 Rn3 Rd3
        Rd_thumb = insn&0x7;
        Rn_thumb = (insn>>3)&0x7;
        imm_thumb = (insn>>6)&0x7;
        break;
    case 5: //imm8 Rn3
        Rn_thumb = (insn>>8)&0x7;
        imm_thumb = insn&0xff;
        break;
    case 6: //imm8 Rdn3
        Rn_thumb = (insn>>8)&0x7;
        Rd_thumb = Rn_thumb;
        imm_thumb = insn&0xff;
        break;
    case 7: //Rm3 Rdn
        Rm_thumb = (insn>>3)&0x7;
        Rd_thumb = insn&0x7;
        Rn_thumb = Rd_thumb;
        break;
    case 8: //Rm3 Rn3
        Rm_thumb = (insn>>3)&0x7;
        Rn_thumb = insn&0x7;
        break;
    case 9: //Rn3 Rd3
        Rn_thumb = (insn>>3)&0x7;
        Rd_thumb = insn&0x7;
        break;
    case 10: //Rn3 Rdm3
        Rn_thumb = (insn>>3)&0x7;
        Rd_thumb = insn&0x7;
        Rm_thumb = Rd_thumb;
        break;
    case 11: //Rm3 Rd3
        Rm_thumb = (insn>>3)&0x7;
        Rd_thumb = insn&0x7;
        break;
    case 12: //Rm4 Rdn3
    	DN = (insn>>7)&1;
        Rm_thumb = (insn>>3)&0xf;
        Rd_thumb = (DN<<3)|insn&0x7;
        Rn_thumb = Rd_thumb;
        break;
    case 13: //Rm4 Rn3
        Rm_thumb = (insn>>3)&0xf;
        Rn_thumb = insn&0x7;
        break;
    case 14: //Rm4 Rd3
        Rm_thumb = (insn>>3)&0xf;
        Rd_thumb = insn&0x7;
        break;
    case 15: //Rm4 000
        Rm_thumb = (insn>>3)&0xf;
        break;
    case 16: //imm5 Rn3 Rt3
        imm_thumb = (insn>>6)&0x1f;
        Rn_thumb = (insn>>3)&0x7;
        Rt_thumb = insn&0x7;
        break;
    case 17: //Rm3 Rn3 Rt3
        Rt_thumb = insn&0x7;
        Rn_thumb = (insn>>3)&0x7;
        Rm_thumb = (insn>>6)&0x7;
        break;
    case 18: //Rt3 imm8
        Rt_thumb = (insn>>8)&0x7;
        imm_thumb = insn&0xff;
        break;
    case 19: //imm5 Rn3
        Rn_thumb = insn&0x7;
        imm_thumb = (insn>>3)&0x1f;
        break;
    case 20: //imm7
        imm_thumb = insn&0x7f;
        break;
    case 21: //Rd3 imm8
        Rd_thumb = (insn>>8)&0x7;
        imm_thumb = insn&0xff;
        break;

    }
    //if(Rt_thumb==0xF)cycle_cnt_thumb += 30;
    //if(Rd_thumb>=0 && Rd_thumb!=0xFFFF)reg_thumb_record[Rd_thumb]++;
    checkdependency(imm_thumb, Rm_thumb, Rd_thumb, Rn_thumb, Rt_thumb);
}
void thumb_table_5(uint16_t insn_p){
	uint16_t op = insn_p>>9;
	switch(op){
	    case 0x0: //00 01100
	    	fprintf(result,"ADD reg\n");
			i_code_thumb.vartype = 3;
			//printf("insn_p %u\n",insn_p);
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 0;
			cycle_cnt_thumb +=1;
            break;
	    case 0x1: //00 01101
	    	fprintf(result,"SUB reg\n");
			i_code_thumb.vartype = 3;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 1;
			cycle_cnt_thumb +=1;
            break;
	    case 0x2: //00 01110
	    	fprintf(result,"ADD 3imm\n");
			i_code_thumb.vartype = 4;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 2;
			cycle_cnt_thumb +=1;
            break;
	    case 0x3: //00 01111
	        fprintf(result,"SUB 3imm\n");
			i_code_thumb.vartype = 4;
	        regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 3;
			cycle_cnt_thumb +=1;
            break;
	}
}

void thumb_table_1(uint16_t insn_p){
	uint16_t op=insn_p>>9;
	switch(op>>2){
	    case 0x0: //00 000xx
	    	fprintf(result,"LSL imm\n");//v
			i_code_thumb.vartype = 1;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 4;
			cycle_cnt_thumb +=1;
            break;
	    case 0x1: //00 001xx
	    	fprintf(result,"LSR imm\n");//v
			i_code_thumb.vartype = 1;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 5;
			cycle_cnt_thumb +=1;
            break;
	    case 0x2: //00 010xx
	    	fprintf(result,"ASR imm\n");//v
			i_code_thumb.vartype = 1;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 6;
			cycle_cnt_thumb +=1;
            break;
	    case 0x3: //00 011xx
	        thumb_table_5(insn_p&0x7ff);

            break;
	    case 0x4: //00 100xx
	        fprintf(result,"MOV imm\n");//v
			i_code_thumb.vartype = 2;
	        regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 7;
			cycle_cnt_thumb +=1;
            break;
	    case 0x5: //00 101xx
	        fprintf(result,"CMP imm\n");//v
			i_code_thumb.vartype = 5;
	        regimmVal_thumb(i_code_thumb.vartype,insn_p);//
			i_code_thumb.code = 8;
			cycle_cnt_thumb +=1;
            break;
	    case 0x6: //00 110xx
	        fprintf(result,"ADD 8imm\n");//v
			i_code_thumb.vartype = 6;
	        regimmVal_thumb(i_code_thumb.vartype,insn_p);//Rn_thumb=Rd_thumb
			i_code_thumb.code = 9;
			cycle_cnt_thumb +=1;
            break;
	    case 0x7: //00 111xx
	        fprintf(result,"SUB 8imm\n");//v
			i_code_thumb.vartype = 6;
	        regimmVal_thumb(i_code_thumb.vartype,insn_p);//Rn_thumb=Rd_thumb
			i_code_thumb.code = 10;
			cycle_cnt_thumb +=1;
            break;
	    default:
	    	fprintf(result,"error\n");
		break;
	}
}
void thumb_table_9(uint16_t insn_p){ //01 0000
	uint16_t op = insn_p>>6;

	//printf("%x %x\n",insn_p,op);
	switch(op){
	    case 0x00:
	    	fprintf(result,"AND reg\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 11;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x01:
	    	fprintf(result,"EOR\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 12;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x02:
	    	fprintf(result,"LSL\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 13;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x03:
	    	fprintf(result,"LSR\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 14;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x04:
	    	fprintf(result,"ASR\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 15;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x05:
	    	fprintf(result,"ADC\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 16;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x06:
	    	fprintf(result,"SBC\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 17;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x07:
	    	fprintf(result,"ROR\n");
			i_code_thumb.vartype= 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 18;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x08:
	    	fprintf(result,"TST\n");
			i_code_thumb.vartype = 8;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 19;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x09:
	    	fprintf(result,"RSB imm\n");
			i_code_thumb.vartype = 9;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 20;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0a:
	    	fprintf(result,"CMP reg\n");
			i_code_thumb.vartype = 8;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 21;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0b:
	    	fprintf(result,"CMN\n");
			i_code_thumb.vartype = 8;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 22;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0c:
	    	fprintf(result,"ORR\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 23;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0d:
	    	fprintf(result,"MUL\n");
			i_code_thumb.vartype = 10;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 24;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0e:
	    	fprintf(result,"BIC\n");
			i_code_thumb.vartype = 7;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 25;
			cycle_cnt_thumb +=1;
	    	break;
	    case 0x0f:
	    	fprintf(result,"MVN\n");
			i_code_thumb.vartype = 11;
	    	regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 26;
			cycle_cnt_thumb +=1;
	    	break;
	    default:
	    	fprintf(result,"errrr");
	    	break;

	}

}
void thumb_table_7(uint16_t insn_p){ //01 001x

    fprintf(result,"LDR\n");
    uint8_t Rt_thumb=(insn_p>>8)&0x7;
    uint8_t imm = insn_p&0xff;
	i_code_thumb.code = 27;
	cycle_cnt_thumb += 2;//ldr 1
//	uint16_t op =
}
void thumb_table_8(uint16_t insn_p){ //01 0001
	uint16_t op = insn_p>>6;
	if(op >> 2 == 0x0){
            fprintf(result,"ADD reg\n");
			i_code_thumb.vartype = 12;
			//printf("type %d, insn_p %u\n",i_code_thumb.vartype,insn_p);
            regimmVal_thumb(i_code_thumb.vartype,insn_p);
			i_code_thumb.code = 28;
			cycle_cnt_thumb +=1;
	}
	else if(op >> 2 == 0x1 && op != 0x4){
		fprintf(result,"CMP reg\n");
		i_code_thumb.vartype = 13;
		regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 29;
		cycle_cnt_thumb +=1;
	}
	else if(op >>2 == 0x2){
        fprintf(result,"MOV reg\n");
		i_code_thumb.vartype = 14;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 30;
		cycle_cnt_thumb +=1;
	}
	else if(op >> 1 ==0x6 ){
        fprintf(result,"BX\n");
		i_code_thumb.vartype = 15;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 31;
		cycle_cnt_thumb +=1; //bx 3
	}
	else if(op >> 1 == 0x7){
        fprintf(result,"BLX reg\n");
		i_code_thumb.vartype = 15;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 32;
		cycle_cnt_thumb +=1;//blx 3
	}
	else fprintf(result,"unpredicthumb_table\n");
}
void thumb_table_6(uint16_t insn_p){//01 1xxx
	uint16_t op = insn_p >> 11;
	if(op == 0x4){
		fprintf(result,"STR imm\n"); //0110 0xx
		i_code_thumb.code = 33;
		cycle_cnt_thumb +=2;
	}
	else if(op == 0x5){
		fprintf(result,"LDR imm\n");//0110 1xx
		i_code_thumb.code = 34;
		cycle_cnt_thumb +=2;
		//cycle_cnt_thumb +=10;
	}
	else if(op == 0x6){
		fprintf(result,"STRB imm\n");//0111 0xx
		i_code_thumb.code = 35;
		cycle_cnt_thumb +=2;
	}
	else if(op == 0x7){
		fprintf(result,"LDRB imm\n");//0111 1xx
		i_code_thumb.code = 36;
		cycle_cnt_thumb +=2;
	}
	i_code_thumb.vartype = 16;
	regimmVal_thumb(i_code_thumb.vartype,insn_p);
}
void thumb_table_10(uint16_t insn_p){//01 01xx
	uint16_t op = insn_p >> 9;
	switch(op){
	    case 0x0: //0101 000
	    	fprintf(result,"STR reg\n");
			i_code_thumb.code = 37;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x1: //0101 001
	    	fprintf(result,"STRH reg\n");
			i_code_thumb.code = 38;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x2: //0101 010
	    	fprintf(result,"STRB reg\n");
			i_code_thumb.code = 39;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x3: //0101 011
	    	fprintf(result,"LDRSB reg\n");
			i_code_thumb.code = 40;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x4: //0101 100
	    	fprintf(result,"LDR reg\n");
			i_code_thumb.code = 41;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x5: //0101 101
	    	fprintf(result,"LDRH reg\n");
			i_code_thumb.code = 42;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x6: //0101 110
	    	fprintf(result,"LDRB reg\n");
			i_code_thumb.code = 43;
			cycle_cnt_thumb +=2;
	    	break;
	    case 0x7: //0101 111
	    	fprintf(result,"LDRSH reg\n");
			i_code_thumb.code = 44;
			cycle_cnt_thumb +=2;
	    	break;
	    default:
	    	fprintf(result,"eeeeeee\n");
		break;
	}
	i_code_thumb.vartype = 17;
	regimmVal_thumb(i_code_thumb.vartype,insn_p);
}
void thumb_table_2(uint16_t insn_p){ //01 xxxx
	uint16_t op = insn_p>>10;
	//op_type optype;
	if(op>>3==0x1){ //01 1xxx
	    thumb_table_6(insn_p);
	}
	else if(op >> 2 == 0x1){ //01 01xx
	    thumb_table_10(insn_p & 0xfff);
	}
	else if(op>>1 == 0x1){ // 01 001x
	    thumb_table_7(insn_p&0x7ff);
	}
	else if(op == 0x1){ // 01 0001
	    thumb_table_8(insn_p&0x3ff);
	}
	else{ //01 0000
	    thumb_table_9(insn_p&0x3ff);
	}
}
void thumb_table_11(uint16_t insn_p){
	uint16_t op = insn_p >>11;
	if(op == 0x0 ){
        fprintf(result,"STRH imm\n");//1000 0xx
		i_code_thumb.vartype = 16;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 45;
		cycle_cnt_thumb +=2;
	}
	else if(op == 0x1){
        fprintf(result,"LDRH imm\n");//1000 1xx
		i_code_thumb.vartype = 16;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 46;
		cycle_cnt_thumb +=2;
	}
	else if(op == 0x2){
        fprintf(result,"STR sp imm\n");//1001 0xx
		i_code_thumb.vartype = 18;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 47;
		cycle_cnt_thumb +=2;
	}
	else if(op == 0x3){
        fprintf(result,"LDR sp imm\n");//1001 1xx
        i_code_thumb.vartype = 18;
		regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 48;
		cycle_cnt_thumb +=2;
	}
	else fprintf(result,"error\n");
}

void thumb_table_21(uint16_t insn_p){

    uint16_t opA = insn_p>>4;

    uint16_t opB = insn_p&0xf;

    if(opB!=0x0){
		fprintf(result,"IT\n");
		i_code_thumb.code = 49;
		cycle_cnt_thumb +=1;
	}

    else if(opA == 0x0){
		fprintf(result,"NOP\n");
		i_code_thumb.code = 50;
		cycle_cnt_thumb +=1;
	}

    else if(opA == 0x1){
		fprintf(result,"YIELD\n");
		i_code_thumb.code = 51;
		cycle_cnt_thumb +=1;
	}

    else if(opA == 0x2){
		fprintf(result,"WFE\n");
		cycle_cnt_thumb +=2;
		i_code_thumb.code = 52;
	}

    else if(opA == 0x3){
		fprintf(result,"WFI\n");
		i_code_thumb.code = 53;
		cycle_cnt_thumb +=2;
	}

    else if(opA == 0x4){
		fprintf(result,"SEV\n");
		i_code_thumb.code = 54;
		cycle_cnt_thumb +=1;
	}
    i_code_thumb.affect = 0;
}

void thumb_table_15(uint16_t insn_p){//1011 000xxxx

    uint16_t op = insn_p >> 8;

    if(op == 0x1){
        fprintf(result,"CBNZ CBZ (cbz)1\n");//1011 0001xxx
        i_code_thumb.vartype = 19;
		regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 55;
		cycle_cnt_thumb +=30;//1;
    }

    else if(insn_p>>7 == 0x1){
        fprintf(result,"SUB sp\n");//1011 00001xx
        i_code_thumb.affect = 0;
		i_code_thumb.vartype = 20;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 56;
		cycle_cnt_thumb +=1;
    }

    else{
        fprintf(result,"ADD sp\n");//1011 00000xx
        i_code_thumb.affect = 0;
		i_code_thumb.vartype = 20;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 57;
		cycle_cnt_thumb +=1;
    }

}

void thumb_table_16(uint16_t insn_p){//1011 001xxxx

    uint16_t op = insn_p >> 6;

    uint16_t op2 = insn_p >> 8;

    if(op2 == 0x3){
        fprintf(result,"CBNZ CBZ (cbz)2\n");//1011 0011xxx
        i_code_thumb.vartype = 19;
		regimmVal_thumb(19,insn_p);
		i_code_thumb.code = 58;
		cycle_cnt_thumb +=30;//1;
    }

    else if(op == 0x8){
        fprintf(result,"SXTH\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 59;
		cycle_cnt_thumb +=1;
    }

    else if(op == 0x9){
        fprintf(result,"SXTB\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 60;
		cycle_cnt_thumb +=1;
    }

    else if(op == 0xa){
        fprintf(result,"UXTH\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 61;
		cycle_cnt_thumb +=1;
    }

    else if(op == 0xb){
        fprintf(result,"UXTB\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 62;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"not know\n");



}

void thumb_table_17(uint16_t insn_p){//1011 010xxxx

    fprintf(result,"PUSH\n");
    uint8_t Rlist_thumb = insn_p&0xff;
    uint8_t M = (insn_p>>8)&1;
    if(M)cycle_cnt_thumb+=1;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
	i_code_thumb.code = 63;
	cycle_cnt_thumb +=1;

}

void thumb_table_18(uint16_t insn_p){//1011 1001xxx

    fprintf(result,"CBZ CBNZ (cbnz)3\n");
	i_code_thumb.vartype = 19;
    regimmVal_thumb(i_code_thumb.vartype,insn_p);
	i_code_thumb.code = 64;
	cycle_cnt_thumb +=30;//1;

}

void thumb_table_19(uint16_t insn_p){//1011 101xxxx

    uint16_t op = insn_p >> 8;

    uint16_t op2 = insn_p >> 6;

    if(op == 0xb){
        fprintf(result,"CBNZ CBZ(cbnz) 4\n");//1011 1011xxx
        i_code_thumb.vartype = 19;
		regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 65;
		cycle_cnt_thumb +=30;//1;
    }

    else if(op2 == 0x2b){
        fprintf(result,"REVSH\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 66;
		cycle_cnt_thumb +=1;
    }

    else if(op2 == 0x29){
        fprintf(result,"REV16\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 67;
		cycle_cnt_thumb +=1;
    }

    else if(op2 == 0x28){
        fprintf(result,"REV\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb(i_code_thumb.vartype,insn_p);
		i_code_thumb.code = 68;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"not know\n");



}

void thumb_table_20(uint16_t insn_p){//1011 110xxxx

    fprintf(result,"POP\n");
    uint8_t Rlist_thumb = insn_p & 0xff;
    uint8_t P = (insn_p>>8)&1;
    if(P)cycle_cnt_thumb+=3;//3
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
	i_code_thumb.code = 69;
	cycle_cnt_thumb +=1;

}
void thumb_table_12(uint16_t insn_p){ //1011 miscellaneous
	uint16_t op = insn_p >> 5;

	uint16_t op2 = insn_p >>9;
	if(op==0x33){
        fprintf(result,"CPS");//1011 0110011
        i_code_thumb.affect = 0;
		i_code_thumb.code = 70;
		cycle_cnt_thumb +=1;
	}
	else if(op2 == 0x0) thumb_table_15(insn_p);//1011 000xxxx
    else if(op2 == 0x1) thumb_table_16(insn_p);//1011 001xxxx
    else if(op2 == 0x2) thumb_table_17(insn_p);//1011 010xxxx
    else if(op2 == 0x4) thumb_table_18(insn_p);//1011 1001xxx
    else if(op2 == 0x5) thumb_table_19(insn_p);//1011 101xxxx
    else if(op2 == 0x6) thumb_table_20(insn_p);//1011 110xxxx
    else if(insn_p >> 8 == 0xe){
        fprintf(result,"BRKT\n");//1011 1110xxx
        i_code_thumb.affect = 0;
		i_code_thumb.code = 71;
		cycle_cnt_thumb +=1;
    }
    else if(insn_p >> 8 == 0xf) thumb_table_21(insn_p & 0xff);
}

void thumb_table_13(uint16_t insn_p){

    fprintf(result,"ADR\n");
	i_code_thumb.vartype = 21;
    regimmVal_thumb(i_code_thumb.vartype,insn_p);
	i_code_thumb.code = 72;
	cycle_cnt_thumb +=1;//pc

}

void thumb_table_14(uint16_t insn_p){

    fprintf(result,"ADD sp\n");
	i_code_thumb.vartype = 21;
    regimmVal_thumb(i_code_thumb.vartype,insn_p);
	i_code_thumb.code = 73;
	cycle_cnt_thumb +=1;
}
void thumb_table_3(uint16_t insn_p){ // 10 xxxx
	uint16_t op = insn_p >> 13;// 1 bit

	if(op == 0)thumb_table_11(insn_p); // 10 0xxx
	else if(insn_p>>12 == 0x3)thumb_table_12(insn_p&0xfff);// 10 11xx
	else if(insn_p >> 11 == 0x4)thumb_table_13(insn_p);// 10 100x
	else if(insn_p >> 11 == 0x5)thumb_table_14(insn_p);// 10 101x


}

void thumb_table_22(uint16_t insn_p){

    uint16_t op = insn_p >> 8 ;

    if(op==0xe)fprintf(result,"permanently undefined\n");

    else if(op == 0xf){
        fprintf(result,"SVC(formerly SWI)\n");
        i_code_thumb.affect = 0;
		i_code_thumb.code = 74;
		cycle_cnt_thumb +=1;
    }
    else{
        fprintf(result,"B\n");
        i_code_thumb.affect = 0;
		i_code_thumb.code = 75;
		cycle_cnt_thumb +=1;
    }
}
void thumb_table_4(uint16_t insn_p){//11 xxxx

    uint16_t op = insn_p >> 11;
    uint8_t Rn_thumb = 0, Rlist_thumb = 0;
    if(op == 0x0){
       fprintf(result,"STM\n");//11000x
       Rn_thumb = (insn_p>>8)&0x7;
       Rlist_thumb = insn_p&0xff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
	   i_code_thumb.code = 76;
	   cycle_cnt_thumb +=1;
    }
    else if(op == 0x1){
       fprintf(result,"LDM\n");//11001x
       Rn_thumb = (insn_p>>8)&0x7;
       Rlist_thumb = insn_p&0xff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
	   i_code_thumb.code = 77;
	   cycle_cnt_thumb +=1;
    }
    else if(op == 0x4){
       fprintf(result,"B\n");//11100x
       i_code_thumb.affect = 0;
	   i_code_thumb.code = 78;
	   cycle_cnt_thumb +=1;
   }

    else if(insn_p>>12==0x1)thumb_table_22(insn_p&0xfff);//1101xx

}
void regimmVal_thumb32(int type, uint16_t insn_hw1, uint32_t insn){
    reg_init();
    fprintf(result,"in regimmVal_thumb32 insn = %X\n", insn);
    switch(type){
    case 1:
	case 100:
        Rn_thumb = insn_hw1&0xf;
        Rlist_thumb = insn&0x1fff;
        break;
    case 2: //pop push
        Rt_thumb = (insn>>12)&0xf;
	if(Rt_thumb==0xF)cycle_cnt_thumb += 30;
        break;
    case 3: //Rn4 Rt4 Rd4 imm8
        Rn_thumb = insn_hw1 & 0xf;
        Rt_thumb = (insn>>12)&0xf;
        Rd_thumb = (insn>>8)&0xf;
        imm_thumb = insn&0xff;
        break;
    case 4: //Rn4 Rt4 1111 imm8
        Rn_thumb = insn_hw1 &0xf;
        Rt_thumb = (insn>>12)&0xf;
        imm_thumb = insn & 0xff;
        break;
    case 5: //Rn4 Rt4 Rt24 imm8
        Rn_thumb = insn_hw1&0xf;
        Rt_thumb = (insn>>12)&0xf;
        Rt2_thumb = (insn>>8)&0xf;
        imm_thumb = insn & 0xff;
        break;
    case 6: //Rt4 Rt24 imm8
        Rt_thumb = (insn>>12)&0xf;
        Rt2_thumb = (insn>>8)&0xf;
        imm_thumb = insn&0xff;
        break;
    case 7:  //Rn4 Rt4 1111 xxxx Rd4
        Rn_thumb = insn_hw1 & 0xf;
        Rt_thumb = (insn>>12)&0xf;
        Rd_thumb = insn&0xf;
        break;
    case 8: //Rn4 xxxx xxxx xxxx Rm4
        Rn_thumb = insn_hw1 & 0xf;
        Rm_thumb = insn&0xf;
        break;
    case 9: // Rn4 Rt4
        Rn_thumb = insn_hw1 &0xf;
        Rt_thumb = (insn>>12)&0xf;
        break;
    case 10:
        Rn_thumb = insn_hw1 & 0xf;
        Rd_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        imm_thumb = (insn>>6)&0x3;
        imm_thumb|=((insn>>12)&0x7)<<2;
        break;
    case 11:
        Rn_thumb = insn_hw1 & 0xf;
        Rm_thumb = insn&0xf;
        imm_thumb = (insn>>6)&0x3;
        imm_thumb|=((insn>>12)&0x7)<<2;
        break;
    case 12:
        Rd_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        imm_thumb = (insn>>6)&0x3;
        imm_thumb|=((insn>>12)&0x7)<<2;
        break;
    case 13:
        Rd_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        break;
    case 14:
        Rn_thumb = insn_hw1 &0xf;
        imm_thumb = insn&0xff;
        break;
    case 15:
        Rt2_thumb = insn_hw1&0xf;
        Rt_thumb = (insn>>12)&0xf;
        break;
    case 16: //case 2
        Rt_thumb = (insn>>12)&0xf;
        break;
    case 17:
        Rn_thumb = insn_hw1 & 0xf;
        Rd_thumb = (insn>>8)&0xf;
        break;
    case 18:
        Rn_thumb = insn_hw1 &0xf;
        break;
    case 19:
        Rd_thumb = (insn>>8)&0xf;
        break;
    case 20:
        Rn_thumb = insn_hw1&0xf;
        Rt_thumb = (insn>>12)&0xf;
        Rm_thumb = insn&0xf;
        break;
    case 21:
        Rn_thumb = insn_hw1 & 0xf;
        Rd_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        break;
    case 22:
        Rn_thumb = insn_hw1 & 0xf;
        Rd_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        Ra_thumb = (insn>>12)&0xf;
        break;
    case 23:
        Rn_thumb = insn_hw1 & 0xf;
        RdHi_thumb = (insn>>8)&0xf;
        Rm_thumb = insn&0xf;
        RdLo_thumb = (insn>>12)&0xf;
        break;
    }
    //if(Rt_thumb==0xF)cycle_cnt_thumb += 30;
    //if(Rd_thumb>=0 && Rd_thumb!=0xFFFF)reg_thumb_record[Rd_thumb]++;

}
void thumb_table32_4(uint32_t insn, uint16_t insn_hw1){// 111 011 op1
//coprocessor instructions
    char in[6];

    uint16_t op=(insn&0x10)>>4;

    uint16_t op1=(insn_hw1>>4)&0x3f;
    uint16_t Rn = insn_hw1&0xF;
    //itoa(op1,in,2);

    sprintf(in,"%b",op1);
    if(in[0]==0&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        fprintf(result,"STC\n");//111 011 0xxxx0
        i_code_thumb.vartype = 14;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 79;
		cycle_cnt_thumb +=1;
    }

    else if(in[0]==1&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        fprintf(result,"LDC imm\n");//111 011 0xxxx1  Rn_thumb ='1111' literal
        i_code_thumb.vartype = 14;
		if(Rn==0xF)cycle_cnt_thumb += 30;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 80;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x4){
        fprintf(result,"MCRR\n");//111 011 000100
		i_code_thumb.vartype = 15;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 81;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x5){
        fprintf(result,"MRRC\n");//111 011 000101
		i_code_thumb.vartype = 15;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 82;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && op ==0){
        fprintf(result,"CDP\n");//111 011 10xxxx op=0
        i_code_thumb.affect = 0;
		i_code_thumb.code = 83;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && in[0]==0 && op==1){
        fprintf(result,"MCR\n");//111 011 10xxx0 op=1
		i_code_thumb.vartype = 16;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 84;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && in[0]==1 && op==1){
        fprintf(result,"MRC\n");//111 011 10xxx1 op=1
		i_code_thumb.vartype = 16;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 85;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");

}

void thumb_table32_8(uint32_t insn, uint16_t insn_hw1){ //111 0101 0010 x 1111 x imm3 xxxx imm2 type2 xxxx
//move register and immediate shifts
    uint8_t type=(insn>>4)&0x3;

    uint8_t imm3=(insn&0x7000)>>12;

    uint8_t imm2=(insn&0xC0)>>6;

    imm2|=imm3<<2;

    if(type==0x0 && imm2==0x0){
        fprintf(result,"MOV.W reg 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 86;
		cycle_cnt_thumb +=1;
    }

    else if(type == 0x0 && imm2!=0x0){
        fprintf(result,"LSL.W imm 32");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 87;
		cycle_cnt_thumb +=1;
    }

    else if(type == 0x1){
        fprintf(result,"LSR.W imm 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 88;
		cycle_cnt_thumb +=1;
    }

    else if(type == 0x2){
        fprintf(result,"ASR.W imm 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 89;
		cycle_cnt_thumb +=1;
    }

    else if(type == 0x3 && imm2==0x0){
        fprintf(result,"RRX.W 32\n");
        i_code_thumb.vartype = 13;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 90;
		cycle_cnt_thumb +=1;
    }

    else if(type == 0x3 && imm2!=0x0){
        fprintf(result,"ROR.W imm 32\n");
        i_code_thumb.vartype = 13;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 91;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");

}

void thumb_table32_5(uint32_t insn, uint16_t insn_hw1){//111 0101 op
//data processing(shifted register)
    uint16_t op = (insn_hw1&0x1E0)>>5;

    uint16_t Rn_thumb = insn_hw1&0xf;

    uint8_t S = (insn_hw1&0x10)>>4;

    uint8_t Rd_thumb = (insn&0xf00)>>8;

    if(op==0 && Rd_thumb!= 0xf){
        fprintf(result,"AND.W reg 32\n");//111 0101 0000 x
        i_code_thumb.vartype = 10;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 92;
		cycle_cnt_thumb +=1;
    }

    else if(op==0 && Rd_thumb==0xf && S==0){
        fprintf(result,"unpredicthumb_table\n");
    }

    else if(op==0 && Rd_thumb==0xf && S==1){
        fprintf(result,"TST.W reg 32\n");//111 0101 0000 1 Rd_thumb=1111
        i_code_thumb.vartype = 11;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 93;
		cycle_cnt_thumb +=1;
    }

    else if(op==1){
        fprintf(result,"BIC.W reg 32\n");//111 0101 0001
        i_code_thumb.vartype = 10;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 94;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x2 && Rn_thumb!=0xf){
        fprintf(result,"ORR.W reg 32\n");//111 0101 0010 Rn_thumb!=1111
        i_code_thumb.vartype = 10;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 95;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x2 && Rn_thumb==0xf)thumb_table32_8(insn, insn_hw1);//111 0101 0010 Rn_thumb==1111

    else if(op==0x3 && Rn_thumb!=0xf){
        fprintf(result,"ORN.W reg 32\n");//111 0101 0011 Rn_thumb!=1111
        i_code_thumb.vartype = 10;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 96;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x3 && Rn_thumb==0xf){
        fprintf(result,"MVN.W reg 32\n");//111 0101 0011 Rn_thumb==1111
        i_code_thumb.vartype = 12;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 97;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x4 && Rd_thumb!=0xf){
        fprintf(result,"EOR.W reg 32\n");//
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 98;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x4 && Rd_thumb==0xf && S==0){
        fprintf(result,"unpredicthumb_table 32\n");
    }

    else if(op==0x4 && Rd_thumb==0xf && S==1){
        fprintf(result,"TEQ.W reg 32\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 99;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x8 && Rd_thumb!=0xf){
        fprintf(result,"ADD.W reg 32\n");
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 100;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x8 && Rd_thumb==0xf && S==0)fprintf(result,"unpredicthumb_table 32\n");

    else if(op==0x8 && Rd_thumb==0xf && S==1){
        fprintf(result,"CMN.W reg 32\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 101;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xa){
        fprintf(result,"ADC.W reg 32\n");
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 102;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xb){
        fprintf(result,"SBC.W reg 32\n");
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 103;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xd && Rd_thumb!=0xf){
        fprintf(result,"SUB.W reg 32\n");
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 104;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xd && Rd_thumb==0xf && S==0){
        fprintf(result,"unpredicthumb_table\n");
    }

    else if(op==0xd && Rd_thumb==0xf && S==1){
        fprintf(result,"CMP.W reg 32\n");
		i_code_thumb.vartype = 11;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 105;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xe){
        fprintf(result,"RSB.W reg 32\n");
		i_code_thumb.vartype = 10;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 106;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");

}

void thumb_table32_6(uint32_t insn, uint16_t insn_hw1){//111 0100 op 0 W L Rn_thumb
//load/store multiple
    uint8_t op=(insn_hw1>>7)&0x3;

    uint8_t L = (insn_hw1>>4)&0x1;

    uint8_t W = (insn_hw1>>5)&0x1;

    uint8_t Rn_thumb = insn_hw1&0xf;
    uint16_t Rlist_thumb;
    Rn_thumb|=W<<4;

    if(op==0x1&& L==0){
        fprintf(result,"STM.W 32\n");
		i_code_thumb.vartype = 1;
       Rlist_thumb = insn&0x1fff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 107;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x1 && L==1 && Rn_thumb!=0x1D){
        fprintf(result,"LDM.W 32\n");
		i_code_thumb.vartype = 1;
       Rlist_thumb = insn&0x1fff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 108;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x1 && L==1 && Rn_thumb==0x1D){
        fprintf(result,"POP.W 32\n");
		i_code_thumb.vartype = 2;
    uint16_t Rlist_thumb = insn & 0x1fff;
    uint8_t M = (insn>>14)&1;
    uint8_t P = (insn>>15)&1;
    if(M)cycle_cnt_thumb+=1;
    if(P)cycle_cnt_thumb+=3;//3
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 109;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x2 && L==0 && Rn_thumb!=0x1D){
        fprintf(result,"STMDB.W 32\n");
		i_code_thumb.vartype = 1;
       Rlist_thumb = insn&0x1fff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 110;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x2 && L==0 && Rn_thumb==0x1D){
        fprintf(result,"PUSH.W 32\n");
		i_code_thumb.vartype = 2;
    uint16_t Rlist_thumb = insn & 0x1fff;
    uint8_t M = (insn>>14)&1;
    if(M)cycle_cnt_thumb+=1;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 111;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x2 && L==1 && W==1){//Rn_thumb writeback
        fprintf(result,"LDMDB.W 32\n");
		i_code_thumb.vartype = 1;
       Rlist_thumb = insn&0x1fff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 112;
		cycle_cnt_thumb +=1;
    }
	else if(op==0x2 && L==1 && W==0){//Rn_thumb not writeback
		fprintf(result,"LDMDB.W 32\n");
		i_code_thumb.vartype = 100;//@@@@@@
       Rlist_thumb = insn&0x1fff;
    while(Rlist_thumb>0){
	if(Rlist_thumb&1==1)cycle_cnt_thumb+=1;
	Rlist_thumb=Rlist_thumb>>1;
    }
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 112;
		cycle_cnt_thumb +=1;
	}

    else fprintf(result,"error\n");

}

void thumb_table32_7(uint32_t insn, uint16_t insn_hw1){//111 0100 op1 1 op2 Rn_thumb
//load/store dual or exclusive, thumb_table branch
    uint8_t op1 = (insn_hw1>>7)&0x3;

    uint8_t op2 = (insn_hw1>>4)&0x3;

    uint8_t op3 = (insn>>4)&0xf;

    if(op1==0x0 && op2==0x0){
        fprintf(result,"STREX.W 32\n");
		i_code_thumb.vartype = 3;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 113;
		cycle_cnt_thumb +=2;
    }
    else if(op1==0x0 && op2==0x1){
        fprintf(result,"LDREX.W 32\n");
		i_code_thumb.vartype = 4;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 114;
		cycle_cnt_thumb +=2;
    }

    else if(op1>>1==0x0 && op2==0x2){
        fprintf(result,"STRD.W imm 1 32\n");
		i_code_thumb.vartype = 5;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 115;
		cycle_cnt_thumb +=2;
    }

    else if(op1>>1==0x1 && ((op2&1)==0)){
        fprintf(result,"STRD.W imm 2 32\n");
		i_code_thumb.vartype = 5;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 116;
		cycle_cnt_thumb +=2;
    }

    else if(op1>>1==0x0 && op2==0x3){
        fprintf(result,"LDRD.W imm 32\n");
		i_code_thumb.vartype = 5;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 117;
		cycle_cnt_thumb +=2;
    }

    else if(op1>>1==0x1 && ((op2&1)==1)){
        fprintf(result,"LDRD.W literal 32\n");
		i_code_thumb.vartype = 6;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 118;
		cycle_cnt_thumb +=2;
		cycle_cnt_thumb +=30;
    }

    else if(op1==0x1){

        op3|=op2<<4;

        if(op3==0x04){
            fprintf(result,"STREXB.W 32\n");
			i_code_thumb.vartype = 7;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 119;
			cycle_cnt_thumb +=2;
        }

        else if(op3==0x05){
            fprintf(result,"STREXH.W 32\n");
			i_code_thumb.vartype = 7;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 120;
			cycle_cnt_thumb +=2;
        }

        else if(op3==0x10){
            fprintf(result,"TBB.W 32\n");
			i_code_thumb.vartype = 8;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 121;
			cycle_cnt_thumb +=2;
        }

        else if(op3==0x11){
            fprintf(result,"TBH.W 32\n");
			i_code_thumb.vartype = 8;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 122;
			cycle_cnt_thumb +=2;
        }

        else if(op3==0x14){
            fprintf(result,"LDREXB.W 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 123;
			cycle_cnt_thumb +=2;
        }

        else if(op3==0x15){
            fprintf(result,"LDREXH.W 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 124;
			cycle_cnt_thumb +=2;
        }

    }

    else fprintf(result," error 32\n");



}

void thumb_table32_1(uint32_t insn, uint16_t insn_hw1){//111 01 xxxxxx

    uint16_t op2 = (insn_hw1&0x7ff)>>4;

    if(op2>>6==1)thumb_table32_4(insn, insn_hw1);//111 01 1xxx xxx

    else if(op2>>5==1)thumb_table32_5(insn, insn_hw1);//111 01 01xx xxx

    else if((op2 & 0x64)==0)thumb_table32_6(insn, insn_hw1);//111 01 00xx 0xx

    else if((op2 & 0x64)==0x4)thumb_table32_7(insn, insn_hw1);//111 01 00xx 1xx

    else fprintf(result,"error\n");

}

void thumb_table32_9(uint32_t insn, uint16_t insn_hw1){//111 10 x0xx xxx  op=0
//data processing (modified immefiate)
    uint8_t op = (insn_hw1>>4)&0x1f;

    uint8_t Rn_thumb = insn_hw1&0xf;

    uint8_t Rd_thumb = (insn>>8)&0xf;

    op=op>>1;

    if(op==0 && Rd_thumb!=0xf){
        fprintf(result,"AND.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 125;
		cycle_cnt_thumb +=1;
    }

    else if(op==0 && Rd_thumb==0xf){
        fprintf(result,"TST.W imm 32\n");
		i_code_thumb.vartype = 18;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 126;
		cycle_cnt_thumb +=1;
    }

    else if(op==1){
        fprintf(result,"BIC.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 127;
		cycle_cnt_thumb +=1;
    }

    else if(op==2 && Rn_thumb!=0xf){
        fprintf(result,"ORR.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 128;
		cycle_cnt_thumb +=1;
    }

    else if(op==2 && Rn_thumb==0xf){
        fprintf(result,"MOV.W imm 32\n");
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 129;
		cycle_cnt_thumb +=1;
    }

    else if(op==3 && Rn_thumb!=0xf){
        fprintf(result,"ORN.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 130;
		cycle_cnt_thumb +=1;
    }

    else if(op==3 && Rn_thumb==0xf){
        fprintf(result,"MVN.W imm 32\n");
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 131;
		cycle_cnt_thumb +=1;
    }

    else if(op==4 && Rd_thumb!=0xf){
        fprintf(result,"EOR.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 132;
		cycle_cnt_thumb +=1;
    }

    else if(op==4 && Rd_thumb==0xf){
        fprintf(result,"TEQ.W imm 32\n");
		i_code_thumb.vartype = 18;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 133;
		cycle_cnt_thumb +=1;
    }

    else if(op==8 && Rd_thumb!=0xf){
        fprintf(result,"ADD.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 134;
		cycle_cnt_thumb +=1;
    }

    else if(op==8 && Rd_thumb==0xf){
        fprintf(result,"CMN.W imm 32\n");
		i_code_thumb.vartype = 18;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 135;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xa){
        fprintf(result,"ADC.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 136;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xb){
        fprintf(result,"SBC.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 137;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xd && Rd_thumb!=0xf){
        fprintf(result,"SUB.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 138;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xd && Rd_thumb==0xf){
        fprintf(result,"CMP.W imm 32\n");
		i_code_thumb.vartype = 18;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 139;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xe){
        fprintf(result,"RSB.W imm 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 140;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");



}

void thumb_table32_10(uint32_t insn, uint16_t insn_hw1){//111 10 x1xx xxx op=0
//data processing (plain binary immediate)
    uint8_t op = (insn_hw1>>4)&0x1f;

    uint8_t Rn_thumb = insn_hw1&0xf;

//fprintf(result, "insn_hw1 %x op %x, Rn_thumb %x\n",insn_hw1, op, Rn_thumb);
    if(op==0x0 && Rn_thumb!=0xf){
        fprintf(result,"ADD.W imm wide12bit 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 141;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x0 && Rn_thumb==0xf){
        fprintf(result,"ADR.W pc 32\n");//label> after current instruction
        i_code_thumb.vartype = 19;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 142;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x4){
        fprintf(result,"MOV.W imm wide16bit 32\n");
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 143;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xa && Rn_thumb!=0xf){
        fprintf(result,"SUB.W imm wide12bit 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 144;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xa && Rn_thumb==0xf){
        fprintf(result,"ADR.W pc 32\n");//label> before current instruction
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 145;
		cycle_cnt_thumb +=1;
    }

    else if(op==0xc){
        fprintf(result,"MOVT.W move top 32\n");
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 146;
		cycle_cnt_thumb +=1;
    }

    else if((op&0x1d)==0x10 && (insn&0x70c0)!=0){
        fprintf(result,"SSAT.W (?)32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 147;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x14){
        fprintf(result,"SBFX.W 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 148;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x16 && Rn_thumb!=0xf){
        fprintf(result,"BFI.W 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 149;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x16 && Rn_thumb==0xf){
        fprintf(result,"BFC.W 32\n");
		i_code_thumb.vartype = 19;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 150;
		cycle_cnt_thumb +=1;
    }

    else if((op&0x1d)==0x18 && (insn&0x70c0)!=0){
        fprintf(result,"USAT.W 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 151;
		cycle_cnt_thumb +=1;
    }

    else if(op==0x1c){
        fprintf(result,"UBFX.W 32\n");
		i_code_thumb.vartype = 17;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 152;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error 32\n");

}

void thumb_table32_12(uint32_t insn, uint16_t insn_hw1){//hint

    uint8_t op1 = (insn>>8)&0x7;

    uint8_t op2 = insn & 0xff;

    if(op1!=0)fprintf(result,"undefined 32\n");

    else if(op2==0x00){
		fprintf(result,"NOP.W 32\n");
		i_code_thumb.code = 153;
		cycle_cnt_thumb +=1;
	}
    else if(op2==0x01){
		fprintf(result,"YIELD.W 32\n");
		i_code_thumb.code = 154;
		cycle_cnt_thumb +=1;
	}
    else if(op2==0x02){
		fprintf(result,"WFE.W 32\n");
		i_code_thumb.code = 155;
		cycle_cnt_thumb +=2;
	}
    else if(op2==0x03){
		fprintf(result,"WFI.W 32\n");
		i_code_thumb.code = 156;
		cycle_cnt_thumb +=2;
	}
    else if(op2==0x04){
		fprintf(result,"SEV.W 32\n");
		i_code_thumb.code = 157;
		cycle_cnt_thumb +=1;
	}
    else if(op2>>4==0xf){
		fprintf(result,"DBG.W 32\n");
		i_code_thumb.code = 158;
		cycle_cnt_thumb +=1;
	}
    else fprintf(result,"error\n");
    i_code_thumb.affect = 0;
}

void thumb_table32_13(uint32_t insn, uint16_t insn_hw1){//miscellaneous control instructions

    uint8_t op = (insn >>4)&0xf;

    if(op==2){
		fprintf(result,"CLREX.W 32\n");
		i_code_thumb.code = 159;
		cycle_cnt_thumb +=1;
	}
    else if(op==4){
		fprintf(result,"DSB.W 32\n");
		i_code_thumb.code = 160;
		cycle_cnt_thumb +=4;
	}
    else if(op==5){
		fprintf(result,"DMB.W 32\n");
		i_code_thumb.code = 161;
		cycle_cnt_thumb +=4;
	}
    else if(op==6){
		fprintf(result,"ISB.W 32\n");
		i_code_thumb.code = 162;
		cycle_cnt_thumb +=4;
	}

    else fprintf(result,"error\n");
    i_code_thumb.affect = 0;
}

void thumb_table32_11(uint32_t insn, uint16_t insn_hw1){//111 10 xxxx xxx op=1
//branches and mescellaneous control
    uint8_t op1 = (insn_hw1>>4)&0x7f;

    uint8_t op2 = (insn>>12)&0x7;

    if((op2&0x5)==0){

        if((op1&0x38)!=0x38){
            fprintf(result,"B.W 32\n");
            i_code_thumb.affect = 0;
			i_code_thumb.code = 163;
			cycle_cnt_thumb +=1;
        }

        else if(op1>>1 == 0x1c){
            fprintf(result,"MSR.W reg 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 164;
			cycle_cnt_thumb +=4;
        }

        else if(op1==0x3A)thumb_table32_12(insn, insn_hw1);

        else if(op1==0x3B)thumb_table32_13(insn, insn_hw1);

        else if(op1>>1 ==0x1F){
            fprintf(result,"MRS.W 32\n");
			i_code_thumb.vartype = 19;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 165;
			cycle_cnt_thumb +=4;
        }

    }

    else if(op2==0x2&&op1==0x7f)fprintf(result,"permanently undefined\n");

    else if((op2&0x5)==1){
        fprintf(result,"B.W 32\n");
		i_code_thumb.code = 166;
        i_code_thumb.affect = 0;
		cycle_cnt_thumb +=1;
    }

    else if((op2&0x5)==5){
        fprintf(result,"BL.W 32\n");
		i_code_thumb.code = 167;
        i_code_thumb.affect = 0;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");

}

void thumb_table32_2(uint32_t insn, uint16_t insn_hw1){//111 10 xxxxxx

    uint8_t op = (insn >>15)&1;

    //uint8_t op1 = (insn_hw1>>11)&0x3;

    uint8_t op2 = (insn_hw1>>4)&0x7f&0x20;

    char cop2[7];
    fprintf(result,"insn_hw1 %x\n",insn_hw1);
    //itoa(op2,cop2,2);
    //fprintf(result,"insn_hw1 = %x ,op2 = %x, cop2[0] = %d,cop2[1] = %d,cop2[2] = %d\n",insn_hw1,op2,cop2[0], cop2[1],cop2[2]);

    //sprintf(cop2,"%b",op2);
    if(/*op1==0x2 && */op2==0x0 && op==0)thumb_table32_9(insn, insn_hw1);//111 10 x0xx xxx  op=0

    else if(/*op1==0x2*/op2==0x20 && op==0)thumb_table32_10(insn, insn_hw1);//111 10 x1xx xxx op=0

    else if(op==1)thumb_table32_11(insn, insn_hw1);//111 10 xxxx xxx op=1

}

void thumb_table32_14(uint32_t insn, uint16_t insn_hw1){//store single data item

    uint8_t op1=(insn_hw1>>5)&0x7;

    uint8_t op2=(insn>>6)&0x3f;

    op2=op2>>5;

    if(op1==0x4){
        fprintf(result,"STRB.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 168;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && op2==1){
        fprintf(result,"STRB.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 169;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && op2==0){
        fprintf(result,"STRB.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 170;
		cycle_cnt_thumb +=2;
    }

    else if(op1==5){
        fprintf(result,"STRH.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 171;
		cycle_cnt_thumb +=2;
    }

    else if(op1==1 && op2==1){
        fprintf(result,"STRH.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 172;
		cycle_cnt_thumb +=2;
    }

    else if(op1==1 && op2==0){
        fprintf(result,"STRH.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 173;
		cycle_cnt_thumb +=2;
    }

    else if(op1==6){
        fprintf(result,"STR.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 174;
		cycle_cnt_thumb +=2;
    }

    else if(op1==2 && op2==1){
        fprintf(result,"STR.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 175;
		cycle_cnt_thumb +=2;
    }

    else if(op1==2 && op2==0){
        fprintf(result,"STR.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 176;
		cycle_cnt_thumb +=2;
    }

    else fprintf(result,"error\n");



}

void thumb_table32_15(uint32_t insn, uint16_t insn_hw1){//load byte, memory hints

        uint8_t op1 = (insn_hw1>>7)&0x3;

        uint8_t Rn_thumb = insn_hw1&0xf;

        uint8_t Rt_thumb = (insn>>12)&0xf;

        uint8_t op2 = (insn>>6)&0x3f;

        if(op1==1 && Rn_thumb!=0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRB.W imm12 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 177;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0 && (op2&0x24)==0x24 && Rn_thumb!=0xf){
            fprintf(result,"LDRB.W imm8 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 178;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0 && (op2&0x3c)==0x30 && Rn_thumb!=0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRB.W imm8 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 179;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0 && (op2>>2)==0xe && Rn_thumb!=0xf){
            fprintf(result,"LDRBT.W 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 180;
			cycle_cnt_thumb +=2;
        }

        else if(op1>>1==0 && Rn_thumb==0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRB.W literal 32\n");
			i_code_thumb.vartype = 16;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 181;
			cycle_cnt_thumb +=2;
			cycle_cnt_thumb +=30;
        }

        else if(op1==0 && op2==0 && Rn_thumb!=0xf && Rn_thumb!=0xf){
            fprintf(result,"LDRB.W reg 32\n");
			i_code_thumb.vartype = 20;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 182;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0x3 && Rn_thumb!=0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRSB.W imm12 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 183;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0x2 && (op2&0x24)==0x24 && Rn_thumb!=0xf){
            fprintf(result,"LDRSB.W imm8 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 184;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0x2 && (op2>>2)==0xc && Rn_thumb!=0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRSB.W imm8 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 185;
			cycle_cnt_thumb +=2;
        }

        else if(op1==0x2 && (op2>>2)==0xe && Rn_thumb!=0xf){
            fprintf(result,"LDRSBT.W 32\n");
			i_code_thumb.vartype = 9;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 186;
			cycle_cnt_thumb +=2;
        }

        else if((op1>>1)==1 && Rn_thumb==0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRSB.W literal 32\n");
			i_code_thumb.vartype = 16;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 187;
			cycle_cnt_thumb +=2;
			cycle_cnt_thumb +=30;
        }

        else if(op1==0x2 && op2==0 && Rn_thumb!=0xf && Rt_thumb!=0xf){
            fprintf(result,"LDRSB.W reg 32\n");
			i_code_thumb.vartype = 20;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 188;
			cycle_cnt_thumb +=2;
        }

        else if(op1==1 && Rn_thumb!=0xf && Rt_thumb==0xf){
            fprintf(result,"PLD.W imm12 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 189;
			cycle_cnt_thumb +=1;
        }

        else if(op1==0 && ((op2>>2)==0xc) && Rn_thumb!=0xf && Rt_thumb==0xf){
            fprintf(result,"PLD.W imm8 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 190;
			cycle_cnt_thumb +=1;
        }

        else if((op1>>1)==0 && Rn_thumb==0xf && Rt_thumb==0xf){
            fprintf(result,"PLD.W imm 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 191;
			cycle_cnt_thumb +=1;
        }

        else if(op1==0 && op2==0 && Rn_thumb!=0xf && Rt_thumb==0xf){
            fprintf(result,"PLD.W reg 32\n");
			i_code_thumb.vartype = 8;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 192;
			cycle_cnt_thumb +=1;
        }

        else if(op1==0x3 && Rn_thumb!=0xf && Rt_thumb==0xf){
            fprintf(result,"PLI.W imm12 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 193;
			cycle_cnt_thumb +=1;
        }

        else if(op1==0x2 && (op2>>2)==0xc && Rn_thumb!=0xf && Rn_thumb==0xf){
            fprintf(result,"PLI.W imm8 32\n");
			i_code_thumb.vartype = 18;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 194;
			cycle_cnt_thumb +=1;
        }

        else if((op1>>1)==1 && Rn_thumb==0xf && Rt_thumb==0xf){
            fprintf(result,"PLI.W label 32\n");
            i_code_thumb.affect = 0;
			i_code_thumb.code = 195;
			cycle_cnt_thumb +=1;
        }

        else if(op1==0x2 && op2==0x0 && Rn_thumb!=0xf && Rt_thumb==0xf){
            fprintf(result,"PLI.W reg 32\n");
			i_code_thumb.vartype = 8;
            regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
			i_code_thumb.code = 196;
			cycle_cnt_thumb +=1;
        }

        else fprintf(result,"error 32\n");

}

void thumb_table32_16(uint32_t insn, uint16_t insn_hw1){//load halfword, unallocated memory hints

    uint8_t op1 = (insn_hw1>>7)&0x3;

    uint8_t op2 = (insn>>6)&0x3f;

    uint8_t Rn_thumb = insn_hw1&0xf;

    uint8_t Rt_thumb = (insn>>12)&0xf;

    if(op1==1 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRH.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 197;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2&0x24)==0x24 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRH.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 198;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2>>2)==0xc && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRH.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 199;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2>>2)==0xe && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRHT.W 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 200;
		cycle_cnt_thumb +=2;
    }

    else if((op1>>1)==0 && Rn_thumb==0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRH.W literal 32\n");
		i_code_thumb.vartype = 16;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 201;
		cycle_cnt_thumb +=2;
		cycle_cnt_thumb +=30;
    }

    else if(op1==0 && op2==0 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRH.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 202;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0x3 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSH.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 203;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0x2 && (op2&0x24)==0x24 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSH.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 204;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0x2 && (op2>>2)==0xc && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSH.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 205;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0x2 && (op2>>2)==0xe && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSHT.W 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 206;
		cycle_cnt_thumb +=2;
    }

    else if((op1>>1)==1 && Rn_thumb==0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSH.W literal 32\n");
		i_code_thumb.vartype = 16;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 207;
		cycle_cnt_thumb +=2;
		cycle_cnt_thumb +=30;
    }

    else if(op1==0x2 && op2==0x0 && Rn_thumb!=0xf && Rt_thumb!=0xf){
        fprintf(result,"LDRSH.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 208;
		cycle_cnt_thumb +=2;
    }

    else fprintf(result,"error 32\n");

}

void thumb_table32_17(uint32_t insn, uint16_t insn_hw1){//load word

    uint8_t op1 = (insn_hw1>>7)&0x3;

    uint8_t Rn_thumb = insn_hw1&0xf;

    uint8_t op2 = (insn>>6)&0x3f;

    if(op1==1 && Rn_thumb!=0xf){
        fprintf(result,"LDR.W imm12 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 209;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2&0x24)==0x24 && Rn_thumb!=0xf){
        fprintf(result,"LDR.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 210;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2>>2)==0xc && Rn_thumb!=0xf){
        fprintf(result,"LDR.W imm8 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 211;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && (op2>>2)==0xe && Rn_thumb!=0xf){
        fprintf(result,"LDRT.W 32\n");
		i_code_thumb.vartype = 9;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 212;
		cycle_cnt_thumb +=2;
    }

    else if(op1==0 && op2==0 && Rn_thumb!=0xf){
        fprintf(result,"LDR.W reg 32\n");
		i_code_thumb.vartype = 20;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 213;
		cycle_cnt_thumb +=2;
    }

    else if((op1>>1)==0 && Rn_thumb==0xf){
        fprintf(result,"LDR.W literal 32\n");
		i_code_thumb.vartype = 16;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 214;
		cycle_cnt_thumb +=2;
		cycle_cnt_thumb +=30;
    }

    else fprintf(result,"error 32\n");



}

void thumb_table32_22(uint32_t insn, uint16_t insn_hw1){//miscellaneous operation

    uint8_t op1 = (insn_hw1>>4)&0x3;

    uint8_t op2 = (insn>>4)&0x3;

    if(((insn>>12)&0xf)!=0xf)fprintf(result,"undefined 32\n");

    else if(op1==0x1 && op2==0x0){
        fprintf(result,"REV.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 215;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x1 && op2==0x1){
        fprintf(result,"REV16.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 216;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x1 && op2==0x2){
        fprintf(result,"RBIT.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 217;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x1 && op2==0x3){
        fprintf(result,"REVSH.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 218;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x3 && op2==0x0){
        fprintf(result,"CLZ.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 219;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error 32\n");

}

void thumb_table32_18(uint32_t insn, uint16_t insn_hw1){//data processing register

    uint8_t op1 = (insn_hw1>>4)&0xf;

    uint8_t op2 = (insn>>4)&0xf;

    uint8_t Rn_thumb = insn_hw1&0xf;

    if((op1>>1)==0 && op2==0){
        fprintf(result,"LSL.W reg 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 220;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>1)==1 && op2==0){
        fprintf(result,"LSR.W reg 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 221;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>1)==2 && op2==0){
        fprintf(result,"ASR.W reg 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 222;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>1)==3 && op2==0){
        fprintf(result,"ROR.W reg 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 223;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0 && (op2>>3)==1 && Rn_thumb==0xf){
        fprintf(result,"SXTH.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 224;
		cycle_cnt_thumb +=1;
    }

    else if(op1==1 && (op2>>3)==1 && Rn_thumb==0xf){
        fprintf(result,"UXTH.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 225;
		cycle_cnt_thumb +=1;
    }

    else if(op1==4 && (op2>>3)==1 && Rn_thumb==0xf){
        fprintf(result,"SXTB.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 226;
		cycle_cnt_thumb +=1;
    }

    else if(op1==5 && (op2>>3)==1 && Rn_thumb==0xf){
        fprintf(result,"UXTB.W 32\n");
		i_code_thumb.vartype = 13;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 227;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>2)==2 && (op2>>2)==0x2)thumb_table32_22(insn, insn_hw1);

    else fprintf(result,"error  32\n");

}

void thumb_table32_19(uint32_t insn, uint16_t insn_hw1){//multiply and multiply accumulate

    uint8_t op1 = (insn_hw1>>4)&0x7;

    uint8_t op2 = (insn>>4)&0x3;

    uint8_t Ra_thumb = (insn>>12)&0xf;

    if(((insn>>6)&0x3)!=0)fprintf(result,"undefined 32\n");

    else if(op1==0 && op2==0 && Ra_thumb!=0xf){
        fprintf(result,"MLA.W 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 228;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0 && op2==0 && Ra_thumb==0xf){
        fprintf(result,"MUL.W 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 229;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0 && op2==1){
        fprintf(result,"MLS.W 32\n");
		i_code_thumb.vartype = 22;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 230;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error 32\n");

}

void thumb_table32_20(uint32_t insn, uint16_t insn_hw1){//long multiply, long multiply accumulate, and divide

    uint8_t op1 = (insn_hw1>>4)&0x7;

    uint8_t op2 = (insn>>4)&0xf;

    if(op1==0 && op2==0x0){
        fprintf(result,"SMULL.W 32\n");
		i_code_thumb.vartype = 23;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 231;
		cycle_cnt_thumb +=1;
    }

    else if(op1==1 && op2==0xf){
        fprintf(result,"SDIV.W 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 232;
		cycle_cnt_thumb +=1;
    }

    else if(op1==2 && op2==0x0){
        fprintf(result,"UMULL.W 32\n");
		i_code_thumb.vartype = 23;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 233;
		cycle_cnt_thumb +=1;
    }

    else if(op1==3 && op2==0xf){
        fprintf(result,"UDIV.W 32\n");
		i_code_thumb.vartype = 21;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 234;
		cycle_cnt_thumb +=1;
    }

    else if(op1==4 && op2==0x0){
        fprintf(result,"SMLAL.W 32\n");
		i_code_thumb.vartype = 23;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 235;
		cycle_cnt_thumb +=1;
    }

    else if(op1==6 && op2==0x0){
        fprintf(result,"UMLAL.W 32\n");
		i_code_thumb.vartype = 23;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 236;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error 32\n");

}

void thumb_table32_21(uint32_t insn, uint16_t insn_hw1){//coprocessor instructions 2

    char in[6];

    uint16_t op=(insn&0x10)>>4;

    uint16_t op1=(insn_hw1>>4)&0x3f;
    uint16_t Rn = insn_hw1&0xF;
    //itoa(op1,in,2);

    sprintf(in,"%b",op1);
    if(in[0]==0&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        fprintf(result,"STC2\n");//111 111 0xxxx0
		i_code_thumb.vartype = 14;
        regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 237;
		cycle_cnt_thumb +=1;
    }

    else if(in[0]==1&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        fprintf(result,"LDC2 imm\n");//111 111 0xxxx1  Rn_thumb ='1111' literal
        i_code_thumb.vartype = 14;
		if(Rn==0xF)cycle_cnt_thumb += 30;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 238;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x4){
        fprintf(result,"MCRR2\n");//111 111 000100
        i_code_thumb.vartype = 15;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 239;
		cycle_cnt_thumb +=1;
    }

    else if(op1==0x5){
        fprintf(result,"MRRC2\n");//111 111 000101
        i_code_thumb.vartype = 15;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 240;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && op ==0){
        fprintf(result,"CDP2\n");//111 111 10xxxx op=0
		i_code_thumb.code = 241;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && in[0]==0 && op==1){
        fprintf(result,"MCR2\n");//111 111 10xxx0 op=1
        i_code_thumb.vartype = 16;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 242;
		cycle_cnt_thumb +=1;
    }

    else if((op1>>4)==0x2 && in[0]==1 && op==1){
        fprintf(result,"MRC2\n");//111 111 10xxx1 op=1
        i_code_thumb.vartype = 16;
		regimmVal_thumb32(i_code_thumb.vartype,insn_hw1,insn);
		i_code_thumb.code = 243;
		cycle_cnt_thumb +=1;
    }

    else fprintf(result,"error\n");

}

void thumb_table32_3(uint32_t insn, uint16_t insn_hw1){//111 11 xxxxxx

    uint8_t op2 = (insn_hw1>>4)&0x7f;

    if((op2&0x71)==0)thumb_table32_14(insn, insn_hw1);//op2=000x xx0

    else if((op2&0x67)==1)thumb_table32_15(insn, insn_hw1); //op2=00xx 001

    else if((op2&0x67)==3)thumb_table32_16(insn, insn_hw1); //op2=00xx 011

    else if((op2&0x67)==5)thumb_table32_17(insn, insn_hw1); //op2=00xx 101

    else if((op2&0x67)==7)fprintf(result,"undefined 32\n"); //op2=00xx 111

    else if(op2 >>4==2)thumb_table32_18(insn, insn_hw1); //op2=010x xxx

    else if(op2>>3 ==6)thumb_table32_19(insn, insn_hw1); //op2=0110 xxx

    else if(op2>>3 ==7)thumb_table32_20(insn, insn_hw1); //op2=0111 xxx

    else if(op2>>6 ==1)thumb_table32_21(insn, insn_hw1); //op2=1xxx xxx

    else fprintf(result,"error 32\n");

}

uint32_t thumb_table32(uint16_t insn_hw1, uint32_t insn){

    //uint32_t insn;

    //insn=insn_thumb_op2;

    //scanf("%x",&insn);

    //insn |= (uint32_t)insn_hw1 <<16;

    uint16_t op1=insn_hw1>>11;
    fprintf(result,"op1 = %x\n",op1);

    if(op1==0x1D)thumb_table32_1(insn,insn_hw1);//111 01 xxx

    else if(op1==0x1E)thumb_table32_2(insn,insn_hw1);//111 10 xxx

    else if(op1==0x1F)thumb_table32_3(insn,insn_hw1);//111 11 xxx

    else fprintf(result,"error");

    return insn;

}

int checkfor32(uint16_t insn){

    uint16_t op = insn >> 11;

    if(op==0x1D ||op==0x1E||op==0x1F)return 1;

    else return 0;

}
char* to_upper(char* t){
	char *s=t;
	int i=0;
	while (*t != '\0') {
		//printf("%c",*t);
		s[i] = toupper(*t);
		//t[i]=s;
		//printf("%d %d",i,t);
		t++;
		i++;
	}
	return s;
}
int readinsn_thumb(){

    strcpy(buff,"");

    fgets(buff,sizeof(buff),fptr);

    if(buff[0]=='\n' ||buff[2]!=' ')return 0;//continue;

    //fwrite(buff,sizeof(buff),1,stdout);
	count++;
	printf("\n%u ",count);
    if(buff[16]!=' '){

        sscanf(buff,"    %x:	%x %x%s",&tmp,&insn_thumb_op1,&insn_thumb_op2,&insn_name);//sscanf(buff,"\000  %d:\t%x %x",&tmp,&op1,&op2);//printf("%c%c%c%c\n",buff[15],buff[16],buff[17],buff[18]);
        //printf("%x %x %s\n",insn_thumb_op1,insn_thumb_op2,insn_name);
	//to_upper(insn_name);
	//count++;
        printf("%s ",to_upper(insn_name));
    }

    else {
	//count++;
        sscanf(buff,"%x:   %x%s",&tmp,&insn_thumb_op1,&insn_name);
        //printf("%x %s\n",insn_thumb_op1,insn_name);
	//to_upper(insn_name);
	printf("%s ",to_upper(insn_name));
    }

    return 1;

}
void checkinsn_thumb(uint16_t insn){
    uint16_t op_class = 0;
    op_class = insn >> 10 ;
    switch(op_class>>4){
        case 0x0:  //00xxxx
            thumb_table_1(insn&0x3fff);
            break;
        case 0x1:  //01xxxx
            thumb_table_2(insn&0x3fff);
            break;
        case 0x2:  //10xxxx
            thumb_table_3(insn&0x3fff);
            break;
        case 0x3:  //11xxxx
            thumb_table_4(insn&0x3fff);
            break;
        default:
            printf("wrong instruction\n");
            break;
    }
}
void checkdependency(uint8_t *imm_thumb, uint8_t *Rm_thumb, uint8_t *Rd_thumb, uint8_t *Rn_thumb, uint8_t *Rt_thumb){
    //printf("imm = %x, Rm = %x, Rd_thumb = %x, Rn_thumb = %x, Rt_thumb = %x\n",imm, Rm, Rd_thumb, Rn_thumb, Rt_thumb);
	fprintf(result,"imm = %x, Rm = %x, Rd_thumb = %x, Rn_thumb = %x, Rt_thumb = %x\n",imm_thumb, Rm_thumb, Rd_thumb, Rn_thumb, Rt_thumb);
}
/*int main(){

    uint16_t insn=0xbb7b;
    uint32_t insn_last = 0;
    uint32_t insn_32 = 0;
    int is32bit = 0;
	i_code_thumb.affect = 1;
	fptr = fopen("1102-obj","r");
	result = fopen("result","w");
	if(!fptr){
            printf("failed to open file\n");
            exit(1);
	}
	while(!feof(fptr)){
        if(!readinsn_thumb())continue;
        insn=insn_thumb_op1;
        if(checkfor32(insn)){
            insn_32 = thumb_table32(insn);
            is32bit = 1;
        }
        else {
            is32bit = 0;
            checkinsn_thumb(insn);
        }
        if(i_code_thumb.affect == 1){
			if(checkLast(i_code_thumb_t)){
				printf("type = %d\n",i_code_thumb.vartype);
				fprintf(result,"type = %d\n",i_code_thumb.vartype);
				regs(i_code_thumb, i_code_thumb_t);
			}
        }/
		i_code_thumb_t.code = i_code_thumb.code;
		i_code_thumb_t.vartype = i_code_thumb.vartype;
		i_code_thumb_t.affect = i_code_thumb.affect;
        i_code_thumb.affect = 1;

	}
	printf("cycle count = %u\n",cycle_cnt_thumb);
	fprintf(result,"cycle count = %u\n",cycle_cnt_thumb);
    return 0;
}*/
void thumb_oneloop(int n){ 
    int i;
    for(i=0;i<16;i++){
        if(n==1 && reg_record[i]>0)reg_record[i]--;	    
	fprintf(result, "r[%d] = %d ",i,reg_record[i]);
    }
    fprintf(result,"\n");
}
int read_thumb_table(uint32_t insn){

    //uint16_t insn=0xbb7b;
    //uint32_t insn_last = 0;
    uint32_t insn_32 = 0;
    int is32bit = 0;
    int temp = 0;
	i_code_thumb.affect = 1;
	//fptr = fopen("Trace3.csv","r");
//#ifdef QEMU_TEST
	//fptr = fopen(filename/*"run_test"*/,"r");

	//result = fopen("/home/yhchiang/tracerlog/test2","w");
//#else
    //result = fopen("/home/yhchiang/tracerlog/resultM","w");
//#endif
	/*if(!result){
            printf("failed to open file\n");
            exit(1);
	}*/
	fprintf(result,"===================\n");
    //while(!feof(fptr)){
	fprintf(result,"insn: %x\n",insn);
        //if(!readinsn_thumb())continue;
        //readinsn_thumb();
        //insn=insn_thumb_op1;
	insn_cnt+=1;
        if(checkfor32((insn&0xffff0000)>>16)){
            insn_32 = thumb_table32((insn&0xffff0000)>>16, insn);
            is32bit = 1;
	    //varVal32(i_code_thumb.vartype);
	    
	    
        }
        else {
            is32bit = 0;
            checkinsn_thumb(insn);
	    if((cycle_cnt_thumb - cycle_last_thumb)==0)i_code_thumb.affect = 0;
	    //varVal(i_code_thumb.vartype);
        }
	thumb_oneloop(0);
        /*if(i_code_thumb.affect == 1){
			if(checkLast(i_code_thumb_t)){
				//printf("type = %d\n",i_code_thumb.vartype);
				//fprintf(result,"type = %d\n",i_code_thumb.vartype);
				regs(i_code_thumb, i_code_thumb_t);
			}
        }
		i_code_thumb_t.code = i_code_thumb.code;
		i_code_thumb_t.vartype = i_code_thumb.vartype;
		i_code_thumb_t.affect = i_code_thumb.affect;
        i_code_thumb.affect = 1;*/
#if 1
	thumb_oneloop(0);
        if(i_code_thumb.affect == 1){
		fprintf(result,"affect\n");
			//if(checkLast(i_code_thumb_t)){
				//printf("type = %d\n",i_code_thumb.vartype);
				//fprintf(result,"type = %d\n",i_code_thumb.vartype);
				regs_thumb(i_code_thumb, i_code_thumb_t);
			//}
        }
	if(in_table > 3)thumb_oneloop(1);
	else in_table++;
	if(is32bit && i_code_thumb.affect)varVal32_thumb(i_code_thumb.vartype);
	else if(i_code_thumb.affect) varVal_thumb(i_code_thumb.vartype);
#endif
		i_code_thumb_t.code = i_code_thumb.code;
		i_code_thumb_t.vartype = i_code_thumb.vartype;
		i_code_thumb_t.affect = i_code_thumb.affect;
        	i_code_thumb.affect = 1;
        fprintf(result,"code num = %d\n",i_code_thumb.code);
        fprintf(result,"%d\n",cycle_cnt_thumb-cycle_last_thumb);
        //fprintf(insn_list," %d\n",cycle_cnt_thumb);
	temp = cycle_cnt_thumb - cycle_last_thumb;
	cycle_last_thumb = cycle_cnt_thumb;
        /*fprintf(result,"===================\n");
	}*/
	//printf("total cycle count = %u\n",cycle_cnt_thumb);
	fprintf(result,"total:%u\n",cycle_cnt_thumb);
	//fprintf(result,"total instruction count = %u\n",insn_cnt);
	final_cnt+=temp;
	//printf("final_cnt = %d\n",final_cnt);
	//printf("  %d\n",temp);
	fflush(stdout);
        return temp;
}
