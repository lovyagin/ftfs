/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include <ftfs.h>
#include <string.h>

#define VERSION "1.0.0"

#define FTMK_MSG_USAGE   (FT_PRG |  0)
#define FTMK_MSG_HELP    (FT_PRG |  1)

const char * const ftprg_errlist[] =
{
#include "msg-err.res"
};

const char * const ftprg_msglist[] =
{
#include "msg-msg.res"
};

int main(int argc, char *argv[] )
{
    ft_type t;
    int rc;
    ft_state *state;
    ft_path path = ft_path_init("/");
    uid_t uid;
    gid_t gid;

    ft_init(argv[0], VERSION);

    if (argc < 2 || argc > 3) { ft_put_msg(FTMK_MSG_USAGE, argv[0]); return 1; }
    else
    {
        if (!strcmp (argv[1], "--help"))         { ft_put_msg(FTMK_MSG_HELP, argv[0]);  return 0; }
        else if (!strcmp (argv[1], "--version")) { ft_msg_version();                    return 0; }
        else if (argv[1][0] == '-')              { ft_put_msg(FTMK_MSG_USAGE, argv[0]); return 1; }
        else
        {
            if (argc == 2) t = FT_SQLITE;
            else
            {
                if (!strcmp (argv[2], "db"))          t = FT_SQLITE;
                else if (!strcmp (argv[2], "xattr"))  t = FT_XATTR;
                else if (!strcmp (argv[2], "fxattr")) t = FT_FXATTR;
                else { ft_put_msg(FTMK_MSG_USAGE, argv[0]); return 1; }
            }

            uid = getuid();
            gid = getgid();

            if ((rc = ft_create(argv[1], t)))             { ft_put_msg(rc); return 2; }

            if ((rc = ft_open (argv[1], 0, &state)))      { ft_put_msg(rc); return 2; }

            if ((rc = ft_add(state, &path, uid, gid)))    { ft_put_msg(rc); return 2; }

            ft_close (&state);

            return 0;

        }
    }

}
