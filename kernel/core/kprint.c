#include <libk/ctype.h>
#include <libk/stdio.h>
#include <core/kprint.h>
#include <core/console.h>

/*
This was originally a static buffer to allow kprint() to work before there was
a functional kmalloc(). I'm leaving it this way for now in case I need to debug
the memory/page allocator again.
*/
char kprint_buf[2048];

void kprint(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int len = vsprintf(kprint_buf, format, parameters);

	va_end(parameters);

	FILE_write(CONSOLE, kprint_buf, len);
}
