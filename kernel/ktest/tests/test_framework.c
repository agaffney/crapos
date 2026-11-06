#include <ktest/ktest.h>

/* The framework runs a test and counts it as passed when its assertions hold. */
KTEST(framework_assertions_pass)
{
	KTEST_ASSERT_TRUE(1 + 1 == 2);
	KTEST_ASSERT_EQ_INT(2 + 2, 4);
}
