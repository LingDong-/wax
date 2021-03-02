#ifndef WAX_TO_SWIFT
#define WAX_TO_SWIFT

#include "text.c"
#include "parser.c"
#include "common.c"

map_t* swift_functable = NULL;

str_t type_to_swift(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"Int");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"Float");
  }else if (typ->tag == TYP_STT){
    str_add(&out,typ->u.name.data);
    str_add(&out,"?");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"w_Arr<");
    str_add(&out,type_to_swift(typ->elem0).data);
    str_add(&out,">?");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"w_Arr<");
    str_add(&out,type_to_swift(typ->elem0).data);
    str_add(&out,">?");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"w_Map<");
    str_add(&out,type_to_swift(typ->elem0).data);
    str_add(&out,",");
    str_add(&out,type_to_swift(typ->u.elem1).data);
    str_add(&out,">?");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"String?");
  }else{
    str_add(&out,"/*type?*/");
  }
  return out;
}
str_t zero_to_swift(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"0");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"0.0");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"nil");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"nil");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"nil");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"nil");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"nil");
  }else{
    str_add(&out,"/*zero?*/");
  }
  return out;
}

str_t expr_to_swift(expr_t* expr, int indent){
  // print_syntax_tree(expr,4);
  // printf("-----\n");
  str_t out = str_new();
  INDENT2(indent);

  if (expr->key == EXPR_LET){
    
    
    str_add(&out,"var ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,":");
    str_add(&out,type_to_swift( (type_t*)(CHILD2->term) ).data);
    str_add(&out," = ");
    str_add(&out,zero_to_swift( (type_t*)(CHILD2->term) ).data);
    
  }else if (expr->key == EXPR_SET){
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out," = ");
    str_add(&out, expr_to_swift(CHILD2,-1).data );

  }else if (expr->key == EXPR_TERM){
    tok_t* tok = ((tok_t*)(expr->term));

    if (tok->tag == TOK_INT && tok->val.data[0] == '\''){
      str_add(&out, "w_charCode(x:");
      str_t s = str_from(tok->val.data,tok->val.len);
      s.data[0] = '"';
      s.data[s.len-1] = '"';
      str_add(&out, s.data);
      str_add(&out, ")");
    }else{
      str_add(&out, tok->val.data);
    }
  
  }else if (expr->key == EXPR_FADD || expr->key == EXPR_IADD ||
            expr->key == EXPR_FSUB || expr->key == EXPR_ISUB || 
            expr->key == EXPR_FMUL || expr->key == EXPR_IMUL ||
            expr->key == EXPR_FDIV || expr->key == EXPR_IDIV ||
            expr->key == EXPR_BAND || 
            expr->key == EXPR_BOR  ||
            expr->key == EXPR_IMOD || expr->key == EXPR_FMOD ||
            expr->key == EXPR_XOR  || 
            expr->key == EXPR_SHL  || expr->key == EXPR_SHR
    ){
    str_add(&out, "((");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ")");
    str_add(&out, expr->rawkey.data);
    str_add(&out, "(");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");
  }else if (expr->key == EXPR_IGEQ || expr->key == EXPR_FGEQ ||
            expr->key == EXPR_ILEQ || expr->key == EXPR_FLEQ ||
            expr->key == EXPR_IGT  || expr->key == EXPR_FGT  ||
            expr->key == EXPR_ILT  || expr->key == EXPR_FLT  
  ){
    str_add(&out, "w_INT(x:(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") ");
    str_add(&out, expr->rawkey.data);
    str_add(&out, " (");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");


  }else if (expr->key == EXPR_LAND){
    str_add(&out, "w_INT(x:w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") && w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_LOR ){
    str_add(&out, "w_INT(x:w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") || w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");


  }else if (expr->key == EXPR_IEQ || expr->key == EXPR_FEQ  || expr->key == EXPR_STREQL){

    str_add(&out, "w_INT(x:(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") == (");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_INEQ || expr->key == EXPR_FNEQ || expr->key == EXPR_STRNEQ){

    str_add(&out, "w_INT(x:(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") != (");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_PTREQL){

    str_add(&out, "w_INT(x:(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") === (");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_PTRNEQ){

    str_add(&out, "w_INT(x:(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ") !== (");
    str_add(&out, expr_to_swift(CHILD2,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_BNEG){
    str_add(&out, "(");
    str_add(&out, expr->rawkey.data);
    str_add(&out, "(");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, "))");

  }else if (expr->key == EXPR_LNOT){
    str_add(&out, "w_NOT(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data );
    str_add(&out, ")");

  }else if (expr->key == EXPR_IF){
    str_add(&out, "if(w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, ")){\n");
    str_add(&out, expr_to_swift(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");
    if (CHILD3){
      str_add(&out, "else{\n");
      str_add(&out, expr_to_swift(CHILD3,indent).data);
      INDENT2(indent);
      str_add(&out, "}");
    }

  }else if (expr->key == EXPR_TIF){
    str_add(&out, "(w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, ") ? (");
    str_add(&out, expr_to_swift(CHILD2,-1).data);
    str_add(&out, ") : (");
    str_add(&out, expr_to_swift(CHILD3,-1).data);
    str_add(&out, "))");

  }else if (expr->key == EXPR_WHILE){
    str_add(&out, "while(w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, ")){\n");
    str_add(&out, expr_to_swift(CHILD2,indent).data);
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FOR){
    str_add(&out, "do{\n");
    INDENT2(indent+1);
    str_add(&out, "var ");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, ":Int = (");
    str_add(&out, expr_to_swift(CHILD2,-1).data);
    str_add(&out, ");\n");
    INDENT2(indent+1);
    str_add(&out, "while(w_BOOL(x:");
    str_add(&out, expr_to_swift(CHILD3,-1).data);
    str_add(&out, ")){\n");
    str_add(&out, expr_to_swift(CHILDN,indent+1).data);
    INDENT2(indent+2);
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, " += (");
    str_add(&out, expr_to_swift(CHILD4,-1).data);
    str_add(&out, ");\n");
    INDENT2(indent+1);
    str_add(&out, "}\n");
    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FORIN){

    str_add(&out, "for (");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out, ",");
    str_add(&out, expr_to_swift(CHILD2,-1).data);
    str_add(&out, ") in (");
    str_add(&out, expr_to_swift(CHILD3,-1).data);
    str_add(&out, ")!.data {\n");

    str_add(&out, expr_to_swift(CHILDN,indent+1).data);

    INDENT2(indent);
    str_add(&out, "}");

  }else if (expr->key == EXPR_FUNC){
    str_add(&out, "static func ");
    list_node_t* it = expr->children.head;

    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);
    str_add(&out, "(");
    it = expr->children.head->next;

    str_t varpar = str_new();

    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key != EXPR_PARAM){
        break;
      }
      if (it != expr->children.head->next){
        str_add(&out,",");
      }
    
      str_t paname = ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val;
      type_t* ptyp = (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term);
      str_t swname = tmp_name("tmp__swap_");

      str_add(&out, paname.data);
      str_add(&out,":");
      str_add(&out,type_to_swift(  ptyp ).data);

      for (int i = 0; i < indent+1; i++){str_add(&varpar, "  ");}

      str_add(&varpar,"var ");
      str_add(&varpar,paname.data);
      str_add(&varpar," = ");
      str_add(&varpar,paname.data);
      str_add(&varpar,";");

      str_add(&varpar,"let ");
      str_add(&varpar,swname.data);
      str_add(&varpar," = ");
      str_add(&varpar,paname.data);
      str_add(&varpar,";"); 

      str_add(&varpar,paname.data);
      str_add(&varpar," = ");
      str_add(&varpar,zero_to_swift(ptyp).data);
      str_add(&varpar,";");

      str_add(&varpar,paname.data);
      str_add(&varpar," = ");
      str_add(&varpar,swname.data);
      str_add(&varpar,";\n");   

      it = it->next;
    }
    str_add(&out, ")");

    if (((expr_t*)(it->data))->key == EXPR_RESULT ){
      str_add(&out, "->");
      str_add(&out, type_to_swift((type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term)).data);
    }

    str_add(&out, "{\n");
    // INDENT2(indent+1);
    str_add(&out, varpar.data);
    str_add(&out, "\n");
    str_add(&out, expr_to_swift(CHILDN,indent).data);
    INDENT2(indent);
    str_add(&out, "}");


  }else if (expr->key == EXPR_CALL){
    str_t funcname = ((tok_t*)(CHILD1->term))->val;

    str_add(&out, funcname.data);
    str_add(&out, "(");
    list_node_t* it = expr->children.head->next;
    func_t* fun = func_lookup(&funcname,swift_functable);
    list_node_t* jt = fun->params.head;

    while(it){
      expr_t* ex = (expr_t*)(it->data);
      if (ex->key == EXPR_RESULT){
        break;
      }
      if (it != expr->children.head->next){
        str_add(&out,",");
      }

      str_add(&out, ((sym_t*)(jt->data))->name.data);
      str_add(&out, ":");
      str_add(&out, expr_to_swift(((expr_t*)(it->data)),-1).data );

      it = it->next;
      jt = jt->next;
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
        str_add(&out,(char*)&expr_to_swift(ex,indent+1).data[indent*2]);
      }else{
        str_add(&out,expr_to_swift(ex,indent+1).data);
      }
      it = it->next;
    }

    indent=-1;

  }else if (expr->key == EXPR_CAST){
    type_t* typl = CHILD1->type;
    type_t* typr = (type_t*)(CHILD2->term);
    if (typl->tag == TYP_INT && typr->tag == TYP_FLT){
      str_add(&out, "Float(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_INT){
      str_add(&out, "Int(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_INT && typr->tag == TYP_STR){
      str_add(&out, "String(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_FLT && typr->tag == TYP_STR){
      str_add(&out, "String(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")");
    }else if (typl->tag == TYP_STR && typr->tag == TYP_INT){
      str_add(&out, "(Int(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")!)");
    
    }else if (typl->tag == TYP_STR && typr->tag == TYP_FLT){
      str_add(&out, "(Float(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")!)");
    }else{
      str_add(&out, "(");
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out, ")");
    }
  }else if (expr->key == EXPR_RETURN){
    str_add(&out,"return");
    if (CHILD1){
      str_add(&out," ");
      str_add(&out,expr_to_swift(CHILD1,-1).data);
    }
  }else if (expr->key == EXPR_STRUCT){
    str_add(&out,"class ");
    str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
    str_add(&out,"{\n");

    list_node_t* it = expr->children.head->next;

    while(it){
      // expr_t* ex = (expr_t*)(it->data);

      INDENT2(indent+1);
      str_add(&out,"var ");
      str_add(&out, ((tok_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->data))->term))->val.data);
      str_add(&out,":");
      str_add(&out,type_to_swift(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out," = ");
      str_add(&out,zero_to_swift(  (type_t*)(((expr_t*)(((expr_t*)(it->data))->children.head->next->data))->term) ).data);
      str_add(&out,";\n");
      it = it->next;
    }
    INDENT2(indent);
    str_add(&out,"}");

  }else if (expr->key == EXPR_NOTNULL){
    str_add(&out,"w_INT(x: (");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out,") != nil)");

  }else if (expr->key == EXPR_SETNULL){
    if (!CHILD2){
      str_add(&out, expr_to_swift(CHILD1,-1).data);
      str_add(&out," = nil");
    }else{
      if (CHILD1->type->tag == TYP_STT){
        str_add(&out,"((");
        str_add(&out,expr_to_swift(CHILD1,-1).data);
        str_add(&out,")!.");
        str_add(&out,expr_to_swift(CHILD2,-1).data);
        str_add(&out," = nil)");
      }else if (CHILD1->type->tag == TYP_ARR){
        str_add(&out,"((");
        str_add(&out,expr_to_swift(CHILD1,-1).data);
        str_add(&out,")!.data[");
        str_add(&out,expr_to_swift(CHILD2,-1).data);
        str_add(&out,"] = nil");
      }else if (CHILD1->type->tag == TYP_VEC){
        str_add(&out,"((");
        str_add(&out,expr_to_swift(CHILD1,-1).data);
        str_add(&out,")!.data[");
        str_add(&out,expr_to_swift(CHILD2,-1).data);
        str_add(&out,"] = nil");
      }
    }
  }else if (expr->key == EXPR_ALLOC){
    type_t* typ = (type_t*)(CHILD1->term);

    if (typ->tag == TYP_STT){
      str_add(&out,typ->u.name.data);
      str_add(&out,"()");

    }else if (typ->tag == TYP_ARR || typ->tag == TYP_VEC){

      str_add(&out,"w_Arr<");
      str_add(&out,type_to_swift(typ->elem0).data);
      str_add(&out,">");

      if (expr->children.len == 1){
        if (typ->tag == TYP_ARR){
          str_add(&out,"(data:[])");
        }else{
          char s[32];
          snprintf(s,sizeof(s),"%d",typ->u.size);
          str_add(&out,"(zero:");
          str_add(&out,zero_to_swift(typ->elem0).data);
          str_add(&out,",n:");
          str_add(&out,s);
          str_add(&out,")");
        }
      }else{
        
        str_add(&out,"(data:[");

        list_node_t* it = expr->children.head->next;
        while (it){
          if (it != expr->children.head->next){
            str_add(&out,",");
          }
          str_add(&out,"(");
          str_add(&out,expr_to_swift((expr_t*)(it->data),-1).data);
          str_add(&out,")");
          it = it->next;
        }
        str_add(&out,"])");
      }

    }else if (typ->tag == TYP_MAP){
      str_add(&out,"w_Map<");
      str_add(&out,type_to_swift(typ->elem0).data);
      str_add(&out,",");
      str_add(&out,type_to_swift(typ->u.elem1).data);
      str_add(&out,">()");

    }else if (typ->tag == TYP_STR){
      if (CHILD2){
        str_add(&out,expr_to_swift(CHILD2,-1).data);
      }else{
        str_add(&out,"\"\"");
      }
    }
  }else if (expr->key == EXPR_FREE){

    str_add(&out,"/*GC `");
    str_add(&out, expr_to_swift(CHILD1,-1).data);
    str_add(&out," = nil`*/\n");
    indent=-1;
  
  }else if (expr->key == EXPR_STRUCTGET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRUCTSET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out," = ");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_VECGET || expr->key == EXPR_ARRGET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data[");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"])");
  }else if (expr->key == EXPR_VECSET || expr->key == EXPR_ARRSET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data[");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"] = ");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRINS){

    str_add(&out,"(");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data.insert(");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,",at:");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRREM){

    str_add(&out,"(");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.remove(i:");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,",n:");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRCPY){
    str_add(&out,"(");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.slice(i:");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,",n:");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_ARRLEN){
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,"!.data.count");

  }else if (expr->key == EXPR_MAPLEN){
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,"!.data.count");

  }else if (expr->key == EXPR_MAPGET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data[");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"] ?? ");
    str_add(&out,zero_to_swift(CHILD1->type->u.elem1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_MAPREM){

    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data.removeValue(forKey:");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_MAPSET){
    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")!.data[");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"]=");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,")");

  }else if (expr->key == EXPR_STRLEN){
    str_add(&out,"w_strBang(x:");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,").count");

  }else if (expr->key == EXPR_STRGET){
    str_add(&out,"w_charCode(x:String(Array(w_strBang(x:");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,"))[");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"]))");

  }else if (expr->key == EXPR_STRADD){
    str_add(&out,"(");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")! += String(UnicodeScalar(UInt8(");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,")))");

  }else if (expr->key == EXPR_STRCAT){

    str_add(&out,"((");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,")! += w_strBang(x:");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_STRCPY){
    str_add(&out,"w_strSlice(x:w_strBang(x:");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,"),i:(");
    str_add(&out,expr_to_swift(CHILD2,-1).data);
    str_add(&out,"),n:(");
    str_add(&out,expr_to_swift(CHILD3,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_PRINT){
    str_add(&out,"print(w_strBang(x:");
    str_add(&out,expr_to_swift(CHILD1,-1).data);
    str_add(&out,"))");

  }else if (expr->key == EXPR_EXTERN){
    //skip
    out.len-=2;
    out.data[out.len] = 0;
    indent=-1;
  }else if (expr->key == EXPR_BREAK){
    str_add(&out,"break");
  }else if (expr->key == EXPR_ASM){
    
    str_add(&out,str_unquote(expr_to_swift(CHILD1,-1)).data);
    indent=-1;

  }else{
    str_add(&out,"/**");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"**/");
  }

  if (indent>=0){str_add(&out,";\n");}
  return out;
}

