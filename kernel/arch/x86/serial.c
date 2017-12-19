#include <arch/x86/io.h>
#include <arch/x86/serial.h>
#include <core/file.h>
#include <core/console.h>
#include <core/vmm.h>
 
void init_serial() {
	outb(SERIAL_COM1_ADDR + 1, 0x00);    // Disable all interrupts
	outb(SERIAL_COM1_ADDR + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outb(SERIAL_COM1_ADDR + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outb(SERIAL_COM1_ADDR + 1, 0x00);    //                  (hi byte)
	outb(SERIAL_COM1_ADDR + 3, 0x03);    // 8 bits, no parity, one stop bit
	outb(SERIAL_COM1_ADDR + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outb(SERIAL_COM1_ADDR + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	FILE * serial_com1 = (FILE *)kmalloc(sizeof(FILE), KMALLOC_ZERO);
	serial_com1->write_func = serial_write;

	console_set(serial_com1);
}

int is_transmit_empty() {
   return inb(SERIAL_COM1_ADDR + 5) & 0x20;
}
 
void write_serial(char a) {
	while (is_transmit_empty() == 0);

	outb(SERIAL_COM1_ADDR, a);
}

int serial_write(FILE * f, char * buf, size_t len) {
	int i;
	for (i = 0; i < len; i++) {
		while (!is_transmit_empty());
		outb(SERIAL_COM1_ADDR, buf[i]);
	}
	return len;
}
