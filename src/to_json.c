#ifndef WAX_TO_JSON
#define WAX_TO_JSON

#include "text.c"
#include "parser.c"
#include "common.c"

const char* opcode_to_name(int x){
if(x==(EXPR_EXTERN))return"EXTERN";
if(x==(EXPR_RETURN))return"RETURN";
if(x==(EXPR_RESULT))return"RESULT";
if(x==(EXPR_STRUCT))return"STRUCT";
if(x==(EXPR_PARAM))return"PARAM";
if(x==(EXPR_WHILE))return"WHILE";
if(x==(EXPR_ALLOC))return"ALLOC";
if(x==(EXPR_FUNC))return"FUNC";
if(x==(EXPR_THEN))return"THEN";
if(x==(EXPR_ELSE))return"ELSE";
if(x==(EXPR_CALL))return"CALL";
if(x==(EXPR_CAST))return"CAST";
if(x==(EXPR_FREE))return"FREE";
if(x==(EXPR_LET))return"LET";
if(x==(EXPR_FOR))return"FOR";
if(x==(EXPR_IF))return"IF";
if(x==(EXPR_DO))return"DO";
if(x==(EXPR_TIF))return"TIF";
if(x==(EXPR_SET))return"SET";
if(x==(EXPR_PRINT))return"PRINT";
if(x==(EXPR_FORIN))return"FORIN";
if(x==(EXPR_BREAK))return"BREAK";
if(x==(EXPR_ASM))return"ASM";
if(x==(EXPR_LAND))return"LAND";
if(x==(EXPR_LOR))return"LOR";
if(x==(EXPR_LNOT))return"LNOT";
if(x==(EXPR_XOR))return"XOR";
if(x==(EXPR_BAND))return"BAND";
if(x==(EXPR_BOR))return"BOR";
if(x==(EXPR_SHL))return"SHL";
if(x==(EXPR_SHR))return"SHR";
if(x==(EXPR_BNEG))return"BNEG";
if(x==(EXPR_IADD))return"IADD";
if(x==(EXPR_ISUB))return"ISUB";
if(x==(EXPR_IMUL))return"IMUL";
if(x==(EXPR_IDIV))return"IDIV";
if(x==(EXPR_IMOD))return"IMOD";
if(x==(EXPR_ILT))return"ILT";
if(x==(EXPR_IGT))return"IGT";
if(x==(EXPR_ILEQ))return"ILEQ";
if(x==(EXPR_IGEQ))return"IGEQ";
if(x==(EXPR_IEQ))return"IEQ";
if(x==(EXPR_INEQ))return"INEQ";
if(x==(EXPR_FADD))return"FADD";
if(x==(EXPR_FSUB))return"FSUB";
if(x==(EXPR_FMUL))return"FMUL";
if(x==(EXPR_FDIV))return"FDIV";
if(x==(EXPR_FMOD))return"FMOD";
if(x==(EXPR_FLT))return"FLT";
if(x==(EXPR_FGT))return"FGT";
if(x==(EXPR_FLEQ))return"FLEQ";
if(x==(EXPR_FGEQ))return"FGEQ";
if(x==(EXPR_FEQ))return"FEQ";
if(x==(EXPR_FNEQ))return"FNEQ";
if(x==(EXPR_VECGET))return"VECGET";
if(x==(EXPR_VECSET))return"VECSET";
if(x==(EXPR_ARRGET))return"ARRGET";
if(x==(EXPR_ARRSET))return"ARRSET";
if(x==(EXPR_ARRREM))return"ARRREM";
if(x==(EXPR_ARRINS))return"ARRINS";
if(x==(EXPR_ARRCPY))return"ARRCPY";
if(x==(EXPR_ARRLEN))return"ARRLEN";
if(x==(EXPR_MAPGET))return"MAPGET";
if(x==(EXPR_MAPSET))return"MAPSET";
if(x==(EXPR_MAPLEN))return"MAPLEN";
if(x==(EXPR_MAPREM))return"MAPREM";
if(x==(EXPR_STRUCTGET))return"STRUCTGET";
if(x==(EXPR_STRUCTSET))return"STRUCTSET";
if(x==(EXPR_STRUCTFLD))return"STRUCTFLD";
if(x==(EXPR_STRGET))return"STRGET";
if(x==(EXPR_STRADD))return"STRADD";
if(x==(EXPR_STRLEN))return"STRLEN";
if(x==(EXPR_STRCPY))return"STRCPY";
if(x==(EXPR_STREQL))return"STREQL";
if(x==(EXPR_STRNEQ))return"STRNEQ";
if(x==(EXPR_STRCAT))return"STRCAT";
if(x==(EXPR_FUNCBODY))return"FUNCBODY";
if(x==(EXPR_FUNCHEAD))return"FUNCHEAD";
if(x==(EXPR_SETNULL))return"SETNULL";
if(x==(EXPR_NOTNULL))return"NOTNULL";
if(x==(EXPR_PTREQL))return"PTREQL";
if(x==(EXPR_PTRNEQ))return"PTRNEQ";
return NULL;
}

