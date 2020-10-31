#ifndef WAX_TO_WAT
#define WAX_TO_WAT

#include "text.c"
#include "parser.c"
#include "tac.c"
#include "common.c"


map_t* wat_functable = NULL;

list_t wat_strs;
int wat_str_ptr;

typedef struct wat_data_st {
  int offset;
  str_t data;
} wat_data_t;

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
    str_add(&out,";;type?;;");
  }
  return out;
}

str_t wat_mat_iter_predecl(expr_t* expr){
  str_t out = str_new();

  list_node_t* it = expr->children.head;
  while(it){
    expr_t* ex = (expr_t*)(it->data);
    
    if (ex->key == EXPR_FORIN){
      str_add(&out, "(local $tmp__it_");
      char s[32];
      sprintf(s,"%p",(void*)ex);
      str_add(&out, s);
      str_add(&out, " i32)");
    }

    str_add(&out,wat_mat_iter_predecl(ex).data);

    it = it->next;
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
      
      str_add(&out, "(i32.const ");
      str_add(&out, s);
      str_add(&out, ")");
      wat_data_t* d = (wat_data_t*)malloc(sizeof(wat_data_t));
      d->data = tok->val;
      d->offset = wat_str_ptr;

      list_add(&wat_strs, d);
      wat_str_ptr += tok->val.len-1; // +1 for null-term, -2 for quotes
    }

  }else if (expr->key == EXPR_IADD){ str_add(&out, "(i32.add "); WAT_CALL2
  }else if (expr->key == EXPR_ISUB){ str_add(&out, "(i32.sub "); WAT_CALL2
  }else if (expr->key == EXPR_IMUL){ str_add(&out, "(i32.mul "); WAT_CALL2 
  }else if (expr->key == EXPR_IDIV){ str_add(&out, "(i32.div_s "); WAT_CALL2 
  }else if (expr->key == EXPR_IGT ){ str_add(&out, "(i32.gt_s " ); WAT_CALL2 
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
  }else if (expr->key == EXPR_IGEQ){ str_add(&out, "(i32.ge_s "); WAT_CALL2 
  }else if (expr->key == EXPR_ILEQ){ str_add(&out, "(i32.le_s "); WAT_CALL2 
  }else if (expr->key == EXPR_FGEQ){ str_add(&out, "(f32.ge "); WAT_CALL2 
  }else if (expr->key == EXPR_FLEQ){ str_add(&out, "(f32.le "); WAT_CALL2 
  }else if (expr->key == EXPR_INEQ){ str_add(&out, "(i32.ne "); WAT_CALL2 
  }else if (expr->key == EXPR_FNEQ){ str_add(&out, "(f32.ne "); WAT_CALL2 
  }else if (expr->key == EXPR_PTRNEQ){ str_add(&out, "(i32.ne "); WAT_CALL2 

  }else if (expr->key == EXPR_FMOD){  
    str_add(&out, "(call $wax::fmod ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out, " ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out, ")");

  }else if (expr->key == EXPR_LAND){
    // tac'ed out
  }else if (expr->key == EXPR_LOR){
    // tac'ed out

  }else if (expr->key == EXPR_BNEG ){
    str_add(&out, "(i32.sub (i32.const -1) ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,"))");

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
    // tac'ed out

  }else if (expr->key == EXPR_WHILE){
    char s[64];
    sprintf(s,"tmp__block_%p",(void*)expr);
    str_add(&out, "block $");
    str_add(&out, s);
    str_add(&out, "\n");
    INDENT2(indent);
    str_t lpname = tmp_name("tmp__lp_");
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
    str_add(&out, "end\n");
    INDENT2(indent);
    str_add(&out, "end");

  }else if (expr->key == EXPR_FOR){
    // tac'ed out

  }else if (expr->key == EXPR_FORIN){
    char s[64];
    sprintf(s,"tmp__it_%p",(void*)expr);

    char sb[64];
    sprintf(sb,"tmp__block_%p",(void*)expr);
    
    str_add(&out, "(local.set $");
    str_add(&out, s);
    str_add(&out, " ");
    str_add(&out, "(call $w_map_iter_new ");
    str_add(&out, expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out, "))\n");

    INDENT2(indent);
    str_add(&out, "block $");
    str_add(&out, sb);
    str_add(&out, "\n");

    INDENT2(indent);
    str_t lpname = tmp_name("tmp_lp_");
    str_add(&out, "loop $");
    str_add(&out, lpname.data);
    str_add(&out, "\n");    
    INDENT2(indent+1);
    str_add(&out, "(if (local.get $");
    str_add(&out, s);
    str_add(&out, ") (then\n");

    INDENT2(indent+2);
    str_add(&out, "(local.set ");
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out, " ");
    str_add(&out, "(call $map_iter_key_h ");
    str_add(&out, expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out, " (local.get $");
    str_add(&out, s);
    str_add(&out, ")))\n");

    INDENT2(indent+2);
    str_add(&out, "(local.set ");
    str_add(&out, expr_to_wat(CHILD2,-1,'l').data);
    str_add(&out, " ");
    str_add(&out, "(call $map_iter_val ");
    str_add(&out, expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out, " (local.get $");
    str_add(&out, s);
    str_add(&out, ")))\n");

    str_add(&out, expr_to_wat(CHILDN,indent+2,'r').data);
    INDENT2(indent+2);
    str_add(&out, "(local.set $");
    str_add(&out, s);
    str_add(&out, " ");
    str_add(&out, "(call $map_iter_next ");
    str_add(&out, expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out, " (local.get $");
    str_add(&out, s);
    str_add(&out, ")))\n");


    str_add(&out, "\n");
    INDENT2(indent+2);
    str_add(&out, "(br $");
    str_add(&out, lpname.data);
    str_add(&out, ")\n");
    INDENT2(indent+1);
    str_add(&out, "))\n");
    INDENT2(indent);
    str_add(&out, "end\n");
    INDENT2(indent);
    str_add(&out, "end\n");

  }else if (expr->key == EXPR_FUNC){
    list_node_t* it = expr->children.head;

    str_add(&out, "(func $");

    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);

    str_add(&out, " (export \"");
    str_add(&out, funcname.data);
    str_add(&out, "\")");

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

    
    int drp = 0;
    func_t* fun = func_lookup(&funcname,wat_functable);
    if (fun->result->tag != TYP_VOD){
      if (!expr->parent || expr->parent->key == EXPR_THEN || expr->parent->key == EXPR_ELSE
        || expr->parent->key == EXPR_DO || expr->parent->key == EXPR_FUNCBODY
      ){
        str_add(&out,"(drop ");
        drp = 1;
      }

    }
    
    str_add(&out, "(call $");
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
    if (drp){
      str_add(&out, ")");
    }

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
    if (expr->key == EXPR_FUNCBODY){
      INDENT2(indent);
      str_add(&out,wat_mat_iter_predecl(expr).data);
      str_add(&out,"\n");
    }

    if (expr->key == EXPR_FUNCBODY){
      INDENT2(indent+1);
      str_add(&out,"(call $wax::push_stack)\n");
      INDENT2(indent);
    }

    list_node_t* it = expr->children.head;
    if (!it){
      str_add(&out,"\n");
    }
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (it==(expr->children.head)){
        str_add(&out,(char*)&expr_to_wat(ex,indent+1,'r').data[indent*2]);
      }else{
        str_add(&out,expr_to_wat(ex,indent+1,'r').data);
      }
      it = it->next;
    }

    if (expr->key == EXPR_FUNCBODY){
      INDENT2(indent+1);
      str_add(&out,"(call $wax::pop_stack)\n");
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
      str_add(&out, "(call $wax::int2str ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "(call $wax::flt2str ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "(call $wax::str2int ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "(call $wax::str2flt ");
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out, ")");
    }else{
      //wtf
      str_add(&out, expr_to_wat(CHILD1,-1,'r').data);
    }
  }else if (expr->key == EXPR_RETURN){

    str_add(&out,"(call $wax::pop_stack)\n");

    INDENT2(indent);

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
      // expr_t* ex = (expr_t*)(it->data);

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
      str_add(&out,")) (local.get $v)))\n");
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

      str_add(&out,"(call $wax::calloc (global.get $sizeof__");
      str_add(&out,typ->name.data);
      str_add(&out,"))");

    }else if (typ->tag == TYP_ARR){
      if (expr->children.len == 1){
        str_add(&out,"(call $wax::arr_new (i32.const 0))");
      }else{
        char s[32];
        sprintf(s,"%d",expr->children.len-1);
        str_add(&out,"(call $w__arr_lit");
        str_add(&out,s);
        list_node_t* it = expr->children.head->next;
        while (it){
          str_add(&out," ");
          if (((type_t*)CHILD1->term)->elem0->tag == TYP_FLT){
            str_add(&out,"(i32.reinterpret_f32 ");
            str_add(&out,expr_to_wat((expr_t*)(it->data),-1,'r').data);
            str_add(&out,")");
          }else{
            str_add(&out,expr_to_wat((expr_t*)(it->data),-1,'r').data);
          }
          it = it->next;
        }
        str_add(&out,")");
      }

    }else if (typ->tag == TYP_VEC){
      char s[32];
      sprintf(s, "%d", typ->size*4);

      if (expr->children.len == 1){
        str_add(&out,"(call $wax::calloc (i32.mul (i32.const 4) (i32.const ");
        str_add(&out,s);
        str_add(&out,")))");
      }else{
        int typtag = ((type_t*)CHILD1->term)->elem0->tag;
        if (typtag == TYP_INT){
          str_add(&out,"(call $w_vec_new_ints");
        }else if (typtag == TYP_FLT){
          str_add(&out,"(call $w_vec_new_flts");
        }else{
          str_add(&out,"(call $w_vec_new_ints");
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
      str_add(&out,"(call $wax::str_new ");
      if (CHILD2){
        str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
      }else{
        str_add(&out,"(i32.const 0)");
      }
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_FREE){
    if (CHILD1->type->tag == TYP_ARR){
      str_add(&out,"(call $wax::arr_free ");
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out,")");
    }else if (CHILD1->type->tag == TYP_MAP){
      str_add(&out,"(call $w_map_free ");
      str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
      str_add(&out,")");
    }else{
      str_add(&out,"(call $wax::free ");
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
    if (expr->type->tag == TYP_FLT){
      str_add(&out,"(f32.reinterpret_i32 ");
    }
    str_add(&out,"(call $wax::arr_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");
    if (expr->type->tag == TYP_FLT){
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_ARRSET){
    str_add(&out,"(call $wax::arr_set ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    if (CHILD3->type->tag == TYP_FLT){
      str_add(&out,"(i32.reinterpret_f32 ");
    }
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    if (CHILD3->type->tag == TYP_FLT){
      str_add(&out,")");
    }
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRINS){
    str_add(&out,"(call $wax::arr_insert ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRREM){
    str_add(&out,"(call $wax::arr_remove ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"(call $wax::arr_slice ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,"(call $wax::arr_length ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,"(call $w_map_len ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");


  }else if (expr->key == EXPR_MAPGET){
    str_add(&out,"(call $w_map_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"(call $w_map_remove ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"(call $w_map_set ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD3,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"(call $wax::str_len ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRGET){
    str_add(&out,"(call $wax::str_get ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRADD){
    tok_t* tok = ((tok_t*)(CHILD1->term));

    if (sym_lookup_nonglobal(&tok->val,expr)){
      str_add(&out,"(local.set ");
    }else{
      str_add(&out,"(global.set ");
    }
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out," (call $wax::str_add ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRCAT){
    tok_t* tok = ((tok_t*)(CHILD1->term));

    if (sym_lookup_nonglobal(&tok->val,expr)){
      str_add(&out,"(local.set ");
    }else{
      str_add(&out,"(global.set ");
    }
    str_add(&out, expr_to_wat(CHILD1,-1,'l').data);
    str_add(&out," (call $wax::str_cat ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRCPY){
    str_add(&out,"(call $wax::str_slice ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STREQL){
    str_add(&out,"(call $wax::str_cmp ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRNEQ){
    str_add(&out,"(i32.eqz (call $wax::str_cmp ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out," ");
    str_add(&out,expr_to_wat(CHILD2,-1,'r').data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"(call $wax::print ");
    str_add(&out,expr_to_wat(CHILD1,-1,'r').data);
    str_add(&out,")");

  }else if (expr->key == EXPR_BREAK){
    expr_t* rt = expr->parent;
    while (rt && rt->key != EXPR_WHILE && rt->key != EXPR_FORIN && rt->key != EXPR_FOR){
      rt = rt->parent;
    }
    char s[64];
    sprintf(s,"tmp__block_%p",(void*)rt);

    str_add(&out,"(br $");
    str_add(&out,s);
    str_add(&out,")");

  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_c(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,";;");
    str_add(&out,expr->rawkey.data);
    str_add(&out,";;\n");
  }

  if (indent>=0){str_add(&out,"\n");}
  return out;
}

str_t tree_to_wat(str_t modname, expr_t* tree, map_t* functable, map_t* stttable, map_t* included){
  compile_tac_tree(tree);
  lift_scope(tree);

  wat_functable = functable;
  wat_strs = list_new();

  str_t out = str_new();
  str_add(&out,";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n;; ");
  str_add(&out,modname.data);
  for (int i = 0; i < 37-modname.len; i++){
    str_addch(&out,' ');
  }
  str_add(&out,";;\n;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n");
  str_add(&out,";;Compiled by WAXC (Version ");
  str_add(&out,__DATE__);
  str_add(&out,");;\n\n");


  str_add(&out,"(module \n");

  str_addconst(&out,TEXT_std_import_wat);
  str_add(&out,"\n");

  str_add(&out,";;=== User Code            BEGIN ===;;\n");

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

  it = wat_strs.head;
  while (it){
    wat_data_t* s = (wat_data_t*)(it->data);
    str_add(&out,"(data (i32.const ");
    char offs[32];
    sprintf(offs,"%d",s->offset);
    str_add(&out,offs);
    str_add(&out,") ");
    str_add(&out,s->data.data);
    str_add(&out,")\n");
    it = it->next;
  }

  char wsps[32];
  sprintf(wsps, "%d", wat_str_ptr+3 & (-4)); //align4
  str_add(&out,"(global $wax::min_addr (mut i32) (i32.const ");
  str_add(&out, wsps);
  str_add(&out,"))\n");
  
  str_add(&out,";;=== User Code            END   ===;;\n\n");

  str_add(&out,";;=== WAX Standard Library BEGIN ===;;\n");
  str_addconst(&out,TEXT_std_malloc_wat);
  str_addconst(&out,TEXT_std_stack_wat);
  str_addconst(&out,TEXT_std_str_wat);
  str_addconst(&out,TEXT_std_arr_wat);
  str_add(&out,";;=== WAX Standard Library END   ===;;\n\n");

  str_add(&out,")\n");
  return out;

}




#endif
