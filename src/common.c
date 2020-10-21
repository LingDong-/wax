#ifndef WAX_COMMON
#define WAX_COMMON

#define no_segfault_yet printf("no segfault yet on line %d!\n",__LINE__);

#ifndef NO_ESC_COLOR
	#define printerr(x) printf("\033[0;31m[" x " error]\033[0m ");printf
#else
	#define printerr(x) printf("[" x " error] ");printf
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

typedef struct str_st {
  int len;
  int cap;
  char* data;
} str_t;


typedef struct list_node_st {
  struct list_node_st *prev;
  struct list_node_st *next;
  void* data;
} list_node_t;

typedef struct list_st {
  list_node_t *head;
  list_node_t *tail;
  int len;
} list_t;


#define NUM_MAP_SLOTS 64
typedef struct map_st {
  int len;
  list_t* slots[NUM_MAP_SLOTS];
} map_t;


list_t list_new(){
  list_t l;
  l.head = NULL;
  l.tail = NULL;
  l.len = 0;
  return l;
}


void list_add_no_reg(list_t* l, void* dataptr){
  list_node_t* n = (list_node_t*)malloc( sizeof(list_node_t) );
  n->data = dataptr;
  n->next = NULL;
  n->prev = l->tail;
  if (l->head == NULL){
    l->head = n;
  }else{
    l->tail->next = n;
  }
  l->tail = n;
  l->len ++;
}


list_t mallocxed;
int mallocxed_initialized = 0;
int num_mallocxed_bytes = 0;

void* mallocx(size_t size){
  if (!mallocxed_initialized){
    mallocxed = list_new();
    mallocxed_initialized = 1;
  }
  void* ptr = malloc(size);
  if (!ptr){
    printf("[error] compiler out of memory.\n");
    exit(1);
  }
  num_mallocxed_bytes += size;
  list_add_no_reg(&mallocxed,ptr);
  return ptr;
}

void freex(){
  #ifdef printinfo
    printinfo("[info] freeing %d bytes (%d objects).\n",num_mallocxed_bytes,mallocxed.len);
  #endif
  if (!mallocxed_initialized){
    return;
  }
  list_node_t* it = mallocxed.head; 
  while (it){
    list_node_t* nxt = it->next;
    // printf("%p\n",it);
    free(it->data);
    free(it);
    it = nxt;
  }
  mallocxed_initialized = 0;
  num_mallocxed_bytes = 0;
}

void* reallocx(void* oldptr, size_t size){
  void* newptr = realloc(oldptr,size);

  if (!mallocxed_initialized){
    return NULL;
  }
  list_node_t* it = mallocxed.tail; 
  while (it){
    list_node_t* nxt = it->prev;
    if ((void*)it->data == oldptr){
      it->data = newptr;
      break;
    }
    it = nxt;
  }
  return newptr;
}

str_t str_new(){
  str_t s;
  s.cap = 31;
  s.data = (char*) mallocx((s.cap+1)*sizeof(char));
  s.data[0] = 0;
  s.len = 0;
  return s;
}

str_t str_from(char* cs, int l){
  str_t s;
  s.cap = l;
  s.len = l;
  s.data = (char*)mallocx(l+1);
  strncpy(s.data,cs,l);
  s.data[l] = 0;
  return s;
}

str_t str_fromch(char c){
  str_t s;
  s.cap = 1;
  s.len = 1;
  s.data = (char*)mallocx(sizeof(char)*2);
  s.data[0] = c;
  s.data[1] = 0;
  return s;
}


void str_add (str_t* s, char* cs){
  int l = strlen(cs);
  if (s->cap < s->len + l){
    int hs = s->cap/2;
    s->cap = s->len+MAX(l,hs);
    s->data = (char*)reallocx(s->data, s->cap+1 );
  }
  strcpy(&s->data[s->len],cs);
  s->len += l;
  s->data[s->len] = 0;
}

void str_addconst (str_t* s, const char* cs){
  int l = strlen(cs);
  if (s->cap < s->len + l){
    int hs = s->cap/2;
    s->cap = s->len+MAX(l,hs);
    s->data = (char*)reallocx(s->data, s->cap+1 );
  }
  strcpy(&s->data[s->len],cs);
  s->len += l;
  s->data[s->len] = 0;
}

void str_addch (str_t* s, char c){
 
  if (s->cap < s->len + 1){
    int hs = s->cap/2;
    s->cap = s->len+MAX(1,hs);
    s->data = (char*)reallocx(s->data, s->cap+1 );
  }
  s->data[s->len] = c;
  s->len += 1;
  s->data[s->len] = 0;
}


