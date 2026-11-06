#include <stdio.h>
#include <string.h>

#include <core/cmdline.h>
#include "harness.h"
#include "stubs.h"

/* Defined in kernel/core/cmdline.c but not exposed in its header. */
extern char _cmdline[];
extern char _cmdline_args[][KERNEL_CMDLINE_MAX_ARG_LEN + 1];
extern int _cmdline_args_len;

/* Fill the parsed-argument table directly, so lookups can be tested without cmdline_parse(). */
static void set_args(const char *const *args, int count)
{
	int i;
	memset(_cmdline_args, 0, KERNEL_CMDLINE_MAX_ARGS * (KERNEL_CMDLINE_MAX_ARG_LEN + 1));
	for (i = 0; i < count; i++) {
		strcpy(_cmdline_args[i], args[i]);
	}
	_cmdline_args_len = count;
}

/* A copy of 'text' in a full-size, zero-padded buffer, like a _cmdline_args entry. */
static void make_arg(char *dest, const char *text)
{
	memset(dest, 0, KERNEL_CMDLINE_MAX_ARG_LEN + 1);
	strcpy(dest, text);
}

/* cmdline_init() stores a short command line unchanged. */
TEST(cmdline_init_copies_input)
{
	cmdline_init("foo=bar baz=123 test");
	ASSERT_EQ_STR(_cmdline, "foo=bar baz=123 test");
}

/*
 * A NULL command line (the bootloader provided none) is accepted: a warning is
 * printed and the command line stays empty instead of crashing.
 */
TEST(cmdline_init_null_prints_warning_and_leaves_command_line_empty)
{
	stub_kprint_reset();
	cmdline_init(NULL);
	ASSERT_EQ_STR(_cmdline, "");
	ASSERT_EQ_STR(stub_kprint_output, "cmdline: empty kernel commandline received\n");
}

/* A NULL init discards a command line that was set earlier, rather than keeping it. */
TEST(cmdline_init_null_clears_previous_command_line)
{
	cmdline_init("quiet debug");
	cmdline_init(NULL);
	ASSERT_EQ_STR(_cmdline, "");
}

/* Parsing after a NULL init finds no arguments. */
TEST(cmdline_parse_after_null_init_gives_no_args)
{
	cmdline_init(NULL);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 0);
}

/* An over-long command line is cut at KERNEL_CMDLINE_MAX_LEN and stays terminated. */
TEST(cmdline_init_truncates_to_max_len)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 50];
	memset(input, 'a', sizeof(input)-1);
	input[sizeof(input) - 1] = '\0';

	cmdline_init(input);

	ASSERT_EQ_INT(strlen(_cmdline), KERNEL_CMDLINE_MAX_LEN);
}

/* cmdline_parse() prints the stored command line via kprint(). */
TEST(cmdline_parse_prints_the_command_line)
{
	stub_kprint_reset();
	cmdline_init("quiet root=/dev/sda1");
	cmdline_parse();
	ASSERT_EQ_STR(stub_kprint_output, "Command line: quiet root=/dev/sda1\n");
}

/* An empty command line parses to no arguments. */
TEST(cmdline_parse_empty_gives_no_args)
{
	cmdline_init("");
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 0);
}

/* Parsing again starts from scratch instead of adding to the previous result. */
TEST(cmdline_parse_resets_arg_count)
{
	_cmdline_args_len = 5;
	cmdline_init("");
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 0);
}

/* Space-separated arguments are stored in order and counted. */
TEST(cmdline_parse_stores_space_separated_args)
{
	cmdline_init("foo=bar baz=123 ");
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 2);
	ASSERT_EQ_STR(_cmdline_args[0], "foo=bar");
	ASSERT_EQ_STR(_cmdline_args[1], "baz=123");
}

/* The last argument is stored even when no separator follows it. */
TEST(cmdline_parse_stores_last_arg_without_trailing_separator)
{
	cmdline_init("quiet debug");
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 2);
	ASSERT_EQ_STR(_cmdline_args[1], "debug");
}

/*
 * Runs of separators must not produce empty arguments. Also guards against the
 * terminator padding after the command line being mistaken for a final
 * argument once the last argument is stored.
 */
TEST(cmdline_parse_collapses_repeated_separators)
{
	cmdline_init("a   b ");
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 2);
	ASSERT_EQ_STR(_cmdline_args[0], "a");
	ASSERT_EQ_STR(_cmdline_args[1], "b");
}

