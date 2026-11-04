/* Built with the host's headers and WITHOUT rename.h. */
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "harness.h"

/*
 * By default every test runs in its own child process, so a crash, a
 * sanitizer abort or an infinite loop only fails that one test. A failed
 * assertion in the child exits with TEST_EXIT_ASSERT; any other abnormal
 * exit (signal, sanitizer's exit status, timeout) is reported as a crash.
 */
#define TEST_EXIT_ASSERT 42
#define TEST_EXIT_SKIP 43
#define DEFAULT_TIMEOUT_SECONDS 10

struct test {
	const char *name;
	test_fn fn;
	int expected_failure;
	struct test *next;
};

enum outcome {
	OUTCOME_PASS,
	OUTCOME_SKIP,   /* the test skipped itself */
	OUTCOME_FAIL,   /* an assertion failed */
	OUTCOME_CRASH,  /* signal, sanitizer abort, timeout, ... */
};

static struct test *first_test, *last_test;

static int in_child;            /* running inside a forked child */
static jmp_buf current_jmp;     /* --no-fork only */
static int current_failed;      /* --no-fork only */
static int current_skipped;     /* --no-fork only */

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

/* Report a failed assertion and abandon the current test. */
static void fail(const char *file, int line, const char *fmt, ...)
	__attribute__((noreturn, format(printf, 3, 4)));

static void fail(const char *file, int line, const char *fmt, ...)
{
	va_list ap;
	printf("    %s:%d: ", file, line);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	if (in_child) {
		fflush(stdout);
		_exit(TEST_EXIT_ASSERT);
	}
	current_failed = 1;
	longjmp(current_jmp, 1);
}

/* Abandon the current test without counting it as a pass or a failure. */
void test_skip(const char *file, int line, const char *reason)
{
	printf("    %s:%d: skipped: %s\n", file, line, reason);
	if (in_child) {
		fflush(stdout);
		_exit(TEST_EXIT_SKIP);
	}
	current_skipped = 1;
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

static unsigned int timeout_seconds(void)
{
	const char *env = getenv("TEST_TIMEOUT");
	int secs = env ? atoi(env) : 0;
	return secs > 0 ? (unsigned int)secs : DEFAULT_TIMEOUT_SECONDS;
}

/* Run a test in a child process. 'detail' describes a crash. */
static enum outcome run_forked(const struct test *t, char *detail, size_t detail_len)
{
	pid_t pid;
	int status;

	/* Don't let the child re-flush output the parent has buffered. */
	fflush(stdout);
	fflush(stderr);

	pid = fork();
	if (pid < 0) {
		perror("fork");
		exit(2);
	}
	if (pid == 0) {
		in_child = 1;
		alarm(timeout_seconds());
		t->fn();
		fflush(stdout);
		/*
		 * _exit() skips atexit handlers, which also skips LeakSanitizer.
		 * That's intended for now: kmalloc() has no free(), so code under
		 * test leaks by design.
		 */
		_exit(0);
	}

	if (waitpid(pid, &status, 0) < 0) {
		perror("waitpid");
		exit(2);
	}

	if (WIFEXITED(status)) {
		int code = WEXITSTATUS(status);
		if (code == 0) {
			return OUTCOME_PASS;
		}
		if (code == TEST_EXIT_ASSERT) {
			return OUTCOME_FAIL;
		}
		if (code == TEST_EXIT_SKIP) {
			return OUTCOME_SKIP;
		}
		snprintf(detail, detail_len, "exited with status %d (sanitizer error?)", code);
		return OUTCOME_CRASH;
	}
	if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		if (sig == SIGALRM) {
			snprintf(detail, detail_len, "timed out after %us", timeout_seconds());
		} else {
			snprintf(detail, detail_len, "killed by signal %d (%s)", sig, strsignal(sig));
		}
		return OUTCOME_CRASH;
	}
	snprintf(detail, detail_len, "ended abnormally");
	return OUTCOME_CRASH;
}

/* Run a test in this process. A crash takes down the whole run. */
static enum outcome run_inline(const struct test *t)
{
	current_failed = 0;
	current_skipped = 0;
	if (setjmp(current_jmp) == 0) {
		t->fn();
	}
	if (current_skipped) {
		return OUTCOME_SKIP;
	}
	return current_failed ? OUTCOME_FAIL : OUTCOME_PASS;
}

static void usage(const char *prog)
{
	fprintf(stderr,
		"usage: %s [--list] [--no-fork] [pattern ...]\n"
		"\n"
		"  pattern     run only tests whose name contains the pattern\n"
		"  --list      print the (matching) test names and exit\n"
		"  --no-fork   run tests in this process, e.g. to use a debugger;\n"
		"              a crash then ends the whole run\n"
		"\n"
		"environment: TEST_TIMEOUT=<seconds> per-test time limit (default %d)\n",
		prog, DEFAULT_TIMEOUT_SECONDS);
}

static int matches(const struct test *t, int npatterns, char **patterns)
{
	int i;
	if (npatterns == 0) {
		return 1;
	}
	for (i = 0; i < npatterns; i++) {
		if (strstr(t->name, patterns[i]) != NULL) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv)
{
	int passed = 0, failed = 0, xfailed = 0, xpassed = 0, skipped = 0;
	int list_only = 0, use_fork = 1;
	int npatterns = 0;
	char **patterns = argv + 1;
	struct test *t;
	int i;

	/* Options come first; everything after them is a name pattern. */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--list") == 0) {
			list_only = 1;
		} else if (strcmp(argv[i], "--no-fork") == 0) {
			use_fork = 0;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			usage(argv[0]);
			return 0;
		} else if (argv[i][0] == '-') {
			fprintf(stderr, "unknown option: %s\n", argv[i]);
			usage(argv[0]);
			return 2;
		} else {
			break;
		}
	}
	patterns = argv + i;
	npatterns = argc - i;

	for (t = first_test; t != NULL; t = t->next) {
		enum outcome outcome;
		char detail[100] = "";

		if (!matches(t, npatterns, patterns)) {
			continue;
		}
		if (list_only) {
			printf("%s%s\n", t->name, t->expected_failure ? " (expected failure)" : "");
			continue;
		}

		outcome = use_fork ? run_forked(t, detail, sizeof(detail)) : run_inline(t);

		if (outcome == OUTCOME_SKIP) {
			printf("SKIP  %s\n", t->name);
			skipped++;
		} else if (outcome == OUTCOME_PASS) {
			if (t->expected_failure) {
				printf("XPASS %s (expected to fail; change it to TEST())\n", t->name);
				xpassed++;
			} else {
				printf("ok    %s\n", t->name);
				passed++;
			}
		} else {
			const char *label = t->expected_failure ? "XFAIL" : "FAIL ";
			if (outcome == OUTCOME_CRASH) {
				printf("%s %s (crashed: %s)\n", label, t->name, detail);
			} else {
				printf("%s %s\n", label, t->name);
			}
			if (t->expected_failure) {
				xfailed++;
			} else {
				failed++;
			}
		}
		fflush(stdout);
	}

	if (list_only) {
		return 0;
	}
	if (npatterns > 0 && passed + failed + xfailed + xpassed + skipped == 0) {
		printf("no tests matched the given pattern(s)\n");
	}
	printf("\n%d passed, %d failed, %d expected failures, %d unexpected passes, %d skipped\n",
		passed, failed, xfailed, xpassed, skipped);
	return (failed || xpassed) ? 1 : 0;
}
