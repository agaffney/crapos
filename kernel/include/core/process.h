#pragma once

#define MAX_PROCS 128
#define KERNEL_PID 0

typedef int t_pid;

typedef struct {
	t_pid pid;
} PROCESS;

void process_init();
