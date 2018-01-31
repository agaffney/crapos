#include <string.h>

void* memcpy(void* __restrict dest, const void* __restrict src, size_t len) {
	int i;
	for (i = 0; i < len; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return dest;
}
