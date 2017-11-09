#ifndef FUSECFG_H_INCLUDED
#define FUSECFG_H_INCLUDED

#define FUSE_USE_VERSION 26

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the `fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1
#include <limits.h>

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1
#include <memory.h>

/* Define to 1 if you have the `mkdir' function. */
#define HAVE_MKDIR 1

/* Define to 1 if you have the `mkfifo' function. */
// #define HAVE_MKFIFO 1

/* Define to 1 if you have the `realpath' function. */
// #define HAVE_REALPATH 1

/* Define to 1 if you have the `rmdir' function. */
#define HAVE_RMDIR 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1
#include <stdint.h>

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1
#include <stdlib.h>

/* Define to 1 if you have the `strerror' function. */
// #define HAVE_STRERROR 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1
#include <strings.h>

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1
#include <string.h>

/* Define to 1 if `st_blksize' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1

/* Define to 1 if `st_blocks' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLOCKS 1

/* Define to 1 if `st_rdev' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_RDEV 1

/* Define to 1 if your `struct stat' has `st_blocks'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLOCKS' instead. */
#define HAVE_ST_BLOCKS 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1
#include <sys/statvfs.h>

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1
#include <sys/stat.h>

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1
#include <sys/types.h>

/* Define to 1 if you have the <sys/xattr.h> header file. */
#define HAVE_SYS_XATTR_H 1
#include <sys/xattr.h>

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1
#include <unistd.h>

/* Define to 1 if you have the `utime' function. */
#define HAVE_UTIME 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1
#include <utime.h>

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing slash. */
#define LSTAT_FOLLOWS_SLASHED_SYMLINK 1

/* Name of package */
#define PACKAGE "ftfs"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "lovyagin@mail.com"

/* Define to the full name of this package. */
#define PACKAGE_NAME "fine tuning file system"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "fine tuning file system 1.0.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "ftfs"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.0"

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "1.0.0"

#include <fuse.h>


#endif // FUSECFG_H_INCLUDED
