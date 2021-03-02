#ifndef WAX_STD
#define WAX_STD

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>

#define W_MIN(a,b) (((a)<(b))?(a):(b))
#define W_MAX(a,b) (((a)>(b))?(a):(b))

void *w_malloc(size_t size){
    void *mem = malloc(size);
    if(!mem){
        exit(-1);
    }
    return mem;
}

void *w_realloc(void* curr_mem, size_t size){
    void *mem = realloc(curr_mem, size);
    if(!mem){
        exit(-1);
    }
    return mem;
}

void w_free(void* x){
  if (x){
    free(x);
  }
}

typedef struct w_arr_st {
  void* data;
  size_t len;
  size_t cap;
  size_t elem_size;
} w_arr_t;

w_arr_t* w_arr_new_impl(int elem_size){
  w_arr_t* arr = (w_arr_t*)w_malloc(sizeof(w_arr_t));
  arr->elem_size = elem_size;
  arr->len = 0;
  arr->cap = 16;
  arr->data = w_malloc((arr->cap)*elem_size);
  return arr;
}

w_arr_t* w_arr_new_ints(int count,...){
  va_list vals;
  w_arr_t* arr = (w_arr_t*)w_malloc(sizeof(w_arr_t));
  arr->elem_size = sizeof(int);
  arr->len = count;
  arr->cap = count;
  arr->data = w_malloc((arr->cap)*arr->elem_size);

  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      ((int*)arr->data)[i]=va_arg(vals, int);
  }
  va_end(vals);
  return arr;
}
w_arr_t* w_arr_new_flts(int count,...){
  va_list vals;
  w_arr_t* arr = (w_arr_t*)w_malloc(sizeof(w_arr_t));
  arr->elem_size = sizeof(float);
  arr->len = count;
  arr->cap = count;
  arr->data = w_malloc((arr->cap)*arr->elem_size);

  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      ((float*)arr->data)[i]=(float)va_arg(vals, double);
  }
  va_end(vals);
  return arr;
}
w_arr_t* w_arr_new_strs(int count,...){
  va_list vals;
  w_arr_t* arr = (w_arr_t*)w_malloc(sizeof(w_arr_t));
  arr->elem_size = sizeof(char*);
  arr->len = count;
  arr->cap = count;
  arr->data = w_malloc((arr->cap)*arr->elem_size);

  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      ((char**)arr->data)[i]=(char*)va_arg(vals, char*);
  }
  va_end(vals);
  return arr;
}



int* w_vec_new_ints(int count,...){
  va_list vals;
  int* vec = (int*)w_malloc(sizeof(int)*count);
  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      vec[i]=va_arg(vals, int);
  }
  va_end(vals);
  return vec;
}
float* w_vec_new_flts(int count,...){
  va_list vals;
  float* vec = (float*)w_malloc(sizeof(float)*count);
  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      vec[i]=(float)va_arg(vals, double);
  }
  va_end(vals);
  return vec;
}
char** w_vec_new_strs(int count,...){
  va_list vals;
  char** vec = (char**)w_malloc(sizeof(char*)*count);
  va_start(vals, count);
  for (int i = 0; i < count; i++) {
      vec[i]=va_arg(vals, char*);
  }
  va_end(vals);
  return vec;
}

#define w_arr_new(type)         (w_arr_new_impl(sizeof(type)))
#define w_arr_get(type,arr,i  ) (((type *)((arr)->data))[(i)])
#define w_arr_set(type,arr,i,x) (((type *)((arr)->data))[(i)]=(x))

void w_arr_insert_impl(w_arr_t* arr,int i) {
  if ((arr)->len >= (arr)->cap){
    (arr)->cap = (arr)->cap+W_MAX(4,(arr)->cap/2);
    (arr)->data = w_realloc((arr)->data, (arr)->elem_size*((arr)->cap));
  }
  if ((i) < (arr)->len){
    memmove((char*)((arr)->data)+((i)+1)*(arr)->elem_size,
            (char*)((arr)->data)+(i)*(arr)->elem_size, 
           ((arr)->len-(i))*(arr)->elem_size );
  }
  (arr)->len++;
}
#define w_arr_insert(type,arr,i,x) {\
  type tmp__x_ = x;\
  int tmp__i_ = i;\
  w_arr_t* tmp__a_ = arr;\
  w_arr_insert_impl((tmp__a_),(tmp__i_));\
  (((type *)((tmp__a_)->data))[(tmp__i_)]=(tmp__x_));\
}


void w_arr_remove(w_arr_t* arr,int i,int n) {
  memmove((char*)((arr)->data)+(i)*(arr)->elem_size, 
          (char*)((arr)->data)+((i)+(n))*(arr)->elem_size,
          ((arr->len)-(i)-(n))*(arr)->elem_size );
  (arr)->len-=(n);
}

w_arr_t* w_arr_slice(w_arr_t*arr,int i,int n) {
  w_arr_t* brr = (w_arr_t*)w_malloc(sizeof(w_arr_t));
  brr->elem_size = (arr)->elem_size;
  brr->len = n;
  brr->cap = n;
  brr->data = w_malloc((brr->cap)*(brr->elem_size));
  memcpy((char*)(brr->data), (char*)((arr)->data) + (i), (n)*((arr)->elem_size));
  return brr;
}

