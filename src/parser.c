#ifndef WAX_PARSER
#define WAX_PARSER

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <math.h>

#include "text.c"
#include "common.c"

#define TOK_LPR 1 //left paran
#define TOK_RPR 2 //right paren
#define TOK_INT 3 //int literal
#define TOK_FLT 4 //float literal
#define TOK_STR 5 //string literal
#define TOK_IDT 6 //identifier
#define TOK_KEY 7 //keyword

const char* tokens_desc[] = {"","L-PAREN","R-PAREN","INT-LIT","FLT-LIT","STR-LIT","IDENTFR","KEYWORD"};

// keywords
#define EXPR_TYPE   -1
#define EXPR_TERM   -2

#define EXPR_TBA    -3 // to be assigned

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

#define NUM_KEYWORDS 50
const char* keywords[NUM_KEYWORDS] = {
  /*6*/"return","result","struct","insert","remove","extern",
  /*5*/"param","local","while","alloc","slice","print","break",
  /*4*/"func","then","else","call","cast","free","null",
  /*3*/"get","set","let","for","asm",
  /*2*/"if","do","<<",">>","=","&&","||",">=","<=","<>","??",
  /*1*/"+","-","*","/","^","%","&","|","!","~","<",">","#","?"
};


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


#define NUM_TYPES 9
const char* types[NUM_TYPES] = {
  "int","float","str","vec","arr","map","struct","void","type",
};


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

typedef struct def_st {
  str_t name;
  tok_t* token;
} def_t;

expr_t* expr_alloc(){
  expr_t* expr = (expr_t*)mallocx(sizeof(expr_t));
  memset(expr, 0, sizeof(expr_t));
  return expr;
}

tok_t* tok_alloc(char tag, int lino){
  tok_t* tok = (tok_t*) mallocx(sizeof(tok_t));
  tok->tag = tag;
  tok->lino = lino;
  return tok;
}



int tok_eq(tok_t* tok, const char* cs){
  if (strcmp(tok->val.data,cs) == 0){
    return 1;
  }
  return 0;
}


str_t read_file_ascii(const char* filename){
  char * buffer = 0;
  long length;
  size_t sz;
  FILE * f = fopen (filename, "r");
  if (f){
    fseek(f, 0, SEEK_END);
    length = ftell (f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length+1);
    if (buffer){
      sz = fread(buffer, 1, length, f);
      if(sz != length) goto failed;
    }
    fclose(f);
  
    buffer[length] = 0;
    str_t s = str_from(buffer, length);
    free(buffer);
    return s;
  }
failed:
  printerr("file")("cannot read file %s.\n",filename);
  printf("exiting with file reading failure.\n");
  freex();exit(1);
  return str_from("",0);
}


void write_file_ascii(const char* filename, char* content){
  FILE *fp;
  fp = fopen(filename, "w");
  fputs(content, fp);
  fclose(fp);
}



tok_t* word_to_token(str_t s, int lino){
  if (('0' <= s.data[0] && s.data[0] <= '9') || s.data[0] == '.' || (s.data[0] == '-' && s.len > 1)){
    int isfloat = 0;
    for (int i = 0; i < s.len; i++){
      if (s.data[i] == '.'){
        isfloat = 1;
      }else if (('0' > s.data[i] || s.data[i] > '9') && s.data[i] != '.' && s.data[i] != 'e' && s.data[i] != 'x' && s.data[0] != '-'){
        printerr("syntax")("line %d: invalid number literal `%s`\n",lino,s.data);
        // free(s.data);
        return NULL;
      }
    }
    if (isfloat){
      tok_t* tok = tok_alloc(TOK_FLT,lino);
      tok->val = s;
      return tok;
    }else{
      tok_t* tok = tok_alloc(TOK_INT,lino);
      tok->val = s;
      return tok;
    }
  }else if (s.data[0] == '\''){
    tok_t* tok = tok_alloc(TOK_INT,lino);
    tok->val = s;
    return tok;
  }else if (s.data[0] == '"'){
    if (s.data[s.len-1] != '"'){
      printerr("syntax")("line %d: invalid string literal `%s`\n",lino,s.data);
      // free(s.data);
      return NULL;
    }else{
      tok_t* tok = tok_alloc(TOK_STR,lino);
      tok->val = s;
      return tok;
    }
  }else{
    for (int i = 0; i < NUM_KEYWORDS; i++){
      if (s.len != strlen(keywords[i])){
        continue;
      }
      int ok = 1;
      for (int j = 0; j < s.len; j++){
        if (s.data[j] != keywords[i][j]){
          ok = 0;
          break;
        }
      }
      if (ok){
        tok_t* tok = tok_alloc(TOK_KEY,lino);
        tok->val = s;
        return tok;
      }
    }
    tok_t* tok = tok_alloc(TOK_IDT,lino);

    tok->val = censor(s);
    return tok;
  }
  printerr("syntax")("line %d: invalid token `%s`\n",lino,s.data);
  // free(s.data);
  return NULL;
}

#define PUSH_TOKEN \
  if (buf.len){\
    tok_t* tok = word_to_token(buf,lino);\
    if (!tok){\
      printf("exiting with tokenization failure.\n");\
      freex();exit(1);\
    }\
    list_add(&tokens,tok);\
    buf = str_new();\
    isquote = 0;\
  }

#define ESC_SPECIAL \
  if (!isquote){\
    if (buf.len){\
      PUSH_TOKEN\
    }\
  }else{\
    str_addch(&buf,src.data[i]);\
  }

list_t tokenize(str_t src){
  list_t tokens;
  list_init(&tokens);
  str_t buf = str_new();
  int lino = 1;
  int isquote = 0;
  int iscomment = 0;
  for (int i = 0; i < src.len; i++){
    if (iscomment){
      if (src.data[i] == '\n'){
        iscomment = 0;
      }else{
        continue;
      }
    }
    if (src.data[i] == '(' || src.data[i] == '[' || src.data[i] == '{'){
      ESC_SPECIAL
      if (!isquote){
        tok_t* tok = tok_alloc(TOK_LPR,lino);
        tok->val = str_fromch(src.data[i]);
        list_add(&tokens,tok);
      }
    }else if (src.data[i] == ')' || src.data[i] == ']' || src.data[i] == '}'){
      ESC_SPECIAL
      if (!isquote){
        tok_t* tok = tok_alloc(TOK_RPR,lino);
        tok->val = str_fromch(src.data[i]);
        list_add(&tokens,tok);
      }
    }else if (src.data[i] == '"'){
      if (i == 0 || src.data[i-1] != '\\'){
        if (buf.len == 0){
          isquote = 1;
          str_addch(&buf,src.data[i]);
        }else{
          str_addch(&buf,src.data[i]);
          PUSH_TOKEN
        }
      }else{
        str_addch(&buf,src.data[i]);
      }
    }else if (src.data[i] == '\''){
      if (!isquote && buf.len == 0){
        str_addch(&buf,src.data[i++]);
        if (src.data[i]=='\\')str_addch(&buf,src.data[i++]);
        if (i < src.len)str_addch(&buf,src.data[i++]);
        if (i < src.len)str_addch(&buf,src.data[i]);
        PUSH_TOKEN
      }else{
        str_addch(&buf,src.data[i]);
      }
    }else if (src.data[i] == ' ' || src.data[i] == '\t'){
      ESC_SPECIAL
    }else if (src.data[i] == ';'){
      if (!isquote){
        iscomment = 1;
      }
      ESC_SPECIAL
    }else if (src.data[i] == '\n'){
      ESC_SPECIAL
      lino++;
    }else{
      str_addch(&buf,src.data[i]);
    }
  }
  return tokens;
}

void print_tokens(list_t* tokens){
  list_node_t* it = tokens->head;
  int i = 0;
  printf("----- tokens (#%4d) [%p %p] -----\n",tokens->len, (void*)(tokens->head), (void*)(tokens->tail));
  while (it && it->prev != tokens->tail){
    tok_t* tok = (tok_t*)(it->data);
    printf("(%5d) line %4d\t[%s]\t%s\n", i, tok->lino, tokens_desc[(int)(tok->tag)], tok->val.data);
    it = it->next;
    i++;
  }
}


int included_lookup(char* fname,map_t* included){

  str_t ss = str_from(fname,strlen(fname));
  int k = map_hash(ss);
  if (included->slots[k]){
    list_node_t* it = included->slots[k]->head;
    while (it){
      char* s = (char*)(it->data);
      // printf("%s %s\n",ss.data,s);
      if (str_eq(&ss,s)){
        return 1;
      }
      it = it->next;
    }
  }
  return 0;
}

tok_t* def_lookup(char* fname, map_t* defs){

  str_t ss = str_from(fname,strlen(fname));
  int k = map_hash(ss);

  if (defs->slots[k]){
    list_node_t* it = defs->slots[k]->tail;
    while (it){
      def_t* d = (def_t*)(it->data);
      // printf("%s %s\n",ss->data,d->name.data);
      if (str_eq(&ss,d->name.data)){

        return d->token;
      }
      it = it->prev;
    }
  }
  return 0;
}

void defs_addbool(map_t* defs, char* name_, int lino){
  str_t name = str_from(name_,strlen(name_));
  tok_t* t = (tok_t*) mallocx(sizeof(tok_t));
  t->tag = TOK_INT;
  t->lino = lino;
  t->val = str_from("1",1);

  def_t* d = (def_t*) mallocx(sizeof(def_t));
  d->name = name;
  d->token = t;
  map_add(defs,name,d);
}

