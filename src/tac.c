// three address code (tac) generator
#ifndef WAX_TAC
#define WAX_TAC

#include "common.c"
#include "parser.c"


expr_t* expr_int_const(expr_t* parent, int b){
  expr_t* ex = expr_alloc();
  ex->key = EXPR_TERM;
  ex->rawkey = str_from("",0);
  ex->lino = parent->lino;
  ex->parent = parent;
  ex->type = prim_type(TYP_INT);

  tok_t* t = (tok_t*) mallocx(sizeof(tok_t));
  t->tag = TOK_INT;
  t->lino = parent->lino;
  t->val = str_from(b?"1":"0",1);

  ex->term = t;

  return ex;
}



expr_t* insert_tac_letset(list_node_t* it, expr_t* ex0, type_t* typ, expr_t* val){
  if (!typ){
    typ = val->type;
  }

  expr_t* expr = (expr_t*)(it->data);

  // let
  expr_t* ex = expr_alloc();
  ex->key = EXPR_LET;
  ex->rawkey = str_from("let",3);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = prim_type(TYP_VOD);

  // let (?)
  ex0->parent = ex;
  list_add(&ex->children,ex0);

  // let _ (?)
  expr_t* ex1 = type_expr(typ,expr->lino);
  ex1->parent = ex;

  list_add(&ex->children,ex1);

  list_insert_l(&expr->parent->children,it,ex);

  if (val){

    // set
    expr_t* ey = expr_alloc();
    ey->key = EXPR_SET;
    ey->rawkey = str_from("set",3);
    ey->lino = expr->lino;
    ey->parent = expr->parent;
    ey->type = typ;

    // set (?)
    expr_t* ey0 = expr_alloc();
    ey0->key = EXPR_TERM;
    ey0->rawkey = str_from("",0);
    ey0->lino = expr->lino;
    ey0->parent = ey;
    ey0->type = typ;
    ey0->term = ex0->term;

    list_add(&ey->children,ey0);


    // set _ (?)
    val->parent = ey;
    list_add(&ey->children,val);

    list_insert_l(&expr->parent->children,it,ey);

  }

  expr_t* ez = expr_alloc();
  ez->key = EXPR_TERM;
  ez->rawkey = str_from("",0);
  ez->lino = expr->lino;
  ez->type = typ;
  ez->term = ex0->term;

  return ez;
}


expr_t* insert_tac_tmp(list_node_t* it, type_t* typ, expr_t* val){
  if (!typ){
    typ = val->type;
  }
  expr_t* expr = (expr_t*)(it->data);

  // let (?)
  expr_t* ex0 = expr_alloc();
  ex0->key = EXPR_TERM;
  ex0->rawkey = str_from("",0);
  ex0->lino = expr->lino;
  ex0->type = typ;

  tok_t* tok = (tok_t*)mallocx(sizeof(tok_t));
  tok->tag = TOK_IDT;
  tok->val = tmp_name("tmp___");
  ex0->term = tok;

  map_add(&expr->parent->symtable, tok->val, sym_new(tok->val, *typ, 0));

  return insert_tac_letset(it,ex0,typ,val);
}


expr_t* dup_term(expr_t* expr){
  // shallow
  expr_t* ex = expr_alloc();
  ex->key = EXPR_TERM;
  ex->rawkey = str_from("",0);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = expr->type;
  ex->term = expr->term;

  return ex;
}

expr_t* tac_block(list_node_t* it){
  // hack: if (1) {...}
  expr_t* expr = (expr_t*)(it->data);

  // if
  expr_t* ex = expr_alloc();
  ex->key = EXPR_IF;
  ex->rawkey = str_from("if",2);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = prim_type(TYP_VOD);
  list_add(&ex->children,expr_int_const(ex,1));

  // then
  expr_t* ex0 = expr_alloc();
  ex0->key = EXPR_THEN;
  ex0->rawkey = str_from("then",4);
  ex0->lino = expr->lino;
  ex0->parent = ex;
  ex0->type = prim_type(TYP_VOD);

  list_add(&ex->children,ex0);

  // list_insert_l(&expr->parent->children,it,ex);

  return ex0;
}

