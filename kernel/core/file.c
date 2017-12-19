#include <core/file.h>
#include <string.h>

int FILE_read(FILE * f, char * buf, size_t len) {
	if (f->read_func == NULL) {
		return -1;
	}
	return f->read_func(f, buf, len);
}

int FILE_write(FILE * f, char * buf, size_t len) {
	if (f->write_func == NULL) {
		return -1;
	}
	return f->write_func(f, buf, len);
}

int FILE_close(FILE * f) {
	if (f->close_func == NULL) {
		return -1;
	}
	return f->close_func(f);
}
