#include <string.h>

void* memset(void* __restrict dest, int fill, size_t len) {
	unsigned char c = (unsigned char) fill;
	size_t i;
	for (i = 0; i < len; i++) {
		((unsigned char *)dest)[i] = c;
	}
	return dest;
}
