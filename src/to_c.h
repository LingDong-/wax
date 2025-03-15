#ifndef WAX_TO_C_H
#define WAX_TO_C_H

#include "common.h"
#include "parser.h"

str_t expr_to_c(expr_t*, int);

str_t tree_to_c(str_t, expr_t*, map_t*, map_t*);

#endif /* WAX_TO_C_H */
