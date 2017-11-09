/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#ifndef FTPC_H_INCLUDED
#define FTPC_H_INCLUDED

#include <ftfs.h>

#define FTPC_MSG_USAGE   (FT_PRG |  0)
#define FTPC_MSG_HELP    (FT_PRG |  1)

#define FTPC_ERR_SETUID  (FT_PRG | FT_ERR |  0)
#define FTPC_ERR_PRM     (FT_PRG | FT_ERR |  1)
#define FTPC_ERR_NUMBER  (FT_PRG | FT_ERR |  2)
#define FTPC_ERR_GROUPS  (FT_PRG | FT_ERR |  3)
#define FTPC_ERR_INH     (FT_PRG | FT_ERR |  4)
#define FTPC_ERR_SET     (FT_PRG | FT_ERR |  5)
#define FTPC_ERR_CAT     (FT_PRG | FT_ERR |  6)
#define FTPC_ERR_PFL     (FT_PRG | FT_ERR |  7)
#define FTPC_ERR_PID     (FT_PRG | FT_ERR |  8)
#define FTPC_ERR_AD      (FT_PRG | FT_ERR |  9)
#define FTPC_ERR_DFP     (FT_PRG | FT_ERR | 10)
#define FTPC_ERR_DFI     (FT_PRG | FT_ERR | 11)
#define FTPC_ERR_PRMGET  (FT_PRG | FT_ERR | 12)

int ftpc_type (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_show (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_test (ft_state *state, ft_prc *prc, int argc, char *argv[]);

int ftpc_get_uid (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_get_gid (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_get_inh (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_get_prm (ft_state *state, ft_prc *prc, int argc, char *argv[]);

int ftpc_set_uid (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_set_gid (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_set_inh (ft_state *state, ft_prc *prc, int argc, char *argv[]);
int ftpc_set_prm (ft_state *state, ft_prc *prc, int argc, char *argv[]);

#endif // FTPC_H_INCLUDED
