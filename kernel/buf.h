/*! \file buf.h
 * \brief fs cache structure
 */

#ifndef INCLUDED_kernel_buf_h
#define INCLUDED_kernel_buf_h

#ifdef __cplusplus
extern "C" {
#endif


struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  uchar data[BSIZE];
};




#ifdef __cplusplus
}
#endif

#endif
