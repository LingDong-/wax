#ifndef WAX_TO_WAT
#define WAX_TO_WAT

#include "text.c"
#include "parser.c"
#include "common.c"


list_t wat_strs;
int wat_str_ptr;

str_t type_to_wat(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"i32");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"f32");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"i32");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"i32");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"i32");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"i32");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"i32");
  }else{
    str_add(&out,"/*type?*/");
  }
  return out;
}

#define WAT_CALL2  str_add(&out, expr_to_wat(CHILD1,-1,'r').data );\
                   str_add(&out, " ");\
                   str_add(&out, expr_to_wat(CHILD2,-1,'r').data );\
                   str_add(&out, ")");


type_t* sym_lookup_nonglobal(str_t* s, expr_t* expr){
  if (!expr->parent){
    return NULL;
  }
  type_t* typ = sym_lookup_local(s,expr);
  if (typ){
    return typ;
  }
  if (expr->parent){
    return sym_lookup(s,expr->parent);
  }
  return NULL;
}





str_t expr_to_wat(expr_t* expr, int indent, char lr){
  // print_syntax_tree(expr,4);
  // printf("-----\n");

  expr_t* root = expr;
  while (root->parent){
    root = root->parent;
  }

  str_t out = str_new();
  INDENT2(indent);

  if (expr->key == EXPR_LET){
    /* skip -- variables must be declared at top of block */
    
  }else if (expr->key == EXPR_SET){
    tok_t* tok = ((tok_t*)(CHILD1->term));

    if (sym_lookup_nonglobal(&tok->val,expr)){
      str_add(&out,"(local.set ");
    }else{
      str_add(&out,"(global.set ");
    }
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out," ");
    str_add(&out, expr_to_wat(CHILD2,-1,'r').data );
    str_add(&out,")");

  }else if (expr->key == EXPR_TERM){
    tok_t* tok = ((tok_t*)(expr->term));
    if (tok->tag == TOK_IDT){
      if (lr == 'l'){
        str_add(&out, "$");
        str_add(&out, (tok->val.data));
      }else{
        if (sym_lookup_nonglobal(&tok->val,expr)){
          str_add(&out, "(local.get $");
        }else{
          str_add(&out, "(global.get $");
        }
        str_add(&out, (tok->val.data));
        str_add(&out, ")");
      }
    }else if (tok->tag == TOK_INT){
      str_add(&out, "(i32.const ");
      str_add(&out, (tok->val.data));
      str_add(&out, ")");
    }else if (tok->tag == TOK_FLT){
      str_add(&out, "(f32.const ");
      str_add(&out, (tok->val.data));
      str_add(&out, ")");
    }else if (tok->tag == TOK_STR){
      char s[32];
      sprintf(s, "%d", wat_str_ptr);
      wat_str_ptr += tok->val.len-1;
      str_add(&out, "(i32.const ");
      str_add(&out, s);
      str_add(&out, ")");
    }

  }else if (expr->key == EXPR_IADD){ str_add(&out, "(i32.add "); WAT_CALL2
  }else if (expr->key == EXPR_ISUB){ str_add(&out, "(i32.sub "); WAT_CALL2
  }else if (expr->key == EXPR_IMUL){ str_add(&out, "(i32.mul "); WAT_CALL2 
  }else if (expr->key == EXPR_IDIV){ str_add(&out, "(i32.div_s "); WAT_CALL2 
  }else if (expr->key == EXPR_IGT ){ str_add(&out, "(i32.gt " ); WAT_CALL2 
  }else if (expr->key == EXPR_ILT ){ str_add(&out, "(i32.lt_s " ); WAT_CALL2 
  }else if (expr->key == EXPR_FADD){ str_add(&out, "(f32.add "); WAT_CALL2
  }else if (expr->key == EXPR_FSUB){ str_add(&out, "(f32.sub "); WAT_CALL2
  }else if (expr->key == EXPR_FMUL){ str_add(&out, "(f32.mul "); WAT_CALL2 
  }else if (expr->key == EXPR_FDIV){ str_add(&out, "(f32.div_s "); WAT_CALL2 
  }else if (expr->key == EXPR_FGT ){ str_add(&out, "(f32.gt " ); WAT_CALL2 
  }else if (expr->key == EXPR_FLT ){ str_add(&out, "(f32.lt " ); WAT_CALL2
  }else if (expr->key == EXPR_IMOD){ str_add(&out, "(i32.rem_s "); WAT_CALL2 
  }else if (expr->key == EXPR_BAND){ str_add(&out, "(i32.and "); WAT_CALL2 
  }else if (expr->key == EXPR_BOR ){ str_add(&out, "(i32.or "); WAT_CALL2 
  }else if (expr->key == EXPR_XOR ){ str_add(&out, "(i32.xor "); WAT_CALL2 
  }else if (expr->key == EXPR_IEQ ){ str_add(&out, "(i32.eq "); WAT_CALL2 
  }else if (expr->key == EXPR_FEQ ){ str_add(&out, "(f32.eq "); WAT_CALL2 
  }else if (expr->key == EXPR_PTREQL){ str_add(&out, "(i32.eq "); WAT_CALL2 
  }else if (expr->key == EXPR_LNOT){ str_add(&out, "(i32.eqz "); WAT_CALL2 

  }else if (expr->key == EXPR_FMOD){  

  }else if (expr->key == EXPR_LAND){
    str_add(&out, "(i32.eqz (i32.eqz (i32.and ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data );\
    str_add(&out, " ");\
    str_add(&out, expr_to_wat(CHILD2,-1,'r').data );\
    str_add(&out, ")))");
  }else if (expr->key == EXPR_LOR){
    str_add(&out, "(i32.eqz (i32.eqz (i32.or ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data );\
    str_add(&out, " ");\
    str_add(&out, expr_to_wat(CHILD2,-1,'r').data );\
    str_add(&out, ")))");
  }else if (expr->key == EXPR_IGEQ){

  }else if (expr->key == EXPR_ILEQ){

  }else if (expr->key == EXPR_FGEQ){

  }else if (expr->key == EXPR_FLEQ){

  }else if (expr->key == EXPR_INEQ || expr->key == EXPR_PTRNEQ){  


  }else if (expr->key == EXPR_FNEQ){


  }else if (expr->key == EXPR_BNEG ){


  }else if (expr->key == EXPR_IF){
    str_add(&out, "(if ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out, " (then\n");
    str_add(&out, expr_to_wat(CHILD2,indent,'r').data);
    INDENT2(indent);
    str_add(&out, ")");
    if (CHILD3){

      str_add(&out, "(else\n");
      str_add(&out, expr_to_wat(CHILD3,indent,'r').data);
      INDENT2(indent);
      str_add(&out, ")");
    }
    str_add(&out, ")");

  }else if (expr->key == EXPR_TIF){


  }else if (expr->key == EXPR_WHILE){
    str_t lpname = tmp_name("tmp_lp_");
    str_add(&out, "loop $");
    str_add(&out, lpname.data);
    str_add(&out, "\n");
    INDENT2(indent+1);
    str_add(&out, "(if ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out, " (then\n");
    str_add(&out, expr_to_wat(CHILD2,indent+1,'r').data);
    str_add(&out, "\n");
    INDENT2(indent+2);
    str_add(&out, "(br $");
    str_add(&out, lpname.data);
    str_add(&out, ")\n");
    INDENT2(indent+1);
    str_add(&out, "))\n");
    INDENT2(indent);
    str_add(&out, "end");

  }else if (expr->key == EXPR_FOR){
    str_t lpname = tmp_name("tmp_lp_");
    str_add(&out, "(block \n");
    INDENT2(indent+1)
    str_add(&out, "(local ");
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out, " i32)\n");
    INDENT2(indent+1)
    str_add(&out, "(local.set ");
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out, " ");
    str_add(&out, expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out, ")\n");
    INDENT2(indent+1)
    str_add(&out, "loop $");
    str_add(&out, lpname.data);
    str_add(&out, "\n");
    INDENT2(indent+2);
    str_add(&out, "(if ");
    str_add(&out, expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out, " (then\n");
    str_add(&out, expr_to_wat(CHILDN,indent+2,'r').data);
    str_add(&out, "\n");
    INDENT2(indent+3);
    str_add(&out, "(local.set ");
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out, " (i32.add ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out, " ");
    str_add(&out, expr_to_wat(CHILD4,-1,'r').data);
    str_add(&out, "))\n");
    INDENT2(indent+3);
    str_add(&out, "(br $");
    str_add(&out, lpname.data);
    str_add(&out, ")\n");
    INDENT2(indent+2);
    str_add(&out, "))\n");
    INDENT2(indent+1);
    str_add(&out, "end\n");
    INDENT2(indent);
    str_add(&out,")");

  // }else if (expr->key == EXPR_FORIN){
  //   str_t mpname = tmp_name("tmp_mp_");
  //   str_t lpname = tmp_name("tmp_lp_");
  //   str_t itname = tmp_name("tmp_it_");

  //   str_add(&out, "w_map_t* ");
  //   str_add(&out, mpname.data);
  //   str_add(&out, "=");
  //   str_add(&out, expr_to_wat(CHILD3,-1).data);
  //   str_add(&out, ";\n");
  //   INDENT2(indent);
  //   str_add(&out, "for(int ");
  //   str_add(&out, lpname.data);
  //   str_add(&out,"=0;");
  //   str_add(&out, lpname.data);
  //   str_add(&out,"<W_NUM_MAP_SLOTS;");
  //   str_add(&out, lpname.data);
  //   str_add(&out, "++){\n");
  //   INDENT2(indent+1);
  //   str_add(&out, "w_slot_t* ");
  //   str_add(&out, itname.data);
  //   str_add(&out, "=");
  //   str_add(&out, mpname.data);
  //   str_add(&out, "->slots[");
  //   str_add(&out, lpname.data);
  //   str_add(&out, "];\n");
  //   INDENT2(indent+1);
  //   str_add(&out, "while(");
  //   str_add(&out, itname.data);
  //   str_add(&out, "){\n");
  //   INDENT2(indent+2);
  //   str_add(&out, type_to_wat(CHILD3->type->elem0).data);
  //   str_add(&out, " ");
  //   str_add(&out, expr_to_wat(CHILD1,-1).data);
  //   str_add(&out, "=(");
  //   str_add(&out, type_to_wat(CHILD3->type->elem0).data);
  //   str_add(&out, ")(");
  //   str_add(&out, itname.data);
  //   str_add(&out, "->key);\n");
  //   INDENT2(indent+2);
  //   str_add(&out, type_to_wat(CHILD3->type->elem1).data);
  //   str_add(&out, " ");
  //   str_add(&out, expr_to_wat(CHILD2,-1).data);
  //   str_add(&out, "=(");
  //   str_add(&out, type_to_wat(CHILD3->type->elem1).data);
  //   str_add(&out, ")(");
  //   str_add(&out, itname.data);
  //   str_add(&out,"->data);\n");
  //   INDENT2(indent+2);
  //   str_add(&out, "{\n");
  //   str_add(&out, expr_to_wat(CHILDN,indent+2).data);
  //   INDENT2(indent+2);
  //   str_add(&out, "}\n");
  //   INDENT2(indent+2);
  //   str_add(&out, itname.data);
  //   str_add(&out, "=");
  //   str_add(&out, itname.data);
  //   str_add(&out, "->next;\n");
  //   INDENT2(indent+1);
  //   str_add(&out, "}\n");
  //   INDENT2(indent);
  //   str_add(&out, "}");


  }else if (expr->key == EXPR_FUNC){
    list_node_t* it = expr->children.head;

    str_add(&out, "(func $");

    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);
    it = expr->children.head->next;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key != EXPR_PARAM){
        break;
      }
      
      str_add(&out," ");
      
      str_add(&out,"(param $");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out," ");
      str_add(&out,type_to_wat(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out,")");
      it = it->next;
    }
    if (((expr_t*)(it->data))->key == EXPR_RESULT){
      str_add(&out," (result ");
      str_add(&out,type_to_wat(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term) ).data);
      str_add(&out,")");
    }
    str_add(&out, "\n");
    str_add(&out, expr_to_wat(CHILDN,indent,'r').data);
    INDENT2(indent);
    str_add(&out, ")");

  }else if (expr->key == EXPR_CALL){
    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, "(call ");
    str_add(&out, funcname.data);


    list_node_t* it = expr->children.head->next;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key == EXPR_RESULT){
        break;
      }
      str_add(&out," ");

      str_add(&out, expr_to_wat(((expr_t*)(it->data)),-1,'r').data );

      it = it->next;
    }
    str_add(&out, ")");

  }else if (expr->key == EXPR_THEN || expr->key == EXPR_ELSE || expr->key == EXPR_DO || expr->key == EXPR_FUNCBODY){
    for (int k = 0; k < NUM_MAP_SLOTS; k++){
      if (!expr->symtable.slots[k]){
        continue;
      }
      list_node_t* it = expr->symtable.slots[k]->head;
      while (it){
        sym_t* sym = (sym_t*)(it->data);
        INDENT2(indent);
        str_add(&out, "(local $");
        str_add(&out, sym->name.data);
        str_add(&out, " ");
        str_add(&out, type_to_wat(&sym->type).data);
        str_add(&out, ")\n");
        INDENT2(indent);
        it = it->next;
      }
    }

    list_node_t* it = expr->children.head;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (it==(expr->children.head)){
        str_add(&out,(char*)&expr_to_wat(ex,indent+1,'r').data[indent*2]);
      }else{
        str_add(&out,expr_to_wat(ex,indent+1,'r').data);
      }
      it = it->next;
    }

    indent=-1;

  }else if (expr->key == EXPR_CAST){
    type_t* typl = CHILD1->type;
    type_t* typr = (type_t*)(CHILD2->term);
    if (typl->tag == TYP_INT && typr->tag == TYP_FLT){
      str_add(&out, "(f32.convert_i32_s ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_INT){
      str_add(&out, "(i32.trunc_f32_s ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_INT && typr->tag == TYP_STR){
      str_add(&out, "(call $w_int2str ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "(call $w_flt2str ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "(call $w_str2int ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "(call $w_flt2str");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else{
      //wtf
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    }
  }else if (expr->key == EXPR_RETURN){
    if (CHILD1){
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    }
    str_add(&out,"\n");
    INDENT2(indent);
    str_add(&out,"return");

  }else if (expr->key == EXPR_STRUCT){
    str_t sttname = ((tok_t*)(CHILD1->term))->val;


    list_node_t* it = expr->children.head->next;

    int offs = 0;
    char s[32];
    while(it){
      expr_t* ex = (expr_t*)(it->data);

      sprintf(s,"%d",offs);

      char* typstr = type_to_wat( (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data;

      str_add(&out,"(func $get__");
      str_add(&out,sttname.data);
      str_add(&out,"__");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out," (param $ptr i32) (result ");
      str_add(&out,typstr);
      str_add(&out,") (");
      str_add(&out,typstr);
      str_add(&out,".load (i32.add (local.get $ptr) (i32.const ");
      str_add(&out,s);
      str_add(&out,"))))\n");
      INDENT2(indent);
      str_add(&out,"(func $set__");
      str_add(&out,sttname.data);
      str_add(&out,"__");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out," (param $ptr i32) (param $v ");
      str_add(&out,typstr);
      str_add(&out,") (");
      str_add(&out,typstr);
      str_add(&out,".store (i32.add (local.get $ptr) (i32.const ");
      str_add(&out,s);
      str_add(&out,")) (local.get $v))))\n");
      INDENT2(indent);

      it = it->next;

      offs += 4;
    }
    str_add(&out,"(global $sizeof__");
    str_add(&out,sttname.data);
    str_add(&out," i32 (i32.const ");
    sprintf(s,"%d",offs);
    str_add(&out,s);
    str_add(&out,"))");

  }else if (expr->key == EXPR_NOTNULL){
    str_add(&out,"(i32.eqz ");
    str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_SETNULL){
    tok_t* tok = ((tok_t*)(CHILD1->term));

    if (sym_lookup_nonglobal(&tok->val,expr)){
      str_add(&out,"(local.set ");
    }else{
      str_add(&out,"(global.set ");
    }

    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out," (i32.const 0))");

  }else if (expr->key == EXPR_ALLOC){
    type_t* typ = (type_t*)(CHILD1->term);

    if (typ->tag == TYP_STT){

      str_add(&out,"(call $w_calloc (global.get $sizeof__");
      str_add(&out,typ->name.data);
      str_add(&out,"))");

    }else if (typ->tag == TYP_ARR){
      if (expr->children.len == 1){
        str_add(&out,"(call $w_arr_new (i32.const 4))");
      }else{
        char s[32];
        sprintf(s,"%d",expr->children.len-1);
        if (((type_t*)CHILD1->term)->elem0->tag == TYP_INT){
          str_add(&out,"(call $w_arr_new_ints");
        }else if (((type_t*)CHILD1->term)->elem0->tag == TYP_FLT){
          str_add(&out,"(call $w_arr_new_flts");
        }else{
          str_add(&out,"(call $w_arr_new_strs");
        }
        str_add(&out,s);
        list_node_t* it = expr->children.head->next;
        while (it){
          str_add(&out," ");
          str_add(&out,expr_to_wat((expr_t*)(it->data),-1,'r').data);
          it = it->next;
        }
        str_add(&out,")");
      }

    }else if (typ->tag == TYP_VEC){
      char s[32];
      sprintf(s, "%d", typ->size*4);

      if (expr->children.len == 1){
        str_add(&out,"(call $w_calloc (i32.const ");
        str_add(&out,s);
        str_add(&out,"))");
      }else{
        int typtag = ((type_t*)CHILD1->term)->elem0->tag;
        if (typtag == TYP_INT){
          str_add(&out,"(call $w_vec_new_ints");
        }else if (typtag == TYP_FLT){
          str_add(&out,"(call $w_vec_new_flts");
        }else{
          str_add(&out,"(call $w_vec_new_strs");
        }
        str_add(&out,s);
        list_node_t* it = expr->children.head->next;
        while (it){
          str_add(&out," ");
          str_add(&out,expr_to_wat((expr_t*)(it->data),-1,'r').data);
          it = it->next;
        }
        str_add(&out,")");        
      }

    }else if (typ->tag == TYP_MAP){
      str_add(&out,"(call $w_map_new (i32.const 64))");

    }else if (typ->tag == TYP_STR){
      str_add(&out,"(call $w_str_new ");
      if (CHILD2){
        str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
      }else{
        str_add(&out,"(i32.const 0)");
      }
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_FREE){
    if (CHILD1->type->tag == TYP_ARR){
      str_add(&out,"(call $w_arr_free ");
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out,")");
    }else if (CHILD1->type->tag == TYP_MAP){
      str_add(&out,"(call $w_map_free ");
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out,")");
    }else{
      str_add(&out,"(call $w_free ");
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_STRUCTGET){
    str_add(&out,"(call $get__");
    str_add(&out,CHILD1->type->name.data);
    str_add(&out,"__");
    str_add(&out,((tok_t*)(CHILD2->term))->val.data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRUCTSET){
    str_add(&out,"(call $set__");
    str_add(&out,CHILD1->type->name.data);
    str_add(&out,"__");
    str_add(&out,((tok_t*)(CHILD2->term))->val.data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_VECGET){
    str_add(&out,"(call $w_vec_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");
  }else if (expr->key == EXPR_VECSET){
    str_add(&out,"(call $w_vec_set ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRGET){
    str_add(&out,"(call $w_arr_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRSET){
    str_add(&out,"(call $w_arr_set ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRINS){
    str_add(&out,"(call $w_arr_insert ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRREM){
    str_add(&out,"(call $w_arr_remove ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"(call $w_arr_slice ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,"(call $w_arr_len ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,"(call $w_map_len ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");


  }else if (expr->key == EXPR_MAPGET){
    str_add(&out,"(call $w_map_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"(call $w_map_remove ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  // }else if (expr->key == EXPR_MAPSET){
  //   str_add(&out,"w_map_set((");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"),(");
  //   if (CHILD1->type->elem0->tag == TYP_STR){
  //     str_add(&out,"&(");
  //     str_add(&out,expr_to_wat(CHILD2,-1).data);
  //     str_add(&out,")),0");
  //   }else{
  //     str_add(&out,expr_to_wat(CHILD2,-1).data);
  //     str_add(&out,"),sizeof(");
  //     str_add(&out,type_to_wat(CHILD1->type->elem0).data);
  //     str_add(&out,")");
  //   }
  //   str_add(&out,",(");
  //   if (CHILD1->type->elem1->tag == TYP_FLT){
  //     str_add(&out,"(int64_t)reinterpret_f2i(");
  //     str_add(&out,expr_to_wat(CHILD3,-1).data);
  //     str_add(&out,")");
  //   }else if (CHILD1->type->elem1->tag == TYP_INT){
  //     str_add(&out,"(int64_t)(");
  //     str_add(&out,expr_to_wat(CHILD3,-1).data);
  //     str_add(&out,")");
  //   }else{
  //     str_add(&out,"(int64_t)(");
  //     str_add(&out,expr_to_wat(CHILD3,-1).data);
  //     str_add(&out,")");
  //   }
  //   str_add(&out,"))");


  // }else if (expr->key == EXPR_STRLEN){
  //   str_add(&out,"strlen(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,")");

  // }else if (expr->key == EXPR_STRGET){
  //   str_add(&out,"(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,")[");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"]");

  // }else if (expr->key == EXPR_STRADD){
  //   str_add(&out,"(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"=w_str_add(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,",");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"))");

  // }else if (expr->key == EXPR_STRCAT){
  //   str_add(&out,"(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"=w_str_cat(");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,",");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"))");

  // }else if (expr->key == EXPR_STRCPY){
  //   str_add(&out,"w_str_cpy((");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"),(");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"),(");
  //   str_add(&out,expr_to_wat(CHILD3,-1).data);
  //   str_add(&out,"))");

  // }else if (expr->key == EXPR_STREQL){
  //   str_add(&out,"(strcmp((");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"),(");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"))==0)");

  // }else if (expr->key == EXPR_STRNEQ){
  //   str_add(&out,"(strcmp((");
  //   str_add(&out,expr_to_wat(CHILD1,-1).data);
  //   str_add(&out,"),(");
  //   str_add(&out,expr_to_wat(CHILD2,-1).data);
  //   str_add(&out,"))!=0)");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"(call $w_print ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  // }else if (expr->key == EXPR_BREAK){
  //   str_add(&out,"break");

  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_c(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,"/**");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"**/");
  }

  if (indent>=0){str_add(&out,"\n");}
  return out;
}

str_t tree_to_wat(str_t modname, expr_t* tree, map_t* functable, map_t* stttable, map_t* included){
  lift_scope(tree);

  str_t out = str_new();
  // str_add(&out,"/*****************************************\n * ");
  // str_add(&out,modname.data);
  // for (int i = 0; i < 38-modname.len; i++){
  //   str_addch(&out,' ');
  // }
  // str_add(&out,"*\n *****************************************/\n");
  // str_add(&out,"/* Compiled by WAXC (Version ");
  // str_add(&out,__DATE__);
  // str_add(&out,")*/\n\n");
  // str_add(&out,"/*=== WAX Standard Library BEGIN ===*/\n");
  // str_addconst(&out,TEXT_std_c);
  // str_add(&out,"/*=== WAX Standard Library END   ===*/\n\n");
  // str_add(&out,"/*=== User Code            BEGIN ===*/\n");
  str_add(&out,"(module \n");
  list_node_t* it = tree->children.head;

  while(it){
    expr_t* expr = (expr_t*)(it->data);

    if (expr->key == EXPR_LET && it->next){
      expr_t* ex1 = (expr_t*)(it->next->data);
      if (ex1->key == EXPR_SET){
        expr_t* ex2 = (expr_t*)(ex1->children.head->data);

        if (ex2->key == EXPR_TERM){
          if (str_eq( &((tok_t*)(CHILD1->term))->val, ((tok_t*)(ex2->term))->val.data )){
            INDENT2(1);
            str_add(&out,"(global $");
            str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
            str_add(&out," (mut ");
            str_add(&out,type_to_wat( (type_t*)(CHILD2->term) ).data);
            str_add(&out,") ");
            str_add(&out,expr_to_wat( (expr_t*)(ex1->children.head->next->data),-1,'r').data);
            str_add(&out,")\n");
            it = it -> next -> next;
            continue;
          }
          
        }
      }
    }

    str_add(&out,expr_to_wat(expr,1,'r').data);


    it = it->next;
  }
  str_add(&out,")");

  // str_add(&out,"/*=== User Code            END   ===*/\n");

  return out;

}




#endif
