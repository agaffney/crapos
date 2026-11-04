/*
 * Minimal test harness. This header deliberately includes nothing, so it can
 * be used from code compiled with -nostdinc against the libc under test.
 * All printing and bookkeeping happens in harness.c, which is built against
 * the host's headers.
 */
#pragma once

typedef void (*test_fn)(void);

void test_register(const char *name, test_fn fn, int expected_failure);
void test_check_int(const char *file, int line, const char *expr,
		long long actual, long long expected);
void test_check_str(const char *file, int line, const char *expr,
		const char *actual, const char *expected);
void test_check_true(const char *file, int line, const char *expr, int value);

/* Define and auto-register a test. */
#define TEST(name) \
	static void name(void); \
	__attribute__((constructor)) static void register_##name(void) { \
		test_register(#name, name, 0); \
	} \
	static void name(void)

/*
 * Define a test for a known bug. It is reported as XFAIL while it fails and
 * the suite still passes. If it starts passing it is reported as XPASS and the
 * suite FAILS, as a reminder to change it into a normal TEST().
 */
#define XFAIL_TEST(name) \
	static void name(void); \
	__attribute__((constructor)) static void register_##name(void) { \
		test_register(#name, name, 1); \
	} \
	static void name(void)

#define ASSERT_EQ_INT(actual, expected) \
	test_check_int(__FILE__, __LINE__, #actual, (long long)(actual), (long long)(expected))
#define ASSERT_EQ_STR(actual, expected) \
	test_check_str(__FILE__, __LINE__, #actual, (actual), (expected))
#define ASSERT_TRUE(cond) \
	test_check_true(__FILE__, __LINE__, #cond, !!(cond))
#define ASSERT_NULL(ptr) \
	test_check_true(__FILE__, __LINE__, #ptr " == NULL", (ptr) == 0)
