#include <arch/x86/video.h>
#include <core/video.h>

char *vidptr = (char*)VIDEO_MEM_BASE_ADDR;

struct video_driver video_drv;

void clear_screen(void) {
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_BLACK);
	}
}

char video_create_attribute_byte(enum video_colors bg, enum video_colors fg) {
	return (bg << 4) + fg;
}

void putchat(char c, short row, short col) {
	int idx = BYTES_FOR_EACH_ELEMENT * ( ( COLUMNS_IN_LINE * (row - 1) ) + (col - 1) );
	vidptr[idx] = c;
	vidptr[idx+1] = video_create_attribute_byte(VIDEO_COLOR_BLACK, VIDEO_COLOR_LIGHT_GREY);
}

void arch_video_init() {
	video_drv.putchat_func = putchat;
	video_drv.clear_func = clear_screen;
	video_drv.rows = LINES;
	video_drv.cols = COLUMNS_IN_LINE;
	register_video_driver(&video_drv);
}
