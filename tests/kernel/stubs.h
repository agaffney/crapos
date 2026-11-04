#pragma once

#include <stddef.h>

/* Everything the stubbed kprint() has printed since the last reset. */
extern char stub_kprint_output[];
void stub_kprint_reset(void);