/* An over-long argument is cut at KERNEL_CMDLINE_MAX_ARG_LEN. */
TEST(cmdline_parse_truncates_long_arg)
{
	char input[KERNEL_CMDLINE_MAX_ARG_LEN + 20];
	memset(input, 'a', KERNEL_CMDLINE_MAX_ARG_LEN + 10);
	strcpy(input + KERNEL_CMDLINE_MAX_ARG_LEN + 10, " b ");
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 2);
	ASSERT_EQ_INT(strlen(_cmdline_args[0]), KERNEL_CMDLINE_MAX_ARG_LEN);
	ASSERT_EQ_STR(_cmdline_args[1], "b");
}

/* A command line of nothing but separators, filling the buffer, has no arguments. */
TEST(cmdline_parse_only_separators_gives_no_args)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 1];
	memset(input, KERNEL_CMDLINE_ARG_SEP, KERNEL_CMDLINE_MAX_LEN);
	input[KERNEL_CMDLINE_MAX_LEN] = '\0';
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 0);
}

/* A command line that fills the whole buffer and ends in a separator keeps its last argument. */
TEST(cmdline_parse_full_buffer_ending_in_separator)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 1];
	memset(input, KERNEL_CMDLINE_ARG_SEP, KERNEL_CMDLINE_MAX_LEN);
	input[KERNEL_CMDLINE_MAX_LEN] = '\0';
	memcpy(input, "a b", 3);
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, 2);
	ASSERT_EQ_STR(_cmdline_args[1], "b");
}

/* Arguments beyond KERNEL_CMDLINE_MAX_ARGS are dropped instead of overflowing the table. */
TEST(cmdline_parse_caps_arg_count)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 1];
	size_t i;
	for (i = 0; i + 1 < KERNEL_CMDLINE_MAX_LEN; i += 2) {
		input[i] = 'x';
		input[i + 1] = ' ';
	}
	input[i] = '\0';
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, KERNEL_CMDLINE_MAX_ARGS);
}

/* Build "x x x ..." with 'count' single-character arguments and no trailing space. */
static void make_n_args(char *dest, int count)
{
	int i;
	for (i = 0; i < count; i++) {
		dest[2 * i] = 'x';
		dest[2 * i + 1] = ' ';
	}
	dest[2 * count - 1] = '\0';
}

/* Exactly KERNEL_CMDLINE_MAX_ARGS arguments all fit, including an unterminated last one. */
TEST(cmdline_parse_exactly_max_args)
{
	char input[2 * KERNEL_CMDLINE_MAX_ARGS + 1];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS);
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, KERNEL_CMDLINE_MAX_ARGS);
	ASSERT_EQ_STR(_cmdline_args[KERNEL_CMDLINE_MAX_ARGS - 1], "x");
}

/* One argument more than the maximum is dropped, leaving exactly the maximum. */
TEST(cmdline_parse_one_over_max_args)
{
	char input[2 * (KERNEL_CMDLINE_MAX_ARGS + 1) + 1];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS + 1);
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_INT(_cmdline_args_len, KERNEL_CMDLINE_MAX_ARGS);
}

#define WARN_TOO_MANY "cmdline: dropping kernel args after 64\n"
#define WARN_TOO_LONG "cmdline: truncating kernel arg longer than 128\n"

/* Parse 'input' and compare everything kprint()ed: the echo of the line, then 'warnings'. */
static void assert_parse_output(const char *input, const char *warnings)
{
	char expected[1024];
	sprintf(expected, "Command line: %s\n%s", input, warnings);
	stub_kprint_reset();
	cmdline_init(input);
	cmdline_parse();
	ASSERT_EQ_STR(stub_kprint_output, expected);
}

/* Exactly KERNEL_CMDLINE_MAX_ARGS arguments fit, so there is nothing to warn about. */
TEST(cmdline_parse_exactly_max_args_prints_no_warning)
{
	char input[2 * KERNEL_CMDLINE_MAX_ARGS + 1];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS);
	assert_parse_output(input, "");
}

/* A 65th argument is dropped with one warning. */
TEST(cmdline_parse_one_over_max_args_prints_warning)
{
	char input[2 * (KERNEL_CMDLINE_MAX_ARGS + 1) + 1];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS + 1);
	assert_parse_output(input, WARN_TOO_MANY);
}

/*
 * Text left over after the table is full is dropped with one warning, and the
 * 64th argument is not overwritten. "y" is still pending when the string ends,
 * so this reaches the bounds check in the final flush.
 */
