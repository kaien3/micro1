/*
 *
 *      MICRO-1 assembler (ANSI C-version)
 *      translated from Pascal-version by kim   Ver. 1.0  2016.4
 *
 *      based on MICRO-1 ASSEMBLER (Ver. 1.2)
 *      PC-9801 Turbo-Pascal version
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

/* max number of bytes for filename */
#ifndef MAX_PATH
#define MAX_PATH        256
#endif

//#define MAXFNAMESIZE    14              /* max size of file name */
#define MAXFNAMESIZE    (MAX_PATH+1)    /* max size of file name */
#define MAXLINESIZE     80              /* max size of line */
#define MAXSYMSIZE      8               /* max size of symbol */
#define HEXDIGITS       4

#define TABLE_SIZE      100             /* size of lable table */

typedef char symbol_str[MAXSYMSIZE+1];  /* type of symbol */
typedef char name_str[MAXFNAMESIZE+1];  /* type of file name */
typedef char line_str[MAXLINESIZE+1];   /* type of line */
typedef char hex_str[HEXDIGITS+1];      /* type of hex number string */

typedef struct  table {                 /* type of label table */
    symbol_str  name;           /* label name */
    int         adrs;           /* label address */
} table;

typedef enum operation {
    add = 0,sub,and_op,or_op,xor_op,mult,div_op,cmp,ex,
    lc,push,pop,
    sl,sa,sc,bix,
    lea,lx,stx,
    l,st,la,
    bdis,bp,bz,bm,bc,bnp,bnz,bnm,bnc,b,bi,bsr,
    rio,wio,
    ret,nop,hlt,
    dc,ds,org,
    title,end_op,
    err,
} operation;    /* operation codes */

const symbol_str code_table[(int)end_op+1] =
{
    "ADD","SUB","AND","OR","XOR","MULT","DIV","CMP","EX",
    "LC","PUSH","POP",
    "SL","SA","SC","BIX",
    "LEA","LX","STX",
    "L","ST","LA",
    "BDIS","BP","BZ","BM","BC","BNP","BNZ","BNM","BNC","B","BI","BSR",
    "RIO","WIO",
    "RET","NOP","HLT",
    "DC","DS","ORG",
    "TITLE","END",
};      /* operation name */

bool    file_ok;                /* exist or not */
char    ch;                     /* one charactor from input */
bool    lower_case;             /* charactor lower or not */
symbol_str      symbol;         /* symbol from source file */
int     number;                 /* number */
hex_str hex_number;             /* hex number */
int     address;                /* address */
operation op_code;              /* operation code */
int     line_no;                /* line number */
int     error;                  /* number of error */
/*
     d_error : err_table;                  { duplicate error table }
     d_e_ptr : table_size;                 { duplicate error table pointer }
*/
int d_error[TABLE_SIZE];        /* error table */
int d_e_ptr;                    /* error table pointer */

table label_table[TABLE_SIZE];  /* label table */
int table_ptr;                  /* label table pointer (actually index) */
line_str prn_line;              /* print one line */

name_str sname;                 /* source file name */
FILE *source;                   /* source file */
name_str pname;                 /* print file name */
FILE *print;                    /* print file */
name_str oname;                 /* object file name */
FILE *objectfile;               /* object file */

/* open input and output files */
void open_files()
{
    char *cp;

    do {
        printf("\n");
        printf(" SOURCE FILE NAME ? ");

//        gets(sname);    /* DO NOT use gets() in modern C program */
        fgets(sname, sizeof sname, stdin);
        cp = strchr(sname, '\n');
        if (cp != NULL) *cp = '\0';             /* remove newline */

        while (sname[0] == ' ')
            strncpy(&sname[0], &sname[0]+1, MAXFNAMESIZE-1);
    } while (strlen(sname) <= 0);

    if ((cp = strchr(sname, ' ')) != NULL)
        *cp = '\0';
    
    if ((cp = strrchr(sname, '.')) == NULL) {
        strncpy(pname, sname, MAXFNAMESIZE);
        strncpy(oname, sname, MAXFNAMESIZE);

        strncat(pname, ".a", MAXFNAMESIZE);
        strncat(oname, ".b", MAXFNAMESIZE);
    }
    else {
        size_t pos = sname - cp;
        if (pos >= MAXFNAMESIZE-2) pos = MAXFNAMESIZE-2;
        strncpy(pname, sname, pos);
        strncpy(oname, sname, pos);

        strncat(pname, ".a", MAXFNAMESIZE);
        strncat(oname, ".b", MAXFNAMESIZE);
    }

    if ((source = fopen(sname, "r")) == NULL) {
        printf("fail to open file '%s'\n", sname);
        exit(1);
    }

    file_ok = true;
}

/* close input and output files */
void close_files()
{
    fclose(source);
    fclose(print);
    fclose(objectfile);
}

/* change dec(value) to hex(string) */
/*hex_str dec_hex(int dec)*/
char *dec_hex(int dec)
{
    int i;
    static char hex[HEXDIGITS+1];

    hex[HEXDIGITS] = '\0';
    for (i = 3; i >= 0; i--) {
        int d;
        d = dec & 0xf;
        if (d > 9) d += 7;
        hex[i] = '0' + d;
        dec >>= 4;
    }

    if (dec != 0 && dec != -1) {
        printf("warn: integer value is out of range of 16-bit %x\n", dec);
    }

    return hex;
}

