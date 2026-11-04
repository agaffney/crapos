#include <stdlib.h>

#include "harness.h"

/* itoa() must emit "0" for zero, not an empty string. */
TEST(itoa_zero)
{
	char buf[50];
	itoa(0, buf, 10);
	ASSERT_EQ_STR(buf, "0");
}

/* itoa() converts multi-digit positive numbers in base 10. */
TEST(itoa_positive_decimal)
{
	char buf[50];
	itoa(12345, buf, 10);
	ASSERT_EQ_STR(buf, "12345");
}

/* itoa() in base 10 prefixes negative numbers with '-'. */
TEST(itoa_negative)
{
	char buf[50];
	itoa(-42, buf, 10);
	ASSERT_EQ_STR(buf, "-42");
}

/* The largest int converts without overflowing the digit loop. */
TEST(itoa_int_max)
{
	char buf[50];
	itoa(2147483647, buf, 10);
	ASSERT_EQ_STR(buf, "2147483647");
}

/*
 * Known bug: itoa() negates the input as a signed int (num = -input), which
 * is undefined behavior for INT_MIN. With sanitizers on, UBSan aborts the
 * test. Without them the wraparound happens to give the right answer, so
 * the test would report XPASS; it skips itself instead (make SAN=0).
 */
XFAIL_TEST(itoa_int_min)
{
	char buf[50];
	if (!TEST_SANITIZERS) {
		SKIP_TEST("the bug (signed overflow) is only detectable with sanitizers");
	}
	itoa(-2147483647 - 1, buf, 10);
	ASSERT_EQ_STR(buf, "-2147483648");
}

/* itoa() in base 2 produces binary digits, and "0" for zero. */
TEST(itoa_binary)
{
	char buf[50];
	itoa(5, buf, 2);
	ASSERT_EQ_STR(buf, "101");
	itoa(0, buf, 2);
	ASSERT_EQ_STR(buf, "0");
}

/* itoa() in base 8 produces octal digits. */
TEST(itoa_octal)
{
	char buf[50];
	itoa(8, buf, 8);
	ASSERT_EQ_STR(buf, "10");
}

/* itoa() in base 16 uses lowercase digits and no "0x" prefix. */
TEST(itoa_hex)
{
	char buf[50];
	itoa(255, buf, 16);
	ASSERT_EQ_STR(buf, "ff");
}

/* Outside base 10, a negative input is treated as its unsigned bit pattern. */
TEST(itoa_negative_in_hex_is_unsigned)
{
	char buf[50];
	itoa(-1, buf, 16);
	ASSERT_EQ_STR(buf, "ffffffff");
}

/* The result is NUL-terminated even when the buffer held a longer string. */
TEST(itoa_terminates_output)
{
	char buf[50] = "xxxxxxxxxxxxxxxx";
	itoa(7, buf, 10);
	ASSERT_EQ_STR(buf, "7");
}
