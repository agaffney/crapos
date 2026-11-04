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