/* space */
/*line_str space(int n)*/
char *space(size_t n)
{
    static char s[MAXLINESIZE+1];
    assert(n <= MAXLINESIZE);

    if (n < 0) n = 0;
    memset(&s[0], ' ', n);
    s[n] = '\0';
    return s;
}

/* read one charactor from source file */
void read_ch()
{
    if (!feof(source)) {
        ch = fgetc(source);
        if (ch == '\n')
            line_no++;
    }
    else
        ch = '\0';

    if (!lower_case)
        ch = toupper(ch);
}

/* skip spaces */
void skip_space()
{
//    while (isspace(ch))    /* DO NOT USE isspace() */
    while (ch == ' ' || ch == '\t')
        read_ch();
}

void skip_ch(char sc[], bool lc)
{
    size_t len = strlen(sc);

    if (lc)
        lower_case = true;

    while (ch != '\0') {
        int i;
        bool is_matched = false;

        for (i = 0; i < len; i++) {
            if (ch == sc[i]) {
                is_matched = true;
                break;
            }
        }
        if (!is_matched) {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len] = ch;
                prn_line[len+1] = '\0';
            }

            read_ch();
        }
        else
            break;
    }

    lower_case = false;
}

/* read one symbol from source file */
void read_symbol()
{
    int i = 0;
    while (isalnum(ch)) {
        if (i < MAXSYMSIZE) {
            symbol[i++] = ch;
        }
        read_ch();
    }
    symbol[i] = '\0';
}

/* get hex number from source file */
void get_hex_number(bool p)
{
    number = 0;
    while (isxdigit(ch)) {
        if (p) {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len] = ch;
                prn_line[len+1] = '\0';
            }
        }

        number = number * 16 + (ch - '0');
        if (isalpha(ch))
            number -= 7;

        read_ch();
    }
}
        
/* get octal number from source file */
void get_oct_number()
{
    number = 0;
    while (isdigit(ch) && !(ch == '8' || ch == '9')) {
        size_t len = strlen(prn_line);
        if (len <= MAXLINESIZE-1) {
            prn_line[len] = ch;
            prn_line[len+1] = '\0';
        }

        number = number * 8 + ch - '0';

        read_ch();
    }
}

/* get binary number from source file */
void get_bin_number()
{
    number = 0;
    while (ch == '0' || ch == '1') {
        size_t len = strlen(prn_line);
        if (len <= MAXLINESIZE-1) {
            prn_line[len] = ch;
            prn_line[len+1] = '\0';
        }

        number = number * 2 + ch - '0';

        read_ch();
    }
}

/* get dec number from source file */
void get_number(bool p)
{
    number = 0;
    while (isdigit(ch)) {
        if (p) {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len] = ch;
                prn_line[len+1] = '\0';
            }
        }

        number = number * 10 + (ch - '0');

        read_ch();
    }

}

void get_unsigned_number(bool *err)
{
    *err = false;
    switch (ch) {
    case '0':    case '1':    case '2':    case '3':
    case '4':    case '5':    case '6':    case '7':
    case '8':    case '9':
        get_number(true);
        break;

    case 'X':   /* hex number */
        read_ch();
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'X';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_hex_number(true);
        }
        else
            *err = true;
        break;

    case 'O':   /* octal number */
        read_ch();
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'O';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_oct_number();
        }
        else
            *err = true;
        break;

    case 'B':   /* binary number */
        read_ch();
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'B';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_bin_number();
        }
        else
            *err = true;
        break;
        
    default:
        assert(0);
        break;
    }
}

/* get signed number */
void get_signed_number(bool *err)
{
    bool plus;
    *err = false;

    plus = true;
    if (ch == '-')
        plus = false;

    if (ch == '+' || ch == '-') {
        size_t len = strlen(prn_line);
        if (len <= MAXLINESIZE-1) {
            prn_line[len] = ch;
            prn_line[len+1] = '\0';
        }

        read_ch();
        skip_space();
    }

    get_unsigned_number(err);

    if (!plus)
        number = -number;
}

/* search label table */
int search_table()
{
    int i = 0;
    bool found = false;

    while (i < table_ptr && !found) {
        if (!strncmp(label_table[i].name, symbol, MAXSYMSIZE)) {
            found = true;
        }
        else {
            i++;
        }
    }
    if (found)
        return i;

    return -1;
}

/* entry label into label_table */
void entry_table()
{
    if (search_table() == -1) {
        strncpy(label_table[table_ptr].name, symbol, MAXSYMSIZE);
        label_table[table_ptr].adrs = address;
        table_ptr++;
    }
    else {
        d_error[d_e_ptr] = line_no;
        d_e_ptr++;
        error++;
    }
}

/* get op_code from symbol */
void get_op_code()
{
    operation op;       /* should I use integer var for array index? */
    bool found;

    op = add;
    found = false;
    while (op <= end_op && !found) {
        if (!strcmp(code_table[op], symbol)) {
            found = true;
        }
        else {
            op = op+1;          /* is it allowed ? */
        }
    }
    if (found)
        op_code = op;
    else
        op_code = err;
}

