/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include <ftfs.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include "ftpc.h"
#include <stdio.h>


const char * const ftprg_errlist[] =
{
#include "msg-err.res"
};

const char * const ftprg_msglist[] =
{
#include "msg-msg.res"
};

char ft_prg_name[FT_LIMIT_PATH];

#define VERSION "1.0.0"

#define USAGE { ft_put_msg(FTPC_MSG_USAGE, ft_prg_name); ft_close(&state); return 1; }

int main(int argc, char *argv[] )
{
    int rc;
    ft_state *state;
    struct stat statbuf;
    ft_prc prc;

    ft_init(argv[0], VERSION);

    if (argc < 2) { ft_put_msg(FTPC_MSG_USAGE, argv[0]); return 1; }

    if (!strcmp (argv[1], "--help"))    { ft_put_msg(FTPC_MSG_HELP, ft_prg_name);  return 0; }
    if (!strcmp (argv[1], "--version")) { ft_msg_version();                        return 0; }
    if (argv[1][0] == '-')              { ft_put_msg(FTPC_MSG_USAGE, ft_prg_name); return 1; }

    if (argc < 3) { ft_put_msg(FTPC_MSG_USAGE, ft_prg_name); return 1; }

    if ((rc = stat (argv[1], &statbuf))) {ft_put_msg(FT_ERR_HOSTERROR); return 2;}

    prc.uid = getuid();
    prc.gid = getgid();
    prc.cmd = NULL;
    prc.ngroups = getgroups (FT_LIMIT_GROUPS, prc.groups);

    if (prc.ngroups == -1) { ft_put_msg(FTPC_ERR_GROUPS, ft_prg_name);                        return 3; }

    if (prc.gid != 0 && prc.gid != statbuf.st_gid)
        if (setgid(statbuf.st_gid)) {ft_put_msg(FTPC_ERR_SETUID); return 3;}
    if (prc.uid != 0 && prc.uid != statbuf.st_uid)
        if (setuid(statbuf.st_uid)) {ft_put_msg(FTPC_ERR_SETUID); return 3;}


    if ((rc = ft_open(argv[1], 1, &state))) {ft_put_msg(rc); return 2;}

    argc -= 2;
    argv += 2;


    if      (!strcmp (argv[0], "type"))            rc = ftpc_type(state, &prc, argc - 1, argv + 1);
    else if (!strcmp (argv[0], "test"))            rc = ftpc_test(state, &prc, argc - 1, argv + 1);
    else if (!strcmp (argv[0], "show"))            rc = ftpc_show(state, &prc, argc - 1, argv + 1);
    else if (!strcmp (argv[0], "get"))
    {
        argc -= 1;
        argv += 1;
        if (argc < 1) USAGE
        if      (!strcmp (argv[0], "uid"))         rc = ftpc_get_uid(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "gid"))         rc = ftpc_get_gid(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "inh"))         rc = ftpc_get_inh(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "prm"))         rc = ftpc_get_prm(state, &prc, argc - 1, argv + 1);
        else    USAGE
    }
    else if (!strcmp (argv[0], "set"))
    {
        argc -= 1;
        argv += 1;
        if (argc < 1) USAGE
        else if (!strcmp (argv[0], "uid"))         rc = ftpc_set_uid(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "gid"))         rc = ftpc_set_gid(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "inh"))         rc = ftpc_set_inh(state, &prc, argc - 1, argv + 1);
        else if (!strcmp (argv[0], "prm"))         rc = ftpc_set_prm(state, &prc, argc - 1, argv + 1);
        else    USAGE
    }

    else    USAGE

    if (rc == FTPC_MSG_USAGE) USAGE

    if (rc) {ft_put_msg(rc); ft_close(&state); return 4;}

    printf ("\n");

    ft_close(&state);

    return 0;
}
