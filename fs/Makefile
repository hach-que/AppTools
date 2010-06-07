include ../Makefile.inc

CPPFLAGS+= -I.. -I/usr/include/fuse
OBJLIBS	= ../libfs.a
OBJS	= do_check.o do_fillstatbuf.o do_killfilebyinode.o do_probe.o do_readinode.o ext2-fileio.o fuse-ext2.o op_access.o op_chmod.o op_chown.o op_create.o op_destroy.o op_fgetattr.o op_flush.o op_fsync.o op_getattr.o op_init.o op_link.o op_mkdir.o op_mknod.o op_open.o op_read.o op_readdir.o op_readlink.o op_release.o op_rename.o op_rmdir.o op_statfs.o op_symlink.o op_truncate.o op_unlink.o op_utimens.o op_write.o vnode_hash.o

all: $(OBJLIBS)

../libfs.a: $(OBJS)
	@$(ECHO) $(AR) rv ../libfs.a $?
	@$(AR) rv ../libfs.a $?
	@$(ECHO) $(RANLIB) ../libfs.a
	@$(RANLIB) ../libfs.a

clean:
	@$(ECHO) cleaning up in fs
	@-$(RM) -f $(OBJS)

force_look:
	@true

