#include <ctype.h>
#include <stdio.h>
#include <string.h>

/*
 * Write the digits of 'value' in the given base (2-16, lowercase) to 'buf' as
 * a NUL-terminated string and return the number of digits. 'buf' must have
 * room for 33 characters. Signs are the caller's job. An unsupported base
 * produces an empty string.
 */
static size_t format_unsigned(char *buf, unsigned int value, unsigned int base) {
	char digits[32];
	size_t count = 0, i;

	// Base 0 would divide by zero and base 1 would never terminate
	if (base < 2 || base > 16) {
		buf[0] = '\0';
		return 0;
	}

	do {
		unsigned int digit = value % base;
		digits[count++] = digit > 9 ? (digit - 10 + 'a') : (digit + '0');
		value /= base;
	} while (value > 0);

	for (i = 0; i < count; i++) {
		buf[i] = digits[count - i - 1];
	}
	buf[count] = '\0';
	return count;
}

int sprintf(char *out, const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int ret = vsprintf(out, format, parameters);

	va_end(parameters);
	return ret;
}

int vsprintf(char *out, const char* restrict format, va_list parameters) {
	int written = 0;
	char num_buf[50];
	int i;

	while (*format != '\0') {
		if (format[0] == '%') {
			format++;
			// %% gives '%'
			if (format[0] == '%') {
				out[written] = format[0];
				written++;
				// Consume the second '%' so it isn't parsed as the start
				// of another conversion
				format++;
				continue;
			}
			int alt_form = 0; //, left_just = 0, prepend_sign = 0;
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
/*
					case '-':
						left_just = 1;
						break;
					case '+':
						prepend_sign = 1;
						break;
*/
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
				size_t len;
				if (val < 0) {
					num_buf[0] = '-';
					// Negate as unsigned, which is well defined for INT_MIN
					len = 1 + format_unsigned(num_buf + 1, 0u - (unsigned int)val, 10);
				} else {
					len = format_unsigned(num_buf, (unsigned int)val, 10);
				}
				for (i = 0; i < len; i++) {
					out[written + i] = num_buf[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'u') {
				unsigned int val = va_arg(parameters, unsigned int);
				size_t len = format_unsigned(num_buf, val, 10);
				for (i = 0; i < len; i++) {
					out[written + i] = num_buf[i];
				}
				format++;
				written += len;
			} else if (format[0] == 'x' || format[0] == 'X') {
				unsigned int val = va_arg(parameters, unsigned int);
				size_t len = format_unsigned(num_buf, val, 16);
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
					out[written + i] = ( format[0] == 'X' ? toupper(num_buf[i]) : num_buf[i] );
				}
				format++;
				written += len;
			// Non-standard format specifier for binary output
			} else if (format[0] == 'b') {
				unsigned int val = va_arg(parameters, unsigned int);
				size_t len = format_unsigned(num_buf, val, 2);
				if (pad_char) {
					for (i = len; i < field_width; i++) {
						out[written++] = pad_char;
					}
				}
				for (i = 0; i < len; i++) {
					out[written + i] = num_buf[i];
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
