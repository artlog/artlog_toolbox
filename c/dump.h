#ifndef __ALDUMP_H__
#define __ALDUMP_H__

#include "allist.h"
#include "alstrings.h"

void * dump_element(struct allistof * list, struct allistelement * element, struct allistelement * next, int count, void * param);

void dump_list(struct allistof * list);

void * dump_element_full(struct allistelement * element);

void dump_indexset(struct indexset * setp);

void dump_context( struct allistcontext * context);


#endif
