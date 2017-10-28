#include <arch/x86/video.h>

/* current cursor location */
unsigned int current_loc = 0;

char *vidptr = (char*)VIDEO_MEM_BASE_ADDR;

void kprint(const char *str)
{
	unsigned int i = 0;
	while (str[i] != '\0') {
		putch(str[i++]);
	}
}

void kprint_newline(void)
{
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
}

void clear_screen(void)
{
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_BLACK);
	}
	current_loc = 0;
}

char video_create_attribute_byte(enum video_colors bg, enum video_colors fg) {
	return (bg << 4) + fg;
}

void putch(char c) {
	vidptr[current_loc++] = c;
	vidptr[current_loc++] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_WHITE);
}
