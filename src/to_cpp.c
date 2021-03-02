#ifndef WAX_TO_CPP
#define WAX_TO_CPP

#include "text.c"
#include "parser.c"
#include "common.c"


str_t type_to_cpp(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"int");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"float");
  }else if (typ->tag == TYP_STT){
    str_add(&out,typ->u.name.data);
    str_add(&out,"*");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"std::vector<");
    str_add(&out,type_to_cpp(typ->elem0).data);
    str_add(&out,">*");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"std::array<");
    str_add(&out,type_to_cpp(typ->elem0).data);
    str_add(&out,",");
    char s[32];
    snprintf(s,sizeof(s),"%d",typ->u.size);
    str_add(&out,s);
    str_add(&out,">*");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"std::map<");
    str_add(&out,type_to_cpp(typ->elem0).data);
    str_add(&out,",");
    str_add(&out,type_to_cpp(typ->u.elem1).data);
    str_add(&out,">*");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"std::string");
  }else{
    str_add(&out,"/*type?*/");
  }
  return out;
}

str_t zero_to_cpp(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"0");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"0.0f");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"NULL");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"NULL");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"NULL");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"NULL");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"\"\"");
  }else{
    str_add(&out,"/*zero?*/");
  }
  return out;
}

str_t expr_to_cpp(expr_t* expr, int indent){
  // print_syntax_tree(expr,4);
  // printf("-----\n");
  str_t out = str_new();
  INDENT2(indent);

  if (expr->key == EXPR_LET){
    
    str_add(&out,type_to_cpp( (type_t*)(CHILD2->term) ).data);
    str_add(&out," ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"=");
    str_add(&out,zero_to_cpp( (type_t*)(CHILD2->term) ).data);
    
  }else if (expr->key == EXPR_SET){
    str_add(&out,"(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"=");

    str_add(&out, expr_to_cpp(CHILD2,-1).data );
    str_add(&out,")");

  }else if (expr->key == EXPR_TERM){

    tok_t* tok = ((tok_t*)(expr->term));
    if (tok->tag == TOK_INT){
      if (tok->val.data[0] == '\''){
        str_add(&out, "(int)");
      }
    }
    str_add(&out, tok->val.data);
    if (tok->tag == TOK_FLT){
      str_add(&out, "f");
    }
  
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
    str_add(&out, type_to_cpp(expr->type).data );
    str_add(&out, ")(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data );
    str_add(&out, expr->rawkey.data);
    str_add(&out, expr_to_cpp(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_IEQ || expr->key == EXPR_FEQ || expr->key == EXPR_PTREQL){

    str_add(&out, "(!!("); //silences -Wparentheses-equality
    str_add(&out, expr_to_cpp(CHILD1,-1).data );
    str_add(&out, "==");
    str_add(&out, expr_to_cpp(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_INEQ || expr->key == EXPR_FNEQ || expr->key == EXPR_PTRNEQ){

    str_add(&out, "(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data );
    str_add(&out, "!=");
    str_add(&out, expr_to_cpp(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_FMOD){
    str_add(&out, "fmod(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data );
    str_add(&out, ",");
    str_add(&out, expr_to_cpp(CHILD2,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_BNEG || expr->key == EXPR_LNOT ){
    str_add(&out, "(");
    str_add(&out, expr->rawkey.data);
    str_add(&out, expr_to_cpp(CHILD1,-1).data );
    str_add(&out, ")");
  }else if (expr->key == EXPR_IF){
    str_add(&out, "if(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, "){\n");
    str_add(&out, expr_to_cpp(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");
    if (CHILD3){

      str_add(&out, "else{\n");
      str_add(&out, expr_to_cpp(CHILD3,indent).data);
      INDENT2(indent);
      str_add(&out, "}");
    }

  }else if (expr->key == EXPR_TIF){
    str_add(&out, "((");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, ")?(");
    str_add(&out, expr_to_cpp(CHILD2,-1).data);
    str_add(&out, "):(");
    str_add(&out, expr_to_cpp(CHILD3,-1).data);
    str_add(&out, "))");

  }else if (expr->key == EXPR_WHILE){
    str_add(&out, "while(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, "){\n");
    str_add(&out, expr_to_cpp(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FOR){
    str_add(&out, "for(int ");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, "=(");
    str_add(&out, expr_to_cpp(CHILD2,-1).data);
    str_add(&out, ");");
    str_add(&out, expr_to_cpp(CHILD3,-1).data);
    str_add(&out, ";");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, "+=(");
    str_add(&out, expr_to_cpp(CHILD4,-1).data);
    str_add(&out, ")){\n");
    str_add(&out, expr_to_cpp(CHILDN,indent).data);
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FORIN){
    str_t itname = tmp_name("tmp__it_");
    str_t mpname = tmp_name("tmp__mp_");

    str_add(&out, type_to_cpp(CHILD3->type).data);
    str_add(&out, " ");
    str_add(&out, mpname.data);
    str_add(&out, "=");
    str_add(&out, expr_to_c(CHILD3,-1).data);
    str_add(&out, ";\n");

    INDENT2(indent);
    str_add(&out, "for (");
    str_add(&out, type_to_cpp(CHILD3->type).data);
    str_add(&out, "::iterator ");
    str_add(&out, itname.data);
    str_add(&out, " = ");
    str_add(&out, mpname.data);
    str_add(&out, ".begin();");
    str_add(&out, itname.data);
    str_add(&out, " != ");
    str_add(&out, mpname.data);
    str_add(&out, ".end();");
    str_add(&out, itname.data);
    str_add(&out, "++){\n");

    INDENT2(indent+1);
    str_add(&out, type_to_cpp(CHILD3->type->elem0).data);
    str_add(&out, " ");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out, "=(");
    str_add(&out, type_to_cpp(CHILD3->type->elem0).data);
    str_add(&out, ")(");
    str_add(&out, itname.data);
    str_add(&out, "->first);\n");

    INDENT2(indent+1);
    str_add(&out, type_to_cpp(CHILD3->type->u.elem1).data);
    str_add(&out, " ");
    str_add(&out, expr_to_cpp(CHILD2,-1).data);
    str_add(&out, "=(");
    str_add(&out, type_to_cpp(CHILD3->type->u.elem1).data);
    str_add(&out, ")(");
    str_add(&out, itname.data);
    str_add(&out, "->second);\n");

    INDENT2(indent+1);
    str_add(&out, "{\n");
    str_add(&out, expr_to_cpp(CHILDN,indent+2).data);
    INDENT2(indent+1);
    str_add(&out, "}\n");

    INDENT2(indent);
    str_add(&out, "}");


  }else if (expr->key == EXPR_FUNC || expr->key == EXPR_FUNCHEAD){
    str_add(&out, "inline ");

    list_node_t* it = expr->children.head;
    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key == EXPR_RESULT){
        break;
      }
      it = it->next;
    }
    if (it && ((expr_t*)(it->data))->key == EXPR_RESULT){
      str_add(&out,type_to_cpp(  ((expr_t*)(((expr_t*)(it->data))->children.head->data))->type ).data);
    }else{
      str_add(&out,"void");
    }
    
    str_add(&out, " ");

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
      // str_add(&out,ex->rawkey.data);
      str_add(&out,type_to_cpp(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out," ");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);

      it = it->next;
    }
    if (expr->key == EXPR_FUNC){
      str_add(&out, "){\n");
      str_add(&out, expr_to_cpp(CHILDN,indent).data);
      INDENT2(indent);
      str_add(&out, "}\n");
      indent = -1;
    }else{
      str_add(&out, ")");
    }

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

      str_add(&out, expr_to_cpp(((expr_t*)(it->data)),-1).data );

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
        str_add(&out,(char*)&expr_to_cpp(ex,indent+1).data[indent*2]);
      }else{
        str_add(&out,expr_to_cpp(ex,indent+1).data);
      }
      it = it->next;
    }

    indent=-1;

  }else if (expr->key == EXPR_CAST){
    type_t* typl = CHILD1->type;
    type_t* typr = (type_t*)(CHILD2->term);
    if (typl->tag == TYP_INT && typr->tag == TYP_FLT){
      str_add(&out, "((float)");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_INT){
      str_add(&out, "((int)");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_INT && typr->tag == TYP_STR){
      str_add(&out, "std::to_string(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "std::to_string(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "std::stoi(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "std::stof(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }else{
      //wtf
      str_add(&out, "(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out, ")");
    }
  }else if (expr->key == EXPR_RETURN){
    str_add(&out,"return");
    if (CHILD1){
      str_add(&out," ");
      str_add(&out,expr_to_cpp(CHILD1,-1).data);
    }
  }else if (expr->key == EXPR_STRUCT){
    str_add(&out,"struct ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"{\n");

    list_node_t* it = expr->children.head->next;

    while(it){
      // expr_t* ex = (expr_t*)(it->data);

      INDENT2(indent+1);

      str_add(&out,type_to_cpp(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out," ");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out,"=");
      str_add(&out,zero_to_cpp(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out,";\n");
      it = it->next;
    }
    INDENT2(indent);
    str_add(&out,"}");

  }else if (expr->key == EXPR_NOTNULL){
    str_add(&out,"(");
    str_add(&out, expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"!=NULL)");

  }else if (expr->key == EXPR_SETNULL){
    if (!CHILD2){
      str_add(&out,"(");
      str_add(&out, expr_to_cpp(CHILD1,-1).data);
      str_add(&out,"=NULL");
    }else{
      if (CHILD1->type->tag == TYP_STT){
        str_add(&out,"((");
        str_add(&out,expr_to_cpp(CHILD1,-1).data);
        str_add(&out,")->");
        str_add(&out,expr_to_cpp(CHILD2,-1).data);
        str_add(&out,"=NULL)");
      }else if (CHILD1->type->tag == TYP_ARR){
        str_add(&out,"((");
        str_add(&out,expr_to_cpp(CHILD1,-1).data);
        str_add(&out,")[");
        str_add(&out,expr_to_cpp(CHILD2,-1).data);
        str_add(&out,"])=NULL");
      }else if (CHILD1->type->tag == TYP_VEC){
        str_add(&out,"((");
        str_add(&out,expr_to_cpp(CHILD1,-1).data);
        str_add(&out,")[");
        str_add(&out,expr_to_cpp(CHILD2,-1).data);
        str_add(&out,"])=NULL");

      }
    }

  }else if (expr->key == EXPR_ALLOC){
    type_t* typ = (type_t*)(CHILD1->term);

    if (typ->tag == TYP_STT){
      str_add(&out,"(new ");
      str_add(&out,typ->u.name.data);
      str_add(&out,")");

    }else if (typ->tag == TYP_ARR){
      if (expr->children.len == 1){
        str_add(&out,"(new std::vector<");
        str_add(&out,type_to_cpp(typ->elem0).data);
        str_add(&out,">)");
      }else{
        str_add(&out,"(new std::vector<");
        str_add(&out,type_to_cpp(typ->elem0).data);
        str_add(&out,">{");
        list_node_t* it = expr->children.head->next;
        while (it){
          if (it != expr->children.head->next){
            str_add(&out,",");
          }
          str_add(&out,"(");
          str_add(&out,expr_to_cpp((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,"})");
      }

    }else if (typ->tag == TYP_VEC){
      char s[32];
      snprintf(s,sizeof(s),"%d",typ->u.size);
      if (expr->children.len == 1){
        str_add(&out,"w_vec_init<");
        str_add(&out,type_to_cpp(typ->elem0).data);
        str_add(&out,",");
        str_add(&out,s);
        str_add(&out,">(");
        str_add(&out,zero_to_cpp(typ->elem0).data);
        str_add(&out,")");
      }else{
        str_add(&out,"(new std::array<");
        str_add(&out,type_to_cpp(typ->elem0).data);
        str_add(&out,",");
        str_add(&out,s);
        str_add(&out,">{");
        list_node_t* it = expr->children.head->next;
        while (it){
          if (it != expr->children.head->next){
            str_add(&out,",");
          }
          str_add(&out,"(");
          str_add(&out,expr_to_cpp((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,"})");
      }
    }else if (typ->tag == TYP_MAP){
      str_add(&out,"(new std::map<");
      str_add(&out,type_to_cpp(typ->elem0).data);
      str_add(&out,",");
      str_add(&out,type_to_cpp(typ->u.elem1).data);
      str_add(&out,">)");

    }else if (typ->tag == TYP_STR){
      str_add(&out,"std::string(");
      if (CHILD2){
        str_add(&out,expr_to_cpp(CHILD2,-1).data);
      }else{
        str_add(&out,"\"\"");
      }
      str_add(&out,")");
    }
  }else if (expr->key == EXPR_FREE){
    if (CHILD1->type->tag != TYP_STR){
      str_add(&out,"delete ");
      str_add(&out,expr_to_cpp(CHILD1,-1).data);
    }
  }else if (expr->key == EXPR_STRUCTGET){
    str_add(&out,"((");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")->");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRUCTSET){
    str_add(&out,"((");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")->");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,")=");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);

  }else if (expr->key == EXPR_VECGET){
    str_add(&out,"((*(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"])");
  }else if (expr->key == EXPR_VECSET){
    str_add(&out,"((*(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"])=");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);

  }else if (expr->key == EXPR_ARRGET){
    str_add(&out,"((*(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"])");

  }else if (expr->key == EXPR_ARRSET){
    str_add(&out,"((*(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"])=");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);

  }else if (expr->key == EXPR_ARRINS){
    str_add(&out,"w_arr_insert(");
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_ARRREM){
    str_add(&out,"w_arr_remove(");
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"w_arr_slice(");
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"->size())");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"->size())");


  }else if (expr->key == EXPR_MAPGET){
    str_add(&out,"w_map_get(");
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,zero_to_cpp(CHILD1->type->u.elem1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")->erase(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"((*(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"])=");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);

  }else if (expr->key == EXPR_STRLEN){
    str_add(&out,"((");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,").length())");

  }else if (expr->key == EXPR_STRGET){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")[");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"]");

  }else if (expr->key == EXPR_STRADD){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")+=(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRCAT){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")+=(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRCPY){
    str_add(&out,"(");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,").substr((");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"),(");
    str_add(&out,expr_to_cpp(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STREQL){
    str_add(&out,"((");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")==(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRNEQ){
    str_add(&out,"((");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,")!=(");
    str_add(&out,expr_to_cpp(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"std::cout << (");
    str_add(&out,expr_to_cpp(CHILD1,-1).data);
    str_add(&out,") << std::endl");

  }else if (expr->key == EXPR_EXTERN){
    //skip
    out.len-=2;
    out.data[out.len] = 0;
    indent=-1;

  }else if (expr->key == EXPR_BREAK){
    str_add(&out,"break");
  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_cpp(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,"/**");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"**/");
  }

  if (indent>=0){str_add(&out,";\n");}
  return out;
}



str_t tree_to_cpp(str_t modname, expr_t* tree, map_t* functable, map_t* stttable){

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

  str_add(&out,"#include <iostream>\n");
  str_add(&out,"#include <string>\n");
  str_add(&out,"#include <vector>\n");
  str_add(&out,"#include <array>\n");
  str_add(&out,"#include <map>\n");
  str_add(&out,"#include <math.h>\n");

  str_add(&out,"\nnamespace ");
  str_add(&out,modname.data);
  str_add(&out,"{\n");

  str_add(&out,"/*=== WAX Standard Library BEGIN ===*/\n");
  str_add(&out,TEXT_std_cpp);
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
            INDENT2(1);
            str_add(&out,type_to_cpp( (type_t*)(CHILD2->term) ).data);
            str_add(&out," ");
            str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
            str_add(&out,"=");
            str_add(&out,expr_to_cpp( (expr_t*)(ex1->children.head->next->data),-1).data);

            str_add(&out,";\n");
            it = it -> next -> next;
            continue;
          }
          
        }
      }
    }

    str_add(&out,expr_to_cpp(expr,1).data);


    it = it->next;
  }
  str_add(&out,"/*=== User Code            END   ===*/\n");
  str_add(&out,"};\n");

  str_t mainstr = str_from("main",4);
  func_t* fun = func_lookup(&mainstr,functable);
  if (fun != NULL){
    if (fun->params.len){
      str_add(&out,"int main(int argc, char** argv){\n");
      str_add(&out,"  std::vector<std::string> args(argv, argv + argc);\n");
      str_add(&out,"  return ");
      str_add(&out,modname.data);
      str_add(&out,"::main(&args);\n");
      str_add(&out,"}\n");
    }else{
      str_add(&out,"int main(){\n");
      str_add(&out,"  return ");
      str_add(&out,modname.data);
      str_add(&out,"::main();\n");
      str_add(&out,"}\n");
    }
  }
  return out;

}




#endif
