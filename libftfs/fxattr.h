/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#ifndef FXATTR_H_INCLUDED
#define FXATTR_H_INCLUDED

#include "ftfs.h"

int fxattr_open (ft_state *state);
int fxattr_close (ft_state *state);
int fxattr_create (const char *hostdir);

int fxattr_get_owner (ft_state *state, ft_path *path, uid_t *uid);
int fxattr_set_owner (ft_state *state, ft_path *path, uid_t uid);

int fxattr_get_group (ft_state *state, ft_path *path, gid_t *gid);
int fxattr_set_group (ft_state *state, ft_path *path, gid_t gid);

int fxattr_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid);
int fxattr_delete (ft_state *state, ft_path *path);
int fxattr_rename (ft_state *state, ft_path *path, const char* newpath);

int fxattr_set_inh (ft_state *state, ft_path *path, uint64_t inh);
int fxattr_get_inh (ft_state *state, ft_path *path, uint64_t *inh);

int fxattr_set_prm (ft_state *state, ft_path *path, const ft_prm *prm);
int fxattr_get_prm (ft_state *state, ft_path *path, ft_prm *prm);
int fxattr_unset_prm (ft_state *state, ft_path *path, const ft_prm *prm);

ft_prms fxattr_get_prms (ft_state *state, ft_path *path);


#endif // FXATTR_H_INCLUDED