#define W_NUM_MAP_SLOTS 64

typedef struct w_slot_st {
  int keylen;
  void* key;
  int64_t data;
  struct w_slot_st* next;
} w_slot_t;

typedef struct w_map_st {
  int key_is_ptr;
  size_t len;
  w_slot_t* slots[W_NUM_MAP_SLOTS]; 
} w_map_t;


w_map_t* w_map_new(char key_is_ptr){
  w_map_t* map = (w_map_t*)w_malloc(sizeof(w_map_t));
  map->key_is_ptr = key_is_ptr;
  for (int i = 0; i < W_NUM_MAP_SLOTS; i++){
    map->slots[i] = NULL;
  }
  map->len = 0;
  return map;
}

int w_map_hash(void* ptr, size_t len){
  int x = 0;
  for (size_t i = 0; i < len; i++){
    unsigned char y = *((unsigned char*)((unsigned char*)ptr+i));
    x ^= y;
  }
  x %= W_NUM_MAP_SLOTS;
  return x;
}

void w_map_set(w_map_t* map, int64_t key, int64_t data){
  int keylen;
  void* keyptr;
  if (map->key_is_ptr){
    keylen = strlen((char*)key);
    keyptr = (char*)key;
  }else{
    keylen = sizeof(key);
    keyptr = &key;
  }
  int k = w_map_hash(keyptr,keylen);
  w_slot_t* it = map->slots[k];
  while (it){
    if (keylen == it->keylen){
      if (memcmp(it->key,keyptr,keylen)==0){
        it->data = data;
        return;
      }
    }
    it = it -> next;
  }
  w_slot_t* nxt = map->slots[k];
  w_slot_t* slot = (w_slot_t*)w_malloc(sizeof(w_slot_t));
  slot->key = w_malloc(keylen);
  memcpy(slot->key,keyptr,keylen);
  slot->data=data;
  slot->next = nxt;
  slot->keylen = keylen;
  map->slots[k] = slot;
  map->len++;
}

int64_t w_map_get(w_map_t* map, int64_t key){
  int keylen;
  void* keyptr;
  if (map->key_is_ptr){
    keylen = strlen((char*)key);
    keyptr = (char*)key;
  }else{
    keylen = sizeof(key);
    keyptr = &key;
  }
  int k = w_map_hash(keyptr,keylen);
  w_slot_t* it = map->slots[k];
  while (it){
    if (keylen == it->keylen){

      if (memcmp(it->key,keyptr,keylen)==0){
        return it->data;
      }
    }
    it = it -> next;
  }
  return 0;
}

void w_map_remove(w_map_t* map, int64_t key){
  size_t keylen;
  void* keyptr;
  if (map->key_is_ptr){
    keylen = strlen((char*)key);
    keyptr = (char*)key;
  }else{
    keylen = sizeof(key);
    keyptr = &key;
  }
  int k = w_map_hash(keyptr,keylen);
  w_slot_t* it = map->slots[k];
  w_slot_t* prev = NULL;
  while (it){
    if (keylen == it->keylen){
      if (memcmp(it->key,keyptr,keylen)==0){
        if (prev){
          prev->next = it->next;
        }else{
          map->slots[k] = it->next;
        }
        map->len--;
        w_free(it->key);
        w_free(it);
        return;
      }
    }
    prev = it;
    it = it -> next;
  }
  return;
}

int w_reinterp_f2i(float x){
  return *((int *)&x);
}
float w_reinterp_i2f(int x){
  return *((float *)&x);
}

typedef struct {
  char data[32];
} w_shortstr_t;

w_shortstr_t w_int2str(int x){
  w_shortstr_t str;
  sprintf(str.data, "%d", x);
  return str;
}
w_shortstr_t w_flt2str(float x){
  w_shortstr_t str;
  sprintf(str.data, "%g", x);
  return str;
}

char* w_str_new(char* x){
  size_t l = strlen(x);
  char* str = (char*)w_malloc(l);
  strncpy(str,x,l);
  str[l] = 0;
  return str;
}


char* w_str_cat(char* x, char* y){
  size_t l0 = strlen(x);
  size_t l1 = strlen(y);
  x = (char*)w_realloc(x,l0+l1+1);
  memcpy(x+l0,y,l1);
  x[l0+l1] = 0;
  return x;
}


char* w_str_add(char* x, int y){
  char c = (char)y;
  size_t l = strlen(x);
  x = (char*)w_realloc(x,l+2);
  x[l] = c;
  x[l+1]=0;
  return x;
}

char* w_str_cpy(char* x, int i, int l){
  char* y = (char*)w_malloc(l+1);
  memcpy(y,x+i,l);
  y[l] = 0;
  return y;
}

void w_free_arr(w_arr_t* x){
  if (x){
    w_free(x->data);
    w_free(x);
  }
}
void w_free_map(w_map_t* map){
  if (!map){
    return;
  }
  for (int i = 0; i < W_NUM_MAP_SLOTS; i++){
    w_slot_t* it = map->slots[i];
    while (it){
      w_slot_t* nxt = it->next;
      w_free(it->key);
      w_free(it);
      it = nxt;
    }
  }
  w_free(map);
}

#endif
