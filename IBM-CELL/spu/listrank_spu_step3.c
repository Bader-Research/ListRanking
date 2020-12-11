#include "../listrank.h"
#include "../listrank_step3.h"
#include <spu_mfcio.h>
#include <profile.h>
#include <stdio.h>

int databufferDMA[SUBLISTS_IN_SPU*4] __attribute__ ((aligned (128)));
int databuffer[SUBLISTS_IN_SPU] __attribute__ ((aligned (128)));
int *data[NO_OF_DMA_BUFFERING];
int *dataDMA[NO_OF_DMA_BUFFERING];

int data_SUBLIST_SPU[(LS_SIZE_FOR_DATA/4)] __attribute__ ((aligned(128)));
int currlnth[SUBLISTS_IN_SPU]; 
int runsublist[SUBLISTS_IN_SPU];

control_block_step3 cb __attribute__ ((aligned (128)));

sub_list sublist[SUBLISTS_IN_SPU];

volatile unsigned int dma_list_cont[NO_OF_DMA_BUFFERING*MAX_NO_OF_SUBLISTS_IN_SPU] __attribute__ ((aligned (8)));
unsigned int *dma_list[NO_OF_DMA_BUFFERING];

int sum_runsublist() {
  int i,sum;
  sum = 0;
  for(i=0;i<SUBLISTS_IN_SPU;i++) 
    sum += runsublist[i];
  return sum;
}