str_t tree_to_swift(str_t modname, expr_t* tree, map_t* functable, map_t* stttable, map_t* included){
  
  swift_functable = functable;

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

  str_add(&out,"import Foundation;\n");

  str_add(&out,"\nstruct ");
  str_add(&out,modname.data);
  str_add(&out,"{\n");

  str_add(&out,"/*=== WAX Standard Library BEGIN ===*/\n");
  str_add(&out,TEXT_std_swift);
  str_add(&out,"/*=== WAX Standard Library END   ===*/\n\n");


  str_add(&out,"/*=== User Code            BEGIN ===*/\n\n");
  list_node_t* it = tree->children.head;

  while(it){
    expr_t* expr = (expr_t*)(it->data);

    if (expr->key == EXPR_LET && it->next){
      expr_t* ex1 = (expr_t*)(it->next->data);
      if (ex1->key == EXPR_SET){
        expr_t* ex2 = (expr_t*)(ex1->children.head->data);

        if (ex2->key == EXPR_TERM){
          if (str_eq( &((tok_t*)(CHILD1->term))->val, ((tok_t*)(ex2->term))->val.data )){
            str_add(&out,"\nstatic var ");
            str_add(&out, ((tok_t*)(CHILD1->term))->val.data);
            str_add(&out,":");
            str_add(&out,type_to_swift( (type_t*)(CHILD2->term) ).data);
            str_add(&out," = ");
            str_add(&out,expr_to_swift( (expr_t*)(ex1->children.head->next->data),-1).data);
            str_add(&out,";\n");
            it = it -> next -> next;
            continue;
          }
          
        }
      }
    }

    str_add(&out,expr_to_swift(expr,1).data);


    it = it->next;
  }
  str_add(&out,"/*=== User Code            END   ===*/\n");
  str_add(&out,"}\n");

  str_t mainstr = str_from("main",4);
  func_t* fun = func_lookup(&mainstr,functable);
  
  if (fun != NULL){
    if (!(fun->params.len)){
      str_add(&out,"exit(Int32(");
      str_add(&out,modname.data);
      str_add(&out,".main()));\n");
    }else{
      str_add(&out,"exit(Int32(");
      str_add(&out,modname.data);
      str_add(&out,".main(");
      str_add(&out,((sym_t*)(fun->params.head->data))->name.data);
      str_add(&out,":w_Arr<String>(data:CommandLine.arguments)");
      str_add(&out,")));");
    }
  }

  return out;

}




#endif
