#ifndef WAXC_LIB
#define WAXC_LIB

int WVERBOSE = 0;
#define printinfo if(WVERBOSE)printf

#define NO_ESC_COLOR

#include "parser.c"
#include "to_c.c"
#include "to_java.c"
#include "to_ts.c"


char* transpile(char* targ, char* filname, char* src, int print_ast){


  str_t buf = str_from(src,strlen(src));
  list_t tokens = tokenize(buf);
  map_t included = map_new();
  map_t defs = map_new();

  if (!strcmp(targ,"c")){
    defs_addbool(&defs,"TARGET_C",0);
  }else if (!strcmp(targ,"java")){
    defs_addbool(&defs,"TARGET_JAVA",0);
  }else if (!strcmp(targ,"ts")){
    defs_addbool(&defs,"TARGET_TS",0);
  }

  printinfo("[info] running preprocessor...\n");
  preprocess(filname,&tokens,&included,&defs);

  // print_tokens(&tokens);


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

  str_t modname = base_name(filname);
  printinfo("[info] generating target code... %s\n",modname.data);
  
  str_t out;
  if (!strcmp(targ,"c")){
    out = tree_to_c(modname,tree,&functable,&stttable);
  }else if (!strcmp(targ,"java")){
    out = tree_to_java(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"ts")){
    out = tree_to_ts(modname,tree,&functable,&stttable,&included);
  }
  char* cout = (char*)malloc(out.len+1);
  memcpy(cout, out.data, out.len);
  cout[out.len] = 0;
  freex();
  return cout;
}



#endif
