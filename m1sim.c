/*
 *
 *      MICRO-1 simulator (ANSI C-version)
 *      by kim         Ver. 1.0  2016.3
 *
 *      based on MICRO-1 SIMULATOR (Ver. 2.1)
 *      PC-9801 CP/M-86 Turbo-Pascal version
 *
 *      by M.INAGAWA   Ver. 1.0  1984.4
 *                     Ver. 2.0  1985.4
 *      by H.SAITO     Ver. 2.1  1986.5
 */
//#define DEBUG

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS         /* disable warnings for insecure libraries used */
/*#define strtoll         _strtoi64*/   /* Visual Studio 2015 has 'strtoll' function */
#define strcasecmp      _stricmp
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_CM          0x1000          /* max size of CM (4K Ctrl-Word) */
#define MAX_MM          0x10000         /* max size of MM (64K Word) */

#define MAX_CMARSTACK   8               /* max size of CMAR stack (depth 8) */
#define MAX_REGS        8               /* max number of registers */

#define MAX_BRKPOINT    32              /* max number of break-points allowed */
#define MAX_TRCPOINT    32              /* max number of trace-points allowed */

#define C_BITLEN        8               /* bit length of C */
#define C_BITMASK       ((1 << C_BITLEN) - 1)   /* bit mask for C */

#define PROMPT          ">> "

typedef unsigned long long MCtrlWord;   /* MICRO-1 control word (40bit) */
typedef unsigned short  MInstWord;      /* MICRO-1 instruction word (16bit) */
typedef long long       MLLong;
typedef long            MLong;
typedef unsigned short  MWord;          /* MICRO-1 data word (16bit) */
typedef unsigned int    MAddr;          /* MICRO-1 address (16bit) */
typedef int             MInt;
typedef char            MChar;
typedef enum MBool { MFALSE = 0, MTRUE, }       MBool;

typedef enum MemCycle   { MemNop = 0, MemRead, MemWrite, } MemCycle;
typedef enum IoCycle    { IoNop = 0,  IoRead,  IoWrite,  } IoCycle;

typedef struct AddrMark { MBool cond; MAddr addr; } AddrMark;

typedef enum DevId {
    DevNULL = 0,
    DevREG, DevPC, DevIR,
    DevLBUS, DevSBUS, DevABUS, DevIOBUS,
    DevFSR,
    DevRBUS, DevCM, DevMM, DevMAR, DevCMAR,
    DevC, DevFLAG,

    /* meta devices (not real device) */
    DevSTAR,
    DevRSTAR, DevBSTAR,
    DevENDMARK,
    DevERR,
    DevMAX,     /* this should be the last */
} DevId;

typedef struct DevNameId {
    DevId       id;
    char        *name;
} DevNameId;

DevNameId DevNameTable[] = {
//    { DevNULL, "" },
//    { DevREG,  "" },
    { DevPC,    "PC" },
    { DevIR,    "IR" },
    { DevLBUS,  "LBUS" },
    { DevSBUS,  "SBUS" },
    { DevABUS,  "ABUS" },
    { DevIOBUS, "IOBUS" },
    { DevFSR,   "FSR" },
    { DevRBUS,  "RBUS" },
    { DevCM,    "CM" },
    { DevMM,    "MM" },
    { DevMAR,   "MAR" },
    { DevCMAR,  "CMAR" },
    { DevC,     "C" },
    { DevFLAG,  "FLAG" },
    { DevSTAR,  "*" },
    { DevRSTAR, "R*" },
    { DevBSTAR, "B*" },
    { DevENDMARK, "." },
//    { DevERR,   "" },
//    { DevMAX,   "" },
};

#define DevNameTableSize        (sizeof(DevNameTable)/sizeof(DevNameId))

typedef enum ErrCode {
    ErrNone = 0,
    ErrUndefinedMicroInst,      /*  1 */
    ErrCMARStackEmpty,          /*  2 */
    ErrOutOfMMAddrLimit,        /*  3 */
    ErrOutOfCMAddrLimit,        /*  4 */
    ErrInvalidLoadFileFormat,   /*  5 */
    ErrIllegalChar,             /*  6 */
    ErrIllegalCommandOrData,    /*  7 */
    ErrUndefinedIoInstOnIR,     /*  8 */
    ErrUndefinedIoDevOnIR,      /*  9 */
    ErrRequestInputForAction,   /* 10 */
    ErrRequestInputForTarget,   /* 11 */
    ErrNoDataOnIoDevice,        /* 12 */
    ErrIllegalCommand,          /* 13 */
    ErrCMARStackOverflow,       /* 14 */
    ErrBPOverflow,              /* 15 */
    ErrTPOverflow,              /* 16 */
    ErrIoError,                 /* 17 */

    /* added */
    ErrMemCycleNotRead,

    /* end marker */
    ErrCodeMAX,
} ErrCode;


/*
 *
 *      hard device image variables
 *
 */

MInstWord       IR;                     /* instruction register (16bit) */
MAddr   PC;                             /* program counter (16bit) */
MWord   LBUS, RBUS, SBUS, ABUS, IOBUS;  /* buses (16bit) */
MWord   R[MAX_REGS];                    /* general-purpose registers (16bit) */
MCtrlWord       CMDR;                   /* control memory data register (40bit) */

MCtrlWord       CM[MAX_CM];             /* control memory */
MWord           MM[MAX_MM];             /* main memory */
MWord           CMAR_STACK[MAX_CMARSTACK]; /* CMAR stack */
MInt            CMAR_STACK_POS;         /* CMAR stack pointer */

MAddr   MAR;                            /* memory address register (16bit) */
MAddr   CMAR;                           /* control memory address register (12bit) */

MInt    C;                              /* counter (8bit) */
MBool   CRY, NEG, OV, ZER, CZ, T;       /* flags */

MWord   FSR;                            /* flag save register */

FILE    *INPUT;                 /* Input device file (ancient Card Reader ?) */
FILE    *OUTPUT;                /* Output device file (ancient Line Printer ?) */


/*
 *
 *      system related variables
 *
 */

MBool   HALT_ON, OV_LAMP, BREAK_ON, TRACE_ON;
MInt    STEP, MAXSTEP;
MInt    LBF, RBF, ALF, SHF, SLF, SBF, MMF, SQF, TSF, EXF, LTF;

MemCycle        MEM_CYCLE;              /* current cycle of main memory access */
IoCycle         IO_CYCLE;               /* current cycle of I/O access */

AddrMark        BPoint[MAX_BRKPOINT];   /* break point table */
AddrMark        TPoint[MAX_TRCPOINT];   /* trace point table */

MBool   TraceDevice[DevMAX];            /* trace device group */
MBool   TraceRegister[MAX_REGS];        /* trace register number */

static char *
remove_heading_space(char *str)
{
    char *p, *q;
    assert(str != NULL);
    for (p = str; *p; p++)
        if (!isspace(*p))
            break;
    for (q = str; *p; )
        *q++ = *p++;
    *q = '\0';

    return str;
}

static char *
read_line(char linebuf[], int linebufsize, FILE *fp)
{
    char *ret;
    int len;
    if ((ret = fgets(linebuf, BUFSIZ, fp)) == NULL)
        return NULL;

    remove_heading_space(linebuf);

    len = strlen(linebuf);
    if (len > 0 && linebuf[len-1] == '\n')      /* remove EOL */
        linebuf[len-1] = '\0';

    return linebuf;
}


