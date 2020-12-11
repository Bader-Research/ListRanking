#ifndef _LISTRANK_H
#define _LISTRANK_H

#include "simple.h"

#define LISTRANK_TYPE     double
#define LISTRANK_OPERATOR +
#define LISTRANK_IDENTITY 0

typedef struct list_d
{
  LISTRANK_TYPE prefix;
  int succ;
} list_t;

/* n = number of elements in the list;
   k = multplier of THREADS, where number of sublists (s) == k*THREADS
   List = the List
   return value = the index of the list head ptr
*/

int list_ranking(int n, int k, list_t *List, THREADED);

#endif

