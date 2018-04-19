/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include "ftfs.h"
#include "xattr.h"
#include <sys/xattr.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>
#include <attr/xattr.h>

int xattr_close (ft_state *state)
{
    (void) state;
    return FT_OK;
}

int xattr_delete (ft_state *state, ft_path *path)
{
    (void) state;
    (void) path;
    return FT_OK;
}

int xattr_rename (ft_state *state, ft_path *path, const char* newpath)
{
    (void) state;
    (void) path;
    (void) newpath;
    return FT_OK;
}


int int_to_hex (uint64_t i, char* hex)
{
    return snprintf (hex, 17, "%016" PRIX64, i) != 16;
}

int hex_to_int (const char* hex, uint64_t *i)
{
    return sscanf (hex, "%" SCNx64, i) != 1;
}

uint64_t CityHash64(const char *s);

int xattr_open (ft_state *state)
{
     char value[5];
     if (lgetxattr(state->datadir, "user.ftfs", value, 6) != 6 ||
         memcmp(value, "xattr", 6)
        )
         return FT_ERR_XATTROPEN;

     state->type = FT_XATTR;
     return FT_OK;
}

int xattr_create (const char *hostdir)
{
    char datadir[FT_LIMIT_PATH];
    if (!ft_str_cat2 (datadir, hostdir, "/data", FT_LIMIT_PATH)) return FT_ERR_PATH;
    return lsetxattr(datadir, "user.ftfs", "xattr", 6, XATTR_CREATE) ? FT_ERR_XATTRCRT : FT_OK;
}

int xattr_set (ft_state *state, ft_path *path, const char *name, const void *value, ssize_t s)
{
    return lsetxattr(FT_HOSTPATH, name, value, s, 0) ? FT_ERR_XATTRCRT : FT_OK;
}

ssize_t xattr_size (ft_state *state, ft_path *path, const char *name)
{
     char value;
     ssize_t s = lgetxattr(FT_HOSTPATH, name, &value, 0);
     return s < 0 && errno == ENOATTR ? 0 : s;
}


/*
 * value - buffer return value
 * bs    - buffer size
 * rs    - real xattr string size, including '\0' if used (rs could be NULL if not needed)
 * return ERR_XATRRFTM in case of too small buffer (rs is set accordingly to real size)
 */
int xattr_get (ft_state *state, ft_path *path, const char *name, void *value, ssize_t bs, ssize_t *rs)
{
     ssize_t s = lgetxattr(FT_HOSTPATH, name, value, 0);
     int rc;

     if (rs) *rs = s;

     if (s < 0 && errno == ENOATTR)
     {
         if (rs) *rs = 0;
         return FT_OK;
     }

     if (s > bs) return FT_ERR_XATTRFMT;

     rc = lgetxattr(FT_HOSTPATH, name, value, s);

     if (rc != s) return FT_ERR_XATTRFMT;

     return FT_OK;
}

int xattr_unset (ft_state *state, ft_path *path, const char *name)
{
    int r = lremovexattr(FT_HOSTPATH, name);
    return (r && errno != ENOATTR) ? FT_ERR_XATTRCRT : FT_OK;
}

int xattr_get_int (ft_state *state, ft_path *path, const char *name, uint64_t *attr)
{
     int rc;
     ssize_t rs;
     char value[17];

     if ((rc = xattr_get(state, path, name, value, 16, &rs))) return rc;

     if (rs == 0)
     {
         *attr = 0;
         return FT_OK;
     }

     if (rs != 16) return FT_ERR_XATTROPEN;

     value[16] = '\0';
     if (hex_to_int(value, attr)) return FT_ERR_XATTRFMT;

     return FT_OK;

}

int xattr_set_int (ft_state *state, ft_path *path, const char *name, uint64_t attr)
{
    char value[17];
    int rc;

    if (int_to_hex(attr, value)) return FT_ERR_XATTRFMT;

    if ((rc = xattr_set(state, path, name, value, 16))) return rc;

    return FT_OK;

}

int xattr_set_string (ft_state *state, ft_path *path, const char *name, const char *str)
{
    size_t s = 2 * strlen(str);
    void *value = malloc (s);
    char *o = value;
    const char *i;
    int rc;

    if (!value) return FT_ERR_MALLOC;

    for (i = str; *i; ++i, o += 2)
        snprintf (o, 3, "%02X", (unsigned int) (unsigned char) *i);

    rc = xattr_set(state, path, name, value, s);

    free(value);

    return rc;
}

/*
 * bs - buffer size including '\0'
 */
