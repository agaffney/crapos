#pragma once

#include <core/vfs.h>
#include <libk/data/linked_list.h>

void fs_test_register(void);

typedef struct {
	Linked_List * files;
} fs_test_dir;

typedef struct {

} fs_test_mount_info;