void
error(ErrCode code)
{
    printf("\nERROR %d\n", code);
    HALT_ON = MTRUE;

    switch (code) {
    case ErrNone:
    case ErrCodeMAX:
        break;
    case ErrUndefinedMicroInst:
        printf(" UNDEFINED MICRO INSTRUCTION CODE !");
        break;
    case ErrCMARStackEmpty:
        printf(" CAN'T POPUP FROM EMPTY CMAR-STACK !");
        break;
    case ErrOutOfMMAddrLimit:
        printf(" OUT OF MM ADDRESS LIMIT (0-%X) ! ", MAX_MM-1);
        break;
    case ErrOutOfCMAddrLimit:
        printf(" OUT OF CM ADDRESS LIMIT (0-%X) ! ", MAX_CM-1);
        break;
    case ErrInvalidLoadFileFormat:
        printf(" INVALID LOAD FILE FOMAT !");
        break;
    case ErrIllegalChar:
        printf(" ILLEGAL CHARACTER !");
        break;
    case ErrIllegalCommandOrData:
        printf(" ILLEGAL INPUT COMMAND OR DATA !");
        break;
    case ErrUndefinedIoInstOnIR:
        printf(" UNDEFINED I/O INSTRUCTION ON IR !");
        break;
    case ErrUndefinedIoDevOnIR:
        printf(" UNDEFINED I/O DEVICE NO. ON IR !");
        break;
    case ErrRequestInputForAction:
        printf(" PLEASE KEY IN 'SET','RESET','DISPLAY' OR '.' !");
        break;
    case ErrRequestInputForTarget:
        printf(" PLEASE KEY IN 'REG','BUS','CM','MM' OR '.' !");
        break;
    case ErrNoDataOnIoDevice:
        printf(" NO DATA ON I/O DEVICE !");
        break;
    case ErrIllegalCommand:
        printf(" *** ILLEGAL COMMAND ***");
        break;
    case ErrCMARStackOverflow:
        printf(" CMAR STACK OVERFLOW (STACK DEPTH IS 8 LEVEL)");
        break;
    case ErrBPOverflow:
        printf(" THE NUMBER OF BREAK POINT IS OVERFLOW");
        break;
    case ErrTPOverflow:
        printf(" THE NUMBER OF TRACE POINT IS OVERFLOW");
        break;
    case ErrIoError:
        printf(" I/O ERROR !");
        break;

        /* added */
    case ErrMemCycleNotRead:
        printf(" MEMORY CYCLE IS NOT READY TO READ");
        break;

    default:
        printf("unknown error code %d\n", code);
        break;
    }
    printf("\n");
}


/* CM ADDRESS(CMAR) RANGE CHECK */
static MBool
CMAR_address_is_OK(MAddr addr)
{
//    if (addr >= 0 && addr < MAX_CM)
    if (addr < MAX_CM)
        return MTRUE;
    error(ErrOutOfCMAddrLimit);
    return MFALSE;
}

/* MM ADDRESS(MAR) RANGE CHECK */
static MBool
MAR_address_is_OK(MAddr addr)
{
//    if (addr >= 0 && addr < MAX_MM)
    if (addr < MAX_MM)
        return MTRUE;
    error(ErrOutOfMMAddrLimit);
    return MFALSE;
}


/*
 * CMAR STACK OPERATIONS
 */

void
push_CMAR_STACK()
{
    if (CMAR_STACK_POS >= MAX_CMARSTACK) {
        error(ErrCMARStackOverflow);
        return;
    }

    CMAR_STACK[CMAR_STACK_POS++] = CMAR+1;
}

void
pop_CMAR_STACK()
{
    if (CMAR_STACK_POS <= 0) {
        error(ErrCMARStackEmpty);
        return;
    }

    CMAR = CMAR_STACK[--CMAR_STACK_POS];
}

/*
 * MACHINE OPARATION SIMULATE ROUTINES
 */

/* MICRO INSTRUCTION FETCH & DECODE */
void
fetch_decode()
{
    CMDR = 0;
    if (CMAR_address_is_OK(CMAR)) {
        CMDR = CM[CMAR];
    }
    else {
        error(ErrOutOfCMAddrLimit);
        CMDR = -1;
    }

    /* extract each field */
    LBF = (CMDR >> 36) & 0xf;   /* bit 39-36 */
    RBF = (CMDR >> 32) & 0xf;   /* bit 35-32 */
    ALF = (CMDR >> 29) & 0x7;   /* bit 31-29 */
    SHF = (CMDR >> 26) & 0x7;   /* bit 28-26 */
    SBF = (CMDR >> 22) & 0xf;   /* bit 25-22 */
    MMF = (CMDR >> 20) & 0x3;   /* bit 21-20 */
    SQF = (CMDR >> 16) & 0xf;   /* bit 19-16 */
    TSF = (CMDR >> 13) & 0x7;   /* bit 15-13 */
    EXF = (CMDR >>  9) & 0xf;   /* bit 12-9  */
    LTF = (CMDR >>  0) & 0x1ff; /* bit  8-0  */
#ifdef  DEBUG
    printf("CMAR:%03X LBF=%X RBF=%X ALF=%X SHF=%X SBF=%X"
           " MMF=%X SQF=%X TSF=%X EXF=%X LTF=%X\n",
           CMAR, LBF, RBF, ALF, SHF, SBF, MMF, SQF, TSF, EXF, LTF);
#endif
}


/* TEST CONDITION */
MBool
eval_condition(MInt TSF)
{
    MBool cond = MFALSE;

    switch (TSF) {
    case 0:
        cond = ZER;
        break;
    case 1:
        cond = NEG;
        break;
    case 2:
        cond = CRY;
        break;
    case 3:
        cond = OV;
        break;
    case 4:
        cond = T;
        break;
    case 5:
        cond = C? MFALSE: MTRUE;
        break;
    case 6:
        error(ErrUndefinedMicroInst);
        break;
    case 7:
        cond = MTRUE;
        break;
    default:
        printf("something is wrong (TSF=%x)\n", TSF);
        break;
    }

    return cond;
}


/* SEQUENCE CONTROL */
void
exec_SQF()
{
    switch (SQF) {
    case 0:     /* B */
        CMAR = (TSF << 9) | LTF;
        break;
    case 1:     /* BP */
        push_CMAR_STACK();
        CMAR = (TSF << 9) | LTF;
        break;
    case 2:     /* BRT */
        pop_CMAR_STACK();
        break;
    case 3:     /* BT */
        if (eval_condition(TSF)) {
            CMAR = (CMAR & ~0x1ff) | LTF;
        }
        else {
            CMAR++;
        }
        break;
    case 4:     /* BF */
        if (!eval_condition(TSF)) {
            CMAR = (CMAR & ~0x1ff) | LTF;
        }
        else {
            CMAR++;
        }
        break;
    case 5:     /* IOP */
        CMAR = ((LTF & 0xff) << 4) | ((IR >> 12) & 0xf);
        break;
    case 6:     /* IRA */
        CMAR = ((TSF & 1) << 11) | ((LTF >> 2) << 4) |
            (((IR >> 10) & 3) << 2) | (LTF & 3);
        break;
    case 7:     /* IAB */
        CMAR = ((TSF & 1) << 11) | ((LTF >> 4) << 6) |
            (((IR >> 8) & 15) << 2) | (LTF & 3);
        break;
    case 8:     /* EI */
        if (!eval_condition(TSF)) {
            CMAR = (CMAR & ~0x1ff) | LTF;
        }
        else {
            CMAR = 0;           /* goto the top of CM */
        }
        break;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
        error(ErrUndefinedMicroInst);
        break;
    case 15:    /* NSQ */
        CMAR++;
        break;
    default:
        printf("something is wrong (SQF=%x)\n", SQF);
        break;
    }
}


/* XXXX -> LBUS */
void
exec_LBF()
{
    switch (LBF) {
    case 0:    case 1:    case 2:    case 3:
    case 4:    case 5:    case 6:    case 7:     /* R0L..R7L */
        LBUS = R[LBF];
        break;
    case 8:     /* RB */
        LBUS = R[((IR >> 8) & 3)];
        break;
    case 9:     /* RBP */
        LBUS = R[((IR >> 8) & 3) + 1];
        break;
    case 10:    /* PC */
        LBUS = PC;
        break;
    case 11:    /* IO */
        LBUS = IOBUS;
        break;
    case 12:    /* MDR */
        if (MEM_CYCLE == MemRead) {
            if (MAR_address_is_OK(MAR)) {
                LBUS = MM[MAR];
            }
            else {
                error(ErrOutOfMMAddrLimit);
            }
        }
        else {
            error(ErrMemCycleNotRead);
        }
        break;
    case 13:    /* IR */
        LBUS = IR;
        break;
    case 14:    /* FSR */
        LBUS = FSR;
        break;
    case 15:    /* NLB */
        LBUS = 0;
        break;

    default:
        printf("something is wrong (LBF=%x)\n", LBF);
        break;
    }

    if (EXF == 5) {
        IOBUS = LBUS;
    }
}


