#pragma once

#include <core/file.h>

#define SERIAL_COM1_ADDR 0x3f8

void init_serial(void);
int is_transmit_empty(void);
void write_serial(char);

int serial_write(FILE *, char *, size_t);
