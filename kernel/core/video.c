#include <core/video.h>

struct video_driver *VIDEO_DRIVER;

int video_current_row, video_current_col;
int video_rows, video_cols;

void video_init() {
	video_rows = VIDEO_DRIVER->rows;
	video_cols = VIDEO_DRIVER->cols;
	video_set_pos(1, 1);
}

void register_video_driver(struct video_driver *vd) {
	VIDEO_DRIVER = vd;
}

void video_set_pos(short row, short col) {
	video_current_row = row;
	video_current_col = col;
}

void video_putch(char c) {
	VIDEO_DRIVER->putchat_func(c, video_current_row, video_current_col);
	video_current_col++;
	if(video_current_col > video_cols) {
		video_current_col = 1;
		video_current_row++;
	}
}

void video_clear() {
	VIDEO_DRIVER->clear_func();
	video_set_pos(1, 1);
}

void kprint(const char *str) {
	unsigned int i = 0;
	while (str[i] != '\0') {
		video_putch(str[i++]);
	}
}

void kprint_newline(void) {
	video_set_pos(video_current_row + 1, 1);
}

