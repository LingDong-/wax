#ifndef WAXC_LIB
#define WAXC_LIB

#define NO_ESC_COLOR

#include <string.h>
#include <stdio.h>

static const int WVERBOSE = 1;
#define printinfo if(WVERBOSE)printf

#include "parser.h"
#include "to_c.h"
#include "to_java.h"
#include "to_ts.h"
#include "to_json.h"
#include "to_py.h"
#include "to_cs.h"
#include "to_cpp.h"
#include "to_swift.h"
#include "to_lua.h"
#include "to_wat.h"

#include "waxc_lib.h"


char* transpile(char* targ, char* filname, char* src, int print_ast){


  str_t buf = str_from(src,strlen(src));
  list_t tokens = tokenize(buf);
  map_t included;
  map_t defs;
  map_clear(&included);
  map_clear(&defs);

  if (!strcmp(targ,"c")){
    defs_addbool(&defs,"TARGET_C",0);
  }else if (!strcmp(targ,"java")){
    defs_addbool(&defs,"TARGET_JAVA",0);
  }else if (!strcmp(targ,"ts")){
    defs_addbool(&defs,"TARGET_TS",0);
  }else if (!strcmp(targ,"json")){
    defs_addbool(&defs,"TARGET_JSON",0);
  }else if (!strcmp(targ,"py")){
    defs_addbool(&defs,"TARGET_PY",0);
  }else if (!strcmp(targ,"cs")){
    defs_addbool(&defs,"TARGET_CS",0);
  }else if (!strcmp(targ,"cpp")){
    defs_addbool(&defs,"TARGET_CPP",0);
  }else if (!strcmp(targ,"swift")){
    defs_addbool(&defs,"TARGET_SWIFT",0);
  }else if (!strcmp(targ,"lua")){
    defs_addbool(&defs,"TARGET_LUA",0);
  }else if (!strcmp(targ,"wat")){
    defs_addbool(&defs,"TARGET_WAT",0);
  }

  printinfo("[info] running preprocessor...\n");
  preprocess(filname,&tokens,&included,&defs);

  // print_tokens(&tokens);


  printinfo("[info] constructing syntax tree...\n");
  expr_t* tree = syntax_tree(&tokens);
  printinfo("[info] compiling syntax tree...\n");
  map_t functable;
  map_t stttable;
  map_clear(&functable);
  map_clear(&stttable);
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
  }else if (!strcmp(targ,"json")){
    out = tree_to_json(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"py")){
    out = tree_to_py(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"cs")){
    out = tree_to_cs(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"cpp")){
    out = tree_to_cpp(modname,tree,&functable,&stttable);
  }else if (!strcmp(targ,"swift")){
    out = tree_to_swift(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"lua")){
    out = tree_to_lua(modname,tree,&functable,&stttable,&included);
  }else if (!strcmp(targ,"wat")){
    out = tree_to_wat(modname,tree,&functable,&stttable,&included);
  }
  char* cout = (char*)malloc(out.len+1);
  memcpy(cout, out.data, out.len);
  cout[out.len] = 0;
  freex();
  return cout;
}



#endif
