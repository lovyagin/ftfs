/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

"Usage: %s [-h|-v] [-o nocmd] hostdir mountpoint",                       /* FTFS_MSG_USAGE     */
"%s version %s, libftfs version %s",                                     /* FTFS_MSG_VERSION   */
"FTFS fuse module\n"
    "%s [-h|-v] [-o nocmd] hostdir mountpoint\n"
    "-h\t\tshow this help\n"
    "-v\t\tprint version\n"
    "-onocmd\tdon't check program\'s permissions\n\t\tthis control is enabled by default on root-mount,\n\t\talways disabled on user mount\n"
    "hostdir\t\tFTFS host directory\n"
    "mountpoint\tmount point\n",                                         /* FTFS_MSG_HELP      */
"%s warning: user mount, -o nocmd applied",                              /* FTFS_MSG_NOCMD     */
