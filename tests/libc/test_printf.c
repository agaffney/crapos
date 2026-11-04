#include <stdio.h>

#include "harness.h"

/* Text without any conversions is copied verbatim and counted. */
TEST(sprintf_literal_text)
{
	char buf[50];
	int n = sprintf(buf, "hello world");
	ASSERT_EQ_STR(buf, "hello world");
	ASSERT_EQ_INT(n, 11);
}

/* An empty format produces an empty string and returns 0. */
TEST(sprintf_empty_format)
{
	char buf[50] = "junk";
	int n = sprintf(buf, "");
	ASSERT_EQ_STR(buf, "");
	ASSERT_EQ_INT(n, 0);
}

/* %c writes a single character. */
TEST(sprintf_char)
{
	char buf[50];
	int n = sprintf(buf, "%c%c", 'o', 'k');
	ASSERT_EQ_STR(buf, "ok");
	ASSERT_EQ_INT(n, 2);
}

/* %s copies a string, and an empty string adds nothing. */
TEST(sprintf_string)
{
	char buf[50];
	sprintf(buf, "[%s]", "hello");
	ASSERT_EQ_STR(buf, "[hello]");
	sprintf(buf, "[%s]", "");
	ASSERT_EQ_STR(buf, "[]");
}

/* %s with the ' ' flag and a width right-justifies using spaces. */
TEST(sprintf_string_space_flag_width)
{
	char buf[50];
	sprintf(buf, "% 8s", "abc");
	ASSERT_EQ_STR(buf, "     abc");
}

/* %d prints zero, positive and negative values in decimal. */
TEST(sprintf_int_decimal)
{
	char buf[50];
	sprintf(buf, "%d", 0);
	ASSERT_EQ_STR(buf, "0");
	sprintf(buf, "%d", 42);
	ASSERT_EQ_STR(buf, "42");
	sprintf(buf, "%d", -42);
	ASSERT_EQ_STR(buf, "-42");
}

/* %i is a synonym for %d. */
TEST(sprintf_int_i_specifier)
{
	char buf[50];
	sprintf(buf, "%i", -7);
	ASSERT_EQ_STR(buf, "-7");
}

/* %u prints small unsigned values in decimal. */
TEST(sprintf_unsigned_small)
{
	char buf[50];
	sprintf(buf, "%u", 0u);
	ASSERT_EQ_STR(buf, "0");
	sprintf(buf, "%u", 42u);
	ASSERT_EQ_STR(buf, "42");
}

/* %x prints lowercase hex and %X uppercase, without a prefix. */
TEST(sprintf_hex_case)
{
	char buf[50];
	sprintf(buf, "%x", 255);
	ASSERT_EQ_STR(buf, "ff");
	sprintf(buf, "%X", 255);
	ASSERT_EQ_STR(buf, "FF");
	sprintf(buf, "%x", 0);
	ASSERT_EQ_STR(buf, "0");
}

/* The '#' flag adds a "0x" prefix to hex output. */
TEST(sprintf_hex_alt_form)
{
	char buf[50];
	sprintf(buf, "%#x", 255);
	ASSERT_EQ_STR(buf, "0xff");
}

/*
 * Known bug: with %X the '#' flag should give an uppercase "0X" prefix. The
 * "0x" prefix is currently hardcoded in lowercase.
 */
XFAIL_TEST(sprintf_hex_alt_form_uppercase)
{
	char buf[50];
	sprintf(buf, "%#X", 255);
	ASSERT_EQ_STR(buf, "0XFF");
}

/* Hex values with the top bit set print as unsigned, not negative. */
TEST(sprintf_hex_high_bit_set)
{
	char buf[50];
	sprintf(buf, "%x", 0xdeadbeefu);
	ASSERT_EQ_STR(buf, "deadbeef");
	sprintf(buf, "%x", 0xffffffffu);
	ASSERT_EQ_STR(buf, "ffffffff");
}

/* %X also prints values with the top bit set as unsigned, in uppercase. */
TEST(sprintf_hex_upper_high_bit_set)
{
	char buf[50];
	sprintf(buf, "%X", 0xdeadbeefu);
	ASSERT_EQ_STR(buf, "DEADBEEF");
	sprintf(buf, "%X", 0xffffffffu);
	ASSERT_EQ_STR(buf, "FFFFFFFF");
}

/* The '0' flag with a width zero-pads hex output on the left. */
TEST(sprintf_hex_zero_padded)
{
	char buf[50];
	sprintf(buf, "%08x", 255);
	ASSERT_EQ_STR(buf, "000000ff");
}

