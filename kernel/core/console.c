#include <core/console.h>
#include <core/kprint.h>

FILE * CONSOLE;

void console_set(FILE * f) {
	CONSOLE = f;
}
