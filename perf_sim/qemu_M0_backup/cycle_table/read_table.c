//ARMv6M
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "insn_code.h"
//uint8_t i_code.affect = 1;
uint16_t tmp,insn_op1=0,insn_op2=0;
//int code = -1;
struct dep i_code;
struct dep i_code_t;
//FILE* fptr;
//FILE *fptr2;
int long count=0;
char insn_name[5];
uint16_t imm=-1,Rm=-1,Rd=-1,Rn=-1,Rt=-1;
uint16_t Ra=-1, Rt2=-1, Rlist=-1;
uint16_t RdLo = -1, RdHi = -1;
unsigned long long cycle_cnt = 0;
char buff[50];
int  optype;
//char* filename = "/home/yhchiang/tracerlog/insn_trace.log";
//FILE * name;
#define QEMU_TEST
//FILE* result;
void regimmVal(int type, uint16_t insn){

    switch(type){
    case 1://imm5 Rm3 Rd3
        Rd = insn&0x7;
        Rm = (insn>>3)&0x7;
        imm = (insn>>6)&0x1f;
        break;
    case 2: //imm8 Rd3
        Rd = (insn>>8)&0x7;
        imm = insn&0xff;
        break;
    case 3: //Rm3 Rn3 Rd3
        Rd = insn&0x7;
        Rn = (insn>>3)&0x7;
        Rm = (insn>>6)&0x7;
        break;
    case 4: //imm3 Rn3 Rd3
        Rd = insn&0x7;
        Rn = (insn>>3)&0x7;
        imm = (insn>>6)&0x7;
        break;
    case 5: //imm8 Rn3
        Rn = (insn>>8)&0x7;
        imm = insn&0xff;
        break;
    case 6: //imm8 Rdn3
        Rn = (insn>>8)&0x7;
        Rd = Rn;
        imm = insn&0xff;
        break;
    case 7: //Rm3 Rdn
        Rm = (insn>>3)&0x7;
        Rd = insn&0x7;
        Rn = Rd;
        break;
    case 8: //Rm3 Rn3
        Rm = (insn>>3)&0x7;
        Rn = insn&0x7;
        break;
    case 9: //Rn3 Rd3
        Rn = (insn>>3)&0x7;
        Rd = insn&0x7;
        break;
    case 10: //Rn3 Rdm3
        Rn = (insn>>3)&0x7;
        Rd = insn&0x7;
        Rm = Rd;
        break;
    case 11: //Rm3 Rd3
        Rm = (insn>>3)&0x7;
        Rd = insn&0x7;
        break;
    case 12: //Rm4 Rdn3
        Rm = (insn>>3)&0xf;
        Rd = insn&0x7;
        Rn = Rd;
        break;
    case 13: //Rm4 Rn3
        Rm = (insn>>3)&0xf;
        Rn = insn&0x7;
        break;
    case 14: //Rm4 Rd3
        Rm = (insn>>3)&0xf;
        Rd = insn&0x7;
        break;
    case 15: //Rm4 000
        Rm = (insn>>3)&0xf;
        break;
    case 16: //imm5 Rn3 Rt3
        imm = (insn>>6)&0x1f;
        Rn = (insn>>3)&0x7;
        Rt = insn&0x7;
        break;
    case 17: //Rm3 Rn3 Rt3
        Rt = insn&0x7;
        Rn = (insn>>3)&0x7;
        Rm = (insn>>6)&0x7;
        break;
    case 18: //Rt3 imm8
        Rt = (insn>>8)&0x7;
        imm = insn&0xff;
        break;
    case 19: //imm5 Rn3
        Rn = insn&0x7;
        imm = (insn>>3)&0x1f;
        break;
    case 20: //imm7
        imm = insn&0x7f;
        break;
    case 21: //Rd3 imm8
        Rd = (insn>>8)&0x7;
        imm = insn&0xff;
        break;

    }
    checkdependency(imm, Rm, Rd, Rn, Rt);
}
void table_5(uint16_t insn_p){
	uint16_t op = insn_p>>9;
	switch(op){
	    case 0x0: //00 01100
	    	fprintf(result,"ADD reg\n");
			i_code.vartype = 3;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 0;
			cycle_cnt +=1;//checked
            break;
	    case 0x1: //00 01101
	    	fprintf(result,"SUB reg\n");
			i_code.vartype = 3;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 1;
			cycle_cnt +=1;//checked
            break;
	    case 0x2: //00 01110
	    	fprintf(result,"ADD 3imm\n");
			i_code.vartype = 4;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 2;
			cycle_cnt +=1; //checked
            break;
	    case 0x3: //00 01111
	        fprintf(result,"SUB 3imm\n");
			i_code.vartype = 4;
	        regimmVal(i_code.vartype,insn_p);
			i_code.code = 3;
			cycle_cnt +=1;
            break;
	}
}

