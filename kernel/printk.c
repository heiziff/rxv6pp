//
// formatted kernel logging -- printk, panic.
//

#include <stdarg.h>

#include "defs.h"

volatile int panicked = 0;

// lock to avoid interleaving concurrent printk's.
static struct {
  struct spinlock lock;
  int locking;
} pr;

static char digits[] = "0123456789abcdef";

static void
printnum(long xx, int base, int sign)
{
  char buf[64]; // We can print up to 64 bit numbers in binary
  long i;
  uint64 x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64 x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

void
setoutputpriority(char level)
{
  const char* levelColorMap[] = {
    [LOGLEVEL_EMERG]    "\e[38;5;196m",
    [LOGLEVEL_WARNING]  "\e[38;5;172m",
    [LOGLEVEL_NOTICE]   "\e[38;5;11m",
    [LOGLEVEL_INFO]     "\e[38;5;240m",
    [LOGLEVEL_DEFAULT]  "\e[0m",
  };
  
  int colorIndex = ( KERN_EMERG[0] <= level && level <= KERN_DEFAULT[0]) ? level - 0x30 : LOGLEVEL_DEFAULT;

  char c;
  for (int i = 0; (c = levelColorMap[colorIndex][i] & 0xff) != 0; i++) {
    consputc(c);
  }

}

// Print to the console. only understands %d, %x, %p, %s.
void
printk(char *fmt, ...)
{
  va_list ap;
  int i, c, locking;
  char *s;

  locking = pr.locking;
  if(locking)
    acquire(&pr.lock);

  if (fmt == 0) 
    panic("null fmt");

  setoutputpriority(fmt[0]);

  va_start(ap, fmt);
  for(i = 1; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printnum(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      printnum(va_arg(ap, int), 16, 1);
      break;
    case 'p':
      printptr(va_arg(ap, uint64));
      break;
    case 's':
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }
  va_end(ap);

  setoutputpriority(LOGLEVEL_DEFAULT);

  if(locking)
    release(&pr.lock);
}

void
panic(char *s)
{
  pr.locking = 0;
  pr_emerg("panic: ");
  pr_emerg("%s", s);
  pr_emerg("\n");
  panicked = 1; // freeze uart output from other CPUs
  timerhalt();
  for(;;)
    ;
}

void
printkinit(void)
{
  initlock(&pr.lock, "pr");
  pr.locking = 1;
}