str_t type_to_json(type_t* typ){
  str_t out = str_new();
  if (typ->tag == TYP_INT){
    str_add(&out,"{\"type\":\"int\"}");
  }else if (typ->tag == TYP_FLT){
    str_add(&out,"{\"type\":\"float\"}");
  }else if (typ->tag == TYP_STT){
    str_add(&out,"{\"type\":\"struct\",\"name\":\"");
    str_add(&out,typ->u.name.data);
    str_add(&out,"\"}");
  }else if (typ->tag == TYP_ARR){
    str_add(&out,"{\"type\":\"arr\",\"elem\":");
    str_add(&out,type_to_json(typ->elem0).data);
    str_add(&out,"}");
  }else if (typ->tag == TYP_VEC){
    str_add(&out,"{\"type\":\"vec\",\"elem\":");
    str_add(&out,type_to_json(typ->elem0).data);
    str_add(&out,"}");
  }else if (typ->tag == TYP_MAP){
    str_add(&out,"{\"type\":\"arr\",\"key\":");
    str_add(&out,type_to_json(typ->elem0).data);
    str_add(&out,",\"value\":");
    str_add(&out,type_to_json(typ->u.elem1).data);
    str_add(&out,"}");
  }else if (typ->tag == TYP_STR){
    str_add(&out,"{\"type\":\"string\"}");
  }else{
    str_add(&out,"{\"type\":\"void\"}");
  }
  return out;
}

