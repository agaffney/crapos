#include <string.h>

char *strcpy(char *dest, const char *src) {
	size_t i;

	for (i = 0; ; i++) {
		dest[i] = src[i];
		if (dest[i] == '\0') {
			break;
		}
	}

	return dest;
}

char *strncpy(char *dest, const char *src, size_t n) {
	size_t i;

	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	// Zero out the rest of the buffer up to specified size
	for ( ; i < n; i++) {
		dest[i] = '\0';
	}

	return dest;
}
