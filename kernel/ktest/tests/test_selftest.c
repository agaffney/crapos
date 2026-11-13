#include <ktest/ktest.h>

/*
 * These fail on purpose, to prove that a failing assertion is reported with
 * its location, marks the test FAIL and makes the run exit with the failure
 * status. They only run when selected by name; "make kernel-test-qemu-selftest" runs
 * them and checks that they do fail.
 *
 * The selftests that cause a fatal CPU exception, which halts the machine, are
 * architecture specific and live in tests/arch/<ARCH>/.
 */

KTEST(selftest_failed_assertion)
{
	KTEST_ASSERT_TRUE(1 == 2);
}

KTEST(selftest_failed_eq_assertion)
{
	KTEST_ASSERT_EQ_INT(2 + 2, 5);
}