int xattr_get_string (ft_state *state, ft_path *path, const char *name, char *str, size_t bs)
{
    size_t s = 2 * (bs - 1);
    char *value = malloc (s), *i = value, *e;
    char buffer[3], *o = str;
    int rc, v;
    ssize_t rs;

    if (!value) return FT_ERR_MALLOC;

    if ((rc = xattr_get(state, path, name, value, s, &rs))) return rc;
    if (rs % 2) return FT_ERR_XATTRFMT;

    buffer[2] = '\0';
    for (e = value + rs; i < e; i += 2, ++o)
    {
        buffer[0] = *((char*) i);
        buffer[1] = *(((char*) i) + 1);

        if (sscanf (buffer, "%x", &v) != 1) return FT_ERR_XATTRFMT;

        *o = (char) v;
    }
    *o = '\0';
    return FT_OK;

}

int xattr_set_hash (ft_state *state, ft_path *path, const char *name, const void *attr, ssize_t s)
{
    char hash_name [44] = "user.ftfs.N", hash_value [44] = "user.ftfs.V";

    char value [FT_LIMIT_PATH];

    uint64_t i;
    int rc;

    char hash  [17];
    char index [17];

    if (strlen (name) >= FT_LIMIT_PATH) return FT_ERR_XATTRFMT;

    int_to_hex (CityHash64(name), hash);
    strcpy (hash_name  + 11, hash);
    strcpy (hash_value + 11, hash);

    i = 0;
    while (i < FT_LIMIT_XHASH)
    {
        int_to_hex(i, index);
        strcpy (hash_name + 27, index);

        if ((rc = xattr_get_string(state, path, hash_name, value, FT_LIMIT_PATH)) != FT_OK) return rc;

        if (!*value)
        {
            if ((rc = xattr_set_string(state, path, hash_name, name)) != FT_OK) return rc;
            strcpy (hash_value + 27, index);
            return xattr_set (state, path, hash_value, attr, s);
        }

        if (!strcmp(value, name))
        {
            strcpy (hash_value + 27, index);
            return xattr_set (state, path, hash_value, attr, s);
        }

        ++i;
    };

    return FT_ERR_XATTROPEN;

}

int xattr_get_hash (ft_state *state, ft_path *path, const char *name, char *attr, ssize_t bs, ssize_t *rs)
{
    char hash_name [44] = "user.ftfs.N", hash_value [44] = "user.ftfs.V";

    char value [FT_LIMIT_PATH];

    uint64_t i;
    int rc;

    char hash  [17];
    char index [17];

    if (strlen (name) >= FT_LIMIT_PATH) return FT_ERR_XATTRFMT;

    int_to_hex (CityHash64(name), hash);
    strcpy (hash_name  + 11, hash);
    strcpy (hash_value + 11, hash);

    i = 0;
    while (i < FT_LIMIT_XHASH)
    {
        int_to_hex(i, index);
        strcpy (hash_name + 27, index);

        if ((rc = xattr_get_string(state, path, hash_name, value, FT_LIMIT_PATH)) != FT_OK) return rc;

        if (!*value)
        {
            *rs = 0;
            *attr = '\0';

            return FT_OK;
        }

        if (strcmp(value, name) == 0)
        {
            strcpy (hash_value + 27, index);
            return xattr_get (state, path, hash_value, attr, bs, rs);
        }

        ++i;
    };

    return FT_ERR_XATTROPEN;

}

