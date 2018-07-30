#include "insn_code.h"
int checkLast(struct dep i_code){
    if(i_code.affect){
        printf("yes\n");
    }
    else {
        printf("no\n");
    }
    return 0;
}
void regs(struct dep insn, struct dep insn_t){
    regVal(insn.vartype,varVal(insn_t.vartype,insn_t),insn);
}
void regimmVal(/*int type,*/ uint32_t insn, struct dep i_code){
	//uint8_t Rd, Rn, Rm, Rs, Ra, Rt, Rt2;
	//uint8_t RdHi, RdLo;
	//uint16_t Rlist;
	int type = i_code.vartype;
	switch(type){
		case 1: // Rn4 Rd4 Rm4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 2: // Rn4 Rm4
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 3: // Rd4 Rm4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 4: // Rn4 Rd4 Rs4 Rm4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rs = (insn>>8)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 5: // Rn4 Rs4 Rm4
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rs = (insn>>8)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 6: // Rd4 Rm4 Rn4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rm = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 7: // Rd4 Rs4 Rn4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rs = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 8: // Rd4
			i_code.Rd = (insn>>12)&0xF;
		break;
		case 9: // Rn4
			i_code.Rn = insn&0xF;
		break;
		case 10: // Rm4
			i_code.Rm = insn&0xF;
		break;
		case 11: // Rd4 Ra4 Rm4 Rn4
			i_code.Ra = (insn>>12)&0xF;
			i_code.Rd = (insn>>16)&0xF;
			i_code.Rm = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 12: // Rd4 Rm4 Rn4
			i_code.Rd = (insn>>16)&0xF;
			i_code.Rm = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 13: // RdHi4 RdLo4 Rm4 Rn4
			i_code.RdLo = (insn>>12)&0xF;
			i_code.RdHi = (insn>>16)&0xF;
			i_code.Rm = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 14: // Rd4 Rm4 Rn4
			i_code.Rd = (insn>>16)&0xF;
			i_code.Rm = (insn>>8)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 15: // Rn4 Rt4 Rt24
			i_code.Rt = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rt2 = insn&0xF;
		break;
		case 16: // Rn4 Rd4 Rt4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rt = insn&0xF;
		break;
		case 17: // Rn4 Rt4
			i_code.Rt = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
		break;
		case 18: // Rn4 Rt4 Rm4
			i_code.Rt = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rm = insn&0xF;
		break;
		case 19: // Rt4
			i_code.Rt = (insn>>12)&0xF;
		break;
		case 20: // Rn4 Rd4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rn = (insn>>16)&0xF;
		break;
		case 21: // Rn4
			i_code.Rn = (insn>>16)&0xF;
		break;
		case 22: // Rd4 Rn4
			i_code.Rd = (insn>>12)&0xF;
			i_code.Rn = insn&0xF;
		break;
		case 23: // Rn4 Rlist16
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rlist = insn&0xFFFF;
		break;
		case 24: // Rn4 Rlist15
			i_code.Rn = (insn>>16)&0xF;
			i_code.Rlist = insn&0x7FFF;
		break;
		case 25: // Rt24 Rt4
			i_code.Rt2 = (insn>>16)&0xF;
			i_code.Rt = (insn>>12)&0xF;
		break;
		default:
			i_code.Rd=0;i_code.Rn=0; i_code.Rm=0; i_code.Rs=0; i_code.Ra=0; i_code.Rt=0; i_code.Rt2=0;
	        i_code.RdHi=0;	i_code.RdLo=0;
			i_code.Rlist=0;
		break;
	}
}
int varVal(int type, struct dep i_code_t){
	uint8_t Rd, Rn, Rm, Rs, Ra, Rt, Rt2;
	uint8_t RdHi, RdLo;
	uint16_t Rlist;
	int n;
	switch(type){
		case 1: // Rn4 Rd4 Rm4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rm = insn&0xF;
		break;
		case 2: // Rn4 Rm4
			//Rn = (insn>>16)&0xF;
			//Rm = insn&0xF;
		break;
		case 3: // Rd4 Rm4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rm = insn&0xF;
		break;
		case 4: // Rn4 Rd4 Rs4 Rm4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rs = (insn>>8)&0xF;
			// Rm = insn&0xF;
		break;
		case 5: // Rn4 Rs4 Rm4
			//Rn = (insn>>16)&0xF;
			//Rs = (insn>>8)&0xF;
			//Rm = insn&0xF;
		break;
		case 6: // Rd4 Rm4 Rn4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 7: // Rd4 Rs4 Rn4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rs = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 8: // Rd4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
		break;
		case 9: // Rn4
			//Rn = insn&0xF;
		break;
		case 10: // Rm4
			//Rm = insn&0xF;
		break;
		case 11: // Rd4 Ra4 Rm4 Rn4
			n = i_code_t.Rd;
			// Ra = (insn>>12)&0xF;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 12: // Rd4 Rm4 Rn4
			n = i_code_t.Rd;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 13: // RdHi4 RdLo4 Rm4 Rn4
			//RdLo = (insn>>12)&0xF;
			//RdHi = (insn>>16)&0xF;
			//Rm = (insn>>8)&0xF;
			//Rn = insn&0xF;
		break;
		case 14: // Rd4 Rm4 Rn4
			n = i_code_t.Rd;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 15: // Rn4 Rt4 Rt24
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
			//Rt2 = insn&0xF;
		break;
		case 16: // Rn4 Rd4 Rt4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rt = insn&0xF;
		break;
		case 17: // Rn4 Rt4
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
		break;
		case 18: // Rn4 Rt4 Rm4
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
			//Rm = insn&0xF;
		break;
		case 19: // Rt4
			//Rt = (insn>>12)&0xF;
		break;
		case 20: // Rn4 Rd4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
		break;
		case 21: // Rn4
			//Rn = (insn>>16)&0xF;
		break;
		case 22: // Rd4 Rn4
			n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = insn&0xF;
		break;
		case 23: // Rn4 Rlist16
			//Rn = (insn>>16)&0xF;
			//Rlist = insn&0xFFFF;
		break;
		case 24: // Rn4 Rlist15
			//Rn = (insn>>16)&0xF;
			//Rlist = insn&0x7FFF;
		break;
		case 25: // Rt24 Rt4
			//Rt2 = (insn>>16)&0xF;
			//Rt = (insn>>12)&0xF;
		break;
		default:
			//i_code_t.Rd=0;
			//i_code_t.Rn=0;
			//i_code_t.Rm=0;
			//i_code_t.Rs=0;
			//i_code_t.Ra=0;
			//i_code_t.Rt=0;
			//i_code_t.Rt2=0;
	        //i_code_t.RdHi=0;
	        //i_code_t.RdLo=0;
			//i_code_t.Rlist=0;
			n= 0;
		break;
	}
	return n;
}
void regVal(int type, int n, struct dep i_code){
	uint8_t Rd, Rn, Rm, Rs, Ra, Rt, Rt2;
	uint8_t RdHi, RdLo;
	uint16_t Rlist;
	switch(type){
		case 1: // Rn4 Rd4 Rm4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rm = insn&0xF;
		break;
		case 2: // Rn4 Rm4
		    if(i_code.Rn==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//Rn = (insn>>16)&0xF;
			//Rm = insn&0xF;
		break;
		case 3: // Rd4 Rm4
		    if(i_code.Rd==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rm = insn&0xF;
		break;
		case 4: // Rn4 Rd4 Rs4 Rm4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rs==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rs = (insn>>8)&0xF;
			// Rm = insn&0xF;
		break;
		case 5: // Rn4 Rs4 Rm4
		    if(i_code.Rn==n||i_code.Rs==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//Rn = (insn>>16)&0xF;
			//Rs = (insn>>8)&0xF;
			//Rm = insn&0xF;
		break;
		case 6: // Rd4 Rm4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 7: // Rd4 Rs4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rs==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rs = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 8: // Rd4
		    if(i_code.Rd==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
		break;
		case 9: // Rn4
		    if(i_code.Rn==n){
                printf("hazard\n");
		    }
			//Rn = insn&0xF;
		break;
		case 10: // Rm4
		    if(i_code.Rm==n){
                printf("hazard\n");
		    }
			//Rm = insn&0xF;
		break;
		case 11: // Rd4 Ra4 Rm4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Ra==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Ra = (insn>>12)&0xF;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 12: // Rd4 Rm4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 13: // RdHi4 RdLo4 Rm4 Rn4
		    if(i_code.Rn==n||i_code.RdHi==n||i_code.RdLo==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//RdLo = (insn>>12)&0xF;
			//RdHi = (insn>>16)&0xF;
			//Rm = (insn>>8)&0xF;
			//Rn = insn&0xF;
		break;
		case 14: // Rd4 Rm4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>16)&0xF;
			// Rm = (insn>>8)&0xF;
			// Rn = insn&0xF;
		break;
		case 15: // Rn4 Rt4 Rt24
		    if(i_code.Rn==n||i_code.Rt==n||i_code.Rt2==n){
                printf("hazard\n");
		    }
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
			//Rt2 = insn&0xF;
		break;
		case 16: // Rn4 Rd4 Rt4
		    if(i_code.Rn==n||i_code.Rd==n||i_code.Rt==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
			// Rt = insn&0xF;
		break;
		case 17: // Rn4 Rt4
		    if(i_code.Rn==n||i_code.Rt==n){
                printf("hazard\n");
		    }
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
		break;
		case 18: // Rn4 Rt4 Rm4
		    if(i_code.Rn==n||i_code.Rt==n||i_code.Rm==n){
                printf("hazard\n");
		    }
			//Rt = (insn>>12)&0xF;
			//Rn = (insn>>16)&0xF;
			//Rm = insn&0xF;
		break;
		case 19: // Rt4
		    if(i_code.Rt==n){
                printf("hazard\n");
		    }
			//Rt = (insn>>12)&0xF;
		break;
		case 20: // Rn4 Rd4
		    if(i_code.Rn==n||i_code.Rd==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = (insn>>16)&0xF;
		break;
		case 21: // Rn4
		    if(i_code.Rn==n){
                printf("hazard\n");
		    }
			//Rn = (insn>>16)&0xF;
		break;
		case 22: // Rd4 Rn4
		    if(i_code.Rn==n||i_code.Rd==n){
                printf("hazard\n");
		    }
			//n = i_code_t.Rd;
			// Rd = (insn>>12)&0xF;
			// Rn = insn&0xF;
		break;
		case 23: // Rn4 Rlist16
		    if(i_code.Rn==n||i_code.Rlist==n){
                printf("hazard\n");
		    }
			//Rn = (insn>>16)&0xF;
			//Rlist = insn&0xFFFF;
		break;
		case 24: // Rn4 Rlist15
		    if(i_code.Rn==n||i_code.Rlist==n){
                printf("hazard\n");
		    }
			//Rn = (insn>>16)&0xF;
			//Rlist = insn&0x7FFF;
		break;
		case 25: // Rt24 Rt4
		    if(i_code.Rt==n||i_code.Rt2==n){
                printf("hazard\n");
		    }
			//Rt2 = (insn>>16)&0xF;
			//Rt = (insn>>12)&0xF;
		break;
		default:
			//i_code_t.Rd=0;
			//i_code_t.Rn=0;
			//i_code_t.Rm=0;
			//i_code_t.Rs=0;
			//i_code_t.Ra=0;
			//i_code_t.Rt=0;
			//i_code_t.Rt2=0;
	        //i_code_t.RdHi=0;
	        //i_code_t.RdLo=0;
			//i_code_t.Rlist=0;
			n= 0;
		break;
	}
}