#ifndef WAX_TO_C
#define WAX_TO_C

#include "text.c"
#include "parser.c"
#include "common.c"

map_t* c_functable = NULL;

str_t type_to_c(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"int");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"float");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"struct ");
    str_add(&out,typ->u.name.data);
    str_add(&out,"*");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"w_arr_t*");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,type_to_c(typ->elem0).data);
    str_add(&out,"*");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"w_map_t*");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"char*");
  }else{
    str_add(&out,"/*type?*/");
  }
  return out;
}


str_t expr_to_c(expr_t* expr, int indent){
  // print_syntax_tree(expr,4);
  // printf("-----\n");
  str_t out = str_new();
  INDENT2(indent);

  if (expr->key == EXPR_LET){
    
    str_add(&out,type_to_c( (type_t*)(CHILD2->term) ).data);
    str_add(&out," ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"=0");
    
  }else if (expr->key == EXPR_SET){
    str_add(&out,"(");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out,"=");

    str_add(&out, expr_to_c(CHILD2,-1).data );
    str_add(&out,")");

  }else if (expr->key == EXPR_TERM){

    str_add(&out, ((tok_t*)(expr->term))->val.data);
  
  }else if (expr->key == EXPR_IADD || expr->key == EXPR_FADD ||
            expr->key == EXPR_ISUB || expr->key == EXPR_FSUB ||
            expr->key == EXPR_IMUL || expr->key == EXPR_FMUL ||
            expr->key == EXPR_IDIV || expr->key == EXPR_FDIV ||
            expr->key == EXPR_IGT  || expr->key == EXPR_FGT  ||
            expr->key == EXPR_ILT  || expr->key == EXPR_FLT  ||
            expr->key == EXPR_IGEQ || expr->key == EXPR_FGEQ ||
            expr->key == EXPR_ILEQ || expr->key == EXPR_FLEQ ||

            expr->key == EXPR_LAND || expr->key == EXPR_BAND ||
            expr->key == EXPR_LOR  || expr->key == EXPR_BOR  ||

            expr->key == EXPR_IMOD || 
            expr->key == EXPR_XOR  ||
            expr->key == EXPR_SHL  || expr->key == EXPR_SHR
    ){
    str_add(&out, "(");
    str_add(&out, expr_to_c(CHILD1,-1).data );
    str_add(&out, expr->rawkey.data);
    str_add(&out, expr_to_c(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_IEQ || expr->key == EXPR_FEQ || expr->key == EXPR_PTREQL){

    str_add(&out, "(!!("); //silences -Wparentheses-equality
    str_add(&out, expr_to_c(CHILD1,-1).data );
    str_add(&out, "==");
    str_add(&out, expr_to_c(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_INEQ || expr->key == EXPR_FNEQ || expr->key == EXPR_PTRNEQ){

    str_add(&out, "(");
    str_add(&out, expr_to_c(CHILD1,-1).data );
    str_add(&out, "!=");
    str_add(&out, expr_to_c(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_FMOD){
    str_add(&out, "fmod(");
    str_add(&out, expr_to_c(CHILD1,-1).data );
    str_add(&out, ",");
    str_add(&out, expr_to_c(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_BNEG || expr->key == EXPR_LNOT ){
    str_add(&out, "(");
    str_add(&out, expr->rawkey.data);
    str_add(&out, expr_to_c(CHILD1,-1).data );
    str_add(&out, ")");
  }else if (expr->key == EXPR_IF){
    str_add(&out, "if(");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, "){\n");
    str_add(&out, expr_to_c(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");
    if (CHILD3){

      str_add(&out, "else{\n");
      str_add(&out, expr_to_c(CHILD3,indent).data);
      INDENT2(indent);
      str_add(&out, "}");
    }

  }else if (expr->key == EXPR_TIF){
    str_add(&out, "((");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, ")?(");
    str_add(&out, expr_to_c(CHILD2,-1).data);
    str_add(&out, "):(");
    str_add(&out, expr_to_c(CHILD3,-1).data);
    str_add(&out, "))");

  }else if (expr->key == EXPR_WHILE){
    str_add(&out, "while(");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, "){\n");
    str_add(&out, expr_to_c(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FOR){
    str_add(&out, "for(int ");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, "=");
    str_add(&out, expr_to_c(CHILD2,-1).data);
    str_add(&out, ";");
    str_add(&out, expr_to_c(CHILD3,-1).data);
    str_add(&out, ";");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, "+=");
    str_add(&out, expr_to_c(CHILD4,-1).data);
    str_add(&out, "){\n");
    str_add(&out, expr_to_c(CHILDN,indent).data);
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FORIN){
    str_t mpname = tmp_name("tmp__mp_");
    str_t lpname = tmp_name("tmp__lp_");
    str_t itname = tmp_name("tmp__it_");
    str_t lbname = tmp_name("tmp__lb_");

    str_add(&out, "w_map_t* ");
    str_add(&out, mpname.data);
    str_add(&out, "=");
    str_add(&out, expr_to_c(CHILD3,-1).data);
    str_add(&out, ";\n");
    INDENT2(indent);
    str_add(&out, "for(int ");
    str_add(&out, lpname.data);
    str_add(&out,"=0;");
    str_add(&out, lpname.data);
    str_add(&out,"<W_NUM_MAP_SLOTS;");
    str_add(&out, lpname.data);
    str_add(&out, "++){\n");
    INDENT2(indent+1);
    str_add(&out, "w_slot_t* ");
    str_add(&out, itname.data);
    str_add(&out, "=");
    str_add(&out, mpname.data);
    str_add(&out, "->slots[");
    str_add(&out, lpname.data);
    str_add(&out, "];\n");
    INDENT2(indent+1);
    str_add(&out, lbname.data);
    str_add(&out, ":\n");
    INDENT2(indent+1);
    str_add(&out, "if(");
    str_add(&out, itname.data);
    str_add(&out, "){\n");
    INDENT2(indent+2);
    str_add(&out, type_to_c(CHILD3->type->elem0).data);
    str_add(&out, " ");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out, "=");
    if (CHILD3->type->elem0->tag == TYP_FLT){
      str_add(&out, "*((float*)(");
      str_add(&out, itname.data);
      str_add(&out, "->key));\n");
    }else if (CHILD3->type->elem0->tag == TYP_INT){
      str_add(&out, "*((int*)(");
      str_add(&out, itname.data);
      str_add(&out, "->key));\n");
    }else{
      str_add(&out, "(char*)(");
      str_add(&out, itname.data);
      str_add(&out, "->key);\n");
    }
    INDENT2(indent+2);
    str_add(&out, type_to_c(CHILD3->type->u.elem1).data);
    str_add(&out, " ");
    str_add(&out, expr_to_c(CHILD2,-1).data);
    str_add(&out, "=");

    if (CHILD3->type->u.elem1->tag != TYP_FLT){
      str_add(&out, "(");
      str_add(&out, type_to_c(CHILD3->type->u.elem1).data);
      str_add(&out, ")(");
      str_add(&out, itname.data);
      str_add(&out,"->data);\n");
    }else{
      str_add(&out, "w_reinterp_i2f(");
      str_add(&out, "(int)(int64_t)(");
      str_add(&out, itname.data);
      str_add(&out, "->data));\n");
    }
    INDENT2(indent+2);
    str_add(&out, "{\n");
    str_add(&out, expr_to_c(CHILDN,indent+2).data);
    INDENT2(indent+2);
    str_add(&out, "}\n");
    INDENT2(indent+2);
    str_add(&out, itname.data);
    str_add(&out, "=");
    str_add(&out, itname.data);
    str_add(&out, "->next;\n");
    INDENT2(indent+2);
    str_add(&out, "goto ");
    str_add(&out, lbname.data);
    str_add(&out, ";\n");
    INDENT2(indent+1);
    str_add(&out, "}\n");
    INDENT2(indent);
    str_add(&out, "}");


  }else if (expr->key == EXPR_FUNC || expr->key == EXPR_FUNCHEAD){
    list_node_t* it = expr->children.head;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key == EXPR_RESULT){
        break;
      }
      it = it->next;
    }
    if (it && ((expr_t*)(it->data))->key == EXPR_RESULT){
      str_add(&out,type_to_c(  ((expr_t*)(((expr_t*)(it->data))->children.head->data))->type ).data);
    }else{
      str_add(&out,"void");
    }
    
    str_add(&out, " ");

    str_t funcname = ((tok_t*)(CHILD1->term))->val;
    if (str_eq(&funcname,"main")){
      int num_params = func_lookup(&funcname,c_functable)->params.len;
      if (num_params != 0){
        funcname = str_from("main_",5);
      }
    }

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
      // str_add(&out,ex->rawkey.data);
      str_add(&out,type_to_c(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out," ");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);

      it = it->next;
    }
    if (expr->key == EXPR_FUNC){
      str_add(&out, "){\n");
      str_add(&out, expr_to_c(CHILDN,indent).data);
      INDENT2(indent);
      str_add(&out, "}\n");
      indent = -1;
    }else{
      str_add(&out, ")");
    }

  }else if (expr->key == EXPR_CALL){
    str_t funcname = ((tok_t*)(CHILD1->term))->val;
    if (str_eq(&funcname,"main")){
      int num_params = func_lookup(&funcname,c_functable)->params.len;
      if (num_params != 0){
        funcname = str_from("main_",5);
      }
    }

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

      str_add(&out, expr_to_c(((expr_t*)(it->data)),-1).data );

      it = it->next;
    }
    str_add(&out, ")");

  }else if (expr->key == EXPR_THEN || expr->key == EXPR_ELSE || expr->key == EXPR_DO || expr->key == EXPR_FUNCBODY){
    list_node_t* it = expr->children.head;
    if (!it){
      str_add(&out,"\n");
    }
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (it==(expr->children.head)){
        str_add(&out,(char*)&expr_to_c(ex,indent+1).data[indent*2]);
      }else{
        str_add(&out,expr_to_c(ex,indent+1).data);
      }
      it = it->next;
    }

    indent=-1;

  }else if (expr->key == EXPR_CAST){
    type_t* typl = CHILD1->type;
    type_t* typr = (type_t*)(CHILD2->term);
    if (typl->tag == TYP_INT && typr->tag == TYP_FLT){
      str_add(&out, "((float)");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_INT){
      str_add(&out, "((int)");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_INT && typr->tag == TYP_STR){
      str_add(&out, "w_int2str(");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ").data");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "w_flt2str(");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ").data");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "atoi(");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ")");
    
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "((float)atof(");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, "))");
    }else{
      //wtf
      str_add(&out, "(");
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out, ")");
    }
  }else if (expr->key == EXPR_RETURN){
    str_add(&out,"return");
    if (CHILD1){
      str_add(&out," ");
      str_add(&out,expr_to_c(CHILD1,-1).data);
    }
  }else if (expr->key == EXPR_STRUCT){
    str_add(&out,"struct ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"{\n");

    list_node_t* it = expr->children.head->next;

    while(it){
      // expr_t* ex = (expr_t*)(it->data);

      INDENT2(indent+1);

      str_add(&out,type_to_c(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out," ");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out,";\n");
      it = it->next;
    }
    INDENT2(indent);
    str_add(&out,"}");

  }else if (expr->key == EXPR_NOTNULL){
    str_add(&out,"(");
    str_add(&out, expr_to_c(CHILD1,-1).data);
    str_add(&out,"!=NULL)");

  }else if (expr->key == EXPR_SETNULL){
    if (!CHILD2){
      str_add(&out, expr_to_c(CHILD1,-1).data);
      str_add(&out,"=NULL");
    }else{
      if (CHILD1->type->tag == TYP_STT){
        str_add(&out,expr_to_c(CHILD1,-1).data);
        str_add(&out,"->");
        str_add(&out,expr_to_c(CHILD2,-1).data);
        str_add(&out,"=NULL");
      }else if (CHILD1->type->tag == TYP_ARR){
        str_add(&out,"(w_arr_set(");
        str_add(&out,type_to_c(CHILD1->type->elem0).data);
        str_add(&out,",(");
        str_add(&out,expr_to_c(CHILD1,-1).data);
        str_add(&out,"),(");
        str_add(&out,expr_to_c(CHILD2,-1).data);
        str_add(&out,"),NULL");
        str_add(&out,"))");
      }else if (CHILD1->type->tag == TYP_VEC){
        str_add(&out,"((");
        str_add(&out,expr_to_c(CHILD1,-1).data);
        str_add(&out,")[");
        str_add(&out,expr_to_c(CHILD2,-1).data);
        str_add(&out,"])=NULL");

      }
    }

  }else if (expr->key == EXPR_ALLOC){
    type_t* typ = (type_t*)(CHILD1->term);

    if (typ->tag == TYP_STT){
      str_add(&out,"(");
      str_add(&out,type_to_c(typ).data);
      str_add(&out,")");
      str_add(&out,"calloc(sizeof(struct ");
      str_add(&out,typ->u.name.data);
      str_add(&out,"),1)");

    }else if (typ->tag == TYP_ARR){
      if (expr->children.len == 1){
        str_add(&out,"w_arr_new(");
        str_add(&out,type_to_c(typ->elem0).data);
        str_add(&out,")");
      }else{
        char s[32];
        snprintf(s,sizeof(s),"%d",expr->children.len-1);
        if (((type_t*)CHILD1->term)->elem0->tag == TYP_INT){
          str_add(&out,"w_arr_new_ints(");
        }else if (((type_t*)CHILD1->term)->elem0->tag == TYP_FLT){
          str_add(&out,"w_arr_new_flts(");
        }else{
          str_add(&out,"w_arr_new_strs(");
        }
        str_add(&out,s);
        list_node_t* it = expr->children.head->next;
        while (it){
          str_add(&out,",(");
          str_add(&out,expr_to_c((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,")");
      }

    }else if (typ->tag == TYP_VEC){
      char s[32];
      snprintf(s, sizeof(s),"%d", typ->u.size);

      if (expr->children.len == 1){
        str_add(&out,"(");
        str_add(&out,type_to_c(typ->elem0).data);
        str_add(&out,"*)");
        str_add(&out,"calloc(sizeof(");
        str_add(&out,type_to_c(typ->elem0).data);
        str_add(&out,"),");
        str_add(&out,s);
        str_add(&out,")");
      }else{
        int typtag = ((type_t*)CHILD1->term)->elem0->tag;
        if (typtag == TYP_INT){
          str_add(&out,"w_vec_new_ints(");
        }else if (typtag == TYP_FLT){
          str_add(&out,"w_vec_new_flts(");
        }else{
          str_add(&out,"w_vec_new_strs(");
        }
        str_add(&out,s);
        list_node_t* it = expr->children.head->next;
        while (it){
          str_add(&out,",(");
          str_add(&out,expr_to_c((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,")");        
      }

    }else if (typ->tag == TYP_MAP){
      if (typ->elem0->tag == TYP_STR){
        str_add(&out,"w_map_new(1)");
      }else{
        str_add(&out,"w_map_new(0)");
      }
    }else if (typ->tag == TYP_STR){
      str_add(&out,"w_str_new(");
      if (CHILD2){
        str_add(&out,expr_to_c(CHILD2,-1).data);
      }else{
        str_add(&out,"\"\"");
      }
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_FREE){
    if (CHILD1->type->tag == TYP_ARR){
      str_add(&out,"w_free_arr(");
      str_add(&out,expr_to_c(CHILD1,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->tag == TYP_MAP){
      str_add(&out,"w_free_map(");
      str_add(&out,expr_to_c(CHILD1,-1).data);
      str_add(&out,")");
    }else{
      str_add(&out,"w_free(");
      str_add(&out,expr_to_c(CHILD1,-1).data);
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_STRUCTGET){
    //str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"->");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    //str_add(&out,")");

  }else if (expr->key == EXPR_STRUCTSET){
    //str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"->");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"=");
    str_add(&out,expr_to_c(CHILD3,-1).data);

  }else if (expr->key == EXPR_VECGET){
    str_add(&out,"((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"])");
  }else if (expr->key == EXPR_VECSET){
    str_add(&out,"((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"])=");
    str_add(&out,expr_to_c(CHILD3,-1).data);

  }else if (expr->key == EXPR_ARRGET){
    str_add(&out,"((");
    str_add(&out,type_to_c(CHILD1->type->elem0).data);
    str_add(&out,")w_arr_get(");
    str_add(&out,type_to_c(CHILD1->type->elem0).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_ARRSET){
    str_add(&out,"w_arr_set(");
    str_add(&out,type_to_c(CHILD1->type->elem0).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRINS){
    str_add(&out,"w_arr_insert(");
    str_add(&out,type_to_c(CHILD1->type->elem0).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRREM){
    str_add(&out,"w_arr_remove(");
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"w_arr_slice(");
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"->len)");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"->len)");


  }else if (expr->key == EXPR_MAPGET){
    if (CHILD1->type->u.elem1->tag == TYP_FLT){
      str_add(&out,"w_reinterp_i2f((int)");
    }else{
      str_add(&out,"((");
      str_add(&out,type_to_c(CHILD1->type->u.elem1).data);
      str_add(&out,")");
    }
    str_add(&out,"w_map_get((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),");
    if (CHILD1->type->elem0->tag == TYP_STR){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_INT){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_FLT){
      str_add(&out,"(int64_t)w_reinterp_f2i(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }
    str_add(&out,"))");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"w_map_remove((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),");
    if (CHILD1->type->elem0->tag == TYP_STR){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_INT){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_FLT){
      str_add(&out,"(int64_t)w_reinterp_f2i(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"w_map_set((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),");
    if (CHILD1->type->elem0->tag == TYP_STR){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_INT){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->elem0->tag == TYP_FLT){
      str_add(&out,"(int64_t)w_reinterp_f2i(");
      str_add(&out,expr_to_c(CHILD2,-1).data);
      str_add(&out,")");
    }
    str_add(&out,",(");
    if (CHILD1->type->u.elem1->tag == TYP_FLT){
      str_add(&out,"(int64_t)w_reinterp_f2i(");
      str_add(&out,expr_to_c(CHILD3,-1).data);
      str_add(&out,")");
    }else if (CHILD1->type->u.elem1->tag == TYP_INT){
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD3,-1).data);
      str_add(&out,")");
    }else{
      str_add(&out,"(int64_t)(");
      str_add(&out,expr_to_c(CHILD3,-1).data);
      str_add(&out,")");
    }
    str_add(&out,"))");


  }else if (expr->key == EXPR_STRLEN){
    str_add(&out,"strlen(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRGET){
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"]");

  }else if (expr->key == EXPR_STRADD){
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"=w_str_add(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRCAT){
    str_add(&out,"(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"=w_str_cat(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,",");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRCPY){
    str_add(&out,"w_str_cpy((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_c(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STREQL){
    str_add(&out,"(strcmp((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"))==0)");

  }else if (expr->key == EXPR_STRNEQ){
    str_add(&out,"(strcmp((");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_c(CHILD2,-1).data);
    str_add(&out,"))!=0)");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"puts(");
    str_add(&out,expr_to_c(CHILD1,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_EXTERN){
    //skip
    indent=-1;
  }else if (expr->key == EXPR_BREAK){
    str_add(&out,"break");
  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_c(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,"/**");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"**/");
  }

  if (indent>=0){str_add(&out,";\n");}
  return out;
}

str_t tree_to_c(str_t modname, expr_t* tree, map_t* functable, map_t* stttable){
  c_functable = functable;

  str_t out = str_new();
  str_add(&out,"/*****************************************\n * ");
  str_add(&out,modname.data);
  for (int i = 0; i < 38-modname.len; i++){
    str_addch(&out,' ');
  }
  str_add(&out,"*\n *****************************************/\n");
  str_add(&out,"/* Compiled by WAXC (Version ");
  str_add(&out,__DATE__);
  str_add(&out,")*/\n\n");
  str_add(&out,"/*=== WAX Standard Library BEGIN ===*/\n");
  str_add(&out,TEXT_std_c);
  str_add(&out,"/*=== WAX Standard Library END   ===*/\n\n");
  str_add(&out,"/*=== User Code            BEGIN ===*/\n");
  str_add(&out,"\n");
  list_node_t* it = tree->children.head;

  while(it){
    expr_t* expr = (expr_t*)(it->data);

    if (expr->key == EXPR_LET && it->next){
      expr_t* ex1 = (expr_t*)(it->next->data);
      if (ex1->key == EXPR_SET){
        expr_t* ex2 = (expr_t*)(ex1->children.head->data);

        if (ex2->key == EXPR_TERM){
          if (str_eq( &((tok_t*)(CHILD1->term))->val, ((tok_t*)(ex2->term))->val.data )){


            str_add(&out,type_to_c( (type_t*)(CHILD2->term) ).data);
            str_add(&out," ");
            str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
            str_add(&out,"=");
            str_add(&out,expr_to_c( (expr_t*)(ex1->children.head->next->data),-1).data);

            str_add(&out,";\n");
            it = it -> next -> next;
            continue;
          }
          
        }
      }
    }

    str_add(&out,expr_to_c(expr,0).data);


    it = it->next;
  }
  str_add(&out,"/*=== User Code            END   ===*/\n");

  str_t mainstr = str_from("main",4);
  func_t* fun = func_lookup(&mainstr,functable);
  if (fun != NULL){
    if (fun->params.len){
      str_add(&out,"int main(int argc, char** argv){\n");
      str_add(&out,"  w_arr_t* args = w_arr_new(char*);\n");
      str_add(&out,"  for (int i = 0; i < argc; i++){\n");
      str_add(&out,"    w_arr_insert(char*,args,i,argv[i])\n");
      str_add(&out,"  }\n");
      str_add(&out,"  int code = main_(args);\n");
      str_add(&out,"  w_free_arr(args);\n");
      str_add(&out,"  return code;\n");
      str_add(&out,"}\n");
    }
  }
  return out;

}




#endif