void insert_tac_breakiffalse(list_node_t* it, expr_t* ea){
  expr_t* expr = (expr_t*)(it->data);

  // if
  expr_t* ex = expr_alloc();
  ex->key = EXPR_IF;
  ex->rawkey = str_from("if",2);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = prim_type(TYP_VOD);

  // if (?)
  ea->parent = ex;
  list_add(&ex->children,ea);

  // then
  expr_t* ex0 = expr_alloc();
  ex0->key = EXPR_THEN;
  ex0->rawkey = str_from("then",4);
  ex0->lino = expr->lino;
  ex0->parent = ex;
  ex0->type = prim_type(TYP_VOD);

  list_add(&ex->children,ex0);

  // else
  expr_t* ex1 = expr_alloc();
  ex1->key = EXPR_ELSE;
  ex1->rawkey = str_from("else",4);
  ex1->lino = expr->lino;
  ex1->parent = ex;
  ex1->type = prim_type(TYP_VOD);

  list_add(&ex->children,ex1);


  // break
  expr_t* ey = expr_alloc();
  ey->key = EXPR_BREAK;
  ey->rawkey = str_from("break",5);
  ey->lino = expr->lino;
  ey->parent = ex1;
  ey->type = prim_type(TYP_VOD);

  list_add(&ex1->children,ey);

  list_insert_l(&expr->parent->children,it,ex);

}

void insert_tac_ifelseset(list_node_t* it, expr_t* ea, expr_t* eb, expr_t* ec, expr_t* ed){

  expr_t* expr = (expr_t*)(it->data);

  // if
  expr_t* ex = expr_alloc();
  ex->key = EXPR_IF;
  ex->rawkey = str_from("if",2);
  ex->lino = expr->lino;
  ex->parent = expr->parent;
  ex->type = prim_type(TYP_VOD);

  // if (?)
  ea->parent = ex;
  list_add(&ex->children,ea);

  // then
  expr_t* ex0 = expr_alloc();
  ex0->key = EXPR_THEN;
  ex0->rawkey = str_from("then",4);
  ex0->lino = expr->lino;
  ex0->parent = ex;
  ex0->type = prim_type(TYP_VOD);

  list_add(&ex->children,ex0);

  // set
  expr_t* ey = expr_alloc();
  ey->key = EXPR_SET;
  ey->rawkey = str_from("set",3);
  ey->lino = expr->lino;
  ey->parent = ex0;
  ey->type = ec->type;

  list_add(&ex0->children,ey);

  // set (?)
  eb->parent = ey;
  list_add(&ey->children,eb);

  // set _ (?)
  ec->parent = ey;
  list_add(&ey->children,ec);


  if (ed){
    expr_t* ef = dup_term(eb);

    // else
    expr_t* ex1 = expr_alloc();
    ex1->key = EXPR_ELSE;
    ex1->rawkey = str_from("else",4);
    ex1->lino = expr->lino;
    ex1->parent = ex;
    ex1->type = prim_type(TYP_VOD);

    list_add(&ex->children,ex1);

    // set
    expr_t* ez = expr_alloc();
    ez->key = EXPR_SET;
    ez->rawkey = str_from("set",3);
    ez->lino = expr->lino;
    ez->parent = ex1;
    ez->type = ed->type;

    list_add(&ex1->children,ez);

    // set (?)
    ef->parent = ez;
    list_add(&ez->children,ef);

    // set _ (?)
    ed->parent = ez;
    list_add(&ez->children,ed);

  }

  list_insert_l(&expr->parent->children,it,ex);

}