/* XXXX -> RBUS */

void
exec_RBF()
{
    switch (RBF) {
    case 0:    case 1:    case 2:    case 3:
    case 4:    case 5:    case 6:    case 7:     /* R0R..R7R */
        RBUS = R[RBF];
        break;
    case 8:     /* RA */
        RBUS = R[((IR >> 10) & 3)];
        break;
    case 9:     /* RAP */
        RBUS = R[((IR >> 10) & 3) + 1];
        break;
    case 10:    /* SLT */
        RBUS = LTF;
        break;
    case 11:    /* LLT */
        RBUS = (TSF << 13) | (EXF << 9) | LTF;
        /* SET USED FIELD TO NOP */
        /* TSF = 7; EXF = 15; */
        break;
    case 12:
    case 13:
    case 14:
        error(ErrUndefinedMicroInst);
        break;
    case 15:    /* NRB */
        RBUS = 0;
        break;

    default:
        printf("something is wrong (RBF=%x)\n", RBF);
        break;
    }
}

/* ADD OPERATION */
static void
ADDOp()
{
    MInt CIN;

    if (EXF == 2) {             /* ASC */
        CIN = CRY? 1: 0;
    }
    else if (EXF == 3) {        /* AS1 */
        CIN = 1;
    }
    else {
        CIN = 0;
    }

    ABUS = LBUS + RBUS + CIN;
#ifdef  DEBUG
    printf("ADDOp: LBUS=%x RBUS=%x CIN=%d => ABUS=%x\n",
           LBUS, RBUS, CIN, ABUS);
#endif

    /* OV */
    OV = ((LBUS & RBUS & ~ABUS & 0x8000) | (~LBUS & ~RBUS & ABUS & 0x8000))?
        MTRUE: MFALSE;
    /* CRY */
    CRY = CIN? MTRUE: MFALSE;
    /* ZER */
    ZER = ABUS? MFALSE: MTRUE;
    /* NEG */
    NEG = (ABUS & 0x8000)? MTRUE: MFALSE;
}

/* SUB OPERATION */
static void
SUBOp()
{
    MInt CIN;

    if (EXF == 2) {             /* ASC */
        CIN = CRY? 1: 0;
    }
    else if (EXF == 3) {        /* AS1 */
        CIN = 1;
    }
    else {
        CIN = 0;
    }

    ABUS = LBUS - RBUS - CIN;
#ifdef  DEBUG
    printf("SUBOp: LBUS=%x RBUS=%x CIN=%d => ABUS=%x\n",
           LBUS, RBUS, CIN, ABUS);
#endif

    /* OV */
    OV = ((LBUS & ~RBUS & ABUS & 0x8000) | (~LBUS & RBUS & ~ABUS & 0x8000))? MTRUE: MFALSE;
    /* CRY */
    CRY = CIN? MFALSE: MTRUE;   /* CIN? MTRUE: MFALSE ? */
    /* ZER */
    ZER = ABUS? MFALSE: MTRUE;
    /* NEG */
    NEG = (ABUS & 0x8000)? MTRUE: MFALSE;
}


/* AND OPERATION */
static void
ANDOp()
{
    ABUS = LBUS & RBUS;

    /* ZER */
    ZER = ABUS? MFALSE: MTRUE;
}


/* OR OPERATION */
static void
OROp(MBool ZERO_ON)
{
    ABUS = LBUS | RBUS;

    /* ZER */
    if (ZERO_ON)
        ZER = ABUS? MFALSE: MTRUE;
}

/* XOR OPERATION */
static void
XOROp()
{
    ABUS = LBUS ^ RBUS;

    /* ZER */
    ZER = ABUS? MFALSE: MTRUE;
}

/* ALU OPERATION */
void
exec_ALF()
{
    if (ALF == 5) {                     /* IAL */
        ALF = (IR >> 4) & 0x7;
    }

    switch (ALF) {
    case 0: ADDOp(); break;             /* ADD */
    case 1: SUBOp(); break;             /* SUB */
    case 2: ANDOp(); break;             /* AND */
    case 3: OROp(MTRUE); break;         /* OR  */
    case 4: XOROp(); break;             /* XOR */

    case 5:
    case 6:
        error(ErrUndefinedMicroInst);
        break;

    case 7: OROp(MFALSE); break;        /* NAL */
    default:
        printf("something is wrong (ALF=%x)\n", ALF);
        break;
    }
}


/* SLL:SHIFT LEFT LOGICAL */
void
SLLOp(MBool shift_in)
{
    CRY = (ABUS & 0x8000)? MTRUE: MFALSE;
    SBUS = ABUS << 1;
    if (shift_in)
        SBUS |= 1;
}

/* SRL:SHIFT RIGHT LOGICAL */
void
SRLOp(MBool shift_in)
{
    CRY = (ABUS & 1)? MTRUE: MFALSE;
    SBUS = ABUS >> 1;
    if (shift_in)
        SBUS |= 0x8000;
}


/* SLA:SHIFT LEFT ARITHMATIC */
void
SLAOp(MBool shift_in)
{
    CRY = (ABUS & 0x8000)? MTRUE: MFALSE;
    SBUS = ABUS << 1;
    if (shift_in)
        SBUS |= 1;
    /* OV */
    OV = ((ABUS & ~SBUS & 0x8000) | (~ABUS & SBUS & 0x8000))? MTRUE: MFALSE;
}


/* SRA:SHIFT RIGHT ARITHMATIC */
void
SRAOp()
{
    CRY = (ABUS & 1)? MTRUE: MFALSE;
    SBUS = ABUS >> 1;
    if ((ABUS & 0x8000))
        SBUS |= 0x8000;
}


/* SIGN EXTEND */
void
SNXOp()
{
    if ((ABUS & 0x80)) {
        SBUS = ABUS | 0xff00;
        CRY = MTRUE;
    }
    else {
        SBUS = ABUS;
        CRY = MFALSE;
    }
}


/* SWAP UPPER AND LOWER */
void
SWPOp()
{
    SBUS = ((ABUS & 0xff) << 8) | ((ABUS & 0xff00) >> 8);
}


/* SHIFT OPERATION */
void
exec_SHF()
{
    MBool shift_in;

    if (EXF == 4)
        IR = LBUS;

    switch (EXF) {
    case 2: shift_in = CRY; break;
    case 3: shift_in = MTRUE; break;
    default:
        shift_in = MFALSE;
        break;
    }

    switch (SHF) {
    case 0: SLLOp(shift_in); break;
    case 1: SRLOp(shift_in); break;
    case 2: SLAOp(shift_in); break;
    case 3: SRAOp(); break;
    case 4: SNXOp(); break;
    case 5: SWPOp(); break;
    case 7: SBUS = ABUS; break;

    case 6: error(ErrUndefinedMicroInst); break;
    default:
        printf("something is wrong (SHF=%x)\n", SHF);
        break;
    }
}


