#include <string.h>

#include "harness.h"

static void fill(unsigned char *buf, unsigned char value, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++) {
		buf[i] = value;
	}
}

/* strlen() counts characters before the terminator, including for "". */
TEST(strlen_basic)
{
	ASSERT_EQ_INT(strlen("hello"), 5);
	ASSERT_EQ_INT(strlen(""), 0);
}

/* strlen() stops at the first NUL, ignoring anything after it. */
TEST(strlen_stops_at_first_nul)
{
	ASSERT_EQ_INT(strlen("ab\0cd"), 2);
}

/* strcpy() copies the terminator and touches nothing after it. */
TEST(strcpy_copies_string_and_terminator)
{
	char dst[8];
	fill((unsigned char *)dst, 'x', sizeof(dst));
	strcpy(dst, "abc");
	ASSERT_EQ_STR(dst, "abc");
	ASSERT_EQ_INT(dst[4], 'x');
}

/* strcpy() returns its destination pointer. */
TEST(strcpy_returns_dest)
{
	char dst[8];
	ASSERT_TRUE(strcpy(dst, "a") == dst);
}

/* strcpy() of an empty string writes just the terminator. */
TEST(strcpy_empty_string)
{
	char dst[4];
	fill((unsigned char *)dst, 'x', sizeof(dst));
	strcpy(dst, "");
	ASSERT_EQ_INT(dst[0], 0);
	ASSERT_EQ_INT(dst[1], 'x');
}

/* strncpy() zero-fills the rest of the buffer up to n when src is shorter. */
TEST(strncpy_pads_with_nuls)
{
	char dst[8];
	int i;
	fill((unsigned char *)dst, 'x', sizeof(dst));
	strncpy(dst, "ab", 6);
	ASSERT_EQ_INT(dst[0], 'a');
	ASSERT_EQ_INT(dst[1], 'b');
	for (i = 2; i < 6; i++) {
		ASSERT_EQ_INT(dst[i], 0);
	}
	ASSERT_EQ_INT(dst[6], 'x');
}

/* strncpy() copies exactly n bytes, with no terminator, when src is longer. */
TEST(strncpy_truncates_without_terminator)
{
	char dst[8];
	fill((unsigned char *)dst, 'x', sizeof(dst));
	strncpy(dst, "abcdef", 3);
	ASSERT_EQ_INT(dst[0], 'a');
	ASSERT_EQ_INT(dst[1], 'b');
	ASSERT_EQ_INT(dst[2], 'c');
	ASSERT_EQ_INT(dst[3], 'x');
}

/* strncpy() with n == 0 leaves the destination untouched and returns it. */
TEST(strncpy_zero_length)
{
	char dst[4];
	fill((unsigned char *)dst, 'x', sizeof(dst));
	ASSERT_TRUE(strncpy(dst, "abc", 0) == dst);
	ASSERT_EQ_INT(dst[0], 'x');
}

/* memcpy() copies exactly len bytes and returns the destination. */
TEST(memcpy_copies_len_bytes)
{
	unsigned char src[5] = {1, 2, 3, 4, 5};
	unsigned char dst[5] = {0, 0, 0, 0, 0};
	ASSERT_TRUE(memcpy(dst, src, 3) == dst);
	ASSERT_EQ_INT(dst[0], 1);
	ASSERT_EQ_INT(dst[1], 2);
	ASSERT_EQ_INT(dst[2], 3);
	ASSERT_EQ_INT(dst[3], 0);
	ASSERT_EQ_INT(dst[4], 0);
}

/* memcpy() copies NUL bytes like any other, unlike the str* functions. */
TEST(memcpy_copies_embedded_nuls)
{
	unsigned char src[4] = {'a', 0, 'b', 0};
	unsigned char dst[4] = {1, 1, 1, 1};
	memcpy(dst, src, sizeof(src));
	ASSERT_EQ_INT(dst[0], 'a');
	ASSERT_EQ_INT(dst[1], 0);
	ASSERT_EQ_INT(dst[2], 'b');
	ASSERT_EQ_INT(dst[3], 0);
}

/* memcpy() with a length of 0 copies nothing. */
TEST(memcpy_zero_length)
{
	unsigned char src[2] = {9, 9};
	unsigned char dst[2] = {1, 1};
	memcpy(dst, src, 0);
	ASSERT_EQ_INT(dst[0], 1);
	ASSERT_EQ_INT(dst[1], 1);
}

