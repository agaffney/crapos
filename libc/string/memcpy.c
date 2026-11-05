#include <string.h>

void* memcpy(void* __restrict dest, const void* __restrict src, size_t len) {
	size_t i;
	for (i = 0; i < len; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return dest;
}
