/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include "ftfs.h"
#include "fxattr.h"
#include <sys/xattr.h>
#include <stdlib.h>
#include <errno.h>
#include <attr/xattr.h>
#include <string.h>

#include "bswap.h"
#ifdef WORDS_BIGENDIAN
#define uint64_convert_order(x) x = bswap_64(x);
#else
#define uint64_convert_order(x)
#endif


int fxattr_open (ft_state *state)
{
     char value;
     if (lgetxattr(state->datadir, "user.ftfs", &value, 1) != 1 ||
         value != '!'
        ) return FT_ERR_XATTROPEN;

     state->type = FT_FXATTR;

     return FT_OK;
}


int fxattr_close (ft_state *state)
{
    (void) state;
    return FT_OK;
}


int fxattr_create (const char *hostdir)
{
    const char c = '!';
    char datadir[FT_LIMIT_PATH];
    if (!ft_str_cat2 (datadir, hostdir, "/data", FT_LIMIT_PATH)) return FT_ERR_PATH;

    return lsetxattr(datadir, "user.ftfs", &c, 1, XATTR_CREATE) ? FT_ERR_XATTRCRT : FT_OK;
}

int fxattr_unset (ft_state *state, ft_path *path, const char *name)
{
    int r = lremovexattr(FT_HOSTPATH, name);
    return (r && errno != ENOATTR) ? FT_ERR_XATTRCRT : FT_OK;
}

int fxattr_get_int (ft_state *state, ft_path *path, const char *name, uint64_t *attr)
{
     int rc = lgetxattr(FT_HOSTPATH, name, attr, 8);

     if (rc == 8)
     {
         uint64_convert_order(*attr);
         return FT_OK;
     }

     if (rc == -1 && errno == ENOATTR)
     {
         *attr = 0;
         return FT_OK;
     }

     return FT_ERR_XATTROPEN;

}

int fxattr_set_int (ft_state *state, ft_path *path, const char *name, uint64_t attr)
{
    uint64_convert_order(attr);

    if (lsetxattr(FT_HOSTPATH, name, &attr, 8, 0))
        return FT_ERR_XATTRCRT;

    return FT_OK;

}



int fxattr_get_owner (ft_state *state, ft_path *path, uid_t *uid)
{
    int rc;
    uint64_t attr;
    rc = fxattr_get_int (state, path, "user.ftfs.owner", &attr);
    *uid = attr;
    return rc;
}
int fxattr_set_owner (ft_state *state, ft_path *path, uid_t uid)
{
    return fxattr_set_int (state, path, "user.ftfs.owner", (uint64_t) uid);
}

int fxattr_get_group (ft_state *state, ft_path *path, gid_t *gid)
{
    int rc;
    uint64_t attr;
    rc = fxattr_get_int (state, path, "user.ftfs.group", &attr);
    *gid = attr;
    return rc;
}
int fxattr_set_group (ft_state *state, ft_path *path, gid_t gid)
{
    return fxattr_set_int (state, path, "user.ftfs.group", (uint64_t) gid);
}

int fxattr_set_inh (ft_state *state, ft_path *path, uint64_t inh)
{
    return fxattr_set_int (state, path, "user.ftfs.inh", inh);
}
int fxattr_get_inh (ft_state *state, ft_path *path, uint64_t *inh)
{
    return fxattr_get_int (state, path, "user.ftfs.inh", inh);
}

int fxattr_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid)
{
    int rc;
    if ((rc = fxattr_set_owner(state, path, uid))) return rc;
    if ((rc = fxattr_set_group(state, path, gid))) return rc;
    return FT_OK;
}

int fxattr_delete (ft_state *state, ft_path *path)
{
    (void) state;
    (void) path;
    return FT_OK;
}

int fxattr_rename (ft_state *state, ft_path *path, const char* newpath)
{
    (void) state;
    (void) path;
    (void) newpath;
    return FT_OK;
}

int int_to_hex (uint64_t i, char* hex);
int hex_to_int (const char* hex, uint64_t *i);