TEST(cmdline_parse_leftover_after_full_table_is_dropped)
{
	char input[160];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS - 1);
	strcpy(input + strlen(input), " z y");
	assert_parse_output(input, WARN_TOO_MANY);
	ASSERT_EQ_INT(_cmdline_args_len, KERNEL_CMDLINE_MAX_ARGS);
	ASSERT_EQ_STR(_cmdline_args[KERNEL_CMDLINE_MAX_ARGS - 2], "x");
	ASSERT_EQ_STR(_cmdline_args[KERNEL_CMDLINE_MAX_ARGS - 1], "z");
}

/* However many arguments are dropped, the "too many" warning is printed once. */
TEST(cmdline_parse_many_over_max_args_prints_one_warning)
{
	char input[KERNEL_CMDLINE_MAX_LEN + 1];
	make_n_args(input, 120);
	assert_parse_output(input, WARN_TOO_MANY);
}

/* Spaces after the 64th argument are not a dropped argument, so they add no warning. */
TEST(cmdline_parse_max_args_then_trailing_spaces_prints_no_warning)
{
	char input[2 * KERNEL_CMDLINE_MAX_ARGS + 4];
	make_n_args(input, KERNEL_CMDLINE_MAX_ARGS);
	strcpy(input + strlen(input), "  ");
	assert_parse_output(input, "");
}

