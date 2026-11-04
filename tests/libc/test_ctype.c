#include <ctype.h>

#include "harness.h"

/* tolower() maps every uppercase ASCII letter to its lowercase form. */
TEST(tolower_uppercase_letters)
{
	int c;
	for (c = 'A'; c <= 'Z'; c++) {
		ASSERT_EQ_INT(tolower(c), c - 'A' + 'a');
	}
}

/* tolower() leaves everything outside 'A'..'Z' alone, including neighbors. */
TEST(tolower_leaves_other_characters)
{
	int c;
	for (c = 0; c < 'A'; c++) {
		ASSERT_EQ_INT(tolower(c), c);
	}
	for (c = 'Z' + 1; c < 128; c++) {
		ASSERT_EQ_INT(tolower(c), c);
	}
}

/* toupper() maps every lowercase ASCII letter to its uppercase form. */
TEST(toupper_lowercase_letters)
{
	int c;
	for (c = 'a'; c <= 'z'; c++) {
		ASSERT_EQ_INT(toupper(c), c - 'a' + 'A');
	}
}

/* toupper() leaves everything outside 'a'..'z' alone, including neighbors. */
TEST(toupper_leaves_other_characters)
{
	int c;
	for (c = 0; c < 'a'; c++) {
		ASSERT_EQ_INT(toupper(c), c);
	}
	for (c = 'z' + 1; c < 128; c++) {
		ASSERT_EQ_INT(toupper(c), c);
	}
}

/* Values outside the ASCII range, such as EOF (-1), pass through unchanged. */
TEST(case_conversion_passes_through_non_ascii)
{
	ASSERT_EQ_INT(tolower(-1), -1);
	ASSERT_EQ_INT(toupper(-1), -1);
	ASSERT_EQ_INT(tolower(200), 200);
	ASSERT_EQ_INT(toupper(200), 200);
}
