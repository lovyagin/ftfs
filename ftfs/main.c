/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
        2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is based on bbfs fuse-tutorial code
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>
*/

#include "ftops.h"

const char * const ftprg_errlist[] =
{
#include "msg-err.res"
};

const char * const ftprg_msglist[] =
{
#include "msg-msg.res"
};

#define FTFS_MSG_USAGE   (FT_PRG |  0)
#define FTFS_MSG_VERSION (FT_PRG |  1)
#define FTFS_MSG_HELP    (FT_PRG |  2)
#define FTFS_MSG_NOCMD   (FT_PRG |  3)

#define VERSION "1.0.0"

char ft_prg_name[FT_LIMIT_PATH];

#define USAGE { ft_put_msg(FTFS_MSG_USAGE, ft_prg_name); return 1; }

int main(int argc, char *argv[])
{

    struct fuse_operations *ftfs_ops = calloc (sizeof (struct fuse_operations), 1);

    char *fuse_args[
#ifndef NDEBUG
7
#else
5
#endif
], *hostdir;

    ft_state *state;

    int rc, cmd = 1;

    ft_str_cpy(ft_prg_name, argv[0], FT_LIMIT_PATH);

    if       (argc == 2)
    {
        if      (!strcmp (argv[1], "-v")) { ft_put_msg(FTFS_MSG_VERSION, ft_prg_name, VERSION, ft_version); return 0; }
        else if (!strcmp (argv[1], "-h")) { ft_put_msg(FTFS_MSG_HELP, ft_prg_name); return 0; }
        else USAGE
    }
    else if (argc == 3)
    {
        hostdir      = realpath(argv[1], NULL);
        fuse_args[4] = argv[2];
    }
    else if (argc == 4)
    {
        if (strcmp (argv[1], "-onocmd")) USAGE
        cmd = 0;
        hostdir      = realpath(argv[2], NULL);
        fuse_args[4] = argv[3];

    }
    else if (argc == 5)
    {
        if (strcmp (argv[1], "-o") || strcmp (argv[2], "nocmd")) USAGE
        cmd = 0;
        hostdir      = realpath(argv[3], NULL);
        fuse_args[4] = argv[4];

    }
    else USAGE

    fuse_args[0] = argv[0];
    fuse_args[1] = "-s";
    fuse_args[2] = "-o";
    fuse_args[3] = "allow_other,auto_unmount,hard_remove";
#ifndef NDEBUG
    fuse_args[5] = "-d";
    fuse_args[6] = "-f";
#endif // NDEBUG

    if (cmd && geteuid() != 0)
    {
        cmd = 0;
        ft_put_msg(FTFS_MSG_NOCMD, argv[0]);
    }

    if ((rc = ft_open(hostdir, cmd, &state))) {ft_put_msg(rc); return 2;}

    if (!ftfs_ops)
    {
        ft_put_msg(FT_ERR_MALLOC);
        return 3;
    }

    ftfs_ops->flag_nullpath_ok   = 0;
    ftfs_ops->flag_nopath        = 0;
    ftfs_ops->flag_utime_omit_ok = 0;

    ftfs_ops->getattr     = ftfs_getattr;
    ftfs_ops->readlink    = ftfs_readlink;
    ftfs_ops->mknod       = ftfs_mknod;
    ftfs_ops->mkdir       = ftfs_mkdir;
    ftfs_ops->unlink      = ftfs_unlink;
    ftfs_ops->rmdir       = ftfs_rmdir;
    ftfs_ops->symlink     = ftfs_symlink;
    ftfs_ops->rename      = ftfs_rename;
    ftfs_ops->chmod       = ftfs_chmod;
    ftfs_ops->chown       = ftfs_chown;
    ftfs_ops->truncate    = ftfs_truncate;
    ftfs_ops->utime       = ftfs_utime;
    ftfs_ops->open        = ftfs_open;
    ftfs_ops->read        = ftfs_read;
    ftfs_ops->write       = ftfs_write;
    ftfs_ops->statfs      = ftfs_statfs;
    ftfs_ops->flush       = ftfs_flush;
    ftfs_ops->release     = ftfs_release;
    ftfs_ops->fsync       = ftfs_fsync;
    ftfs_ops->setxattr    = ftfs_setxattr;
    ftfs_ops->getxattr    = ftfs_getxattr;
    ftfs_ops->listxattr   = ftfs_listxattr;
    ftfs_ops->removexattr = ftfs_removexattr;
    ftfs_ops->opendir     = ftfs_opendir;
    ftfs_ops->readdir     = ftfs_readdir;
    ftfs_ops->releasedir  = ftfs_releasedir;
/*    ftfs_ops->fsyncdir    = ftfs_fsyncdir; */
/*    ftfs_ops->access      = ftfs_access;   */
/*    ftfs_ops->create      = ftfs_create;   */
    ftfs_ops->ftruncate   = ftfs_ftruncate;
    ftfs_ops->fgetattr    = ftfs_fgetattr;

    rc = fuse_main(
#ifndef NDEBUG
7
#else
5
#endif
, fuse_args, ftfs_ops, state);

    free (ftfs_ops);

    ft_close(&state);
    free(hostdir);


    return rc;
}