expr_t* tac_increment(expr_t* ea, expr_t* eb){


  // set
  expr_t* ex = expr_alloc();
  ex->key = EXPR_SET;
  ex->rawkey = str_from("set",3);
  ex->lino = ea->lino;
  ex->type = ea->type;

  // set (?)
  ea->parent = ex;
  list_add(&ex->children,ea);

  // +
  expr_t* ey = expr_alloc();
  ey->key = EXPR_IADD;
  ey->rawkey = str_from("+",1);
  ey->lino = ea->lino;
  ey->parent = ex;
  ey->type = ea->type;

  // + (?)
  expr_t* ec = dup_term(ea);
  ec->parent = ey;
  list_add(&ey->children,ec);

  // + _ ?
  eb->parent = ey;
  list_add(&ey->children,eb);

  // set _ (?)
  ey->parent = ex;
  list_add(&ex->children,ey);

  return ex;

}

void compile_tac_tree(expr_t* tree);

void compile_tac_node(list_node_t* it, expr_t* expr){
  // print_syntax_tree(expr,2);
  // printf("=======\n");
  if (expr->key == EXPR_TERM || expr->key == EXPR_TYPE){
    return;
  }
  if (expr->key == EXPR_EXTERN || expr->key == EXPR_FUNCHEAD || expr->key == EXPR_STRUCT){
    return;
  }
  if (expr->key == EXPR_IF){
    list_node_t* jt = expr->children.head;
    expr_t* ex0 = (expr_t*)(jt->data);
    if (ex0->key != EXPR_TERM && ex0->key != EXPR_TYPE){

      expr_t* ex = insert_tac_tmp(it,NULL,ex0);
      ex->parent = expr;
      jt->data = ex;

      compile_tac_node(it->prev, (expr_t*)(((expr_t*)(it->prev->data))->children.tail->data)  );
    }
    compile_tac_tree((expr_t*)((expr->children).head->next->data));
    if ((expr->children).head->next->next){
      compile_tac_tree((expr_t*)((expr->children).head->next->next->data));
    }
    return;
  }

  if (expr->key == EXPR_LAND){
    // a && b
    // t = a; if (t) {t = b;} return t!=0

    list_node_t* jt = expr->children.head;
    expr_t* ex0 = (expr_t*)(jt->data);

    expr_t* ex = insert_tac_tmp(it,NULL,ex0);
    ex->parent = expr;

    compile_tac_node(it->prev, (expr_t*)(((expr_t*)(it->prev->data))->children.tail->data)  );

    insert_tac_ifelseset(it,ex,dup_term(ex),(expr_t*)(jt->next->data),NULL);

    compile_tac_tree((expr_t*)(((expr_t*)(it->prev->data))->children.tail->data) );

    expr->key = EXPR_INEQ;
    expr->rawkey = str_from("!=",0);
    expr->lino = expr->lino;
    list_init(&expr->children);
    map_clear(&expr->symtable);


    list_add(&expr->children,ex);
    list_add(&expr->children,expr_int_const(expr,0));

    return;
  }

  if (expr->key == EXPR_LOR){
    // a || b
    // t = a; s = (t==0); if (s) {t = b} return t!=0

    list_node_t* jt = expr->children.head;
    expr_t* ex0 = (expr_t*)(jt->data);

    expr_t* ex = insert_tac_tmp(it,NULL,ex0);
    ex->parent = expr;

    compile_tac_node(it->prev, (expr_t*)(((expr_t*)(it->prev->data))->children.tail->data)  );

    expr_t* ey = expr_alloc();
    ey->key = EXPR_IEQ;
    ey->rawkey = str_from("=",1);
    ey->lino = expr->lino;
    ey->parent = ex0;
    ey->type = ex->type;
    list_add(&ey->children,ex);
    list_add(&ey->children,expr_int_const(ey,0));

    expr_t* ez = insert_tac_tmp(it,NULL,ey);

    insert_tac_ifelseset(it,ez,dup_term(ex),(expr_t*)(jt->next->data),NULL);

    compile_tac_tree((expr_t*)(((expr_t*)(it->prev->data))->children.tail->data) );

    expr->key = EXPR_INEQ;
    expr->rawkey = str_from("!=",2);
    list_init(&expr->children);
    map_clear(&expr->symtable);


    list_add(&expr->children,ex);
    list_add(&expr->children,expr_int_const(expr,0));

    return;
  }

  if (expr->key == EXPR_TIF){
    // a ? b : c
    // t = a; let s; if (t) {s = b} else {s = c}; return s;

    list_node_t* jt = expr->children.head;

    expr_t* ex0 = (expr_t*)(jt->data);
    expr_t* ex1 = (expr_t*)(jt->next->data);
    expr_t* ex2 = (expr_t*)(jt->next->next->data);

    expr_t* et = insert_tac_tmp(it,NULL,ex0);
    compile_tac_node(it->prev, (expr_t*)(((expr_t*)(it->prev->data))->children.tail->data)  );

    expr_t* es = insert_tac_tmp(it,ex1->type,NULL);

    insert_tac_ifelseset(it,et,es,ex1,ex2);

    compile_tac_tree((expr_t*)(((expr_t*)(it->prev->data))->children.tail->prev->data) );
    
    compile_tac_tree((expr_t*)(((expr_t*)(it->prev->data))->children.tail->data) );

    expr->key = EXPR_TERM;
    expr->rawkey = str_from("",0);
    list_init(&expr->children);
    map_clear(&expr->symtable);
    expr->term = es->term;

    return;
  }

  if (expr->key == EXPR_WHILE){
    // while (a) { b c d }
    // while (1) {t = a; if (t) {} else {break} b c d}
    list_node_t* jt = expr->children.head;
    expr_t* ex0 = (expr_t*)(jt->data);
    expr_t* ex1 = (expr_t*)(jt->next->data);

    jt->data = expr_int_const(expr,1);

    list_node_t* kt = ex1->children.head;

    expr_t* ex = insert_tac_tmp(kt,NULL,ex0);
    ex->parent = ex1;

    insert_tac_breakiffalse(kt,ex);

    compile_tac_tree((expr_t*)((expr->children).tail->data));
    return;

  }


  if (expr->key == EXPR_FOR){
    // for (i = a; b; i += c ){ d e f }
    // if (1) { i = a; while (1) { if (b) {} else {break}  d e f; i += c } }

    list_node_t* jt = expr->children.head;
    expr_t* ex0 = (expr_t*)(jt->data);
    expr_t* ex1 = (expr_t*)(jt->next->data);
    expr_t* ex2 = (expr_t*)(jt->next->next->data);
    expr_t* ex3 = (expr_t*)(jt->next->next->next->data);

    expr_t* blk = tac_block(it);

    expr->parent = blk;

    list_add(&blk->children, expr);

    insert_tac_letset(blk->children.head, ex0, NULL, ex1);

    it->data = blk->parent;


    expr->key = EXPR_WHILE;
    expr->rawkey = str_from("while",5);

    list_pophead(&expr->children);
    list_pophead(&expr->children);
    list_pophead(&expr->children);
    list_pophead(&expr->children);

    list_insert_l(&expr->children,expr->children.head,ex2);


    expr_t* ei = tac_increment(ex0,ex3);
    ei->parent = (expr_t*)(expr->children.tail->data);

    list_add( &((expr_t*)(expr->children.tail->data))->children,ei);

    compile_tac_tree(blk);
    return;

  }

  if (expr->key == EXPR_WHILE || expr->key == EXPR_FUNC || expr->key == EXPR_FORIN){
    //...
    compile_tac_tree((expr_t*)((expr->children).tail->data));
    return;
  }

  
  list_node_t* jt = expr->children.head;
  while (jt){
    expr_t* ex0 = (expr_t*)(jt->data);
    if (ex0->key != EXPR_TERM && ex0->key != EXPR_TYPE){

      expr_t* ex = insert_tac_tmp(it,NULL,ex0);
      ex->parent = expr;
      jt->data = ex;

      compile_tac_node(it->prev, (expr_t*)(((expr_t*)(it->prev->data))->children.tail->data)  );
    }
    jt = jt -> next;
  }
  
}

void compile_tac_tree(expr_t* tree){

  list_node_t* it = tree->children.head;

  while (it){
    compile_tac_node(it,(expr_t*)(it->data));
    it = it -> next;
  }

  return;
}

#endif
