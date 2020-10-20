#ifndef WAXC
#define WAXC

int WVERBOSE = 1;
#define printinfo if(WVERBOSE)printf

#include "parser.c"
#include "to_c.c"
#include "to_java.c"
#include "to_ts.c"
#include "to_wat.c"

#define TARG_C     0x1
#define TARG_JAVA  0x2
#define TARG_TS    0x4
#define TARG_WAT   0x8

void print_help(){
  printf(" _____                                          \n");
  printf("|||'  |                                         \n");
  printf("|''   |                                         \n");
  printf("|_WAX_| Compiler                                \n\n");
  printf("built " __DATE__ "                              \n\n");
  printf("USAGE: waxc [options] code.wax                  \n\n");
  printf("OPTIONS:                                        \n");
  printf("--c    path/out.c     transpile to c            \n");
  printf("--java path/out.java  transpile to java         \n");
  printf("--ts   path/out.ts    transpile to typescript   \n");
  printf("--tokens              print tokenization        \n");
  printf("--ast                 print abstract syntax tree\n");
  printf("--silent              don't print info          \n");
  printf("--help                print this message        \n");
}


void transpile(int targ, char* input_file, char* path, int print_tok, int print_ast){


  str_t buf = read_file_ascii(input_file);
  list_t tokens = tokenize(buf);
  map_t included = map_new();
  map_t defs = map_new();

  if (targ == TARG_C){
    defs_addbool(&defs,"TARGET_C",0);
  }else if (targ == TARG_JAVA){
    defs_addbool(&defs,"TARGET_JAVA",0);
  }else if (targ == TARG_TS){
    defs_addbool(&defs,"TARGET_TS",0);
  }else if (targ == TARG_WAT){
    defs_addbool(&defs,"TARGET_WAT",0);
  }


  printinfo("[info] running preprocessor...\n");
  preprocess(input_file,&tokens,&included,&defs);
  if (print_tok){
    print_tokens(&tokens);
  }

  printinfo("[info] constructing syntax tree...\n");
  expr_t* tree = syntax_tree(&tokens);
  printinfo("[info] compiling syntax tree...\n");
  map_t functable = map_new();
  map_t stttable = map_new();
  compile_syntax_tree(tree,&functable,&stttable);


  if (print_ast){
    print_stttable(&stttable);
    print_functable(&functable);
    print_syntax_tree(tree,0);
  }

  printinfo("[info] generating target code: %s\n",path);
  str_t modname = base_name(path);
  str_t out;
  if (targ == TARG_C){
    out = tree_to_c(modname,tree,&functable,&stttable);
  }else if (targ == TARG_JAVA){
    out = tree_to_java(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_TS){
    out = tree_to_ts(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_WAT){
    out = tree_to_wat(modname,tree,&functable,&stttable);
  }
  write_file_ascii(path, out.data);
  freex();
}


int main(int argc, char** argv){
  char* path_c = 0;
  char* path_java = 0;
  char* path_wat = 0;
  char* path_ts = 0;
  
  char* input_file = 0;

  int print_ast = 0;
  int print_tok = 0;

  int i = 1;
  while (i < argc){
    if (!strcmp(argv[i],"--c")){
      path_c = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--java")){
      path_java = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--ts")){
      path_ts = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--wat")){
      path_wat = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--ast")){
      print_ast = 1;
      i++;
    }else if (!strcmp(argv[i],"--tokens")){
      print_tok = 1;
      i++;
    }else if (!strcmp(argv[i],"--silent")){
      WVERBOSE = 0;
      i++;
    }else if (!strcmp(argv[i],"--help")){
      print_help();
      exit(0);
    }else{
      if (input_file){
        printf("[error] cannot parse commandline argument %s.\n",argv[i]);
        printf("exiting with commandline parsing failure.\n");
        exit(1);
      }else{
        input_file = argv[i];
        i++;
      }
    }
  }

  if (input_file == 0){
    printf("[warn] no input file.\n");
    exit(0);
  }

  if (path_c){
    printinfo("[info] transpiling '%s' to C...\n",input_file);
    transpile(TARG_C, input_file, path_c, print_tok, print_ast);
  }

  if (path_java){
    printinfo("[info] transpiling '%s' to Java...\n",input_file);
    transpile(TARG_JAVA, input_file, path_java, print_tok, print_ast);
  }

  if (path_ts){
    printinfo("[info] transpiling '%s' to TypeScript...\n",input_file);
    transpile(TARG_TS, input_file, path_ts, print_tok, print_ast);
  }

  if (path_wat){
    printinfo("[info] transpiling '%s' to WebAssembly Text...\n",input_file);
    transpile(TARG_WAT, input_file, path_wat, print_tok, print_ast);
  }

  return 0;
}

#endif
