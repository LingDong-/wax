#ifndef WAXC
#define WAXC

int WVERBOSE = 1;
#define printinfo if(WVERBOSE)printf

#include "parser.c"
#include "tac.c"
#include "to_c.c"
#include "to_java.c"
#include "to_ts.c"
#include "to_json.c"
#include "to_py.c"
#include "to_cs.c"
#include "to_cpp.c"
#include "to_swift.c"
#include "to_lua.c"
#include "to_wat.c"

#define TARG_C     1
#define TARG_JAVA  2
#define TARG_TS    4
#define TARG_JSON  8
#define TARG_PY    16
#define TARG_CS    32
#define TARG_CPP   64
#define TARG_SWIFT 128
#define TARG_LUA   256
#define TARG_WAT   512

void print_help(){
  printf(" _____                                           \n");
  printf("|||'  |                                          \n");
  printf("|''   |                                          \n");
  printf("|_WAX_| Compiler                                 \n\n");
  printf("built " __DATE__ "                               \n\n");
  printf("USAGE: waxc [options] code.wax                   \n\n");
  printf("OPTIONS:                                         \n");
  printf("--c     path/out.c     transpile to c            \n");
  printf("--java  path/out.java  transpile to java         \n");
  printf("--ts    path/out.ts    transpile to typescript   \n");
  printf("--py    path/out.py    transpile to python       \n");
  printf("--cs    path/out.cs    transpile to c#           \n");
  printf("--cpp   path/out.cpp   transpile to c++          \n");
  printf("--swift path/out.swift transpile to swift        \n");
  printf("--lua   path/out.lua   transpile to lua          \n");
  printf("--wat   path/out.wat   transpile to webassembly  \n");
  printf("--json  path/out.json  syntax tree to JSON file  \n");
  printf("--tokens               print tokenization        \n");
  printf("--ast                  print abstract syntax tree\n");
  printf("--silent               don't print info          \n");
  printf("--help                 print this message        \n");
}


void transpile(int targ, const char* input_file, const char* path, int print_tok, int print_ast){


  str_t buf = read_file_ascii(input_file);
  list_t tokens = tokenize(buf);
  map_t included, defs;
  map_clear(&included);
  map_clear(&defs);

  if (targ == TARG_C){
    defs_addbool(&defs,"TARGET_C",0);
  }else if (targ == TARG_JAVA){
    defs_addbool(&defs,"TARGET_JAVA",0);
  }else if (targ == TARG_TS){
    defs_addbool(&defs,"TARGET_TS",0);
  }else if (targ == TARG_JSON){
    defs_addbool(&defs,"TARGET_JSON",0);
  }else if (targ == TARG_PY){
    defs_addbool(&defs,"TARGET_PY",0);
  }else if (targ == TARG_CS){
    defs_addbool(&defs,"TARGET_CS",0);
  }else if (targ == TARG_CPP){
    defs_addbool(&defs,"TARGET_CPP",0);
  }else if (targ == TARG_SWIFT){
    defs_addbool(&defs,"TARGET_SWIFT",0);
  }else if (targ == TARG_LUA){
    defs_addbool(&defs,"TARGET_LUA",0);
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
  map_t functable, stttable;
  map_clear(&functable);
  map_clear(&stttable);
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
  }else if (targ == TARG_JSON){
    out = tree_to_json(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_PY){
    out = tree_to_py(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_CS){
    out = tree_to_cs(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_CPP){
    out = tree_to_cpp(modname,tree,&functable,&stttable);
  }else if (targ == TARG_SWIFT){
    out = tree_to_swift(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_LUA){
    out = tree_to_lua(modname,tree,&functable,&stttable,&included);
  }else if (targ == TARG_WAT){
    out = tree_to_wat(modname,tree,&functable,&stttable,&included);
  }
  write_file_ascii(path, out.data);
  freex();
}


int main(int argc, char** argv){
  const char* path_c = 0;
  const char* path_java = 0;
  const char* path_ts = 0;
  const char* path_json = 0;
  const char* path_py = 0;
  const char* path_cs = 0;
  const char* path_cpp = 0;
  const char* path_swift = 0;
  const char* path_lua = 0;
  const char* path_wat = 0;
  const char* input_file = 0;

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
    }else if (!strcmp(argv[i],"--json")){
      path_json = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--py")){
      path_py = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--cs")){
      path_cs = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--cpp")){
      path_cpp = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--swift")){
      path_swift = argv[i+1];
      i+=2;
    }else if (!strcmp(argv[i],"--lua")){
      path_lua = argv[i+1];
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
    printf("[warn] no input file. (try '--help' for usage.)\n");
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

  if (path_json){
    printinfo("[info] transpiling '%s' to JSON...\n",input_file);
    transpile(TARG_JSON, input_file, path_json, print_tok, print_ast);
  }

  if (path_py){
    printinfo("[info] transpiling '%s' to Python...\n",input_file);
    transpile(TARG_PY, input_file, path_py, print_tok, print_ast);
  }

  if (path_cs){
    printinfo("[info] transpiling '%s' to C#...\n",input_file);
    transpile(TARG_CS, input_file, path_cs, print_tok, print_ast);
  }

  if (path_cpp){
    printinfo("[info] transpiling '%s' to C++...\n",input_file);
    transpile(TARG_CPP, input_file, path_cpp, print_tok, print_ast);
  }

  if (path_swift){
    printinfo("[info] transpiling '%s' to Swift...\n",input_file);
    transpile(TARG_SWIFT, input_file, path_swift, print_tok, print_ast);
  }

  if (path_lua){
    printinfo("[info] transpiling '%s' to Lua...\n",input_file);
    transpile(TARG_LUA, input_file, path_lua, print_tok, print_ast);
  }

  if (path_wat){
    printinfo("[info] transpiling '%s' to WebAssembly Text...\n",input_file);
    transpile(TARG_WAT, input_file, path_wat, print_tok, print_ast);
  }

  return 0;
}

#endif