/* SBUS -> XXX */
void
exec_SBF()
{
    switch (SBF) {
    case 0:    case 1:    case 2:    case 3:
    case 4:    case 5:    case 6:    case 7:     /* R0S..R7S */
        R[SBF] = SBUS;
        break;
    case 8:     /* SA */
        R[((IR >> 12) & 3)] = SBUS;
        break;
    case 9:     /* SAP */
        R[((IR >> 12) & 3) + 1] = SBUS;
        break;
    case 10:    /* SB */
        R[((IR >> 8) & 3)] = SBUS;
        break;
    case 11:    /* SBP */
        R[((IR >> 8) & 3) + 1] = SBUS;
        break;
    case 12:    /* PCS */
        PC = SBUS;
        break;

    case 13:
    case 14:
        error(ErrUndefinedMicroInst);
        break;

    case 15:    /* NSB */
        /* nothing to be written */
        break;

    default:
        printf("something is wrong (SBF=%x)\n", SBF);
        break;
    }
}


/* MM READ OR WRITE */
void
exec_MMF()
{
    switch (MEM_CYCLE) {
    case MemWrite:
        if (MAR_address_is_OK(MAR)) {
            MM[MAR] = LBUS;
            MEM_CYCLE = MemNop;
        }
        else {
            error(ErrOutOfMMAddrLimit);
        }
        break;
    case MemRead:
        MEM_CYCLE = MemNop;
        break;
    case MemNop:
        break;
    default:
        printf(" ILLEGAL MEMORY CYCLE STATE\n");
        break;
    }

    switch (IO_CYCLE) {
    case IoRead:        /* INPUT ONE CHAR FROM CARD READER IMAGE FILE */
    {
        int c;
        if ((c = fgetc(INPUT)) == EOF) {
            error(ErrNoDataOnIoDevice);
        }
        IOBUS = c;
        IO_CYCLE = IoNop;
    }
    break;
    case IoWrite:       /* OUTPUT ONE CHAR TO LINE PRINTER */
        fputc(IOBUS, OUTPUT);
        IO_CYCLE = IoNop;
        break;
    case IoNop:
        break;
    default:
        printf(" ILLEGAL IO CYCLE STATE\n");
        break;
    }

    switch (MMF) {
    case 0:             /* RM */
        MAR = LBUS;
        MEM_CYCLE = MemRead;
        break;
    case 1:             /* WM */
        MAR = LBUS;
        MEM_CYCLE = MemWrite;
        break;
    case 2:
        break;
        error(ErrUndefinedMicroInst);
    case 3:             /* NMM */
        break;
    default:
        printf("something is wrong (MMF=%x)\n", MMF);
        break;
    }
}


/* EXECUTE I/O */
void
execute_IO()
{
    MInt IoCommand;     /* FOR I/O COMMAND ON IR */
    MInt IoDevice;      /* FOR I/O DEVICE NO. */

    IoCommand = (IR >> 8) & 0xff;
    IoDevice  = IR & 0xff;

    switch (IoCommand) {
    case 236:           /* 236:I/O READ , DEVICE(0):CARD_READER */
        if (IoDevice == 0) {
            if (INPUT == NULL) {
                INPUT = fopen("MSIM.CRD", "r");
                assert(INPUT != NULL);
            }
        }
        else {
            error (ErrUndefinedIoDevOnIR);
        }
        IO_CYCLE = IoRead;
        break;

    case 237:           /* 237:I/O WRITE , DEVICE(1):LINE_PRINTER */
        if (IoDevice == 1) {
            if (OUTPUT == NULL) {
                OUTPUT = fopen("MSIM.LPT", "w");
                assert(OUTPUT != NULL);
            }
        }
        else {
            error (ErrUndefinedIoDevOnIR);
        }
        IO_CYCLE = IoWrite;
        break;

    default:
        error(ErrUndefinedIoInstOnIR);
        break;
    }
}
    

/* EX FIELD OPERATION */
void
exec_EXF()
{
    switch (EXF) {
    case 0:             /* CM1 */
        C = (C - 1) & C_BITMASK;
        break;
    case 1:             /* FLS (FLAG SAVE) */
        FSR = 0;
        if (ZER) FSR |= 0x8;
        if (NEG) FSR |= 0x4;
        if (CRY) FSR |= 0x2;
        if (OV)  FSR |= 0x1;
        break;
    case 2:             /* ASC */
    case 3:             /* AS1 */
        /* SHIFTER */
        break;

    case 4:             /* LIR */
        break;
    case 5:             /* LIO */
        break;

    case 6:             /* SC  */
        C = RBUS & C_BITMASK;
        break;

    case 7:             /* EIO */
        execute_IO();
        break;

    case 8:             /* ST  */
        T = MTRUE;
        break;
    case 9:             /* RT  */
        T = MFALSE;
        break;

    case 10:            /* INA */
        IR = (IR & 0xf3ff) | ((((IR >> 10) + 1) & 3) << 10);
        break;
    case 11:            /* INB */
        IR = (IR & 0xfcff) | ((((IR >> 8) + 1) & 3) << 8);
        break;
    case 12:            /* DCB */
        IR = (IR & 0xfcff) | ((((IR >> 8) + 3) & 3) << 8);
        break;
    case 13:            /* HLT */
        HALT_ON = MTRUE;
        break;
    case 14:            /* OV  */
        HALT_ON = MTRUE;
        OV_LAMP = MTRUE;
        printf("\n OV LAMP ON !\n");
        break;
    case 15:            /* NEX */
        break;
        
    default:
        printf("something is wrong (EXF=%x)\n", EXF);
        break;
    }

    /* update CZ */
    CZ = (C == 0)? MTRUE: MFALSE;
}
    

/* EXECUTE MICRO INSTRUCTION */
void
execute()
{
    fetch_decode();

    exec_SQF();
    exec_LBF();
    exec_RBF();
    exec_MMF();
    exec_ALF();
    exec_SHF();
    exec_SBF();
    exec_EXF();
}


/* CHECK BREAK OR NOT */
void
check_breakpoint()
{
    int i;
    for (i = 0; i < MAX_BRKPOINT; i++) {
        if (BPoint[i].cond &&
            BPoint[i].addr == CMAR) {
            printf(" BREAK !! AT %03X\n", CMAR);
            HALT_ON = MTRUE;
        }
    }
}
    

static void
dump_string(char *s)
{
    printf("%s", s);
}

/* TRACE DEVICE NAME DUMP */
void
trace_dump_name()
{
    if (TraceDevice[DevCMAR])
        dump_string("CMAR ");
    if (TraceDevice[DevMAR])
        dump_string("MAR  ");
    if (TraceDevice[DevC])
        dump_string("C  ");
    if (TraceDevice[DevPC])
        dump_string(" PC  ");
    if (TraceDevice[DevIR])
        dump_string(" IR  ");
    if (TraceDevice[DevFLAG])
        dump_string("ZNCOCT ");
    if (TraceDevice[DevFSR])
        dump_string("FSR ");

    if (TraceDevice[DevREG]) {
        int i;
        for (i = 0; i < MAX_REGS; i++) {
            if (TraceRegister[i]) {
                char sbuf[BUFSIZ];
                sprintf(sbuf, " R%d  ", i);
                dump_string(sbuf);
            }
        }
    }

    if (TraceDevice[DevLBUS])
        dump_string("LBUS ");
    if (TraceDevice[DevRBUS])
        dump_string("RBUS ");
    if (TraceDevice[DevABUS])
        dump_string("ABUS ");
    if (TraceDevice[DevSBUS])
        dump_string("SBUS ");
    if (TraceDevice[DevIOBUS])
        dump_string("IOBUS ");

    dump_string("\n");
}



