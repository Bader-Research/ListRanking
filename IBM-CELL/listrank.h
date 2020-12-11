/* --------------------------------------------------------------  */
/* (C)Copyright 2001,2006,                                         */
/* International Business Machines Corporation,                    */
/* Sony Computer Entertainment, Incorporated,                      */
/* Toshiba Corporation,                                            */
/*                                                                 */
/* All Rights Reserved.                                            */
/* --------------------------------------------------------------  */
/* PROLOG END TAG zYx                                              */
#ifndef __listrank_h__
#define __listrank_h__

#include <stdlib.h> 

/* This union helps clarify calling parameters between the PPE and the SPE. */

/***** No of Sublists *****/
#define NO_SUBLISTS 64

#define LOOPOVER 100

/*No of SPUs */
#define NO_SPU 8

/* Overflow Size of the Sublist (x) where Total Allocated size for sublists is = 2n + nX*/
#define X 3

typedef union
{
  unsigned long long ull;
  unsigned int ui[2];
}
addr64;


#endif /* __listrank_h__ */
