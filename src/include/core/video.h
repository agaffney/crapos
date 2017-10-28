struct video_driver {
	short rows;
	short cols;
	void (*putchat_func)(char, short, short);
	void (*clear_func)(void);
};

void video_init();
void register_video_driver(struct video_driver *);
void video_clear();

void kprint(const char *);
void kprint_newline();