#define FT_UNSET(name) \
    if ((rc = fxattr_unset(state, path, name))) return rc;

#define FT_SET(name,type) \
    if ((rc = fxattr_set_int(state, path, name, prm->type))) return rc;

#define FT_GET(name,type) \
    if ((rc = fxattr_get_int(state, path, name, &prm->type))) return rc;

int fxattr_set_prm (ft_state *state, ft_path *path, const ft_prm *prm)
{
    if (!(prm->allow | prm->deny)) return fxattr_unset_prm(state, path, prm);
    char name1 [FT_LIMIT_PATH], name2[FT_LIMIT_PATH], index[17];
    int rc;

    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/
                         FT_SET("user.ftfs.Aall", allow)
                         FT_SET("user.ftfs.Dall", deny )
                         return FT_OK;
        case FT_CAT_OUS: /**< Owner USer                                 **/
                         FT_SET("user.ftfs.Aous", allow)
                         FT_SET("user.ftfs.Dous", deny )
                         return FT_OK;
        case FT_CAT_OGR: /**< Owner Group                                **/
                         FT_SET("user.ftfs.Aogr", allow)
                         FT_SET("user.ftfs.Dogr", deny )
                         return FT_OK;
        case FT_CAT_OTH: /**< OTHer                                      **/
                         FT_SET("user.ftfs.Aoth", allow)
                         FT_SET("user.ftfs.Doth", deny )
                         return FT_OK;
        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name1, "user.ftfs.Auid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Duid.", index, 32);
                         FT_SET(name1, allow)
                         FT_SET(name2, deny )
                         return FT_OK;
        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name1, "user.ftfs.Agid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Dgid.", index, 32);
                         FT_SET(name1, allow)
                         FT_SET(name2, deny )
                         return FT_OK;
        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         if (!ft_str_cat2(name1, "user.ftfs.Apex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         if (!ft_str_cat2(name1, "user.ftfs.Dpex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         FT_SET(name1, allow)
                         FT_SET(name2, deny )
                         return FT_OK;
        default:         return FT_ERR_PRMCAT;
    }
}

int fxattr_get_prm (ft_state *state, ft_path *path, ft_prm *prm)
{
    char name1 [FT_LIMIT_PATH], name2[FT_LIMIT_PATH], index[17];
    int rc;

    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/
                         FT_GET("user.ftfs.Aall", allow)
                         FT_GET("user.ftfs.Dall", deny )
                         return FT_OK;
        case FT_CAT_OUS: /**< Owner USer                                 **/
                         FT_GET("user.ftfs.Aous", allow)
                         FT_GET("user.ftfs.Dous", deny )
                         return FT_OK;
        case FT_CAT_OGR: /**< Owner Group                                **/
                         FT_GET("user.ftfs.Aogr", allow)
                         FT_GET("user.ftfs.Dogr", deny )
                         return FT_OK;
        case FT_CAT_OTH: /**< OTHer                                      **/
                         FT_GET("user.ftfs.Aoth", allow)
                         FT_GET("user.ftfs.Doth", deny )
                         return FT_OK;
        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name1, "user.ftfs.Auid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Duid.", index, 32);
                         FT_GET(name1, allow)
                         FT_GET(name2, deny )
                         return FT_OK;
        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name1, "user.ftfs.Agid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Dgid.", index, 32);
                         FT_GET(name1, allow)
                         FT_GET(name2, deny )
                         return FT_OK;
        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         if (!ft_str_cat2(name1, "user.ftfs.Apex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         if (!ft_str_cat2(name1, "user.ftfs.Dpex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         FT_GET(name1, allow)
                         FT_GET(name2, deny )
                         return FT_OK;
        default:         return FT_ERR_PRMCAT;
    }

}