/* print error */
void prn_error()
{
    if (prn_line[0] == ' ')
        prn_line[0] = 'F';
    else
        prn_line[1] = 'F';

    error++;
}

/* get constant */
void get_const(bool *err)
{
    bool plus;
    bool sp;
    int i;

    *err = false;
    plus = true;
    if (ch == '-')
        plus = false;

    if (ch == '+' || ch == '-') {
        size_t len = strlen(prn_line);
        if (len <= MAXLINESIZE-1) {
            prn_line[len] = ch;
            prn_line[len+1] = '\0';
        }

        read_ch();
        skip_space();
    }

    switch (ch) {
    case '0':    case '1':    case '2':    case '3':
    case '4':    case '5':    case '6':    case '7':
    case '8':    case '9':
        get_number(true);
        break;

    case 'X':
        read_ch();
        if (ch == ' ') {
            sp = true;
            skip_space();
        }
        else {
            sp = false;
        }
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'X';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_hex_number(true);
        }
        else {
            if (!sp) {
                int i;
                read_symbol();

                for (i = 7; i > 0; i--) {
                    symbol[i] = symbol[i-1];
                }
                symbol[0] = 'X';
                symbol[MAXSYMSIZE] = '\0';
            }
            else {
                symbol[0] = 'X'; symbol[1] = '\0';
            }

            assert(strlen(prn_line) <= MAXLINESIZE);
            strncat(prn_line, symbol, MAXLINESIZE);
            if ((i = search_table()) == -1)
                *err = true;
            else
                number = label_table[i].adrs;
        }
        break;

    case 'O':
        read_ch();
        if (ch == ' ') {
            sp = true;
            skip_space();
        }
        else {
            sp = false;
        }
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'O';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_oct_number();
        }
        else {
            if (!sp) {
                int i;
                read_symbol();

                for (i = 7; i > 0; i--) {
                    symbol[i] = symbol[i-1];
                }
                symbol[0] = 'O';
                symbol[MAXSYMSIZE] = '\0';
            }
            else {
                symbol[0] = 'O'; symbol[1] = '\0';
            }

            assert(strlen(prn_line) <= MAXLINESIZE);
            strncat(prn_line, symbol, MAXLINESIZE);
            if ((i = search_table()) == -1)
                *err = true;
            else
                number = label_table[i].adrs;
        }
        break;

    case 'B':
        read_ch();
        if (ch == ' ') {
            sp = true;
            skip_space();
        }
        else {
            sp = false;
        }
        if (ch == '"') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-2) {
                prn_line[len  ] = 'B';
                prn_line[len+1] = '"';
                prn_line[len+2] = '\0';
            }

            read_ch();
            skip_space();
            get_bin_number();
        }
        else {
            if (!sp) {
                int i;
                read_symbol();

                for (i = 7; i > 0; i--) {
                    symbol[i] = symbol[i-1];
                }
                symbol[0] = 'B';
                symbol[MAXSYMSIZE] = '\0';
            }
            else {
                symbol[0] = 'B'; symbol[1] = '\0';
            }

            assert(strlen(prn_line) <= MAXLINESIZE);
            strncat(prn_line, symbol, MAXLINESIZE);
            if ((i = search_table()) == -1)
                *err = true;
            else
                number = label_table[i].adrs;
        }
        break;

    case '\'':
        {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len  ] = '\'';
                prn_line[len+1] = '\0';
            }
        }
        number = 0;
        lower_case = true;
        read_ch();
        if (ch != '\n' && ch != '\0') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len  ] = '\'';
                prn_line[len  ] = '\0';
            }

            number = ch;
            read_ch();
            if (ch != '\n' && ch != '\0') {
                size_t len = strlen(prn_line);
                if (len <= MAXLINESIZE-1) {
                    prn_line[len  ] = '\'';
                    prn_line[len  ] = '\0';
                }

                number = ch;
                read_ch();
                number = number * 256 + ch;
            }
            else {
                number = number * 256 + 32;     /* 32 means space ? */
            }
        }
        else {
            number = 8224;      /* ??? 2 spaces? (= 0x20 * 256 + 0x20) */
        }
        lower_case = false;
        break;

    default:
/*        if (isalpha(ch) && isupper(ch)) */
        if (ch == 'A' || (ch >= 'C' && ch <= 'N') ||
            ch == 'P' || ch == 'W' || ch == 'Y' || ch == 'Z') {
            read_symbol();
            assert(strlen(prn_line) <= MAXLINESIZE);
            strncat(prn_line, symbol, MAXLINESIZE);
            if ((i = search_table()) == -1)
                *err = true;
            else
                number = label_table[i].adrs;
        }
        else
            *err = true;
        break;
    }

    if (!plus)
        number = -number;
}

