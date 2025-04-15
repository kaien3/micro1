/*
 *
 *      MICRO-1 micro-assembler (ANSI C-version)
 *      translated from Pascal-version by kim   Ver. 1.0  2016.4
 *
 *      based on MICRO-1 MICROASSEMBLER (Ver. 3.1)
 *      PC-9801 MS-DOS Turbo-Pascal version
 */
 //#define DEBUG

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* start of type definition */
typedef enum TOKENBS {
    DLB,R0L,R1L,R2L,R3L,R4L,R5L,R6L,R7L,RB,RBP,PC,IO,
    MM,IR,FSR,NLB,ZERO,

    DRB,R0R,R1R,R2R,R3R,R4R,R5R,R6R,R7R,RA,RAP,SLT,LLT,
    U1,U2,U3,NRB,

    DAL,ADDOP,SUBOP,ANDOP,OROP,XOROP,IALOP,U4,NAL,
    DSH,SLL,SRL,SLA,SRA,SNX,SWP,U7,NSH,
    DSB,R0S,R1S,R2S,R3S,R4S,R5S,R6S,R7S,SA,SAP,SB,SBP,
    PCS,U8,U9,NSB,

    DMM,RM,WM,U10,NMM,

    DSQ,B,BP,RTN,BT,BF,IOP,IRA,IAB,EI,U12,U13,U14,U15,
    U16,U17,NSQ,

    DTS,ZER,NEG,CRY,OV,T,CZ,U18,NTS,
    DEX,CM1,FLS,ASC,AS1,LIR,LIO,SC,EXIO,ST,RT,INA,INB,
    DCB,HLT,SOV,NEX,

    R0,R1,R2,R3,R4,R5,R6,R7,LBUS,RBUS,
    ERR,IDNT,INTGR,KEYW,TEND,TITLE,
    TIF,TGOTO,TCALL,RET,TELSE,TTHEN,FETCH,
    TREAD,TWRITE,

    BY,C,TSET,TEXECUTE,TWITH,IRB,SAVE,FLAG,
    ASTAR,CORN,SEMC,PERIOD,ASSIGNMENT,EQUAL,EOST,EOSF,ONE,
} TOKENBS;

typedef enum TOKENBS    LBBS;
typedef enum TOKENBS    RBBS;
typedef enum TOKENBS    ALBS;
typedef enum TOKENBS    SHBS;
typedef enum TOKENBS    SBBS;
typedef enum TOKENBS    MMBS;
typedef enum TOKENBS    SQBS;
typedef enum TOKENBS    TSBS;
typedef enum TOKENBS    EXBS;
/*
     LBBS=DLB..NLB;
     RBBS=DRB..NRB;
     ALBS=DAL..NAL;
     SHBS=DSH..NSH;
     SBBS=DSB..NSB;
     MMBS=DMM..NMM;
     SQBS=DSQ..NSQ;
     TSBS=DTS..NTS;
     EXBS=DEX..NEX;
*/
typedef enum REFE {
    NOLBL,NREF,OTHER,IRAREF,IOPREF,IABREF,
} REFE;
/* REFBS=NOLBL..IABREF; */
typedef REFE    REFBS;

typedef enum REFDEF {
    DEF,UNDEF,NOREF
} REFDEF;
typedef REFDEF  REFDEFBS;
/*
     REFDEFBS=DEF..NOREF;
*/
/*     CHAR10=packed array[1..10]of char;*/
typedef char    CHAR8[8+1];
typedef char    CHAR6[6+1];
typedef char    CHAR4[4+1];

/* max number of bytes for filename */
#ifndef MAX_PATH
#define MAX_PATH        256
#endif

typedef char    FILENAME[MAX_PATH+1];

typedef enum ADFLGBS {
    DE,SP,
} ADFLGBS;

/* ADFLG=DE..SP; */
typedef ADFLGBS ADFLG;

struct TYDEFTABLE;
struct TYUNDTABLE;

typedef struct TYMIDCODE {
    int ADDR;
    ADFLG ADATTR;
//    TYMDP NEXTMID;
    struct TYMIDCODE    *NEXTMID;
    REFBS REF;
//    TYDTP SDTP;
    struct TYDEFTABLE   *SDTP;
    LBBS LB;
    RBBS RB;
    ALBS AL;
    SHBS SH;
    SBBS SB;
    MMBS MM;
    SQBS SQ;
    TSBS TS;
    EXBS EX;
    int LT;
    REFDEFBS FLG;

    /* union */
//    TYDTP DEFTP;
    struct TYDEFTABLE   *DEFTP;
    struct TYMIDCODE    *MIDP;
    /* 
                 case FLG:REFDEFBS of
                   DEF:(DEFTP:TYDTP);
                   UNDEF:(MIDP:TYMDP);
                   NOREF:();
                end;
    */
} TYMIDCODE;
typedef TYMIDCODE       *TYMDP;

typedef struct TYDEFTABLE {
    CHAR8 IDENT;
    TYMDP MIDP;
    struct TYDEFTABLE   *NEXTDP;
} TYDEFTABLE;
typedef TYDEFTABLE      *TYDTP;

typedef struct TYUNDTABLE {
    CHAR8       IDENT;
//    REFBS       FLG;
    REFE        FLG;
    TYMDP       MIDP;
//    TYUTP       NEXTUT;
    struct TYUNDTABLE   *NEXTUT;
} TYUNDTABLE;
typedef TYUNDTABLE      *TYUTP;

/*     PMODE=set of char;*/

/* end of type definition */

/* global variables */

/* PMODE MODE = { false, };*/
bool    MODE_S;
bool    MODE_O;
bool    MODE_M;

TOKENBS         TOKEN;

/* POINTER TO FILE */
FILE    *SIFP;
FILE    *SOFP;
FILE    *OFP;
FILE    *MFP;

bool    CONTINUE;
bool    EXECUTE;
/*     HEAPTOP:^integer; */


/* start of EX_ASSEMBLE */
/*  procedure EX_ASSEMBLE;*/
const int       MAXADRS = 4095;
CHAR8           IDENT;
int             INTGER;
TYMDP           TPMIDCODE;
TYMDP           BTMIDCODE;
/* POINTER TO INTERMEDIATE LANGUAGE */
TYDTP           TPDTABLE;
TYDTP           BTDTABLE;
/* POINTER TO TABLE OF DEFINITION LABEL */
TYUTP           TPUTABLE;
TYUTP           BTUTABLE;
/* POINTER TO TABLE OF UNDEFINITION LABEL */
int             LAST_ADDR;
bool            ERRO;

/* start of PASS1 */
/*    procedure PASS1;*/
bool            TER;
//char            CH;
int             CH;     /* should be 'int' */
TYUTP           UCP;

/* start of GET_LN */
void GET_LN()
{
//      const LF=#10;
//            EOF=#26;
    do {
        if (MODE_S) {
            fprintf(SOFP, "%c", CH);
        }
        CH = fgetc(SIFP);
        CH = toupper(CH);
    } while (CH != '\n' && CH != EOF);

    if (MODE_S) {
        fprintf(SOFP, "%c", CH);
    }
    if (CH != EOF) {
        CH = fgetc(SIFP);
        CH = toupper(CH);
    }
}
/* end of GET_LN */

/* start of GET_TOKEN */
/*      procedure GET_TOKEN;  {  TRANSACT TOKEN ROUTINE  }*/
    /* TRANSACT TOKEN ROUTINE */
/*
      const TAB=#9;
            CR=#13;
            EOF=#26;
*/

