#include "user/user.h"

#include <stdarg.h>

static char digits[] = "0123456789ABCDEF";

static void putc(int fd, char c) {
  write(fd, &c, 1);
}

static void printnum(int fd, long xx, int base, int sign) {
  char buf[64]; // We can print up to 64 bit numbers in binary
  long i;
  uint64 x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do { buf[i++] = digits[x % base]; } while ((x /= base) != 0);

  if (sign) buf[i++] = '-';

  while (--i >= 0) putc(fd, buf[i]);
}

static void printptr(int fd, uint64 x) {
  int i;
  putc(fd, '0');
  putc(fd, 'x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    putc(fd, digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the given fd.
// Supports:
// %l, %lu, %lx, %lb, %d, %b, %x, %p, %c, %s, %u
void vprintf(int fd, const char *fmt, va_list ap) {
  char *s;
  int c, i;
  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    // If the character is not '%', just print it
    // Characters following '%' are handled later
    if (c != '%') {
      putc(fd, c);
      continue;
    }
    // Get the next char in the format string. Only called if previous char was '%'
    c = fmt[++i] & 0xff;

    switch (c) {
    // First: The easy cases
    case 'd': // Print int in decimal
      printnum(fd, va_arg(ap, int), 10, 1);
      break;
    case 'u': // Print unsigned int decimal
      printnum(fd, va_arg(ap, unsigned int), 10, 0);
      break;
    case 'l':              // Print long in decimal
      c = fmt[++i] & 0xff; // Look at the next char. Decrement later if it's a format char
      switch (c) {
      case 'u': // Print unsigned long
        printnum(fd, va_arg(ap, unsigned long), 10, 0);
        break;
      case 'x': // Print hexadecimal long
        printnum(fd, va_arg(ap, long), 16, 0);
        break;
      case 'b': // Print binary long
        printnum(fd, va_arg(ap, long), 2, 0);
        break;
      default: // Print long
        printnum(fd, va_arg(ap, long), 10, 1);
        i--; // Not a format char. Decrement i so it's printed properly
        break;
      }
      break;
    case 'x': // Print int in hex
      printnum(fd, va_arg(ap, int), 16, 0);
      break;
    case 'b': // Print int in binary
      printnum(fd, va_arg(ap, int), 2, 0);
      break;
    case 'p': // Print a pointer
      printptr(fd, va_arg(ap, uint64));
      break;
    case 'c': // Print a char
      putc(fd, va_arg(ap, uint));
      break;
    case '%': //Print literal '%'
      putc(fd, c);
      break;
    // And now the complicated ones
    case 's': // Print a string
      s = va_arg(ap, char *);
      if (s == 0) s = "(null)";
      while (*s != 0) {
        putc(fd, *s);
        s++;
      }
      break;

    default: // Print %<sequence> for unknown sequences
      putc(fd, '%');
      putc(fd, c);
      break;
    }
  }
}

void fprintf(int fd, const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vprintf(fd, fmt, ap);
}

void printf(const char *fmt, ...) {
  va_list ap;

  va_start(ap, fmt);
  vprintf(1, fmt, ap);
}
