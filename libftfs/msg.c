/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include <stdarg.h>
#include <stdio.h>

#include "ftfs.h"

const char * const ft_errlist[] =
{
#include "msg-err.res"
};
const char * const ft_msglist[] =
{
#include "msg-msg.res"
};

const char *ft_msg (size_t idx)
{
    const char * const * list = idx & FT_ERR ? (idx & FT_PRG ? ftprg_errlist : ft_errlist)
                                             : (idx & FT_PRG ? ftprg_msglist : ft_msglist);
    idx &= ~(FT_ERR | FT_PRG);

    return list[idx];
}

void ft_put_msg (size_t idx, ...)
{
    FILE *stream = (idx & FT_ERR) ? stderr : stdout;

    if (idx & FT_ERR) fprintf(stream, "%s error: ", ft_prg_name);

    va_list ap;
    va_start(ap, idx);
    vfprintf(stream, ft_msg(idx), ap);
    fprintf(stream, "\n");
}

void ft_msg_version ()
{
    ft_put_msg(FT_MSG_VERSION, ft_prg_name, ft_prg_version, ft_name, ft_version);
}

