#include "../listrank.h"
#include "../listrank_step1.h"
#include <spu_mfcio.h>
#include <stdio.h>

/* data buffer in local store, to hold the data that is brought using DMA */
/* DMA in 4096 elements at a time - largest size of a standard DMA (16,384 bytes) */
/* Since double-buffering, allocate 8192.    */
int databuffer[8192] __attribute__ ((aligned (128)));

/* Here we define the pointers which will point at the upper and lower parts   */
/* of this data buffer.                                                        */
int *data[2];

/* how many DMA cycles each task must perform */
int loopcount;
double sum;
/* control structure */
control_block cb __attribute__ ((aligned (128)));

/* Sum the data that is brought using DMA*/

void sum_data(int *dest) { 
  int i;
  for (i=0; i<4096; ++i) {
    sum = sum + (double)dest[i];
  }
}

/* DMA task modules. */
/* Each of them processes the data in the assigned regions in 16 kbyte chunks */

void load_doublebuffer(unsigned int addr)
{
  int i;
  mfc_get(data[0], addr, 16384, 20, 0, 0);
  for (i=1; i<loopcount; ++i) {
    mfc_get(data[i&1], addr+16384*i, 16384, 20+(i&1), 0, 0);
    mfc_write_tag_mask(1<<(21-(i&1)));
    mfc_read_tag_status_all();
    sum_data(data[(i-1)&1]);
  }
  mfc_write_tag_mask(1<<(20+(i-1)&1));
  mfc_read_tag_status_all();
  sum_data(data[(i-1)&1]);
}

/* here is the location where the SPE begins execution, once its thread is created */
int main(unsigned long long speid, addr64 argp, addr64 envp) {
  sum = 0.0;
  double sumptr[2];
  /* DMA control block information from system memory. */
  mfc_get(&cb, argp.ui[1], sizeof(cb), 31, 0, 0);
  mfc_write_tag_mask(1<<31);
  mfc_read_tag_status_all();

  /* compute how many DMA cycles will be needed by each task */
  loopcount = cb.chunk_size >> 14;
  /* load the pointers so the point to the right part of the local store buffer */
  data[0] = &databuffer[0];
  data[1] = &databuffer[4096];

  /* run the four tasks, indicating which portion of memory they should work with */
  load_doublebuffer     (cb.addrDB);

  /* dma the sum back to ppu*/
  sumptr[0] = sum;

  mfc_put(sumptr, cb.addrSUM, 16, 20, 0, 0);
  mfc_write_tag_mask(1<<20);
  mfc_read_tag_status_all();
  
  return 0;
}