/* get address from source file */
void get_address(bool *err)
{
    int i;
    bool plus;

    *err = false;

    if (ch == '*') {
        size_t len = strlen(prn_line);
        if (len <= MAXLINESIZE-1) {
            prn_line[len  ] = ch;
            prn_line[len+1] = '\0';
        }

        read_ch();
        skip_space();
        plus = true;
        if (ch == '-')
            plus = false;
        if (ch == '+' || ch == '-') {
            size_t len = strlen(prn_line);
            if (len <= MAXLINESIZE-1) {
                prn_line[len  ] = ch;
                prn_line[len+1] = '\0';
            }

            read_ch();
            skip_space();
            get_number(true);
            if (plus)
                number = address + number;
            else
                number = address - number;
        }
    }
    else if (isalpha(ch) && isupper(ch)) {
        read_symbol();
        skip_space();
        assert(strlen(prn_line) <= MAXLINESIZE);
        strncat(prn_line, symbol, MAXLINESIZE);
        if ((i = search_table()) == -1) {
            *err = true;
        }
        else {
            plus = true;
            if (ch == '-')
                plus = false;
            
            if (ch == '+' || ch == '-') {
                size_t len = strlen(prn_line);
                if (len <= MAXLINESIZE-1) {
                    prn_line[len  ] = ch;
                    prn_line[len+1] = '\0';
                }

                read_ch();
                skip_space();
                get_number(true);
                if (plus)
                    number = label_table[i].adrs + number;
                else
                    number = label_table[i].adrs - number;
            }
            else {
                number = label_table[i].adrs;
            }
        }
    }
    else {
        *err = true;
    }

}

/* get resister from source file */
void get_register(bool *err)
{
    *err = false;
    get_number(true);
    if (number < 0 || number > 3)
        *err = true;
}

/* PASS1 */
void pass1()
{
    fseek(source, 0, SEEK_SET);
    line_no = 1;
    address = 0;

    do {
        read_ch();
        skip_space();
        if (isalpha(ch) && isupper(ch)) {       /* 'A'..'Z' */
            read_symbol();
            skip_space();
            if (ch == ':') {        /* symbol is label */
                entry_table();
                read_ch();
                skip_space();
                read_symbol();
                skip_space();
            }

            get_op_code();

            switch (op_code) {
            case org:
                get_hex_number(false);
                address = number;
                break;
            case ds:
                get_number(false);
                address += number;
                break;
            default:
                address++;
                break;
            }
        }
        else {
            op_code = err;
            if (ch != ';')
                address++;
        }

        /* skip the remaining char of current line */
        while (ch != '\n' && ch != '\0')
            read_ch();

    } while (ch != '\0' && op_code != end_op);
}

/* print label table */
void print_label()
{
    static line_str line;
    int i;

    fprintf(print, "\n");
    fprintf(print, "LABEL(S)\n");
    for (i = 0; i < table_ptr; i++) {
        sprintf(line, "%s:%s",
                label_table[i].name,
                space(10));
        line[10] = '\0';        /* truncate to 10 chars */

        fprintf(print, "%s%s    ", line, dec_hex(label_table[i].adrs));
        if (i % 4 == 3)
            fputc('\n', print);
    }
    fprintf(print, "\n");
}

