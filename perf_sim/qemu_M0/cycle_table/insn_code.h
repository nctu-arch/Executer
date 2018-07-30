//16 bit
#define ADD_reg  0
#define SUB_reg  1
#define ADD_imm3 2
#define SUB_imm3 3
#define LSL_imm  4
#define LSR_imm  5
#define ASR_imm  6
#define MOV_imm  7
#define CMP_imm  8
#define ADD_imm8 9
#define SUB_imm8 10
#define AND_reg  11
#define EOR_reg  12
#define LSL_reg  13
#define LSR_reg  14
#define ASR_reg  15
#define ADC_reg  16
#define SBC_reg  17
#define ROR_reg  18
#define TST_reg  19
#define RSB_imm  20
#define CMP_reg  21
#define CMN_reg  22
#define ORR_reg  23
#define MUL_reg  24
#define BIC_reg  25
#define MVN_reg  26
#define LDR_imm  27     // cycle 2
#define ADD_reg2 28 //table_8
#define CMP_reg2 29 //table_8
#define MOV_reg  30
#define BX       31 //table_8
#define BLX_reg  32 //table_8  cycle 12
#define STR_imm  33
#define LDR_imm2 34 //table_6
#define STRB_imm 35
#define LDRB_imm 36
#define STR_reg  37
#define STRH_reg 38
#define STRB_reg 39
#define LDRSB_reg 40
#define LDR_reg  41
#define LDRH_reg 42
#define LDRB_reg 43
#define LDRSH_reg 44
#define STRH_imm 45
#define LDRH_imm 46
#define STR_sp_i 47
#define LDR_sp_i 48
#define IT       49
#define NOP      50
#define YIELD    51
#define WFE      52
#define WFI      53
#define SEV      54
#define CBZ      55 //table_15
#define SUB_sp   56
#define ADD_sp   57
#define CBZ2     58 //table_16
#define SXTH     59
#define SXTB     60
#define UXTH     61
#define UXTB     62
#define PUSH16   63 //table_17
#define CBNZ     64 //table_18
#define CBNZ2    65 //table_19
#define REVSH    66
#define REV16    67
#define REV      68
#define POP16    69 //table_20
#define CPS      70 //table_12
#define BRKT     71 //table_12
#define ADR      72
#define ADD_sp2  73 //table_14
#define SVC      74 //table_22
#define B        75 //table_22
#define STM      76
#define LDM      77
#define B2       78 //table_4
//32 bit
#define STC      79
#define LDC_imm  80
#define MCRR     81
#define MRRC     82
#define CDP      83
#define MCR      84
#define MRC      85
#define MOVw_reg 86
#define LSLw_imm 87
#define LSRw_imm 88
#define ASRw_imm 89
#define RRXw_imm 90
#define RORw_imm 91
#define ANDw_reg 92
#define TSTw_reg 93
#define BICw_reg 94
#define ORRw_reg 95
#define ORNw_reg 96
#define MVNw_reg 97
#define EORw_reg 98
#define TEQw_reg 99
#define ADDw_reg 100
#define CMNw_reg 101
#define ADCw_reg 102
#define SBCw_reg 103
#define SUBw_reg 104
#define CMPw_reg 105
#define RSBw_reg 106
#define STMw     107
#define LDMw     108
#define POPw     109
#define STMDBw   110
#define PUSHw    111
#define LDMDBw   112
#define STREXw   113
#define LDREXw   114
#define STRDw_i1 115
#define STRDw_i2 116
#define LDRDw_i  117
#define LDRDw_l  118
#define STREXB   119
#define STREXH   120
#define TBB      121
#define TBH      122
#define LDREXB   123
#define LDREXH   124
#define ANDw_imm 125
#define TSTw_imm 126
#define BICw_imm 127
#define ORRw_imm 128
#define MOVw_imm 129
#define ORNw_imm 130
#define MVNw_imm 131
#define EORw_imm 132
#define TEQw_imm 133
#define ADDw_imm 134
#define CMNw_imm 135
#define ADCw_imm 136
#define SBCw_imm 137
#define SUBw_imm 138
#define CMPw_imm 139
#define RSBw_imm 140
#define ADDw_i12 141 //table32_10
#define ADRw_pc1 142 //table32_10-1
#define MOVw_i16 143 //table32_10
#define SUBw_i12 144 //table32_10
#define ADRw_pc2 145 //table32_10-2
#define MOVTw    146
#define SSAT     147
#define SBFX     148
#define BFI      149
#define BFC      150
#define USAT     151
#define UBFX     152
#define NOPw     153
#define YIELDw   154
#define WFEw     155
#define WFIw     156
#define SEVw     157
#define DBGw     158
#define CLREX    159
#define DSB      160
#define DMB      161
#define ISB      162
#define Bw1      163 //table32_11-1
#define MSR_reg  164
#define MRS_reg  165
#define Bw2      166 //table32_11-2
#define BLw      167
#define STRB_i12 168
#define STRB_i8  169
#define STRBw_reg 170
#define STRH_i12 171
#define STRH_i8  172
#define STRHw_reg 173
#define STR_i12  174
#define STR_i8   175
#define STRw_reg 176
#define LDRBw_i12  177
#define LDRBw_i81  178
#define LDRBw_i82  179
#define LDRBTw     180
#define LDRBw_l    181
#define LDRBw_reg  182
#define LDRSBw_i12 183
#define LDRSBw_i81 184
#define LDRSBw_i82 185
#define LDRSBTw    186
#define LDRSBw_l   187
#define LDRSBw_reg 188
#define PLD_imm12  189
#define PLD_imm8   190
#define PLD_label  191
#define PLD_reg    192
#define PLI_imm12  193
#define PLI_imm8   194
#define PLI_label  195
#define PLI_reg    196
#define LDRHw_i12  197
#define LDRHw_i81  198
#define LDRHw_i82  199
#define LDRHTw     200
#define LDRHw_l    201
#define LDRHw_reg  202
#define LDRSHw_i12 203
#define LDRSHw_i81 204
#define LDRSHw_i82 205
#define LDRSHTw    206
#define LDRSHw_l   207
#define LDRSHw_reg 208
#define LDRw_imm12 209
#define LDRw_imm81 210
#define LDRw_imm82 211
#define LDRTw      212
#define LDRw_reg   213
#define LDRw_l     214
#define REVw    215
#define REV16w  216
#define RBIT    217
#define REVSHw  218
#define CLZ     219
#define LSLw_reg   220
#define LSRw_reg   221
#define ASRw_reg   222
#define RORw_reg   223
#define SXTHw      224
#define UXTHw      225
#define SXTBw      226
#define UXTBw      227
#define MLA        228
#define MULw       229
#define MLS        230
#define SMULL      231
#define SDIV       232
#define UMULL      233
#define UDIV       234
#define SMLAL      235
#define UMLAL      236
#define STC2       237
#define LDC2       238
#define MCRR2      239
#define MRRC2      240
#define CDP2       241
#define MCR2       242
#define MRC2       243
struct dep{
	int code;
	int affect;
	int vartype;
};
int checkLast(struct dep insn);
int regs(struct dep insn, struct dep insn_t);
void regVal(int type, int n);
void regVal32(int type, int n);
int varVal(int type);
int varVal32(int type);

