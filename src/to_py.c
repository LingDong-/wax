#ifndef WAX_TO_PY
#define WAX_TO_PY

#include "text.c"
#include "parser.c"
#include "common.c"

str_t zero_to_py(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"0");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"0.0");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"None");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"None");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"None");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"None");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"None");
  }
  return out;
}

str_t globals_to_py(expr_t* expr){
  str_t out = str_new();
  expr_t* root = expr;
  while (root->key != EXPR_PRGM){
    root = root->parent;
  }
  int first = 1;

  int k = 0;
  while (k < NUM_MAP_SLOTS){
    while (!root->symtable.slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!root->symtable.slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* jt = root->symtable.slots[k]->head;
    while (jt){
      sym_t* sym = (sym_t*)(jt->data);


      if (sym_lookup_local(&sym->name,expr)==NULL){
        if (first){
          str_add(&out,"global ");
        }else{
          str_add(&out,",");
        }
        first = 0;
        str_add(&out,sym->name.data);
      }

      jt = jt->next;
    }
    k++;
  }
  return out;
}

str_t expr_to_py(expr_t* expr, int indent){
  // print_syntax_tree(expr,4);
  // printf("-----\n");
  str_t out = str_new();
  INDENT4(indent);

  if (expr->key == EXPR_LET){
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"=");
    str_add(&out,zero_to_py( (type_t*)(CHILD2->term) ).data);
    
  }else if (expr->key == EXPR_SET){
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out,"=");
    str_add(&out, expr_to_py(CHILD2,-1).data );

  }else if (expr->key == EXPR_TERM){
    tok_t* tok = ((tok_t*)(expr->term));

    if (tok->tag == TOK_INT && tok->val.data[0] == '\''){
      str_add(&out, "ord(");
      str_add(&out, tok->val.data);
      str_add(&out, ")");
    }else{
      str_add(&out, tok->val.data);
    }
  
  }else if (expr->key == EXPR_FADD || expr->key == EXPR_IADD ||
            expr->key == EXPR_FSUB || expr->key == EXPR_ISUB || 
            expr->key == EXPR_FMUL || expr->key == EXPR_IMUL ||
            expr->key == EXPR_FDIV ||
            expr->key == EXPR_BAND || 
            expr->key == EXPR_BOR  || 
            expr->key == EXPR_IMOD || expr->key == EXPR_FMOD ||
            expr->key == EXPR_XOR  || 
            expr->key == EXPR_SHL  || expr->key == EXPR_SHR
    ){
    str_add(&out, "((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ")");
    str_add(&out, expr->rawkey.data);
    str_add(&out, "(");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");
  }else if (expr->key == EXPR_LAND){
    str_add(&out, "int(bool((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ") and (");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, ")))");

  }else if (expr->key == EXPR_LOR) {
    str_add(&out, "int(bool((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ") or (");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, ")))");

  }else if (expr->key == EXPR_IGEQ || expr->key == EXPR_FGEQ ||
            expr->key == EXPR_ILEQ || expr->key == EXPR_FLEQ ||
            expr->key == EXPR_IGT  || expr->key == EXPR_FGT  ||
            expr->key == EXPR_ILT  || expr->key == EXPR_FLT  
  ){
    str_add(&out, "int((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ")");
    str_add(&out, expr->rawkey.data);
    str_add(&out, "(");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");
  }else if (expr->key == EXPR_IDIV){
    str_add(&out, "int(float(");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ")/(");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_IEQ || expr->key == EXPR_FEQ || expr->key == EXPR_STREQL){

    str_add(&out, "int((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ")==(");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_PTREQL){
    str_add(&out, "int((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ") is (");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");
  }else if (expr->key == EXPR_INEQ || expr->key == EXPR_FNEQ || expr->key == EXPR_STRNEQ){

    str_add(&out, "int((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ")!=(");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_PTRNEQ){

    str_add(&out, "int(not((");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, ") is (");
    str_add(&out, expr_to_py(CHILD2,-1).data );
    str_add(&out, ")))");

  }else if (expr->key == EXPR_BNEG){
    str_add(&out, "(");
    str_add(&out, expr->rawkey.data);
    str_add(&out, "(");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_LNOT){
    str_add(&out, "int(not");
    str_add(&out, "(");
    str_add(&out, expr_to_py(CHILD1,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_IF){
    str_add(&out, "if ");
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out, ":\n");
    str_add(&out, expr_to_py(CHILD2,indent).data);
    if (CHILD3){
      INDENT4(indent);
      str_add(&out, "else:\n");
      str_add(&out, expr_to_py(CHILD3,indent).data);
    }
  }else if (expr->key == EXPR_TIF){
    str_add(&out, "((");
    str_add(&out, expr_to_py(CHILD2,-1).data);
    str_add(&out, ") if (");
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out, ") else (");
    str_add(&out, expr_to_py(CHILD3,-1).data);
    str_add(&out, "))");

  }else if (expr->key == EXPR_WHILE){
    str_add(&out, "while ");
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out, ":\n");
    str_add(&out, expr_to_py(CHILD2,indent).data);

  }else if (expr->key == EXPR_FOR){
    str_t itname = tmp_name("tmp_it_");
    str_add(&out, itname.data);
    str_add(&out, "=");
    str_add(&out, expr_to_py(CHILD2,-1).data);
    str_add(&out, "\n");
    INDENT4(indent);
    str_add(&out, "while True:\n");
    INDENT4(indent+1);
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out, "=");
    str_add(&out, itname.data);
    str_add(&out, ";\n");
    INDENT4(indent+1);
    str_add(&out, "if ");
    str_add(&out, expr_to_py(CHILD3,-1).data);
    str_add(&out, ":\n");
    str_add(&out, expr_to_py(CHILDN,indent+1).data);
    INDENT4(indent+2);
    str_add(&out, itname.data);
    str_add(&out, "+=");
    str_add(&out, expr_to_py(CHILD4,-1).data);
    str_add(&out, "\n");
    INDENT4(indent+1);
    str_add(&out, "else:break");

  }else if (expr->key == EXPR_FORIN){

    str_add(&out, "for ");
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out, ",");
    str_add(&out, expr_to_py(CHILD2,-1).data);
    str_add(&out, " in (");
    str_add(&out, expr_to_py(CHILD3,-1).data);
    str_add(&out, ").items():\n");

    str_add(&out, expr_to_py(CHILDN,indent+1).data);


  }else if (expr->key == EXPR_FUNC){
    str_add(&out, "def ");
    list_node_t* it = expr->children.head;

    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);
    str_add(&out, "(");
    it = expr->children.head->next;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key != EXPR_PARAM){
        break;
      }
      if (it != expr->children.head->next){
        str_add(&out,",");
      }
    
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);

      it = it->next;
    }
    str_add(&out, ")");


    str_add(&out, ":\n");
    str_add(&out, expr_to_py(CHILDN,indent).data);
    INDENT4(indent);


  }else if (expr->key == EXPR_CALL){
    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);
    str_add(&out, "(");
    list_node_t* it = expr->children.head->next;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key == EXPR_RESULT){
        break;
      }
      if (it != expr->children.head->next){
        str_add(&out,",");
      }

      str_add(&out, expr_to_py(((expr_t*)(it->data)),-1).data );

      it = it->next;
    }
    str_add(&out, ")");

  }else if (expr->key == EXPR_THEN || expr->key == EXPR_ELSE || expr->key == EXPR_DO || expr->key == EXPR_FUNCBODY){
    if (expr->key == EXPR_FUNCBODY){
      INDENT4(indent+1);
      str_add(&out,globals_to_py(expr).data);
      str_add(&out,"\n");
    }
    if (!expr->children.len){
      INDENT4(1);
      str_add(&out,"pass\n");
    }

    list_node_t* it = expr->children.head;

    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (it==(expr->children.head)){
        str_add(&out,(char*)&expr_to_py(ex,indent+1).data[indent*4]);
      }else{
        str_add(&out,expr_to_py(ex,indent+1).data);
      }
      it = it->next;
    }

    indent=-1;

  }else if (expr->key == EXPR_CAST){
    type_t* typl = CHILD1->type;
    type_t* typr = (type_t*)(CHILD2->term);
    if (typl->tag == TYP_INT && typr->tag == TYP_FLT){
      str_add(&out, expr_to_py(CHILD1,-1).data);
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_INT){
      str_add(&out, "int(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_INT && typr->tag == TYP_STR){
      str_add(&out, "str(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "str(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "int(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "float(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    }else{
      str_add(&out, "(");
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out, ")");
    }
  }else if (expr->key == EXPR_RETURN){
    str_add(&out,"return");
    if (CHILD1){
      str_add(&out," ");
      str_add(&out,expr_to_py(CHILD1,-1).data);
    }
  }else if (expr->key == EXPR_STRUCT){
    str_add(&out,"class ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,":\n");

    list_node_t* it = expr->children.head->next;

    while(it){
      // expr_t* ex = (expr_t*)(it->data);

      INDENT4(indent+1);

      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out,"=");
      str_add(&out,zero_to_py(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out,"\n");
      it = it->next;
    }
    INDENT4(indent);

  }else if (expr->key == EXPR_NOTNULL){
    str_add(&out,"(");
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out,"!=None)");

  }else if (expr->key == EXPR_SETNULL){
    if (!CHILD2){
      str_add(&out, expr_to_py(CHILD1,-1).data);
      str_add(&out,"=None");
    }else{
      if (CHILD1->type->tag == TYP_STT){
        str_add(&out,"(");
        str_add(&out,expr_to_py(CHILD1,-1).data);
        str_add(&out,").");
        str_add(&out,expr_to_py(CHILD2,-1).data);
        str_add(&out,"=None");
      }else if (CHILD1->type->tag == TYP_ARR){
        str_add(&out,"(");
        str_add(&out,expr_to_py(CHILD1,-1).data);
        str_add(&out,")[");
        str_add(&out,expr_to_py(CHILD2,-1).data);
        str_add(&out,"]=None");
      }else if (CHILD1->type->tag == TYP_VEC){
        str_add(&out,"(");
        str_add(&out,expr_to_py(CHILD1,-1).data);
        str_add(&out,")[");
        str_add(&out,expr_to_py(CHILD2,-1).data);
        str_add(&out,"]=None");
      }
    }
  }else if (expr->key == EXPR_ALLOC){
    type_t* typ = (type_t*)(CHILD1->term);

    if (typ->tag == TYP_STT){
      str_add(&out,typ->u.name.data);
      str_add(&out,"()");

    }else if (typ->tag == TYP_ARR){
      if (expr->children.len == 1){
        str_add(&out,"[]");
      }else{

        str_add(&out,"[");
        list_node_t* it = expr->children.head->next;
        while (it){
          if (it != expr->children.head->next){
            str_add(&out,",");
          }
          str_add(&out,"(");
          str_add(&out,expr_to_py((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,"]");
      }

    }else if (typ->tag == TYP_VEC){

      if (expr->children.len == 1){
        char s[32];
        snprintf(s,sizeof(s),"%d",typ->u.size);
        str_add(&out,"([");
        str_add(&out,zero_to_py(typ->elem0).data);
        str_add(&out,"]*");
        str_add(&out,s);
        str_add(&out,")");
      }else{
        str_add(&out,"[");
        list_node_t* it = expr->children.head->next;
        while (it){
          if (it != expr->children.head->next){
            str_add(&out,",");
          }
          str_add(&out,"(");
          str_add(&out,expr_to_py((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,"]");      
      }
    }else if (typ->tag == TYP_MAP){
        str_add(&out,"{}");
    }else if (typ->tag == TYP_STR){
      if (CHILD2){
        str_add(&out,expr_to_py(CHILD2,-1).data);
      }else{
        str_add(&out,"\"\"");
      }
    }
  }else if (expr->key == EXPR_FREE){
    str_add(&out, expr_to_py(CHILD1,-1).data);
    str_add(&out,"=None#(GC)");

  }else if (expr->key == EXPR_STRUCTGET){
    str_add(&out,"((");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,").");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRUCTSET){
    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,").");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"=");
    str_add(&out,expr_to_py(CHILD3,-1).data);

  }else if (expr->key == EXPR_VECGET){
    str_add(&out,"((");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"])");
  }else if (expr->key == EXPR_VECSET){
    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"]=");
    str_add(&out,expr_to_py(CHILD3,-1).data);

  }else if (expr->key == EXPR_ARRGET){
    str_add(&out,"((");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"])");

  }else if (expr->key == EXPR_ARRSET){
    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"]=");
    str_add(&out,expr_to_py(CHILD3,-1).data);

  }else if (expr->key == EXPR_ARRINS){

    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,").insert((");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_py(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_ARRREM){

    str_add(&out,"w_arr_remove(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"w_slice(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,"len(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,"len(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPGET){
    str_add(&out,"w_map_get(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,zero_to_py(CHILD1->type->u.elem1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"w_map_remove(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"]=");
    str_add(&out,expr_to_py(CHILD3,-1).data);

  }else if (expr->key == EXPR_STRLEN){
    str_add(&out,"len(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRGET){
    str_add(&out,"ord((");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,"])");

  }else if (expr->key == EXPR_STRADD){
    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")+=chr(");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRCAT){

    str_add(&out,"(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")+=(");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRCPY){
    str_add(&out,"w_slice(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_py(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"print(");
    str_add(&out,expr_to_py(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_EXTERN){
    //skip
    out.len-=4;
    if(out.len < 0) out.len = 0;
    out.data[out.len] = 0;
    indent=-1;
  }else if (expr->key == EXPR_BREAK){
    str_add(&out,"break");
  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_c(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,"#");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"#");
  }

  if (indent>=0){str_add(&out,"\n");}
  return out;
}

str_t tree_to_py(str_t modname, expr_t* tree, map_t* functable, map_t* stttable, map_t* included){
  lift_scope(tree);

  str_t out = str_new();
  str_add(&out,"#########################################\n# ");
  str_add(&out,modname.data);
  for (int i = 0; i < 38-modname.len; i++){
    str_addch(&out,' ');
  }
  str_add(&out,"#\n#########################################\n");
  str_add(&out,"# Compiled by WAXC (Version ");
  str_add(&out,__DATE__);
  str_add(&out,")\n\n");


  str_add(&out,"# === WAX Standard Library BEGIN ===\n");
  str_add(&out,TEXT_std_py);
  str_add(&out,"# === WAX Standard Library END   ===\n\n");


  str_add(&out,"# === User Code            BEGIN ===\n\n");
  list_node_t* it = tree->children.head;

  while(it){
    expr_t* expr = (expr_t*)(it->data);

    str_add(&out,expr_to_py(expr,0).data);


    it = it->next;
  }
  str_add(&out,"# === User Code            END   ===\n");

  str_t mainstr = str_from("main",4);
  func_t* fun = func_lookup(&mainstr,functable);
  
  if (fun != NULL){
    if (!(fun->params.len)){
      str_add(&out,"exit(main())\n");
    }else{
      str_add(&out,"import sys\n");
      str_add(&out,"exit(main(sys.argv[1:]))\n");
    }
  }

  
  return out;

}




#endif