/* TRACE VALU DUMP */
void
trace_dump_value()
{
    char sbuf[BUFSIZ];
    if (TraceDevice[DevCMAR]) {
        sprintf(sbuf, "%03X  ", CMAR);
        dump_string(sbuf);
    }
    if (TraceDevice[DevMAR]) {
        sprintf(sbuf, "%04X ", MAR);
        dump_string(sbuf);
    }
    if (TraceDevice[DevC]) {
        sprintf(sbuf, "%02X ", C);
        dump_string(sbuf);
    }
    if (TraceDevice[DevPC]) {
        sprintf(sbuf, "%04X ", PC);
        dump_string(sbuf);
    }
    if (TraceDevice[DevIR]) {
        sprintf(sbuf, "%04X ", IR);
        dump_string(sbuf);
    }
    if (TraceDevice[DevFLAG]) {
        sprintf(sbuf, "%c%c%c%c%c%c ",
                ZER? '1': '0', NEG? '1': '0', CRY? '1': '0', OV? '1': '0',
                (C == 0)? '1': '0', T? '1': '0');
        dump_string(sbuf);
    }
    if (TraceDevice[DevFSR]) {
        sprintf(sbuf, " %1X  ", FSR);
        dump_string(sbuf);
    }

    if (TraceDevice[DevREG]) {
        int i;
        for (i = 0; i < MAX_REGS; i++) {
            if (TraceRegister[i]) {
                sprintf(sbuf, "%04X ", R[i]);
                dump_string(sbuf);
            }
        }
    }

    if (TraceDevice[DevLBUS]) {
        sprintf(sbuf, "%04X ", LBUS);
        dump_string(sbuf);
    }
    if (TraceDevice[DevRBUS]) {
        sprintf(sbuf, "%04X ", RBUS);
        dump_string(sbuf);
    }
    if (TraceDevice[DevABUS]) {
        sprintf(sbuf, "%04X ", ABUS);
        dump_string(sbuf);
    }
    if (TraceDevice[DevSBUS]) {
        sprintf(sbuf, "%04X ", SBUS);
        dump_string(sbuf);
    }
    if (TraceDevice[DevIOBUS]) {
        sprintf(sbuf, "%04X ", IOBUS);
        dump_string(sbuf);
    }

    dump_string("\n");
}


/* TRACE DUMP */
void
trace_dump()
{
    if (TRACE_ON) {
        int i;
        for (i = 0; i < MAX_TRCPOINT; i++) {
            if (TPoint[i].cond && TPoint[i].addr == CMAR) {
                trace_dump_name();
                trace_dump_value();
                break;
            }
        }
    }
    else {
        trace_dump_name();
        trace_dump_value();
    }
}


/* DEVICE INPUT ROUTIN FOR TRACE OR CHANGE COMMAND */

DevId
read_Device(MInt *RegNo)
{
    DevId dev_id;
    char devname[BUFSIZ];

    assert(RegNo != NULL);
    *RegNo = -1;

    printf("  DEVICE ?");
    read_line(devname, BUFSIZ, stdin);
    if (devname[0] == '\n' || devname[0] == '\0' ||
        devname[0] == '.') {
        dev_id = DevENDMARK;
    }
    else if (toupper(devname[0]) == 'R' && isdigit(devname[1])) {
        int regno;
        regno = atoi(devname+1);
        if (regno >= 0 && regno < MAX_REGS) {
            dev_id = DevREG;
            *RegNo = regno;
        }
        else {
            error(ErrIllegalCommandOrData);
            dev_id = DevERR;
        }
    }
    else {
        int i;
        for (i = 0; i < DevNameTableSize; i++) {
            if (!strcasecmp(devname, DevNameTable[i].name)) {
                dev_id = DevNameTable[i].id;
                break;
            }
        }
        if (i == DevNameTableSize) {    /* not found in table */
            error(ErrIllegalCommandOrData);
            dev_id = DevERR;
        }
    }

    return dev_id;
}


/* TRACE DEVICE SET */
void
trace_set(MBool OnOff)
{
    DevId dev_id;
    MInt reg_no;
    int i;

    printf(" ");
    while ((dev_id = read_Device(&reg_no)) != DevENDMARK) {
        switch (dev_id) {
        case DevREG:
            TraceRegister[reg_no] = OnOff;
            /* PASS THROUGH */
        case DevPC:
        case DevIR:
        case DevLBUS:
        case DevRBUS:
        case DevABUS:
        case DevSBUS:
        case DevIOBUS:
        case DevCM:
        case DevMM:
        case DevMAR:
        case DevCMAR:
        case DevC:
        case DevFLAG:
        case DevFSR:
            TraceDevice[dev_id] = OnOff;
            break;

        case DevRSTAR:
            TraceDevice[DevREG]  = OnOff;
            for (i = 0; i < MAX_REGS; i++)
                TraceRegister[i] = OnOff;
            break;

        case DevBSTAR:
            TraceDevice[DevLBUS] = OnOff;
            TraceDevice[DevRBUS] = OnOff;
            TraceDevice[DevSBUS] = OnOff;
            TraceDevice[DevABUS] = OnOff;
            TraceDevice[DevIOBUS] = OnOff;
            break;

        case DevSTAR:
            TraceDevice[DevREG]  = OnOff;
            for (i = 0; i < MAX_REGS; i++)
                TraceRegister[i] = OnOff;
            TraceDevice[DevPC]   = OnOff;
            TraceDevice[DevIR]   = OnOff;
            TraceDevice[DevMAR]  = OnOff;
            TraceDevice[DevCMAR] = OnOff;
            TraceDevice[DevC]    = OnOff;
            TraceDevice[DevFLAG] = OnOff;
            TraceDevice[DevFSR]  = OnOff;
            break;

        case DevERR:
            break;
        default:
            break;
        }

        printf(" ");
    }

}


/* TRACE DEVICE CHANGE */
void
trace_change()
{
    char sbuf[BUFSIZ];

    printf("   TD?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL) {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'S':
            trace_set(MTRUE);
            break;
        case 'R':
            trace_set(MFALSE);
            break;
        case 'D':
            trace_dump_name();
            break;
        default:
            printf("Please key in S)et, R)eset, D)isplay or \".\"(end)\n");
            break;
        }

        printf("   TD?");
    }

    printf("\n");
}


/* -------------------------------------------------------------------TRACE-- */

/* TRACE RUN  COMMAND */
void
trace_run()
{
    MBool MAXUSE;
    char sbuf[BUFSIZ];

    printf(" CMAR=?");
    read_line(sbuf, BUFSIZ, stdin);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.')
        return;

    if (strcasecmp(sbuf, "CNT"))
        CMAR = strtol(sbuf, NULL, 16);

    if (!CMAR_address_is_OK(CMAR)) {
        error(ErrOutOfCMAddrLimit);
        CMAR = 0;
    }

    MAXSTEP = -1;
    printf(" MAX STEP=?");
    read_line(sbuf, BUFSIZ, stdin);
    if (sbuf[0] == '\n' || sbuf[0] == '\0') {   /* no max steps specified */
        MAXUSE = MTRUE;
        printf("   MAX STEP IS NOT USED\n");
    }
    else if (sbuf[0] == '.') {
        return;
    }
    else {
        MAXSTEP = atoi(sbuf);
        MAXUSE = MFALSE;
    }

    HALT_ON = MFALSE;
    OV_LAMP = MFALSE;

    STEP = 0;
    while (!HALT_ON && (STEP < MAXSTEP || MAXUSE)) {    /* EXECUTION MAIN LOOP */
        execute();
        trace_dump();
        if (BREAK_ON)
            check_breakpoint();

        STEP++;
    }

    printf(" CPU HALTED !\n");
    if (STEP == MAXSTEP)
        printf(" MAX STEPS EXAUSTED\n");
    else
        printf(" % 3d STEPS USED\n", STEP);
}


/* -------------------------------------------------------------------RUN-- */