void preprocess(const char* filename, list_t* tokens, map_t* included, map_t* defs){

  str_t fnstr = str_from(filename,strlen(filename));

  list_node_t* it = tokens->head;
  while (it){
    
    tok_t* tok = (tok_t*)(it->data);


    int has_next  = (it->next) && (it->next->next) && (it->next->next->next);
    int has_next2 = has_next  && (it->next->next->next->next);
    int has_prev = (it->prev) && (it->prev->prev);


    if (tok_eq(tok,"@pragma")){
      if (it->next && tok_eq((tok_t*)(it->next->data),"once") ){
        map_add(included,fnstr,fnstr.data);
      }else{
        printerr("syntax")("invalid @pragma.\n");
        goto crash;
      }
      goto gouge;

    }else if (tok_eq(tok,"@define")){
      if (!it->prev || ((tok_t*)(it->prev->data))->tag != TOK_LPR){
        printerr("syntax")("invalid @define.\n");
        goto crash;
      }
      if (it->next && it->next->next && it->next->next->next 
        && ((tok_t*)(it->next->next->next->data))->tag == TOK_RPR 
      ){
        str_t name = ((tok_t*)(it->next->data))->val;
        tok_t* t = (tok_t*)(it->next->next->data);
        
        def_t* d = (def_t*) mallocx(sizeof(def_t));
        d->name = name;
        d->token = t;

        map_add(defs,name,d);

        goto gouge2;
      }else if (it->next && it->next->next && ((tok_t*)(it->next->next->data))->tag == TOK_RPR ){
        str_t name = ((tok_t*)(it->next->data))->val;

        defs_addbool(defs,name.data,tok->lino);

        goto gouge;
      }else{
        printerr("syntax")("invalid @define.\n");
        goto crash;
      }

    }else if (tok_eq(tok,"@if")){
      if (!it->prev || !it->next || !it->next->next || !it->next->next->next || ((tok_t*)(it->prev->data))->tag != TOK_LPR ){
        printerr("syntax")("line %d: invalid @if.\n",tok->lino);
        goto crash;
      }
      int lvl = 1;
      int n = 4;
      list_node_t* jt = it->next->next->next;
      while (jt){
        tok_t* t = (tok_t*)(jt->data);
        n++;
        if (t->tag == TOK_RPR){
          lvl --;
          if (lvl == 0){
            break;
          }
        }else if (t->tag == TOK_LPR){
          lvl ++;
        }
        jt = jt->next;
      }
      if (lvl != 0){
        printerr("syntax")("line %d: invalid @if (unmatched parenthesis?).\n",tok->lino);
        goto crash;
      }

      tok_t* t0 = (tok_t*)(it->next->data);
      tok_t* t1 = (tok_t*)(it->next->next->data);

      if (t0->tag != TOK_IDT){
        printerr("syntax")("line %d: first param to @if must be an identifier.\n",tok->lino);
        goto crash;
      }

      str_t name = t0->val;
      tok_t* d = def_lookup(name.data,defs);

      if (d && tok_eq(d,t1->val.data)){

        if (jt -> next){
          jt->next->prev = jt->prev;
          jt->prev->next = jt->next;
        }else{
          tokens->tail = jt->prev;
          jt->prev->next = NULL;
        }
        tokens->len --;

        goto gouge;

      }else{

        int jn = !!(jt -> next);

        if (!has_prev && !jn){
          tokens->head = NULL;
          tokens->tail = NULL;
          tokens->len = 0;
          return;
        }else if (!jn){
          tokens->tail = it->prev->prev;
          it->prev->prev->next = NULL;

        }else if (!has_prev){
          tokens->head = jt->next;
          jt->next->prev = NULL;

        }else{
          it->prev->prev->next = jt->next;
          jt->next->prev = it->prev->prev;
        }
        tokens->len -= n;
        it = jt->next;
        continue;
      }
    }else if (tok_eq(tok,"@include")){
      if (!it->prev || !it->next || !it->next->next || ((tok_t*)(it->next->next->data))->tag != TOK_RPR || ((tok_t*)(it->prev->data))->tag != TOK_LPR ){
        printerr("syntax")("line %d: invalid @include.\n",tok->lino);
        goto crash;
      }
      str_t name = ((tok_t*)(it->next->data))->val;

      int is_std;
      if (name.data[0] == '"'){
        name.data[name.len-1]=0;
        is_std = 0;
      }else{
        is_std = 1;
      }

      char* fname;

      if (is_std){
        fname = name.data;
      }else{
        fname = (char*)&name.data[1];
      }
      

      if (!included_lookup(fname,included)){
        #ifdef printinfo
          printinfo("[info] preprocessing '%s' included by '%s'.\n",fname,filename);
        #endif
        str_t inc;

        if (is_std){
          inc = str_new();
          if (str_eq(&name,"math")){
            str_add(&inc, TEXT_math_wax);
          }else{
            printerr("file")("stdlib not found: '%s'.\n",fname);
            goto crash;
          }
        }else{
          inc = read_file_ascii(fname);
        }

        list_t toks = tokenize(inc);
        preprocess(fname, &toks, included, defs);

        if (!toks.len){
          if (!is_std){
            name.data[name.len-1]='"';
          }
          goto gouge;
        }

        if (!has_next && !has_prev){
          tokens->head = toks.head;
          tokens->tail = toks.tail;
          tokens->len = toks.len;
          return;
        }else if (!has_next){
          it->prev->prev->next = toks.head;
          tokens->tail = toks.tail;
          toks.head->prev = it->prev->prev;
        }else if (!has_prev){
          tokens->head = toks.head;
          it->next->next->next->prev = toks.tail;
          toks.tail->next = it->next->next->next;
        }else{
          toks.head->prev = it->prev->prev;
          toks.tail->next = it->next->next->next;
          it->prev->prev->next = toks.head;
          it->next->next->next->prev = toks.tail;
        }

        tokens->len = tokens->len - 4 + toks.len;
        it = it->next->next->next;

        if (!is_std){
          name.data[name.len-1]='"';
        }
      }else{
        if (!is_std){
          name.data[name.len-1]='"';
        }
        goto gouge;
      }

    }else if (tok->val.data[0]=='@'){
      char* name = (char*)&(tok->val.data[1]);
      tok_t* d = def_lookup(name,defs);
      if (!d){
        tok_t* t = (tok_t*) mallocx(sizeof(tok_t));
        t->tag = TOK_INT;
        t->lino = tok->lino;
        t->val = str_from("0",1);
        it->data = t;
      }else{
        it->data = d;
      }
      it = it->next;
      continue;
    }else{
      it = it->next;
      continue;
    }
    goto next;

    gouge:
    {
      if (!has_next && !has_prev){
        tokens->head = NULL;
        tokens->tail = NULL;
        tokens->len = 0;
        return;
      }else if (!has_next){
        it->prev->prev->next = NULL;
        tokens->tail = it->prev->prev;
        
      }else if (!has_prev){
        tokens->head = it->next->next->next;
        it->next->next->next->prev = NULL;
      
      }else{
        it->next->next->next->prev = it->prev->prev;
        it->prev->prev->next = it->next->next->next;
      }

      tokens->len -= 4;
      it = it->next->next->next;
      goto next;
    }

    gouge2:
    {
      if (!has_next2 && !has_prev){
        tokens->head = NULL;
        tokens->tail = NULL;
        tokens->len = 0;
        return;
      }else if (!has_next2){
        it->prev->prev->next = NULL;
        tokens->tail = it->prev->prev;
        
      }else if (!has_prev){
        tokens->head = it->next->next->next->next;
        it->next->next->next->next->prev = NULL;
      
      }else{
        it->next->next->next->next->prev = it->prev->prev;
        it->prev->prev->next = it->next->next->next->next;
      }

      tokens->len -= 5;
      it = it->next->next->next->next;
      goto next;
    }
    next:
    {}
  }

  goto done;

  crash:
  printf("exiting with preprocessor failure.\n");
  freex();exit(1);

  done:
  return;
}






type_t* prim_type(int tag){
  type_t* typ = (type_t*)mallocx(sizeof(type_t));
  typ->tag = tag;
  typ->elem0 = NULL;
  typ->u.elem1 = NULL;
  return typ;
}

expr_t* type_expr(type_t* typ, int lino){
  expr_t* expr = expr_alloc();
  expr->key = EXPR_TYPE;
  expr->rawkey = str_from("",0);

  expr->term = typ;
  expr->lino = lino;
  expr->type = prim_type(TYP_TYP);
  return expr;
}


expr_t* parse_terminal(tok_t* tok){
 
  if ( tok_eq(tok,"int") ){
    return type_expr(prim_type(TYP_INT),tok->lino);
  }else if (tok_eq(tok,"float")){
    return type_expr(prim_type(TYP_FLT),tok->lino);
  }else if (tok_eq(tok,"str")){
    return type_expr(prim_type(TYP_STR),tok->lino);
  }else if (tok_eq(tok,"void")){
    return type_expr(prim_type(TYP_VOD),tok->lino);
  }else if (tok_eq(tok,"map") || tok_eq(tok,"vec") || tok_eq(tok,"arr") || tok_eq(tok,"struct") ){
    printerr("syntax")("line %d: incomplete type: `%s`.\n",tok->lino,tok->val.data);
    return NULL;
  }

  expr_t* expr = expr_alloc();
  expr->key = EXPR_TERM;
  expr->rawkey = str_from("",0);
  expr->term = tok;
  expr->lino = tok->lino;
  return expr;
}

list_t* token_groups(list_t* tokens){
  list_t* groups = list_new_on_heap();

  list_node_t* it = tokens->head;
  while (it && (it->prev != tokens->tail)){
    tok_t* tok = (tok_t*)(it->data);
    if (tok->tag == TOK_LPR){
      int lvl = 0;
      list_t* sub = list_new_on_heap();
      it = it->next;
      sub->head = it;
      sub->len = 0;
      while (it && (it->prev != tokens->tail)){
        
        tok_t* t = (tok_t*)(it->data);
        if (t->tag == TOK_LPR){
          lvl ++;
        }else if (t->tag == TOK_RPR){
          if (lvl == 0){
            lvl--;
            break;
          }
          lvl --;          
        }
        sub->tail = it;
        it = it->next;
        sub->len++;
      }

      if (lvl != -1){
        printerr("syntax")("line %d: unmatched parenthesis\n",tok->lino);
        printf("exiting with syntax tree parsing error.\n");
        freex();exit(1);
      }
      
      list_add(groups,sub);

      it = it->next;
    }else{

      list_t* sub = list_new_on_heap();
      sub->head = it;
      sub->tail = it;
      sub->len = 1;
      list_add(groups,sub);

      it = it->next;
    }
  }
  return groups;
}

type_t* parse_type(list_t* tokens){
  // print_tokens(tokens);
  list_node_t* it = tokens->head;
  tok_t* tok = (tok_t*)(it->data);

  if (tokens->len == 1){
    if ( tok_eq(tok,"int") ){
      return prim_type(TYP_INT);
    }else if (tok_eq(tok,"float")){
      return prim_type(TYP_FLT);
    }else if (tok_eq(tok,"str")){
      return prim_type(TYP_STR);
    }else if (tok_eq(tok,"void")){
      return prim_type(TYP_VOD);
    }else {
      printerr("syntax")("line %d: invalid type: `%s`.\n",tok->lino,tok->val.data);
      return NULL;
    }
  }


  type_t* typ = (type_t*)mallocx(sizeof(type_t));

  tokens->head = it->next;
  list_t* groups = token_groups(tokens);

  if (tok_eq(tok,"map")){
    if (groups->len != 2){
      printerr("syntax")("line %d: map type takes exactly 2 params.\n",tok->lino);
      return NULL;
    }
    list_t* toks0 = (list_t *)(groups->head->data);
    if (toks0->len != 1){
      printerr("syntax")("line %d: map key must be a primitive type.\n",tok->lino);
      return NULL;
    }
    list_t* toks1 = (list_t *)(groups->head->next->data);
    typ->tag = TYP_MAP;
    typ->elem0 = parse_type(toks0);
    if (!typ->elem0){
      return NULL;
    }
    typ->u.elem1 = parse_type(toks1);
    if (!typ->u.elem1){
      return NULL;
    }
  
  }else if (tok_eq(tok,"arr")){
    if (groups->len != 1){
      printerr("syntax")("line %d: arr type takes exactly 1 param.\n",tok->lino);
      return NULL;
    }
    list_t* toks0 = (list_t *)(groups->head->data);
    typ->tag = TYP_ARR;
    typ->elem0 = parse_type(toks0);
    if (!typ->elem0){
      return NULL;
    }

  }else if (tok_eq(tok,"vec")){
    if (groups->len != 2){
      printerr("syntax")("line %d: vec type takes exactly 2 params.\n",tok->lino);
      return NULL;
    }
    list_t* toks0 = (list_t *)(groups->head->data);
    list_t* toks1 = (list_t *)(groups->head->next->data);
    typ->tag = TYP_VEC;
    typ->elem0 = parse_type(toks1);
    if (!typ->elem0){
      return NULL;
    }

    tok_t* tok0 = (tok_t*)(toks0->head->data);
    if (toks0->len != 1 || tok0->tag != TOK_INT){
      printerr("syntax")("line %d: vec size must be a constant int.\n",tok->lino);
      return NULL;
    }
    int size = atoi(tok0->val.data);
    if (size <= 0){
      printerr("syntax")("line %d: vec size must be a positive int (parsed: %d).\n",tok->lino,size);
      return NULL;
    }
    typ->u.size = size;
  }else if (tok_eq(tok,"struct")){
    if (groups->len != 1){
      printerr("syntax")("line %d: invalid struct type name.\n",tok->lino);
      return NULL;
    }
    list_t* toks0 = (list_t *)(groups->head->data);
    tok_t* tok0 = (tok_t*)(toks0->head->data);
    if (toks0->len != 1 || tok0->tag != TOK_IDT){
      printerr("syntax")("line %d: invalid struct type name.\n",tok->lino);
      return NULL;
    }
    typ->tag = TYP_STT;
    typ->u.name = tok0->val;

  }
  return typ;
}



expr_t* parse_expr(list_t* tokens,int level){
  // print_tokens(tokens);
  expr_t* expr = expr_alloc();
  
  list_node_t* it = tokens->head;
  tok_t* tok = (tok_t*)(it->data);

  if (level){
    for (int i = 0; i < NUM_TYPES; i++){
      if ( tok_eq(tok,types[i]) ){
        type_t* typ = parse_type(tokens);
        if (!typ){
          return NULL;
        }
        expr->key = EXPR_TYPE;
        expr->term = typ;
        expr->lino = tok->lino;
        return expr;
      }
    }
  }

  if (tok->tag != TOK_KEY){
      printerr("syntax")("line %d: expression must begin with a keyword, not `%s`.\n",tok->lino,tok->val.data);
      return NULL;
  }
  expr->key = EXPR_TBA;
  expr->rawkey = tok->val;

  tokens->head = it->next;

  list_t* groups = token_groups(tokens);
  it = groups->head;

  while(it){
    list_t* toks = (list_t*)(it->data);

    if (!toks){
      return NULL;
    }
    expr_t* chld;
    // print_tokens(toks);
    if (toks->len == 1 && (toks->head->prev == NULL || ((tok_t*)(toks->head->prev->data))->tag != TOK_LPR )){
      tok_t* tok = (tok_t*)(toks->head->data);
      chld = parse_terminal(tok);
    }else{
      chld = parse_expr(toks,level+1);
    }
    if (!chld){
      return NULL;
    }
    chld->parent = expr;
    list_add(&(expr->children),chld);
    it = it->next;
  }

  expr->lino = ((tok_t*)(tokens->head->data))->lino;
  map_clear(&expr->symtable);
  expr->type = NULL;
  return expr;
}

