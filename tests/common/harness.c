/* Built with the host's headers and WITHOUT rename.h. */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"

struct test {
	const char *name;
	test_fn fn;
	int expected_failure;
	struct test *next;
};

static struct test *first_test, *last_test;
static jmp_buf current_jmp;
static int current_failed;

void test_register(const char *name, test_fn fn, int expected_failure)
{
	struct test *t = calloc(1, sizeof(*t));
	t->name = name;
	t->fn = fn;
	t->expected_failure = expected_failure;
	if (last_test) {
		last_test->next = t;
	} else {
		first_test = t;
	}
	last_test = t;
}

/* Record a failure and abandon the current test. */
static void fail(const char *file, int line, const char *fmt, ...)
	__attribute__((noreturn, format(printf, 3, 4)));

#include <stdarg.h>
static void fail(const char *file, int line, const char *fmt, ...)
{
	va_list ap;
	printf("    %s:%d: ", file, line);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	current_failed = 1;
	longjmp(current_jmp, 1);
}

void test_check_int(const char *file, int line, const char *expr,
		long long actual, long long expected)
{
	if (actual != expected) {
		fail(file, line, "%s: got %lld, expected %lld", expr, actual, expected);
	}
}

void test_check_str(const char *file, int line, const char *expr,
		const char *actual, const char *expected)
{
	if (actual == NULL || expected == NULL) {
		if (actual != expected) {
			fail(file, line, "%s: got %s, expected %s", expr,
				actual ? actual : "NULL", expected ? expected : "NULL");
		}
		return;
	}
	if (strcmp(actual, expected) != 0) {
		fail(file, line, "%s: got \"%s\", expected \"%s\"", expr, actual, expected);
	}
}

void test_check_true(const char *file, int line, const char *expr, int value)
{
	if (!value) {
		fail(file, line, "assertion failed: %s", expr);
	}
}

int main(void)
{
	int passed = 0, failed = 0, xfailed = 0, xpassed = 0;
	struct test *t;

	for (t = first_test; t != NULL; t = t->next) {
		current_failed = 0;
		if (setjmp(current_jmp) == 0) {
			t->fn();
		}
		if (current_failed) {
			if (t->expected_failure) {
				printf("XFAIL %s\n", t->name);
				xfailed++;
			} else {
				printf("FAIL  %s\n", t->name);
				failed++;
			}
		} else if (t->expected_failure) {
			printf("XPASS %s (expected to fail; change it to TEST())\n", t->name);
			xpassed++;
		} else {
			printf("ok    %s\n", t->name);
			passed++;
		}
		fflush(stdout);
	}

	printf("\n%d passed, %d failed, %d expected failures, %d unexpected passes\n",
		passed, failed, xfailed, xpassed);
	return (failed || xpassed) ? 1 : 0;
}