int xattr_unset_hash (ft_state *state, ft_path *path, const char *name)
{
    char hash_name [44] = "user.ftfs.N", hash_value [44] = "user.ftfs.V";

    char value [FT_LIMIT_PATH];

    uint64_t i;
    int rc;

    char hash  [17];
    char index [17];

    uint64_t pos = 0, last = 0;

    if (strlen (name) >= FT_LIMIT_PATH) return FT_ERR_XATTRFMT;

    int_to_hex (CityHash64(name), hash);
    strcpy (hash_name  + 11, hash);
    strcpy (hash_value + 11, hash);

    i = 0;
    while (i < FT_LIMIT_XHASH)
    {
        int_to_hex(i, index);
        strcpy (hash_name + 27, index);

        if ((rc = xattr_get_string(state, path, hash_name, value, FT_LIMIT_PATH)) != FT_OK) return rc;

        if (!*value) return FT_OK;

        if (!strcmp(value, name))
        {
            strcpy (hash_value + 27, index);

            pos = i++;
            last = pos;

            break;
        }

        ++i;
    };

    while (i < FT_LIMIT_XHASH)
    {
        int_to_hex(i, index);
        strcpy (hash_name + 27, index);

        if ((rc = xattr_get_string(state, path, hash_name, value, FT_LIMIT_PATH)) != FT_OK) return rc;

        if (!*value) break;

        last = i;
        ++i;

    }

    if (last != pos)
    {
        void *buffer;
        ssize_t lsize, rlsize;

        int_to_hex(last, index);
        strcpy (hash_name  + 27, index);
        strcpy (hash_value + 27, index);

        lsize = xattr_size(state, path, hash_value);
        if (lsize <= 0) return FT_ERR_XATTRFMT;
        buffer = malloc (lsize);
        if (!buffer) return FT_ERR_MALLOC;

        if ((rc = xattr_get_string(state, path, hash_name,  value, FT_LIMIT_PATH  )) != FT_OK) return rc;
        if ((rc = xattr_get       (state, path, hash_value, buffer, lsize, &rlsize)) != FT_OK) return rc;
        xattr_unset(state, path, hash_name);
        xattr_unset(state, path, hash_value);

        int_to_hex(pos, index);
        strcpy (hash_name  + 27, index);
        strcpy (hash_value + 27, index);

        if ((rc = xattr_set_string(state, path, hash_name, value           )) != FT_OK) return rc;
        if ((rc = xattr_set       (state, path, hash_value, buffer, rlsize )) != FT_OK) return rc;
    }

    return FT_OK;
}


int xattr_set_inh (ft_state *state, ft_path *path, uint64_t inh)
{
    return xattr_set_int (state, path, "user.ftfs.inh", inh);
}

int xattr_get_inh (ft_state *state, ft_path *path, uint64_t *inh)
{
    return xattr_get_int (state, path, "user.ftfs.inh", inh);
}

int xattr_set_owner (ft_state *state, ft_path *path, uid_t uid)
{
    return xattr_set_int (state, path, "user.ftfs.owner", uid);
}

int xattr_set_group (ft_state *state, ft_path *path, gid_t gid)
{
    return xattr_set_int (state, path, "user.ftfs.group", gid);
}

int xattr_get_owner (ft_state *state, ft_path *path, uid_t *uid)
{
    uint64_t attr;
    int rc = xattr_get_int (state, path, "user.ftfs.owner", &attr);
    if (rc) return rc;

    *uid = attr;

    return rc;
}

int xattr_get_group (ft_state *state, ft_path *path, uid_t *uid)
{
    uint64_t attr;
    int rc = xattr_get_int (state, path, "user.ftfs.group", &attr);
    if (rc) return rc;

    *uid = attr;

    return rc;
}

int xattr_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid)
{
    int rc;
    rc = xattr_set_owner(state, path, uid);
    if (rc) return rc;

    rc = xattr_set_group(state, path, gid);
    if (rc) return rc;

    return FT_OK;
}



int xattr_unset_prm (ft_state *state, ft_path *path, const ft_prm *prm)
{
    char name[31], index[17];
    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/
                         return xattr_unset(state, path, "user.ftfs.all");
        case FT_CAT_OUS: /**< Owner USer                                 **/
                         return xattr_unset(state, path, "user.ftfs.ous");
        case FT_CAT_OGR: /**< Owner Group                                **/
                         return xattr_unset(state, path, "user.ftfs.ogr");
        case FT_CAT_OTH: /**< OTHer                                      **/
                         return xattr_unset(state, path, "user.ftfs.oth");
        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name, "user.ftfs.uid.", index, 31);
                         return xattr_unset(state, path, name);
        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name, "user.ftfs.gid.", index, 31);
                         return xattr_unset(state, path, name);
        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         return xattr_unset_hash(state, path, prm->prc.cmd);
        default:         return FT_ERR_PRMCAT;
    }

    return FT_OK;

}


int xattr_set_prm (ft_state *state, ft_path *path, const ft_prm *prm)
{
    if (!(prm->allow | prm->deny)) return xattr_unset_prm(state, path, prm);
    char allow[17], deny [17],
         name [31], index[17],
         value[33];

    int_to_hex(prm->allow, allow);
    int_to_hex(prm->deny,  deny );

    ft_str_cat2 (value, allow, deny, 33);

    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/
                         return xattr_set(state, path, "user.ftfs.all", value, 32);
        case FT_CAT_OUS: /**< Owner USer                                 **/
                         return xattr_set(state, path, "user.ftfs.ous", value, 32);
        case FT_CAT_OGR: /**< Owner Group                                **/
                         return xattr_set(state, path, "user.ftfs.ogr", value, 32);
        case FT_CAT_OTH: /**< OTHer                                      **/
                         return xattr_set(state, path, "user.ftfs.oth", value, 32);
        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name, "user.ftfs.uid.", index, 31);
                         return xattr_set(state, path, name, value, 32);
        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name, "user.ftfs.gid.", index, 31);
                         return xattr_set(state, path, name, value, 32);
        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         return xattr_set_hash(state, path, prm->prc.cmd, value, 32);
        default:         return FT_ERR_PRMCAT;
    }
    return FT_OK;
}