expr_t* syntax_tree(list_t* tokens){
  expr_t* prgm = expr_alloc();
  prgm->key = EXPR_PRGM;
  prgm->rawkey = str_from("program",7);
  prgm->type = prim_type(TYP_VOD);
  prgm->lino = 0;
  prgm->parent = NULL;

  

  list_t* groups = token_groups(tokens);
  list_node_t* it = groups->head;
  while(it){
    list_t* toks = (list_t*)(it->data);

    if (!toks){
      goto crash;
    }
    if (toks->len == 1){
      tok_t* tok = (tok_t*)(toks->head->data);
      printerr("syntax")("line %d: invalid terminal node at top level `%s`.\n",tok->lino,tok->val.data);
      goto crash;
    }
    expr_t* expr = parse_expr(toks,0);
    if (!expr){
      goto crash;
    }
    expr->parent = prgm;
    list_add(&(prgm->children),expr);
    it = it->next;
  }

  // free_token_groups(groups);

  goto done;

  crash:
  printf("exiting with syntax tree parsing failure.\n");
  freex();exit(1);

  done:
  return prgm;
}

#define PRINT_INDENT(x) for (int i = 0; i < (x); i++){printf("` ");}

void print_type(type_t* typ){
  if (typ == NULL){
    printf("???");
    return;
  }
  printf("%s",types[typ->tag-1]);
  if (typ->tag == TYP_VEC){
    printf("%d",typ->u.size);
    printf("<");
    print_type(typ->elem0);
    printf(">");
  }else if (typ->tag == TYP_ARR){
    printf("<");
    print_type(typ->elem0);
    printf(">");
  }else if (typ->tag == TYP_MAP){
    printf("<");
    print_type(typ->elem0);
    printf(":");
    print_type(typ->u.elem1);
    printf(">");
  }else if (typ->tag == TYP_STT){
    printf(" %s",typ->u.name.data);
  }else if (typ->tag == TYP_TYP){
    printf("<type>");
  }
}


