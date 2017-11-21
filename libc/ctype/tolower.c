#include <ctype.h>

// FIXME: add locale support
int tolower(int c) {
	if (c >= 'A' && c <= 'Z') {
		// adding 32 gives lowercase char in ASCII
		c = c + 32;
	}
	return c;
}
