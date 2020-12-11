#include "../listrank.h"
#include "../listrank_step1.h"
#include "../listrank_step3.h"
#include <sched.h>
#include <libspe.h>
#include <stdio.h>
#include <errno.h>

#include <sys/time.h>


/* eight control blocks, one for each SPE. */
control_block cb[NO_SPU] __attribute__ ((aligned (128)));
control_block_step3 cb3[NO_SPU] __attribute__ ((aligned (128)));

/* pointer to the SPE code, used at thread creation time */
extern spe_program_handle_t listrank_spu; //for calculating head
extern spe_program_handle_t listrank_spu_step3; //for calculating rank within sublist

/* group environment, gid, to handle the group of threads*/
spe_gid_t gid;

/* 8 handles returned by "spe_create_thread" */
speid_t speids[NO_SPU];

/* variable used to return data regarding an abnormal return from the SPE */
int status[NO_SPU];

/* DATA - list */
int *data;

/* Storing Sublists in contiguous locations in memory*/
int *data_sublist;

/* structure that holds the return value of sum of indices .. to calculate the head, dma transfers minimum 16 bytes, 12 byte padding is used in this structure */
msg_sum msg[NO_SPU] ;


sub_list sublist[NO_SUBLISTS];

void load_data_sequential(unsigned long array_size) {
  unsigned long i;
  for (i=0; i<array_size; i++) data[i] = i+1;
  data[array_size] = -1; // data[array_size] acts as a ground for the end os list.
}


void load_data_random(unsigned long array_size) {
  unsigned long i;
  int i1,j1,temp;
  int current=0;
  int *data1;
  data1 = (int *) malloc(127 + (array_size+1)*sizeof(int));
  while (((int) data1) & 0x7f) ++data1;

  srand48(23);
  //  srand48(time(NULL));

  for (i=0; i<array_size; i++) data1[i] = i+1;
  data1[array_size] = -1;

  //n swaps for fairly random access
  for(i=0;i<2*array_size;i++) {
    i1 = j1 = (int)(drand48() * (array_size-1));
    while(i1 == j1)
      j1 = (int)(drand48() * (array_size-1));//rand() % (array_size-1);
    
    temp = data1[i1];
    data1[i1] = data1[j1];
    data1[j1] = temp;
  }
  
  //make random permutation
  data[0] = data1[0];
  current = data[0];
  for(i=0;i<array_size;i++) {
    data[current] = data1[i+1];
    current = data1[i+1];
  }
  data[array_size] = -1;
}



