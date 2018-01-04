#include <core/cmdline.h>
#include <libk/string.h>
#include <core/kprint.h>

char cmdline[KERNEL_CMDLINE_MAX_LEN + 1];

void cmdline_init(const char * input) {
	strncpy(cmdline, input, KERNEL_CMDLINE_MAX_LEN);
	kprint("Command line: %s\n", cmdline);
}
