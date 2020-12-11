# Parallel List Ranking

## SIMPLE

This SIMPLE/SMP module (written by David R. Helman and Joseph JáJá ,
and later modified by David A. Bader) implements a randomized,
parallel code for link ranking.

Helman and JáJá introduce a new optimal prefix computation algorithm
on linked lists which builds upon the sparse ruling set approach of
Reid-Miller and Blelloch. Besides being somewhat simpler and requiring
nearly half the number of memory accesses, and can bound our
complexity with high probability instead of merely on
average. Moreover, whereas Reid-Miller and Blelloch targeted their
algorithm for implementation on a vector multiprocessor architecture,
they develop an algorithm for implementation on the symmetric
multiprocessor architecture (SMP). These symmetric multiprocessors
dominate the high-end server market and are currently the primary
candidate for constructing large scale multiprocessor systems. The
authors ran this code using a variety of benchmarks which they
identified to examine the dependence of the algorithm on memory access
patterns. For some problems, their algorithm actually matched or
exceeded the optimal sequential solution using only a single
thread. Moreover, in spite of the fact that the processors must
compete for access to main memory, their algorithm still resulted in
scalable performance up to 16 processors, which was the largest
platform available to them.

References:

D. R. Helman and J. JáJá , "Prefix Computations on Symmetric Multiprocessors," Journal of Parallel and Distributed Computing, 61(2):265--278, 2001.

## SMP

Irregular problems such as those from graph theory pose serious
challenges for parallel machines due to non-contiguous accesses to
global data structures with low degrees of locality. These parallel
codes perform list ranking on two types of shared-memory computers:
symmetric multiprocessors (SMP) and multithreaded architectures (MTA)
such as the Cray MTA-2. Previous studies show that for SMPs
performance is primarily a function of non-contiguous memory accesses,
whereas for the MTA, it is primarily a function of the number of
concurrent operations.

References:

D.A. Bader, G. Cong, and J. Feo, "On the Architectural Requirements
for Efficient Execution of Graph Algorithms," The 33rd International
Conference on Parallel Processing (ICPP 2005), pp. 547-556, Georg
Sverdrups House, University of Oslo, Norway, June 14-17, 2005.

## IBM-CELL

Parallel List Ranking for the Sony-Toshiba-IBM Cell Broadband Engine Processor

## CRAY-MTA

Parallel List Ranking for the Cray Multithreaded Architecture (MTA/XMT)

