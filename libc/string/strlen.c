#include <string.h>

size_t strlen(const char *str) {
	size_t len;
	while(str[len]) {
		len++;
	}
	return len;
}
