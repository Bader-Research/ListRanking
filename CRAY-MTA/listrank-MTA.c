#define NWALK     16384
#define NLIST  40000000

#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include <sys/mta_task.h>
#include <machine/mtaops.h>
#include <machine/runtime.h>

int list[NLIST+1], rank[NLIST+1];

double time1, time2, time3, time4, time5, time6;

double timer()
{ return((double)mta_get_clock(0) / mta_clock_freq()); }

double phantoms()
{ return((double)MTA_COUNT_PHANTOMS() / mta_clock_freq()); }


void ListFB(int *list)
{ int i;

  list[0]     = 0;
  list[NLIST] = 0;
  for (i = 1; i < NLIST; i++) list[i] = i + 1;
}


void ListBF(int *list)
{ int i;

  list[0] = 0;
  list[1] = 0;
  for (i = 2; i <= NLIST; i++) list[i] = i - 1;
}


void ListRandom(int *list)
{ int i, j, k, head, next, temp[NLIST];
  double rn[NLIST];

  j = NLIST;
  prand_(&j, rn);
  for (i = 0; i < NLIST; i++) temp[i] = i + 1;

  k       = j * rn[j - 1];
  head    = temp[k];
  temp[k] = temp[j - 1];

  for (j = NLIST - 1; j > 0; j--) {
      k          = j * rn[j - 1];
      list[head] = temp[k];
      head       = temp[k];
      temp[k]    = temp[j - 1];
  }

  list[0]    = 0;
  list[head] = 0;
}


void RankList(list, rank)
  int *list, *rank;
{ int i, first;
  int tmp1[NWALK+1], tmp2[NWALK+1];
  int head[NWALK+1], tail[NWALK+1], lnth[NWALK+1], next[NWALK+1];

#pragma mta assert noalias *rank, head, tail, lnth, next, tmp1, tmp2

  time1 = timer();

/* find first node in list */
/*                         */
  first = 0;
#pragma mta use 100 streams
  for (i = 1; i <= NLIST; i++) first += list[i];

  first = ((NLIST * NLIST + NLIST) / 2) - first;

  time2 = timer();

/* initialize walks */
/*                  */
/* Initialize walks by marking every (NLIST/NWALK) node.         */
/* head[i] = first node of walk i                                */
/* tail[i] = last  node of walk i                                */
/* lnth[i] = length of walk i - 1                                */
/* next[i] = id of walk starting after tail[i]                   */
/*                                                               */
/* Temporarily, let rank(i) = walk id, where i is a marked node. */
  head[0] = 0;
  tail[0] = 0;
  lnth[0] = 0;
  rank[0] = 0;

  head[1]     = first;
  tail[1]     = 0;
  lnth[1]     = 0;
  rank[first] = 1;

  for (i = 2; i <= NWALK; i++) {
      int node = i * (NLIST / NWALK);
      head[i]    = node;
      tail[i]    = 0;
      lnth[i]    = 0;
      rank[node] = i;
  }

  time3 = timer();

/* from each head node, walk list until the next head node */
/*                                                         */
#pragma mta use 100 streams
#pragma mta assert no dependence lnth 
  for (i = 1; i <= NWALK; i++) {
      int j, count, next_walk;

         count = 0;
         j     = head[i];
         do {count++; j = list[j];} while (rank[j] == -1);

         next_walk = rank[j];

         tail[i]         = j;
         lnth[next_walk] = count;
         next[i]         = next_walk;
  }

  time4 = timer();

/* in a manner similar to the middle step of cyclic reduction, */
/* compute the rank of the head node of each walk              */
/*                                                             */
  while (next[1] != 0) {

#pragma mta assert no dependence tmp1
      for (i = 1; i <= NWALK; i++) {
          int n   = next[i];
          tmp1[n] = lnth[i];
          tmp2[i] = next[n];
      }

      for (i = 1; i <= NWALK; i++) {
          lnth[i] += tmp1[i];
          next[i]  = tmp2[i];
          tmp1[i]  = 0;
  }   }

  time5 = timer();

/* for each walk, compute the rank of each node of the walk */
/*                                                          */
#pragma mta use 100 streams
#pragma mta assert no dependence *rank
  for (i = 1; i <= NWALK; i++) {
      int j, k, count;
      j     = head[i];
      k     = tail[i];
      count = NLIST - lnth[i];
      while (j != k) {rank[j] = count; count--; j = list[j];}
  }

  time6 = timer();
}


void main()
{ int i;

  ListFB(list);
  /* ListBF(list); */
  /* ListRandom(list); */

  for (i = 0; i <= NLIST; i++) rank[i] = -1;

  RankList(list, rank);

  printf("First time = %lf\n", time2 - time1);
  printf("Init  time = %lf\n", time3 - time2);
  printf("Walk  time = %lf\n", time4 - time3);
  printf("Count time = %lf\n", time5 - time4);
  printf("Rank  time = %lf\n", time6 - time5);
  printf("Total time = %lf\n", time6 - time1);

#pragma mta assert parallel
  for (i = 1; i <= NLIST; i++)
      if (rank[i] != rank[list[i]] + 1) printf("Error %d\n", i);
}
