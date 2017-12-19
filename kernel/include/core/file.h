#pragma once

#include <stddef.h>

struct _FILE;

typedef int (*FILE_READ_FUNC)(struct _FILE *, char *, size_t);
typedef int (*FILE_WRITE_FUNC)(struct _FILE *, char *, size_t);
typedef int (*FILE_CLOSE_FUNC)(struct _FILE *);

struct _FILE {
	FILE_READ_FUNC read_func;
	FILE_WRITE_FUNC write_func;
	FILE_CLOSE_FUNC close_func;
};

typedef struct _FILE FILE;

int FILE_read(FILE *, char *, size_t);
int FILE_write(FILE *, char *, size_t);
int FILE_close(FILE *);
