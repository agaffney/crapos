#include <ctype.h>
#include <stdio.h>
#include <core/kprint.h>
// TODO: replace with generic serial/console interface
#include <arch/x86/serial.h>

char kprint_buf[2048];

void kprint(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	vsprintf(kprint_buf, format, parameters);

	va_end(parameters);

	unsigned int i = 0;
	while (kprint_buf[i] != '\0') {
		write_serial(kprint_buf[i]);
		i++;
	}
}