int xattr_get_prm (ft_state *state, ft_path *path, ft_prm *prm)
{
    char name [31], index[17],
         value[33];
    int rc;
    ssize_t s;

    switch (prm->cat)
    {
        case FT_CAT_ALL: /**< ALL users                                  **/

                         rc = xattr_get(state, path, "user.ftfs.all", value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;
        case FT_CAT_OUS: /**< Owner USer                                 **/

                         rc = xattr_get(state, path, "user.ftfs.ous", value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;

        case FT_CAT_OGR: /**< Owner Group                                **/

                         rc = xattr_get(state, path, "user.ftfs.ogr", value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;

        case FT_CAT_OTH: /**< OTHer                                      **/

                         rc = xattr_get(state, path, "user.ftfs.oth", value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;

        case FT_CAT_UID: /**< User IDentifier                            **/
                         int_to_hex(prm->prc.uid, index);
                         ft_str_cat2(name, "user.ftfs.uid.", index, 31);

                         rc = xattr_get(state, path, name, value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;

        case FT_CAT_GID: /**< Group IDentifier                           **/
                         int_to_hex(prm->prc.gid, index);
                         ft_str_cat2(name, "user.ftfs.gid.", index, 31);

                         rc = xattr_get(state, path, name, value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;

                         break;

        case FT_CAT_PEX: /**< Process EXecutable                         **/
                         rc = xattr_get_hash(state, path, prm->prc.cmd, value, 33, &s);
                         if (rc != FT_OK || (s != 32 && s != 0)) return FT_ERR_XATTRFMT;
                         break;

        default:         return FT_ERR_PRMCAT;
    }

    if (s)
    {
        value[32] = '\0';
        hex_to_int(value + 16, &(prm->deny ));
        value[16] = '\0';
        hex_to_int(value     , &(prm->allow));
    }
    else
    {
        prm->allow = prm->deny = 0;
    }

    return FT_OK;
}

#define FT_PRM_ADD                                  \
    if (xattr_get_prm(state,path, &prm) != FT_OK)   \
    {                                               \
        ft_prms_free(&prms);                        \
        return ft_prms_error();                     \
    }                                               \
    ft_prms_push (&prms, &prm);                     \
    if (ft_prms_is_error (&prms)) return prms;                                                      \

ft_prms xattr_get_prms (ft_state *state, ft_path *path)
{
    char *xlist = NULL;
    ft_prms prms = ft_prms_init();
    ssize_t i = 0, s = llistxattr(FT_HOSTPATH, xlist, 0);

    xlist = malloc (s);
    if (!xlist || llistxattr(FT_HOSTPATH, xlist, s) != s) return ft_prms_error();

    while (i < s)
    {
        char *attr = xlist + i, *id;
        ft_prm prm;
        i += strlen (attr);
        if (!memcmp(attr, "user.ftfs.all", 13))
        {
            prm.cat = FT_CAT_ALL;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.ous", 13))
        {
            prm.cat = FT_CAT_OUS;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.ogr", 13))
        {
            prm.cat = FT_CAT_OGR;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.oth", 13))
        {
            prm.cat = FT_CAT_OTH;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.uid.", 14))
        {
            uint64_t uid;
            id = attr + 14;
            if (!hex_to_int(id, &uid))
            {
                ft_prms_free(&prms);
                return ft_prms_error();
            }
            prm.cat = FT_CAT_UID;
            prm.prc.uid = uid;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.gid.", 14))
        {
            uint64_t gid;
            id = attr + 14;
            if (!hex_to_int(id, &gid))
            {
                ft_prms_free(&prms);
                return ft_prms_error();
            }
            prm.prc.gid = gid;
            prm.cat = FT_CAT_GID;
            FT_PRM_ADD
        }
        else if (!memcmp(attr, "user.ftfs.N", 11))
        {
            char exec [FT_LIMIT_PATH];
            if (xattr_get_string (state, path, attr, exec, FT_LIMIT_PATH) != FT_OK)
            {
                ft_prms_free(&prms);
                return ft_prms_error();
            }

            prm.cat = FT_CAT_PEX;
            prm.prc.cmd = exec;
            FT_PRM_ADD
        }
    }
    return prms;
}
