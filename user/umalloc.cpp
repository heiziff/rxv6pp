#include "user/user.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

#ifdef __cplusplus
extern "C" {
#endif

union fl_header {
  struct {
    union fl_header *ptr;
    uint size;
  } s;
};

typedef union fl_header Fl_Header;

static Fl_Header base;
static Fl_Header *freep;

static Fl_Header*
fl_morecore(uint nu)
{
  char *p;
  Fl_Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Fl_Header));
  if(p == (char*)-1)
    return 0;
  hp = (Fl_Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

void*
fl_malloc(uint nbytes)
{
  Fl_Header *p, *prevp;
  uint nunits;

  nunits = (nbytes + sizeof(Fl_Header) - 1)/sizeof(Fl_Header) + 1;
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      if(p->s.size == nunits)
        prevp->s.ptr = p->s.ptr;
      else {
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    if(p == freep)
      if((p = fl_morecore(nunits)) == 0)
        return 0;
  }
}

void
fl_free(void *ap)
{
  Fl_Header *bp, *p;

  bp = (Fl_Header*)ap - 1;
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
  freep = p;
}

#define BLOCK_SIZE 32
#define BLOCK_AMOUNT (1 << 21)
#define BUF_CAP BLOCK_AMOUNT * BLOCK_SIZE

typedef struct pl_header {
  struct pl_header *next;
} Pl_Header;

static char *pl_buf;
static size_t pl_buf_size;
static Pl_Header *freeList;

void
pl_init()
{
  pl_buf = sbrk(BUF_CAP);
  pl_buf_size = 0;

  Pl_Header *current = (Pl_Header*) pl_buf;
  freeList = current;

  for (size_t i = 0; i < BLOCK_AMOUNT - 1; ++i) {
    uint8 *adr = (uint8*) current;
    adr += BLOCK_SIZE;

    current->next = (Pl_Header*) adr;
    current = current->next;
  }

  current->next = 0;
}

void*
pl_malloc(uint nbytes)
{
  if (freeList == 0)
  {
    pl_init();
  }

  if (freeList == 0)
  {
    printf("malloc impl broken pls help\n");
    return 0;
  }

  if (nbytes > BLOCK_SIZE)
  {
    printf("[ERROR | malloc] pool allocation requested with invalid size\n");
    return 0;
  }

  if (freeList->next == 0)
  {
    printf("[ERROR | malloc] pool allocator capacity exceeded, help\n");
    return 0;
  }

  Pl_Header *head = freeList;
  freeList = freeList->next;
  return (void*) head;
}

void
pl_free(void *ap)
{
  Pl_Header *block = (Pl_Header*) ap;
  block->next = freeList;
  freeList = block;
}

void*
malloc(uint nbytes)
{
  if (nbytes <= BLOCK_SIZE)
  {
    return pl_malloc(nbytes);
  }
  else
  {
    return fl_malloc(nbytes);
  }
}

void
free(void *ap)
{
  if (pl_buf <= ap && ap < pl_buf + BUF_CAP)
  {
    pl_free(ap);
  }
  else
  {
    fl_free(ap);
  }
}

#ifdef __cplusplus
}
#endif
