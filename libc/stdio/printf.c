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
			// %% gives '%'
			if (format[0] == '%') {
				out[written] = format[0];
				written++;
				continue;
			}
			int alt_form = 0, left_just = 0, prepend_sign = 0;
			char pad_char = '\0';
			int field_width = 0;
			// Look for flags
			int break_out = 0;
			while(format[0] != '\0') {
				switch(format[0]) {
					case '0':
						pad_char = '0';
						break;
					case ' ':
						pad_char = ' ';
						break;
					case '#':
						alt_form = 1;
						break;
					case '-':
						left_just = 1;
						break;
					case '+':
						prepend_sign = 1;
						break;
					default:
						break_out = 1;
						break;
				}
				if (break_out) {
					break;
				}
				format++;
			}
			// Look for field-width
			while (format[0] >= '0' && format[0] <= '9') {
				// Multiply by 10 for each additional digit
				if (field_width > 0) {
					field_width = field_width * 10;
				}
				field_width += format[0] - '0';
				format++;
			}
			if (format[0] == 'c') {
				char c = (char) va_arg(parameters, int /* char promotes to int */);
				out[written] = c;
				format++;
				written++;
			} else if (format[0] == 's') {
				const char* str = va_arg(parameters, const char*);
				size_t len = strlen(str);
				if (pad_char) {
					for (i = len; i < field_width; i++) {
						out[written++] = pad_char;
					}
				}
				for (i = 0; i < len; i++) {
					out[written + i] = str[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'd' || format[0] == 'i') {
				signed int val = va_arg(parameters, signed int);
				itoa(val, itoa_buf, 10);
				size_t len = strlen(itoa_buf);
				for (i = 0; i < len; i++) {
					out[written + i] = itoa_buf[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'u') {
				unsigned int val = va_arg(parameters, unsigned int);
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
				if (alt_form) {
					out[written++] = '0';
					out[written++] = 'x';
				}
				if (pad_char) {
					for (i = len; i < field_width; i++) {
						out[written++] = pad_char;
					}
				}
				for (i = 0; i < len; i++) {
					out[written + i] = ( format[0] == 'x' ? tolower(itoa_buf[i]) : toupper(itoa_buf[i]) );
				}
				format++;
				written += len;
			// Non-standard format specifier for binary output
			} else if (format[0] == 'b') {
				unsigned int val = va_arg(parameters, unsigned int);
				itoa(val, itoa_buf, 2);
				size_t len = strlen(itoa_buf);
				if (pad_char) {
					for (i = len; i < field_width; i++) {
						out[written++] = pad_char;
					}
				}
				for (i = 0; i < len; i++) {
					out[written + i] = itoa_buf[i];
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
