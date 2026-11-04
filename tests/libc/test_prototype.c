/* Just enough tests to prove the build approach works end to end. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"

/* itoa() must emit "0" for zero, not an empty string. */
TEST(itoa_zero)
{
	char buf[50];
	itoa(0, buf, 10);
	ASSERT_EQ_STR(buf, "0");
}

/* itoa() in base 10 prefixes negative numbers with '-'. */
TEST(itoa_negative)
{
	char buf[50];
	itoa(-42, buf, 10);
	ASSERT_EQ_STR(buf, "-42");
}

/* itoa() in base 16 uses lowercase digits and no "0x" prefix. */
TEST(itoa_hex)
{
	char buf[50];
	itoa(255, buf, 16);
	ASSERT_EQ_STR(buf, "ff");
}

/* strlen() counts characters before the terminator, including for "". */
TEST(strlen_basic)
{
	ASSERT_EQ_INT(strlen("hello"), 5);
	ASSERT_EQ_INT(strlen(""), 0);
}

/* sprintf() handles %d and %s together and returns the number of characters written. */
TEST(sprintf_int_and_string)
{
	char buf[50];
	int n = sprintf(buf, "%d-%s", 42, "x");
	ASSERT_EQ_STR(buf, "42-x");
	ASSERT_EQ_INT(n, 4);
}

/*
 * Known bug: after emitting '%' for "%%", vsprintf doesn't advance past the
 * second '%', so it is parsed as the start of another conversion. Here "%%d"
 * consumes the argument as an int and prints "%7" instead of "%d".
 */
XFAIL_TEST(sprintf_percent_percent)
{
	char buf[50];
	sprintf(buf, "%%d", 7);
	ASSERT_EQ_STR(buf, "%d");
}
