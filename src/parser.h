#ifndef WAX_PARSER_H
#define WAX_PARSER_H

#include "common.h"

typedef struct tok_st {
    int tag;
    int lino; // line no.
    str_t val;
} tok_t;


typedef struct type_st {
    int tag;
    struct type_st *elem0;
    union{
        int size;
        str_t name;
        struct type_st *elem1;
    } u;
} type_t;


typedef struct sym_st {
    int is_local;
    str_t name;
    type_t type;
} sym_t;

typedef struct stt_st {
    str_t name;
    list_t fields;
} stt_t;

typedef struct func_st {
    str_t name;
    list_t params;
    type_t* result;
} func_t;


typedef struct expr_st {
    int key;
    int lino;
    str_t rawkey;
    void* term;
    list_t children;
    struct expr_st* parent;
    map_t symtable;
    type_t* type;
} expr_t;

// keywords
#define EXPR_TYPE   (-1)
#define EXPR_TERM   (-2)

#define EXPR_TBA    (-3) // to be assigned

#define EXPR_PRGM    0

#define EXPR_EXTERN  1
#define EXPR_RETURN  2
#define EXPR_RESULT  3
#define EXPR_STRUCT  4
#define EXPR_PARAM   5
#define EXPR_WHILE   7
#define EXPR_ALLOC   8
#define EXPR_FUNC    9
#define EXPR_THEN   10
#define EXPR_ELSE   11
#define EXPR_CALL   12
#define EXPR_CAST   13
#define EXPR_FREE   14
#define EXPR_LET    15
#define EXPR_FOR    16
#define EXPR_IF     17
#define EXPR_DO     18
#define EXPR_TIF    19
#define EXPR_SET    20
#define EXPR_PRINT  21
#define EXPR_FORIN  22
#define EXPR_BREAK  23
#define EXPR_ASM    24

// logical operators
#define EXPR_LAND  101
#define EXPR_LOR   102
#define EXPR_LNOT  103

// int-only operators
#define EXPR_XOR   151
#define EXPR_BAND  152
#define EXPR_BOR   153
#define EXPR_SHL   154
#define EXPR_SHR   155
#define EXPR_BNEG  156

// more int operators
#define EXPR_IADD  201
#define EXPR_ISUB  202
#define EXPR_IMUL  203
#define EXPR_IDIV  204
#define EXPR_IMOD  205

// int comparison
#define EXPR_ILT   251
#define EXPR_IGT   252
#define EXPR_ILEQ  253
#define EXPR_IGEQ  254
#define EXPR_IEQ   255
#define EXPR_INEQ  256

// float operators
#define EXPR_FADD  301
#define EXPR_FSUB  302
#define EXPR_FMUL  303
#define EXPR_FDIV  304
#define EXPR_FMOD  305

// float comparison
#define EXPR_FLT   351
#define EXPR_FGT   352
#define EXPR_FLEQ  353
#define EXPR_FGEQ  354
#define EXPR_FEQ   355
#define EXPR_FNEQ  356

// vec things
#define EXPR_VECGET 401
#define EXPR_VECSET 402

// arr things
#define EXPR_ARRGET 451
#define EXPR_ARRSET 452
#define EXPR_ARRREM 453
#define EXPR_ARRINS 454
#define EXPR_ARRCPY 455
#define EXPR_ARRLEN 456

// map things
#define EXPR_MAPGET 501
#define EXPR_MAPSET 502
#define EXPR_MAPLEN 503
#define EXPR_MAPREM 504

// struct things
#define EXPR_STRUCTGET 551
#define EXPR_STRUCTSET 552
#define EXPR_STRUCTFLD 553

// string things
#define EXPR_STRGET 601
#define EXPR_STRADD 602
#define EXPR_STRLEN 603
#define EXPR_STRCPY 604
#define EXPR_STREQL 605
#define EXPR_STRNEQ 606
#define EXPR_STRCAT 607

// function things
#define EXPR_FUNCBODY 651
#define EXPR_FUNCHEAD 652

// pointer-y things
#define EXPR_SETNULL 701
#define EXPR_NOTNULL 702
#define EXPR_PTREQL  703
#define EXPR_PTRNEQ  704

/* /end keywords */

#define TYP_NONE 0
#define TYP_INT 1
#define TYP_FLT 2
#define TYP_STR 3
#define TYP_VEC 4
#define TYP_ARR 5
#define TYP_MAP 6
#define TYP_STT 7
#define TYP_VOD 8
#define TYP_TYP 9

#define TOK_LPR 1 //left paran
#define TOK_RPR 2 //right paren
#define TOK_INT 3 //int literal
#define TOK_FLT 4 //float literal
#define TOK_STR 5 //string literal
#define TOK_IDT 6 //identifier
#define TOK_KEY 7 //keyword

expr_t* expr_alloc();

type_t* prim_type(int);

expr_t* type_expr(type_t*, int);

sym_t* sym_new(str_t, type_t, char);

func_t* func_lookup(str_t*, map_t*);

int included_lookup(char*,map_t*);

type_t* sym_lookup_local(str_t*, expr_t*);

void lift_scope(expr_t*);

list_t tokenize(str_t);

void print_tokens(list_t*);

void print_stttable(map_t*);

void print_functable(map_t*);

void print_syntax_tree(expr_t*, int);

void write_file_ascii(const char*, char*);

str_t read_file_ascii(const char*);

void defs_addbool(map_t*, char*, int);

void preprocess(const char*, list_t*, map_t*, map_t*);

expr_t* syntax_tree(list_t*);

void compile_syntax_tree(expr_t* tree, map_t* functable, map_t* stttable);

extern const char* tokens_desc[];

#endif /* WAX_PARSER_H */
