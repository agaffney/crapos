/*
 * Stand-ins for kernel facilities that the code under test calls. Built like
 * the libc sources (-nostdinc, rename.h force-included).
 */
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include <core/kprint.h>
#include "stubs.h"

char stub_kprint_output[4096];
static size_t stub_kprint_len;

void stub_kprint_reset(void)
{
	stub_kprint_len = 0;
	stub_kprint_output[0] = '\0';
}

/* Capture output instead of writing to the console. */
void kprint(const char *restrict format, ...)
{
	va_list parameters;
	va_start(parameters, format);
	stub_kprint_len += vsprintf(stub_kprint_output + stub_kprint_len, format, parameters);
	va_end(parameters);
}
