#pragma once

void kprint(const char* restrict format, ...);

#define kdebug(fmt, ...) kprint("[DEBUG] (%s:%d):%s() " fmt,  __FILE__, __LINE__, __func__, ##__VA_ARGS__)
