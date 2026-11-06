#include <ktest/ktest.h>

/* A breakpoint exception (vector 3) is handled and execution continues after it. */
KTEST(int3_returns_to_caller)
{
	int reached = 0;
	asm volatile ("int $3" ::: "memory");
	reached = 1;
	KTEST_ASSERT_TRUE(reached);
}

/* The syscall gate (int 0x80) is handled and returns to the caller. */
KTEST(syscall_returns_to_caller)
{
	int reached = 0;
	asm volatile ("mov $3, %%eax\n\tint $0x80" ::: "eax", "memory");
	reached = 1;
	KTEST_ASSERT_TRUE(reached);
}