int main(unsigned long long speid, addr64 argp, addr64 envp) {
  unsigned int i,j,k;
  unsigned int sublists_per_dma;
  unsigned int flag;
  unsigned int subliststart;
  unsigned int datastorepos;
  int loopover;
  unsigned int tstart;
  unsigned int tstop;

  for(i=0;i<NO_OF_DMA_BUFFERING;i++) {
    dma_list[i] = &dma_list_cont[i*MAX_NO_OF_SUBLISTS_IN_SPU];
  }

  mfc_get(&cb, argp.ui[1], sizeof(cb), 31, 0, 0);
  mfc_write_tag_mask(1<<31);
  mfc_read_tag_status_all();

  sublists_per_dma = SUBLISTS_IN_SPU/NO_OF_DMA_BUFFERING;
  for(i=0;i<NO_OF_DMA_BUFFERING;i++) {
    data[i] = &databuffer[i*sublists_per_dma];
    dataDMA[i] = &databufferDMA[4*i*sublists_per_dma];
  }

#ifdef PROFILING
  for(loopover=0;loopover<LOOPOVER;loopover++) {
#endif
    mfc_get(sublist, cb.addrSUBSTART, 8*4*sizeof(unsigned int), 20, 0, 0);
    runsublist[0] = 1;
    currlnth[0] = 0;
    runsublist[1] = 1;
    currlnth[1] = 0;
    runsublist[2] = 1;
    currlnth[2] = 0;
    runsublist[3] = 1;
    currlnth[3] = 0;
    runsublist[4] = 1;
    currlnth[4] = 0;
    runsublist[5] = 1;
    currlnth[5] = 0;
    runsublist[6] = 1;
    currlnth[6] = 0;
    runsublist[7] = 1;
    currlnth[7] = 0;
    mfc_write_tag_mask(1<<20);
    mfc_read_tag_status_all();
    
    
    data[0][0] = sublist[0].succ;
    dataDMA[0][(data[0][0]%4)] = data[0][0];
    data[1][0] = sublist[1].succ;
    dataDMA[1][(data[1][0]%4)] = data[1][0];
    data[2][0] = sublist[2].succ;
    dataDMA[2][(data[2][0]%4)] = data[2][0];
    data[3][0] = sublist[3].succ;
    dataDMA[3][(data[3][0]%4)] = data[3][0];
    data[4][0] = sublist[4].succ;
    dataDMA[4][(data[4][0]%4)] = data[4][0];
    data[5][0] = sublist[5].succ;
    dataDMA[5][(data[5][0]%4)] = data[5][0];
    data[6][0] = sublist[6].succ;
    dataDMA[6][(data[6][0]%4)] = data[6][0];
    data[7][0] = sublist[7].succ;
    dataDMA[7][(data[7][0]%4)] = data[7][0];
    flag = 1;
    
    while(flag) {
      for(k=0;k<(ELEMENTS_PER_SUBLIST_IN_LS-1);k++) {
	//	prof_stop();
	mfc_write_tag_mask(1<<(10));
	mfc_read_tag_status_all();
	
	if(runsublist[0] == 1) {
	  data[0][0] = dataDMA[0][(data[0][0]%4)];
	  if(data[0][0] >= 0) {
	    sublist[0].succ = data[0][0];
	    dma_list[0][0] = sizeof(int);
	    dma_list[0][1] = cb.addrDATA + (data[0][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[0],(unsigned int)&dma_list[0][0],((sizeof(int))*2),10,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[0] = 0;
	  }
	}
	
	if(runsublist[0] == 1) {
	  datastorepos = currlnth[0];
	  data_SUBLIST_SPU[datastorepos] = data[0][0];
	  currlnth[0] += 1;
	}
	
	mfc_write_tag_mask(1<<(11));
	mfc_read_tag_status_all();
	if(runsublist[1] == 1) {
	  data[1][0] = dataDMA[1][(data[1][0]%4)];
	  if(data[1][0] >= 0) {
	    sublist[1].succ = data[1][0];
	    dma_list[1][0] = sizeof(int);
	    dma_list[1][1] = cb.addrDATA + (data[1][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[1],(unsigned int)&dma_list[1][0],((sizeof(int))*2),11,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[1] = 0;
	  }
	}
	
	
	if(runsublist[1] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS + currlnth[1];
	  data_SUBLIST_SPU[datastorepos] = data[1][0];
	  currlnth[1] += 1;
	}
	
	mfc_write_tag_mask(1<<(12));
	mfc_read_tag_status_all();
	if(runsublist[2] == 1) {
	  data[2][0] = dataDMA[2][(data[2][0]%4)];
	  if(data[2][0] >= 0) {
	    sublist[2].succ = data[2][0];
	    dma_list[2][0] = sizeof(int);
	    dma_list[2][1] = cb.addrDATA + (data[2][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[2],(unsigned int)&dma_list[2][0],((sizeof(int))*2),12,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[2] = 0;
	  }
	  
	}
	
	if(runsublist[2] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*2 + currlnth[2];
	  data_SUBLIST_SPU[datastorepos] = data[2][0];
	  currlnth[2] += 1;
	}
	
	mfc_write_tag_mask(1<<(13));
	mfc_read_tag_status_all();
	if(runsublist[3] == 1) {
	  data[3][0] = dataDMA[3][(data[3][0]%4)];
	  if(data[3][0] >= 0) {
	    sublist[3].succ = data[3][0];
	    dma_list[3][0] = sizeof(int);
	    dma_list[3][1] = cb.addrDATA + (data[3][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[3],(unsigned int)&dma_list[3][0],((sizeof(int))*2),13,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[3] = 0;
	  }
	}
	
	
	if(runsublist[3] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*3 + currlnth[3];
	  data_SUBLIST_SPU[datastorepos] = data[3][0];
	  currlnth[3] += 1;
	}
	///
	mfc_write_tag_mask(1<<(14));
	mfc_read_tag_status_all();
	if(runsublist[4] == 1) {
	  data[4][0] = dataDMA[4][(data[4][0]%4)];
	  if(data[4][0] >= 0) {
	    sublist[4].succ = data[4][0];
	    dma_list[4][0] = sizeof(int);
	    dma_list[4][1] = cb.addrDATA + (data[4][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[4],(unsigned int)&dma_list[4][0],((sizeof(int))*2),14,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[4] = 0;
	  }
	}
	
	if(runsublist[4] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*4 + currlnth[4];
	  data_SUBLIST_SPU[datastorepos] = data[4][0];
	  currlnth[4] += 1;
	}
	
	mfc_write_tag_mask(1<<(15));
	mfc_read_tag_status_all();
	if(runsublist[5] == 1) {
	  data[5][0] = dataDMA[5][(data[5][0]%4)];
	  if(data[5][0] >= 0) {
	    sublist[5].succ = data[5][0];
	    dma_list[5][0] = sizeof(int);
	    dma_list[5][1] = cb.addrDATA + (data[5][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[5],(unsigned int)&dma_list[5][0],((sizeof(int))*2),15,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[5] = 0;
	  }
	}
	
	
	if(runsublist[5] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*5 + currlnth[5];
	  data_SUBLIST_SPU[datastorepos] = data[5][0];
	  currlnth[5] += 1;
	}
	
	mfc_write_tag_mask(1<<(16));
	mfc_read_tag_status_all();
	if(runsublist[6] == 1) {
	  data[6][0] = dataDMA[6][(data[6][0]%4)];
	  if(data[6][0] >= 0) {
	    sublist[6].succ = data[6][0];
	    dma_list[6][0] = sizeof(int);
	    dma_list[6][1] = cb.addrDATA + (data[6][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[6],(unsigned int)&dma_list[6][0],((sizeof(int))*2),16,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[6] = 0;
	  }
	}
	
	if(runsublist[6] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*6 + currlnth[6];
	  data_SUBLIST_SPU[datastorepos] = data[6][0];
	  currlnth[6] += 1;
	}
	
	mfc_write_tag_mask(1<<(17));
	mfc_read_tag_status_all();
	if(runsublist[7] == 1) {
	  data[7][0] = dataDMA[7][(data[7][0]%4)];
	  if(data[7][0] >= 0) {
	    sublist[7].succ = data[7][0];
	    dma_list[7][0] = sizeof(int);
	    dma_list[7][1] = cb.addrDATA + (data[7][0])*sizeof(int);
	    spu_mfcdma32(dataDMA[7],(unsigned int)&dma_list[7][0],((sizeof(int))*2),17,MFC_GETL_CMD);
	  }
	  else {
	    runsublist[7] = 0;
	  }
	}
	
	if(runsublist[7] == 1) {
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*7 + currlnth[7];
	  data_SUBLIST_SPU[datastorepos] = data[7][0];
	  currlnth[7] += 1;
	}
      }
      
      if(currlnth[0]) {
	mfc_write_tag_mask(1<<(10));
	mfc_read_tag_status_all();
	if(runsublist[0] == 1)
	  data[0][0] = dataDMA[0][(data[0][0]%4)];
	if(runsublist[0] == 1 && data[0][0] >= 0) {
	  sublist[0].succ = data[0][0];
	  datastorepos = currlnth[0];
	  data_SUBLIST_SPU[datastorepos] = data[0][0];
	  currlnth[0] += 1;
	}
	else runsublist[0] = 0;
	
	dma_list[0][0] = (currlnth[0]*sizeof(int));
	if((currlnth[0]%16 != 0)) 
	  dma_list[0][0] = (1+dma_list[0][0]/16)*16;
	dma_list[0][1] = (cb.addrSUBSTOREDATA[0] + sizeof(int)*sublist[0].lnth);
	sublist[0].lnth += currlnth[0];
	currlnth[0] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[0],0,dma_list[0],(sizeof(unsigned int))*2,10,0,0);
      }
      
      if(currlnth[1]) {
	mfc_write_tag_mask(1<<(11));
	mfc_read_tag_status_all();
	if(runsublist[1] == 1)
	  data[1][0] = dataDMA[1][(data[1][0]%4)];
	if(runsublist[1] == 1 && data[1][0] >= 0) {
	  sublist[1].succ = data[1][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS + currlnth[1];
	  data_SUBLIST_SPU[datastorepos] = data[1][0];
	  currlnth[1] += 1;
	}
	else runsublist[1] = 0;
	
	dma_list[1][0] = (currlnth[1]*sizeof(int));
	if((currlnth[1]%16 != 0)) 
	  dma_list[1][0] = (1+dma_list[1][0]/16)*16;
	dma_list[1][1] = (cb.addrSUBSTOREDATA[1] + sizeof(int)*sublist[1].lnth);
	sublist[1].lnth += currlnth[1];
	currlnth[1] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS],0,dma_list[1],(sizeof(unsigned int))*2,11,0,0);
      }
      
      if(currlnth[2]) {
	mfc_write_tag_mask(1<<(12));
	mfc_read_tag_status_all();
	if(runsublist[2] == 1)
	  data[2][0] = dataDMA[2][(data[2][0]%4)];
	if(runsublist[2] == 1 && data[2][0] >= 0) {
	  sublist[2].succ = data[2][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*2 + currlnth[2];
	  data_SUBLIST_SPU[datastorepos] = data[2][0];
	  currlnth[2] += 1;
	}
	else runsublist[2] = 0;
	
	dma_list[2][0] = (currlnth[2]*sizeof(int));
	if((currlnth[2]%16 != 0)) 
	  dma_list[2][0] = (1+dma_list[2][0]/16)*16;
	dma_list[2][1] = (cb.addrSUBSTOREDATA[2] + sizeof(int)*sublist[2].lnth);
	sublist[2].lnth += currlnth[2];
	currlnth[2] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*2],0,dma_list[2],(sizeof(unsigned int))*2,12,0,0);
      }	
      
      
      if(currlnth[3]) {
	mfc_write_tag_mask(1<<(13));
	mfc_read_tag_status_all();
	if(runsublist[3] == 1)
	  data[3][0] = dataDMA[3][(data[3][0]%4)];
	if(runsublist[3] == 1 && data[3][0] >= 0) {
	  sublist[3].succ = data[3][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*3 + currlnth[3];
	  data_SUBLIST_SPU[datastorepos] = data[3][0];
	  currlnth[3] += 1;
	}
	else runsublist[3] = 0;
	
	dma_list[3][0] = (currlnth[3]*sizeof(int));
	if((currlnth[3]%16 != 0)) 
	  dma_list[3][0] = (1+dma_list[3][0]/16)*16;
	dma_list[3][1] = (cb.addrSUBSTOREDATA[3] + sizeof(int)*sublist[3].lnth);
	sublist[3].lnth += currlnth[3];
	currlnth[3] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*3],0,dma_list[3],(sizeof(unsigned int))*2,13,0,0);
      }
      
      if(currlnth[4]) {
	mfc_write_tag_mask(1<<(14));
	mfc_read_tag_status_all();
	if(runsublist[4] == 1) 
	  data[4][0] = dataDMA[4][(data[4][0]%4)];
	if(runsublist[4] == 1 && data[4][0] >= 0) {
	  sublist[4].succ = data[4][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*4 + currlnth[4];
	  data_SUBLIST_SPU[datastorepos] = data[4][0];
	  currlnth[4] += 1;
	}
	else runsublist[4] = 0;
	
	dma_list[4][0] = (currlnth[4]*sizeof(int));
	if((currlnth[4]%16 != 0)) 
	  dma_list[4][0] = (1+dma_list[4][0]/16)*16;
	dma_list[4][1] = (cb.addrSUBSTOREDATA[4] + sizeof(int)*sublist[4].lnth);
	sublist[4].lnth += currlnth[4];
	currlnth[4] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*4],0,dma_list[4],(sizeof(unsigned int))*2,14,0,0);
      }
      
      if(currlnth[5]) {
	mfc_write_tag_mask(1<<(15));
	mfc_read_tag_status_all();
	if(runsublist[5] == 1)
	  data[5][0] = dataDMA[5][(data[5][0]%4)];
	if(runsublist[5] == 1 && data[5][0] >= 0) {
	  sublist[5].succ = data[5][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*5 + currlnth[5];
	  data_SUBLIST_SPU[datastorepos] = data[5][0];
	  currlnth[5] += 1;
	}
	else runsublist[5] = 0;
	
	dma_list[5][0] = (currlnth[5]*sizeof(int));
	if((currlnth[5]%16 != 0)) 
	  dma_list[5][0] = (1+dma_list[5][0]/16)*16;
	dma_list[5][1] = (cb.addrSUBSTOREDATA[5] + sizeof(int)*sublist[5].lnth);
	sublist[5].lnth += currlnth[5];
	currlnth[5] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*5],0,dma_list[5],(sizeof(unsigned int))*2,15,0,0);
      }
      
      if(currlnth[6]) {
	mfc_write_tag_mask(1<<(16));
	mfc_read_tag_status_all();
	if(runsublist[6] == 1) 
	  data[6][0] = dataDMA[6][(data[6][0]%4)];
	if(runsublist[6] == 1 && data[6][0] >= 0) {
	  sublist[6].succ = data[6][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*6 + currlnth[6];
	  data_SUBLIST_SPU[datastorepos] = data[6][0];
	  currlnth[6] += 1;
	}
	else runsublist[6] = 0;
	
	dma_list[6][0] = (currlnth[6]*sizeof(int));
	if((currlnth[6]%16 != 0)) 
	  dma_list[6][0] = (1+dma_list[6][0]/16)*16;
	dma_list[6][1] = (cb.addrSUBSTOREDATA[6] + sizeof(int)*sublist[6].lnth);
	sublist[6].lnth += currlnth[6];
	currlnth[6] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*6],0,dma_list[6],(sizeof(unsigned int))*2,16,0,0);
      }
      
      if(currlnth[7]) {
	mfc_write_tag_mask(1<<(17));
	mfc_read_tag_status_all();
	if(runsublist[7] == 1)
	  data[7][0] = dataDMA[7][(data[7][0]%4)];
	if(runsublist[7] == 1 && data[7][0] >= 0) {
	  sublist[7].succ = data[7][0];
	  datastorepos = ELEMENTS_PER_SUBLIST_IN_LS*7 + currlnth[7];
	  data_SUBLIST_SPU[datastorepos] = data[7][0];
	  currlnth[7] += 1;
	}
	else runsublist[7] = 0;
	
	dma_list[7][0] = (currlnth[7]*sizeof(int));
	if((currlnth[7]%16 != 0)) 
	  dma_list[7][0] = (1+dma_list[7][0]/16)*16;
	dma_list[7][1] = (cb.addrSUBSTOREDATA[7] + sizeof(int)*sublist[7].lnth);
	sublist[7].lnth += currlnth[7];
	currlnth[7] = 0;
	
	mfc_putl(&data_SUBLIST_SPU[ELEMENTS_PER_SUBLIST_IN_LS*7],0,dma_list[7],(sizeof(unsigned int))*2,17,0,0);
      }
      //loop back for by fetching the successor of the last element
      //wait for the last cycle to complete
      mfc_write_tag_mask(1<<(10));
      mfc_read_tag_status_all();
      if(runsublist[0] == 1) {
	dma_list[0][0] = sizeof(unsigned int);
	dma_list[0][1] = cb.addrDATA + data[0][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[0],(unsigned int)&dma_list[0][0],((sizeof(int))*2),10,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(11));
      mfc_read_tag_status_all();
      if(runsublist[1] == 1) {
	dma_list[1][0] = sizeof(unsigned int);
	dma_list[1][1] = cb.addrDATA + data[1][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[1],(unsigned int)&dma_list[1][0],((sizeof(int))*2),11,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(12));
      mfc_read_tag_status_all();
      if(runsublist[2] == 1) {
	dma_list[2][0] = sizeof(unsigned int);
	dma_list[2][1] = cb.addrDATA + data[2][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[2],(unsigned int)&dma_list[2][0],((sizeof(int))*2),12,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(13));
      mfc_read_tag_status_all();
      if(runsublist[3] == 1) {
	dma_list[3][0] = sizeof(unsigned int);
	dma_list[3][1] = cb.addrDATA + data[3][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[3],(unsigned int)&dma_list[3][0],((sizeof(int))*2),13,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(14));
      mfc_read_tag_status_all();
      if(runsublist[4] == 1) {
	dma_list[4][0] = sizeof(unsigned int);
	dma_list[4][1] = cb.addrDATA + data[4][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[4],(unsigned int)&dma_list[4][0],((sizeof(int))*2),14,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(15));
      mfc_read_tag_status_all();
      if(runsublist[5] == 1) {
	dma_list[5][0] = sizeof(unsigned int);
	dma_list[5][1] = cb.addrDATA + data[5][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[5],(unsigned int)&dma_list[5][0],((sizeof(int))*2),15,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(16));
      mfc_read_tag_status_all();
      if(runsublist[6] == 1) {
	dma_list[6][0] = sizeof(unsigned int);
	dma_list[6][1] = cb.addrDATA + data[6][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[6],(unsigned int)&dma_list[6][0],((sizeof(int))*2),16,MFC_GETL_CMD);
      }
      
      mfc_write_tag_mask(1<<(17));
      mfc_read_tag_status_all();
      if(runsublist[7] == 1) {
	dma_list[7][0] = sizeof(unsigned int);
	dma_list[7][1] = cb.addrDATA + data[7][0]*sizeof(unsigned int);
	spu_mfcdma32(dataDMA[7],(unsigned int)&dma_list[7][0],((sizeof(int))*2),17,MFC_GETL_CMD);
      }
      
      flag = runsublist[0] + runsublist[1] + runsublist[2] + runsublist[3] + runsublist[4] + runsublist[5] + runsublist[6] + runsublist[7];
    }
#ifdef PROFILING
  }
#endif

  mfc_write_tag_mask(1<<(10));
  mfc_read_tag_status_all();
  mfc_put(sublist, cb.addrSUBSTART, 8*4*sizeof(unsigned int), 10, 0, 0);
  mfc_write_tag_mask(1<<(10));
  mfc_read_tag_status_all();
  
  //  tstop = spu_read_decrementer();
  //  fprintf(stdout,"Decrementer Cycle Counter pow16 iter50 sub64 buf8 : %u \n",tstart-tstop);

  return 0;
}