/* NORMAL RUN   COMMAND */
void
run()
{
    MBool MAXUSE;
    char sbuf[BUFSIZ];

    printf("RUN\n");

    printf(" CMAR=?");
    read_line(sbuf, BUFSIZ, stdin);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.')
        return;

    if (strcasecmp(sbuf, "CNT"))
        CMAR = strtol(sbuf, NULL, 16);

    if (!CMAR_address_is_OK(CMAR)) {
        error(ErrOutOfCMAddrLimit);
        CMAR = 0;
    }

    MAXSTEP = -1;
    printf(" MAX STEP=?");
    read_line(sbuf, BUFSIZ, stdin);
    if (sbuf[0] == '\n' || sbuf[0] == '\0') {   /* no max steps specified */
        MAXUSE = MTRUE;
        printf("   MAX STEP IS NOT USED\n");
    }
    else if (sbuf[0] == '.') {
        return;
    }
    else {
        MAXSTEP = atoi(sbuf);
        MAXUSE = MFALSE;
    }

    HALT_ON = MFALSE;
    OV_LAMP = MFALSE;

    STEP = 0;
    printf(" Running\n");
    while (!HALT_ON && (STEP < MAXSTEP || MAXUSE)) {    /* EXECUTION MAIN LOOP */
        if (BREAK_ON)
            check_breakpoint();
        execute();
//        trace_dump();
        if (BREAK_ON)
            check_breakpoint();

        STEP++;
    }

    printf(" CPU HALTED !\n");
    if (STEP == MAXSTEP)
        printf(" MAX STEPS EXAUSTED\n");
    else
        printf(" % 3d STEPS USED\n", STEP);
}


/* -------------------------------------------------------------------LOAD---- */

/* LOAD COMMAND */
void
load()
{
    FILE *fp;
    char sbuf[BUFSIZ];

    printf("LOAD\n");

    printf("  FILE NAME=?");
    read_line(sbuf, BUFSIZ, stdin);
    printf("  LOADING '%s'\n", sbuf);

    if ((fp = fopen(sbuf, "r")) == NULL) {     /* fail to open file */
        error(ErrIoError);
        return;
    }

    if (fgets(sbuf, BUFSIZ, fp) != NULL) {
        if (sbuf[0] == 'C' && sbuf[1] == 'M') {             /* load to CM */
            while (fgets(sbuf, BUFSIZ, fp) != NULL) {
                MAddr addr = strtoul(sbuf, NULL, 16);
                MCtrlWord cw = strtoll(sbuf+5, NULL, 16);
                if (CMAR_address_is_OK(addr)) {
                    CM[addr] = cw;
                }
                else {
                    error(ErrOutOfCMAddrLimit);
                }
#ifdef  DEBUG
                printf("%03X %010llX\n", addr, cw);
#endif
            }
            printf(" LOADED TO CM\n");
        }
        else if (sbuf[0] == 'M' && sbuf[1] == 'M') {        /* load to MM */
            while (fgets(sbuf, BUFSIZ, fp) != NULL) {
                MAddr addr = strtoul(sbuf, NULL, 16);
                MWord word = strtoul(sbuf+6, NULL, 16) & 0xffff;
                if (MAR_address_is_OK(addr)) {
                    MM[addr] = word;
                }
                else {
                    error(ErrOutOfMMAddrLimit);
                }
#ifdef  DEBUG
                printf("%04X %04X\n", addr, word);
#endif
            }
            printf(" LOADED TO MM\n");
        }
        else {
            error(ErrInvalidLoadFileFormat);
        }
    }

    fclose(fp);
}


/* ------------------------------------------------------------------DUMP----- */

static char *
to_bitstring(MWord v, MInt nbits)
{
    static char sbuf[BUFSIZ];
    int i;

    assert(nbits < BUFSIZ-1);
    for (i = 0; i < nbits; i++) {
        sbuf[i] = ((v >> (nbits - i -1)) & 1)? '1': '0';
    }
    sbuf[nbits] = '\0';

    return sbuf;
}

/* REGISTER GROUP DUMP */
void
dumpREG()
{
    int i;
    char sbuf[BUFSIZ];

    dump_string("CMAR MAR  C   PC   IR  ZNCOCT FSR ");
    for (i = 0; i < MAX_REGS; i++) {
        sprintf(sbuf, " R%d  ", i);
        dump_string(sbuf);
    }
    dump_string("\n");

    sprintf(sbuf, "%03X  %04X %02X %04X %04X %s  %01X  ",
            CMAR, MAR, C, PC, IR, to_bitstring(FSR, 6), FSR);
    dump_string(sbuf);
    for (i = 0; i < MAX_REGS; i++) {
        sprintf(sbuf, "%04X ", R[i]);
        dump_string(sbuf);
    }
    dump_string("\n");
}


/* BUS GROUP DUMP */
void
dumpBUS()
{
    char sbuf[BUFSIZ];
    
    dump_string("LBUS RBUS ABUS SBUS IOBUS\n");
    sprintf(sbuf, "%04X %04X %04X %04X %04X\n",
            LBUS, RBUS, ABUS, SBUS, IOBUS);
    dump_string(sbuf);
}


/* read word value */
static MWord
read_word(FILE *fp, MBool *err_flag)
{
    char sbuf[BUFSIZ];

    if (err_flag)
        *err_flag = MFALSE;

    read_line(sbuf, BUFSIZ, fp);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.') {
        if (err_flag)
            *err_flag = MTRUE;
    }

    return strtol(sbuf, NULL, 16) & 0xffff;
}
    
/* read long long word value */
static MLLong
read_long_long_word(FILE *fp, MBool *err_flag)
{
    char sbuf[BUFSIZ];

    if (err_flag)
        *err_flag = MFALSE;

    read_line(sbuf, BUFSIZ, fp);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.') {
        if (err_flag)
            *err_flag = MTRUE;
    }

    return strtoll(sbuf, NULL, 16);
}

/* read boolean */
static MBool
read_bit(FILE *fp, MBool *err_flag)
{
    char sbuf[BUFSIZ];

    if (err_flag)
        *err_flag = MFALSE;

    read_line(sbuf, BUFSIZ, fp);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.') {
        if (err_flag)
            *err_flag = MTRUE;
    }

    if (sbuf[0] != '1' && sbuf[1] != '0' &&
        sbuf[0] != 'T' && sbuf[1] != 'F') {
        if (err_flag)
            *err_flag = MTRUE;

        error(ErrIllegalCommandOrData);
    }

    return sbuf[0] == '0' || sbuf[0] == 'F'? MFALSE: MTRUE;
}

/* read CM address */
static MAddr
read_CM_address(FILE *fp, MBool *err_flag)
{
    char sbuf[BUFSIZ];
    MAddr addr;

    if (err_flag)
        *err_flag = MFALSE;

    read_line(sbuf, BUFSIZ, fp);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.') {
        if (err_flag)
            *err_flag = MTRUE;
    }

    addr = strtoul(sbuf, NULL, 16);

    if (!CMAR_address_is_OK(addr)) {
        error(ErrOutOfCMAddrLimit);
        if (err_flag)
            *err_flag = MTRUE;
    }

    return addr;
}

/* read MM address */
static MAddr
read_MM_address(FILE *fp, MBool *err_flag)
{
    char sbuf[BUFSIZ];
    MAddr addr;

    if (err_flag)
        *err_flag = MFALSE;

    read_line(sbuf, BUFSIZ, fp);
    if (sbuf[0] == '\n' || sbuf[0] == '\0' || sbuf[0] == '.') {
        if (err_flag)
            *err_flag = MTRUE;
    }

    addr = strtoul(sbuf, NULL, 16);

    if (!MAR_address_is_OK(addr)) {
        error(ErrOutOfMMAddrLimit);
        if (err_flag)
            *err_flag = MTRUE;
    }

    return addr;
}


/* CM DUMP */
void
dumpCM()
{
    MAddr start_addr;
    MAddr end_addr;
    MAddr addr;
    char sbuf[BUFSIZ];

    printf("    START ADRS ?");
    start_addr = read_CM_address(stdin, NULL);

    printf("    END   ADRS ?");
    end_addr = read_CM_address(stdin, NULL);

    for (addr = start_addr; addr <= end_addr; addr++) {
        MCtrlWord cw = CM[addr];
        sprintf(sbuf, "%03X:%010llX\n", addr, cw);
        dump_string(sbuf);
    }
}