/* memset() fills exactly len bytes and leaves the bytes around them alone. */
TEST(memset_fills_len_bytes)
{
	unsigned char buf[8];
	size_t i;
	fill(buf, 'x', sizeof(buf));
	memset(buf + 2, 'a', 4);
	for (i = 0; i < sizeof(buf); i++) {
		ASSERT_EQ_INT(buf[i], (i >= 2 && i < 6) ? 'a' : 'x');
	}
}

/* memset() returns its destination pointer. */
TEST(memset_returns_dest)
{
	char buf[4];
	ASSERT_TRUE(memset(buf, 0, sizeof(buf)) == buf);
}

/* A zero length writes nothing. */
TEST(memset_zero_length)
{
	unsigned char buf[4];
	fill(buf, 'x', sizeof(buf));
	memset(buf, 'a', 0);
	ASSERT_EQ_INT(buf[0], 'x');
	ASSERT_EQ_INT(buf[3], 'x');
}

/* The fill value is converted to unsigned char, so only its low byte is used. */
TEST(memset_fill_converted_to_unsigned_char)
{
	unsigned char buf[4];
	memset(buf, 0x1ff, sizeof(buf));
	ASSERT_EQ_INT(buf[0], 0xff);
	ASSERT_EQ_INT(buf[3], 0xff);
	memset(buf, -1, sizeof(buf));
	ASSERT_EQ_INT(buf[1], 0xff);
}

/* memset() with zero clears a buffer, including bytes that were 0xff. */
TEST(memset_zero_clears)
{
	unsigned char buf[4];
	fill(buf, 0xff, sizeof(buf));
	memset(buf, 0, sizeof(buf));
	ASSERT_EQ_INT(buf[0] | buf[1] | buf[2] | buf[3], 0);
}

/* memcmp() returns 0 for equal buffers, and for a zero length. */
TEST(memcmp_equal)
{
	ASSERT_EQ_INT(memcmp("abc", "abc", 3), 0);
	ASSERT_EQ_INT(memcmp("abc", "xyz", 0), 0);
}

/* memcmp() is negative when the first differing byte is smaller in the first buffer. */
TEST(memcmp_less)
{
	ASSERT_TRUE(memcmp("abc", "abd", 3) < 0);
}

/* memcmp() is positive when the first differing byte is larger in the first buffer. */
TEST(memcmp_greater)
{
	ASSERT_TRUE(memcmp("abd", "abc", 3) > 0);
}

/* Only the first len bytes are compared. */
TEST(memcmp_respects_len)
{
	ASSERT_EQ_INT(memcmp("abcX", "abcY", 3), 0);
}

/* The first difference decides the result, regardless of later bytes. */
TEST(memcmp_first_difference_wins)
{
	ASSERT_TRUE(memcmp("ab9", "ac0", 3) < 0);
}

/* memcmp() does not stop at an embedded NUL, unlike strcmp(). */
TEST(memcmp_embedded_nuls)
{
	ASSERT_EQ_INT(memcmp("a\0b", "a\0b", 3), 0);
	ASSERT_TRUE(memcmp("a\0b", "a\0c", 3) < 0);
}

/* Bytes compare as unsigned: 0xff is greater than 0x01, not negative. */
TEST(memcmp_compares_unsigned_bytes)
{
	unsigned char a[1] = { 0xff }, b[1] = { 0x01 };
	ASSERT_TRUE(memcmp(a, b, 1) > 0);
	ASSERT_TRUE(memcmp(b, a, 1) < 0);
}

/* memcmp() finds a difference in the last byte of a long run of equal bytes. */
TEST(memcmp_difference_in_last_byte_of_long_run)
{
	unsigned char a[1024], b[1024];
	fill(a, 0x5a, sizeof(a));
	fill(b, 0x5a, sizeof(b));
	ASSERT_EQ_INT(memcmp(a, b, sizeof(a)), 0);
	b[sizeof(b) - 1] = 0x5b;
	ASSERT_TRUE(memcmp(a, b, sizeof(a)) < 0);
	ASSERT_TRUE(memcmp(b, a, sizeof(a)) > 0);
	// The differing byte is outside the compared range when len is shorter
	ASSERT_EQ_INT(memcmp(a, b, sizeof(a) - 1), 0);
}

/* memset() fills a long buffer completely without writing past either end. */
TEST(memset_long_length)
{
	unsigned char buf[4096 + 2];
	size_t i;
	fill(buf, 'x', sizeof(buf));
	memset(buf + 1, 'a', 4096);
	ASSERT_EQ_INT(buf[0], 'x');
	ASSERT_EQ_INT(buf[4097], 'x');
	for (i = 1; i <= 4096; i++) {
		ASSERT_EQ_INT(buf[i], 'a');
	}
}
