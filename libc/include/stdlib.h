#ifndef _STDLIB_H
#define _STDLIB_H

#include "sys/cdefs.h"

__attribute__((__noreturn__))
void abort(void);

void itoa(int, char *, int);

#endif
