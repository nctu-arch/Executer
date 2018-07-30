#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "insn_code.h"
int var2flag = 0;
extern uint16_t imm,Rm,Rd,Rn,Rt;
extern uint16_t Ra, Rt2, Rlist;
extern uint16_t RdLo, RdHi;
extern FILE *result;
uint8_t reg_record[300]={0};
extern cycle_cnt;
void checkdep(uint8_t *imm, uint8_t *Rm, uint8_t *Rd, uint8_t *Rn, uint8_t *Rt, int *n, int *n2, int *var2flag){
    //printf("imm = %x, Rm = %x, Rd = %x, Rn = %x, Rt = %x, n = %d, n2 = %d, var2flag = %d\n",imm, Rm, Rd, Rn, Rt, n, n2, var2flag);
	//fprintf(result,"imm = %x, Rm = %x, Rd = %x, Rn = %x, Rt = %x, n = %d, n2 = %d, var2flag = %d\n",imm, Rm, Rd, Rn, Rt, n, n2, var2flag);

}
int checkLast(struct dep insn){
	if(insn.affect){//affect
		//printf("yes\n");
		//fprintf(result,"yes\n");
		return 1;
	}
	else{
		//printf("no\n");
		//fprintf(result,"no\n");
		return -1;
	}
	return 0;
}
int regs(struct dep insn,struct dep insn_t){
	if(insn.code>78||insn.code<244){
		if(insn_t.code >= 0 &&insn_t.code <79)
			regVal32(insn.vartype,varVal(insn_t.vartype));
		else if(insn_t.code>78||insn_t.code<244)
			regVal32(insn.vartype,varVal32(insn_t.vartype));
		else printf("errrrrrrrr\n");
	}
	else if(insn.code >= 0 &&insn.code <79){
		if(insn_t.code >= 0 &&insn_t.code <79)
			regVal(insn.vartype,varVal(insn_t.vartype));
		else if(insn_t.code>78||insn_t.code<244)
			regVal(insn.vartype,varVal32(insn_t.vartype));
		else printf("eeeeeeeeerrrrrrrrrrrrr\n");
	}
	else printf("error hohohohohoho\n");
	return 0;
}
void regVal(int type, int n){
	int n2=-1;
	if(var2flag == 1){
		n2 = n&0xf;
		n = n>>4;
	}
    switch(type){
    case 1://imm5 Rm3 Rd3
		if(Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rd = insn&0x7;
        //Rm = (insn>>3)&0x7;
        //imm = (insn>>6)&0x1f;
        break;
    case 2: //imm8 Rd3
        if(Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rd = (insn>>8)&0x7;
        //imm = insn&0xff;
        break;
    case 3: //Rm3 Rn3 Rd3
		if(Rd==n||Rn==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2||Rn==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rd = insn&0x7;
        //Rn = (insn>>3)&0x7;
        //Rm = (insn>>6)&0x7;
        break;
    case 4: //imm3 Rn3 Rd3
		if(Rd==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rd = insn&0x7;
        //Rn = (insn>>3)&0x7;
        //imm = (insn>>6)&0x7;
        break;
    case 5: //imm8 Rn3
		if(Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rn = (insn>>8)&0x7;
        //imm = insn&0xff;
        break;
    case 6: //imm8 Rdn3
		if(Rd==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rn = (insn>>8)&0x7;
        //Rd = Rn;
        //imm = insn&0xff;
        break;
    case 7: //Rm3 Rdn
		if(Rm==n||Rd==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rm==n2||Rd==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rm = (insn>>3)&0x7;
        //Rd = insn&0x7;
        //Rn = Rd;
        break;
    case 8: //Rm3 Rn3
		if(Rm==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rm==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rm = (insn>>3)&0x7;
        //Rn = insn&0x7;
        break;
    case 9: //Rn3 Rd3
		if(Rd==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rn = (insn>>3)&0x7;
        //Rd = insn&0x7;
        break;
    case 10: //Rn3 Rdm3
		if(Rn==n||Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rn==n2||Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rn = (insn>>3)&0x7;
        //Rd = insn&0x7;
        //Rm = Rd;
        break;
    case 11: //Rm3 Rd3
		if(Rm==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rm==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rm = (insn>>3)&0x7;
        //Rd = insn&0x7;
        break;
    case 12: //Rm4 Rdn3
		if(Rm==n||Rd==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rm==n2||Rd==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rm = (insn>>3)&0xf;
        //Rd = insn&0x7;
        //Rn = Rd;
        break;
    case 13: //Rm4 Rn3
		if(Rm==n||Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rm==n2||Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rm = (insn>>3)&0xf;
        //Rn = insn&0x7;
        break;
    case 14: //Rm4 Rd3
		if(Rm==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rm==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rm = (insn>>3)&0xf;
        //Rd = insn&0x7;
        break;
    case 15: //Rm4 000
		if(Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rm = (insn>>3)&0xf;
        break;
    case 16: //imm5 Rn3 Rt3
		if(Rn==n||Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rn==n2||Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //imm = (insn>>6)&0x1f;
        //Rn = (insn>>3)&0x7;
        //Rt = insn&0x7;
        break;
    case 17: //Rm3 Rn3 Rt3
		if(Rt==n||Rn==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rt==n2||Rn==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rt = insn&0x7;
        //Rn = (insn>>3)&0x7;
        //Rm = (insn>>6)&0x7;
        break;
    case 18: //Rt3 imm8
		if(Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rt = (insn>>8)&0x7;
        //imm = insn&0xff;
        break;
    case 19: //imm5 Rn3
		if(Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rn = insn&0x7;
        //imm = (insn>>3)&0x1f;
        break;
    case 20: //imm7
        //imm = insn&0x7f;
        break;
    case 21: //Rd3 imm8
		if(Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rd = (insn>>8)&0x7;
        //imm = insn&0xff;
        break;

    }
	checkdep(imm, Rm, Rd, Rn, Rt,n,n2,var2flag);
}
void regVal32(int type,int n){
	int n2=-1;
	if(var2flag == 1){
		n2 = n&0xf;
		n =  n>>4;
	}
    switch(type){
    case 1:
	case 100:
		if(Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1&0xf;
        //Rlist = insn&0x1fff;
        break;
    case 2: //pop push
		if(Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
        //Rt = (insn>>12)&0xf;
        break;
    case 3: //Rn4 Rt4 Rd4 imm8
        if(Rn==n||Rt==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
		if(var2flag == 1){
			if(Rn==n2||Rt==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rt = (insn>>12)&0xf;
        //Rd = (insn>>8)&0xf;
        //imm = insn&0xff;
        break;
    case 4: //Rn4 Rt4 1111 imm8
		if(Rn==n||Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 &0xf;
        //Rt = (insn>>12)&0xf;
        //imm = insn & 0xff;
        break;
    case 5: //Rn4 Rt4 Rt24 imm8
		if(Rn==n||Rt==n||Rt2==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rt==n2||Rt2==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1&0xf;
        //Rt = (insn>>12)&0xf;
        //Rt2 = (insn>>8)&0xf;
        //imm = insn & 0xff;
        break;
    case 6: //Rt4 Rt24 imm8
		if(Rt==n||Rt2==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rt==n2||Rt2==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rt = (insn>>12)&0xf;
        //Rt2 = (insn>>8)&0xf;
        //imm = insn&0xff;
        break;
    case 7:  //Rn4 Rt4 1111 xxxx Rd4
		if(Rn==n||Rt==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rt==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rt = (insn>>12)&0xf;
        //Rd = insn&0xf;
        break;
    case 8: //Rn4 xxxx xxxx xxxx Rm4
		if(Rn==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rm = insn&0xf;
        break;
    case 9: // Rn4 Rt4
		if(Rn==n||Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 &0xf;
        //Rt = (insn>>12)&0xf;
        break;
    case 10:
		if(Rn==n||Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rd = (insn>>8)&0xf;
        //Rm = insn&0xf;
        //imm = (insn>>6)&0x3;
        //imm|=((insn>>12)&0x7)<<2;
        break;
    case 11:
		if(Rn==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rm = insn&0xf;
        //imm = (insn>>6)&0x3;
        //imm|=((insn>>12)&0x7)<<2;
        break;
    case 12:
		if(Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rd = (insn>>8)&0xf;
        //Rm = insn&0xf;
        //imm = (insn>>6)&0x3;
        //imm|=((insn>>12)&0x7)<<2;
        break;
    case 13:
		if(Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rd = (insn>>8)&0xf;
        //Rm = insn&0xf;
        break;
    case 14:
		if(Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 &0xf;
        //imm = insn&0xff;
        break;
    case 15:
		if(Rt==n||Rt2==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rt==n2||Rt2==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rt2 = insn_hw1&0xf;
        //Rt = (insn>>12)&0xf;
        break;
    case 16: //case 2
		if(Rt==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rt==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rt = (insn>>12)&0xf;
        break;
    case 17:
		if(Rn==n||Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rd = (insn>>8)&0xf;
        break;
    case 18:
		if(Rn==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 &0xf;
        break;
    case 19:
		if(Rd==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rd==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rd = (insn>>8)&0xf;
        break;
    case 20:
		if(Rn==n||Rt==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rt==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1&0xf;
        //Rt = (insn>>12)&0xf;
        //Rm = insn&0xf;
        break;
    case 21:
		if(Rn==n||Rd==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rd==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rd = (insn>>8)&0xf;
        //Rm = insn&0xf;
        break;
    case 22:
		if(Rn==n||Rd==n||Rm==n||Ra==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||Rd==n2||Rm==n2||Ra==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //Rd = (insn>>8)&0xf;
        //Rm = insn&0xf;
        //Ra = (insn>>12)&0xf;
        break;
    case 23:
		if(Rn==n||((RdHi<<4)|RdLo)==n||Rm==n){
			fprintf(result,"hazard %c\n",7);
			cycle_cnt +=1;
		}
		else fprintf(result,"pass\n");
        if(var2flag == 1){
			if(Rn==n2||((RdHi<<4)|RdLo)==n2||Rm==n2){
				fprintf(result,"hazard %c\n",7);
				cycle_cnt +=1;
			}
			else fprintf(result,"pass\n");
		}
		//Rn = insn_hw1 & 0xf;
        //RdHi = (insn>>8)&0xf;
        //Rm = insn&0xf;
        //RdLo = (insn>>12)&0xf;
        break;
    }
	checkdep(imm, Rm, Rd, Rn, Rt,n,n2,var2flag);
}
int varVal(int type){
	var2flag = 0;
    //uint8_t imm=-1,Rm=-1,Rd=-1,Rn=-1,Rt=-1;
	int n=-1;
    switch(type){
    case 1://imm5 Rm3 Rd3
		n = Rd;
        //Rd = insn&0x7;
        //Rm = (insn>>3)&0x7;
        //imm = (insn>>6)&0x1f;
        break;
    case 2: //imm8 Rd3
		n = Rd;
        //Rd = (insn>>8)&0x7;
        //imm = insn&0xff;
        break;
    case 3: //Rm3 Rn3 Rd3
		n = Rd;
        //Rd = insn&0x7;
        //Rn = (insn>>3)&0x7;
        //Rm = (insn>>6)&0x7;
        break;
    case 4: //imm3 Rn3 Rd3
		n = Rd;
        // Rd = insn&0x7;
        // Rn = (insn>>3)&0x7;
        // imm = (insn>>6)&0x7;
        break;
    case 5: //imm8 Rn3
		//n = Rd;
        // Rn = (insn>>8)&0x7;
        // imm = insn&0xff;
        break;
    case 6: //imm8 Rdn3
		n = Rd;
        // Rn = (insn>>8)&0x7;
        // Rd = Rn;
        // imm = insn&0xff;
        break;
    case 7: //Rm3 Rdn
		n = Rd;
        // Rm = (insn>>3)&0x7;
        // Rd = insn&0x7;
        // Rn = Rd;
        break;
    case 8: //Rm3 Rn3
		//n = Rd;
        // Rm = (insn>>3)&0x7;
        // Rn = insn&0x7;
        break;
    case 9: //Rn3 Rd3
		n = Rd;
        // Rn = (insn>>3)&0x7;
        // Rd = insn&0x7;
        break;
    case 10: //Rn3 Rdm3
		n = Rd;
        // Rn = (insn>>3)&0x7;
        // Rd = insn&0x7;
        // Rm = Rd;
        break;
    case 11: //Rm3 Rd3
		n = Rd;
        // Rm = (insn>>3)&0x7;
        // Rd = insn&0x7;
        break;
    case 12: //Rm4 Rdn3
		n = Rd;
        // Rm = (insn>>3)&0xf;
        // Rd = insn&0x7;
        // Rn = Rd;
        break;
    case 13: //Rm4 Rn3
		//n = Rd;
        // Rm = (insn>>3)&0xf;
        // Rn = insn&0x7;
        break;
    case 14: //Rm4 Rd3
		n = Rd;
        // Rm = (insn>>3)&0xf;
        // Rd = insn&0x7;
        break;
    case 15: //Rm4 000
		//n = Rd;
        // Rm = (insn>>3)&0xf;
        break;
    case 16: //imm5 Rn3 Rt3
		n = Rt;
		reg_record[n]++;
        // imm = (insn>>6)&0x1f;
        // Rn = (insn>>3)&0x7;
        // Rt = insn&0x7;
        break;
    case 17: //Rm3 Rn3 Rt3
		n = Rt;
		reg_record[n]++;
        // Rt = insn&0x7;
        // Rn = (insn>>3)&0x7;
        // Rm = (insn>>6)&0x7;
        break;
    case 18: //Rt3 imm8
		n = Rt;
		reg_record[n]++;
        // Rt = (insn>>8)&0x7;
        // imm = insn&0xff;
        break;
    case 19: //imm5 Rn3
		//n = Rd;
        // Rn = insn&0x7;
        // imm = (insn>>3)&0x1f;
        break;
    case 20: //imm7
        // imm = insn&0x7f;
        break;
    case 21: //Rd3 imm8
		n = Rd;
        // Rd = (insn>>8)&0x7;
        // imm = insn&0xff;
        break;

    }
	//printf("type = %d, n = %d\n",type,n);
	//fprintf(result,"type = %d, n = %d\n",type,n);
	return n;
}
int varVal32(int type){
	var2flag = 0;
    int n=-1;
    switch(type){
    case 1:
		n = Rn;
		reg_record[n]++;
        // Rn = insn_hw1&0xf;
        // Rlist = insn&0x1fff;
        break;
    case 2: //pop push
		n = Rt;
		reg_record[n]++;
        // Rt = (insn>>12)&0xf;
        break;
    case 3: //Rn4 Rt4 Rd4 imm8
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rt = (insn>>12)&0xf;
        // Rd = (insn>>8)&0xf;
        // imm = insn&0xff;
        break;
    case 4: //Rn4 Rt4 1111 imm8
		n = Rt;
		reg_record[n]++;
        // Rn = insn_hw1 &0xf;
        // Rt = (insn>>12)&0xf;
        // imm = insn & 0xff;
        break;
    case 5: //Rn4 Rt4 Rt24 imm8
		n = (Rt<<4)|Rt2;
		reg_record[n]++;
        // Rn = insn_hw1&0xf;
        // Rt = (insn>>12)&0xf;
        // Rt2 = (insn>>8)&0xf;
        // imm = insn & 0xff;
		var2flag = 1;
        break;
    case 6: //Rt4 Rt24 imm8
		n = (Rt<<4)|Rt2;
		reg_record[n]++;
        // Rt = (insn>>12)&0xf;
        // Rt2 = (insn>>8)&0xf;
        // imm = insn&0xff;
		var2flag = 1;
        break;
    case 7:  //Rn4 Rt4 1111 xxxx Rd4
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rt = (insn>>12)&0xf;
        // Rd = insn&0xf;
        break;
    case 8: //Rn4 xxxx xxxx xxxx Rm4
		//n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rm = insn&0xf;
        break;
    case 9: // Rn4 Rt4
		n = Rt;
		reg_record[n]++;
        // Rn = insn_hw1 &0xf;
        // Rt = (insn>>12)&0xf;
        break;
    case 10:
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rd = (insn>>8)&0xf;
        // Rm = insn&0xf;
        // imm = (insn>>6)&0x3;
        // imm|=((insn>>12)&0x7)<<2;
        break;
    case 11:
		//n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rm = insn&0xf;
        // imm = (insn>>6)&0x3;
        // imm|=((insn>>12)&0x7)<<2;
        break;
    case 12:
		n = Rd;
        // Rd = (insn>>8)&0xf;
        // Rm = insn&0xf;
        // imm = (insn>>6)&0x3;
        // imm|=((insn>>12)&0x7)<<2;
        break;
    case 13:
		n = Rd;
        // Rd = (insn>>8)&0xf;
        // Rm = insn&0xf;
        break;
    case 14:
		n = Rd;
        // Rn = insn_hw1 &0xf;
        // imm = insn&0xff;
        break;
    case 15:
		n = (Rt<<4)|Rt2;
		var2flag = 1;
		reg_record[n]++;
        // Rt2 = insn_hw1&0xf;
        // Rt = (insn>>12)&0xf;
        break;
    case 16: //case 2
		n = Rt;
		reg_record[n]++;
        // Rt = (insn>>12)&0xf;
        break;
    case 17:
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rd = (insn>>8)&0xf;
        break;
    case 18:
		//n = Rd;
        // Rn = insn_hw1 &0xf;
        break;
    case 19:
		n = Rd;
        // Rd = (insn>>8)&0xf;
        break;
    case 20:
		n = Rt;
		reg_record[n]++;
        // Rn = insn_hw1&0xf;
        // Rt = (insn>>12)&0xf;
        // Rm = insn&0xf;
        break;
    case 21:
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rd = (insn>>8)&0xf;
        // Rm = insn&0xf;
        break;
    case 22:
		n = Rd;
        // Rn = insn_hw1 & 0xf;
        // Rd = (insn>>8)&0xf;
        // Rm = insn&0xf;
        // Ra = (insn>>12)&0xf;
        break;
    case 23:
		n = (RdHi<<16)|RdLo;
		reg_record[n]++;
        // Rn = insn_hw1 & 0xf;
        // RdHi = (insn>>8)&0xf;
        // Rm = insn&0xf;
        // RdLo = (insn>>12)&0xf;
        break;
	case 100://Rn not writeback
		break;
    }
	//printf("type = %d, n = %d\n",type,n);
	//fprintf(result,"type = %d, n = %d\n",type,n);
	return n;
}
