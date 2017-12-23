#ifndef _STDIO_H
#define _STDIO_H

#include "sys/cdefs.h"

#define EOF (-1)

int sprintf(char *, const char * __restrict, ...);
int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

#include <stdarg.h>

int vsprintf(char *, const char* restrict, va_list);

#endif