/* MM DUMP */
void
dumpMM()
{
    MAddr start_addr;
    MAddr end_addr;
    MAddr addr;
    char sbuf[BUFSIZ];

    printf("    START ADRS ?");
    start_addr = read_MM_address(stdin, NULL);

    printf("    END   ADRS ?");
    end_addr = read_MM_address(stdin, NULL);

    for (addr = start_addr; addr <= end_addr; addr++) {
        sprintf(sbuf, "%04X:%04X\n", addr, MM[addr]);
        dump_string(sbuf);
    }
}


/* DUMP COMMAND */
void
dump()
{
    char sbuf[BUFSIZ];

    printf("DUMP\n");

    printf("  D?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL) {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'R': dumpREG(); break;
        case 'B': dumpBUS(); break;
        case 'C': dumpCM(); break;
        case 'M': dumpMM(); break;
        default:
            printf(" Please key in R)eg, B)us, C)m, M)m or \".\"(end)\n");
            break;
        }

        printf("  D?");
    }
}


/* -------------------------------------------------------OUTPUT FILE CHANGE---- */

/* OUTPUT FILE CHANGE COMMAND */
void
change_file()
{
    printf("FILE\n");

    /* THIS COMMAND IS NOT SUPPORTED */
    printf("THIS COMMAND IS NOT SUPPORTED\n");
    assert(0);
}


/* ---------------------------------------------------------------------SAVE--- */

/* SAVE COMMAND */
void
save()
{
    char sbuf[BUFSIZ];

    printf("SAVE\n");

    printf("   CM OR MM?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] == '.') {
        FILE *fp;
        MAddr start_addr;
        MAddr end_addr;
        MAddr addr;
        MChar CMorMM;           /* 'C' or 'M' */
        char fname[BUFSIZ];

        CMorMM = sbuf[0];
        if (CMorMM != 'C' && CMorMM != 'M') {
            printf("PLEASE SELECT CM OR MM\n");
            continue;
        }

        printf("%cM\n", CMorMM);

        printf("   FILE NAME=?");
        read_line(fname, BUFSIZ, stdin);

        if ((fp = fopen(fname, "w")) == NULL) {
            error(ErrIoError);
            printf("   CM OR MM?");
            continue;
        }

        if (CMorMM == 'M') {            /* SAVE MM */
            printf("   START ADRS ?");
            start_addr = read_MM_address(stdin, NULL);
            printf("   END   ADRS ?");
            end_addr = read_MM_address(stdin, NULL);

            printf("    SAVE FILE=%s  START=%04X END=%04X\n",
                   fname, start_addr, end_addr);

            fprintf(fp, "MM\n");
            for (addr = start_addr; addr <= end_addr; addr++) {
                fprintf(fp, "%04X  %04X\n", addr, MM[addr]);
            }
        }
        else {                          /* SAVE CM */
            MCtrlWord cw;

            printf("   START ADRS ?");
            start_addr = read_CM_address(stdin, NULL);
            printf("   END   ADRS ?");
            end_addr = read_CM_address(stdin, NULL);

            printf("    SAVE FILE=%s  START=%03X END=%03X\n",
                   fname, start_addr, end_addr);

            fprintf(fp, "CM\n");
            for (addr = start_addr; addr <= end_addr; addr++) {
                cw = CM[addr];
                fprintf(fp, "%03X  %010llX\n", addr, cw);
            }
        }

        fclose(fp);

        printf("   CM OR MM?");
    }
}


/* ----------------------------------------------------------BREAK POINT----- */

static MBool
set_address(MAddr addr, AddrMark table[], int table_size)
{
    int i;
    for (i = 0; i < table_size; i++) {
        if (table[i].cond && table[i].addr == addr) {
            /* already recorded */
            return MTRUE;
        }
    }
    for (i = 0; i < table_size; i++) {
        if (!table[i].cond) {
            table[i].cond = MTRUE;
            table[i].addr = addr;
            return MTRUE;
        }
    }

    return MFALSE;
}

static MBool
reset_address(MAddr addr, AddrMark table[], int table_size)
{
    int i;
    for (i = 0; i < table_size; i++) {
        if (table[i].cond && table[i].addr == addr) {
            table[i].cond = MFALSE;
        }
    }
    return MTRUE;
}

/* BREAK_POINT COMMAND */
void
break_point()
{
    MAddr addr;
    int i;
    char sbuf[BUFSIZ];

    printf("BREAK POINT\n");

    printf("  B?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'S':               /* BREAK POINT SET */
            printf("   ADRS ?");
            while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
                addr = strtol(sbuf, NULL, 16);
                if (!CMAR_address_is_OK(addr)) {
                    error(ErrOutOfCMAddrLimit);
                }
                if (!set_address(addr, BPoint, MAX_BRKPOINT)) {
                    error(ErrBPOverflow);
                }
                printf("   ADRS ?");
            }
            break;

        case 'R':               /* BREAK POINT RESET */
            printf("   ADRS ?");
            while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
                addr = strtol(sbuf, NULL, 16);
                if (!CMAR_address_is_OK(addr)) {
                    error(ErrOutOfCMAddrLimit);
                }
                reset_address(addr, BPoint, MAX_BRKPOINT);
                printf("   ADRS ?");
            }
            break;

        case 'D':               /* BREAK POINT DUMP */
            for (i = 0; i < MAX_BRKPOINT; i++) {
                if (BPoint[i].cond) {
                    printf("     %04X\n", BPoint[i].addr);
                }
            }
            break;

        default:
            printf("Please key in S)et, R)eset, D)isplay or \".\"(end)\n");
            break;
        }
        
        printf("  B?");
    }

    for (i = 0; i < MAX_BRKPOINT; i++)
        if (BPoint[i].cond)
            break;
    BREAK_ON = (i == MAX_BRKPOINT)? MFALSE: MTRUE;
}


/* ---------------------------------------------------------------TRACE_POINT-- */

/* TRACE_POINT COMMAND */
void
trace_point()
{
    MAddr addr;
    int i;
    char sbuf[BUFSIZ];

    printf("   TA?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'S':               /* TRACE POINT SET */
            printf("    ADRS ?");
            while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
                addr = strtol(sbuf, NULL, 16);
                if (!CMAR_address_is_OK(addr)) {
                    error(ErrOutOfCMAddrLimit);
                }
                if (!set_address(addr, TPoint, MAX_TRCPOINT)) {
                    error(ErrTPOverflow);
                }
                printf("    ADRS ?");
            }
            break;

        case 'R':               /* TRACE POINT RESET */
            printf("    ADRS ?");
            while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
                addr = strtol(sbuf, NULL, 16);
                if (!CMAR_address_is_OK(addr)) {
                    error(ErrOutOfCMAddrLimit);
                }
                reset_address(addr, TPoint, MAX_TRCPOINT);
                printf("    ADRS ?");
            }
            break;

        case 'D':               /* TRACE POINT DUMP */
            for (i = 0; i < MAX_TRCPOINT; i++) {
                if (TPoint[i].cond) {
                    printf("     %04X\n", TPoint[i].addr);
                }
            }
            break;

        default:
            printf("Please key in S)et, R)eset, D)isplay or \".\"(end)\n");
            break;
        }
        
        printf("   TA?");
    }

    for (i = 0; i < MAX_TRCPOINT; i++)
        if (TPoint[i].cond)
            break;
    TRACE_ON = (i == MAX_TRCPOINT)? MFALSE: MTRUE;
}

/* TRACE COMMAND */
void
trace()
{
    char sbuf[BUFSIZ];

    printf("TRACE\n");

    printf("  T?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'A': trace_point(); break;
        case 'G': trace_run(); break;
        case 'D': trace_change(); break;
        case '.': break;
        default:
            printf("Please key in A)ddress, G)o, D)evice or \".\"(end)\n");
            break;
        }

        printf("  T?");
    }
}


/* --------------------------------------------------------------------CHANGE-- */

static void
change_word(MWord *w)
{
    char sbuf[BUFSIZ];
    printf("    %04X->", *w);
    read_line(sbuf, BUFSIZ, stdin);
    *w = strtol(sbuf, NULL, 16) & 0xffff;
}

