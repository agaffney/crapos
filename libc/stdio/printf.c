#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sprintf(char *out, const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int ret = vsprintf(out, format, parameters);

	va_end(parameters);
	return ret;
}

int vsprintf(char *out, const char* restrict format, va_list parameters) {
	int written = 0;
	char itoa_buf[50];
	int i;

	while (*format != '\0') {
		if (format[0] == '%') {
			format++;
			if (format[0] == '%') {
				out[written] = format[0];
				written++;
			} else if (format[0] == 'c') {
				char c = (char) va_arg(parameters, int /* char promotes to int */);
				out[written] = c;
				format++;
				written++;
			} else if (format[0] == 's') {
				const char* str = va_arg(parameters, const char*);
				size_t len = strlen(str);
				int i;
				for (i = 0; i < len; i++) {
					out[written + i] = str[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'd' || format[0] == 'i') {
				signed int val  = va_arg(parameters, signed int);
				itoa(val, itoa_buf, 10);
				size_t len = strlen(itoa_buf);
				for (i = 0; i < len; i++) {
					out[written + i] = itoa_buf[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'u') {
				unsigned int val  = va_arg(parameters, unsigned int);
				itoa(val, itoa_buf, 10);
				size_t len = strlen(itoa_buf);
				for (i = 0; i < len; i++) {
					out[written + i] = itoa_buf[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'x' || format[0] == 'X') {
				unsigned int val = va_arg(parameters, unsigned int);
				itoa(val, itoa_buf, 16);
				size_t len = strlen(itoa_buf);
				for (i = 0; i < len; i++) {
					out[written + i] = ( format[0] == 'x' ? tolower(itoa_buf[i]) : toupper(itoa_buf[i]) );
				}
				format++;
				written += len;
			}
		} else {
			out[written] = format[0];
			format++;
			written++;
		}
	}

	out[written] = 0;
	return written;
}
