#ifndef __listrank_step1_h__
#define __listrank_step1_h__


/* This "control block" contains data neede by the SPE.    */
/* When the SPE starts executing it's main() code, the     */
/* first thing it will do is DMA in the contents of this   */
/* control block structure.                                */

typedef struct _control_block {

  unsigned int  chunk_size; /* size, in bytes, of each of these array pieces */
  //  unsigned int  addrSB;     /* address to be filled by single-buffered DMA */
  unsigned int  addrDB;     /* address to be filled by double-buffered DMA */
  unsigned int  addrSBL;    /* address to be filled by single-buffered DMA list */
  unsigned int  addrDBL;    /* address to be filled by double-buffered DMA list */
  unsigned int  addrSUM;
  unsigned char pad[108];   /* pad to a full cache line (128 bytes) */

} control_block;

typedef struct _msg_sum {
  double msg; // 8 messages to be received from the 8 spus
  unsigned char pad[8]; //12 passding for making it to 16 bits .. 
} msg_sum;


#endif /* __listrank_step1_h__ */