/* start of TOKEN_IS_SEMC */
void TOKEN_IS_SEMC()
{
    TOKEN = SEMC;
    CH = fgetc(SIFP);
    CH = toupper(CH);
    GET_LN();
}
/* end of TOKEN_IS_SEMC */
/* start of TOKEN_IS_ADD */
void TOKEN_IS_ADD()
{
    TOKEN = ADDOP;
    CH = fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_ADD */
/* start of TOKEN_IS_SUB */
void TOKEN_IS_SUB()
{
    TOKEN = SUBOP;
    CH = fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_SUB */
/* start of TOKEN_IS_NAL */
void TOKEN_IS_NAL()
{
    TOKEN = NAL;
    CH =fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_NAL */
/* start of TOKEN_IS_ASTAR */
void TOKEN_IS_ASTAR()
{
    TOKEN = ASTAR;
    CH = fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_ASTRR */
/* start of TOKEN_IS_EQUAL */
void TOKEN_IS_EQUAL()
{
    TOKEN = EQUAL;
    CH = fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_EQUAL */
/* start of TOKEN_IS_IAL */
void TOKEN_IS_IAL()
{
    TOKEN = IALOP;
    CH = fgetc(SIFP);
    CH = toupper(CH);
}
/* end of TOKEN_IS_IAL */
/* start of TOKEN_IS_CORN */
void TOKEN_IS_CORN()
{
    CH = fgetc(SIFP);
    CH = toupper(CH);
    if (CH == '=') {
        CH = fgetc(SIFP);
        CH = toupper(CH);
        TOKEN = ASSIGNMENT;
        if (MODE_S) {
            fprintf(SOFP, "=");
        }
    }
    else {
        TOKEN = CORN;
    }
}
/* end of TOKEN_IS_CORN */
/* start of TOKEN_IS_PERIOD */
void TOKEN_IS_PERIOD()
{
    CHAR6 SPRSV;
    int I;

    CH = toupper(fgetc(SIFP));
    I = 0;

//    strncpy(SPRSV, "      ", 6);
    SPRSV[0] = '\0';

    while (isalpha(CH) && isupper(CH) && I < 6) {
        SPRSV[I] = CH;
        if (MODE_S)
            fprintf(SOFP, "%c", CH);
        I++;
        CH = fgetc(SIFP);
        CH = toupper(CH);
    }
    SPRSV[I] = '\0';

    if (I >= 6) {
        GET_LN();
        TOKEN = ERR;
    }
    else if (!strncmp(SPRSV, "TITLE", 6)) {
        TOKEN = TITLE;
    }
    else if (!strncmp(SPRSV, "END", 6)) {
        TOKEN = TEND;
    }
    else {
        GET_LN();
        TOKEN = ERR;
    }
}
/* end of TOKEN_IS_PERIOD */
/* start of INTGR_HANDLE */
void INTGR_HANDLE(int BASE)
{
    int INT;
/*            AREA:set of char;*/
    bool LMTFLG;

    LMTFLG = true;
/*
            case BASE of
              16:AREA:=['0'..'9','A'..'F'];
              10:AREA:=['0'..'9'];
              2:AREA:=['0'..'1'];
            end;
*/
    switch (BASE) {
    case 16:
        if (isxdigit(CH)) {
            INT = isdigit(CH)? CH - 48: CH - 55;
            INTGER = INT;
            CH = fgetc(SIFP);
            CH = toupper(CH);
            while (isxdigit(CH)) {
                if (MODE_S) {
                    fprintf(SOFP, "%c", CH);
                }
                INT = isdigit(CH)? CH - 48: CH - 55;
                if (INTGER < 0)
                    LMTFLG = false;
                INTGER = INTGER * 16 + INT;
                if (!(INTGER <= 0 || LMTFLG))
                    TER = true;
                CH = fgetc(SIFP);
                CH = toupper(CH);
            }
            TOKEN = INTGR;
        }
        break;
    case 10:
        if (isdigit(CH)) {
            INT = CH - '0';
            INTGER = INT;
            CH = fgetc(SIFP);
            CH = toupper(CH);
            while (isdigit(CH)) {
                if (MODE_S) {
                    fprintf(SOFP, "%c", CH);
                }
                INT = CH - '0';
                if (INTGER < 0)
                    LMTFLG = false;
                INTGER = INTGER * 10 + INT;
                if (!(INTGER <= 0 || LMTFLG))
                    TER = true;
                CH = fgetc(SIFP);
                CH = toupper(CH);
            }
            TOKEN = INTGR;
        }
        break;
    case 2:
        if (CH == '0' || CH == '1') {
            INT = CH - '0';
            INTGER = INT;
            CH = fgetc(SIFP);
            CH = toupper(CH);
            while (CH == '0' || CH == '1') {
                if (MODE_S) {
                    fprintf(SOFP, "%c", CH);
                }
                INT = CH - '0';
                if (INTGER < 0)
                    LMTFLG = false;
                INTGER = INTGER * 2 + INT;
                if (!(INTGER <= 0 || LMTFLG))
                    TER = true;
                CH = fgetc(SIFP);
                CH = toupper(CH);
            }
            TOKEN = INTGR;
        }
        break;
    default:
        assert(0);
        break;
    }
}
/* end of INTGR_HANDLE */
/* start of IDNT_HANDLE */
void IDNT_HANDLE()
{
    int J;
//    strncpy(IDENT, "        ", 8);
    IDENT[0] = CH;
    J = 1;
    CH = fgetc(SIFP);
    CH = toupper(CH);
    while (J < 8 && (isalnum(CH) || CH == '_')) {
        IDENT[J] = CH;
        if (MODE_S)
            fprintf(SOFP, "%c", CH);
        J++;
        CH = fgetc(SIFP);
        CH = toupper(CH);
    }
    IDENT[J] = '\0';

    while (isalnum(CH) || CH == '_') {
        if (MODE_S)
            fprintf(SOFP, "%c", CH);
        CH = fgetc(SIFP);
        CH = toupper(CH);
    }

    if (!strncmp(IDENT, "GOTO", 8))
        TOKEN = TGOTO;
    else if (!strncmp(IDENT, "HLT", 8))
        TOKEN = HLT;
    else if (!strncmp(IDENT, "IF", 8))
        TOKEN = TIF;
    else if (!strncmp(IDENT, "IAB", 8))
        TOKEN = IAB;
    else if (!strncmp(IDENT, "IO", 8))
        TOKEN = IO;
    else if (!strncmp(IDENT, "IOP", 8))
        TOKEN = IOP;
    else if (!strncmp(IDENT, "IR", 8))
        TOKEN = IR;
    else if (!strncmp(IDENT, "IRA", 8))
        TOKEN = IRA;
    else if (!strncmp(IDENT, "IRB", 8))
        TOKEN = IRB;
    else if (!strncmp(IDENT, "LBUS", 8))
        TOKEN = LBUS;
    else if (!strncmp(IDENT, "MM", 8))
        TOKEN = MM;
    else if (!strncmp(IDENT, "NEG", 8))
        TOKEN = NEG;
    else if (!strncmp(IDENT, "NEX", 8))
        TOKEN = NEX;
    else if (!strncmp(IDENT, "NSH", 8))
        TOKEN = NSH;
    else if (!strncmp(IDENT, "NSQ", 8))
        TOKEN = NSQ;
    else if (!strncmp(IDENT, "ONE", 8))
        TOKEN = ONE;
    else if (!strncmp(IDENT, "OR", 8))
        TOKEN = OROP;
    else if (!strncmp(IDENT, "OV", 8))
        TOKEN = OV;
    else if (!strncmp(IDENT, "PC", 8))
        TOKEN = PC;
    else if (!strncmp(IDENT, "R0", 8))
        TOKEN = R0;
    else if (!strncmp(IDENT, "R1", 8))
        TOKEN = R1;
    else if (!strncmp(IDENT, "R2", 8))
        TOKEN = R2;
    else if (!strncmp(IDENT, "R3", 8))
        TOKEN = R3;
    else if (!strncmp(IDENT, "R4", 8))
        TOKEN = R4;
    else if (!strncmp(IDENT, "R5", 8))
        TOKEN = R5;
    else if (!strncmp(IDENT, "R6", 8))
        TOKEN = R6;
    else if (!strncmp(IDENT, "R7", 8))
        TOKEN = R7;
    else if (!strncmp(IDENT, "RA", 8))
        TOKEN = RA;
    else if (!strncmp(IDENT, "RAP", 8))
        TOKEN = RAP;
    else if (!strncmp(IDENT, "RB", 8))
        TOKEN = RB;
    else if (!strncmp(IDENT, "RBP", 8))
        TOKEN = RBP;
    else if (!strncmp(IDENT, "RBUS", 8))
        TOKEN = RBUS;
    else if (!strncmp(IDENT, "READ", 8))
        TOKEN = TREAD;
    else if (!strncmp(IDENT, "RETURN", 8))
        TOKEN = RET;
    else if (!strncmp(IDENT, "SAVE", 8))
        TOKEN = SAVE;
    else if (!strncmp(IDENT, "SET", 8))
        TOKEN = TSET;
    else if (!strncmp(IDENT, "SNX", 8))
        TOKEN = SNX;
    else if (!strncmp(IDENT, "SLA", 8))
        TOKEN = SLA;
    else if (!strncmp(IDENT, "SLL", 8))
        TOKEN = SLL;
    else if (!strncmp(IDENT, "SRA", 8))
        TOKEN = SRA;
    else if (!strncmp(IDENT, "SRL", 8))
        TOKEN = SRL;
    else if (!strncmp(IDENT, "SWP", 8))
        TOKEN = SWP;
    else if (!strncmp(IDENT, "T", 8))
        TOKEN = T;
    else if (!strncmp(IDENT, "THEN", 8))
        TOKEN = TTHEN;
    else if (!strncmp(IDENT, "WITH", 8))
        TOKEN = TWITH;
    else if (!strncmp(IDENT, "WRITE", 8))
        TOKEN = TWRITE;
    else if (!strncmp(IDENT, "XOR", 8))
        TOKEN = XOROP;
    else if (!strncmp(IDENT, "ZER", 8))
        TOKEN = ZER;
    else if (!strncmp(IDENT, "ZERO", 8))
        TOKEN = ZERO;
    else
        TOKEN = IDNT;
}
/* end of IDNT_HANDLE */
/* start of VAGUE_HANDLE */
void VAGUE_HANDLE()
{
    int J,I,INT;
    bool VAGUE;
    char LASTCH;

    LASTCH = CH;
    CH = fgetc(SIFP);
    CH = toupper(CH);
    if (CH == '"') {
        if (LASTCH != 'B' && LASTCH != 'D') {
            GET_LN();
            TOKEN = ERR;
        }
        else {
            if (MODE_S)
                fprintf(SOFP, "%c", CH);
            CH = fgetc(SIFP);
            CH = toupper(CH);
            if (MODE_S)
                fprintf(SOFP, "%c", CH);
            switch (LASTCH) {
            case 'B':
                INTGR_HANDLE(2);
                break;
            case 'D':
                INTGR_HANDLE(10);
                break;
            default:
                break;
            }
        }
    }
    else {
        VAGUE = true;
        IDENT[0] = LASTCH;
        J = 1;
        while (J < 8 && (isalnum(CH) || CH == '_')) {
            IDENT[J] = CH;
            if (!isxdigit(CH))
                VAGUE = false;
            if (MODE_S)
                fprintf(SOFP, "%c", CH);
            J++;
            CH = fgetc(SIFP);
            CH = toupper(CH);
        }
        IDENT[J] = '\0';
        while (isalnum(CH) || CH == '_') {
            if (MODE_S)
                fprintf(SOFP, "%c", CH);
            CH = fgetc(SIFP);
            CH = toupper(CH);
        }
        if (!VAGUE) {
            if (!strncmp(IDENT, "AND", 8))
                TOKEN = ANDOP;
            else if (!strncmp(IDENT, "BY", 8))
                TOKEN = BY;
            else if (!strncmp(IDENT, "CALL", 8))
                TOKEN = TCALL;
            else if (!strncmp(IDENT, "CRY", 8))
                TOKEN = CRY;
            else if (!strncmp(IDENT, "CZ", 8))
                TOKEN = CZ;
            else if (!strncmp(IDENT, "ELSE", 8))
                TOKEN = TELSE;
            else if (!strncmp(IDENT, "EXECUTE", 8))
                TOKEN = TEXECUTE;
            else if (!strncmp(IDENT, "FETCH", 8))
                TOKEN = FETCH;
            else if (!strncmp(IDENT, "FLAG", 8))
                TOKEN = FLS;
            else if (!strncmp(IDENT, "FSR", 8))
                TOKEN = FSR;
            else
                TOKEN = IDNT;
        }
        else {
            if (!strncmp(IDENT, "C", 8) && (TOKEN == SEMC || TOKEN == EOST)) {
                TOKEN = C;
            }
            else if (TOKEN == ASSIGNMENT ||
                     (TOKEN >= SLL && TOKEN <=  NSH) ||
                     (TOKEN >= ADDOP && TOKEN <= NAL)) {
                TOKEN = INTGR;
            }
            else if (TOKEN == TITLE ||
                     TOKEN == IRA || TOKEN == IOP || TOKEN == IAB ||
                     TOKEN == TGOTO || TOKEN == TCALL || TOKEN == TTHEN) {
                TOKEN = IDNT;
            }
            else {
                while (CH == ' ' || CH == '\t')
                    CH = fgetc(SIFP);
                CH = toupper(CH);
                if (CH == ':')
                    TOKEN = IDNT;
                else
                    TOKEN = INTGR;
            }

            if (TOKEN == INTGR) {
                INTGER = 0;
                if (J < 5) {
                    for (I = 0; I < J; I++) {
                        if (isdigit(IDENT[I]))
                            INT = IDENT[I] - 48;
                        else if (isxdigit(IDENT[I]))
                            INT = IDENT[I] - 55;
                        INTGER = INTGER * 16 + INT;
                    }
                }
                else
                    TER = true;
            }
        }
    }
}
/* end of VAGUE_HANDLE */
/* start of GET_TOKEN */
void GET_TOKEN()
{
    /* TRANSACT TOKEN ROUTINE */

    while (CH == '\t' || CH == ' ') {
        if (MODE_S)
            fprintf(SOFP, "%c", CH);
        CH = fgetc(SIFP);
        CH = toupper(CH);
    }
    if (CH != EOF) {
        if (CH != '\n' && CH != '\r') {
            if (CH == ';' || CH ==  '.' || isalnum(CH) ||
                CH == ':' || CH == '+' || CH == '-' ||
                CH == '@' || CH == '*' || CH == '=' || CH == '$') {

                if (MODE_S)
                    fprintf(SOFP, "%c", CH);

                switch (CH) {
                case ';': TOKEN_IS_SEMC(); break;
                case '.': TOKEN_IS_PERIOD(); break;
                case ':': TOKEN_IS_CORN(); break;
                case '+': TOKEN_IS_ADD(); break;
                case '-': TOKEN_IS_SUB(); break;
                case '@': TOKEN_IS_NAL(); break;
                case '*': TOKEN_IS_ASTAR(); break;
                case '=': TOKEN_IS_EQUAL(); break;
                case '$': TOKEN_IS_IAL(); break;

                default:
                    if (CH >= 'A' && CH <= 'F')
                        VAGUE_HANDLE();
                    else if (CH >= 'G' && CH <= 'Z')
                        IDNT_HANDLE();
                    else if (isdigit(CH))
                        INTGR_HANDLE(16);
                    else
                        assert(0);
                }
            }
            else {
                GET_LN();
                if (MODE_S)
                    fprintf(SOFP, "ERROR: ILLEGAL CHARACTER '%c'\n", CH);
                TOKEN = ERR;
            }
        }
        else {
            TOKEN = EOST;
#if !defined(_MSC_VER)
            /* for non-MS (DOS) environment, read one more byte at End-Of-Line */
            CH = fgetc(SIFP);
            CH = toupper(CH);
#endif
            if (MODE_S)
                fprintf(SOFP, "\n");
            CH = fgetc(SIFP);
            CH = toupper(CH);
        }
    }
    else {
        TOKEN = EOSF;
        if (MODE_S)
            fprintf(SOFP, "\n");
    }
    if (TOKEN == ERR)
        GET_TOKEN();
}
/* end of GET_TOKEN */
/* start of ERROR */
void ERROR(int I)
{
    /* ERROR HANDLING ROUTINE */

    if (I != 20)
        GET_LN();

    if (MODE_S) {
        fprintf(SOFP, "\n");
        switch (I) {
        case 1:
            fprintf(SOFP, "ERROR%5d"
                    ":THIS LABEL ALREADY DEFINED.\n", I);
            break;
        case 2:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL FORMAT OF PROGRAM TITLE.\n", I);
            break;
        case 3:
            fprintf(SOFP, "ERROR%5d"
                    ":END OF MICRO_INSTRUCTION MUST BE \";\" OR END OF LINE.\n", I);
            break;
        case 4:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL FORMAT OF TEST CONDITION.\n", I);
            break;
        case 5:
            fprintf(SOFP, "ERROR%5d"
                    ":NO THEN CLAUSE FOR IF STATEMENT.\n", I);
            break;
        case 6:
            fprintf(SOFP, "ERROR%5d"
                    ":LABEL IS EXPECTED IN THIS CLAUSE.\n", I);
            break;
        case 7:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL IF STATEMENT.\n", I);
            break;
        case 8:
            fprintf(SOFP, "ERROR%5d"
                    ":ELSE CLAUSE MUST BE \"FETCH\".\n", I);
            break;
        case 9:
            fprintf(SOFP, "ERROR%5d"
                    ":LT FEILD ALREADY DEFINED.\n", I);
            break;
        case 10:
            fprintf(SOFP, "ERROR%5d"
                    ":LITERAL IS OUT OF RANGE.\n", I);
            break;
        case 11:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL EXPRESSION OR ASSIGNMENT.\n", I);
            break;
        case 12:
            fprintf(SOFP, "ERROR%5d"
                    ":RB FEILD ALREADY DEFINED.\n", I);
            break;
        case 13:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL EXCEPTION STATEMENT.\n", I);
            break;
        case 14:
            fprintf(SOFP, "ERROR%5d"
                    ":LB FEILD ALREADY DEFINED.\n", I);
            break;
        case 15:
            fprintf(SOFP, "ERROR%5d"
                    ":EX FEILD ALREADY DEFINED.\n", I);
            break;
        case 16:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL FORMAT OF INSTRUCTION HEADING OR UNKNOWN STATEMENT.\n", I);
            break;
        case 17:
            fprintf(SOFP, "ERROR%5d"
                    ":SPECIFIED ADDRESS OVERLAPPED.\n", I);
            break;
        case 18:
            fprintf(SOFP, "ERROR%5d"
                    ":ADDRESS IS OUT OF LIMIT.\n", I);
            break;
        case 19:
            fprintf(SOFP, "ERROR%5d"
                    ":.END EXPECTED.\n", I);
            break;
        case 20:
            fprintf(SOFP, "ERROR%5d"
                    ":LABELS(',UCP^.IDENT,') IS NOT DECLARED.\n", I);
            break;
        case 21:
            fprintf(SOFP, "ERROR%5d"
                    ":ILLEGAL TYPE OF PROGRAM ENDING.\n", I);
            break;
        default:
            assert(0);
            break;
        }
    }
    while (TOKEN != ASTAR && TOKEN != TEND && TOKEN != EOSF)
        GET_TOKEN();
    ERRO = true;
    TER = true;
}
/* end of ERROR */
/* start of TABLE_ENTRY */
void TABLE_ENTRY()
{
    TYDTP DTP;
    TYUTP UTP;
    TYUTP LASTUTP;
    TYMDP MCP;
    TYMDP NEXTMCP;
    bool ER, DIS;
    REFBS REFFLG;

    ER = true;
    DTP = BTDTABLE;
    while (DTP->NEXTDP != NULL && ER) {
        if (!strncmp(DTP->IDENT, IDENT, 8)) {
            ERROR(1);
            ER = false;
        }
        DTP = DTP->NEXTDP;
    }
    if (ER) {
        DIS = false;
        UTP = BTUTABLE;
        LASTUTP = UTP;
        while (UTP->NEXTUT != NULL && !DIS) {
            if (!strncmp(UTP->IDENT, IDENT, 8)) {
                MCP = UTP->MIDP;
                REFFLG = UTP->FLG;
                while (MCP != NULL) {
                    NEXTMCP = MCP->MIDP;
                    MCP->FLG = DEF;
                    MCP->DEFTP = DTP;
                    MCP = NEXTMCP;
                }
                UTP = UTP->NEXTUT;
                if (UTP == BTUTABLE->NEXTUT)
                    BTUTABLE = UTP;
                else
                    LASTUTP->NEXTUT = UTP;
                DIS = true;
            }
            else {
                LASTUTP = UTP;
                UTP = UTP->NEXTUT;
            }
        }
        strncpy(DTP->IDENT, IDENT, 8);

        DTP->MIDP = TPMIDCODE;
        DTP->MIDP->SDTP = DTP;
        if (DIS)
            DTP->MIDP->REF = REFFLG;
        else
            DTP->MIDP->REF = NREF;

        DTP->NEXTDP = (TYDTP)malloc(sizeof(TYDEFTABLE));
        assert(DTP->NEXTDP != NULL);
        /*new(NEXTDT);*/
        TPDTABLE = DTP->NEXTDP;
        DTP->NEXTDP->NEXTDP = NULL;
        DTP->NEXTDP->MIDP = NULL;
    }
}
/* end of TABLE_ENTRY */
/* start of HEADING}H */
void HEADING()
{
    /* TRANSACT HEADING-PART ROUTINE */

    CH = fgetc(SIFP);
    do {
        GET_TOKEN();
    } while (TOKEN == SEMC || TOKEN == EOST);

    if (TOKEN != TITLE)
        ERROR(2);
    else {
        GET_TOKEN();
        if (TOKEN != IDNT)
            ERROR(2);
        else {
            GET_TOKEN();
            if (TOKEN != SEMC && TOKEN != EOST)
                ERROR(3);
        }
    }
    if (!ERRO) {
        if (MODE_O)
            fprintf(OFP, "CM   %-8s\n", IDENT);
        if (MODE_M) {
            fprintf(MFP, ".TITLE     %-8s\n", IDENT);
            fprintf(MFP,
                    "LABEL   ADRS  LB  RB  AL  SH  SB  MM  SQ  TS  EX  LT\n");
        }
        GET_TOKEN();
    }
}
/* end of HEADING */
/* start of BLOCK */
/* procedure BLOCK;  {  TRANSACT BLOCK-PART ROUTINE  } */
bool DEFIN;
/* start of SEARCH */
void SEARCH(REFBS REFSW)
{
    TYDTP DTP;
    TYUTP UTP;
    TYMDP MCP;
    TYMDP LASTMCP;
    bool DIS;

    DTP = BTDTABLE;
    DIS = false;
    while (DTP->NEXTDP != NULL && !DIS) {
        if (!strncmp(DTP->IDENT, IDENT, 8)) {
            TPMIDCODE->FLG = DEF;
            TPMIDCODE->DEFTP = DTP;
            if (DTP->MIDP->REF < REFSW)
                DTP->MIDP->REF = REFSW;
            DIS = true;
        }
        else {
            DTP = DTP->NEXTDP;
        }
    }
    if (DIS)
        DEFIN = true;
    else {
        TPMIDCODE->FLG = UNDEF;
        TPMIDCODE->MIDP = NULL;
        DEFIN = false;
    }
    UTP = BTUTABLE;
    LASTMCP = NULL;
    while (UTP->NEXTUT != NULL && !DIS) {
        if (!strncmp(UTP->IDENT, IDENT, 8)) {
            MCP = UTP->MIDP;
            while (MCP != NULL) {
                LASTMCP = MCP;
                MCP = MCP->MIDP;
            }
            LASTMCP->MIDP = TPMIDCODE;
            DIS = true;
            if (UTP->FLG < REFSW)
                UTP->FLG = REFSW;
        }
        else {
            UTP = UTP->NEXTUT;
        }
    }
    if (!DIS) {
        strncpy(TPUTABLE->IDENT, IDENT, 8);
        TPUTABLE->FLG = REFSW;
        TPUTABLE->MIDP = TPMIDCODE;
        TPUTABLE->NEXTUT = (TYUTP)malloc(sizeof(TYUNDTABLE));
        assert(TPUTABLE->NEXTUT != NULL);
        /*new(TPUTABLE^.NEXTUT);*/
        TPUTABLE = TPUTABLE->NEXTUT;
        TPUTABLE->NEXTUT = NULL;
        TPUTABLE->MIDP = NULL;
    }
}
/* end of SEARCH */
/* start of MICRO_INSTRUCTION */
/*        procedure MICRO_INSTRUCTION;
          {  TRANSACT MICRO_INSTRUCTION ROUTINE  }*/
/* start of LBUSORD */
void LBUSORD()
{
    switch (TOKEN) {
    case R0: TOKEN = R0L; break;
    case R1: TOKEN = R1L; break;
    case R2: TOKEN = R2L; break;
    case R3: TOKEN = R3L; break;
    case R4: TOKEN = R4L; break;
    case R5: TOKEN = R5L; break;
    case R6: TOKEN = R6L; break;
    case R7: TOKEN = R7L; break;
    case ZERO: TOKEN = NLB; break;
    default:
        assert(0);
        break;
    }
}
/* end of LBUSORD */
/* start of RBUSORD */
void RBUSORD()
{
    switch (TOKEN) {
    case R0: TOKEN = R0R; break;
    case R1: TOKEN = R1R; break;
    case R2: TOKEN = R2R; break;
    case R3: TOKEN = R3R; break;
    case R4: TOKEN = R4R; break;
    case R5: TOKEN = R5R; break;
    case R6: TOKEN = R6R; break;
    case R7: TOKEN = R7R; break;
    default:
        assert(0);
        break;
    }
}
/* end of RBUSORD */
/* start of SBUSORD */
void SBUSORD()
{
    switch (TOKEN) {
    case R0: TOKEN = R0S; break;
    case R1: TOKEN = R1S; break;
    case R2: TOKEN = R2S; break;
    case R3: TOKEN = R3S; break;
    case R4: TOKEN = R4S; break;
    case R5: TOKEN = R5S; break;
    case R6: TOKEN = R6S; break;
    case R7: TOKEN = R7S; break;
    case RA: TOKEN = SA; break;
    case RAP: TOKEN = SAP; break;
    case RB: TOKEN = SB; break;
    case RBP: TOKEN = SBP; break;
    case PC: TOKEN = PCS; break;
    default:
        assert(0);
        break;
    }
}
/* end of SBUSORD */
/* start of TEST_STATEMENT */
/*          procedure TEST_STATEMENT;*/
/* start of IF_STATEMENT */
void IF_STATEMENT()
{
    GET_TOKEN();
    if (! (TOKEN >= ZER && TOKEN <= CZ)) {
        ERROR(4);
    }
    else {
        TPMIDCODE->TS = TOKEN;
        GET_TOKEN();
        if (TOKEN != EQUAL) {
            ERROR(4);
        }
        else {
            GET_TOKEN();
            if (TOKEN != INTGR ||
                (INTGER != 0 && INTGER != 1)) {
                ERROR(4);
            }
            else {
                if (INTGER == 0) {
                    TPMIDCODE->SQ = BF;
                }
                else {
                    TPMIDCODE->SQ = BT;
                }
                GET_TOKEN();
                if (TOKEN != TTHEN) {
                    ERROR(5);
                }
                else {
                    GET_TOKEN();
                    if (TOKEN != IDNT) {
                        ERROR(6);
                    }
                    else {
                        SEARCH(OTHER);
                        GET_TOKEN();
                        if (TOKEN == TELSE) {
                            if (TPMIDCODE->SQ != BF) {
                                ERROR(7);
                            }
                            else {
                                TPMIDCODE->SQ = EI;
                                GET_TOKEN();
                                if (TOKEN != FETCH) {
                                    ERROR(8);
                                }
                                else {
                                    GET_TOKEN();
                                }
                            }
                        }
                        if (TOKEN != SEMC && TOKEN != EOST) {
                            ERROR(3);
                        }
                    }
                }
            }
        }
    }
}
/* end of IF_STATEMENT */
/* start of GOTO_STATEMENT */
void GOTO_STATEMENT()
{
    GET_TOKEN();
    if (TOKEN != IDNT && TOKEN != FETCH) {
        ERROR(6);
    }
    else {
        if (TOKEN == IDNT) {
            SEARCH(OTHER);
            TPMIDCODE->SQ = B;
        }
        else {
            TPMIDCODE->SQ = EI;
            TPMIDCODE->TS = NTS;
        }
        GET_TOKEN();
        if (TOKEN != SEMC && TOKEN != EOST) {
            ERROR(3);
        }
    }
}
/* end of GOTO_STATEMENT */
/* start of CALL_STATEMENT */
void CALL_STATEMENT()
{
    GET_TOKEN();
    if (TOKEN != IDNT) {
        ERROR(6);
    }
    else {
        SEARCH(OTHER);
        TPMIDCODE->SQ = BP;
        GET_TOKEN();
        if (TOKEN != SEMC && TOKEN != EOST) {
            ERROR(3);
        }
    }
}
/* end of CALL_STATEMENT */
/* start of IOP_STATEMENT */
void IOP_STATEMENT()
{
    GET_TOKEN();
    if (TOKEN != IDNT) {
        ERROR(6);
    }
    else {
        SEARCH(IOPREF);
        TPMIDCODE->SQ = IOP;
        GET_TOKEN();
        if (TOKEN != SEMC && TOKEN != EOST) {
            ERROR(3);
        }
    }
}
/* end of IOP_STATEMENT */
/* start of IRA_STATEMENT */
void IRA_STATEMENT()
{
    GET_TOKEN();
    if (TOKEN != IDNT) {
        ERROR(6);
    }
    else {
        SEARCH(IRAREF);
        TPMIDCODE->SQ = IRA;
        GET_TOKEN();
        if (TOKEN != SEMC && TOKEN != EOST) {
            ERROR(3);
        }
    }
}
/* end of IRA_STATEMENT */
/* start of IAB_STATEMENT */
void IAB_STATEMENT()
{
    GET_TOKEN();
    if (TOKEN != IDNT) {
        ERROR(6);
    }
    else {
        SEARCH(IABREF);
        TPMIDCODE->SQ = IAB;
        GET_TOKEN();
        if (TOKEN != SEMC && TOKEN != EOST) {
            ERROR(3);
        }
    }
}
/* end of IAB_STATEMENT */
/* start of RETURN_STATEMENT */
void RETURN_STATEMENT()
{
    TPMIDCODE->SQ = RTN;
    GET_TOKEN();
    if (TOKEN != SEMC && TOKEN != EOST) {
        ERROR(3);
    }
}
/* end of RETURN_STATEMENT */
/* start of NSQ_STATEMENT */
void NSQ_STATEMENT()
{
    TPMIDCODE->SQ = NSQ;
    GET_TOKEN();
    if (TOKEN != SEMC && TOKEN != EOST) {
        ERROR(3);
    }
}
/* end of NSQ_STATEMENT */
/* start of TEST_STATEMENT */
void TEST_STATEMENT()
{
    switch (TOKEN) {
    case TIF: IF_STATEMENT(); break;
    case TGOTO: GOTO_STATEMENT(); break;
    case TCALL: CALL_STATEMENT(); break;
    case IOP: IOP_STATEMENT(); break;
    case IRA: IRA_STATEMENT(); break;
    case IAB: IAB_STATEMENT(); break;
    case RET: RETURN_STATEMENT(); break;
    case NSQ: NSQ_STATEMENT(); break;
    default:
        assert(0);
        break;
    }
    while (TOKEN == SEMC || TOKEN == EOST)
        GET_TOKEN();
}
/* end of TEST_STATEMENT */
/* start of MEM_STATEMENT */
void MEM_STATEMENT()
{
    if (TOKEN == TREAD) {
        TPMIDCODE->MM = RM;
    }
    else {
        TPMIDCODE->MM = WM;
    }
    GET_TOKEN();
    if (TOKEN != SEMC && TOKEN != EOST) {
        ERROR(3);
    }
    while (TOKEN == SEMC || TOKEN == EOST)
        GET_TOKEN();
}
/* end of MEM_STATEMENT */
/* start of OP_STATEMENT */
/*          procedure OP_STATEMENT;*/
/* start of EXPRESSION */
void EXPRESSION()
{
    const int REJECT = 10;
    int STATUS;

    STATUS = 0;
    GET_TOKEN();
    if (TOKEN >= SLL && TOKEN <= NSH) {
        TPMIDCODE->SH = TOKEN;
        GET_TOKEN();
        STATUS = 4;
    }
    if ((TOKEN >= R0 && TOKEN <= R7) ||
        (TOKEN >= RB && TOKEN <= FSR) || TOKEN == ZERO) {

        if ((TOKEN >= R0 && TOKEN <= R7) || TOKEN == ZERO) {
            LBUSORD();
        }
        TPMIDCODE->LB = TOKEN;
        switch (STATUS) {
        case 0: STATUS = 1; break;
        case 4: STATUS = 5; break;
        default: STATUS = REJECT; break;
        }
        GET_TOKEN();
    }
    if ((TOKEN >= ADDOP && TOKEN <= NAL) && STATUS == 1) {
        TPMIDCODE->AL = TOKEN;
        STATUS = 2;
        GET_TOKEN();
    }
    if (TOKEN == CORN && STATUS == 2) {
        GET_TOKEN();
        if (TOKEN >= SLL && TOKEN <= NSH) {
            TPMIDCODE->SH = TOKEN;
            STATUS = 3;
            GET_TOKEN();
        }
        else {
            STATUS = REJECT;
        }
    }
    if ((TOKEN >= R0 && TOKEN <= R7) ||
        TOKEN == RA || TOKEN == RAP || TOKEN == INTGR || TOKEN == C) {
        if (STATUS == 0 || STATUS == 2 || STATUS == 3 || STATUS == 4) {
            STATUS = 5;
            if (TOKEN == INTGR)
                if (INTGER != 0 && (
                        TPMIDCODE->SQ == B ||
                        TPMIDCODE->SQ == BP ||
                        TPMIDCODE->SQ == BT ||
                        TPMIDCODE->SQ == BF ||
                        TPMIDCODE->SQ == EI ||
                        TPMIDCODE->SQ == IOP ||
                        TPMIDCODE->SQ == IRA ||
                        TPMIDCODE->SQ == IAB))
                    ERROR(9);
                else {
                    TPMIDCODE->LT = INTGER;
                    if (!TER)
                        if (INTGER == 0)
                            TPMIDCODE->RB = NRB;
                        else if (INTGER >= 1 && INTGER <= 511)
                            TPMIDCODE->RB = SLT;
                        else
                            TPMIDCODE->RB = LLT;
                    else {
                        ERROR(10);
                        TER = false;
                    }
                }
            else {
                if (TOKEN >= R0 && TOKEN <= R7)
                    RBUSORD();
                TPMIDCODE->RB = TOKEN;
            }
            if (!TER)
                GET_TOKEN();
        }
        else
            STATUS = REJECT;
    }
    if (STATUS != 1 && STATUS != 5)
        ERROR(11);
    else if (TPMIDCODE->SB == NSB &&
             TPMIDCODE->AL == DAL)
        TPMIDCODE->AL = OROP;
}
/* end of EXPRESSION */
/* start of OP_STATEMENT */
void OP_STATEMENT()
{
    if ((TOKEN >= R0 && TOKEN <= R7) ||
        TOKEN == RA || TOKEN == RAP ||
        TOKEN == RB || TOKEN == RBP || TOKEN == PC) {

        SBUSORD();
        TPMIDCODE->SB = TOKEN;
        GET_TOKEN();
        if (TOKEN != ASSIGNMENT)
            ERROR(11);
    }
    else {
        TPMIDCODE->SB = NSB;
    }
    EXPRESSION();
    if (!(TER || (TOKEN == SEMC || TOKEN == EOST)))
        ERROR(3);
    while (TOKEN == SEMC || TOKEN == EOST)
        GET_TOKEN();
}
/* end of OP_STATEMENT */
/* start of EX_STATEMENT */
/*          procedure EX_STATEMENT;*/
/* start of EX_IS_C */
void EX_IS_C()
{
    GET_TOKEN();
    if (TOKEN == SUBOP) {
        GET_TOKEN();
        if (TOKEN == INTGR && INTGER == 1)
            TPMIDCODE->EX = CM1;
        else
            ERROR(13);
    }
    else if (TOKEN == ASSIGNMENT) {
        GET_TOKEN();
        if (TOKEN == RBUS)
            TPMIDCODE->EX = SC;
        else if ((TOKEN >= R0 && TOKEN <= R7) ||
                 TOKEN == RA || TOKEN == RAP || TOKEN == INTGR || TOKEN == C)
            if (TPMIDCODE->RB != DRB)
                ERROR(12);
            else {
                if (TOKEN == INTGR)
                    if (TPMIDCODE->SQ == B ||
                        TPMIDCODE->SQ == BP ||
                        (TPMIDCODE->SQ >= BT && TPMIDCODE->SQ <= EI))
                        ERROR(9);
                    else {
                        TPMIDCODE->LT = INTGER;
                        if (!TER)
                            if (INTGER == 0)
                                TPMIDCODE->RB = NRB;
                            else if (INTGER >= 1 && INTGER <= 511)
                                TPMIDCODE->RB = SLT;
                            else
                                ERROR(15);
                        else {
                            ERROR(10);
                            TER = false;
                        }
                    }
                else {
                    if (TOKEN >= R0 && TOKEN <= R7)
                        RBUSORD();
                    TPMIDCODE->RB = TOKEN;
                }
                TPMIDCODE ->EX = SC;
            }
        else
            ERROR(13);
    }
}
/* end of EX_IS_C */
/* start of EX_IS_FLS */
void EX_IS_FLS()
{
    GET_TOKEN();
    if (TOKEN == SAVE)
        TPMIDCODE->EX = FLS;
    else
        ERROR(13);
}
/* end of EX_IS_FLS */
/* start of EX_IS_WITH */
void EX_IS_WITH()
{
    GET_TOKEN();
    switch (TOKEN) {
    case CRY: TPMIDCODE->EX = ASC; break;
    case ONE: TPMIDCODE->EX = AS1; break;
    default:
        ERROR(13);
        break;
    }
}
/* end of EX_IS_WITH */
/* start of EX_IS_T */
void EX_IS_T()
{
    GET_TOKEN();
    if (TOKEN == ASSIGNMENT) {
        GET_TOKEN();
        if (TOKEN == INTGR)
            switch (INTGER) {
            case 1: TPMIDCODE->EX = ST; break;
            case 0: TPMIDCODE->EX = RT; break;
            default:
                ERROR(13);
                break;
            }
        else
            ERROR(11);
    }
    else
        ERROR(11);
}
/* end of EX_IS_T */
/* start of EX_IS_IR */
void EX_IS_IR()
{
    GET_TOKEN();
    if (TOKEN == ASSIGNMENT) {
        GET_TOKEN();
        if (TOKEN == LBUS)
            TPMIDCODE->EX = LIR;
        else if ((TOKEN >= R0 && TOKEN <= R7) ||
                 (TOKEN >= RB && TOKEN <= MM) ||
                 TOKEN == FSR || TOKEN == ZERO)
            if (TPMIDCODE->LB != DLB)
                ERROR(14);
            else {
                if ((TOKEN >= R0 && TOKEN <= R7) ||
                    TOKEN == ZERO)
                    LBUSORD();
                TPMIDCODE->LB = TOKEN;
                TPMIDCODE->EX = LIR;
            }
        else
            ERROR(11);
    }
    else
        ERROR(11);
}
/* end of EX_IS_IR */
/* start of EX_IS_IO */
void EX_IS_IO()
{
    GET_TOKEN();
    if (TOKEN == ASSIGNMENT) {
        GET_TOKEN();
        if (TOKEN == LBUS)
            TPMIDCODE->EX = LIO;
        else if ((TOKEN >= R0 && TOKEN <= R7) ||
                 (TOKEN >= RB && TOKEN <= PC) ||
                 (TOKEN >= MM && TOKEN <= FSR) ||
                 TOKEN == ZERO)
            if (TPMIDCODE->LB != DLB)
                ERROR(14);
            else {
                if ((TOKEN >= R0 && TOKEN <= R7) || TOKEN == ZERO)
                    LBUSORD();
                TPMIDCODE->LB = TOKEN;
                TPMIDCODE->EX = LIO;
            }
        else
            ERROR(11);
    }
    else
        ERROR(11);
}
/* end of EX_IS_IO */
/* start of EX_IS_EXECUTE */
void EX_IS_EXECUTE()
{
    GET_TOKEN();
    if (TOKEN == IO)
        TPMIDCODE->EX = EXIO;
    else
        ERROR(13);
}
/* end of EX_IS_EXECUTE */
/* start of EX_IS_IRA */
void EX_IS_IRA()
{
    GET_TOKEN();
    if (TOKEN == ADDOP) {
        GET_TOKEN();
        if (TOKEN == INTGR && INTGER == 1)
            TPMIDCODE->EX = INA;
        else
            ERROR(13);
    }
    else
        ERROR(13);
}
/* end of EX_IS_IRA */
/* start of EX_IS_IRB */
void EX_IS_IRB()
{
    GET_TOKEN();
    if (TOKEN == ADDOP) {
        GET_TOKEN();
        if (TOKEN == INTGR && INTGER ==1)
            TPMIDCODE->EX = INB;
        else
            ERROR(13);
    }
    else if (TOKEN == SUBOP) {
        GET_TOKEN();
        if (TOKEN == INTGR && INTGER == 1)
            TPMIDCODE->EX = DCB;
        else
            ERROR(13);
    }
    else
        ERROR(13);
}
/* end of EX_IS_IRB */
/* start of EX_IS_SET */
void EX_IS_SET()
{
    if (TOKEN == TSET)
        GET_TOKEN();
    switch (TOKEN) {
    case HLT: TPMIDCODE->EX = HLT; break;
    case OV: TPMIDCODE->EX = SOV; break;
    default:
        ERROR(13);
        break;
    }
}
/* end of EX_IS_SET */
/* start of EX_IS_NEX */
void EX_IS_NEX()
{
    TPMIDCODE->EX = NEX;
}
/* end of EX_IS_NEX */
/* start of EX_STATEMENT */
void EX_STATEMENT()
{
    /* EXCEPTION OPERATION */
    if (TPMIDCODE->RB == LLT)
        ERROR(15);
    else if (!((TOKEN >= C && TOKEN <= IRB) ||
               TOKEN == IRA ||
               TOKEN == IO ||
               TOKEN == IR ||
               TOKEN == FLS ||
               TOKEN == NEX ||
               TOKEN == T ||
               TOKEN == OV ||
               TOKEN == HLT))
        ERROR(13);
    else
        switch (TOKEN) {
        case C: EX_IS_C(); break;
        case FLS: EX_IS_FLS(); break;
        case TWITH: EX_IS_WITH(); break;
        case T: EX_IS_T(); break;
        case IR: EX_IS_IR(); break;
        case IO: EX_IS_IO(); break;
        case TEXECUTE: EX_IS_EXECUTE(); break;
        case IRA: EX_IS_IRA(); break;
        case IRB: EX_IS_IRB(); break;
        case TSET:
        case OV:
        case HLT:
            EX_IS_SET();
            break;
        case NEX: EX_IS_NEX(); break;
        default:
            assert(0);
            break;
        }
    if (!TER)
        GET_TOKEN();
    if (TOKEN != SEMC && TOKEN != EOST)
        ERROR(3);
    while (TOKEN == SEMC || TOKEN == EOST)
        GET_TOKEN();
}
/* end of EX_STATEMENT*/
/* start of MICRO_INSTRUCTION */
void MICRO_INSTRUCTION()
{
    GET_TOKEN();
    if (TOKEN == IDNT) {
        TABLE_ENTRY();
        GET_TOKEN();
        if (TOKEN != CORN)
            ERROR(16);
        GET_TOKEN();
    }
    if (TOKEN == INTGR) {
        TPMIDCODE->ADDR = INTGER;
        TPMIDCODE->ADATTR = SP;
        GET_TOKEN();
    }
    else
        TPMIDCODE->ADDR = LAST_ADDR + 1;
    if (TPMIDCODE->ADDR <= LAST_ADDR)
        ERROR(17);
    LAST_ADDR = TPMIDCODE->ADDR;
    if (LAST_ADDR > MAXADRS)
        ERROR(18);
    else if (TOKEN !=SEMC && TOKEN != EOST)
        ERROR(3);
    else {
        while (TOKEN == SEMC || TOKEN == EOST)
            GET_TOKEN();

        if (!TER && ((TOKEN >= TIF && TOKEN <= RET) ||
                     TOKEN == IOP || TOKEN == IRA || TOKEN == IAB ||
                     TOKEN == NSQ))
            TEST_STATEMENT();
        if (!TER && (TOKEN == TREAD || TOKEN == TWRITE))
            MEM_STATEMENT();
        if (!TER && ((TOKEN >= R0 && TOKEN <= R7) ||
                     TOKEN == RA || TOKEN == RAP ||
                     TOKEN == RB || TOKEN == RBP || TOKEN == PC)) {
            OP_STATEMENT();
            if (!TER &&
                ((TOKEN >= C && TOKEN <= IRB) ||
                 TOKEN == IRA || TOKEN == IO || TOKEN == IR ||
                 TOKEN == FLS || TOKEN == NEX || TOKEN == T))
                EX_STATEMENT();
        }
        else if (!TER && TOKEN == TSET) {
            GET_TOKEN();
            if (TOKEN == BY) {
                OP_STATEMENT();
                if (!TER &&
                    ((TOKEN >= C && TOKEN <= IRB) ||
                     TOKEN == IRA || TOKEN == IO || TOKEN == IR ||
                     TOKEN == FLS || TOKEN == NEX || TOKEN == T))
                    EX_STATEMENT();
            }
            else
                EX_STATEMENT();
        }
        if (!TER &&
            ((TOKEN >= C && TOKEN <= IRB) ||
             TOKEN == IRA || TOKEN == IO || TOKEN == IR ||
             TOKEN == FLS || TOKEN == NEX || TOKEN == T))
            EX_STATEMENT();
    }
}
/* end of MICRO_INSTRUCTION */
/* start of BLOCK */
void BLOCK()
{
    while (TOKEN == SEMC || TOKEN == EOST)
        GET_TOKEN();
    if (TOKEN != ASTAR) {
        ERROR(16);
    }
    else {
        while (TOKEN == ASTAR) {
            TER = false;
            MICRO_INSTRUCTION();
            TPMIDCODE->NEXTMID = (TYMDP)malloc(sizeof(TYMIDCODE));
            assert(TPMIDCODE->NEXTMID != NULL);
            /*new(TPMIDCODE^.NEXTMID);*/
            {
                TYMDP p = TPMIDCODE->NEXTMID;
                p->ADATTR = DE;
                p->REF = NOLBL;
                p->SDTP = NULL;
                p->NEXTMID = NULL;
                p->LB = DLB;
                p->RB = DRB;
                p->AL = DAL;
                p->SH = DSH;
                p->SB = DSB;
                p->MM = DMM;
                p->SQ = DSQ;
                p->TS = DTS;
                p->EX = DEX;
                p->LT = 0;
                p->FLG = NOREF;
            }
            TPMIDCODE = TPMIDCODE->NEXTMID;
        }
        if (TER) {
            BLOCK();
        }
        else if (TOKEN != TEND) {
            if (TOKEN != EOSF) {
                ERROR(16);
                BLOCK();
            }
            else
                ERROR(19);
        }
        else {
            GET_TOKEN();
            if (TOKEN != EOSF) {
                while (TOKEN == SEMC || TOKEN == EOST)
                    GET_TOKEN();
                if (TOKEN != EOSF) {
                    ERROR(21);
                    while (TOKEN != EOSF)
                        GET_TOKEN();
                }
            }
        }
    }
}    
/* end of BLOCK */
/* start of PASS1 */
void PASS1()
{
    HEADING();
    LAST_ADDR = -1;
    BTDTABLE = (TYDTP)malloc(sizeof(TYDEFTABLE));
    assert(BTDTABLE != NULL);
    /*new(BTDTABLE);*/
    TPDTABLE = BTDTABLE;
    BTDTABLE->NEXTDP = NULL;
    BTDTABLE->MIDP = NULL;
    BTUTABLE = (TYUTP)malloc(sizeof(TYUNDTABLE));
    assert(BTUTABLE != NULL);
    /* new(BTUTABLE);*/
    TPUTABLE = BTUTABLE;
    BTUTABLE->NEXTUT = NULL;
    BTUTABLE->MIDP = NULL;
    BTMIDCODE = (TYMDP)malloc(sizeof(TYMIDCODE));
    assert(BTMIDCODE != NULL);
    /*new(BTMIDCODE);*/
    TPMIDCODE = BTMIDCODE;
    {
        TYMDP p = TPMIDCODE;
        p->ADATTR = DE;
        p->REF = NOLBL;
        p->SDTP = NULL;
        p->NEXTMID = NULL;
        p->LB = DLB;
        p->RB = DRB;
        p->AL = DAL;
        p->SH = DSH;
        p->SB = DSB;
        p->MM = DMM;
        p->SQ = DSQ;
        p->TS = DTS;
        p->EX = DEX;
        p->LT = 0;
        p->FLG = NOREF;
    }
    BLOCK();
    UCP = BTUTABLE;
    while (UCP->NEXTUT != NULL) {
        ERROR(20);
        UCP = UCP->NEXTUT;
    }
}
/* end of PASS1 */

/*{$I masm2.pas}*/

/* start of PASS2 */
/*    procedure PASS2;*/
int     AD;
bool    ER;
TYMDP   MCP;

/* start of NOOP_ADDRES */
void NOOP_ADDRES()
{
    if (MCP->ADATTR == SP)
        if (AD <= MCP->ADDR)
            AD = MCP->ADDR+1;
        else
            ER = true;
    else {
        MCP->ADDR = AD;
        AD = AD+1;
    }
}
/* end of NOOP_ADDRESS */
/* start of IRA_ADDRES */
void IRA_ADDRES()
{
    if (MCP->ADATTR == SP) {
        if (AD <= MCP->ADDR) {
            if (((MCP->ADDR / 4) % 4) != 0)
                ER = true ;
        }
        else
            ER = true;
    }
    else
        MCP->ADDR = AD+ 16 - ((AD % 16)-(AD % 4));
    AD = MCP->ADDR+1;
}
/* end of IRA_ADDRES */
/* start of IOP_ADDRES */
void IOP_ADDRES()
{
    if (MCP->ADATTR == SP) {
        if (AD <= MCP->ADDR) {
            if ((MCP->ADDR % 16) != 0)
                ER = true;
        }
        else
        ER = true;
    }
    else
        MCP->ADDR = AD+ 16 - (AD % 16);
    AD = MCP->ADDR+1;
}
/* end of IOP_ADDRES */
/* start of IAB_ADDRES */
void IAB_ADDRES()
{
    if (MCP->ADATTR == SP) {
        if (AD <= MCP->ADDR) {
            if ((MCP->ADDR % 64) - (MCP->ADDR % 4) != 0)
                ER = true;
        }
        else
        ER = true;
    }
    else
    MCP->ADDR = AD+ 64 - (AD % 64);
    AD = MCP->ADDR+1;
}
/* end of IAB_ADDRES */
/* start of PASS2 */
void PASS2()
{
    ER = false;
    MCP = BTMIDCODE;
    AD = MCP->ADDR;
    while (MCP != NULL && !ER) {
        switch (MCP->REF) {
        case NOLBL: case NREF: case OTHER:
            NOOP_ADDRES(); break;
        case IRAREF: IRA_ADDRES(); break;
        case IOPREF: IOP_ADDRES(); break;
        case IABREF: IAB_ADDRES(); break;
        default:
            assert(0);
            break;
        }
        if (AD-1 > MAXADRS) {
            ER = true;
            printf("ADRS OVER\n");
        }
        if (ER) {
            if (MODE_S)
                fprintf(SOFP,
                        "ERROR: INVALID SPECIFIED ADDRESS IN %d\n",
                        MCP->ADDR);
                ERRO = true;
        }
        MCP = MCP->NEXTMID;
    }
}
/* end of PASS2 */
/* start of PASS3 */
/*    procedure PASS3;*/
int OBJCODE;
TYMDP MCP;
CHAR4 HEXCODE;

/* start of LTMOD */
int LTMOD(int LT, int I)
{
    if (LT < 0)
        LT = LT+32767+1;
    return LT % I;
}
/* end of LTMOD */
/* start of LTDIV */
int LTDIV(int LT, int I)
{
    if (LT < 0)
        return (int)((LT+65536.0)/I);
    else
        return LT / I;
}
/* end of LTDIV */
/* start of TRANSFORM */
void TRANSFORM(int LNGTH)
{
    char HD;
    int I, HEX;

    strncpy(HEXCODE, "    ", 4);
    for (I = LNGTH-1; I >= 0; I--) {
        HEX = LTMOD(OBJCODE, 16);
        OBJCODE = LTDIV(OBJCODE, 16);
        if (HEX >= 0 && HEX <= 9)
            HD = HEX + 48;
        else if (HEX >= 10 && HEX <= 15)
            HD = HEX + 55;
        else
            assert(0);
        HEXCODE[I] = HD;
    }
}
/* end of TRANSFORM */
/* start of OBJECT */
/*      procedure OBJECTFILE;*/
/* start of OADDRESS */
void OADDRESS()
{
    int I;

    OBJCODE = MCP->ADDR;
    TRANSFORM(3);
    if (MODE_O) {
        for (I = 0; I < 3; I++)
            fprintf(OFP, "%c", HEXCODE[I]);
        fprintf(OFP, "  ");
    }
}
/* end of OADDRESS */
/* start of UPPER_PART */
void UPPER_PART()
{
    int I;

    if (MCP->LB == DLB)
        OBJCODE = 15;
    else
        OBJCODE = MCP->LB - R0L;
    if (MCP->RB == DRB)
        OBJCODE = OBJCODE*16 + 15;
    else
        OBJCODE = OBJCODE*16 + (MCP->RB - R0R);

    TRANSFORM(2);

    if (MODE_O)
        for (I = 0; I < 2; I++)
            fprintf(OFP, "%c", HEXCODE[I]);
}
/* end of UPPER_PART */
/* start of MIDLE_PART */
void MIDLE_PART()
{
    if (MCP->AL == DAL)
        OBJCODE = 7;
    else
        OBJCODE = MCP->AL - ADDOP;
    if (MCP->SH == DSH)
        OBJCODE = OBJCODE*8 + 7;
    else
        OBJCODE = OBJCODE*8 + (MCP->SH - SLL);
    if (MCP->SB == DSB)
        OBJCODE = OBJCODE*16 + 15;
    else 
        OBJCODE = OBJCODE*16 + (MCP->SB - R0S);
    if (MCP->MM == DMM)
        OBJCODE = OBJCODE*4 + 3;
    else
        OBJCODE = OBJCODE*4 + (MCP->MM - RM);
    if (MCP->SQ == DSQ)
        OBJCODE = OBJCODE*16 + 15;
    else
        OBJCODE = OBJCODE*16 + (MCP->SQ - B);

    if (MCP->AL == DAL)
        OBJCODE = 7;
    else
        OBJCODE = MCP->AL - ADDOP;
    if (MCP->SH == DSH)
        OBJCODE = OBJCODE*8 + 7;
    else
        OBJCODE = OBJCODE*8 + (MCP->SH - SLL);
    if (MCP->SB == DSB)
        OBJCODE = OBJCODE*16 + 15;
    else
        OBJCODE = OBJCODE*16 + (MCP->SB - R0S);
    if (MCP->MM == DMM)
        OBJCODE = OBJCODE*4 + 3;
    else
        OBJCODE = OBJCODE*4 + (MCP->MM - RM);
    if (MCP->SQ == DSQ)
        OBJCODE = OBJCODE*16 + 15;
    else
        OBJCODE = OBJCODE*16 + (MCP->SQ - B);

    TRANSFORM(4);

    if (MODE_O)
        fprintf(OFP, "%s", HEXCODE);
}
/* end of MIDLE_PART */
/* start of LOWER_PART */
void LOWER_PART()
{
    if (MCP->SQ == B || MCP->SQ == BP ||
        (MCP->SQ >= BT && MCP->SQ <= IAB) ||
        (MCP->SQ == EI && MCP->TS != NTS))
        MCP->LT = MCP->DEFTP->MIDP->ADDR;
    if (MCP->SQ == B || MCP->SQ == BP)
        OBJCODE = LTDIV(MCP->LT,512);
    else if (MCP->SQ == IAB || MCP->SQ == IRA)
        OBJCODE = LTDIV(MCP->LT,2048);
    else if (MCP->RB == LLT)
        OBJCODE = LTDIV(MCP->LT,8192);
    else if (MCP->TS == DTS)
        OBJCODE = 7;
    else
        OBJCODE = MCP->TS - ZER;
    if (MCP->RB == LLT)
        OBJCODE = OBJCODE*16 + LTMOD(LTDIV(MCP->LT,512),16);
    else if (MCP->EX == DEX)
        OBJCODE = OBJCODE*16 + 15;
    else
        OBJCODE = OBJCODE*16 + (MCP->EX - CM1);
    if (MCP->SQ == IOP)
        MCP->LT = LTDIV(MCP->LT,16);
    else if (MCP->SQ == IAB || MCP->SQ == IRA)
        MCP->LT = LTDIV(MCP->LT,4) + LTMOD(MCP->LT,4);
    OBJCODE = OBJCODE*512 + LTMOD(MCP->LT,512);

    TRANSFORM(4);
    if (MODE_O)
        fprintf(OFP, "%s", HEXCODE);
}
/* end of LOWER_PART */
/* start of OBJECT */
void OBJECTFILE()
{
    OADDRESS();
    UPPER_PART();
    MIDLE_PART();
    LOWER_PART();
    if (MODE_O)
        fprintf(OFP, "\n");
}
/* end of OBJECTFILE */
/* start of MNEMONIC */
/*     procedure MNEMONIC;*/
CHAR4 MNMNC;
int I;

/* start of NADDRESS */
void NADDRESS()
{
    OBJCODE = MCP->ADDR;
    TRANSFORM(3);
    fprintf(MFP, "%s", HEXCODE);
}
/* end of NADDRESS */
/* start of NLABEL */
void NLABEL()
{
    if (MCP->REF != NOLBL)
        fprintf(MFP, "%-8s", MCP->SDTP->IDENT);
    else
        fprintf(MFP, "        ");
    fprintf(MFP, " ");
}
/* end of NLABEL */
/* start of MNELB */
void MNELB()
{
    switch (MCP->LB) {
    case R0L: strncpy(MNMNC, "R0L ", 4); break;
    case R1L: strncpy(MNMNC, "R1L ", 4); break;
    case R2L: strncpy(MNMNC, "R2L ", 4); break;
    case R3L: strncpy(MNMNC, "R3L ", 4); break;
    case R4L: strncpy(MNMNC, "R4L ", 4); break;
    case R5L: strncpy(MNMNC, "R5L ", 4); break;
    case R6L: strncpy(MNMNC, "R6L ", 4); break;
    case R7L: strncpy(MNMNC, "R7L ", 4); break;
    case RB:  strncpy(MNMNC, " RB ", 4); break;
    case RBP: strncpy(MNMNC, "RBP ", 4); break;
    case PC:  strncpy(MNMNC, " PC ", 4); break;
    case IO:  strncpy(MNMNC, " IO ", 4); break;
    case MM:  strncpy(MNMNC, " MM ", 4); break;
    case IR:  strncpy(MNMNC, " IR ", 4); break;
    case FSR: strncpy(MNMNC, "FSR ", 4); break;
    case DLB: strncpy(MNMNC, "NLB ", 4); break;
    case NLB: strncpy(MNMNC, "NLB ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNELB */
/* start of MNERB */
void MNERB()
{
    switch (MCP->RB) {
    case R0R: strncpy(MNMNC, "R0R ", 4); break;
    case R1R: strncpy(MNMNC, "R1R ", 4); break;
    case R2R: strncpy(MNMNC, "R2R ", 4); break;
    case R3R: strncpy(MNMNC, "R3R ", 4); break;
    case R4R: strncpy(MNMNC, "R4R ", 4); break;
    case R5R: strncpy(MNMNC, "R5R ", 4); break;
    case R6R: strncpy(MNMNC, "R6R ", 4); break;
    case R7R: strncpy(MNMNC, "R7R ", 4); break;
    case RA:  strncpy(MNMNC, " RA ", 4); break;
    case RAP: strncpy(MNMNC, "RAP ", 4); break;
    case SLT: strncpy(MNMNC, "SLT ", 4); break;
    case LLT: strncpy(MNMNC, "LLT ", 4); break;
    case DRB: strncpy(MNMNC, "NRB ", 4); break;
    case NRB: strncpy(MNMNC, "NRB ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNERB */
/* start of MNEAL */
void MNEAL()
{
    switch (MCP->AL) {
    case ADDOP: strncpy(MNMNC, "ADD ", 4); break;
    case SUBOP: strncpy(MNMNC, "SUB ", 4); break;
    case ANDOP: strncpy(MNMNC, "AND ", 4); break;
    case OROP:  strncpy(MNMNC, " OR ", 4); break;
    case XOROP: strncpy(MNMNC, "XOR ", 4); break;
    case IALOP: strncpy(MNMNC, "IAL ", 4); break;
    case DAL:   strncpy(MNMNC, "NAL ", 4); break;
    case NAL:   strncpy(MNMNC, "NAL ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNEAL */
/* start of MNESH */
void MNESH()
{
    switch (MCP->SH) {
    case SLL: strncpy(MNMNC, "SLL ", 4); break;
    case SRL: strncpy(MNMNC, "SRL ", 4); break;
    case SLA: strncpy(MNMNC, "SLA ", 4); break;
    case SRA: strncpy(MNMNC, "SRA ", 4); break;
    case SNX: strncpy(MNMNC, "SNX ", 4); break;
    case SWP: strncpy(MNMNC, "SWP ", 4); break;
    case DSH: strncpy(MNMNC, "NSH ", 4); break;
    case NSH: strncpy(MNMNC, "NSH ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNESH */
/* start of MNESB */
void MNESB()
{
    switch (MCP->SB) {
    case R0S: strncpy(MNMNC, "R0S ", 4); break;
    case R1S: strncpy(MNMNC, "R1S ", 4); break;
    case R2S: strncpy(MNMNC, "R2S ", 4); break;
    case R3S: strncpy(MNMNC, "R3S ", 4); break;
    case R4S: strncpy(MNMNC, "R4S ", 4); break;
    case R5S: strncpy(MNMNC, "R5S ", 4); break;
    case R6S: strncpy(MNMNC, "R6S ", 4); break;
    case R7S: strncpy(MNMNC, "R7S ", 4); break;
    case SA:  strncpy(MNMNC, " SA ", 4); break;
    case SAP: strncpy(MNMNC, "SAP ", 4); break;
    case SB:  strncpy(MNMNC, " SB ", 4); break;
    case SBP: strncpy(MNMNC, "SBP ", 4); break;
    case PCS: strncpy(MNMNC, "PCS ", 4); break;
    case DSB: strncpy(MNMNC, "NSB ", 4); break;
    case NSB: strncpy(MNMNC, "NSB ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNESB */
/* start of MNEMM */
void MNEMM()
{
    switch (MCP->MM) {
    case RM:  strncpy(MNMNC, " RM ", 4); break;
    case WM:  strncpy(MNMNC, " WM ", 4); break;
    case DMM: strncpy(MNMNC, "NMM ", 4); break;
    case NMM: strncpy(MNMNC, "NMM ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNEMM */
/* start of MNESQ */
void MNESQ()
{
    switch (MCP->SQ) {
    case B:   strncpy(MNMNC, "  B ", 4); break;
    case BP:  strncpy(MNMNC, " BP ", 4); break;
    case RTN: strncpy(MNMNC, "RTN ", 4); break;
    case BT:  strncpy(MNMNC, " BT ", 4); break;
    case BF:  strncpy(MNMNC, " BF ", 4); break;
    case IOP: strncpy(MNMNC, "IOP ", 4); break;
    case IRA: strncpy(MNMNC, "IRA ", 4); break;
    case IAB: strncpy(MNMNC, "IAB ", 4); break;
    case EI:  strncpy(MNMNC, " EI ", 4); break;
    case DSQ: strncpy(MNMNC, "NSQ ", 4); break;
    case NSQ: strncpy(MNMNC, "NSQ ", 4); break;
    default:
        assert(0);
        break;
    }
    fprintf(MFP, "%s", MNMNC);
}
/* end of MNESQ */
/* start of MNETS */
void MNETS()
{
    if (MCP->SQ == B ||   MCP->SQ == BP ||
        MCP->SQ == IRA || MCP->SQ == IAB) {

        OBJCODE = LTDIV(MCP->LT,512);
        TRANSFORM(3);
        fprintf(MFP, "%s", HEXCODE);
    }
    else if (MCP->RB == LLT) {
        OBJCODE = LTDIV(MCP->LT,8192);
        TRANSFORM(3);
        fprintf(MFP, "%s", HEXCODE);
    }
    else {
        switch (MCP->TS) {
        case ZER: strncpy(MNMNC, "ZER ", 4); break;
        case NEG: strncpy(MNMNC, "NEG ", 4); break;
        case CRY: strncpy(MNMNC, "CRY ", 4); break;
        case OV:  strncpy(MNMNC, " OV ", 4); break;
        case T:   strncpy(MNMNC, "  T ", 4); break;
        case CZ:  strncpy(MNMNC, " CZ ", 4); break;
        case DTS: strncpy(MNMNC, "NTS ", 4); break;
        case NTS: strncpy(MNMNC, "NTS ", 4); break;
        default:
            assert(0);
            break;
        }
        fprintf(MFP, "%s", MNMNC);
    }
}
/* end of MNETS */
/* start of MNEEX */
void MNEEX()
{
    if (MCP->RB == LLT) {
        OBJCODE = LTMOD(LTDIV(MCP->LT,512),16);
        TRANSFORM(3);
        fprintf(MFP, "%s", HEXCODE);
    }
    else {
        switch (MCP->EX) {
        case CM1: strncpy(MNMNC, "CM1 ", 4); break;
        case FLS: strncpy(MNMNC, "FLS ", 4); break;
        case ASC: strncpy(MNMNC, "ASC ", 4); break;
        case AS1: strncpy(MNMNC, "AS1 ", 4); break;
        case LIR: strncpy(MNMNC, "LIR ", 4); break;
        case LIO: strncpy(MNMNC, "LIO ", 4); break;
        case SC:  strncpy(MNMNC, " SC ", 4); break;
        case EXIO: strncpy(MNMNC, "EIO ", 4); break;
        case ST:  strncpy(MNMNC, " ST ", 4); break;
        case RT:  strncpy(MNMNC, " RT ", 4); break;
        case INA: strncpy(MNMNC, "INA ", 4); break;
        case INB: strncpy(MNMNC, "INB ", 4); break;
        case DCB: strncpy(MNMNC, "DCB ", 4); break;
        case HLT: strncpy(MNMNC, "HLT ", 4); break;
        case SOV: strncpy(MNMNC, " OV ", 4); break;
        case DEX: strncpy(MNMNC, "NEX ", 4); break;
        case NEX: strncpy(MNMNC, "NEX ", 4); break;
        default:
            assert(0);
            break;
        }
        fprintf(MFP, "%s", MNMNC);
    }
}
/* end of MNEEX */
/* start of MNELT */
void MNELT()
{
    OBJCODE = LTMOD(MCP->LT,512);
    TRANSFORM(3);
    fprintf(MFP, "%s", HEXCODE);
}
/* end of MNELT */
/* start of MNEMONIC */
void MNEMONIC()
{
    NLABEL();
    NADDRESS();
    MNELB();
    MNERB();
    MNEAL();
    MNESH();
    MNESB();
    MNEMM();
    MNESQ();
    MNETS();
    MNEEX();
    MNELT();
    if (MCP->FLG == DEF)
        fprintf(MFP, "%-8s", MCP->DEFTP->IDENT);
    else
        fprintf(MFP, "        ");
    fprintf(MFP, "\n");
}
/* end of MNEMONIC */
/* start of PASS3 */
void PASS3()
{
    MCP = BTMIDCODE;
    while (MCP->NEXTMID != NULL) {
        OBJECTFILE();
        if (MODE_M)
            MNEMONIC();
        MCP = MCP->NEXTMID;
    }
}
/* end of PASS3 */
/* start of EX_ASSEMBLE */
void EX_ASSEMBLE()
{
    ERRO = false;
    PASS1();
    if (!ERRO) PASS2();
    if (!ERRO) PASS3();
    if (!ERRO) {
        if (MODE_S)
            fprintf(SOFP,
                    "THERE WAS NO ERROR IN ASSEMBLE.\n");
        printf("\n");
        printf("NORMAL TERMINATION!\n");
        printf("\n");
    }
    else {
        printf("\n");
        printf("ERROR OCCURRED IN ASSEMBLE-TIME!\n");
        printf("\n");
    }
}
/* end of EX_ASSEMBLE */
/* start of USER_INTERFACE */
/*  procedure USER_INTERFACE;{  USER INTERFACE ROUTINE  } */
const char *SP10="          ";
const char CR  = 13;
const char DEL = 24;
const char BS  = 8;
const char BEL = 7;
/*(*  var I,J,K:integer;*)*/
int I, J;
/*(*      SIFILE,SOFILE,OFILE,MFILE:CHAR10;*)*/
FILENAME SIFILE;        /* source input file name */
FILENAME SOFILE;        /* source output file (listing file) name */
FILENAME OFILE;         /* object file name */
FILENAME MFILE;         /* mnemonic file name */

char CONT, STFL;
bool ENDOPEN;

/* start of CRATOFILE */
void CRATOFILE()
{
    strncpy(OFILE, SIFILE, 13);
    strncat(OFILE, ".o", 13);
    if ((OFP = fopen(OFILE, "w")) == NULL) {
        printf("CAN NOT CREATE FILE '%s'\n", OFILE);
        exit(1);
    }
}
/* end of CRATOFILE */
/* start of CRATSOFILE */
void CRATSOFILE()
{
    strncpy(SOFILE, SIFILE, 13);
    strncat(SOFILE, ".s", 13);
    if ((SOFP = fopen(SOFILE, "w")) == NULL) {
        printf("CAN NOT CREATE FILE '%s'\n", SOFILE);
        exit(1);
    }
}
/* end of CRATSOFILE */
/* start of CRATMFILE */
void CRATMFILE()
{
    strncpy(MFILE, SIFILE, 13);
    strncat(MFILE, ".m", 13);
    if ((MFP = fopen(MFILE, "w")) == NULL) {
        printf("CAN NOT CREATE FILE '%s'\n", MFILE);
        exit(1);
    }
}
/* end of CRATMFILE */
/* start of INPUT_SOURCE_FILE */
void INPUT_SOURCE_FILE()
{
    size_t J;
    char *cp;

    ENDOPEN = false;
    do {
        printf("SOURCE FILE NAME? ");
//        strncpy(SIFILE, SP10, MAX_PATH);      /* useless */
        fgets(SIFILE, MAX_PATH, stdin);
        cp = strchr(SIFILE, '\n');
        if (cp != NULL) *cp = '\0';             /* remove newline */

        J = strlen(SIFILE);

        if (J <= MAX_PATH-2) {
            if (J > 0) {
                if ((SIFP = fopen(SIFILE, "r")) == NULL) {
                    printf("FILE '%s' DOES NOT EXIST.\n", SIFILE);
                }
                else {
                    ENDOPEN = true;
                }
            }
        }
        else
            printf("FILE NAME TOO LONG.\n");
    } while (!ENDOPEN);

    if (J == 1) ENDOPEN = false;        /* ??? what does it work ? */
}
/* end of INPUT_SOURCE_FILE */
/* start of INPUT_OPTION */
void INPUT_OPTION()
{
    char OPTIONS[BUFSIZ];
    int I;
    size_t len;
    char *cp;

    MODE_S = false;
    MODE_O = false;
    MODE_M = false;
    
    printf("OPTION? ");
    fgets(OPTIONS, BUFSIZ, stdin);
    cp = strchr(OPTIONS, '\n');
    if (cp != NULL) *cp = '\0';             /* remove newline */
    len = strlen(OPTIONS);

    for (I = 0; I < len; I++) {
        switch (toupper(OPTIONS[I])) {
        case 'S': MODE_S = true; break;
        case 'O': MODE_O = true; break;
        case 'M': MODE_M = true; break;
        case ' ':
        case '\t':
            break;
        default:
            printf("ILLEGAL OPTION'%c' CHARACTER SPECIFIED.\n", OPTIONS[I]);
            break;
        }
    }
}
/* end of INPUT_OPTION */
/* start of USER_INTERFACE */
void USER_INTERFACE()
{
    /*  USER INTERFACE ROUTINE  */

    if (!CONTINUE) {
        if (!EXECUTE) {
            if (MODE_S) fclose(SOFP);
            if (MODE_O) fclose(OFP);
            if (MODE_M) fclose(MFP);
        }
        printf("CONTINUE? (Y/N):");
        do {
            CONT = toupper(getchar());
        } while (CONT != 'Y' && CONT != 'N');
        printf("\n");
        if (CONT == 'Y')
            CONTINUE = true;
        else
            CONTINUE = false;
    }
    if (CONTINUE) {
        INPUT_SOURCE_FILE();
        if (ENDOPEN) {
            INPUT_OPTION();
            printf("START? (Y/N):");
            do {
                STFL = toupper(getchar());
            } while (STFL != 'Y' && STFL != 'N');
            printf("\n");
            if (STFL == 'Y') {
                EXECUTE = true;
                if (MODE_O) CRATOFILE();
                if (MODE_S) CRATSOFILE();
                if (MODE_M) CRATMFILE();
            }
            else
                EXECUTE = false;
        }
        else
            EXECUTE = false;
    }
}
/* end of USER_INTERFACE */

/* main of MICRO-1 */
int main()
{
    printf("\n");
    printf("\n");
/*    writeln('   *** MICRO-1 MICROASSEMBLER (Ver. 3.1a) ***');*/
    printf("   *** MICRO-1 MICROASSEMBLER (C-Ver. 1.0) ***\n");
    printf("\n");

    CONTINUE = true;
    USER_INTERFACE();
    while (CONTINUE) {
        if (EXECUTE) EX_ASSEMBLE();
        CONTINUE = false;
        USER_INTERFACE();
    }
}

/* end of masm.c */
