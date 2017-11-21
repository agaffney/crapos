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
//	size_t max_remain;

	while (*format != '\0') {
//		max_remain = INT_MAX - written;

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
			} else if (format[0] == 'x') {
				unsigned int val = va_arg(parameters, unsigned int);
				char buf[50];
				itoa(val, buf, 16);
				size_t len = strlen(buf);
				int i;
				for (i = 0; i < len; i++) {
					out[written + i] = buf[i];
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
