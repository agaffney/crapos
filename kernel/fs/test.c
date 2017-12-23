#include <fs/test.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <libk/string.h>

VFS_REGISTER_FUNC(fs_test_register);
VFS_REGISTER_FUNC(fs_test_register2);

void fs_test_register() {
	vfs_filesystem * fs = (vfs_filesystem *)kmalloc(sizeof(vfs_filesystem), KMALLOC_ZERO);
	strcpy(fs->name, "test1");
	vfs_add_filesystem(fs);
}

void fs_test_register2() {
	vfs_filesystem * fs = (vfs_filesystem *)kmalloc(sizeof(vfs_filesystem), KMALLOC_ZERO);
	strcpy(fs->name, "test2");
	vfs_add_filesystem(fs);
}
