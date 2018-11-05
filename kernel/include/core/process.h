#pragma once

#define MAX_PROCS 128
#define KERNEL_PID 0

typedef struct {
	int pid;
} PROCESS;

void process_init();