/* CHANGE COMMAND */
void
change()
{
    DevId dev_id;
    MInt reg_no;

    printf("CHANGE\n");

    while ((dev_id = read_Device(&reg_no)) != DevENDMARK) {
        switch (dev_id) {
        case DevREG:
            printf("    R[%d]:%04X->", reg_no, R[reg_no]);
            R[reg_no] = read_word(stdin, NULL);
            break;
        case  DevPC:
            printf("    PC:%04X->", PC);
            PC = read_word(stdin, NULL);
            break;
        case DevIR:
            printf("    IR:%04X->", IR);
            IR = read_word(stdin, NULL);
            break;
        case DevLBUS:
            printf("    LBUS:%04X->", LBUS);
            LBUS = read_word(stdin, NULL);
            break;
        case DevRBUS:
            printf("    RBUS:%04X->", RBUS);
            RBUS = read_word(stdin, NULL);
            break;
        case DevABUS:
            printf("    ABUS:%04X->", ABUS);
            ABUS = read_word(stdin, NULL);
            break;
        case DevSBUS:
            printf("    SBUS:%04X->", SBUS);
            SBUS = read_word(stdin, NULL);
            break;
        case DevIOBUS:
            printf("    IOBUS:%04X->", IOBUS);
            IOBUS = read_word(stdin, NULL);
            break;

        case DevCMAR:
            printf("    CMAR:%03X->", CMAR);
            CMAR = read_CM_address(stdin, NULL);
            break;
        case DevMAR:
            printf("    MAR:%04X->", MAR);
            MAR = read_MM_address(stdin, NULL);
            break;

        case DevCM: {
            MAddr addr;
            MBool done = MFALSE;

            printf("    FROM ADRS ?");
            addr = read_CM_address(stdin, NULL);
            while (!done && CMAR_address_is_OK(addr)) {
                MCtrlWord cw;
                printf("     %03X:%010llX->", addr, CM[addr]);
                cw = read_long_long_word(stdin, &done);
                CM[addr] = cw & 0xffffffffffLL;
                addr++;
            }
        }
            break;

        case DevMM: {
            MAddr addr;
            MBool done = MFALSE;

            printf("    FROM ADRS ?");
            addr = read_MM_address(stdin, NULL);
            while (!done && MAR_address_is_OK(addr)) {
                printf("     %04X:%04X->", addr, MM[addr]);
                MM[addr] = read_word(stdin, &done);
                addr++;
            }
        }
            break;

        case DevC:
            printf("    C:%02X->", C);
            C = read_word(stdin, NULL) & 0xff;
            break;

        case DevFSR:
            printf("    FSR:%1X->", FSR & 0xf);
            FSR = read_word(stdin, NULL) & 0xf;
            break;

        case DevFLAG: {
            printf("    PLEASE KEY IN (0 OR 1)");
            printf("    ZER ?"); ZER = read_bit(stdin, NULL);
            printf("    NEG ?"); NEG = read_bit(stdin, NULL);
            printf("    CRY ?"); CRY = read_bit(stdin, NULL);
            printf("    OVR ?"); OV  = read_bit(stdin, NULL);
            printf("    CZ  ?"); CZ  = read_bit(stdin, NULL);
            printf("    T   ?"); T   = read_bit(stdin, NULL);
        }
            break;

            /* meta devices (not real device) */
        case DevSTAR:
        case DevRSTAR:
        case DevBSTAR:
        case DevENDMARK:
        case DevERR:
        case DevNULL:
        case DevMAX:
        default:
            break;
        }
    }

}


/* ---------------------------------------------------------INITIALIZE---- */

/* SYSTEM INITIALIZE */
void
init_REG()
{
    int i;

    PC = 0; IR = 0; CMDR = 0;
    LBUS = RBUS = SBUS = ABUS = IOBUS = 0;
    MAR = 0; CMAR = 0; C = 0;
    CRY = NEG = OV = ZER = CZ = T = MFALSE;
    for (i = 0; i < MAX_REGS; i++)
        R[i] = 0;

    BREAK_ON  = MFALSE;
    TRACE_ON  = MFALSE;
    MEM_CYCLE = MemNop;
    IO_CYCLE  = IoNop;

    CMAR_STACK_POS = 0;

    for (i = 0; i < MAX_BRKPOINT; i++)
        BPoint[i].cond = MFALSE;

    for (i = 0; i < MAX_TRCPOINT; i++)
        TPoint[i].cond = MFALSE;

    for (i = 0; i < DevMAX; i++)
        TraceDevice[i] = MFALSE;
    for (i = 0; i < MAX_REGS; i++)
        TraceRegister[i] = MFALSE;
    TraceDevice[DevCMAR] = MTRUE;

    TRACE_ON = MFALSE;

    FSR = 0;
}

void
init_MM()
{
    MAddr addr;
    for (addr = 0; addr < MAX_MM; addr++)
        MM[addr] = 0;
}

void
init_CM()
{
    MAddr addr;
    for (addr = 0; addr < MAX_CM; addr++)
        CM[addr] = 0;
}


void
initialize()
{
    char sbuf[BUFSIZ];

    printf("INITIALIZE\n");

    printf("  I?");
    while (read_line(sbuf, BUFSIZ, stdin) != NULL && sbuf[0] != '.') {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'C': init_CM(); break;
        case 'M': init_MM(); break;
        case 'A': init_REG(); init_CM(); init_MM(); break;
        case 'R': init_REG(); break;
        default:
            printf("Please key in R)eg,C)m,M)m,A)ll or \".\"(end)\n");
            break;
        }

        printf("  I?");
    }
    printf("\n");
}


/* menu command */
void
menu()
{
    /* clear_screen(); */
    printf("MENU\n");

    printf("\n"
           "  COMMAND MENU FOR THE MICRO-1 SIMULATOR\n"
           "\n"
           "COMMAND             FUNCTION                        SUB COMMAND\n"
           "\n"
           " B)reak      set or reset some break-points      SET,RESET,DISPLAY,..\n"
           " C)hange     change memory or register valu      CM,MM,CMAR,R0,..\n"
           " D)ump       dump memory or register valu        REG,BUS,CM,MM,..\n"
           " E)nd or Q)uit or \".\"    end/quit simulator\n"
           " I)nitialize initialize system                   REG,CM,MM,ALL,..\n"
           " L)oad       load program or data to MM or CM    \n"
           " M)enu       command menu\n"
           " R)un        simulator normal run\n"
           " F)ile       set output file for dump or trace\n"
           " S)ave       save MM or CM                       CM,MM,..\n"
           " T)race      simulator trace run                 SET,RESET,DISPLAY,..\n"
           "\n");
}

void
command_list()
{
    printf(" Please key in\n"
           "  B)reak,C)hange,D)ump,E)nd,F)ileset,I)nitialize,L)oad,"
           "M)enu,R)un,S)ave,T)race.\n");
}


int
main()
{
    MBool IsRunning = MTRUE;
    char sbuf[BUFSIZ];

    printf("\n"
           "   *** MICRO-1 SIMULATOR (C-Ver. 1.0) 2016 ***\n"
           "\n");

    init_REG(); init_CM(); init_MM();

    printf("%s", PROMPT);
    while (IsRunning && read_line(sbuf, BUFSIZ, stdin) != NULL) {
        int c;
        c = toupper(sbuf[0]);
        if (c == '.') break;
        switch (c) {
        case 'E':
        case 'Q':
        case '.':
            IsRunning = MFALSE;
            break;
        case 'S': save(); break;
        case 'R': run(); break;
        case 'T': trace(); break;
        case 'D': dump(); break;
        case 'F': change_file(); break;
        case 'M': menu(); break;
        case 'B': break_point(); break;
        case 'C': change(); break;
        case 'I': initialize(); break;
        case 'L': load(); break;
        default: command_list(); break;
        }

        printf("%s", PROMPT);
    }

    printf("  MICRO-1 TERMINATED \n");

    return 0;
}

/* end of msim.c */
