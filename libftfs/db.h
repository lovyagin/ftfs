/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#ifndef DB_H_INCLUDED
#define DB_H_INCLUDED

#include "ftfs.h"

#define QUERY_BUFFER  2048

int db_open (ft_state *state);
int db_close (ft_state *state);
int db_create (const char *hostdir);

int db_get_owner (ft_state *state, ft_path *path, uid_t *uid);
int db_set_owner (ft_state *state, ft_path *path, uid_t uid);

int db_get_group (ft_state *state, ft_path *path, gid_t *gid);
int db_set_group (ft_state *state, ft_path *path, gid_t gid);

int db_get_inh (ft_state *state, ft_path *path, uint64_t *inh);
int db_set_inh (ft_state *state, ft_path *path, uint64_t inh);

int db_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid);
int db_delete (ft_state *state, ft_path *path);
int db_rename (ft_state *state, ft_path *path, const char* newpath);


int db_get_prm (ft_state *state, ft_path *path, ft_prm *prm);
int db_set_prm (ft_state *state, ft_path *path, const ft_prm *prm);
int db_unset_prm (ft_state *state, ft_path *path, const ft_prm *prm);

ft_prms db_get_prms (ft_state *state, ft_path *path);


#endif // DB_H_INCLUDED
