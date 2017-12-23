#include <core/vfs.h>
#include <core/kprint.h>

vfs_filesystem * VFS_FILESYSTEMS[VFS_MAX_FILESYSTEMS];
int vfs_filesystems_idx;

void vfs_init() {
	size_t vfs_register_func_count = (_register_func_vfs_end - _register_func_vfs_start);
	int i;
	for (i = 0; i < vfs_register_func_count; ++i) {
		_register_func_vfs_start[i]();
	}
	for (i = 0; i < vfs_filesystems_idx; i++) {
		kdebug("filesystem %d: name - %s\n", i, VFS_FILESYSTEMS[i]->name);
	}
}

void vfs_add_filesystem(vfs_filesystem * fs) {
	VFS_FILESYSTEMS[vfs_filesystems_idx++] = fs;
}
