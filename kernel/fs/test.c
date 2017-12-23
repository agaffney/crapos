#include <fs/test.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <libk/string.h>

VFS_REGISTER_FUNC(fs_test_register);
VFS_REGISTER_FUNC(fs_test_register2);

void fs_test_readdir() {
	kdebug("here I am\n");
}

void fs_test_register() {
	vfs_filesystem * fs = (vfs_filesystem *)kmalloc(sizeof(vfs_filesystem), KMALLOC_ZERO);
	strcpy(fs->name, "test1");
	fs->readdir_func = fs_test_readdir;
	vfs_add_filesystem(fs);
}

void fs_test_register2() {
	vfs_filesystem * fs = (vfs_filesystem *)kmalloc(sizeof(vfs_filesystem), KMALLOC_ZERO);
	strcpy(fs->name, "test2");
	fs->readdir_func = fs_test_readdir;
	vfs_add_filesystem(fs);
}
