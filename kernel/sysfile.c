//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include "defs.h"
#include "fcntl.h"

// Fetch the nth word-sized system call argument as a file descriptor
// and return both the descriptor and the corresponding struct file.
static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd;
  struct file *f;

  argint(n, &fd);
  if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(pf)
    *pf = f;
  return 0;
}

// Allocate a file descriptor for the given file.
// Takes over file reference from caller on success.
static int
fdalloc(struct file *f)
{
  int fd;
  struct proc *p = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

uint64
sys_dup(void)
{
  struct file *f;
  int fd;

  if(argfd(0, 0, &f) < 0)
    return -1;
  if((fd=fdalloc(f)) < 0)
    return -1;
  filedup(f);
  return fd;
}

uint64
sys_read(void)
{
  struct file *f;
  int n;
  uint64 p;

  argaddr(1, &p);
  argint(2, &n);
  if(argfd(0, 0, &f) < 0)
    return -1;
  return fileread(f, p, n);
}

uint64
sys_write(void)
{
  struct file *f;
  int n;
  uint64 p;
  
  argaddr(1, &p);
  argint(2, &n);
  if(argfd(0, 0, &f) < 0)
    return -1;

  return filewrite(f, p, n);
}

uint64
sys_close(void)
{
  int fd;
  struct file *f;

  if(argfd(0, &fd, &f) < 0)
    return -1;
  myproc()->ofile[fd] = 0;
  fileclose(f);
  return 0;
}

uint64
sys_fstat(void)
{
  struct file *f;
  uint64 st; // user pointer to struct stat

  argaddr(1, &st);
  if(argfd(0, 0, &f) < 0)
    return -1;
  return filestat(f, st);
}

// Create the path new as a link to the same inode as old.
uint64
sys_link(void)
{
  char name[DIRSIZ], new[MAXPATH], old[MAXPATH];
  struct inode *dp, *ip;

  if(argstr(0, old, MAXPATH) < 0 || argstr(1, new, MAXPATH) < 0)
    return -1;

  begin_op();
  if((ip = namei(old)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  if(ip->type == T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->nlink++;
  iupdate(ip);
  iunlock(ip);

  if((dp = nameiparent(new, name)) == 0)
    goto bad;
  ilock(dp);
  if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
    iunlockput(dp);
    goto bad;
  }
  iunlockput(dp);
  iput(ip);

  end_op();

  return 0;

bad:
  ilock(ip);
  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);
  end_op();
  return -1;
}

// Is the directory dp empty except for "." and ".." ?
static int
isdirempty(struct inode *dp)
{
  int off;
  struct dirent de;

  for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
    if(readi(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
      panic("isdirempty: readi");
    if(de.inum != 0)
      return 0;
  }
  return 1;
}

uint64
sys_unlink(void)
{
  struct inode *ip, *dp;
  struct dirent de;
  char name[DIRSIZ], path[MAXPATH];
  uint off;

  if(argstr(0, path, MAXPATH) < 0)
    return -1;

  begin_op();
  if((dp = nameiparent(path, name)) == 0){
    end_op();
    return -1;
  }

  ilock(dp);

  // Cannot unlink "." or "..".
  if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
    goto bad;

  if((ip = dirlookup(dp, name, &off)) == 0)
    goto bad;
  ilock(ip);

  if(ip->nlink < 1)
    panic("unlink: nlink < 1");
  if(ip->type == T_DIR && !isdirempty(ip)){
    iunlockput(ip);
    goto bad;
  }

  memset(&de, 0, sizeof(de));
  if(writei(dp, 0, (uint64)&de, off, sizeof(de)) != sizeof(de))
    panic("unlink: writei");
  if(ip->type == T_DIR){
    dp->nlink--;
    iupdate(dp);
  }
  iunlockput(dp);

  ip->nlink--;
  iupdate(ip);
  iunlockput(ip);

  end_op();

  return 0;

bad:
  iunlockput(dp);
  end_op();
  return -1;
}

static struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;

  ilock(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && (ip->type == T_FILE || ip->type == T_DEVICE))
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0){
    iunlockput(dp);
    return 0;
  }

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      goto fail;
  }

  if(dirlink(dp, name, ip->inum) < 0)
    goto fail;

  if(type == T_DIR){
    // now that success is guaranteed:
    dp->nlink++;  // for ".."
    iupdate(dp);
  }

  iunlockput(dp);

  return ip;

 fail:
  // something went wrong. de-allocate ip.
  ip->nlink = 0;
  iupdate(ip);
  iunlockput(ip);
  iunlockput(dp);
  return 0;
}

