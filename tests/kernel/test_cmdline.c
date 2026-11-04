#include <string.h>

#include <core/cmdline.h>
#include "harness.h"
#include "stubs.h"

/* Defined in kernel/core/cmdline.c but not exposed in its header. */
extern char cmdline[];

/* cmdline_init() stores a short command line unchanged. */
TEST(cmdline_init_copies_input)
{
	cmdline_init("foo=bar baz=123 test");
	ASSERT_EQ_STR(cmdline, "foo=bar baz=123 test");
}

/* An over-long command line is cut at KERNEL_CMDLINE_MAX_LEN and stays terminated. */
TEST(cmdline_init_truncates_to_max_len)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 50];
	size_t i;
	/* libc doesn't define memset() yet, so fill by hand */
	for (i = 0; i < sizeof(input) - 1; i++) {
		input[i] = 'a';
	}
	input[sizeof(input) - 1] = '\0';

	cmdline_init(input);

	ASSERT_EQ_INT(strlen(cmdline), KERNEL_CMDLINE_MAX_LEN);
}

/* cmdline_parse() prints the stored command line via kprint(). */
TEST(cmdline_parse_prints_the_command_line)
{
	stub_kprint_reset();
	cmdline_init("quiet root=/dev/sda1");
	cmdline_parse();
	ASSERT_EQ_STR(stub_kprint_output, "Command line: quiet root=/dev/sda1\n");
}
