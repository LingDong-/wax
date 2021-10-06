#ifndef WAX_COMMON_H
#define WAX_COMMON_H

#include <stdlib.h>

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

#define INDENT2(x) {int i; for (i = 0; i < (x); i++){str_add(&out, "  ");}}
#define INDENT4(x) {int i; for (i = 0; i < (x); i++){str_add(&out, "    ");}}

extern struct expr_st *expr_t_nullptr;

#define CHILD1 ((expr->children.len>0)?((expr_t*)(expr->children.head->data)):expr_t_nullptr)
#define CHILD2 ((expr->children.len>1)?((expr_t*)(expr->children.head->next->data)):expr_t_nullptr)
#define CHILD3 ((expr->children.len>2)?((expr_t*)(expr->children.head->next->next->data)):expr_t_nullptr)
#define CHILD4 ((expr->children.len>3)?((expr_t*)(expr->children.head->next->next->next->data)):expr_t_nullptr)
#define CHILDN ((expr->children.len>0)?((expr_t*)(expr->children.tail->data)):expr_t_nullptr)

typedef struct str_st {
    size_t len;
    size_t cap;
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

void freex();

str_t censor(str_t);

str_t str_from(const char*, size_t);

str_t str_new();

str_t str_fromch(char);

void str_addch(str_t*, char);

void str_add(str_t*, const char*);

int str_eq(str_t*, const char*);

str_t str_unquote(str_t);

int char_literal(char*);

void list_add(list_t*, void*);

void list_init(list_t *);

void list_pophead(list_t*);

list_t* list_new_on_heap();

void list_insert_l(list_t*, list_node_t*, void*);

void list_pop(list_t*);

void list_insert_r(list_t*, list_node_t*, void*);

int map_hash(str_t);

void map_add(map_t*, str_t, void*);

void map_clear(map_t *);

void* mallocx(size_t);

void* reallocx(void*, size_t);

str_t tmp_name(char*);

str_t base_name(const char*);

#endif /* WAX_COMMON_H */