void print_symtable(map_t* symtable, int indent){
  int k = 0;
  printf("     |  |");
  PRINT_INDENT(indent);
  printf("+------symb------+------type------\n");

  while (k < NUM_MAP_SLOTS){

    while (!symtable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!symtable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = symtable->slots[k]->head;

    while (it){
      sym_t* sym = (sym_t*)(it->data);
      printf("     |  |");
      PRINT_INDENT(indent);
      printf("| %-11s#%03d| ",sym->name.data,k);
      print_type(&sym->type);
      printf("\n");
      printf("     |  |");
      PRINT_INDENT(indent);
      printf("+----------------+----------------\n");
      it = it->next;
    }
    k++;
  }
}
void print_stttable(map_t* stttable){
  int k = 0;
  while (k < NUM_MAP_SLOTS){

    while (!stttable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!stttable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = stttable->slots[k]->head;

    while (it){
      stt_t* stt = (stt_t*)(it->data);
      printf("+-------struct-------+\n");
      printf("| %-14s#%03d |\n",stt->name.data,k);
      printf("+--------field-------+--------type--------\n");

      list_node_t* jt = stt->fields.head;

      while (jt){

        sym_t* sym = (sym_t*)(jt->data);
        printf("| %-19s| ",sym->name.data);
        print_type(&sym->type);
        printf("\n");
        printf("+--------------------+--------------------\n");
        jt = jt->next;
      }

      it = it->next;
    }
    k++;
  }
}


void print_functable(map_t* functable){
  int k = 0;
  while (k < NUM_MAP_SLOTS){

    while (!functable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!functable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = functable->slots[k]->head;

    while (it){
      func_t* func = (func_t*)(it->data);
      printf("+--------func--------+-------result-------\n");
      printf("| %-14s#%03d | ",func->name.data,k);
      print_type(func->result);
      printf("\n");
      printf("+--------param-------+--------type--------\n");

      list_node_t* jt = func->params.head;

      while (jt){

        sym_t* sym = (sym_t*)(jt->data);
        printf("| %-19s| ",sym->name.data);
        print_type(&sym->type);
        printf("\n");
        printf("+--------------------+--------------------\n");
        jt = jt->next;
      }

      it = it->next;
    }
    k++;
  }
}


void print_syntax_tree(expr_t* tree, int indent){
  list_t exprs = tree->children;

  printf("%4d ",tree->lino);

  if (tree->children.len){
    printf("[%2d]",tree->children.len);
  }else{
    printf("|  |");
  }
  
  PRINT_INDENT(indent);

  if (tree->key == EXPR_TYPE){
    printf("(type) ");
    print_type((type_t*)tree->term);
    printf("\n");
    return;
  }else if (tree->key == EXPR_TERM){
    tok_t* tok = (tok_t*)tree->term;
    printf("(term) %s '%s' : ",tokens_desc[(int)(tok->tag)],tok->val.data);
    print_type(tree->type);
    printf("\n");
    return;
  }

  printf("%-6s<op%3d> : ",tree->rawkey.data,tree->key);
  print_type(tree->type);
  printf("\n");

  if (tree->symtable.len){
    // printf("%d",tree->symtable.len);
    print_symtable(&tree->symtable,indent);

  }

  list_node_t* it = exprs.head;
  while (it){
    expr_t* expr = (expr_t*)(it->data);
    print_syntax_tree(expr,indent+1);
    it = it->next;
  }

}

sym_t* sym_new(str_t name, type_t typ, char is_local){
  sym_t* sym = (sym_t*)mallocx(sizeof(sym_t));
  sym->name = name;
  sym->type = typ;
  sym->is_local = is_local;
  return sym;
}

type_t* sym_lookup_local(str_t* s, expr_t* expr){
  int k = map_hash(*s);
  if (expr->symtable.slots[k]){
    list_node_t* it = expr->symtable.slots[k]->head;
    while (it){
      sym_t* sym = (sym_t*)(it->data);
      if (str_eq(s,sym->name.data)){
        return &sym->type;
      }
      it = it->next;
    }
  }
  return NULL;
}

func_t* func_lookup(str_t* s, map_t* functable){
  int k = map_hash(*s);
  if (functable->slots[k]){
    list_node_t* it = functable->slots[k]->head;
    while (it){
      func_t* func = (func_t*)(it->data);
      if (str_eq(s,func->name.data)){
        return func;
      }
      it = it->next;
    }
  }
  return NULL;
}

type_t* struct_lookup(str_t* s, str_t* fname, map_t* stttable){
  int k = map_hash(*s);
  if (stttable->slots[k]){
    list_node_t* it = stttable->slots[k]->head;
    while (it){
      stt_t* stt = (stt_t*)(it->data);
      if (str_eq(s,stt->name.data)){
        list_node_t* jt = stt->fields.head;
        while (jt){
          sym_t* sym = (sym_t*)(jt->data);
          if (str_eq(fname,sym->name.data)){
            return &sym->type;
          }
          jt = jt->next;
        }
        return NULL;
      }
      it = it->next;
    }
  }
  return NULL;
}


type_t* sym_lookup(str_t* s, expr_t* expr){
  type_t* typ = sym_lookup_local(s,expr);
  if (typ){
    return typ;
  }
  if (expr->parent){
    return sym_lookup(s,expr->parent);
  }
  return NULL;
}


expr_t* caster(expr_t* expr, type_t* typ){

  type_t* ty0 = expr->type;
  if (ty0->tag == TYP_MAP || ty0->tag == TYP_VEC || ty0->tag == TYP_ARR || ty0->tag == TYP_STT ||
      typ->tag == TYP_MAP || typ->tag == TYP_VEC || typ->tag == TYP_ARR || typ->tag == TYP_STT
    ){
    printerr("syntax")("line %d: cannot implicitly cast from ",expr->lino);
    print_type(ty0);
    printf(" to ");
    print_type(typ);
    printf(".\n");
    freex();exit(1);
  }


  expr_t* ex = expr_alloc();
  ex->key = EXPR_CAST;
  ex->rawkey = str_from("cast",4);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = typ;

  expr_t* tex = type_expr(typ,expr->lino);
  list_add(&ex->children,expr);
  list_add(&ex->children,tex);
  return ex;
}

int type_eq(type_t* t0, type_t* t1){

  if (t0->tag == TYP_INT && t1->tag == TYP_INT){
    return 1;
  }
  if (t0->tag == TYP_FLT && t1->tag == TYP_FLT){
    return 1;
  }
  if (t0->tag == TYP_STR && t1->tag == TYP_STR){
    return 1;
  }
  if (t0->tag == TYP_VOD && t1->tag == TYP_VOD){
    return 1;
  }
  if (t0->tag == TYP_ARR && t1->tag == TYP_ARR){
    return type_eq(t0->elem0,t1->elem0);
  }
  if (t0->tag == TYP_VEC && t1->tag == TYP_VEC){
    return type_eq(t0->elem0,t1->elem0);
  }
  if (t0->tag == TYP_MAP && t1->tag == TYP_MAP){
    return type_eq(t0->elem0,t1->elem0) && type_eq(t0->u.elem1,t1->u.elem1);
  }
  if (t0->tag == TYP_STT && t1->tag == TYP_STT){
    return str_eq(&t0->u.name,t1->u.name.data);
  }
  return 0;
}


int local_need_auto_free(list_node_t* it, expr_t* tree){
  // expr_t* expr = (expr_t*)(it->data);
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (tree->symtable.slots[k]){
      list_node_t* jt = tree->symtable.slots[k]->head;
      while(jt){
        sym_t* sym = (sym_t*)(jt->data);
        if (sym->is_local && (
               sym->type.tag != TYP_INT
            && sym->type.tag != TYP_FLT
          )){
          return 1;
        }
        jt = jt->next;
      }
    }
  }
  if (tree->key != EXPR_FUNCBODY && tree->parent){
    return local_need_auto_free(it,tree->parent);
  }
  return 0;
}

void local_auto_free(void* it, expr_t* tree, int rec){
  expr_t* expr;
  if (rec){
    expr = (expr_t*)(((list_node_t*)it)->data);
  }else{
    expr = (expr_t*)it;
  }
  for (int k = 0; k < NUM_MAP_SLOTS; k++){
    if (tree->symtable.slots[k]){
      list_node_t* jt = tree->symtable.slots[k]->head;
      while(jt){
        sym_t* sym = (sym_t*)(jt->data);
        if (sym->is_local && (
               sym->type.tag != TYP_INT
            && sym->type.tag != TYP_FLT
          )){
          expr_t* ex = expr_alloc();
          ex->key = EXPR_FREE;
          ex->rawkey = str_from("free",4);
          ex->lino = tree->lino;
          ex->parent = tree;
          ex->type = prim_type(TYP_VOD);

          expr_t* ex0 = expr_alloc();
          ex0->key = EXPR_TERM;
          ex0->rawkey = str_from("",0);
          ex0->lino = tree->lino;
          ex0->parent = ex;
          ex0->type = &sym->type;

          tok_t* tok = (tok_t*)mallocx(sizeof(tok_t));
          tok->tag = TOK_IDT;
          tok->val = sym->name;
          ex0->term = tok;


          list_add(&ex->children,ex0);

          if (rec){
            list_insert_l(&expr->parent->children,it,ex);
          }else{
            list_add(&expr->children,ex);
          }
          
        }
        jt = jt->next;
      }
    }
  }
  if (rec){
    if (tree->key != rec && tree->parent){
      local_auto_free(it,tree->parent,rec);
    }
  }
}


tok_t* insert_tmp_var_l(list_node_t* it, expr_t* val){
  expr_t* expr = (expr_t*)(it->data);

  expr_t* ex = expr_alloc();
  ex->key = EXPR_LET;
  ex->rawkey = str_from("let",3);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = prim_type(TYP_VOD);

  expr_t* ex0 = expr_alloc();
  ex0->key = EXPR_TERM;
  ex0->rawkey = str_from("",0);
  ex0->lino = expr->lino;
  ex0->parent = ex;
  ex0->type = val->type;

  tok_t* tok = (tok_t*)mallocx(sizeof(tok_t));
  tok->tag = TOK_IDT;
  tok->val = tmp_name("tmp__var_");
  ex0->term = tok;

  list_add(&ex->children,ex0);

  expr_t* ex1 = type_expr(val->type,expr->lino);
  ex1->parent = ex;

  list_add(&ex->children,ex1);

  val->parent = ex;
  list_add(&ex->children,val);

  ex->parent = expr->parent;
  list_insert_l(&expr->parent->children,it,ex);

  return tok;
}




void compile_syntax_tree(expr_t* tree, map_t* functable, map_t* stttable);

void compile_syntax_tree_node(list_node_t* it, map_t* functable, map_t* stttable){

  expr_t* expr = (expr_t*)(it->data);
  expr_t* tree = expr->parent;

  // print_syntax_tree(expr);
  // printf("-------------\n");

  if (expr->key == EXPR_TYPE){
    return;
  }
  if (expr->key == EXPR_TERM){
    tok_t* tok = (tok_t*)expr->term;
    if (tok->tag == TOK_IDT){
      type_t* typ = sym_lookup(&tok->val,expr);
      if (typ == NULL){
        printerr("syntax")("line %d: identifier '%s' is not defined.\n",expr->lino,tok->val.data);
        goto crash;
      }
      expr->type = typ;
    }else if (tok->tag == TOK_INT){
      expr->type = prim_type(TYP_INT);
    }else if (tok->tag == TOK_FLT){
      expr->type = prim_type(TYP_FLT);
    }else if (tok->tag == TOK_STR){
      expr->type = prim_type(TYP_STR);
    }
  }

  if (str_eq(&expr->rawkey,"let") || str_eq(&expr->rawkey,"local")){
    expr->key = EXPR_LET;
    expr->type = prim_type(TYP_VOD);
    str_t name;
    type_t* typ;
    if ((expr->children).len == 2 || (expr->children).len == 3){
      {
        expr_t* ex = (expr_t*)((expr->children).head->next->data);
        if (ex->key != EXPR_TYPE ){
          printerr("syntax")("line %d: invalid type in '%s' statement.\n",ex->lino,expr->rawkey.data);
          goto crash;
        }
        typ = (type_t*)(ex->term);
      } {
        expr_t* ex = (expr_t*)((expr->children).head->data);
        if (ex->key != EXPR_TERM || ((tok_t*)ex->term)->tag != TOK_IDT ){
          printerr("syntax")("line %d: invalid variable name in '%s' statement.\n",ex->lino,expr->rawkey.data);
          goto crash;
        }
        ex->type = typ;
        name = ((tok_t*)ex->term)->val;
      }
    }else{
      printerr("syntax")("line %d: invalid '%s' statement.\n",expr->lino,expr->rawkey.data);
      goto crash;
    }
    if (sym_lookup_local(&name,tree)){
      printerr("syntax")("line %d: duplicate variable declaration for '%s'.\n",expr->lino,name.data);
      goto crash;
    }

    map_add(&tree->symtable, name, sym_new(name, *typ, str_eq(&expr->rawkey,"local")));

    if ((expr->children).len == 3){

      expr_t* ex = expr_alloc();
      ex->key = EXPR_SET;
      ex->rawkey = str_from("set",3);
      ex->lino = expr->lino;
      ex->parent = expr->parent;
      ex->type = typ;

      list_add(&ex->children, (expr->children).head->data);
      list_add(&ex->children, (expr->children).head->next->next->data);

      list_pop(&(expr->children));
      list_insert_r(&tree->children, it, ex);

    }
    
  }else if (str_eq(&expr->rawkey,"set")){
    if ((expr->children).len != 2 && (expr->children).len != 3){
      printerr("syntax")("line %d: 'set' takes either 2 or 3 params, not %d.\n",expr->lino,(expr->children).len);
      goto crash;
    }
    if ((expr->children).len == 2){
      expr->key = EXPR_SET;

      compile_syntax_tree_node((expr->children).head,functable,stttable);
      compile_syntax_tree_node((expr->children).head->next,functable,stttable);
      
      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

      expr->type = ex0->type;

      if (ex0->type != NULL && ex1->type != NULL){
        if (!type_eq(ex1->type,ex0->type)){
          expr_t* ex2 = caster(ex1,ex0->type);
          list_pop(&(expr->children));
          list_add(&(expr->children),ex2);
        }

      }else{
        printf("[syntax warn] line %d: 'set' cannot parse type of expression.\n",expr->lino);
      }
    }else{


      compile_syntax_tree_node(expr->children.head,functable,stttable);
      compile_syntax_tree_node(expr->children.head->next->next,functable,stttable);
      
      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
      expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);


      if (ex0->type == NULL || ex2->type == NULL){
        printf("[syntax warn] line %d: 'set' cannot parse type of expression.\n",expr->lino);

      }else{

        if (ex0->type->tag == TYP_STT){
          expr->key = EXPR_STRUCTSET;
          if (ex1->children.len != 0 || ((tok_t*)ex1->term)->tag != TOK_IDT ){
            printerr("syntax")("line %d: 'set' expecting a struct field name.\n",expr->lino);
            goto crash;
          }
          type_t* typ = struct_lookup(&ex0->type->u.name,&((tok_t*)ex1->term)->val,stttable);
          if (typ == NULL){
            printerr("syntax")("line %d: struct '%s' has no field '%s' to 'set'.\n",expr->lino,ex0->type->u.name.data,((tok_t*)ex1->term)->val.data);
            goto crash;
          }
          if (!type_eq(typ,ex2->type)){
            printerr("syntax")("line %d: cannot set struct field of type ",expr->lino);
            print_type(typ);
            printf(" with expression of type ");
            print_type(ex2->type);
            printf(".\n");
            goto crash;
          }
          ex1->type = prim_type(TYP_VOD);
          expr->type = typ;


        }else if (ex0->type->tag == TYP_VEC){
          expr->key = EXPR_VECSET;
          
          compile_syntax_tree_node((expr->children).head->next,functable,stttable);
          ex1 = (expr_t*)((expr->children).head->next->data);


          if (ex0->type == NULL || ex1->type == NULL){
            printf("[syntax warn] line %d: 'set' cannot parse type of expression.\n",expr->lino);
          }else{
            type_t* typl = ex0->type->elem0;
            type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

            if (typr->tag != TYP_INT){
              printerr("syntax")("line %d: vec index must be int.\n",expr->lino);
              goto crash;
            }
            if (!type_eq(typl,ex2->type)){
              printerr("syntax")("line %d: cannot set vec element of type ",expr->lino);
              print_type(typl);
              printf(" with expression of type ");
              print_type(ex2->type);
              printf(".\n");
              goto crash;
            }
            expr->type = typl;
          }

        }else if (ex0->type->tag == TYP_ARR){
          expr->key = EXPR_ARRSET;
          
          compile_syntax_tree_node((expr->children).head->next,functable,stttable);
          ex1 = (expr_t*)((expr->children).head->next->data);


          if (ex0->type == NULL || ex1->type == NULL){
            printf("[syntax warn] line %d: 'set' cannot parse type of expression.\n",expr->lino);
          }else{
            type_t* typl = ex0->type->elem0;
            type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

            if (typr->tag != TYP_INT){
              printerr("syntax")("line %d: arr index must be int.\n",expr->lino);
              goto crash;
            }
            if (!type_eq(typl,ex2->type)){
              printerr("syntax")("line %d: cannot set arr element of type ",expr->lino);
              print_type(typl);
              printf(" with expression of type ");
              print_type(ex2->type);
              printf(".\n");
              goto crash;
            }
            expr->type = typl;
          }


        }else if (ex0->type->tag == TYP_MAP){
          expr->key = EXPR_MAPSET;
          
          compile_syntax_tree_node((expr->children).head->next,functable,stttable);
          ex1 = (expr_t*)((expr->children).head->next->data);


          if (ex0->type == NULL || ex1->type == NULL){
            printf("[syntax warn] line %d: 'set' cannot parse type of expression.\n",expr->lino);
          }else{
            type_t* typl = ex0->type->elem0;
            type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

            if (!type_eq(typl,typr)){
              printerr("syntax")("line %d: map 'set' expecting ",expr->lino);
              print_type(typl);
              printf(", got ");
              print_type(typr);
              printf(".\n");
              goto crash;
            }
            expr->type = ex0->type->u.elem1;
          }

        }else{
          printerr("syntax")("line %d: cannot use 'set' on \n",expr->lino);
          print_type(ex0->type);
          printf(" type.\n");
          goto crash;
        }


      }

      // if (!type_eq(expr->type,ex2->type)){
      //   expr_t* ex3 = caster(ex2,expr->type);
      //   list_pop(&(expr->children));
      //   list_add(&(expr->children),ex3);
      // }
    }


  }else if (str_eq(&expr->rawkey,"get")){
    expr_t* ptr = expr;
    if ((expr->children).len > 2){

      expr_t* ex = expr_alloc();
      ex->key = EXPR_TBA;
      ex->rawkey = str_from("get",3);
      ex->lino = expr->lino;
      ex->parent = expr->parent;
      expr->parent = ex;

      ptr = ex;
      list_add(&ex->children, expr);
      list_add(&ex->children, (expr->children).tail->data);

      list_pop(&(expr->children));
      it->data = ex;

    }
    expr = ptr;
    

    compile_syntax_tree_node(expr->children.head,functable,stttable);
    
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

    if (ex0->type == NULL){
      printf("[syntax warn] line %d: 'get' cannot parse type of expression.\n",expr->lino);

    }else{

      if (ex0->type->tag == TYP_STT){
        expr->key = EXPR_STRUCTGET;
        if (ex1->children.len != 0 || ((tok_t*)ex1->term)->tag != TOK_IDT ){
          printerr("syntax")("line %d: 'get' expecting a struct field name.\n",expr->lino);
          goto crash;
        }
        type_t* typ = struct_lookup(&ex0->type->u.name,&((tok_t*)ex1->term)->val,stttable);
        if (typ == NULL){
          printerr("syntax")("line %d: struct '%s' has no field '%s' to 'get'.\n",expr->lino,ex0->type->u.name.data,((tok_t*)ex1->term)->val.data);
          goto crash;
        }

        ex1->type = prim_type(TYP_VOD);
        expr->type = typ;

      }else if (ex0->type->tag == TYP_VEC){
        expr->key = EXPR_VECGET;
        
        compile_syntax_tree_node((expr->children).head->next,functable,stttable);
        ex1 = (expr_t*)((expr->children).head->next->data);


        if (ex0->type == NULL || ex1->type == NULL){
          printf("[syntax warn] line %d: 'get' cannot parse type of expression.\n",expr->lino);
        }else{
          type_t* typl = ex0->type->elem0;
          type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

          if (typr->tag != TYP_INT){
            printerr("syntax")("line %d: vec index must be int.\n",expr->lino);
            goto crash;
          }
          expr->type = typl;
        }

      }else if (ex0->type->tag == TYP_ARR){
        expr->key = EXPR_ARRGET;
        
        compile_syntax_tree_node((expr->children).head->next,functable,stttable);
        ex1 = (expr_t*)((expr->children).head->next->data);


        if (ex0->type == NULL || ex1->type == NULL){
          printf("[syntax warn] line %d: 'get' cannot parse type of expression.\n",expr->lino);
        }else{
          type_t* typl = ex0->type->elem0;
          type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

          if (typr->tag != TYP_INT){
            printerr("syntax")("line %d: arr index must be int.\n",expr->lino);
            goto crash;
          }
          expr->type = typl;
        }

      }else if (ex0->type->tag == TYP_STR){
        expr->key = EXPR_STRGET;
        
        compile_syntax_tree_node((expr->children).head->next,functable,stttable);
        ex1 = (expr_t*)((expr->children).head->next->data);


        if (ex0->type == NULL || ex1->type == NULL){
          printf("[syntax warn] line %d: 'get' cannot parse type of expression.\n",expr->lino);
        }else{
          type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

          if (typr->tag != TYP_INT){
            printerr("syntax")("line %d: str index must be int.\n",expr->lino);
            goto crash;
          }
          expr->type = prim_type(TYP_INT);
        }

      }else if (ex0->type->tag == TYP_MAP){
        expr->key = EXPR_MAPGET;
        
        compile_syntax_tree_node((expr->children).head->next,functable,stttable);
        ex1 = (expr_t*)((expr->children).head->next->data);


        if (ex0->type == NULL || ex1->type == NULL){
          printf("[syntax warn] line %d: 'get' cannot parse type of expression.\n",expr->lino);
        }else{
          type_t* typl = ex0->type->elem0;
          type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

          if (!type_eq(typl,typr)){
            printerr("syntax")("line %d: map 'get' expecting ",expr->lino);
            print_type(typl);
            printf(", got ");
            print_type(typr);
            printf(".\n");
            goto crash;
          }
          expr->type = ex0->type->u.elem1;
        }

      }else{
        printerr("syntax")("line %d: cannot use 'get' on \n",expr->lino);
        print_type(ex0->type);
        printf(" type.\n");
        goto crash;
      }
    }

  }else if (str_eq(&expr->rawkey,"struct")){
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr->key = EXPR_STRUCT;
    expr->type = prim_type(TYP_VOD);
    type_t* typ = (type_t*)mallocx(sizeof(type_t));
    typ->tag = TYP_STT;
    
    if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
      printerr("syntax")("line %d: struct name must be an identifier.\n",ex0->lino);
      goto crash;
    }

    stt_t* stt = (stt_t*)mallocx(sizeof(stt_t));
    stt->name = ((tok_t*)ex0->term)->val;
    list_init(&stt->fields);

    map_add(stttable,stt->name,stt);

    list_node_t* jt = ((expr->children).head->next);
    while (jt){
      expr_t* ex = (expr_t*)(jt->data);
      if (!str_eq(&ex->rawkey,"let") ){
        printerr("syntax")("line %d: invalid `%s` inside struct (expecting `let`).\n",ex0->lino,ex->rawkey.data);
        goto crash;
      }
      if (ex->children.len != 2){
        printerr("syntax")("line %d: struct field takes exactly 2 params (let name type).\n",ex0->lino);
        goto crash;
      }
      expr_t* e0 = (expr_t*)(ex->children.head->data);
      expr_t* e1 = (expr_t*)(ex->children.head->next->data);
      if (e0->key != EXPR_TERM || ((tok_t*)e0->term)->tag != TOK_IDT ){
        printerr("syntax")("line %d: invalid field name.\n",e0->lino);
        goto crash;
      }
      if (e1->key != EXPR_TYPE){
        printerr("syntax")("line %d: invalid field type.\n",e1->lino);
        goto crash;
      }
      str_t fname = ((tok_t*)e0->term)->val;
      type_t* ftyp = ((type_t*)e1->term);

      list_add(&stt->fields,sym_new(fname,*ftyp,0));
      e0->type = ftyp;
      ex->type = prim_type(TYP_VOD);
      ex->key = EXPR_STRUCTFLD;
      jt = jt->next;
    }
    typ->u.name = ((tok_t*)ex0->term)->val;
    ex0->type = typ;      

  }else if (str_eq(&expr->rawkey,"func")){

    if (expr->children.len < 1){
      printerr("syntax")("line %d: 'func' must at least have a name.\n",expr->lino);
      goto crash;
    }

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr->key = EXPR_FUNC;
    expr->type = prim_type(TYP_VOD);
    ex0->type = prim_type(TYP_VOD);
    
    if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
      printerr("syntax")("line %d: function name must be an identifier.\n",ex0->lino);
      goto crash;
    }

    func_t* func = (func_t*)mallocx(sizeof(func_t));
    func->name = ((tok_t*)ex0->term)->val;
    list_init(&func->params);

    map_add(functable,func->name,func);

    list_node_t* jt = ((expr->children).head->next);
    while (jt){
      expr_t* ex = (expr_t*)(jt->data);
      if (!str_eq(&ex->rawkey,"param")){
        break;
      }
      if (ex->children.len != 2){
        printerr("syntax")("line %d: 'param' keyword takes exactly 2 params (param name type).\n",ex0->lino);
        goto crash;
      }
      expr_t* e0 = (expr_t*)(ex->children.head->data);
      expr_t* e1 = (expr_t*)(ex->children.head->next->data);
      if (e0->key != EXPR_TERM || ((tok_t*)e0->term)->tag != TOK_IDT ){
        printerr("syntax")("line %d: invalid param name.\n",e0->lino);
        goto crash;
      }
      if (e1->key != EXPR_TYPE){
        printerr("syntax")("line %d: invalid param type.\n",e1->lino);
        goto crash;
      }
      str_t pname = ((tok_t*)e0->term)->val;
      type_t* ptyp = ((type_t*)e1->term);

      list_add(&func->params,sym_new(pname,*ptyp,0));
      e0->type = ptyp;
      ex->type = prim_type(TYP_VOD);
      ex->key = EXPR_PARAM;

      map_add(&expr->symtable, pname, sym_new(pname, *ptyp, 0));

      jt = jt->next;
    }

    expr_t* ex = NULL;
    if (jt){
      ex = (expr_t*)(jt->data);
    }

    if (ex && str_eq(&ex->rawkey,"result")){
      if (ex->children.len != 1){
        printerr("syntax")("line %d: 'result' keyword takes exactly 1 param (result type).\n",ex0->lino);
        goto crash;
      }
      expr_t* e0 = (expr_t*)(ex->children.head->data);
      if (e0->key != EXPR_TYPE){
        printerr("syntax")("line %d: invalid result type.\n",e0->lino);
        goto crash;
      }
      type_t* rtyp = ((type_t*)e0->term);

      e0->type = rtyp;
      ex->type = prim_type(TYP_VOD);
      ex->key = EXPR_RESULT;
      func->result = rtyp;
      jt = jt->next;
    }else{
      func->result = prim_type(TYP_VOD);
    }

    if (jt){

      expr_t* fexpr = expr_alloc();
      fexpr->key = EXPR_FUNCBODY;
      fexpr->rawkey = str_from("funcbody",8);
      fexpr->term = NULL;
      fexpr->lino = expr->lino;
      fexpr->type = prim_type(TYP_VOD);
      fexpr->parent = expr;
      int nrem = 0;

      while(jt){
        expr_t* ex = (expr_t*)(jt->data);
        ex->parent = fexpr;
        list_add(&fexpr->children,ex);
        nrem ++;
        jt = jt->next;
      }
      for (int i = 0; i < nrem; i++){
        list_pop(&(expr->children));
      }
      list_add(&(expr->children),fexpr);
      compile_syntax_tree(fexpr,functable,stttable);
    }else{
      expr->key = EXPR_FUNCHEAD;
    }

  }else if (str_eq(&expr->rawkey,"extern")){
    
    if (expr->children.len < 1){
      printerr("syntax")("line %d: 'extern' must at least have a name.\n",expr->lino);
      goto crash;
    }

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr->key = EXPR_EXTERN;
    expr->type = prim_type(TYP_VOD);
    ex0->type = prim_type(TYP_VOD);
    
    if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
      printerr("syntax")("line %d: 'extern' function name must be an identifier.\n",ex0->lino);
      goto crash;
    }


    if (expr->children.len == 2 && ((expr_t*)((expr->children).head->next->data))->key == EXPR_TYPE){
      
      map_add(&tree->symtable, ((tok_t*)ex0->term)->val, sym_new(((tok_t*)ex0->term)->val, *(type_t*)(((expr_t*)((expr->children).head->next->data))->term), 0));


    }else{
      

      func_t* func = (func_t*)mallocx(sizeof(func_t));
      func->name = ((tok_t*)ex0->term)->val;
      list_init(&func->params);

      map_add(functable,func->name,func);

      list_node_t* jt = ((expr->children).head->next);
      while (jt){
        expr_t* ex = (expr_t*)(jt->data);
        if (!str_eq(&ex->rawkey,"param")){
          break;
        }
        if (ex->children.len != 2){
          printerr("syntax")("line %d: 'param' keyword takes exactly 2 params (param name type).\n",ex0->lino);
          goto crash;
        }
        expr_t* e0 = (expr_t*)(ex->children.head->data);
        expr_t* e1 = (expr_t*)(ex->children.head->next->data);
        if (e0->key != EXPR_TERM || ((tok_t*)e0->term)->tag != TOK_IDT ){
          printerr("syntax")("line %d: invalid param name.\n",e0->lino);
          goto crash;
        }
        if (e1->key != EXPR_TYPE){
          printerr("syntax")("line %d: invalid param type.\n",e1->lino);
          goto crash;
        }
        str_t pname = ((tok_t*)e0->term)->val;
        type_t* ptyp = ((type_t*)e1->term);

        list_add(&func->params,sym_new(pname,*ptyp ,0));
        e0->type = ptyp;
        ex->type = prim_type(TYP_VOD);
        ex->key = EXPR_PARAM;

        map_add(&expr->symtable, pname, sym_new(pname, *ptyp ,0));

        jt = jt->next;
      }
      if (jt){
        expr_t* ex = (expr_t*)(jt->data);
        if (str_eq(&ex->rawkey,"result")){
          if (ex->children.len != 1){
            printerr("syntax")("line %d: 'result' keyword takes exactly 1 param (result type).\n",ex0->lino);
            goto crash;
          }
          expr_t* e0 = (expr_t*)(ex->children.head->data);
          if (e0->key != EXPR_TYPE){
            printerr("syntax")("line %d: invalid result type.\n",e0->lino);
            goto crash;
          }
          type_t* rtyp = ((type_t*)e0->term);

          e0->type = rtyp;
          ex->type = prim_type(TYP_VOD);
          ex->key = EXPR_RESULT;
          func->result = rtyp;
          jt = jt->next;
        }else{
          func->result = prim_type(TYP_VOD);
        }
      }else{
        func->result = prim_type(TYP_VOD);
      }
      
    }

  }else if (str_eq(&expr->rawkey,"=") || str_eq(&expr->rawkey,"<>")){
    if ((expr->children).len != 2){
      printerr("syntax")("line %d: '%s' takes exactly 2 params.\n",expr->lino, expr->rawkey.data);
      goto crash;
    }

    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

    int intver;
    int fltver;
    int strver;
    int ptrver;
    if (str_eq(&expr->rawkey,"=")){
      intver = EXPR_IEQ; 
      fltver = EXPR_FEQ;
      strver = EXPR_STREQL;
      ptrver = EXPR_PTREQL;
    }else{
      intver = EXPR_INEQ; 
      fltver = EXPR_FNEQ;
      strver = EXPR_STRNEQ;
      ptrver = EXPR_PTRNEQ;
    }
    expr->type = prim_type(TYP_INT);

    if (ex0->type != NULL && ex1->type != NULL){

      if (ex0->type->tag == TYP_FLT && ex1->type->tag == TYP_INT){
        expr_t* ex2 = caster(ex1,ex0->type);
        list_pop(&(expr->children));
        list_add(&(expr->children),ex2);
        expr->key = fltver;

      }else if (ex0->type->tag == TYP_INT && ex1->type->tag == TYP_FLT){

        expr_t* ex2 = caster(ex0,ex1->type);

        list_pop(&expr->children);
        list_pop(&expr->children);
        list_add(&(expr->children),ex2);
        list_add(&(expr->children),ex1);

        expr->key = fltver;

      }else if (ex0->type->tag == TYP_INT && ex1->type->tag == TYP_INT){
        expr->key = intver;

      }else if (ex0->type->tag == TYP_FLT && ex1->type->tag == TYP_FLT){
        expr->key = fltver;

      }else if (ex0->type->tag == TYP_STR && ex1->type->tag == TYP_STR){
        expr->key = strver;

      }else if (ex0->type->tag == TYP_STR){
        expr_t* ex2 = caster(ex1,ex0->type);
        list_pop(&(expr->children));
        list_add(&(expr->children),ex2);
        expr->key = strver;

      }else if (ex1->type->tag == TYP_STR){
        expr_t* ex2 = caster(ex0,ex1->type);
        list_pop(&(expr->children));
        list_pop(&(expr->children));
        list_add(&(expr->children),ex2);
        list_add(&(expr->children),ex1);
        expr->key = strver;

      }else if (type_eq(ex0->type,ex1->type)){
        expr->key = ptrver;
        
      }else{
        printerr("syntax")("line %d: '%s' operator not defined on types ",expr->lino,expr->rawkey.data);
        print_type(ex0->type);
        printf(" and ");
        print_type(ex1->type);
        printf(".\n");
        goto crash;
      }
    }else{
      printf("[syntax warn] line %d: '%s' cannot parse type of expression.\n",expr->lino,expr->rawkey.data);
    }




  }else if (str_eq(&expr->rawkey,"+")||
            str_eq(&expr->rawkey,"-")||str_eq(&expr->rawkey,"*")||str_eq(&expr->rawkey,"/" )||str_eq(&expr->rawkey,"%" )||\
            str_eq(&expr->rawkey,">")||str_eq(&expr->rawkey,"<")||str_eq(&expr->rawkey,">=")||str_eq(&expr->rawkey,"<=")){


    expr_t* ex0 = (expr_t*)((expr->children).head->data);

    if (str_eq(&expr->rawkey,"+") || str_eq(&expr->rawkey,"*")){
      while ((expr->children).len > 2){
        expr_t* ex = expr_alloc();
        ex->key = EXPR_TBA;
        ex->rawkey = str_from(expr->rawkey.data,1);
        ex->lino = expr->lino;
        ex->parent = expr->parent;
        list_add(&ex->children, (expr->children).head->data);
        list_add(&ex->children, (expr->children).head->next->data);
        list_pophead(&expr->children);
        expr->children.head->data = ex;
      }
    }

    int did_comp_ex0 = 0;
    if (str_eq(&expr->rawkey,"-") && (expr->children).len == 1){
      expr_t* ex = expr_alloc();
      ex->key = EXPR_TERM;
      ex->rawkey = str_from("",0);
      ex->lino = expr->lino;
      ex->parent = expr->parent;
      
      compile_syntax_tree_node((expr->children).head,functable,stttable);
      
      if (ex0->type == NULL){
        printf("[syntax warn] line %d: '%s' cannot parse type of expression.\n",expr->lino,expr->rawkey.data);
      }else{
        ex->type = ex0->type;
      }
      
      tok_t* tok = (tok_t*)mallocx(sizeof(tok_t));
      tok->tag = ex->type->tag == TYP_INT ? TOK_INT : TOK_FLT;
      tok->val = ex->type->tag == TYP_INT ? str_from("0",1) : str_from("0.0",3);
      ex->term = tok;

      list_pop(&expr->children);
      list_add(&expr->children,ex);
      list_add(&expr->children,ex0);
    }

    if ((expr->children).len != 2){
      printerr("syntax")("line %d: '%s' takes 2 params.\n",expr->lino, expr->rawkey.data);
      goto crash;
    }

    ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
    if (!did_comp_ex0){
      compile_syntax_tree_node((expr->children).head,functable,stttable);
    }
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    
    ex0 = (expr_t*)((expr->children).head->data);
    ex1 = (expr_t*)((expr->children).head->next->data);

    int intver = 0;
    int fltver = 0;
    int ispred = 0;
         if (str_eq(&expr->rawkey,"+" )){ intver = EXPR_IADD; fltver = EXPR_FADD;}
    else if (str_eq(&expr->rawkey,"-" )){ intver = EXPR_ISUB; fltver = EXPR_FSUB;}
    else if (str_eq(&expr->rawkey,"*" )){ intver = EXPR_IMUL; fltver = EXPR_FMUL;}
    else if (str_eq(&expr->rawkey,"/" )){ intver = EXPR_IDIV; fltver = EXPR_FDIV;}
    else if (str_eq(&expr->rawkey,"%" )){ intver = EXPR_IMOD; fltver = EXPR_FMOD;}
    else if (str_eq(&expr->rawkey,">" )){ intver = EXPR_IGT;  fltver = EXPR_FGT; ispred=1;}
    else if (str_eq(&expr->rawkey,"<" )){ intver = EXPR_ILT;  fltver = EXPR_FLT; ispred=1;}
    else if (str_eq(&expr->rawkey,">=")){ intver = EXPR_IGEQ; fltver = EXPR_FGEQ;ispred=1;}
    else if (str_eq(&expr->rawkey,"<=")){ intver = EXPR_ILEQ; fltver = EXPR_FLEQ;ispred=1;}

    if (ex0->type != NULL && ex1->type != NULL){

      if (ex0->type->tag == TYP_FLT && ex1->type->tag == TYP_INT){
        expr_t* ex2 = caster(ex1,ex0->type);
        list_pop(&(expr->children));
        list_add(&(expr->children),ex2);
        expr->key = fltver;
        expr->type = ispred?prim_type(TYP_INT):(ex0->type);

      }else if (ex0->type->tag == TYP_INT && ex1->type->tag == TYP_FLT){

        expr_t* ex2 = caster(ex0,ex1->type);

        list_pop(&expr->children);
        list_pop(&expr->children);
        list_add(&(expr->children),ex2);
        list_add(&(expr->children),ex1);

        expr->key = fltver;
        expr->type = ispred?prim_type(TYP_INT):(ex1->type);

      }else if (ex0->type->tag == TYP_INT && ex1->type->tag == TYP_INT){
        expr->key = intver;
        expr->type = ispred?prim_type(TYP_INT):(ex0->type);
      }else if (ex0->type->tag == TYP_FLT && ex1->type->tag == TYP_FLT){
        expr->key = fltver;
        expr->type = ispred?prim_type(TYP_INT):(ex0->type);
      }else{
        printerr("syntax")("line %d: '%s' operator not defined on types ",expr->lino,expr->rawkey.data);
        print_type(ex0->type);
        printf(" and ");
        print_type(ex1->type);
        printf(".\n");
        goto crash;
      }
    }else{
      printf("[syntax warn] line %d: '%s' cannot parse type of expression.\n",expr->lino,expr->rawkey.data);
    }

  }else if (str_eq(&expr->rawkey,"||")||str_eq(&expr->rawkey,"&&")||str_eq(&expr->rawkey,"<<")||str_eq(&expr->rawkey,">>")\
          ||str_eq(&expr->rawkey,"^") ||str_eq(&expr->rawkey,"&") ||str_eq(&expr->rawkey,"|")){

    if (str_eq(&expr->rawkey,"&&") || str_eq(&expr->rawkey,"||")){
      while ((expr->children).len > 2){
        expr_t* ex = expr_alloc();
        ex->key = EXPR_TBA;
        ex->rawkey = str_from(expr->rawkey.data,2);
        ex->lino = expr->lino;
        ex->parent = expr->parent;
        list_add(&ex->children, (expr->children).head->data);
        list_add(&ex->children, (expr->children).head->next->data);
        list_pophead(&expr->children);
        expr->children.head->data = ex;
      }
    }

    if ((expr->children).len != 2){
      printerr("syntax")("line %d: '%s' takes exactly 2 params.\n",expr->lino, expr->rawkey.data);
      goto crash;
    }

    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

    if (ex0->type != NULL && ex1->type != NULL){

      if (ex0->type->tag == TYP_INT && ex1->type->tag == TYP_INT){
             if (str_eq(&expr->rawkey,"||" )){ expr->key = EXPR_LOR;}
        else if (str_eq(&expr->rawkey,"&&" )){ expr->key = EXPR_LAND;}
        else if (str_eq(&expr->rawkey,"<<" )){ expr->key = EXPR_SHL;}
        else if (str_eq(&expr->rawkey,">>" )){ expr->key = EXPR_SHR;}
        else if (str_eq(&expr->rawkey,"^"  )){ expr->key = EXPR_XOR;}
        else if (str_eq(&expr->rawkey,"&"  )){ expr->key = EXPR_BAND;}
        else if (str_eq(&expr->rawkey,"|"  )){ expr->key = EXPR_BOR;}

        expr->type = ex0->type;

      }else if (ex0->type->tag == TYP_STR && str_eq(&expr->rawkey,"<<")){
        if (ex0->key != EXPR_TERM || ((tok_t*)(ex0->term))->tag != TOK_IDT ){
          printerr("syntax")("line %d: left hand side of str << must be an identifier. (Assign r-value to a variable first).\n",expr->lino);
          goto crash;
        }
        if (ex1->type->tag == TYP_INT){
          expr->key = EXPR_STRADD;
        } else {
          expr->key = EXPR_STRCAT;
          if (ex1->type->tag != TYP_STR){
            expr_t* ex2 = caster(ex1,ex0->type);
            list_pop(&(expr->children));
            list_add(&(expr->children),ex2);
          }
        }
        expr->type = ex0->type;

      }else{
        printerr("syntax")("line %d: '%s' operator not defined on types ",expr->lino,expr->rawkey.data);
        print_type(ex0->type);
        printf(" and ");
        print_type(ex1->type);
        printf(".\n");
        goto crash;
      }
    }else{
      printf("[syntax warn] line %d: '%s' cannot parse type of expression.\n",expr->lino,expr->rawkey.data);
    }

  }else if (str_eq(&expr->rawkey,"~")||str_eq(&expr->rawkey,"!")){

    if ((expr->children).len != 1){
      printerr("syntax")("line %d: '%s' takes exactly 1 param.\n",expr->lino, expr->rawkey.data);
      goto crash;
    }
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    if (ex0->type != NULL){

      if (ex0->type->tag == TYP_INT){
             if (str_eq(&expr->rawkey,"~" )){ expr->key = EXPR_BNEG;}
        else if (str_eq(&expr->rawkey,"!" )){ expr->key = EXPR_LNOT;}

        expr->type = ex0->type;
      }else{
        printerr("syntax")("line %d: '%s' operator not defined on type ",expr->lino,expr->rawkey.data);
        print_type(ex0->type);
        printf(".\n");
        goto crash;
      }
    }else{
      printf("[syntax warn] line %d: '%s' cannot parse type of expression.\n",expr->lino,expr->rawkey.data);
    }

  }else if (str_eq(&expr->rawkey,"return")){

    expr_t* ex = expr->parent;
    while (ex->key != EXPR_FUNC){
      ex = ex->parent;
    }

    str_t s = ((tok_t*)(((expr_t*)ex->children.head->data)->term))->val;
    
    type_t* rtyp = func_lookup(&s,functable)->result;

    expr->key = EXPR_RETURN;
    expr->type = prim_type(TYP_VOD);

    if (expr->children.len > 1){
      printerr("syntax")("line %d: '%s' takes at most 1 param.\n",expr->lino, expr->rawkey.data);
      goto crash;

    }else if (expr->children.len == 0){
      if (rtyp->tag != TYP_VOD){
        printerr("syntax")("line %d: wrong return type: expecting ",expr->lino);
        print_type(rtyp);
        printf(", got nothing.\n");
        goto crash;
      }
    }else{
      compile_syntax_tree_node(expr->children.head,functable,stttable);

      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      if (ex0->type){
        if (!type_eq(ex0->type,rtyp)){
          printerr("syntax")("line %d: wrong return type: expecting ",expr->lino);
          print_type(rtyp);
          printf(", got ");
          print_type(ex0->type);
          printf(".\n");
          goto crash;
        }
      }else{
        printf("[syntax warn] line %d: 'return' cannot parse type of expression.\n",expr->lino);
      }

    }


    if (local_need_auto_free(it,tree)){


      if (expr->children.len && ((expr_t*)(expr->children.head->data))->key != EXPR_TERM ){
        tok_t* t = insert_tmp_var_l(it, (expr_t*)((expr->children).head->data));
        compile_syntax_tree_node(it->prev,functable,stttable);

        expr_t* ex = expr_alloc();
        ex->key = EXPR_TERM;
        ex->rawkey = str_from("",0);
        ex->lino = expr->lino;
        ex->parent = expr;
        ex->type = rtyp;
        ex->term = t;

        expr->children.head->data = ex;

      }
      local_auto_free(it,tree,EXPR_FUNCBODY);
    }


  }else if (str_eq(&expr->rawkey,"call")){

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr->key = EXPR_CALL;

    ex0->type = prim_type(TYP_VOD);
    
    if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
      printerr("syntax")("line %d: 'call' function name must be an identifier.\n",ex0->lino);
      goto crash;
    }

    func_t* func = func_lookup(&((tok_t*)ex0->term)->val,functable);
    if (func == NULL){
      printerr("syntax")("line %d: function to be 'call'ed is not defined: %s.\n",ex0->lino,((tok_t*)ex0->term)->val.data);
      goto crash;

    }
    expr->type = func->result;

    list_node_t* jt = ((expr->children).head->next);
    list_node_t* kt = (func->params.head);
    while (jt||kt){
      if (!kt){
        printerr("syntax")("line %d: too many arguments to function call, expecting %d.\n",ex0->lino,func->params.len);
        goto crash;
      }
      if (!jt){
        printerr("syntax")("line %d: too few arguments to function call, expecting %d.\n",ex0->lino,func->params.len);
        goto crash;
      }
      compile_syntax_tree_node(jt,functable,stttable);
      expr_t* ex = (expr_t*)(jt->data);
      sym_t* sym = (sym_t*)(kt->data);

      if (ex->type == NULL ){
        printf("[syntax warn] line %d: 'call' cannot parse type of expression.\n",expr->lino);
      }else{
        if (!type_eq(ex->type,&sym->type)){
          expr_t* ex2 = caster(ex,&sym->type);
          jt->data = ex2;
        }
      }

      jt = jt->next;
      kt = kt->next;
    }

  }else if (str_eq(&expr->rawkey,"if")){
    if (expr->children.len != 2 && expr->children.len != 3){
      printerr("syntax")("line %d: 'if' must have either 2 or 3 parts, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_IF;
    expr->type = prim_type(TYP_VOD);
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

    if (str_eq(&ex1->rawkey, "then") ){
      ex1->key = EXPR_THEN;
      ex1->type = prim_type(TYP_VOD);
    }else{
      printerr("syntax")("line %d: the second part of 'if' must be 'then', not '%s'.\n",expr->lino,ex1->rawkey.data);
      goto crash;
    }
    compile_syntax_tree((expr_t*)((expr->children).head->next->data),functable,stttable);

    if (expr->children.len == 3){

      expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);
      if (str_eq(&ex2->rawkey, "else") ){
        ex2->key = EXPR_ELSE;
        ex2->type = prim_type(TYP_VOD);
      }else{
        printerr("syntax")("line %d: the third part of 'if' must be 'else', not '%s'.\n",expr->lino,ex2->rawkey.data);
        goto crash;
      }

      compile_syntax_tree((expr_t*)((expr->children).head->next->next->data),functable,stttable);
    }

  }else if (str_eq(&expr->rawkey,"for")){

    if (expr->children.len != 4 && expr->children.len != 5){
      printerr("syntax")("line %d: 'for' must have 4 or 5 parts, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }


    if (expr->children.len == 5){

      expr->key = EXPR_FOR;
      expr->type = prim_type(TYP_VOD);

      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
        printerr("syntax")("line %d: invalid looper name in 'for' statement.\n",expr->lino);
        goto crash;
      }

      ex0->type = prim_type(TYP_INT);
      str_t name = ((tok_t*)ex0->term)->val;
      type_t* typ = prim_type(TYP_INT);
      map_add(&expr->symtable, name, sym_new(name, *typ ,0));


      compile_syntax_tree_node((expr->children).head->next,functable,stttable);
      compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);
      compile_syntax_tree_node((expr->children).head->next->next->next,functable,stttable);

      if  (((expr_t*)((expr->children).head->next->data))->type == NULL 
        || ((expr_t*)((expr->children).head->next->next->data))->type == NULL 
        || ((expr_t*)((expr->children).head->next->next->next->data))->type == NULL ){
        printf("[syntax warn] line %d: 'for' cannot parse type of expression.\n",expr->lino);
      }else{
        if  (((expr_t*)((expr->children).head->next->data))->type->tag != TYP_INT 
          || ((expr_t*)((expr->children).head->next->next->data))->type->tag != TYP_INT 
          || ((expr_t*)((expr->children).head->next->next->next->data))->type->tag != TYP_INT){
          printerr("syntax")("line %d: begin, end and step in 'for' must be ints, got ",expr->lino);
          print_type(((expr_t*)((expr->children).head->next->data))->type);
          printf(", ");
          print_type(((expr_t*)((expr->children).head->next->next->data))->type);
          printf(" and ");
          print_type(((expr_t*)((expr->children).head->next->next->next->data))->type);
          printf(".\n");
          goto crash;
        }

      }
      
      expr_t* ex1 = (expr_t*)((expr->children).tail->data);

      if (str_eq(&ex1->rawkey, "do") ){
        ex1->key = EXPR_DO;
        ex1->type = prim_type(TYP_VOD);
      }else{
        printerr("syntax")("line %d: the last part of 'for' must be 'do', not '%s'.\n",expr->lino,ex1->rawkey.data);
        goto crash;
      }
      compile_syntax_tree((expr_t*)((expr->children).tail->data),functable,stttable);


    }else{
      expr->key = EXPR_FORIN;
      expr->type = prim_type(TYP_VOD);

      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      if (ex0->key != EXPR_TERM || ((tok_t*)ex0->term)->tag != TOK_IDT ){
        printerr("syntax")("line %d: invalid key name in 'for' statement.\n",expr->lino);
        goto crash;
      }
      expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
      if (ex1->key != EXPR_TERM || ((tok_t*)ex1->term)->tag != TOK_IDT ){
        printerr("syntax")("line %d: invalid value name in 'for' statement.\n",expr->lino);
        goto crash;
      }

      compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);
      expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);

      if (ex2->type->tag != TYP_MAP){
        printerr("syntax")("line %d: key-value loop works for maps only.\n",expr->lino);
        goto crash;
      }

      ex0->type = ex2->type->elem0;
      ex1->type = ex2->type->u.elem1;

      str_t name0 = ((tok_t*)ex0->term)->val;
      str_t name1 = ((tok_t*)ex1->term)->val;

      map_add(&expr->symtable, name0, sym_new(name0, *ex0->type ,0));
      map_add(&expr->symtable, name1, sym_new(name1, *ex1->type ,0));      

      expr_t* ex = (expr_t*)((expr->children).tail->data);

      if (str_eq(&ex->rawkey, "do") ){
        ex->key = EXPR_DO;
        ex->type = prim_type(TYP_VOD);
      }else{
        printerr("syntax")("line %d: the last part of 'for' must be 'do', not '%s'.\n",expr->lino,ex->rawkey.data);
        goto crash;
      }
      compile_syntax_tree((expr_t*)((expr->children).tail->data),functable,stttable);

    }

  }else if (str_eq(&expr->rawkey,"while")){
    if (expr->children.len != 2){
      printerr("syntax")("line %d: 'while' must have 2 parts, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_WHILE;
    expr->type = prim_type(TYP_VOD);

    compile_syntax_tree_node((expr->children).head,functable,stttable);
    if (((expr_t*)((expr->children).head->data))->type == NULL){
      printf("[syntax warn] line %d: 'while' cannot parse type of expression.\n",expr->lino);
    }else{
      if  (((expr_t*)((expr->children).head->data))->type->tag != TYP_INT ){
        printerr("syntax")("line %d: the condition for 'while' must evaluate to int, got ",expr->lino);
        print_type(((expr_t*)((expr->children).head->data))->type);
        printf(".\n");
        goto crash;
      }
    }
    expr_t* ex1 = (expr_t*)((expr->children).tail->data);
    if (str_eq(&ex1->rawkey, "do") ){
      ex1->key = EXPR_DO;
      ex1->type = prim_type(TYP_VOD);
    }else{
      printerr("syntax")("line %d: the last part of 'while' must be 'do', not '%s'.\n",expr->lino,ex1->rawkey.data);
      goto crash;
    }
    compile_syntax_tree((expr_t*)((expr->children).tail->data),functable,stttable);

  }else if (str_eq(&expr->rawkey,"cast")){
    if (expr->children.len != 2){
      printerr("syntax")("line %d: 'cast' must have 2 params, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_CAST;
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).tail,functable,stttable);

    if (((expr_t*)((expr->children).head->data))->type == NULL || ((expr_t*)((expr->children).tail->data))->type == NULL ){
      printf("[syntax warn] line %d: 'cast' cannot parse type of expression.\n",expr->lino);
    }

    if (((expr_t*)((expr->children).tail->data))->key != EXPR_TYPE ){
      printerr("syntax")("line %d: the second param to 'cast' must be a type.\n",expr->lino);
      goto crash;
    }

    type_t* typl = ((expr_t*)((expr->children).head->data))->type;
    type_t* typr = (type_t*)(((expr_t*)((expr->children).tail->data))->term);

    if (typl->tag == TYP_MAP || typl->tag == TYP_VEC || typl->tag == TYP_ARR || typl->tag == TYP_STT ||
        typr->tag == TYP_MAP || typr->tag == TYP_VEC || typr->tag == TYP_ARR || typr->tag == TYP_STT
      ){
      printerr("syntax")("line %d: cannot cast from ",expr->lino);
      print_type(typl);
      printf(" to ");
      print_type(typr);
      printf(".\n");
      goto crash;
    }
    expr->type = typr;


  }else if (str_eq(&expr->rawkey,"alloc")){

    if (expr->children.len < 1){
      printerr("syntax")("line %d: 'alloc' must have at least 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_ALLOC;
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);

    if (ex->key != EXPR_TYPE ){
      printerr("syntax")("line %d: the param to 'alloc' must be a type.\n",expr->lino);
      goto crash;
    }

    type_t* typ = (type_t*)(ex->term);
    
    if (typ->tag != TYP_ARR && typ->tag != TYP_VEC && typ->tag != TYP_MAP && typ->tag != TYP_STT  && typ->tag != TYP_STR){
      printerr("syntax")("line %d: only str or arr or vec or map or struct can be 'alloc'ed, not ",expr->lino);
      print_type(typ);
      printf(".\n");
      goto crash;
    }

    if (typ->tag == TYP_STR){
      if (expr->children.len == 2){
        compile_syntax_tree_node((expr->children).head->next,functable,stttable);
      }else if (expr->children.len > 2){
        printerr("syntax")("line %d: 'alloc' for str must have 1 or 2 params, not %d.\n",expr->lino,expr->children.len);
        goto crash;
      }
    }else if (expr->children.len > 1){
      if ( (typ->tag != TYP_ARR && typ->tag != TYP_VEC) || (typ->elem0->tag != TYP_INT && typ->elem0->tag != TYP_FLT && typ->elem0->tag != TYP_STR)){
        printerr("syntax")("line %d: 'alloc' with literal only works for arr or vec (of ints or floats), not ",expr->lino);
        print_type(typ);
        printf(".\n");
        goto crash;
      }
      if (typ->tag == TYP_VEC && typ->u.size != expr->children.len-1){
        printerr("syntax")("line %d: number of elements in 'alloc' literal does not agree with vec size: expecting %d, got %d.\n",expr->lino,typ->u.size,expr->children.len-1);
        goto crash;
      }

      list_node_t* jt = expr->children.head->next;
      while(jt){
        compile_syntax_tree_node(jt,functable,stttable);

        expr_t* e = (expr_t*)(jt->data);
        if (!type_eq(e->type,typ->elem0)){
          expr_t* ex2 = caster(e,typ->elem0);
          jt->data = ex2;
        }
        jt = jt->next;
      }

    
    }
    expr->type = typ;

  }else if (str_eq(&expr->rawkey,"??")){
    if (expr->children.len != 1){
      printerr("syntax")("line %d: '\?\?'(null check) must have 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_NOTNULL;
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);

    type_t* typ = ex->type;

    if (typ == NULL){
      printf("[syntax warn] line %d: '\?\?'(null check) cannot parse type of expression.\n",expr->lino);
    }else{

      if (typ->tag != TYP_ARR && typ->tag != TYP_VEC && typ->tag != TYP_MAP && typ->tag != TYP_STT){
        printerr("syntax")("line %d: only arr or vec or map or struct can be checked null, not ",expr->lino);
        print_type(typ);
        printf(".\n");
        goto crash;
      }
    }
    
    expr->type = prim_type(TYP_INT);

  }else if (str_eq(&expr->rawkey,"null")){
    if (expr->children.len != 1 && expr->children.len != 2){
      printerr("syntax")("line %d: 'null' must have 1 or 2 params, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_SETNULL;

    if (expr->children.len == 1){
      compile_syntax_tree_node((expr->children).head,functable,stttable);

      expr_t* ex = (expr_t*)((expr->children).head->data);

      type_t* typ = ex->type;

      if (typ == NULL){
        printf("[syntax warn] line %d: 'null' cannot parse type of expression.\n",expr->lino);
      }else{

        if (typ->tag != TYP_ARR && typ->tag != TYP_VEC && typ->tag != TYP_MAP && typ->tag != TYP_STT){
          printerr("syntax")("line %d: only arr or vec or map or struct can be set to null, not ",expr->lino);
          print_type(typ);
          printf(".\n");
          goto crash;
        }
      }
      
      
    }else{
      compile_syntax_tree_node(expr->children.head,functable,stttable);
      
      expr_t* ex0 = (expr_t*)((expr->children).head->data);
      expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

      if (ex0->type == NULL){
        printf("[syntax warn] line %d: 'null' cannot parse type of expression.\n",expr->lino);

      }else{
        if (ex0->type->tag == TYP_STT){

          if (ex1->children.len != 0 || ((tok_t*)ex1->term)->tag != TOK_IDT ){
            printerr("syntax")("line %d: 'null' expecting a struct field name.\n",expr->lino);
            goto crash;
          }
          type_t* typ = struct_lookup(&ex0->type->u.name,&((tok_t*)ex1->term)->val,stttable);
          if (typ == NULL){
            printerr("syntax")("line %d: struct '%s' has no field '%s' to nullify.\n",expr->lino,ex0->type->u.name.data,((tok_t*)ex1->term)->val.data);
            goto crash;
          }
          ex1->type = prim_type(TYP_VOD);

        }else if (ex0->type->tag == TYP_VEC){

          compile_syntax_tree_node((expr->children).head->next,functable,stttable);
          ex1 = (expr_t*)((expr->children).head->next->data);

          if (ex0->type == NULL || ex1->type == NULL){
            printf("[syntax warn] line %d: 'null' cannot parse type of expression.\n",expr->lino);
          }else{
            type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;

            if (typr->tag != TYP_INT){
              printerr("syntax")("line %d: vec index must be int.\n",expr->lino);
              goto crash;
            }
          }

        }else if (ex0->type->tag == TYP_ARR){
          
          compile_syntax_tree_node((expr->children).head->next,functable,stttable);
          ex1 = (expr_t*)((expr->children).head->next->data);

          if (ex0->type == NULL || ex1->type == NULL){
            printf("[syntax warn] line %d: 'null' cannot parse type of expression.\n",expr->lino);
          }else{
            type_t* typr = ((expr_t*)((expr->children).head->next->data))->type;
            if (typr->tag != TYP_INT){
              printerr("syntax")("line %d: arr index must be int.\n",expr->lino);
              goto crash;
            }
          }

        }else{
          printerr("syntax")("line %d: cannot use 'null' on \n",expr->lino);
          print_type(ex0->type);
          printf(" type.\n");
          goto crash;
        }
      }
    }

    expr->type = prim_type(TYP_VOD);

  }else if (str_eq(&expr->rawkey,"#")){
    if (expr->children.len != 1){
      printerr("syntax")("line %d: '#'(len) must have 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);

    type_t* typ = ex->type;

    if (typ == NULL){
      printf("[syntax warn] line %d: '#' cannot parse type of expression.\n",expr->lino);
    }else{
      if (typ->tag == TYP_ARR){
        expr->key = EXPR_ARRLEN;
      }else if (typ->tag == TYP_STR){

        expr->key = EXPR_STRLEN;
      }else if (typ->tag == TYP_MAP){

        expr->key = EXPR_MAPLEN;
      }else{
        printerr("syntax")("line %d: '#'(len) only works for arr or str or map, not ",expr->lino);
        print_type(typ);
        printf(".\n");
        goto crash;
      }
    }
    
    expr->type = prim_type(TYP_INT);

  }else if (str_eq(&expr->rawkey,"slice")){
    if (expr->children.len != 3){
      printerr("syntax")("line %d: 'slice' must have 3 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
    expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);

    type_t* typ = ex->type;

    if (typ == NULL){
      printf("[syntax warn] line %d: 'slice' cannot parse type of expression.\n",expr->lino);
    }else{
      if (typ->tag == TYP_ARR){
        expr->key = EXPR_ARRCPY;
      }else if (typ->tag == TYP_STR){
        expr->key = EXPR_STRCPY;
      }else{
        printerr("syntax")("line %d: 'slice' only works for arr or str, not ",expr->lino);
        print_type(typ);
        printf(".\n");
        goto crash;
      }
    }
    

    if (ex1->type == NULL || ex2->type == NULL){
      printf("[syntax warn] line %d: slice cannot parse type of expression.\n",expr->lino);
    }else{
      if (!(ex1->type->tag == TYP_INT) || !(ex2->type->tag == TYP_INT)){
        printerr("syntax")("line %d: slice begin index and length must be int, got ",expr->lino);
        print_type(ex1->type);
        printf(" and ");
        print_type(ex2->type);
        printf(".\n");
        goto crash;
      }
    }
    
    expr->type = typ;

  }else if (str_eq(&expr->rawkey,"remove")){
    if (expr->children.len != 2 && expr->children.len != 3){
      printerr("syntax")("line %d: 'remove' must have 2 or 3 params, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);

    expr->type = prim_type(TYP_VOD);

    if (ex0->type == NULL || ex1->type == NULL){
      printf("[syntax warn] line %d: arr 'remove' cannot parse type of expression.\n",expr->lino);
    }else{
      if (ex0->type->tag == TYP_ARR){
        expr->key = EXPR_ARRREM;
        compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);
        expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);

        if (ex1->type == NULL){
          printf("[syntax warn] line %d: arr 'remove' cannot parse type of expression.\n",expr->lino);
        }else{
          if (!(ex1->type->tag == TYP_INT) || !(ex2->type->tag == TYP_INT)){
            printerr("syntax")("line %d: arr 'remove' begin index and length must be int, got ",expr->lino);
            print_type(ex1->type);
            printf(" and ");
            print_type(ex2->type);
            printf(".\n");
            goto crash;
          }

        }

      }else if (ex0->type->tag == TYP_MAP){
        expr->key = EXPR_MAPREM;
        if (!type_eq(ex1->type,ex0->type->elem0)){
          printerr("syntax")("line %d: map 'remove' key expecting ",expr->lino);
          print_type(ex0->type->elem0);
          printf(", got ");
          print_type(ex1->type);
          printf(".\n");
          goto crash;
        }
      }
    }

  }else if (str_eq(&expr->rawkey,"insert")){
    if (expr->children.len != 3){
      printerr("syntax")("line %d: 'insert' must have 3 params, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
    expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);

    expr->type = prim_type(TYP_VOD);

    if (ex0->type == NULL || ex1->type == NULL || ex2->type == NULL){
      printf("[syntax warn] line %d: 'insert' cannot parse type of expression.\n",expr->lino);
    }else{
      if (ex0->type->tag == TYP_ARR){
        expr->key = EXPR_ARRINS;

        if (!(ex1->type->tag == TYP_INT) || !type_eq(ex2->type,ex0->type->elem0)){
          printerr("syntax")("line %d: arr 'insert' expecting int and ",expr->lino);
          print_type(ex0->type->elem0);
          printf(", got ");
          print_type(ex1->type);
          printf(" and ");
          print_type(ex2->type);
          printf(".\n");
          goto crash;
        }

        
      }else if (ex0->type->tag == TYP_MAP){
        expr->key = EXPR_MAPSET;
        if (!type_eq(ex1->type,ex0->type->elem0) || !type_eq(ex2->type,ex0->type->u.elem1)){
          printerr("syntax")("line %d: map 'insert' expecting ",expr->lino);
          print_type(ex0->type->elem0);
          printf(" and ");
          print_type(ex0->type->u.elem1);
          printf(", got ");
          print_type(ex1->type);
          printf(" and ");
          print_type(ex2->type);
          printf(".\n");
          goto crash;
        }
      }
    }

  }else if (str_eq(&expr->rawkey,"free")){
    if (expr->children.len != 1){
      printerr("syntax")("line %d: 'free' must have 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_FREE;
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);

    // if (ex->key != EXPR_TERM || ((tok_t*)(ex->term))->tag != TOK_IDT ){
    //   printerr("syntax")("line %d: param to 'free' must be an identifier (Assign r-value to a variable first).\n",expr->lino);
    //   goto crash;
    // }

    type_t* typ = ex->type;

    if (typ == NULL){
      printf("[syntax warn] line %d: 'free' cannot parse type of expression.\n",expr->lino);
    }else{

      if (typ->tag != TYP_ARR && typ->tag != TYP_VEC && typ->tag != TYP_MAP && typ->tag != TYP_STT && typ->tag != TYP_STR){
        printerr("syntax")("line %d: only str or arr or vec or map or struct can be freed, not ",expr->lino);
        print_type(typ);
        printf(".\n");
        goto crash;
      }
    }
    
    expr->type = prim_type(TYP_VOD);

  }else if (str_eq(&expr->rawkey,"print")){
    if (expr->children.len != 1){
      printerr("syntax")("line %d: 'print' must have 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    expr->key = EXPR_PRINT;
    compile_syntax_tree_node((expr->children).head,functable,stttable);

    expr_t* ex = (expr_t*)((expr->children).head->data);

    type_t* typ = ex->type;

    if (typ == NULL){
      printf("[syntax warn] line %d: 'print' cannot parse type of expression.\n",expr->lino);
    }else{
      if (typ->tag != TYP_STR){

        expr_t* ex2 = caster(ex,prim_type(TYP_STR));
        list_pop(&expr->children);
        list_add(&expr->children,ex2);

      }
    }
    expr->type = prim_type(TYP_VOD);
    
  }else if (str_eq(&expr->rawkey,"?")){
    if (expr->children.len != 3){
      printerr("syntax")("line %d: '?'(ternary op) must have 3 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next,functable,stttable);
    compile_syntax_tree_node((expr->children).head->next->next,functable,stttable);

    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    expr_t* ex1 = (expr_t*)((expr->children).head->next->data);
    expr_t* ex2 = (expr_t*)((expr->children).head->next->next->data);

    if (ex0->type == NULL || ex1->type == NULL || ex2->type == NULL){
      printf("[syntax warn] line %d: '?'(ternary op) cannot parse type of expression.\n",expr->lino);
    }else{
      if (ex0->type->tag != TYP_INT){
        printerr("syntax")("line %d: condition of '?'(ternary op) of must evaluate to int, not ",expr->lino);
        print_type(ex0->type);
        printf(".\n");
        goto crash;
      }

      if (!type_eq(ex1->type,ex2->type)){
        printerr("syntax")("line %d: then/else of '?'(ternary op) of must evaluate to the same type, got ",expr->lino);
        print_type(ex0->type);
        printf(" and ");
        print_type(ex1->type);
        printf(".\n");
        goto crash;
      }

    }
    expr->key = EXPR_TIF;
    expr->type = ex1->type;

  }else if (str_eq(&expr->rawkey,"break")){
    local_auto_free(it,tree,EXPR_DO);
    expr->key = EXPR_BREAK;
    expr->type = prim_type(TYP_VOD);
    expr->term = NULL;
  }else if (str_eq(&expr->rawkey,"asm")){
    expr->key = EXPR_ASM;
    expr->type = prim_type(TYP_VOD);
    if (expr->children.len != 1){
      printerr("syntax")("line %d: 'asm' must have 1 param, not %d.\n",expr->lino,expr->children.len);
      goto crash;
    }
    compile_syntax_tree_node((expr->children).head,functable,stttable);
    expr_t* ex0 = (expr_t*)((expr->children).head->data);
    if (ex0->type->tag != TYP_STR || ex0->key != EXPR_TERM){
      printerr("syntax")("line %d: 'asm' param must be a constant string.\n",expr->lino);
      goto crash;
    }

  }else{
    if (expr->key == EXPR_TBA){
      printf("[syntax warn] line %d: misplaced keyword: '%s'.\n",expr->lino,expr->rawkey.data);
    }else{
      // already handled
    }
  }

  goto done;

  crash:

  printf("exiting with syntax tree compilation failure.\n");
  freex();exit(1);

  done:
  return;
}




void compile_syntax_tree(expr_t* tree, map_t* functable, map_t* stttable){


  list_node_t* it = tree->children.head;
  map_clear(&tree->symtable);

  while (it){
  
    compile_syntax_tree_node(it,functable,stttable);
    it = it -> next;

  }

  if (tree->key != EXPR_PRGM){
    if (tree->children.len && ((expr_t*)(tree->children.tail->data))->key != EXPR_RETURN){
      local_auto_free(tree,tree,0);
    }
  }

  return;
}



void rename_identifier(expr_t* tree, char* rfrom, char* rto){
  list_node_t* it = tree->children.head;
  while (it){
    expr_t* expr = (expr_t*)(it->data);
    if (expr->key == EXPR_TERM){
      tok_t* tok = (tok_t*)(expr->term);
      if (tok->tag == TOK_IDT){
        if (str_eq(&tok->val,rfrom)){
          tok->val = str_from(rto,strlen(rto));
        }
      }
    }else{
      rename_identifier(expr,rfrom,rto);
    }
    it = it->next;
  }
}


void lift_scope(expr_t* expr){
  if (expr->key == EXPR_FUNCBODY || expr->key == EXPR_PRGM || expr->key == EXPR_FUNC || expr->key == EXPR_EXTERN || !expr->parent){
    //ok
  }else{
    expr_t* root = expr;
    while (root->key != EXPR_FUNCBODY && root->key != EXPR_PRGM){
      root = root->parent;
    }

    if (expr->symtable.len){
      char* pfx = tmp_name("__s").data;
      int k = 0;
      while (k < NUM_MAP_SLOTS){
        while (!expr->symtable.slots[k] && k < NUM_MAP_SLOTS){
          k++;
        }
        if (!expr->symtable.slots[k] || k >= NUM_MAP_SLOTS){
          break;
        }
        list_node_t* jt = expr->symtable.slots[k]->head;
        while (jt){
          sym_t* sym = (sym_t*)(jt->data);
          
          str_t nname = str_from(sym->name.data,sym->name.len);
          str_add(&nname,pfx);
          rename_identifier(expr,sym->name.data,nname.data);
          sym->name = nname;
          map_add(&root->symtable, sym->name, sym);
          jt = jt->next;
        }
        k++;
      }
      map_clear(&expr->symtable);
    }

  }

  list_node_t* it = expr->children.head;
  while (it){
    expr_t* ex = (expr_t*)(it->data);
    if(ex) lift_scope(ex);
    it = it->next;
  }
}


#endif
