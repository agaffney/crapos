#define SERIAL_COM1_ADDR 0x3f8

void init_serial();
int is_transmit_empty();
void write_serial(char);
