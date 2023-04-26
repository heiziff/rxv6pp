#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void
putc(int fd, char c)
{
  write(fd, &c, 1);
}

static void
printnum(int fd, long xx, int base, int sign)
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
    putc(fd, buf[i]);
}

static void
printptr(int fd, uint64 x) {
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd. Only understands %d, $l, %x, %b, %p, %s, %u, %ul, %bs
void
vprintf(int fd, const char *fmt, va_list ap)
{
  char *s;
  int c, i, state;

  state = 0;
  for(i = 0; fmt[i]; i++){
    c = fmt[i] & 0xff;
    if(state == 0){
      if(c == '%'){
        state = '%';
      } else {
        putc(fd, c);
      }
    } else if(state == '%'){
      switch (c) {
        case 'd':
          printnum(fd, va_arg(ap, int), 10, 1);
          break;
        case 'l':
          printnum(fd, va_arg(ap, long), 10, 1);
          break;
        case 'u':
          state = 'u';
          break;
        case 'x':
          printnum(fd, va_arg(ap, int), 16, 0);
          break;
        case 'b':
          printnum(fd, va_arg(ap, uint64), 2, 0);
          break;
        case 'p':
          printptr(fd, va_arg(ap, uint64));
          break;
        case 's':
          s = va_arg(ap, char*);
          if(s == 0)
            s = "(null)";
          while(*s != 0){
            putc(fd, *s);
            s++;
          }
          break;
        case 'c':
          putc(fd, va_arg(ap, uint));
          break;
        case '%':
          putc(fd, c);
        default:
          putc(fd, '%');
          putc(fd, c);
      }
      state = state == '%' ? 0 : state;
    } else if (state == 'u') {
      switch(c) {
        case 'l':
          printnum(fd, va_arg(ap, long), 10, 0);
          break;
        default:
          printnum(fd, va_arg(ap, int), 10, 0);
          putc(fd, c);
          break;
      }
      state = 0;
    }
  }
}

void
fprintf(int fd, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void
printf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
