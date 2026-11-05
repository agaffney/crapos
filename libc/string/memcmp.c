#include <string.h>

int memcmp(const void* v1, const void* v2, size_t len) {
	const unsigned char *a = v1, *b = v2;
	size_t i;
	for (i = 0; i < len; i++) {
		if (a[i] != b[i]) {
			return a[i] - b[i];
		}
	}
	return 0;
}