uint64
sys_open(void)
{
  char path[MAXPATH];
  int fd, omode;
  struct file *f;
  struct inode *ip;
  int n;

  argint(1, &omode);
  if((n = argstr(0, path, MAXPATH)) < 0)
    return -1;

  begin_op();

  if(omode & O_CREATE){
    ip = create(path, T_FILE, 0, 0);
    if(ip == 0){
      end_op();
      return -1;
    }
  } else {
    if((ip = namei(path)) == 0){
      end_op();
      return -1;
    }
    ilock(ip);
    if(ip->type == T_DIR && omode != O_RDONLY){
      iunlockput(ip);
      end_op();
      return -1;
    }
  }

  if(ip->type == T_DEVICE && (ip->major < 0 || ip->major >= NDEV)){
    iunlockput(ip);
    end_op();
    return -1;
  }

  if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
    if(f)
      fileclose(f);
    iunlockput(ip);
    end_op();
    return -1;
  }

  if(ip->type == T_DEVICE){
    f->type = FD_DEVICE;
    f->major = ip->major;
  } else {
    f->type = FD_INODE;
    f->off = 0;
  }
  f->ip = ip;
  f->readable = !(omode & O_WRONLY);
  f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

  if((omode & O_TRUNC) && ip->type == T_FILE){
    itrunc(ip);
  }

  iunlock(ip);
  end_op();

  return fd;
}