list_t* list_new_on_heap(){
  list_t* l = (list_t*)mallocx(sizeof(list_t));
  l->head = NULL;
  l->tail = NULL;
  l->len = 0;
  return l;
}


void list_add(list_t* l, void* dataptr){
  list_node_t* n = (list_node_t*)mallocx( sizeof(list_node_t) );
  n->data = dataptr;
  n->next = NULL;
  n->prev = l->tail;
  if (l->head == NULL){
    l->head = n;
  }else{
    l->tail->next = n;
  }
  l->tail = n;
  l->len ++;
}

void list_insert_l(list_t* l, list_node_t* node, void* dataptr){
  if (l->head == NULL){
    list_add(l,dataptr);
    return;
  }
  list_node_t* n = (list_node_t*)mallocx( sizeof(list_node_t) );
  n->data = dataptr;

  n->prev = node->prev;
  n->next = node;

  if (l->head == node){
    l->head = n;
  }else{
    node->prev->next = n;
  }
  node->prev = n;

  l->len ++;
}

void list_insert_r(list_t* l, list_node_t* node, void* dataptr){
  if (node == l->tail || l->head == NULL){
    list_add(l,dataptr);
    return;
  }
  list_node_t* n = (list_node_t*)mallocx( sizeof(list_node_t) );
  n->data = dataptr;
  n->prev = node;
  n->next = node->next;
  node->next->prev = n;
  node->next = n;

  l->len ++;
}

void list_pop(list_t* l){
  if (l->tail == l->head){
    l->head = NULL;
    l->tail = NULL;
    l->len --;
    return;
  }
  l->tail->prev->next = NULL;
  l->tail = l->tail->prev;
  l->len --;
}

void list_pophead(list_t* l){
  if (l->tail == l->head){
    l->head = NULL;
    l->tail = NULL;
    l->len --;
    return;
  }
  l->head->next->prev = NULL;
  l->head = l->head->next;
  l->len --;
}


map_t map_new(){
  map_t m;
  for (int i = 0; i < NUM_MAP_SLOTS; i++){
    m.slots[i] = NULL;
  }
  m.len = 0;
  return m;
}

int map_hash(str_t s){
  int x = 0;
  for (int i = 0; i < s.len; i++){
    char y = s.data[i];
    x ^= y;
  }
  x %= NUM_MAP_SLOTS;
  return x;
}

void map_add(map_t* m, str_t s, void* dataptr){
  int k = map_hash(s);
  if (m->slots[k] == NULL){
    m->slots[k] = list_new_on_heap();
  }
  list_add(m->slots[k],dataptr);
  m->len++;
}

bool str_eq(str_t* s, const char* cs){
  return strcmp(s->data,cs) == 0;
}

str_t str_unquote(str_t src){
  str_t out = str_new();
  int i = 1;
  while (i < src.len-1){
    if (src.data[i]=='\\'){
      if (src.data[i+1]=='"'){
        str_addch(&out,'"');
        i+=2;
      }else{
        str_addch(&out,src.data[i]);
        i++;
      }
    }else{
      str_addch(&out,src.data[i]);
      i++;
    }
  }
  return out;
}



#define INDENT2(x) for (int i = 0; i < (x); i++){str_add(&out, "  ");}

#define CHILD1 ((expr->children.len>0)?((expr_t*)(expr->children.head->data)):NULL)
#define CHILD2 ((expr->children.len>1)?((expr_t*)(expr->children.head->next->data)):NULL)
#define CHILD3 ((expr->children.len>2)?((expr_t*)(expr->children.head->next->next->data)):NULL)
#define CHILD4 ((expr->children.len>3)?((expr_t*)(expr->children.head->next->next->next->data)):NULL)
#define CHILDN ((expr->children.len>0)?((expr_t*)(expr->children.tail->data)):NULL)

int tmp_name_cnt = 0;
str_t tmp_name(char* prefix){
  str_t out = str_from(prefix,strlen(prefix));
  // for (int i = 0; i < 4; i++){
  //   str_addch(&out, RAND%26+97);
  // }
  char x[16];
  sprintf(x,"%02x",tmp_name_cnt);
  tmp_name_cnt ++;
  str_add(&out,x);
  return out;
}


str_t base_name(char* path){
	int l = strlen(path);
	if (!l){
		return str_new();
	}
	int i = 0;
	int j = -1;

	for (i = l-1; i >= 0; i--){
		
		if (path[i] == '.' && j == -1){
			j = i;
		}
		if (path[i] == '/' || path[i] == '\\'){
			i++;
			break;
		}
	}
	if (i < 0){
		i = 0;
	}
	str_t s = str_from((char*)&path[i],j-i);

	return s;

}

#endif
