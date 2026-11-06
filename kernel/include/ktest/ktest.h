#pragma once

#include <core/kprint.h>

/*
 * In-kernel tests, run under QEMU. They are only compiled and linked into the
 * kernel when it is built with KTEST=1 (see kernel/Makefile), and only run when
 * the kernel command line contains "ktest" (all tests) or "ktest=<name>" (one
 * test). Results go to the console, and the kernel then asks QEMU to exit with
 * a status that says whether everything passed.
 *
 * Define a test with KTEST(name) in any file under kernel/ktest/tests/. Tests
 * are collected by the linker into the ".ktests" section, so there is no list
 * to maintain. Names must be unique (a duplicate fails the whole run), and
 * tests run in alphabetical order. Tests named selftest_* fail on purpose, to
 * check the framework itself, and only run when selected by name (see
 * "make kernel-test-qemu-selftest").
 *
 * A failed assertion records the failure and returns from the function it is
 * written in. Use the assertion macros directly in the test body: in a helper
 * function they would only return from the helper, and the test would carry on
 * as if nothing had failed. Have the helper return a value and assert on that
 * in the test.
 */

struct ktest {
	const char *name;
	void (*fn)(void);
};

#define KTEST(test_name) \
	static void ktest_fn_##test_name(void); \
	static const struct ktest ktest_entry_##test_name \
		__attribute__((used, section(".ktests." #test_name))) = { #test_name, ktest_fn_##test_name }; \
	static void ktest_fn_##test_name(void)

/*
 * Report a failure with a printf-style message. The message goes straight to
 * kprint() rather than through a buffer, so a long stringified expression
 * can't overflow anything.
 */
#define KTEST_FAIL(...) \
	do { \
		ktest_fail(__FILE__, __LINE__); \
		kprint(__VA_ARGS__); \
	} while (0)

#define KTEST_ASSERT_TRUE(cond) \
	do { \
		if (!(cond)) { \
			KTEST_FAIL("%s\n", #cond); \
			return; \
		} \
	} while (0)

#define KTEST_ASSERT_EQ_INT(actual, expected) \
	do { \
		_Static_assert(sizeof(actual) <= sizeof(int) && sizeof(expected) <= sizeof(int), \
			"KTEST_ASSERT_EQ_INT would truncate its arguments"); \
		int ktest_actual = (int)(actual); \
		int ktest_expected = (int)(expected); \
		if (ktest_actual != ktest_expected) { \
			KTEST_FAIL("%s: got %d, expected %d\n", #actual, ktest_actual, ktest_expected); \
			return; \
		} \
	} while (0)

/*
 * Record a failure of the running test and print the "file:line: " prefix of
 * its message. Use KTEST_FAIL() rather than calling this directly.
 */
void ktest_fail(const char *file, int line);

/*
 * Run the selected tests, report, and exit QEMU. Does not return. Only linked
 * into KTEST=1 kernels. The definition is a normal strong symbol, so a second
 * one fails at link time; main.c marks its own use as weak (#pragma weak) to
 * test whether this was linked in.
 */
void ktest_run(void);

/* Architecture hook: stop the machine, telling QEMU whether the run passed. */
void ktest_exit(int passed);