uint64
sys_mkdir(void)
{
  char path[MAXPATH];
  struct inode *ip;

  begin_op();
  if(argstr(0, path, MAXPATH) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_mknod(void)
{
  struct inode *ip;
  char path[MAXPATH];
  int major, minor;

  begin_op();
  argint(1, &major);
  argint(2, &minor);
  if((argstr(0, path, MAXPATH)) < 0 ||
     (ip = create(path, T_DEVICE, major, minor)) == 0){
    end_op();
    return -1;
  }
  iunlockput(ip);
  end_op();
  return 0;
}

uint64
sys_chdir(void)
{
  char path[MAXPATH];
  struct inode *ip;
  struct proc *p = myproc();
  
  begin_op();
  if(argstr(0, path, MAXPATH) < 0 || (ip = namei(path)) == 0){
    end_op();
    return -1;
  }
  ilock(ip);
  if(ip->type != T_DIR){
    iunlockput(ip);
    end_op();
    return -1;
  }
  iunlock(ip);
  iput(p->cwd);
  end_op();
  p->cwd = ip;
  return 0;
}

uint64
sys_exec(void)
{
  char path[MAXPATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;

  argaddr(1, &uargv);
  if(argstr(0, path, MAXPATH) < 0) {
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = exec(path, argv);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_pipe(void)
{
  uint64 fdarray; // user pointer to array of two integers
  struct file *rf, *wf;
  int fd0, fd1;
  struct proc *p = myproc();

  argaddr(0, &fdarray);
  if(pipealloc(&rf, &wf) < 0)
    return -1;
  fd0 = -1;
  if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
    if(fd0 >= 0)
      p->ofile[fd0] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  if(copyout(p->pagetable, fdarray, (char*)&fd0, sizeof(fd0)) < 0 ||
     copyout(p->pagetable, fdarray+sizeof(fd0), (char *)&fd1, sizeof(fd1)) < 0){
    p->ofile[fd0] = 0;
    p->ofile[fd1] = 0;
    fileclose(rf);
    fileclose(wf);
    return -1;
  }
  return 0;
}

#define MMAP_VA_BEGIN ((uint64) 1 << 30)
#define MMAP_VA_SIZE  ((uint64) 1 << 34)
#define MMAP_VA_END   (MAXVA)
// #define MMAP_VA_END   (MMAP_VA_BEGIN + MMAP_VA_SIZE)

// #define MMAP_VA_BITMAP_CAP (MMAP_VA_SIZE / PGSIZE)
// static uint8 free_va_bitmap[MMAP_VA_BITMAP_CAP / 8] = { 0 };

uint64
mmap_find_space(pagetable_t pagetable, uint64 begin, int n_pages)
{
  if (begin <= MMAP_VA_BEGIN || begin >= (MMAP_VA_END - n_pages * PGSIZE))
    begin = MMAP_VA_BEGIN;

  for (;begin < MMAP_VA_END; begin += PGSIZE) {
    int found = 1;
    for (int j = 0; j < n_pages; j++) {
      uint64 addr = walkaddr(pagetable, begin + (j * PGSIZE));
      if (addr) {
        found = 0;
        break;
      }
    }
    if (found) return begin;
  }

  panic("mmap_find_space no free pages left");
  return -1;

}

uint64
sys_mmap(void)
{
  uint64 va;
  int size, n_pages, prot, flags;

  struct proc *p = myproc();

  argaddr(0, &va);
  argint(1, &size);
  argint(2, &prot);
  argint(3, &flags);

  n_pages = (size + PGSIZE - 1) / PGSIZE;
  va = PGROUNDDOWN(va);

  // POSIX says, so I follow
  if (!(flags & MAP_PRIVATE)) return MAP_FAILED;
  if (n_pages <= 0) return MAP_FAILED;
  if (!(flags & MAP_ANON)) return MAP_FAILED;
  // if (va % PGSIZE != 0) return MAP_FAILED;

  int perm = 0;
  perm |= PTE_U;
  if (prot & PROT_READ) perm |= PTE_R;
  if (prot & PROT_WRITE) perm |= PTE_W;
  if (prot & PROT_EXEC) perm |= PTE_X;

  // we do not allow mmap to map fixed allocations into our buddy allocator
  if ((flags & MAP_FIXED || flags & MAP_FIXED_NOREPLACE) && (va < MMAP_VA_BEGIN || va >= MMAP_VA_END - n_pages)) {
    return MAP_FAILED;
  }

  // TODO: maybe do all this with a process lock, to prevent whacky stuff
  // i.e. when passing MAP_FIXED_NOREPLACE

  // address hint points into buddy allocator region, search for another addr
  if (!(flags & MAP_FIXED || flags & MAP_FIXED_NOREPLACE)) {
    va = mmap_find_space(p->pagetable, va, n_pages);
    if (va == -1) return MAP_FAILED;
  }

  // checks for MAP_FIXED:
  //    fail if there is a non user accessible page inside the requested region
  // checks for MAP_FIXED_NOREPLACE:
  //    fail if there is ANY already existing mapping in the requested region
  if (flags & MAP_FIXED || flags & MAP_FIXED_NOREPLACE) {
    for (int i = 0; i < n_pages; i++) {
      if (walkaddr(p->pagetable, va + i*PGSIZE)) {
        if (flags & MAP_FIXED_NOREPLACE)
          return MAP_FAILED;
        
        if (flags & MAP_FIXED) {
          pte_t *pte = walk(p->pagetable, va + i*PGSIZE, 0);
          if (!(*pte & PTE_U))
            return MAP_FAILED;
        }
      }
    }
  }

  // we now have a valid va to work with, allocate physical memory and 
  // map va's to it
  for (int i = 0; i < n_pages; i++) {
    // va not mapped
    if (walkaddr(p->pagetable, va + i*PGSIZE) == 0) {
      uint64 pa = (uint64) kalloc();
      if (mappages(p->pagetable, va + i*PGSIZE, PGSIZE, pa, perm) < 0) {
        panic("mmap");
      }
    }
    // va already mapped and MAP_FIXED => update flags
    else if (flags & MAP_FIXED) {
      pte_t *pte = walk(p->pagetable, va + i*PGSIZE, 0);
      *pte = *pte & ~0x3FF;
      *pte |= perm | PTE_V;
    }
    // va already mapped and no MAP_FIXED => what is going on
    else {
      panic("mmap unexpected remap");
    }

    if (!(flags & MAP_UNINITIALIZED)) {
      void* pa = (void*) walkaddr(p->pagetable, va + i*PGSIZE);
      if (pa == 0) panic("mmap wrong mapping");
      memset(pa, 0, PGSIZE);
    }
  }

  // add the new mapping to proc struct
  taken_list *entry = p->mmaped_pages;
  while (entry->used) {
    entry++;
    if ((uint64)entry % PGSIZE == 0) {
      if ((entry - 1)->next == 0) {
        (entry - 1)->next = kalloc();
        memset((entry - 1)->next, 0, PGSIZE);
      }
      
      entry = (entry - 1)->next;
    }
  }

  entry->va = va;
  entry->n_pages = n_pages;
  entry->used = 1;

  return va;
}

int
sys_munmap(void)
{
  uint64 addr;
  int size, n_pages;

  argaddr(0, &addr);
  argint(1, &size);

  if (size % PGSIZE != 0) return -1;
  n_pages = size / PGSIZE;
  if (addr % PGSIZE != 0) return -1;
  if (addr < MMAP_VA_BEGIN || addr >= MMAP_VA_END - n_pages * PGSIZE) return -1;

  struct proc *p = myproc();

  for (int i = 0; i < n_pages; i++) {
    uint64 va = addr + i * PGSIZE;
    if (walkaddr(p->pagetable, va)) {
      pte_t *pte = walk(p->pagetable, va, 0);
      if (*pte & PTE_U)
        uvmunmap(p->pagetable, va, 1, 1);
    }
  }

  return 0;
}