/* An argument of exactly the maximum length is kept whole, with no warning. */
TEST(cmdline_parse_exactly_max_arg_len_prints_no_warning)
{
	char input[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	memset(input, 'a', KERNEL_CMDLINE_MAX_ARG_LEN);
	input[KERNEL_CMDLINE_MAX_ARG_LEN] = '\0';
	assert_parse_output(input, "");
}

/* An argument one character over the maximum length is truncated with one warning. */
TEST(cmdline_parse_one_over_max_arg_len_prints_warning)
{
	char input[KERNEL_CMDLINE_MAX_ARG_LEN + 2];
	memset(input, 'a', KERNEL_CMDLINE_MAX_ARG_LEN + 1);
	input[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = '\0';
	assert_parse_output(input, WARN_TOO_LONG);
}

/* However far past the limit an argument runs, its "too long" warning is printed once. */
TEST(cmdline_parse_long_arg_prints_one_warning)
{
	char input[KERNEL_CMDLINE_MAX_ARG_LEN + 11];
	memset(input, 'a', KERNEL_CMDLINE_MAX_ARG_LEN + 10);
	input[KERNEL_CMDLINE_MAX_ARG_LEN + 10] = '\0';
	assert_parse_output(input, WARN_TOO_LONG);
}

/* A short argument after an over-long one is kept whole and adds no warning. */
TEST(cmdline_parse_short_arg_after_long_arg_prints_no_extra_warning)
{
	char input[KERNEL_CMDLINE_MAX_ARG_LEN + 10];
	memset(input, 'a', KERNEL_CMDLINE_MAX_ARG_LEN + 2);
	strcpy(input + KERNEL_CMDLINE_MAX_ARG_LEN + 2, " ok");
	assert_parse_output(input, WARN_TOO_LONG);
	ASSERT_EQ_STR(_cmdline_args[1], "ok");
}

/* _cmdline_split_arg() separates "name=value" at the first '='. */
TEST(cmdline_split_arg_name_and_value)
{
	char arg[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	make_arg(arg, "root=/dev/sda1");
	_cmdline_split_arg(arg, name, value);
	ASSERT_EQ_STR(name, "root");
	ASSERT_EQ_STR(value, "/dev/sda1");
}

/* Only the first '=' splits; later ones belong to the value. */
TEST(cmdline_split_arg_value_keeps_later_equals)
{
	char arg[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	make_arg(arg, "opt=a=b");
	_cmdline_split_arg(arg, name, value);
	ASSERT_EQ_STR(name, "opt");
	ASSERT_EQ_STR(value, "a=b");
}

/* "name=" has an empty value. */
TEST(cmdline_split_arg_empty_value)
{
	char arg[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	make_arg(arg, "empty=");
	_cmdline_split_arg(arg, name, value);
	ASSERT_EQ_STR(name, "empty");
	ASSERT_EQ_STR(value, "");
}

/* An argument without '=' is all name, with an empty value. */
TEST(cmdline_split_arg_flag_without_value)
{
	char arg[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	make_arg(arg, "quiet");
	_cmdline_split_arg(arg, name, value);
	ASSERT_EQ_STR(name, "quiet");
	ASSERT_EQ_STR(value, "");
}

/* A name without '=' that is shorter than a full table entry is not read past its end. */
TEST(cmdline_split_arg_stops_at_terminator)
{
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	if (!TEST_SANITIZERS) {
		SKIP_TEST("reading past the end of the string is only caught by a sanitizer");
	}
	_cmdline_split_arg("quiet", name, value);
	ASSERT_EQ_STR(name, "quiet");
}

/* A "name=value" string shorter than a full table entry is not read past its end. */
TEST(cmdline_split_arg_value_stops_at_terminator)
{
	char name[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	char value[KERNEL_CMDLINE_MAX_ARG_LEN + 1] = {0};
	if (!TEST_SANITIZERS) {
		SKIP_TEST("reading past the end of the string is only caught by a sanitizer");
	}
	_cmdline_split_arg("foo=bar", name, value);
	ASSERT_EQ_STR(name, "foo");
	ASSERT_EQ_STR(value, "bar");
}

/* cmdline_has_arg() finds both "name=value" and bare-flag arguments by name. */
TEST(cmdline_has_arg_finds_args)
{
	const char *args[] = { "foo=bar", "baz=123", "test" };
	set_args(args, 3);
	ASSERT_EQ_INT(cmdline_has_arg("foo"), 1);
	ASSERT_EQ_INT(cmdline_has_arg("baz"), 1);
	ASSERT_EQ_INT(cmdline_has_arg("test"), 1);
}

/* An argument that isn't on the command line is not found. */
TEST(cmdline_has_arg_missing)
{
	const char *args[] = { "foo=bar", "test" };
	set_args(args, 2);
	ASSERT_EQ_INT(cmdline_has_arg("nope"), 0);
}

/* With no parsed arguments nothing is found. */
TEST(cmdline_has_arg_no_args)
{
	set_args(0, 0);
	ASSERT_EQ_INT(cmdline_has_arg("foo"), 0);
}

/* The whole name must match: neither a prefix nor an extension of it counts. */
TEST(cmdline_has_arg_requires_exact_name)
{
	const char *args[] = { "foo=bar" };
	set_args(args, 1);
	ASSERT_EQ_INT(cmdline_has_arg("fo"), 0);
	ASSERT_EQ_INT(cmdline_has_arg("fooo"), 0);
	ASSERT_EQ_INT(cmdline_has_arg("foo=bar"), 0);
}

/* The value is not part of the name, so searching for it finds nothing. */
TEST(cmdline_has_arg_does_not_match_values)
{
	const char *args[] = { "foo=bar" };
	set_args(args, 1);
	ASSERT_EQ_INT(cmdline_has_arg("bar"), 0);
}

/* cmdline_arg() returns 1 for a found argument and 0, leaving dest alone, for a missing one. */
TEST(cmdline_arg_return_value)
{
	const char *args[] = { "foo=bar" };
	char out[16] = "unchanged";
	set_args(args, 1);
	ASSERT_EQ_INT(cmdline_arg("foo", out), 1);
	strcpy(out, "unchanged");
	ASSERT_EQ_INT(cmdline_arg("nope", out), 0);
	ASSERT_EQ_STR(out, "unchanged");
}

/*
 * cmdline_arg() copies the value, terminator included, to the caller's buffer.
 * The buffer starts out filled with 'x' to catch a missing terminator.
 */
TEST(cmdline_arg_copies_value)
{
	const char *args[] = { "foo=bar", "baz=123" };
	char out[16];
	set_args(args, 2);
	memset(out, 'x', sizeof(out) - 1);
	out[sizeof(out) - 1] = '\0';
	ASSERT_EQ_INT(cmdline_arg("foo", out), 1);
	ASSERT_EQ_STR(out, "bar");
	memset(out, 'x', sizeof(out) - 1);
	ASSERT_EQ_INT(cmdline_arg("baz", out), 1);
	ASSERT_EQ_STR(out, "123");
}

/* A bare flag gives an empty, terminated value. */
TEST(cmdline_arg_flag_gives_empty_value)
{
	const char *args[] = { "quiet" };
	char out[16];
	set_args(args, 1);
	memset(out, 'x', sizeof(out) - 1);
	out[sizeof(out) - 1] = '\0';
	ASSERT_EQ_INT(cmdline_arg("quiet", out), 1);
	ASSERT_EQ_STR(out, "");
}
