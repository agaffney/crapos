#include <fs/test.h>
#include <core/kprint.h>
#include <core/vmm.h>
#include <libk/string.h>

VFS_REGISTER_FUNC(fs_test_register);

void fs_test_readdir() {
	kdebug("here I am\n");
}

void fs_test_mount(vfs_mount_args * args) {
	kdebug("device = %s, mountpoint = %s, options = %s\n", args->device, args->mountpoint, args->options);
	vfs_mount * mount = (vfs_mount *)kmalloc(sizeof(vfs_mount), KMALLOC_ZERO);
	mount->mount_data = (void *)kmalloc(sizeof(fs_test_mount_info), KMALLOC_ZERO);
}

void fs_test_register() {
	vfs_filesystem * fs = (vfs_filesystem *)kmalloc(sizeof(vfs_filesystem), KMALLOC_ZERO);
	strcpy(fs->name, "test");
	fs->mount_func = fs_test_mount;
	fs->readdir_func = fs_test_readdir;
	vfs_add_filesystem(fs);
}
