#include "listrank-SMP.h"
#include <sys/types.h>

#define NANO 1000000000
int nlist;

void ListFB(list_t * list)
{ int i;

  for (i = 0; i < nlist; i++) list[i].succ = i + 1;
  list[nlist-1].succ = 0;
}


void ListBF(list_t *list)
{ int i;

  list[0].succ = 0;
  list[1].succ = 0;
  for (i = 2; i <nlist; i++) list[i].succ = i - 1;
}


void ListRandom(list_t *list)
{ 
  int i,j,t, *buf;
  double s;
  
  buf = malloc(sizeof(int)*nlist);
  
  for(i=0; i<nlist; i++)
  	buf[i]=i;


  for(i=0; i<nlist;i++)
  	{
		s = drand48();
        t = i+(s*nlist-i);
        if(t!=i)
        {
			j = buf[i];
			buf[i]=buf[t];
			buf[t]=j;
        }

	}
 
  for(i=0;i<nlist-1;i++)
  	list[buf[i]].succ=buf[i+1];

  list[buf[nlist-1]].succ=0;
  
  free(buf);
}

void *SIMPLE_main(THREADED)
{
	list_t * list;
	double interval;
	int i,j,k;

	on_one {
		nlist = atoi(THARGV[0]);
	}
	if (THARGC != 1) {
		printf("\nCall with one argument ...\n");
		printf("listrank-smp -t <no. of threads> -- <no. of elements in list>\n");
		exit(-1);
	}

	node_Barrier();
	
	list = node_malloc(sizeof(list_t)*nlist,TH);
	
	on_one{
		ListFB(list);
	}
	node_Barrier();
	
	k = (int) log2(nlist);
	list_ranking(nlist, k, list, TH);
	node_Barrier();

    	on_one{
		ListBF(list);
	}
	node_Barrier();
	
	k = (int) log2(nlist);
	list_ranking(nlist, k, list, TH);
	node_Barrier();
	
	on_one{
		ListRandom(list);
	}
	node_Barrier();
	
	k = (int) log2(nlist);
	list_ranking(nlist, k, list, TH);
	node_Barrier();
	
	node_free(list,TH);
	SIMPLE_done(TH);
}
