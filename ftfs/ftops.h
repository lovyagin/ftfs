/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is based on bbfs fuse-tutorial code
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>
*/

#include "fusecfg.h"
#include <ftfs.h>

int ftfs_getattr     (const char *, struct stat *);

int ftfs_readlink    (const char *, char *, size_t);

int ftfs_mknod       (const char *, mode_t, dev_t);

int ftfs_mkdir       (const char *, mode_t);

int ftfs_unlink      (const char *);

int ftfs_rmdir       (const char *);

int ftfs_symlink     (const char *, const char *);

int ftfs_rename      (const char *, const char *);

int ftfs_chmod       (const char *, mode_t);

int ftfs_chown       (const char *, uid_t, gid_t);

int ftfs_truncate    (const char *, off_t);

int ftfs_utime       (const char *, struct utimbuf *);

int ftfs_open        (const char *, struct fuse_file_info *);

int ftfs_read        (const char *, char *, size_t, off_t, struct fuse_file_info *);

int ftfs_write       (const char *, const char *, size_t, off_t, struct fuse_file_info *);

int ftfs_statfs      (const char *, struct statvfs *);

int ftfs_flush       (const char *, struct fuse_file_info *);

int ftfs_release     (const char *, struct fuse_file_info *);

int ftfs_fsync       (const char *, int, struct fuse_file_info *);

int ftfs_setxattr    (const char *, const char *, const char *, size_t, int);

int ftfs_getxattr    (const char *, const char *, char *, size_t);

int ftfs_listxattr   (const char *, char *, size_t);

int ftfs_removexattr (const char *, const char *);

int ftfs_opendir     (const char *, struct fuse_file_info *);

int ftfs_readdir     (const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);

int ftfs_releasedir  (const char *, struct fuse_file_info *);

/* int ftfs_fsyncdir    (const char *, int, struct fuse_file_info *); */

/* int ftfs_access      (const char *, int); */

/* int ftfs_create      (const char *, mode_t, struct fuse_file_info *); */

int ftfs_ftruncate   (const char *, off_t, struct fuse_file_info *);

int ftfs_fgetattr    (const char *, struct stat *, struct fuse_file_info *);