int fxattr_unset_prm (ft_state *state, ft_path *path, const ft_prm *prm)
{
    char name1 [FT_LIMIT_PATH], name2[FT_LIMIT_PATH], index[17];
    int rc;

    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/
                         FT_UNSET("user.ftfs.Aall")
                         FT_UNSET("user.ftfs.Dall")
                         return FT_OK;
        case FT_CAT_OUS: /**< Owner USer                                 **/
                         FT_UNSET("user.ftfs.Aous")
                         FT_UNSET("user.ftfs.Dous")
                         return FT_OK;
        case FT_CAT_OGR: /**< Owner Group                                **/
                         FT_UNSET("user.ftfs.Aogr")
                         FT_UNSET("user.ftfs.Dogr")
                         return FT_OK;
        case FT_CAT_OTH: /**< OTHer                                      **/
                         FT_UNSET("user.ftfs.Aoth")
                         FT_UNSET("user.ftfs.Doth")
                         return FT_OK;
        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name1, "user.ftfs.Auid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Duid.", index, 32);
                         FT_UNSET(name1)
                         FT_UNSET(name2)
                         return FT_OK;
        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name1, "user.ftfs.Agid.", index, 32);
                         ft_str_cat2(name2, "user.ftfs.Dgid.", index, 32);
                         FT_UNSET(name1)
                         FT_UNSET(name2)
                         return FT_OK;
        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         if (!ft_str_cat2(name1, "user.ftfs.Apex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         if (!ft_str_cat2(name1, "user.ftfs.Dpex.", prm->prc.cmd, FT_LIMIT_PATH)) return FT_ERR_PATH;
                         FT_UNSET(name1)
                         FT_UNSET(name2)
                         return FT_OK;
        default:         return FT_ERR_PRMCAT;
    }

}

#define FT_PRM_ADD                                  \
    if (fxattr_get_prm(state, path, &prm) != FT_OK) \
    {                                               \
        ft_prms_free(&prms);                        \
        return ft_prms_error();                     \
    }                                               \
    ft_prms_push (&prms, &prm);                     \
    if (ft_prms_is_error (&prms)) return prms;                                                      \

ft_prms fxattr_get_prms (ft_state *state, ft_path *path)
{
    char *xlist = NULL, *e, *id;
    ft_prm prm;
    ft_prms prms = ft_prms_init();
    ssize_t s;

    s = llistxattr(FT_HOSTPATH, xlist, 0);
    xlist = malloc (s);
    if (!xlist || llistxattr(FT_HOSTPATH, xlist, s) != s) return ft_prms_error();

    for (e = xlist + s - 14; xlist < e; xlist += strlen(xlist) + 1)
    {
        if (!memcmp(xlist, "user.ftfs.Aall", 14))
        {
            prm.cat = FT_CAT_ALL;
            FT_PRM_ADD
        }
        else if (!memcmp(xlist, "user.ftfs.Aous", 14))
        {
            prm.cat = FT_CAT_OUS;
            FT_PRM_ADD
        }
        else if (!memcmp(xlist, "user.ftfs.Aogr", 14))
        {
            prm.cat = FT_CAT_OGR;
            FT_PRM_ADD
        }
        else if (!memcmp(xlist, "user.ftfs.Aoth", 14))
        {
            prm.cat = FT_CAT_OTH;
            FT_PRM_ADD
        }
        else if (xlist + 1 < e && !memcmp(xlist, "user.ftfs.Auid.", 15))
        {
            uint64_t uid;
            id = xlist + 15;
            if (!hex_to_int(id, &uid))
            {
                ft_prms_free(&prms);
                return ft_prms_error();
            }
            prm.cat = FT_CAT_UID;
            prm.prc.uid = uid;
            FT_PRM_ADD
        }
        else if (xlist + 1 < e && !memcmp(xlist, "user.ftfs.Agid.", 15))
        {
            uint64_t gid;
            id = xlist + 15;
            if (!hex_to_int(id, &gid))
            {
                ft_prms_free(&prms);
                return ft_prms_error();
            }
            prm.prc.gid = gid;
            prm.cat = FT_CAT_GID;
            FT_PRM_ADD
        }
        else if (xlist + 1 < e && !memcmp(xlist, "user.ftfs.Apex.", 15))
        {
            prm.cat = FT_CAT_PEX;
            prm.prc.cmd = xlist + 15;
            FT_PRM_ADD
        }
    }
    return prms;

}