/* The ' ' flag with a width space-pads hex output on the left. */
TEST(sprintf_hex_space_padded)
{
	char buf[50];
	sprintf(buf, "% 4x", 255);
	ASSERT_EQ_STR(buf, "  ff");
}

/* %b (non-standard) prints binary, optionally zero-padded. */
TEST(sprintf_binary)
{
	char buf[50];
	sprintf(buf, "%b", 5);
	ASSERT_EQ_STR(buf, "101");
	sprintf(buf, "%08b", 5);
	ASSERT_EQ_STR(buf, "00000101");
}

/* Several conversions mix with literal text, and the return value counts all of it. */
TEST(sprintf_multiple_conversions_and_return_value)
{
	char buf[50];
	int n = sprintf(buf, "x=%d y=%d", 1, 2);
	ASSERT_EQ_STR(buf, "x=1 y=2");
	ASSERT_EQ_INT(n, 7);
}

/* "%%" at the end of the format yields one literal '%'. */
TEST(sprintf_percent_percent_at_end)
{
	char buf[50];
	int n = sprintf(buf, "50%%");
	ASSERT_EQ_STR(buf, "50%");
	ASSERT_EQ_INT(n, 3);
}

/*
 * "%%" is a literal '%' even when a conversion character follows: "%%d" is
 * the text "%d" and must not consume an argument.
 */
TEST(sprintf_percent_percent_before_conversion_char)
{
	char buf[50];
	sprintf(buf, "%%d", 7);
	ASSERT_EQ_STR(buf, "%d");
}

/* Consecutive "%%" pairs each produce one '%'. */
TEST(sprintf_percent_percent_repeated)
{
	char buf[50];
	int n = sprintf(buf, "%%%%");
	ASSERT_EQ_STR(buf, "%%");
	ASSERT_EQ_INT(n, 2);
}

/* "%%" followed by a real conversion: the escape doesn't swallow the argument. */
TEST(sprintf_percent_percent_then_conversion)
{
	char buf[50];
	sprintf(buf, "%%%d", 7);
	ASSERT_EQ_STR(buf, "%7");
}

/* A format that is just "%%" yields a single '%'. */
TEST(sprintf_percent_percent_alone)
{
	char buf[50];
	int n = sprintf(buf, "%%");
	ASSERT_EQ_STR(buf, "%");
	ASSERT_EQ_INT(n, 1);
}

/* %u prints values above INT_MAX as large positive numbers, not negative ones. */
TEST(sprintf_unsigned_above_int_max)
{
	char buf[50];
	sprintf(buf, "%u", 4000000000u);
	ASSERT_EQ_STR(buf, "4000000000");
}

/* %u prints the largest unsigned int in full. */
TEST(sprintf_unsigned_max)
{
	char buf[50];
	sprintf(buf, "%u", 4294967295u);
	ASSERT_EQ_STR(buf, "4294967295");
}

/* %d prints the extreme int values, including INT_MIN, which has no positive form. */
TEST(sprintf_int_extremes)
{
	char buf[50];
	sprintf(buf, "%d", 2147483647);
	ASSERT_EQ_STR(buf, "2147483647");
	sprintf(buf, "%d", -2147483647 - 1);
	ASSERT_EQ_STR(buf, "-2147483648");
}

/* %b prints all 32 bits of a value with the top bit set. */
TEST(sprintf_binary_full_width)
{
	char buf[50];
	sprintf(buf, "%b", 0x80000000u);
	ASSERT_EQ_STR(buf, "10000000000000000000000000000000");
}

/* Known bug: %d ignores the field width, so no padding is added. */
XFAIL_TEST(sprintf_int_field_width)
{
	char buf[50];
	sprintf(buf, "%5d", 42);
	ASSERT_EQ_STR(buf, "   42");
}

/* Known bug: %d ignores the '0' flag as well as the width. */
XFAIL_TEST(sprintf_int_zero_padded)
{
	char buf[50];
	sprintf(buf, "%05d", 42);
	ASSERT_EQ_STR(buf, "00042");
}

/*
 * Known bug: %s only pads when a '0' or ' ' flag is given. A plain width
 * should pad with spaces, as in standard C.
 */
XFAIL_TEST(sprintf_string_field_width_without_flag)
{
	char buf[50];
	sprintf(buf, "%5s", "abc");
	ASSERT_EQ_STR(buf, "  abc");
}
