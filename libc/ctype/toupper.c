#include <ctype.h>

// FIXME: add locale support
int toupper(int c) {
	if (c >= 'a' && c <= 'z') {
		// subtracting 32 gives uppercase char in ASCII
		c = c - 32;
	}
	return c;
}