str_t symtable_to_json(map_t* symtable, int indent){
  str_t out = str_new();
  int k = 0;
  
  int first = 1;
  while (k < NUM_MAP_SLOTS){
    while (!symtable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!symtable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = symtable->slots[k]->head;
    while (it){
      if (!first){
        str_add(&out,",\n");
      }
      first = 0;
      INDENT2(indent);
      sym_t* sym = (sym_t*)(it->data);
      str_add(&out,"{\"");
      str_add(&out,sym->name.data);
      str_add(&out,"\":");
      str_add(&out,type_to_json(&sym->type).data);
      str_add(&out,"}");
      it = it->next;
    }
    k++;
  }
  return out;
}

str_t functable_to_json(map_t* functable, int indent){
  str_t out = str_new();
  int k = 0;
  int first = 1;
  while (k < NUM_MAP_SLOTS){

    while (!functable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!functable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = functable->slots[k]->head;

    while (it){
      if (!first){
        str_add(&out,",\n");
      }
      first = 0;
      INDENT2(indent);

      func_t* func = (func_t*)(it->data);
      str_add(&out,"{\"function\":\"");
      str_add(&out,func->name.data);
      str_add(&out,"\",\"return\":");
      str_add(&out,type_to_json(func->result).data);
      str_add(&out,",\"params\":[");
      list_node_t* jt = func->params.head;

      while (jt){
        if (jt != func->params.head){
          str_add(&out,",");
        }
        sym_t* sym = (sym_t*)(jt->data);
        str_add(&out,"{\"");
        str_add(&out,sym->name.data);
        str_add(&out,"\":");
        str_add(&out,type_to_json(&sym->type).data);
        str_add(&out,"}");
        jt = jt->next;
      }
      str_add(&out,"]}");

      it = it->next;
    }
    k++;
  }
  return out;
}

str_t stttable_to_json(map_t* stttable,int indent){
  str_t out = str_new();
  int k = 0;
  int first = 1;
  while (k < NUM_MAP_SLOTS){

    while (!stttable->slots[k] && k < NUM_MAP_SLOTS){
      k++;
    }
    if (!stttable->slots[k] || k >= NUM_MAP_SLOTS){
      break;
    }
    list_node_t* it = stttable->slots[k]->head;

    while (it){
      if (!first){
        str_add(&out,",\n");
      }
      first = 0;
      INDENT2(indent);
      stt_t* stt = (stt_t*)(it->data);
      str_add(&out,"{\"struct\":\"");
      str_add(&out,stt->name.data);
      str_add(&out,"\",\"fields\":[");

      list_node_t* jt = stt->fields.head;

      while (jt){
        if (jt != stt->fields.head){
          str_add(&out,",");
        }
        sym_t* sym = (sym_t*)(jt->data);
        str_add(&out,"{\"");
        str_add(&out,sym->name.data);
        str_add(&out,"\":");
        str_add(&out,type_to_json(&sym->type).data);
        str_add(&out,"}");
        jt = jt->next;
      }
      str_add(&out,"]}");
      it = it->next;
    }
    k++;
  }
  return out;
}



str_t expr_to_json(expr_t* expr, int indent){
  str_t out = str_new();

  INDENT2(indent);
  if (expr->key == EXPR_TYPE){
    str_add(&out,type_to_json((type_t*)(expr->term)).data);
  }else if (expr->key == EXPR_TERM){
    tok_t* tok = (tok_t*)(expr->term);
    str_add(&out,"{\"atom\":\"");
    str_add(&out,(char*)tokens_desc[(int)(tok->tag)]);
    str_add(&out,"\",\"value\":\"");
    if (tok->tag == TOK_STR){
      tok->val.data[tok->val.len-1]=0;
      str_add(&out,&tok->val.data[1]);
      tok->val.data[tok->val.len-1]='"';
    }else{
      str_add(&out,tok->val.data);
    }
    str_add(&out,"\"}");
  }else{
    str_add(&out,"{\"expr\":\"");
    str_add(&out,expr->rawkey.data);
    str_add(&out,"\",\"op\":\"");
    str_add(&out,(char*)opcode_to_name(expr->key));
    char s[32];
    str_add(&out,"\",\"line\":");
    snprintf(s,sizeof(s),"%d",expr->lino);
    str_add(&out,s);
    str_add(&out,",\"dtype\":");
    str_add(&out,type_to_json(expr->type).data);

    if (expr->symtable.len){
      str_add(&out,",\n");
      INDENT2(indent);
      str_add(&out,"\"symbols\":[\n");
      str_add(&out,symtable_to_json(&expr->symtable,indent+1).data);
      str_add(&out,"],\n");
      INDENT2(indent);
    }else{
      str_add(&out,",");
    }

    str_add(&out,"\"children\":[");

    list_node_t* it = expr->children.head;
    while (it){
      if (it != expr->children.head){
        str_add(&out,",\n");
      }else{
        str_add(&out,"\n");
      }
      expr_t* ex = (expr_t*)(it->data);
      str_add(&out,expr_to_json(ex, indent+1).data);
      
      it = it->next;
    }
    str_add(&out,"]}");
  }
  return out;
}

str_t tree_to_json(str_t modname, expr_t* tree, map_t* functable, map_t* stttable, map_t* included){
  str_t out = str_new();

  str_add(&out,"{\"program\":\"");

  str_add(&out,modname.data);
  str_add(&out,"\",");

  if (functable->len){
    str_add(&out,"\n");
    INDENT2(1);
    str_add(&out,"\"signatures\":[\n");
    str_add(&out,functable_to_json(functable,2).data);
    str_add(&out,"],");
    INDENT2(1);
  }

  if (stttable->len){
    str_add(&out,"\n");
    INDENT2(1);
    str_add(&out,"\"structs\":[\n");
    str_add(&out,stttable_to_json(stttable,2).data);
    str_add(&out,"],");
    INDENT2(1);
  }

  if (tree->symtable.len){
    str_add(&out,"\n");
    INDENT2(1);
    str_add(&out,"\"symbols\":[\n");
    str_add(&out,symtable_to_json(&tree->symtable,2).data);
    str_add(&out,"],\n");
    INDENT2(1);
  }

  str_add(&out,"\"children\":[\n");

  list_node_t* it = tree->children.head;
  while(it){
    if (it != tree->children.head){
      str_add(&out,",\n");
    }
    expr_t* expr = (expr_t*)(it->data);

    str_add(&out,expr_to_json(expr,1).data);
    it = it->next;
  }

  str_add(&out,"]}\n");
  
  return out;

}




#endif
