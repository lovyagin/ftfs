/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#ifndef XATTR_H_INCLUDED
#define XATTR_H_INCLUDED

#include "ftfs.h"

int xattr_open (ft_state *state);
int xattr_close (ft_state *state);
int xattr_create (const char *hostdir);

int xattr_get_owner (ft_state *state, ft_path *path, uid_t *uid);
int xattr_set_owner (ft_state *state, ft_path *path, uid_t uid);

int xattr_get_group (ft_state *state, ft_path *path, gid_t *gid);
int xattr_set_group (ft_state *state, ft_path *path, gid_t gid);

int xattr_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid);
int xattr_delete (ft_state *state, ft_path *path);
int xattr_rename (ft_state *state, ft_path *path, const char* newpath);

int xattr_set_inh (ft_state *state, ft_path *path, uint64_t inh);
int xattr_get_inh (ft_state *state, ft_path *path, uint64_t *inh);

int xattr_set_prm (ft_state *state, ft_path *path, const ft_prm *prm);
int xattr_get_prm (ft_state *state, ft_path *path, ft_prm *prm);
int xattr_unset_prm (ft_state *state, ft_path *path, const ft_prm *prm);

ft_prms xattr_get_prms (ft_state *state, ft_path *path);


#endif // XATTR_H_INCLUDED
