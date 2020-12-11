#ifndef __listrank_step3_h__
#define __listrank_step3_h__

#define LS_SIZE_FOR_DATA (1<<14) //4 kb 

#define SUBLISTS_IN_SPU (NO_SUBLISTS/NO_SPU)

#define ELEMENTS_PER_SUBLIST_IN_LS (LS_SIZE_FOR_DATA/(SUBLISTS_IN_SPU*4))

#define ELEMENTS_PER_DMA_LIST_TRANSFER (SUBLISTS_IN_SPU/4)

#define NO_OF_DMA_BUFFERING 8

#define MAX_NO_OF_SUBLISTS_IN_SPU 128

typedef struct _sub_list {
  int head; //first node of sublist i
  int tail; //last node of sublist i
  int lnth; //length of sublist i-1
  int succ; //initially it is the succesor of the head of sublist
} sub_list;

typedef struct _control_block_step3 {
  unsigned int sublists_per_spu;
  unsigned int addrDATA;
  unsigned int addrSUBSTOREDATA[SUBLISTS_IN_SPU];
  unsigned int maxsubsize;
  unsigned int addrSUBSTART;
  unsigned char pad[512-(SUBLISTS_IN_SPU+4)*4]; //NO OF SUBLISTS < 224
} control_block_step3;



#endif /* __listrank_step3_h__ */