void table_1(uint16_t insn_p){
	uint16_t op=insn_p>>9;
	switch(op>>2){
	    case 0x0: //00 000xx
	    	fprintf(result,"LSL(S) imm\n");//v
			i_code.vartype = 1;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 4;
			cycle_cnt +=1;//checked
            break;
	    case 0x1: //00 001xx
	    	fprintf(result,"LSR imm\n");//v
			i_code.vartype = 1;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 5;
			cycle_cnt +=1;
            break;
	    case 0x2: //00 010xx
	    	fprintf(result,"ASR imm\n");//v
			i_code.vartype = 1;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 6;
			cycle_cnt +=1;
            break;
	    case 0x3: //00 011xx
	        table_5(insn_p&0x7ff);

            break;
	    case 0x4: //00 100xx
	        fprintf(result,"MOV(S) imm\n");//v
			i_code.vartype = 2;
	        regimmVal(i_code.vartype,insn_p);
			i_code.code = 7;
			cycle_cnt +=1;//checked
            break;
	    case 0x5: //00 101xx
	        fprintf(result,"CMP imm\n");//v
			i_code.vartype = 5;
	        regimmVal(i_code.vartype,insn_p);//
			i_code.code = 8;
			cycle_cnt +=1;//checked
            break;
	    case 0x6: //00 110xx
	        fprintf(result,"ADD 8imm\n");//v
			i_code.vartype = 6;
	        regimmVal(i_code.vartype,insn_p);//Rn=Rd
			i_code.code = 9;
			cycle_cnt +=1;
            break;
	    case 0x7: //00 111xx
	        fprintf(result,"SUB 8imm\n");//v
			i_code.vartype = 6;
	        regimmVal(i_code.vartype,insn_p);//Rn=Rd
			i_code.code = 10;
			cycle_cnt +=1;//checked
            break;
	    default:
	    	fprintf(result,"error\n");
		break;
	}
}
void table_9(uint16_t insn_p){ //01 0000
	uint16_t op = insn_p>>6;

	//printf("%x %x\n",insn_p,op);
	switch(op){
	    case 0x00:
	    	fprintf(result,"AND reg\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 11;
			cycle_cnt +=1; //checked
	    	break;
	    case 0x01:
	    	fprintf(result,"EOR\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 12;
			cycle_cnt +=1;//checked
	    	break;
	    case 0x02:
	    	fprintf(result,"LSL\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 13;
			cycle_cnt +=1;
	    	break;
	    case 0x03:
	    	fprintf(result,"LSR\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 14;
			cycle_cnt +=1;
	    	break;
	    case 0x04:
	    	fprintf(result,"ASR\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 15;
			cycle_cnt +=1;
	    	break;
	    case 0x05:
	    	fprintf(result,"ADC\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 16;
			cycle_cnt +=1;
	    	break;
	    case 0x06:
	    	fprintf(result,"SBC\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 17;
			cycle_cnt +=1;
	    	break;
	    case 0x07:
	    	fprintf(result,"ROR\n");
			i_code.vartype= 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 18;
			cycle_cnt +=1;
	    	break;
	    case 0x08:
	    	fprintf(result,"TST\n");
			i_code.vartype = 8;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 19;
			cycle_cnt +=1; //checked
	    	break;
	    case 0x09:
	    	fprintf(result,"RSB imm\n");
			i_code.vartype = 9;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 20;
			cycle_cnt +=1;
	    	break;
	    case 0x0a:
	    	fprintf(result,"CMP reg\n");
			i_code.vartype = 8;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 21;
			cycle_cnt +=1;//check
	    	break;
	    case 0x0b:
	    	fprintf(result,"CMN\n");
			i_code.vartype = 8;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 22;
			cycle_cnt +=1;
	    	break;
	    case 0x0c:
	    	fprintf(result,"ORR\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 23;//checked
			cycle_cnt +=1;
	    	break;
	    case 0x0d:
	    	fprintf(result,"MUL\n");
			i_code.vartype = 10;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 24;
			cycle_cnt +=1;
	    	break;
	    case 0x0e:
	    	fprintf(result,"BIC\n");
			i_code.vartype = 7;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 25;
			cycle_cnt +=1; //checked
	    	break;
	    case 0x0f:
	    	fprintf(result,"MVN\n");
			i_code.vartype = 11;
	    	regimmVal(i_code.vartype,insn_p);
			i_code.code = 26;
			cycle_cnt +=1;
	    	break;
	    default:
	    	fprintf(result,"errrr");
	    	break;

	}

}
void table_7(uint16_t insn_p){ //01 001x

    fprintf(result,"LDR\n");
    uint8_t Rt=(insn_p>>8)&0x7;
    uint8_t imm = insn_p&0xff;
	i_code.code = 27;
	cycle_cnt += 2; //checked
//	uint16_t op =
}
void table_8(uint16_t insn_p){ //01 0001
	uint16_t op = insn_p>>6;
	if(op >> 2 == 0x0){
            fprintf(result,"ADD reg\n");
			i_code.vartype = 12;
            regimmVal(i_code.vartype,insn_p);
			i_code.code = 28;
			cycle_cnt +=1; //checked
	}
	else if(op >> 2 == 0x1 && op != 0x4){
		fprintf(result,"CMP reg\n");
		i_code.vartype = 13;
		regimmVal(i_code.vartype,insn_p);
		i_code.code = 29;
		cycle_cnt +=1;
	}
	else if(op >>2 == 0x2){
        fprintf(result,"MOV reg\n");
		i_code.vartype = 14;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 30;
		cycle_cnt +=1; //checked
	}
	else if(op >> 1 ==0x6 ){
        fprintf(result,"BX\n");
		i_code.vartype = 15;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 31;
		cycle_cnt +=3;//checked
	}
	else if(op >> 1 == 0x7){
        fprintf(result,"BLX reg\n");
		i_code.vartype = 15;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 32;
		cycle_cnt += 1;//3;//12;//checked//3;
	}
	else fprintf(result,"unpredictable\n");
}
void table_6(uint16_t insn_p){//01 1xxx
	uint16_t op = insn_p >> 11;
	if(op == 0x4){
		fprintf(result,"STR imm\n"); //0110 0xx
		i_code.code = 33;
		cycle_cnt +=2;//checked
	}
	else if(op == 0x5){
		fprintf(result,"LDR imm\n");//0110 1xx
		i_code.code = 34;
		cycle_cnt +=2;//checked
	}
	else if(op == 0x6){
		fprintf(result,"STRB imm\n");//0111 0xx
		i_code.code = 35;
		cycle_cnt +=2;
	}
	else if(op == 0x7){
		fprintf(result,"LDRB imm\n");//0111 1xx
		i_code.code = 36;
		cycle_cnt +=2;//checked
	}
	i_code.vartype = 16;
	regimmVal(i_code.vartype,insn_p);
}
void table_10(uint16_t insn_p){//01 01xx
	uint16_t op = insn_p >> 9;
	switch(op){
	    case 0x0: //0101 000
	    	fprintf(result,"STR reg\n");
			i_code.code = 37;
			cycle_cnt +=2;
	    	break;
	    case 0x1: //0101 001
	    	fprintf(result,"STRH reg\n");
			i_code.code = 38;
			cycle_cnt +=2;
	    	break;
	    case 0x2: //0101 010
	    	fprintf(result,"STRB reg\n");
			i_code.code = 39;
			cycle_cnt +=2;
	    	break;
	    case 0x3: //0101 011
	    	fprintf(result,"LDRSB reg\n");
			i_code.code = 40;
			cycle_cnt +=2;
	    	break;
	    case 0x4: //0101 100
	    	fprintf(result,"LDR reg\n");
			i_code.code = 41;
			cycle_cnt +=2;
	    	break;
	    case 0x5: //0101 101
	    	fprintf(result,"LDRH reg\n");
			i_code.code = 42;
			cycle_cnt +=2;
	    	break;
	    case 0x6: //0101 110
	    	fprintf(result,"LDRB reg\n");
			i_code.code = 43;
			cycle_cnt +=2;
	    	break;
	    case 0x7: //0101 111
	    	fprintf(result,"LDRSH reg\n");
			i_code.code = 44;
			cycle_cnt +=2;
	    	break;
	    default:
	    	fprintf(result,"eeeeeee\n");
		break;
	}
	i_code.vartype = 17;
	regimmVal(i_code.vartype,insn_p);
}
void table_2(uint16_t insn_p){ //01 xxxx
	uint16_t op = insn_p>>10;
	//op_type optype;
	if(op>>3==0x1){ //01 1xxx
	    table_6(insn_p);
	}
	else if(op >> 2 == 0x1){ //01 01xx
	    table_10(insn_p & 0xfff);
	}
	else if(op>>1 == 0x1){ // 01 001x
	    table_7(insn_p&0x7ff);
	}
	else if(op == 0x1){ // 01 0001
	    table_8(insn_p&0x3ff);
	}
	else{ //01 0000
	    table_9(insn_p&0x3ff);
	}
}
void table_11(uint16_t insn_p){
	uint16_t op = insn_p >>11;
	if(op == 0x0 ){
        fprintf(result,"STRH imm\n");//1000 0xx
		i_code.vartype = 16;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 45;
		cycle_cnt +=2;
	}
	else if(op == 0x1){
        fprintf(result,"LDRH imm\n");//1000 1xx
		i_code.vartype = 16;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 46;
		cycle_cnt +=2;
	}
	else if(op == 0x2){
        fprintf(result,"STR sp imm\n");//1001 0xx
		i_code.vartype = 18;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 47;
		cycle_cnt +=2; //check
	}
	else if(op == 0x3){
        fprintf(result,"LDR sp imm\n");//1001 1xx
        i_code.vartype = 18;
		regimmVal(i_code.vartype,insn_p);
		i_code.code = 48;
		cycle_cnt +=2;//check
	}
	else fprintf(result,"error\n");
}

void table_21(uint16_t insn_p){

    uint16_t opA = insn_p>>4;

    uint16_t opB = insn_p&0xf;

    /*if(opB!=0x0){
		fprintf(result,"IT\n");
		i_code.code = 49;
		cycle_cnt +=1;
	}*/

    /*else*/ if(opA == 0x0){
		fprintf(result,"NOP\n");
		i_code.code = 50;
		cycle_cnt +=1;
	}

    else if(opA == 0x1){
		fprintf(result,"YIELD\n");
		i_code.code = 51;
		cycle_cnt +=1;
	}

    else if(opA == 0x2){
		fprintf(result,"WFE\n");
		cycle_cnt +=2;
		i_code.code = 52;
	}

    else if(opA == 0x3){
		fprintf(result,"WFI\n");
		i_code.code = 53;
		cycle_cnt +=2;
	}

    else if(opA == 0x4){
		fprintf(result,"SEV\n");
		i_code.code = 54;
		cycle_cnt +=1;
	}
    i_code.affect = 0;
}

void table_15(uint16_t insn_p){//1011 000xxxx

    uint16_t op = insn_p >> 8;

    /*if(op == 0x1){
        fprintf(result,"CBNZ CBZ (cbz)1\n");//1011 0001xxx
        i_code.vartype = 19;
		regimmVal(i_code.vartype,insn_p);
		i_code.code = 55;
		cycle_cnt +=1;
    }*/

    /*else*/ if(insn_p>>7 == 0x1){
        fprintf(result,"SUB sp\n");//1011 00001xx
        i_code.affect = 0;
		i_code.vartype = 20;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 56;
		cycle_cnt +=1;
    }

    else{
        fprintf(result,"ADD sp\n");//1011 00000xx
        i_code.affect = 0;
		i_code.vartype = 20;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 57;
		cycle_cnt +=1; //checked
    }

}

void table_16(uint16_t insn_p){//1011 001xxxx

    uint16_t op = insn_p >> 6;

    uint16_t op2 = insn_p >> 8;

    if(op2 == 0x3){
        fprintf(result,"CBNZ CBZ (cbz)2\n");//1011 0011xxx
        i_code.vartype = 19;
		regimmVal(19,insn_p);
		i_code.code = 58;
		cycle_cnt +=1;
    }

    else if(op == 0x8){
        fprintf(result,"SXTH\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 59;
		cycle_cnt +=1;
    }

    else if(op == 0x9){
        fprintf(result,"SXTB\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 60;
		cycle_cnt +=1;
    }

    else if(op == 0xa){
        fprintf(result,"UXTH\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 61;
		cycle_cnt +=1;
    }

    else if(op == 0xb){
        fprintf(result,"UXTB\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 62;
		cycle_cnt +=1;
    }

    else fprintf(result,"not know\n");



}

void table_17(uint16_t insn_p){//1011 010xxxx

    fprintf(result,"PUSH\n");
    uint8_t Rlist = insn_p&0xff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
	i_code.code = 63;
	cycle_cnt += 1;// +#;//;

}

void table_18(uint16_t insn_p){//1011 1001xxx

    fprintf(result,"CBZ CBNZ (cbnz)3\n");
	i_code.vartype = 19;
    regimmVal(i_code.vartype,insn_p);
	i_code.code = 64;
	cycle_cnt +=1;

}

void table_19(uint16_t insn_p){//1011 101xxxx

    uint16_t op = insn_p >> 8;

    uint16_t op2 = insn_p >> 6;

    /*if(op == 0xb){
        fprintf(result,"CBNZ CBZ(cbnz) 4\n");//1011 1011xxx
        i_code.vartype = 19;
		regimmVal(i_code.vartype,insn_p);
		i_code.code = 65;
		cycle_cnt +=1;
    }*/

    /*else*/ if(op2 == 0x2b){
        fprintf(result,"REVSH\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 66;
		cycle_cnt +=1;
    }

    else if(op2 == 0x29){
        fprintf(result,"REV16\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 67;
		cycle_cnt +=1;
    }

    else if(op2 == 0x28){
        fprintf(result,"REV\n");
		i_code.vartype = 11;
        regimmVal(i_code.vartype,insn_p);
		i_code.code = 68;
		cycle_cnt +=1;
    }

    else fprintf(result,"not know\n");



}

void table_20(uint16_t insn_p){//1011 110xxxx

    fprintf(result,"POP\n");
    uint8_t Rlist = insn_p & 0xff;
	while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	}
	i_code.code = 69;
	cycle_cnt += 1;//3+ #;//7;//1;

}
void table_12(uint16_t insn_p){ //1011 miscellaneous
	uint16_t op = insn_p >> 5;

	uint16_t op2 = insn_p >>9;
	if(op==0x33){
        fprintf(result,"CPS");//1011 0110011
        i_code.affect = 0;
		i_code.code = 70;
		cycle_cnt +=1;
	}
	else if(op2 == 0x0) table_15(insn_p);//1011 000xxxx
    else if(op2 == 0x1) table_16(insn_p);//1011 001xxxx
    else if(op2 == 0x2) table_17(insn_p);//1011 010xxxx
    //else if(op2 == 0x4) table_18(insn_p);//1011 1001xxx
    else if(op2 == 0x5) table_19(insn_p);//1011 101xxxx
    else if(op2 == 0x6) table_20(insn_p);//1011 110xxxx
    else if(insn_p >> 8 == 0xe){
        fprintf(result,"BRKT\n");//1011 1110xxx
        i_code.affect = 0;
		i_code.code = 71;
    }
    else if(insn_p >> 8 == 0xf) table_21(insn_p & 0xff);
}

void table_13(uint16_t insn_p){

    fprintf(result,"ADR\n");
	i_code.vartype = 21;
    regimmVal(i_code.vartype,insn_p);
	i_code.code = 72;
	cycle_cnt +=1; //check

}

void table_14(uint16_t insn_p){

    fprintf(result,"ADD sp\n");
	i_code.vartype = 21;
    regimmVal(i_code.vartype,insn_p);
	i_code.code = 73;
	cycle_cnt +=1;
}
void table_3(uint16_t insn_p){ // 10 xxxx
	uint16_t op = insn_p >> 13;// 1 bit

	if(op == 0)table_11(insn_p); // 10 0xxx
	else if(insn_p>>12 == 0x3)table_12(insn_p&0xfff);// 10 11xx
	else if(insn_p >> 11 == 0x4)table_13(insn_p);// 10 100x
	else if(insn_p >> 11 == 0x5)table_14(insn_p);// 10 101x


}

void table_22(uint16_t insn_p){

    uint16_t op = insn_p >> 8 ;

    if(op==0xe)fprintf(result,"permanently undefined\n");

    else if(op == 0xf){
        fprintf(result,"SVC(formerly SWI)\n");
        i_code.affect = 0;
		i_code.code = 74;
		cycle_cnt +=1;
    }
	else if(op == 0x0){
		fprintf(result,"BEQ\n");
		i_code.affect = 0;
		i_code.code = 75;
		cycle_cnt += 1;//3;
	}
	else if(op == 0x5){
		fprintf(result,"BPL\n");
		i_code.affect = 0;
		i_code.code = 75;
		cycle_cnt +=1;
	}
	else if(op == 0x3){
		fprintf(result,"BCC\n");
		i_code.affect = 0;
		i_code.code = 75;
		cycle_cnt +=1;
	}
	else if(op == 0x1){
		fprintf(result,"BNE\n");
		i_code.affect = 0;
		i_code.code = 75;
		cycle_cnt += 1;//3;
	}
    else{
        fprintf(result,"B (BEQ)(BPL)(BNE)\n");
        i_code.affect = 0;
		i_code.code = 75;
		//cycle_cnt +=1; //checked (BEQ BPL)
		cycle_cnt += 1;//3; //BNE
    }
}
void table_4(uint16_t insn_p){//11 xxxx

    uint16_t op = insn_p >> 11;
    uint8_t Rn = 0, Rlist = 0;
    if(op == 0x0){
       fprintf(result,"STM\n");//11000x
       Rn = (insn_p>>8)&0x7;
       Rlist = insn_p&0xff;
	   while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	   }
	   i_code.code = 76;
	   cycle_cnt += 1;//+(#r);//5;//1;
    }
    else if(op == 0x1){
       fprintf(result,"LDM\n");//11001x
       Rn = (insn_p>>8)&0x7;
       Rlist = insn_p&0xff;
	   while(Rlist>0){
		   if(Rlist&1==1)cycle_cnt+=1;
		   Rlist=Rlist>>1;
	   }
	   i_code.code = 77;
	   cycle_cnt += 1;//+ (#r);//5;//2;//1;checked
    }
    else if(op == 0x4){
       fprintf(result,"B\n");//11100x
       i_code.affect = 0;
	   i_code.code = 78;
	   cycle_cnt +=1;
   }

    else if(insn_p>>12==0x1)table_22(insn_p&0xfff);//1101xx

}
void regimmVal32(int type, uint16_t insn_hw1, uint32_t insn){

    switch(type){
    /*case 1:
	case 100:
        Rn = insn_hw1&0xf;
        Rlist = insn&0x1fff;
        break;
    case 2: //pop push
        Rt = (insn>>12)&0xf;
        break;
    case 3: //Rn4 Rt4 Rd4 imm8
        Rn = insn_hw1 & 0xf;
        Rt = (insn>>12)&0xf;
        Rd = (insn>>8)&0xf;
        imm = insn&0xff;
        break;
    case 4: //Rn4 Rt4 1111 imm8
        Rn = insn_hw1 &0xf;
        Rt = (insn>>12)&0xf;
        imm = insn & 0xff;
        break;
    case 5: //Rn4 Rt4 Rt24 imm8
        Rn = insn_hw1&0xf;
        Rt = (insn>>12)&0xf;
        Rt2 = (insn>>8)&0xf;
        imm = insn & 0xff;
        break;
    case 6: //Rt4 Rt24 imm8
        Rt = (insn>>12)&0xf;
        Rt2 = (insn>>8)&0xf;
        imm = insn&0xff;
        break;
    case 7:  //Rn4 Rt4 1111 xxxx Rd4
        Rn = insn_hw1 & 0xf;
        Rt = (insn>>12)&0xf;
        Rd = insn&0xf;
        break;
    case 8: //Rn4 xxxx xxxx xxxx Rm4
        Rn = insn_hw1 & 0xf;
        Rm = insn&0xf;
        break;
    case 9: // Rn4 Rt4
        Rn = insn_hw1 &0xf;
        Rt = (insn>>12)&0xf;
        break;
    case 10:
        Rn = insn_hw1 & 0xf;
        Rd = (insn>>8)&0xf;
        Rm = insn&0xf;
        imm = (insn>>6)&0x3;
        imm|=((insn>>12)&0x7)<<2;
        break;
    case 11:
        Rn = insn_hw1 & 0xf;
        Rm = insn&0xf;
        imm = (insn>>6)&0x3;
        imm|=((insn>>12)&0x7)<<2;
        break;
    case 12:
        Rd = (insn>>8)&0xf;
        Rm = insn&0xf;
        imm = (insn>>6)&0x3;
        imm|=((insn>>12)&0x7)<<2;
        break;
    case 13:
        Rd = (insn>>8)&0xf;
        Rm = insn&0xf;
        break;
    case 14:
        Rn = insn_hw1 &0xf;
        imm = insn&0xff;
        break;
    case 15:
        Rt2 = insn_hw1&0xf;
        Rt = (insn>>12)&0xf;
        break;
    case 16: //case 2
        Rt = (insn>>12)&0xf;
        break;
    case 17:
        Rn = insn_hw1 & 0xf;
        Rd = (insn>>8)&0xf;
        break;*/
    case 18:
        Rn = insn_hw1 &0xf;
        break;
    case 19:
        Rd = (insn>>8)&0xf;
        break;
    /*case 20:
        Rn = insn_hw1&0xf;
        Rt = (insn>>12)&0xf;
        Rm = insn&0xf;
        break;
    case 21:
        Rn = insn_hw1 & 0xf;
        Rd = (insn>>8)&0xf;
        Rm = insn&0xf;
        break;
    case 22:
        Rn = insn_hw1 & 0xf;
        Rd = (insn>>8)&0xf;
        Rm = insn&0xf;
        Ra = (insn>>12)&0xf;
        break;
    case 23:
        Rn = insn_hw1 & 0xf;
        RdHi = (insn>>8)&0xf;
        Rm = insn&0xf;
        RdLo = (insn>>12)&0xf;
        break;*/
    }
}
// void table32_4(uint32_t insn, uint16_t insn_hw1){// 111 011 op1
// //coprocessor instructions
    // char in[6];

    // uint16_t op=(insn&0x10)>>4;

    // uint16_t op1=(insn_hw1>>4)&0x3f;

    // //itoa(op1,in,2);

    // sprintf(in,"%b",op1);
    // if(in[0]==0&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        // fprintf(result,"STC\n");//111 011 0xxxx0
        // i_code.vartype = 14;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 79;
		// cycle_cnt +=1;
    // }

    // else if(in[0]==1&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        // fprintf(result,"LDC imm\n");//111 011 0xxxx1  rn ='1111' literal
        // i_code.vartype = 14;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 80;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x4){
        // fprintf(result,"MCRR\n");//111 011 000100
		// i_code.vartype = 15;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 81;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x5){
        // fprintf(result,"MRRC\n");//111 011 000101
		// i_code.vartype = 15;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 82;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && op ==0){
        // fprintf(result,"CDP\n");//111 011 10xxxx op=0
        // i_code.affect = 0;
		// i_code.code = 83;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && in[0]==0 && op==1){
        // fprintf(result,"MCR\n");//111 011 10xxx0 op=1
		// i_code.vartype = 16;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 84;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && in[0]==1 && op==1){
        // fprintf(result,"MRC\n");//111 011 10xxx1 op=1
		// i_code.vartype = 16;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 85;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error\n");

// }

// void table32_8(uint32_t insn, uint16_t insn_hw1){ //111 0101 0010 x 1111 x imm3 xxxx imm2 type2 xxxx
// //move register and immediate shifts
    // uint8_t type=(insn>>4)&0x3;

    // uint8_t imm3=(insn&0x7000)>>12;

    // uint8_t imm2=(insn&0xC0)>>6;

    // imm2|=imm3<<2;

    // if(type==0x0 && imm2==0x0){
        // fprintf(result,"MOV.W reg 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 86;
		// cycle_cnt +=1;
    // }

    // else if(type == 0x0 && imm2!=0x0){
        // fprintf(result,"LSL.W imm 32");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 87;
		// cycle_cnt +=1;
    // }

    // else if(type == 0x1){
        // fprintf(result,"LSR.W imm 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 88;
		// cycle_cnt +=1;
    // }

    // else if(type == 0x2){
        // fprintf(result,"ASR.W imm 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 89;
		// cycle_cnt +=1;
    // }

    // else if(type == 0x3 && imm2==0x0){
        // fprintf(result,"RRX.W 32\n");
        // i_code.vartype = 13;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 90;
		// cycle_cnt +=1;
    // }

    // else if(type == 0x3 && imm2!=0x0){
        // fprintf(result,"ROR.W imm 32\n");
        // i_code.vartype = 13;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 91;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error\n");

// }

// void table32_5(uint32_t insn, uint16_t insn_hw1){//111 0101 op
// //data processing(shifted register)
    // uint16_t op = (insn_hw1&0x1E0)>>5;

    // uint16_t Rn = insn_hw1&0xf;

    // uint8_t S = (insn_hw1&0x10)>>4;

    // uint8_t Rd = (insn&0xf00)>>8;

    // if(op==0 && Rd!= 0xf){
        // fprintf(result,"AND.W reg 32\n");//111 0101 0000 x
        // i_code.vartype = 10;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 92;
		// cycle_cnt +=1;
    // }

    // else if(op==0 && Rd==0xf && S==0){
        // fprintf(result,"unpredictable\n");
    // }

    // else if(op==0 && Rd==0xf && S==1){
        // fprintf(result,"TST.W reg 32\n");//111 0101 0000 1 Rd=1111
        // i_code.vartype = 11;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 93;
		// cycle_cnt +=1;
    // }

    // else if(op==1){
        // fprintf(result,"BIC.W reg 32\n");//111 0101 0001
        // i_code.vartype = 10;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 94;
		// cycle_cnt +=1;
    // }

    // else if(op==0x2 && Rn!=0xf){
        // fprintf(result,"ORR.W reg 32\n");//111 0101 0010 Rn!=1111
        // i_code.vartype = 10;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 95;
		// cycle_cnt +=1;
    // }

    // else if(op==0x2 && Rn==0xf)table32_8(insn, insn_hw1);//111 0101 0010 Rn==1111

    // else if(op==0x3 && Rn!=0xf){
        // fprintf(result,"ORN.W reg 32\n");//111 0101 0011 Rn!=1111
        // i_code.vartype = 10;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 96;
		// cycle_cnt +=1;
    // }

    // else if(op==0x3 && Rn==0xf){
        // fprintf(result,"MVN.W reg 32\n");//111 0101 0011 Rn==1111
        // i_code.vartype = 12;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 97;
		// cycle_cnt +=1;
    // }

    // else if(op==0x4 && Rd!=0xf){
        // fprintf(result,"EOR.W reg 32\n");//
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 98;
		// cycle_cnt +=1;
    // }

    // else if(op==0x4 && Rd==0xf && S==0){
        // fprintf(result,"unpredictable 32\n");
    // }

    // else if(op==0x4 && Rd==0xf && S==1){
        // fprintf(result,"TEQ.W reg 32\n");
		// i_code.vartype = 11;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 99;
		// cycle_cnt +=1;
    // }

    // else if(op==0x8 && Rd!=0xf){
        // fprintf(result,"ADD.W reg 32\n");
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 100;
		// cycle_cnt +=1;
    // }

    // else if(op==0x8 && Rd==0xf && S==0)fprintf(result,"unpredictable 32\n");

    // else if(op==0x8 && Rd==0xf && S==1){
        // fprintf(result,"CMN.W reg 32\n");
		// i_code.vartype = 11;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 101;
		// cycle_cnt +=1;
    // }

    // else if(op==0xa){
        // fprintf(result,"ADC.W reg 32\n");
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 102;
		// cycle_cnt +=1;
    // }

    // else if(op==0xb){
        // fprintf(result,"SBC.W reg 32\n");
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 103;
		// cycle_cnt +=1;
    // }

    // else if(op==0xd && Rd!=0xf){
        // fprintf(result,"SUB.W reg 32\n");
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 104;
		// cycle_cnt +=1;
    // }

    // else if(op==0xd && Rd==0xf && S==0){
        // fprintf(result,"unpredictable\n");
    // }

    // else if(op==0xd && Rd==0xf && S==1){
        // fprintf(result,"CMP.W reg 32\n");
		// i_code.vartype = 11;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 105;
		// cycle_cnt +=1;
    // }

    // else if(op==0xe){
        // fprintf(result,"RSB.W reg 32\n");
		// i_code.vartype = 10;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 106;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error\n");

// }

// void table32_6(uint32_t insn, uint16_t insn_hw1){//111 0100 op 0 W L Rn
// //load/store multiple
    // uint8_t op=(insn_hw1>>7)&0x3;

    // uint8_t L = (insn_hw1>>4)&0x1;

    // uint8_t W = (insn_hw1>>5)&0x1;

    // uint8_t Rn = insn_hw1&0xf;

    // Rn|=W<<4;

    // if(op==0x1&& L==0){
        // fprintf(result,"STM.W 32\n");
		// i_code.vartype = 1;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 107;
		// cycle_cnt +=1;
    // }

    // else if(op==0x1 && L==1 && Rn!=0x1D){
        // fprintf(result,"LDM.W 32\n");
		// i_code.vartype = 1;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 108;
		// cycle_cnt +=1;
    // }

    // else if(op==0x1 && L==1 && Rn==0x1D){
        // fprintf(result,"POP.W 32\n");
		// i_code.vartype = 2;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 109;
		// cycle_cnt +=1;
    // }

    // else if(op==0x2 && L==0 && Rn!=0x1D){
        // fprintf(result,"STMDB.W 32\n");
		// i_code.vartype = 1;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 110;
		// cycle_cnt +=1;
    // }

    // else if(op==0x2 && L==0 && Rn==0x1D){
        // fprintf(result,"PUSH.W 32\n");
		// i_code.vartype = 2;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 111;
		// cycle_cnt +=1;
    // }

    // else if(op==0x2 && L==1 && W==1){//Rn writeback
        // fprintf(result,"LDMDB.W 32\n");
		// i_code.vartype = 1;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 112;
		// cycle_cnt +=1;
    // }
	// else if(op==0x2 && L==1 && W==0){//Rn not writeback
		// fprintf(result,"LDMDB.W 32\n");
		// i_code.vartype = 100;//@@@@@@
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 112;
		// cycle_cnt +=1;
	// }

    // else fprintf(result,"error\n");

// }

// void table32_7(uint32_t insn, uint16_t insn_hw1){//111 0100 op1 1 op2 Rn
// //load/store dual or exclusive, table branch
    // uint8_t op1 = (insn_hw1>>7)&0x3;

    // uint8_t op2 = (insn_hw1>>4)&0x3;

    // uint8_t op3 = (insn>>4)&0xf;

    // if(op1==0x0 && op2==0x0){
        // fprintf(result,"STREX.W 32\n");
		// i_code.vartype = 3;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 113;
		// cycle_cnt +=2;
    // }
    // else if(op1==0x0 && op2==0x1){
        // fprintf(result,"LDREX.W 32\n");
		// i_code.vartype = 4;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 114;
		// cycle_cnt +=2;
    // }

    // else if(op1>>1==0x0 && op2==0x2){
        // fprintf(result,"STRD.W imm 1 32\n");
		// i_code.vartype = 5;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 115;
		// cycle_cnt +=2;
    // }

    // else if(op1>>1==0x1 && ((op2&1)==0)){
        // fprintf(result,"STRD.W imm 2 32\n");
		// i_code.vartype = 5;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 116;
		// cycle_cnt +=2;
    // }

    // else if(op1>>1==0x0 && op2==0x3){
        // fprintf(result,"LDRD.W imm 32\n");
		// i_code.vartype = 5;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 117;
		// cycle_cnt +=2;
    // }

    // else if(op1>>1==0x1 && ((op2&1)==1)){
        // fprintf(result,"LDRD.W literal 32\n");
		// i_code.vartype = 6;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 118;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x1){

        // op3|=op2<<4;

        // if(op3==0x04){
            // fprintf(result,"STREXB.W 32\n");
			// i_code.vartype = 7;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 119;
			// cycle_cnt +=2;
        // }

        // else if(op3==0x05){
            // fprintf(result,"STREXH.W 32\n");
			// i_code.vartype = 7;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 120;
			// cycle_cnt +=2;
        // }

        // else if(op3==0x10){
            // fprintf(result,"TBB.W 32\n");
			// i_code.vartype = 8;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 121;
			// cycle_cnt +=2;
        // }

        // else if(op3==0x11){
            // fprintf(result,"TBH.W 32\n");
			// i_code.vartype = 8;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 122;
			// cycle_cnt +=2;
        // }

        // else if(op3==0x14){
            // fprintf(result,"LDREXB.W 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 123;
			// cycle_cnt +=2;
        // }

        // else if(op3==0x15){
            // fprintf(result,"LDREXH.W 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 124;
			// cycle_cnt +=2;
        // }

    // }

    // else fprintf(result," error 32\n");



// }

// void table32_1(uint32_t insn, uint16_t insn_hw1){//111 01 xxxxxx

    // uint16_t op2 = (insn_hw1&0x7ff)>>4;

    // if(op2>>6==1)table32_4(insn, insn_hw1);//111 01 1xxx xxx

    // else if(op2>>5==1)table32_5(insn, insn_hw1);//111 01 01xx xxx

    // else if((op2 & 0x64)==0)table32_6(insn, insn_hw1);//111 01 00xx 0xx

    // else if((op2 & 0x64)==0x4)table32_7(insn, insn_hw1);//111 01 00xx 1xx

    // else fprintf(result,"error\n");

// }

// void table32_9(uint32_t insn, uint16_t insn_hw1){//111 10 x0xx xxx  op=0
// //data processing (modified immefiate)
    // uint8_t op = (insn_hw1>>4)&0x1f;

    // uint8_t Rn = insn_hw1&0xf;

    // uint8_t Rd = (insn>>8)&0xf;

    // op=op>>1;

    // if(op==0 && Rd!=0xf){
        // fprintf(result,"AND.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 125;
		// cycle_cnt +=1;
    // }

    // else if(op==0 && Rd==0xf){
        // fprintf(result,"TST.W imm 32\n");
		// i_code.vartype = 18;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 126;
		// cycle_cnt +=1;
    // }

    // else if(op==1){
        // fprintf(result,"BIC.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 127;
		// cycle_cnt +=1;
    // }

    // else if(op==2 && Rn!=0xf){
        // fprintf(result,"ORR.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 128;
		// cycle_cnt +=1;
    // }

    // else if(op==2 && Rn==0xf){
        // fprintf(result,"MOV.W imm 32\n");
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 129;
		// cycle_cnt +=1;
    // }

    // else if(op==3 && Rn!=0xf){
        // fprintf(result,"ORN.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 130;
		// cycle_cnt +=1;
    // }

    // else if(op==3 && Rn==0xf){
        // fprintf(result,"MVN.W imm 32\n");
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 131;
		// cycle_cnt +=1;
    // }

    // else if(op==4 && Rd!=0xf){
        // fprintf(result,"EOR.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 132;
		// cycle_cnt +=1;
    // }

    // else if(op==4 && Rd==0xf){
        // fprintf(result,"TEQ.W imm 32\n");
		// i_code.vartype = 18;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 133;
		// cycle_cnt +=1;
    // }

    // else if(op==8 && Rd!=0xf){
        // fprintf(result,"ADD.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 134;
		// cycle_cnt +=1;
    // }

    // else if(op==8 && Rd==0xf){
        // fprintf(result,"CMN.W imm 32\n");
		// i_code.vartype = 18;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 135;
		// cycle_cnt +=1;
    // }

    // else if(op==0xa){
        // fprintf(result,"ADC.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 136;
		// cycle_cnt +=1;
    // }

    // else if(op==0xb){
        // fprintf(result,"SBC.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 137;
		// cycle_cnt +=1;
    // }

    // else if(op==0xd && Rd!=0xf){
        // fprintf(result,"SUB.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 138;
		// cycle_cnt +=1;
    // }

    // else if(op==0xd && Rd==0xf){
        // fprintf(result,"CMP.W imm 32\n");
		// i_code.vartype = 18;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 139;
		// cycle_cnt +=1;
    // }

    // else if(op==0xe){
        // fprintf(result,"RSB.W imm 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 140;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error\n");



// }

// void table32_10(uint32_t insn, uint16_t insn_hw1){//111 10 x1xx xxx op=0
// //data processing (plain binary immediate)
    // uint8_t op = (insn_hw1>>4)&&0x1f;

    // uint8_t Rn = insn_hw1&0xf;

    // if(op==0x0 && Rn!=0xf){
        // fprintf(result,"ADD.W imm wide12bit 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 141;
		// cycle_cnt +=1;
    // }

    // else if(op==0x0 && Rn==0xf){
        // fprintf(result,"ADR.W pc 32\n");//label> after current instruction
        // i_code.vartype = 19;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 142;
		// cycle_cnt +=1;
    // }

    // else if(op==0x4){
        // fprintf(result,"MOV.W imm wide16bit 32\n");
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 143;
		// cycle_cnt +=1;
    // }

    // else if(op==0xa && Rn!=0xf){
        // fprintf(result,"SUB.W imm wide12bit 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 144;
		// cycle_cnt +=1;
    // }

    // else if(op==0xa && Rn==0xf){
        // fprintf(result,"ADR.W pc 32\n");//label> before current instruction
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 145;
		// cycle_cnt +=1;
    // }

    // else if(op==0xc){
        // fprintf(result,"MOVT.W move top 32\n");
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 146;
		// cycle_cnt +=1;
    // }

    // else if((op&0x1d)==0x10 && (insn&0x70c0)!=0){
        // fprintf(result,"SSAT.W (?)32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 147;
		// cycle_cnt +=1;
    // }

    // else if(op==0x14){
        // fprintf(result,"SBFX.W 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 148;
		// cycle_cnt +=1;
    // }

    // else if(op==0x16 && Rn!=0xf){
        // fprintf(result,"BFI.W 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 149;
		// cycle_cnt +=1;
    // }

    // else if(op==0x16 && Rn==0xf){
        // fprintf(result,"BFC.W 32\n");
		// i_code.vartype = 19;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 150;
		// cycle_cnt +=1;
    // }

    // else if((op&0x1d)==0x18 && (insn&0x70c0)!=0){
        // fprintf(result,"USAT.W 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 151;
		// cycle_cnt +=1;
    // }

    // else if(op==0x1c){
        // fprintf(result,"UBFX.W 32\n");
		// i_code.vartype = 17;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 152;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error 32\n");

// }

// void table32_12(uint32_t insn, uint16_t insn_hw1){//hint

    // uint8_t op1 = (insn>>8)&0x7;

    // uint8_t op2 = insn & 0xff;

    // if(op1!=0)fprintf(result,"undefined 32\n");

    // else if(op2==0x00){
		// fprintf(result,"NOP.W 32\n");
		// i_code.code = 153;
		// cycle_cnt +=1;
	// }
    // else if(op2==0x01){
		// fprintf(result,"YIELD.W 32\n");
		// i_code.code = 154;
		// cycle_cnt +=1;
	// }
    // else if(op2==0x02){
		// fprintf(result,"WFE.W 32\n");
		// i_code.code = 155;
		// cycle_cnt +=2;
	// }
    // else if(op2==0x03){
		// fprintf(result,"WFI.W 32\n");
		// i_code.code = 156;
		// cycle_cnt +=2;
	// }
    // else if(op2==0x04){
		// fprintf(result,"SEV.W 32\n");
		// i_code.code = 157;
		// cycle_cnt +=1;
	// }
    // else if(op2>>4==0xf){
		// fprintf(result,"DBG.W 32\n");
		// i_code.code = 158;
		// cycle_cnt +=1;
	// }
    // else fprintf(result,"error\n");
    // i_code.affect = 0;
// }

// void table32_13(uint32_t insn, uint16_t insn_hw1){//miscellaneous control instructions

    // uint8_t op = (insn >>4)&0xf;

    // if(op==2){
		// fprintf(result,"CLREX.W 32\n");
		// i_code.code = 159;
		// cycle_cnt +=1;
	// }
    // else if(op==4){
		// fprintf(result,"DSB.W 32\n");
		// i_code.code = 160;
		// cycle_cnt +=4;
	// }
    // else if(op==5){
		// fprintf(result,"DMB.W 32\n");
		// i_code.code = 161;
		// cycle_cnt +=4;
	// }
    // else if(op==6){
		// fprintf(result,"ISB.W 32\n");
		// i_code.code = 162;
		// cycle_cnt +=4;
	// }

    // else fprintf(result,"error\n");
    // i_code.affect = 0;
// }

// void table32_11(uint32_t insn, uint16_t insn_hw1){//111 10 xxxx xxx op=1
// //branches and mescellaneous control
    // uint8_t op1 = (insn_hw1>>4)&0x7f;

    // uint8_t op2 = (insn>>12)&0x7;

    // if((op2&0x5)==0){

        // if((op1&0x38)!=0x38){
            // fprintf(result,"B.W 32\n");
            // i_code.affect = 0;
			// i_code.code = 163;
			// cycle_cnt +=1;
        // }

        // else if(op1>>1 == 0x1c){
            // fprintf(result,"MSR.W reg 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 164;
			// cycle_cnt +=4;
        // }

        // else if(op1==0x3A)table32_12(insn, insn_hw1);

        // else if(op1==0x3B)table32_13(insn, insn_hw1);

        // else if(op1>>1 ==0x1F){
            // fprintf(result,"MRS.W 32\n");
			// i_code.vartype = 19;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 165;
			// cycle_cnt +=4;
        // }

    // }

    // else if(op2==0x2&&op1==0x7f)fprintf(result,"permanently undefined\n");

    // else if((op2&0x5)==1){
        // fprintf(result,"B.W 32\n");
		// i_code.code = 166;
        // i_code.affect = 0;
		// cycle_cnt +=1;
    // }

    // else if((op2&0x5)==5){
        // fprintf(result,"BL.W 32\n");
		// i_code.code = 167;
        // i_code.affect = 0;
		// cycle_cnt +=4;
    // }

    // else fprintf(result,"error\n");

// }

// void table32_2(uint32_t insn, uint16_t insn_hw1){//111 10 xxxxxx

    // uint8_t op = (insn >>15)&1;

    // //uint8_t op1 = (insn_hw1>>11)&0x3;

    // uint8_t op2 = (insn_hw1>>4)&0x7f;

    // char cop2[7];

    // //itoa(op2,cop2,2);

    // sprintf(cop2,"%b",op2);
    // if(/*op1==0x2 && */cop2[5]==0 && op==0)table32_9(insn, insn_hw1);//111 10 x0xx xxx  op=0

    // else if(/*op1==0x2*/cop2[5]==1 && op==0)table32_10(insn, insn_hw1);//111 10 x1xx xxx op=0

    // else if(op==1)table32_11(insn, insn_hw1);//111 10 xxxx xxx op=1

// }

// void table32_14(uint32_t insn, uint16_t insn_hw1){//store single data item

    // uint8_t op1=(insn_hw1>>5)&0x7;

    // uint8_t op2=(insn>>6)&0x3f;

    // op2=op2>>5;

    // if(op1==0x4){
        // fprintf(result,"STRB.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 168;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && op2==1){
        // fprintf(result,"STRB.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 169;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && op2==0){
        // fprintf(result,"STRB.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 170;
		// cycle_cnt +=2;
    // }

    // else if(op1==5){
        // fprintf(result,"STRH.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 171;
		// cycle_cnt +=2;
    // }

    // else if(op1==1 && op2==1){
        // fprintf(result,"STRH.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 172;
		// cycle_cnt +=2;
    // }

    // else if(op1==1 && op2==0){
        // fprintf(result,"STRH.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 173;
		// cycle_cnt +=2;
    // }

    // else if(op1==6){
        // fprintf(result,"STR.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 174;
		// cycle_cnt +=2;
    // }

    // else if(op1==2 && op2==1){
        // fprintf(result,"STR.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 175;
		// cycle_cnt +=2;
    // }

    // else if(op1==2 && op2==0){
        // fprintf(result,"STR.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 176;
		// cycle_cnt +=2;
    // }

    // else fprintf(result,"error\n");



// }

// void table32_15(uint32_t insn, uint16_t insn_hw1){//load byte, memory hints

        // uint8_t op1 = (insn_hw1>>7)&0x3;

        // uint8_t Rn = insn_hw1&0xf;

        // uint8_t Rt = (insn>>12)&0xf;

        // uint8_t op2 = (insn>>6)&0x3f;

        // if(op1==1 && Rn!=0xf && Rt!=0xf){
            // fprintf(result,"LDRB.W imm12 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 177;
			// cycle_cnt +=2;
        // }

        // else if(op1==0 && (op2&0x24)==0x24 && Rn!=0xf){
            // fprintf(result,"LDRB.W imm8 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 178;
			// cycle_cnt +=2;
        // }

        // else if(op1==0 && (op2&0x3c)==0x30 && Rn!=0xf && Rt!=0xf){
            // fprintf(result,"LDRB.W imm8 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 179;
			// cycle_cnt +=2;
        // }

        // else if(op1==0 && (op2>>2)==0xe && Rn!=0xf){
            // fprintf(result,"LDRBT.W 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 180;
			// cycle_cnt +=2;
        // }

        // else if(op1>>1==0 && Rn==0xf && Rt!=0xf){
            // fprintf(result,"LDRB.W literal 32\n");
			// i_code.vartype = 16;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 181;
			// cycle_cnt +=2;
        // }

        // else if(op1==0 && op2==0 && Rn!=0xf && Rn!=0xf){
            // fprintf(result,"LDRB.W reg 32\n");
			// i_code.vartype = 20;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 182;
			// cycle_cnt +=2;
        // }

        // else if(op1==0x3 && Rn!=0xf && Rt!=0xf){
            // fprintf(result,"LDRSB.W imm12 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 183;
			// cycle_cnt +=2;
        // }

        // else if(op1==0x2 && (op2&0x24)==0x24 && Rn!=0xf){
            // fprintf(result,"LDRSB.W imm8 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 184;
			// cycle_cnt +=2;
        // }

        // else if(op1==0x2 && (op2>>2)==0xc && Rn!=0xf && Rt!=0xf){
            // fprintf(result,"LDRSB.W imm8 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 185;
			// cycle_cnt +=2;
        // }

        // else if(op1==0x2 && (op2>>2)==0xe && Rn!=0xf){
            // fprintf(result,"LDRSBT.W 32\n");
			// i_code.vartype = 9;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 186;
			// cycle_cnt +=2;
        // }

        // else if((op1>>1)==1 && Rn==0xf && Rt!=0xf){
            // fprintf(result,"LDRSB.W literal 32\n");
			// i_code.vartype = 16;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 187;
			// cycle_cnt +=2;
        // }

        // else if(op1==0x2 && op2==0 && Rn!=0xf && Rt!=0xf){
            // fprintf(result,"LDRSB.W reg 32\n");
			// i_code.vartype = 20;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 188;
			// cycle_cnt +=2;
        // }

        // else if(op1==1 && Rn!=0xf && Rt==0xf){
            // fprintf(result,"PLD.W imm12 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 189;
			// cycle_cnt +=1;
        // }

        // else if(op1==0 && ((op2>>2)==0xc) && Rn!=0xf && Rt==0xf){
            // fprintf(result,"PLD.W imm8 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 190;
			// cycle_cnt +=1;
        // }

        // else if((op1>>1)==0 && Rn==0xf && Rt==0xf){
            // fprintf(result,"PLD.W imm 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 191;
			// cycle_cnt +=1;
        // }

        // else if(op1==0 && op2==0 && Rn!=0xf && Rt==0xf){
            // fprintf(result,"PLD.W reg 32\n");
			// i_code.vartype = 8;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 192;
			// cycle_cnt +=1;
        // }

        // else if(op1==0x3 && Rn!=0xf && Rt==0xf){
            // fprintf(result,"PLI.W imm12 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 193;
			// cycle_cnt +=1;
        // }

        // else if(op1==0x2 && (op2>>2)==0xc && Rn!=0xf && Rn==0xf){
            // fprintf(result,"PLI.W imm8 32\n");
			// i_code.vartype = 18;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 194;
			// cycle_cnt +=1;
        // }

        // else if((op1>>1)==1 && Rn==0xf && Rt==0xf){
            // fprintf(result,"PLI.W label 32\n");
            // i_code.affect = 0;
			// i_code.code = 195;
			// cycle_cnt +=1;
        // }

        // else if(op1==0x2 && op2==0x0 && Rn!=0xf && Rt==0xf){
            // fprintf(result,"PLI.W reg 32\n");
			// i_code.vartype = 8;
            // regimmVal32(i_code.vartype,insn_hw1,insn);
			// i_code.code = 196;
			// cycle_cnt +=1;
        // }

        // else fprintf(result,"error 32\n");

// }

// void table32_16(uint32_t insn, uint16_t insn_hw1){//load halfword, unallocated memory hints

    // uint8_t op1 = (insn_hw1>>7)&0x3;

    // uint8_t op2 = (insn>>6)&0x3f;

    // uint8_t Rn = insn_hw1&0xf;

    // uint8_t Rt = (insn>>12)&0xf;

    // if(op1==1 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRH.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 197;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2&0x24)==0x24 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRH.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 198;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2>>2)==0xc && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRH.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 199;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2>>2)==0xe && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRHT.W 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 200;
		// cycle_cnt +=2;
    // }

    // else if((op1>>1)==0 && Rn==0xf && Rt!=0xf){
        // fprintf(result,"LDRH.W literal 32\n");
		// i_code.vartype = 16;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 201;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && op2==0 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRH.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 202;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x3 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRSH.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 203;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x2 && (op2&0x24)==0x24 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRSH.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 204;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x2 && (op2>>2)==0xc && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRSH.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 205;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x2 && (op2>>2)==0xe && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRSHT.W 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 206;
		// cycle_cnt +=2;
    // }

    // else if((op1>>1)==1 && Rn==0xf && Rt!=0xf){
        // fprintf(result,"LDRSH.W literal 32\n");
		// i_code.vartype = 16;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 207;
		// cycle_cnt +=2;
    // }

    // else if(op1==0x2 && op2==0x0 && Rn!=0xf && Rt!=0xf){
        // fprintf(result,"LDRSH.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 208;
		// cycle_cnt +=2;
    // }

    // else fprintf(result,"error 32\n");

// }

// void table32_17(uint32_t insn, uint16_t insn_hw1){//load word

    // uint8_t op1 = (insn_hw1>>7)&0x3;

    // uint8_t Rn = insn_hw1&0xf;

    // uint8_t op2 = (insn>>6)&0x3f;

    // if(op1==1 && Rn!=0xf){
        // fprintf(result,"LDR.W imm12 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 209;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2&0x24)==0x24 && Rn!=0xf){
        // fprintf(result,"LDR.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 210;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2>>2)==0xc && Rn!=0xf){
        // fprintf(result,"LDR.W imm8 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 211;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && (op2>>2)==0xe && Rn!=0xf){
        // fprintf(result,"LDRT.W 32\n");
		// i_code.vartype = 9;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 212;
		// cycle_cnt +=2;
    // }

    // else if(op1==0 && op2==0 && Rn!=0xf){
        // fprintf(result,"LDR.W reg 32\n");
		// i_code.vartype = 20;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 213;
		// cycle_cnt +=2;
    // }

    // else if((op1>>1)==0 && Rn==0xf){
        // fprintf(result,"LDR.W literal 32\n");
		// i_code.vartype = 16;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 214;
		// cycle_cnt +=2;
    // }

    // else fprintf(result,"error 32\n");



// }

// void table32_22(uint32_t insn, uint16_t insn_hw1){//miscellaneous operation

    // uint8_t op1 = (insn_hw1>>4)&0x3;

    // uint8_t op2 = (insn>>4)&0x3;

    // if(((insn>>12)&0xf)!=0xf)fprintf(result,"undefined 32\n");

    // else if(op1==0x1 && op2==0x0){
        // fprintf(result,"REV.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 215;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x1 && op2==0x1){
        // fprintf(result,"REV16.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 216;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x1 && op2==0x2){
        // fprintf(result,"RBIT.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 217;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x1 && op2==0x3){
        // fprintf(result,"REVSH.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 218;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x3 && op2==0x0){
        // fprintf(result,"CLZ.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 219;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error 32\n");

// }

// void table32_18(uint32_t insn, uint16_t insn_hw1){//data processing register

    // uint8_t op1 = (insn_hw1>>4)&0xf;

    // uint8_t op2 = (insn>>4)&0xf;

    // uint8_t Rn = insn_hw1&0xf;

    // if((op1>>1)==0 && op2==0){
        // fprintf(result,"LSL.W reg 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 220;
		// cycle_cnt +=1;
    // }

    // else if((op1>>1)==1 && op2==0){
        // fprintf(result,"LSR.W reg 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 221;
		// cycle_cnt +=1;
    // }

    // else if((op1>>1)==2 && op2==0){
        // fprintf(result,"ASR.W reg 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 222;
		// cycle_cnt +=1;
    // }

    // else if((op1>>1)==3 && op2==0){
        // fprintf(result,"ROR.W reg 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 223;
		// cycle_cnt +=1;
    // }

    // else if(op1==0 && (op2>>3)==1 && Rn==0xf){
        // fprintf(result,"SXTH.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 224;
		// cycle_cnt +=1;
    // }

    // else if(op1==1 && (op2>>3)==1 && Rn==0xf){
        // fprintf(result,"UXTH.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 225;
		// cycle_cnt +=1;
    // }

    // else if(op1==4 && (op2>>3)==1 && Rn==0xf){
        // fprintf(result,"SXTB.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 226;
		// cycle_cnt +=1;
    // }

    // else if(op1==5 && (op2>>3)==1 && Rn==0xf){
        // fprintf(result,"UXTB.W 32\n");
		// i_code.vartype = 13;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 227;
		// cycle_cnt +=1;
    // }

    // else if((op1>>2)==2 && (op2>>2)==0x2)table32_22(insn, insn_hw1);

    // else fprintf(result,"error  32\n");

// }

// void table32_19(uint32_t insn, uint16_t insn_hw1){//multiply and multiply accumulate

    // uint8_t op1 = (insn_hw1>>4)&0x7;

    // uint8_t op2 = (insn>>4)&0x3;

    // uint8_t Ra = (insn>>12)&0xf;

    // if(((insn>>6)&0x3)!=0)fprintf(result,"undefined 32\n");

    // else if(op1==0 && op2==0 && Ra!=0xf){
        // fprintf(result,"MLA.W 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 228;
		// cycle_cnt +=1;
    // }

    // else if(op1==0 && op2==0 && Ra==0xf){
        // fprintf(result,"MUL.W 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 229;
		// cycle_cnt +=1;
    // }

    // else if(op1==0 && op2==1){
        // fprintf(result,"MLS.W 32\n");
		// i_code.vartype = 22;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 230;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error 32\n");

// }

// void table32_20(uint32_t insn, uint16_t insn_hw1){//long multiply, long multiply accumulate, and divide

    // uint8_t op1 = (insn_hw1>>4)&0x7;

    // uint8_t op2 = (insn>>4)&0xf;

    // if(op1==0 && op2==0x0){
        // fprintf(result,"SMULL.W 32\n");
		// i_code.vartype = 23;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 231;
		// cycle_cnt +=1;
    // }

    // else if(op1==1 && op2==0xf){
        // fprintf(result,"SDIV.W 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 232;
		// cycle_cnt +=1;
    // }

    // else if(op1==2 && op2==0x0){
        // fprintf(result,"UMULL.W 32\n");
		// i_code.vartype = 23;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 233;
		// cycle_cnt +=1;
    // }

    // else if(op1==3 && op2==0xf){
        // fprintf(result,"UDIV.W 32\n");
		// i_code.vartype = 21;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 234;
		// cycle_cnt +=1;
    // }

    // else if(op1==4 && op2==0x0){
        // fprintf(result,"SMLAL.W 32\n");
		// i_code.vartype = 23;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 235;
		// cycle_cnt +=1;
    // }

    // else if(op1==6 && op2==0x0){
        // fprintf(result,"UMLAL.W 32\n");
		// i_code.vartype = 23;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 236;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error 32\n");

// }

// void table32_21(uint32_t insn, uint16_t insn_hw1){//coprocessor instructions 2

    // char in[6];

    // uint16_t op=(insn&0x10)>>4;

    // uint16_t op1=(insn_hw1>>4)&0x3f;

    // //itoa(op1,in,2);

    // sprintf(in,"%b",op1);
    // if(in[0]==0&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        // fprintf(result,"STC2\n");//111 111 0xxxx0
		// i_code.vartype = 14;
        // regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 237;
		// cycle_cnt +=1;
    // }

    // else if(in[0]==1&&in[0]==0&&!(in[1]==0&&in[3]==0&&in[4]==0&&in[5]==0)){
        // fprintf(result,"LDC2 imm\n");//111 111 0xxxx1  rn ='1111' literal
        // i_code.vartype = 14;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 238;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x4){
        // fprintf(result,"MCRR2\n");//111 111 000100
        // i_code.vartype = 15;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 239;
		// cycle_cnt +=1;
    // }

    // else if(op1==0x5){
        // fprintf(result,"MRRC2\n");//111 111 000101
        // i_code.vartype = 15;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 240;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && op ==0){
        // fprintf(result,"CDP2\n");//111 111 10xxxx op=0
		// i_code.code = 241;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && in[0]==0 && op==1){
        // fprintf(result,"MCR2\n");//111 111 10xxx0 op=1
        // i_code.vartype = 16;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 242;
		// cycle_cnt +=1;
    // }

    // else if((op1>>4)==0x2 && in[0]==1 && op==1){
        // fprintf(result,"MRC2\n");//111 111 10xxx1 op=1
        // i_code.vartype = 16;
		// regimmVal32(i_code.vartype,insn_hw1,insn);
		// i_code.code = 243;
		// cycle_cnt +=1;
    // }

    // else fprintf(result,"error\n");

// }

// void table32_3(uint32_t insn, uint16_t insn_hw1){//111 11 xxxxxx

    // uint8_t op2 = (insn_hw1>>4)&0x7f;

    // if((op2&0x71)==0)table32_14(insn, insn_hw1);//op2=000x xx0

    // else if((op2&0x67)==1)table32_15(insn, insn_hw1); //op2=00xx 001

    // else if((op2&0x67)==3)table32_16(insn, insn_hw1); //op2=00xx 011

    // else if((op2&0x67)==5)table32_17(insn, insn_hw1); //op2=00xx 101

    // else if((op2&0x67)==7)fprintf(result,"undefined 32\n"); //op2=00xx 111

    // else if(op2 >>4==2)table32_18(insn, insn_hw1); //op2=010x xxx

    // else if(op2>>3 ==6)table32_19(insn, insn_hw1); //op2=0110 xxx

    // else if(op2>>3 ==7)table32_20(insn, insn_hw1); //op2=0111 xxx

    // else if(op2>>6 ==1)table32_21(insn, insn_hw1); //op2=1xxx xxx

    // else fprintf(result,"error 32\n");

// }
void table32_2(uint32_t insn, uint16_t insn_hw1){
    uint8_t op = (insn>>4)&0xf;
    if(op==0x4){
        fprintf(result,"DSB 32\n");
        i_code.code = 160;
        cycle_cnt +=4;
    }
    else if(op==0x5){
        fprintf(result,"DMB 32\n");
        i_code.code = 161;
        cycle_cnt +=4;
    }
    else if(op==0x6){
        fprintf(result,"ISB 32\n");
        i_code.code = 162;
        cycle_cnt +=4;
    }
    else fprintf(result,"error\n");
}
void table32_1(uint32_t insn, uint16_t insn_hw1){//Branch and miscellaneous control
    uint8_t op1 = (insn_hw1 >> 4)&0x7f;
    uint8_t op2 = (insn >> 12)&0x7;
    //uint8_t op2_0 = op2&0x1;
    //uint8_t op2_2 = op2>>2;
    if(op2==0x2 && op1==0x7f){
        fprintf(result,"undefined\n");
    }
    else if(op2&0x5==0/*op2_0==0&&op2_2==0*/){
        if(op1>>1==0x1C){
            fprintf(result,"MSR reg 32\n");
            i_code.vartype = 18;
            regimmVal32(i_code.vartype,insn_hw1,insn);
			i_code.code = 164;
			cycle_cnt +=4;
        }
        else if(op1==0x3B)table32_2(insn,insn_hw1);
        else if(op1>>1==0x1F){
            fprintf(result,"MRS 32\n");
            i_code.vartype = 19;
            regimmVal32(i_code.vartype,insn_hw1,insn);
			i_code.code = 165;
			cycle_cnt +=4;
        }
    }
    else if(op2&0x5==5){
        fprintf(result,"BL 32\n");
        i_code.code = 167;
        i_code.affect = 0;
        cycle_cnt += 3;//4;

    }
    else fprintf(result,"error\n");

}
uint32_t table32(uint16_t insn_hw1,uint32_t insn){

    //uint32_t insn;

    //insn=insn_op2;

    //scanf("%x",&insn);
    //uint8_t op = insn>>15;
    //insn |= (uint32_t)insn_hw1 <<16;

    uint16_t op1 = insn_hw1>>11;

    if(op1&0x1==1)printf("undefined\n");
    else if(op1==0x1E && op==0)printf("undefined\n");
    else if(op1==0x1E && op==1)table32_1(insn,insn_hw1);
    else printf("error\n");
    /*uint16_t op1=insn_hw1>>11;

    if(op1==0x1D)table32_1(insn,insn_hw1);//111 01 xxx

    else if(op1==0x1E)table32_2(insn,insn_hw1);//111 10 xxx

    else if(op1==0x1F)table32_3(insn,insn_hw1);//111 11 xxx

    else fprintf(result,"error");*/

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
int readinsn(){
    uint32_t insn;
    strcpy(buff,"");

    fgets(buff,sizeof(buff),fptr);

#if 0
    if(buff[0]=='\n' ||buff[2]!=' ')return 0;//continue;

    //fwrite(buff,sizeof(buff),1,stdout);
	count++;
	printf("\n%u ",count);
    if(buff[16]!=' '){

        sscanf(buff,"    %x:	%x %x%s",&tmp,&insn_op1,&insn_op2,&insn_name);//sscanf(buff,"\000  %d:\t%x %x",&tmp,&op1,&op2);//printf("%c%c%c%c\n",buff[15],buff[16],buff[17],buff[18]);
        //printf("%x %x %s\n",insn_op1,insn_op2,insn_name);
	//to_upper(insn_name);
	//count++;
        printf("%s ",to_upper(insn_name));
    }

    else {
	//count++;
        sscanf(buff,"%x:   %x%s",&tmp,&insn_op1,&insn_name);
        //printf("%x %s\n",insn_op1,insn_name);
	//to_upper(insn_name);
	printf("%s ",to_upper(insn_name));
    }
#endif
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
        fgets(buff,sizeof(buff),fptr);
    }
    sscanf(buff,"%x",&insn);
    fprintf(result,"op1 = %X ",insn);
    if(checkfor32(insn)){
            insn_op1 = insn;
            strcpy(buff,"");
            fgets(buff,sizeof(buff),fptr);
            sscanf(buff,"%x",&insn);
            fprintf(result,"op2 = %X ",insn);
            insn_op2 = insn;
    }
    else insn_op1 = insn;
#else
    sscanf(buff,"%x,%x\n",&tmp,&insn);
    fprintf(result,"opcode = %X ",insn);

    if(checkfor32(insn>>16)){
            insn_op1 = insn>>16;
            insn_op2 = insn&0xFFFF;
    }
    else insn_op1 = insn;
    //fprintf(result,"op1 = %X ",insn_op1);
#endif
#endif // 1
    return 1;

}
void checkinsn(uint16_t insn){
    uint16_t op_class = 0;
    op_class = insn >> 10 ;
    switch(op_class>>4){
        case 0x0:  //00xxxx
            table_1(insn&0x3fff);
            break;
        case 0x1:  //01xxxx
            table_2(insn&0x3fff);
            break;
        case 0x2:  //10xxxx
            table_3(insn&0x3fff);
            break;
        case 0x3:  //11xxxx
            table_4(insn&0x3fff);
            break;
        default:
            printf("wrong instruction\n");
            break;
    }
}
void checkdependency(uint8_t *imm, uint8_t *Rm, uint8_t *Rd, uint8_t *Rn, uint8_t *Rt){
    printf("imm = %x, Rm = %x, Rd = %x, Rn = %x, Rt = %x\n",imm, Rm, Rd, Rn, Rt);
	//fprintf(result,"imm = %x, Rm = %x, Rd = %x, Rn = %x, Rt = %x\n",imm, Rm, Rd, Rn, Rt);
}
int read_table(uint32_t insn){

    //uint16_t insn=0xbb7b;
    //uint32_t insn_last = 0;
    //uint32_t insn_32 = 0;
    int is32bit = 0;
	//i_code.affect = 1;
	//fptr = fopen("Trace3.csv","r");
#ifdef QEMU_TEST
	//fptr = fopen(filename/*"run_test"*/,"r");

	//result = fopen("test2","w");
#else
    //result = fopen("result2","w");
#endif
	/*if(!fptr){
            printf("failed to open file\n");
            exit(1);
	}*/
	//fprintf(result,"===================\n");
    //while(!feof(fptr)){

        //if(!readinsn())continue;
        //readinsn();
        //insn=insn_op1;
        if(checkfor32(insn)){
            insn_32 = table32(insn);
            is32bit = 1;
        }
        else {
            is32bit = 0;
            checkinsn(insn);
        }
        /*if(i_code.affect == 1){
			if(checkLast(i_code_t)){
				printf("type = %d\n",i_code.vartype);
				//fprintf(result,"type = %d\n",i_code.vartype);
				//regs(i_code, i_code_t);
			}
        }
		i_code_t.code = i_code.code;
		i_code_t.vartype = i_code.vartype;
		i_code_t.affect = i_code.affect;
        i_code.affect = 1;*/
        //fprintf(result,"code num = %d\n",i_code.code);
        /*fprintf(result,"Cycle: %d\n",cycle_cnt);
        fprintf(result,"===================\n");
	}
	printf("cycle count = %u\n",cycle_cnt);
	fprintf(result,"cycle count = %u\n",cycle_cnt);*/

        return cycle_cnt;
}