/* PASS2 */
void pass2()
{
    int i, n;
    bool bl;
    int ra, rb;
    int adrs;
    
    fseek(source, 0, SEEK_SET);

    if ((print = fopen(pname, "w")) == NULL) {
        printf("fail to open file '%s' for output\n", pname);
        exit(1);
    }
    if ((objectfile = fopen(oname, "w")) == NULL) {
        printf("fail to open file '%s' for object code\n", oname);
        exit(1);
    }

    line_no = 1;
    address = 0;

    read_ch();
    skip_space();

    do {
        if (ch == ';') {
            fprintf(print, "%s", space(23));
            lower_case = true;
            while (ch != '\n' && ch != '\0') {
                fputc(ch, print);
                read_ch();
            }
            fputc('\n', print);
            lower_case = false;
            read_ch();
            skip_space();
        }
    } while (ch == ';');

    read_symbol();
    get_op_code();

    if (op_code == title) {
        skip_space();
        read_symbol();
        fprintf(objectfile, "MM %s\n", symbol);
        fprintf(print, "%sTITLE %s\n", space(23), symbol);
        while (ch != '\n' && ch != '\0')
            read_ch();
    }
    else {
        fprintf(objectfile, "MM\n");
        fseek(source, 0, SEEK_SET);
        line_no = 1;
        /* should be repen print file ? */
        /* rewrite(print) */
    }

    do {
        read_ch();
        skip_space();

        if (isalpha(ch) && isupper(ch)) {
            read_symbol();
            skip_space();
            sprintf(prn_line, "  %s%s", dec_hex(address), space(9));

            if (ch == ':') {
                i = 0;
                bl = false;
                while (i < d_e_ptr && !bl) {
                    if (d_error[i] == line_no)
                        bl = true;
                    else
                        i++;
                }
                if (bl)
                    prn_line[0] = 'D';
                assert(strlen(prn_line) <= MAXLINESIZE);
                strncat(prn_line, symbol, MAXLINESIZE);
                strncat(prn_line, ":", MAXLINESIZE);
                strncat(prn_line, space(MAXSYMSIZE-1 - strlen(symbol)), MAXLINESIZE);
                
                read_ch();
                skip_space();
                read_symbol();
                skip_space();
            }
            else {
                assert(strlen(prn_line) <= MAXLINESIZE);
                strncat(prn_line, space(8), MAXLINESIZE);
            }

            assert(strlen(prn_line) <= MAXLINESIZE);
            strncat(prn_line, symbol, MAXLINESIZE);
            strncat(prn_line, space(6 - strlen(symbol)), MAXLINESIZE);

            get_op_code();
            if (op_code >= add && op_code <= hlt) {
                fprintf(objectfile, "%s  ", dec_hex(address));
            }

            switch (op_code) {
            case add:
            case sub:
            case and_op:
            case or_op:
            case xor_op:
            case mult:
            case div_op:
            case cmp:
            case ex:
                get_register(&bl);
                if (!bl) {
                    rb = number;
                    skip_space();
                    if (ch == ',') {
                        size_t len = strlen(prn_line);
                        if (len <= MAXLINESIZE-1) {
                            prn_line[len  ] = ch;
                            prn_line[len+1] = '\0';
                        }

                        read_ch();
                        skip_space();
                        if (isdigit(ch) ||
                            ch == 'X' || ch == 'O' || ch == 'B' ||
                            ch == '(') {

                            if (ch == '(') {    /* without offset value */
                                size_t len = strlen(prn_line);
                                if (len <= MAXLINESIZE-1) {
                                    prn_line[len  ] = ch;
                                    prn_line[len+1] = '\0';
                                }

                                read_ch();
                                skip_space();
                                get_register(&bl);
                                skip_space();
                                if (!bl && ch == ')') {
                                    size_t len = strlen(prn_line);
                                    if (len <= MAXLINESIZE-1) {
                                        prn_line[len  ] = ch;
                                        prn_line[len+1] = '\0';
                                    }

                                    read_ch();
                                    ra = number;
                                    i = 0;
                                }
                                else {
                                    if (!bl)
                                        prn_error();
                                    skip_ch(";\n\0", false);
                                    i = -1;
                                }
                            }
                            else {      /* with offset value */
                                get_unsigned_number(&bl);
                                if (!bl) {
                                    i = number;
                                    skip_space();
                                    if (ch == '(') {
                                        size_t len = strlen(prn_line);
                                        if (len <= MAXLINESIZE-1) {
                                            prn_line[len  ] = ch;
                                            prn_line[len+1] = '\0';
                                        }

                                        read_ch();
                                        skip_space();
                                        get_register(&bl);
                                        skip_space();
                                        if (!bl && ch == ')') {
                                            size_t len = strlen(prn_line);
                                            if (len <= MAXLINESIZE-1) {
                                                prn_line[len  ] = ch;
                                                prn_line[len+1] = '\0';
                                            }

                                            read_ch();
                                            ra = number;
                                        }
                                        else {
                                            if (!bl)
                                                prn_error();
                                            skip_ch(";\n\0", false);
                                            i = -1;
                                        }
                                    }
                                    else {
                                        ra = 0;
                                    }
                                }
                                else {
                                    prn_error();
                                    skip_ch(";\n\0", false);
                                    i = -1;
                                }
                            }

                            if (i != -1) {  /* no error */
                                n = op_code;
                                if (n > 4) n++;
                                if (n != 9) {
                                    prn_line[7] = '0' + n;
                                    fprintf(objectfile, "%c", '0' + n);
                                }
                                else {
                                    prn_line[7] = 'F';
                                    fprintf(objectfile, "F");
                                }
                                prn_line[9] = '0' + ra;
                                prn_line[10] = '0' + rb;
                                n = ra * 4 + rb;
                                if (n > 9) n += 7;
                                fprintf(objectfile, "%c", '0' + n);
                                strncpy(hex_number, dec_hex(i), HEXDIGITS);
                                prn_line[12] = hex_number[2];
                                prn_line[13] = hex_number[3];
                                fprintf(objectfile, "%c%c\n",
                                        hex_number[2], hex_number[3]);
                            }
                        }
                        else {
                            prn_error();
                            skip_ch(";\n\0", false);
                        }
                    }
                    else {
                        prn_error();
                        skip_ch(";\n\0", false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0", false);
                }
                break;

            case lc:
            case push:
            case pop:
                get_register(&bl);
                if (!bl) {
                    rb = number;
                    skip_space();
                    if (ch == ',') {
                        size_t len = strlen(prn_line);
                        if (len <= MAXLINESIZE-1) {
                            prn_line[len  ] = ch;
                            prn_line[len+1] = '\0';
                        }

                        read_ch();
                        skip_space();
                        if (isdigit(ch) ||
                            ch == 'X' || ch == 'O' || ch == 'B') {
                            get_unsigned_number(&bl);
                            if (!bl) {
                                if (op_code == lc) {
                                    prn_line[7] = '9';
                                    fprintf(objectfile, "9");
                                    prn_line[9] = '3';
                                    prn_line[10] = '0' + rb;
                                    fprintf(objectfile, "%c", rb+67);
                                }
                                else {
                                    prn_line[7] = 'D';
                                    fprintf(objectfile, "D");
                                    ra = op_code - 10;
                                    prn_line[9] = '0' + ra;
                                    prn_line[10] = '0' + rb;
                                    n = ra * 4 + rb;
                                    if (n > 9) n += 7;
                                    fprintf(objectfile, "%c", n+'0');
                                }
                                strncpy(hex_number, dec_hex(number), HEXDIGITS);
                                prn_line[12] = hex_number[2];
                                prn_line[13] = hex_number[3];
                                fprintf(objectfile, "%c%c\n",
                                        hex_number[2], hex_number[3]);
                            }
                            else {
                                prn_error();
                                skip_ch(";\n\0",false);
                            }
                        }
                        else {
                            prn_error();
                            skip_ch(";\n\0",false);
                        }
                    }
                    else {
                        prn_error();
                        skip_ch(";\n\0",false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0",false);
                }
                break;

            case sl:
            case sa:
            case sc:
            case bix:
                get_register(&bl);
                if (!bl) {
                    rb = number;
                    skip_space();
                    if (ch == ',') {
                        size_t len = strlen(prn_line);
                        if (len <= MAXLINESIZE-1) {
                            prn_line[len  ] = ch;
                            prn_line[len+1] = '\0';
                        }

                        read_ch();
                        skip_space();
                        if (isdigit(ch) || ch == '+' || ch == '-' ||
                            ch == 'X' || ch == 'O' || ch == 'B') {

                            get_signed_number(&bl);
                            if (!bl) {
                                if (op_code >= sl && op_code <= sc) {
                                    prn_line[7] = '5';
                                    fprintf(objectfile,"5");
                                    ra = op_code - 12;
                                    prn_line[9] = '0' + ra;
                                    prn_line[10] = '0' + rb;
                                    n = ra * 4 + rb;
                                    if (n > 9) n += 7;
                                    fprintf(objectfile, "%c", '0' + n);
                                }
                                else {
                                    prn_line[7] = 'D';
                                    fprintf(objectfile, "D");
                                    prn_line[9] = '2';
                                    prn_line[10] = '0' + rb;
                                    fprintf(objectfile, "%c", 56 + rb);
                                }
                                strncpy(hex_number, dec_hex(number), HEXDIGITS);
                                prn_line[12] = hex_number[2];
                                prn_line[13] = hex_number[3];
                                fprintf(objectfile, "%c%c\n",
                                        hex_number[2], hex_number[3]);
                            }
                            else {
                                prn_error();
                                skip_ch(";\n\0",false);
                            }
                        }
                        else {
                            prn_error();
                            skip_ch(";\n\0",false);
                        }
                    }
                    else {
                        prn_error();
                        skip_ch(";\n\0",false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0",false);
                }
                break;

            case lea:
            case lx:
            case stx:
                get_register(&bl);
                if (!bl) {
                    rb = number;
                    skip_space();
                    if (ch == ',') {
                        size_t len = strlen(prn_line);
                        if (len <= MAXLINESIZE-1) {
                            prn_line[len  ] = ch;
                            prn_line[len+1] = '\0';
                        }

                        read_ch();
                        skip_space();
                        if (isdigit(ch) || ch == '+' || ch == '-' ||
                            ch == 'X' || ch == 'O' || ch == 'B' || ch == '(') {
                            if (ch != '(') {
                                get_signed_number(&bl);
                                i = number;
                                skip_space();
                            }
                            else {
                                i = 0;
                            }
                            if (!bl && ch == '(') {
                                size_t len = strlen(prn_line);
                                if (len <= MAXLINESIZE-1) {
                                    prn_line[len  ] = ch;
                                    prn_line[len+1] = '\0';
                                }

                                read_ch();
                                skip_space();
                                get_register(&bl);
                                skip_space();
                                if (!bl && ch == ')') {
                                    size_t len = strlen(prn_line);
                                    if (len <= MAXLINESIZE-1) {
                                        prn_line[len  ] = ch;
                                        prn_line[len+1] = '\0';
                                    }

                                    read_ch();
                                    ra = number;
                                    n = op_code + 49;
                                    prn_line[7] = n;
                                    fprintf(objectfile, "%c", n);
                                    prn_line[9] = '0' + ra;
                                    prn_line[10] = '0' + rb;
                                    n = ra * 4 + rb;
                                    if (n > 9) n += 7;
                                    fprintf(objectfile, "%c", '0' + n);
                                    strncpy(hex_number, dec_hex(i), HEXDIGITS);
                                    prn_line[12] = hex_number[2];
                                    prn_line[13] = hex_number[3];
                                    fprintf(objectfile, "%c%c\n",
                                            hex_number[2], hex_number[3]);
                                }
                                else {
                                    if (!bl) {
                                        prn_error();
                                        skip_ch(";\n\0",false);
                                    }
                                }
                            }
                            else {
                                prn_error();
                                skip_ch(";\n\0",false);
                            }
                        }
                        else {
                            prn_error();
                            skip_ch(";\n\0",false);
                        }
                    }
                    else {
                        prn_error();
                        skip_ch(";\n\0",false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0",false);
                }
                break;

            case l:
            case st:
            case la:
                get_register(&bl);
                if (!bl) {
                    rb = number;
                    skip_space();
                    if (ch == ',') {
                        size_t len = strlen(prn_line);
                        if (len <= MAXLINESIZE-1) {
                            prn_line[len  ] = ch;
                            prn_line[len+1] = '\0';
                        }

                        read_ch();
                        skip_space();
                        get_address(&bl);
                        if (!bl) {
                            adrs = number - address;
                            if (adrs >= -128 && adrs <= 127) {
                                strncpy(hex_number, dec_hex(adrs), HEXDIGITS);
                                prn_line[7] = '9';
                                fprintf(objectfile, "9");
                                ra = op_code - 19;
                                prn_line[9] = '0' + ra;
                                prn_line[10] = '0' + rb;
                                prn_line[12] = hex_number[2];
                                prn_line[13] = hex_number[3];
                                n = ra * 4 + rb;
                                if (n > 9) n += 7;
                                fprintf(objectfile, "%c", n+48);
                                fprintf(objectfile, "%c%c\n",
                                        hex_number[2], hex_number[3]);
                            }
                            else {
                                if (prn_line[0] == ' ')
                                    prn_line[0] = 'L';
                                else
                                    prn_line[1] = 'L';
                                error++;
                                skip_ch(";\n\0",false);
                            }
                        }
                        else {
                            prn_error();
                            skip_ch(";\n\0",false);
                        }
                    }
                    else {
                        prn_error();
                        skip_ch(";\n\0",false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0",false);
                }
                break;

            case bdis:
            case bp:
            case bz:
            case bm:
            case bc:
            case bnp:
            case bnz:
            case bnm:
            case bnc:
            case b:
            case bi:
            case bsr:
                get_address(&bl);
                if (!bl) {
                    number = number - address;
                    if (number >= -128 && number <= 127) {
                        if (op_code == bdis) {
                            prn_line[7] = 'D';
                            prn_line[9] = '3';
                            prn_line[10] = '0';
                            fprintf(objectfile, "DC");
                        }
                        else {
                            prn_line[7] = 'E';
                            fprintf(objectfile, "E");
                            n = op_code - 23;
                            ra = n / 4;
                            rb = n % 4;
                            prn_line[9] = '0' + ra;
                            prn_line[10] = '0' + rb;
                            n = ra * 4 + rb;
                            if (n > 9) n += 7;
                            fprintf(objectfile, "%c", '0' + n);
                        }
                        strncpy(hex_number, dec_hex(number), HEXDIGITS);
                        prn_line[12] = hex_number[2];
                        prn_line[13] = hex_number[3];
                        fprintf(objectfile, "%c%c\n",
                                hex_number[2], hex_number[3]);
                    }
                    else {
                        if (prn_line[0] == ' ')
                            prn_line[0] = 'L';
                        else
                            prn_line[1] = 'L';
                        error++;
                        skip_ch(";\n\0",false);
                    }
                }
                else {
                    prn_error();
                    skip_ch(";\n\0",false);
                }
                break;

            case rio:
            case wio:
                if (!isalpha(ch) || !isupper(ch)) {
                    get_number(true);
                    if (number < 0 || number > 255) {
                        prn_error();
                        number = -1;
                    }
                }
                else {
                    read_symbol();
                    assert(strlen(prn_line) <= MAXLINESIZE);
                    strncat(prn_line, symbol, MAXLINESIZE);
                    if (!strncmp(symbol, "CR", 8))
                        number = 0;
                    else
                        if (!strncmp(symbol, "LPT", 8))
                            number = 1;
                        else {
                            prn_error();
                            number = -1;
                        }
                }

                if (number != -1) {
                    prn_line[7] = 'E';
                    prn_line[9] = '3';
                    fprintf(objectfile,"E");
                    if (op_code == rio) {
                        prn_line[10] = '0';
                        fprintf(objectfile,"C");
                    }
                    else {
                        prn_line[10] = '1';
                        fprintf(objectfile,"D");
                    }
                    strncpy(hex_number, dec_hex(number), HEXDIGITS);
                    prn_line[12] = hex_number[2];
                    prn_line[13] = hex_number[3];
                    fprintf(objectfile,"%c%c\n",
                            hex_number[2],hex_number[3]);
                }
                break;

            case ret:
            case nop:
            case hlt:
                prn_line[7] = 'E';
                prn_line[12] = '0';
                prn_line[13] = '0';
                switch (op_code) {
                case ret:
                    prn_line[ 9] = '2';
                    prn_line[10] = '3';
                    fprintf(objectfile, "EB00\n");
                    break;
                case nop:
                    prn_line[ 9] = '3';
                    prn_line[10] = '2';
                    fprintf(objectfile, "EE00\n");
                    break;
                case hlt:
                    prn_line[ 9] = '3';
                    prn_line[10] = '3';
                    fprintf(objectfile, "EF00\n");
                    break;
                default:
                    assert(0);
                    break;
                }
                break;

            case dc:
                get_const(&bl);
                if (!bl) {
/*
                    delete(prn_line,8,4);
                    insert(dec_hex(number),prn_line,8)
*/
                    memcpy(&prn_line[7], dec_hex(number), HEXDIGITS);
                }
                else {
                    prn_error();
                    skip_ch(";\n\0", false);
                }
                fprintf(objectfile, "%s  ", dec_hex(address));
                fprintf(objectfile, "%s\n", dec_hex(number));
                break;

            case ds:
                if (isdigit(ch)) {
                    get_number(true);
                    prn_line[7] = '0';
                    prn_line[8] = '0';
                    prn_line[9] = '0';
                    prn_line[10] = '0';

                    fprintf(objectfile, "%s  0000\n", dec_hex(address));
                    while (ch != ';' && ch != '\n' && ch != '\0')
                        read_ch();
                    if (ch == ';') {
                        assert(strlen(prn_line) <= MAXLINESIZE);
                        strncat(prn_line, space(11), MAXLINESIZE);
                        prn_line[41] = '\0';    /* truncate to 41 chars */
                        skip_ch("\n\0", true);
                    }
                    for (i = 0; i < number-1; i++) {
                        fprintf(print, "%s\n", prn_line);
                        address++;
                        sprintf(prn_line, "  %s 0000", dec_hex(address));
                        fprintf(objectfile, "%s  0000\n", dec_hex(address));
                    }
                    address++;
                }
                else {
                    prn_error();
                    skip_ch(";\n\0", false);
                }
                break;

            case org:
                get_hex_number(true);
                address = number;
                prn_line[2] = ' ';
                prn_line[3] = ' ';
                prn_line[4] = ' ';
                prn_line[5] = ' ';
                break;

/*            case title:
              break;*/

            case end_op:
                prn_line[2] = ' ';
                prn_line[3] = ' ';
                prn_line[4] = ' ';
                prn_line[5] = ' ';
                break;

            default:
                prn_error();
                break;
            }

            if (op_code != ds && op_code != org && op_code != end_op)
                address++;

            while (ch != ';' && ch != '\n' && ch != '\0')
                read_ch();

            if (ch == ';') {
                assert(strlen(prn_line) <= MAXLINESIZE);
                strncat(prn_line, space(18), MAXLINESIZE);
                prn_line[41] = '\0';    /* truncate to 41 chars */
                skip_ch("\n\0", true);
            }
        }
        else {          /* comment or illegal symbol */
            op_code = err;
            if (ch != ';') {    /* illegal symbol */
                strncpy(prn_line, "F ", MAXLINESIZE);
                strncat(prn_line, dec_hex(address), MAXLINESIZE);
                strncat(prn_line, space(17), MAXLINESIZE);

                error++;
                address++;
                skip_ch(" :;\n\0",false);

                if (ch == ' ' || ch ==  ':') {

                    if (ch == ' ')
                        skip_space();

                    if (ch == ':') {
                        strncpy(&prn_line[6], &prn_line[6+8], MAXLINESIZE-15);
                        strncat(prn_line, ":       ", MAXLINESIZE);
                        prn_line[23] = '\0';    /* truncate to 23 chars */
                        read_ch();
                        skip_space();
                        skip_ch(" ;\n\0",false);
                    }

                    strncat(prn_line, space(6), MAXLINESIZE);
                    prn_line[29] = '\0';        /* truncate to 29 chars */
                    skip_ch(";\n\0",false);
                }
                if (ch == ';') {
                    strncat(prn_line, space(18), MAXLINESIZE);
                    prn_line[41] = '\0';        /* truncate to 41 chars */
                }
            }
            else {
                strncpy(prn_line, space(23), MAXLINESIZE);
            }
            skip_ch("\n\0",true);
        }

        fprintf(print, "%s\n", prn_line);

    } while (ch != '\0' && op_code != end_op);

    if (op_code != end_op) {
        fprintf(print, "F");
        error++;
    }
    fputc('\n', print);

    switch (error) {
    case 0:
        fprintf(print, "THERE WERE NO ERRORS.\n");
        break;
    case 1:
        fprintf(print, "THERE WAS ONE ERROR.\n");
        break;
    default:
        fprintf(print, "THERE WERE %d ERRORS.\n", error);
        break;
    }

    print_label();
}


/* MICRO-1 ASSEMBLER main routine */

int main()
{
    /* clrscr; */
//    printf("   *** MICRO-1 ASSEMBLER (Ver. 1.2a) ***\n");
    printf("   *** MICRO-1 ASSEMBLER (C-Ver. 1.0) ***\n");

    do {
        do {
            open_files();

            if (file_ok) {
                putchar('\n');
                printf(" START ? (Y/N):");

                do {
                    ch = getchar();
                } while (toupper(ch) != 'Y' && toupper(ch) != 'N');

                file_ok = toupper(ch) == 'Y'? true: false;
            }
            else {
                printf("  FILE %s NOT FOUND !\n", sname);
            }

            if (!file_ok)
                fclose(source);
        }  while (file_ok == false);

        putchar('\n');

        table_ptr = 0;
        error = 0;
        d_e_ptr = 0;
        lower_case = false;

        /* assemble */
        pass1();
        pass2();

        if (error == 0)
            printf(" NORMAL TERMINATION !\n");
        else
            printf(" ERROR OCCURED IN ASSEMBLE TIME !\n");
//            printf("^['[33m ERROR OCCURED IN ASSEMBLE TIME !'^['[m\n");

        putchar('\n');

        close_files();

        if (error > 0)
            remove(oname);

        printf(" CONTINUE ? (Y/N):");

        do {
            ch = getchar();
        } while (toupper(ch) != 'Y' && toupper(ch) != 'N');

    } while (toupper(ch) == 'Y');

    return 0;
}

/* end of asm.c */
