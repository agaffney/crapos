#include <core/cmdline.h>
#include <core/kprint.h>
#include <ktest/ktest.h>
#include <libk/string.h>

/* Start and end of the .ktests section, set by the linker script. */
extern const struct ktest _ktests_start[], _ktests_end[];

static int current_failed;

void ktest_fail(const char *file, int line) {
	kprint("    %s:%d: ", file, line);
	current_failed = 1;
}

static int name_equals(const char *a, const char *b) {
	size_t len = strlen(a);
	return len == strlen(b) && memcmp(a, b, len) == 0;
}

/* Selftests fail on purpose, so they only run when selected by name. */
static int is_selftest(const char *name) {
	return strlen(name) >= 9 && memcmp(name, "selftest_", 9) == 0;
}

/* Tests are sorted by name, so duplicates are adjacent. */
static int has_duplicate_names(void) {
	const struct ktest *test;
	int duplicates = 0;

	for (test = _ktests_start; test + 1 < _ktests_end; test++) {
		if (name_equals(test->name, (test + 1)->name)) {
			kprint("ktest: duplicate test name \"%s\"\n", test->name);
			duplicates = 1;
		}
	}
	return duplicates;
}

void ktest_run(void) {
	char filter[KERNEL_CMDLINE_MAX_ARG_LEN + 1];
	const struct ktest *test;
	int ran = 0, failed = 0;

	// "ktest" alone gives an empty value, which selects every test
	filter[0] = '\0';
	cmdline_arg("ktest", filter);

	kprint("ktest: running %s\n", filter[0] ? filter : "all tests");

	if (has_duplicate_names()) {
		ktest_exit(0);
	}

	for (test = _ktests_start; test < _ktests_end; test++) {
		if (filter[0] ? !name_equals(filter, test->name) : is_selftest(test->name)) {
			continue;
		}
		current_failed = 0;
		test->fn();
		ran++;
		if (current_failed) {
			failed++;
			kprint("FAIL  %s\n", test->name);
		} else {
			kprint("ok    %s\n", test->name);
		}
	}

	// A filter that matches nothing is a mistake, not a pass
	if (ran == 0) {
		kprint("ktest: no tests matched\n");
		ktest_exit(0);
	}

	kprint("ktest: %d passed, %d failed\n", ran - failed, failed);
	ktest_exit(failed == 0);
}