int main(int argc, char *argv[]) {
  double sumall; //total sum
  unsigned int headfromsum;
  double totalsumexp;
  int i,j,l;
  int log_array_size;
  unsigned long array_size,chunk_size;
  int sublistheadnode;
  unsigned int lists_per_spu;
  unsigned int sub_size;
  int randomseq;
  int rank_headnodes[NO_SUBLISTS];
  int verify,head1,dhead1;

#ifdef PROFILING
  struct timeval start, finish;
  int usec;
#endif

  /* user specifies the size of the large array */
  if (argc != 3) {
    printf("usage: listrank <log of #elements in big array (16 <= x <= 22)> <random or sequential <{0,1}>>\n");
    return -1;
  }

  log_array_size = atoi(argv[1]);
  randomseq = atoi(argv[2]);

  if (log_array_size < 16 || log_array_size > 22) {
    printf("usage: listrank <log of #elements in big array (16 <= x <= 22)> <random or sequential <{0,1}>>\n");
    return -1;
  }

  /* compute array size from input parameter */
  array_size = 1 << log_array_size;

  /**** CALCULATING HEAD ****/

  /* dividing task into the 8 speis using double buffering, thus dividing array size by 8 */
  chunk_size = array_size / NO_SPU;

  printf("Initializing  list ... \n");
  /* the big array needs to be aligned on a 128-byte cache line */
  data = (int *) malloc(127 + (array_size+1)*sizeof(int));
  while (((int) data) & 0x7f) ++data;

  /* load the array with initial values (sequential for now) */
  if(randomseq == 0)
    load_data_random(array_size);
  else
    load_data_sequential(array_size);

  /* Create an SPE group which enables SPE events. */
  printf("Calculating the location of head ... \n");
  gid = spe_create_group (SCHED_OTHER, 0, 1);
  if (gid == NULL) {
    fprintf(stderr, "Failed spe_create_group(errno=%d)\n", errno);
    return -1;
  }

  if (spe_group_max (gid) < 8) {
    fprintf(stderr, "System doesn't have 8 working SPEs.  I'm leaving.\n");
    return -1;
  }

  /* load the control blocks for each SPE with data, giving the address for start of data for each SPE*/
  for (i = 0; i < NO_SPU; i++) {
    cb[i].chunk_size = chunk_size * sizeof(int); /* convert to units of bytes */
    cb[i].addrDB     = (unsigned int) &data[chunk_size*i];
    cb[i].addrSUM    = (unsigned int) &msg[i];
  }

  /* allocate SPE tasks */
  for (i = 0; i < NO_SPU; i++) {
    speids[i] = spe_create_thread (gid, &listrank_spu, (unsigned long long *) &cb[i], NULL, -1, 0);
    if (speids[i] == NULL) {
      fprintf (stderr, "FAILED: spe_create_thread(num=%d, errno=%d)\n", i, errno);
      exit (3+i);
    }
  }

  /* waiting for SPEs to all finish */
  for (i=0; i<NO_SPU; ++i) spe_wait(speids[i], &status[i], 0);
  /* the task for the SPEs was to sum the data of all array elements allocated to it */
  /* now sum them all, and subtract from the total */
  sumall = 0.0;
  for (i=0; i<NO_SPU; ++i) {
    sumall += msg[i].msg;
  }
  totalsumexp = ((double)(array_size+1))*((double)array_size/2.0);
  sumall -= totalsumexp;
  headfromsum = (unsigned int)sumall;
  
#ifdef PROFILING
  gettimeofday(&start,NULL);
#endif

  /*** Choosing sublists ***/ 
  printf("Choosing sublist head nodes ... \n");
  sublist[0].head = headfromsum;
  sublist[0].tail = 0;
  sublist[0].lnth = 0;
  sublist[0].succ = data[headfromsum];

  for(i = 1; i < NO_SUBLISTS; i++) {
    sublistheadnode = i * (array_size / NO_SUBLISTS);
    if(sublistheadnode == headfromsum) sublistheadnode++;//doesnt clash with the headof list
    sublist[i].head = sublistheadnode;
    sublist[i].tail = 0;
    sublist[i].lnth = 0;
    sublist[i].succ = data[sublistheadnode];
    data[sublistheadnode] = -2;
  }
  lists_per_spu = NO_SUBLISTS / NO_SPU;

  /*** Allocate sublists to spus ***/

  printf("Calculating rank of nodes w.r.t. the sublist head nodes ... \n");
  /* Create an SPE group which enables SPE events. */
  gid = spe_create_group (SCHED_OTHER, 0, 1);
  if (gid == NULL) {
    fprintf(stderr, "Failed spe_create_group(errno=%d)\n", errno);
    return -1;
  }

  if (spe_group_max (gid) < 8) {
    fprintf(stderr, "System doesn't have eight working SPEs.  I'm leaving.\n");
    return -1;
  }
  
  sub_size = 2*array_size + X*array_size; 
  /* the big array needs to be aligned on a 128-byte cache line */
  data_sublist = (int *) malloc(127 + sub_size*sizeof(int));
  while (((int) data_sublist) & 0x7f) ++data_sublist;

  /* load the control blocks for each SPE with data, giving the address for start of data for each SPE*/
  for (i = 0; i < NO_SPU; i++) {
    cb3[i].sublists_per_spu = lists_per_spu;
    cb3[i].addrDATA = (unsigned int) &data[0];
    for(j=0;j<lists_per_spu;j++)
      cb3[i].addrSUBSTOREDATA[j] = (unsigned int) &data_sublist[(sub_size/NO_SUBLISTS)*(lists_per_spu*i+j)];
    cb3[i].maxsubsize = sub_size/NO_SUBLISTS;
    cb3[i].addrSUBSTART    = (unsigned int) &sublist[(i*NO_SUBLISTS)/NO_SPU];
  }
  
  /**********************************/
  
  for (i = 0; i < NO_SPU; i++) {
    speids[i] = spe_create_thread (gid, &listrank_spu_step3, (unsigned long long *) &cb3[i], NULL, -1, 0);
    if (speids[i] == NULL) {
      fprintf (stderr, "FAILED: spe_create_thread(num=%d, errno=%d)\n", i, errno);
      exit (3+i);
    }
  }
  
  /* waiting for SPEs to all finish */
  for (i=0; i<NO_SPU; ++i) 
    spe_wait(speids[i], &status[i], 0);
  
  printf("Calculating global ranks ... \n");

  for(i=0;i<NO_SUBLISTS;i++) {
    data[sublist[i].head] = i; //storing sublist no in tail
  }
  
  i = headfromsum;
  j = 0;
  
  while(sublist[i].succ != array_size) {
    rank_headnodes[i] = j;
    j += sublist[i].lnth;
    //printf("i: %d, rank: %d, succ_sublist: %d \n",i, rank_headnodes[i], data[sublist[i].succ]);
    i = data[sublist[i].succ];
    
  }
  
#ifdef PROFILING
  gettimeofday(&finish,NULL);
  usec = finish.tv_sec*1000*1000 + finish.tv_usec;
  usec -= (start.tv_sec*1000*1000 + start.tv_usec);
  printf("Time for list size %d is  %f millisec\n", array_size, ((float)usec/(LOOPOVER*1000)));
#endif

#ifdef OUTPUT
  printf("\n**************************************\n");
  for(i=0;i<NO_SUBLISTS;i++) {
    printf("sublist head: %d   tail: %d length: %d\n",sublist[i].head,sublist[i].succ,sublist[i].lnth);
  }
  printf("**************************************\n");
#endif

#ifdef VERIFY
  printf("Checking for verification ...\n");
  verify = 1;
  for(i=0;i<NO_SUBLISTS;i++) {
    head1 = sublist[i].head;
    dhead1 = data_sublist[i*(sub_size/NO_SUBLISTS)];
    if(sublist[i].lnth*NO_SUBLISTS > sub_size) {
      printf("Error: %d length of sublist to huge to fit in array, please run again\n",sublist[i].lnth);
      return 0;
    }
    for(j=0;j<sublist[i].lnth;j++) {
      if(data_sublist[i*(sub_size/NO_SUBLISTS)+j] != dhead1) {
	verify = 0;
      }
      dhead1 = data[dhead1];
    }
  }
  if(verify) 
    printf("Verified !\n");
  else 
    printf("Incorrect results .. \n");
  
#endif

  __asm__ __volatile__ ("sync" : : : "memory");
  return 0;
}
