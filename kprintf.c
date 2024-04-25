//
// formatted console output -- printf, panic.
//

#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"


static char digits[] = "0123456789abcdef";

/* 将不定长参数的整形参数 xx 解析并输出 */
static void printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  uint x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do
  {
    buf[i++] = digits[x % base];
  } while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    console_wChar(buf[i]);
}

/* 将不定长参数的字符串参数 x 解析并输出 */
static void printptr(uint64 x)
{
  int i;
  console_wChar('0');
  console_wChar('x');
  for (i = 0; i < (sizeof(uint64) * 2); i++, x <<= 4)
    console_wChar(digits[x >> (sizeof(uint64) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void kprintf (char *fmt, ...)
{
  va_list ap;
  int i, c;
  char *s;


  if (fmt == 0)
    kError(errParameterFormat);

  va_start(ap, fmt);
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++)
  {
    /* 不需要解析的数据则直接输出 */
    if(c != '%')
    {
      console_wChar(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c)
    {
    case 'd': /* 处理 %d */
      printint(va_arg(ap, int), 10, 1);
      break;
    case 'x': /* 处理 %x */
      printint(va_arg(ap, int), 16, 1);
      break;
    case 'p': /* 处理 %p */
      printptr(va_arg(ap, uint64));
      break;
    case 's': /* 处理 %s */
      if((s = va_arg(ap, char*)) == 0)
        s = "(null)";
      for(; *s; s++)
        console_wChar(*s);
      break;
    case '%': /* 处理 %% */
      console_wChar('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      console_wChar('%');
      console_wChar(c);
      break;
    }
  }
  va_end(ap);
}

