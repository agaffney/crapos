#pragma once

#define KERNEL_CMDLINE_MAX_LEN      255
#define KERNEL_CMDLINE_MAX_ARGS     64
#define KERNEL_CMDLINE_MAX_ARG_LEN  128
#define KERNEL_CMDLINE_SPLIT_CHAR   '='
#define KERNEL_CMDLINE_ARG_SEP      ' '

void cmdline_init(const char *);
void cmdline_parse(void);
int cmdline_has_arg(const char *);
int cmdline_arg(const char *, char *);
void _cmdline_split_arg(const char *, char *, char *);
