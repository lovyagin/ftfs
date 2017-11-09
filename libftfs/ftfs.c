/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#define _XOPEN_SOURCE 500


#include "ftfs.h"
#include "xattr.h"
#include "fxattr.h"
#include "db.h"
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

const char ft_version[] = "1.0.0";
const char ft_name[] = "libftfs";
char ft_prg_name[FT_LIMIT_PATH];
char ft_prg_version[FT_LIMIT_PATH];

void ft_init (const char *name, const char *version)
{
    ft_str_cpy(ft_prg_name,    name,    FT_LIMIT_PATH);
    ft_str_cpy(ft_prg_version, version, FT_LIMIT_PATH);
}

int ft_open   (const char *hostdir, int check_prexec, ft_state **state)
{
    struct stat statbuf;

    if (!hostdir) return FT_ERR_HOSTERROR;

    if (!(*state = malloc(sizeof(ft_state))))         return FT_ERR_MALLOC;

    if (!ft_str_cpy ((*state)->hostdir, hostdir, FT_LIMIT_PATH)) return FT_ERR_PATH;
    if (!ft_str_cat2 ((*state)->datadir, (*state)->hostdir, "/data", FT_LIMIT_PATH)) return FT_ERR_PATH;

    if (access ((*state)->datadir, F_OK)) return FT_ERR_NODATA;

    if (stat ((*state)->hostdir, &statbuf)) return FT_ERR_HOSTERROR;

    (*state)->uid = statbuf.st_uid;
    (*state)->gid = statbuf.st_gid;

    if (db_open(*state)) { return FT_ERR_DBOPEN; }
    if (!(**state).db && fxattr_open(*state) && xattr_open(*state)) return FT_ERR_XATTROPEN;

    (**state).check_prexec = check_prexec;

    return FT_OK;

}

void ft_close (ft_state **state)
{
    switch ((*state)->type)
    {
        case FT_XATTR:  xattr_close(*state);  break;
        case FT_FXATTR: fxattr_close(*state); break;
        case FT_SQLITE: db_close(*state);     break;
    }

    free (*state);

    *state = NULL;

}

int ft_create (const char *hostdir, ft_type type)
{
    char datadir[FT_LIMIT_PATH];
    DIR* dir = opendir(hostdir);

    if (dir)
    {
        struct dirent *d;
        d = readdir(dir);
        d = readdir(dir);
        d = readdir(dir);
        closedir(dir);
        if (d) return FT_ERR_HOSTEMPTY;
        if (chmod(hostdir, 0700)) return FT_ERR_HOSTERROR;
    }
    else if (errno == ENOENT)
    {
        if (mkdir(hostdir, 0700)) return FT_ERR_HOSTERROR;
    }
    else
    {
        return FT_ERR_HOSTDIR;
    }

    if (!ft_str_cat2 (datadir, hostdir, "/data", FT_LIMIT_PATH)) return FT_ERR_PATH;
    if (mkdir(datadir, 0700)) return FT_ERR_HOSTERROR;

    switch (type)
    {
        case FT_XATTR:  return xattr_create(hostdir);
        case FT_FXATTR: return fxattr_create(hostdir);
        case FT_SQLITE: return db_create(hostdir);
        default:        return FT_ERR_TYPE;
    }

}

struct stat * ft_prestat (ft_state *state, ft_path *path)
{
    if (path->statset == UNSET)
    {
        char *host = FT_HOSTPATH;
        if (!host || lstat (host, &path->statbuf) < 0) return NULL;
        path->statset = FALSE;
        return &path->statbuf;
    }
    return &path->statbuf;
}

void swappath (ft_path **path1, ft_path **path2)
{
    ft_path *t = *path1;
    *path1     = *path2;
    *path2     = t;
}

#define FT_CPC \
    if (ft_get_prm(state, path, &p) != FT_OK) return -1; \
    if (p.deny & prm) return -1;                         \
    allow |= p.allow;
int ft_check_entry_prm (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t prm)
{
    uid_t uid; gid_t gid;
    ft_prm p;
    int allow = 0;
    const gid_t *i = prc->groups, *e = i + prc->ngroups;

    p.cat = FT_CAT_ALL; FT_CPC
    if (prc->uid != (uid_t) -1) { p.cat = FT_CAT_UID; p.prc.uid = prc->uid; FT_CPC }
    if (prc->gid != (uid_t) -1) { p.cat = FT_CAT_GID; p.prc.gid = prc->gid; FT_CPC }

    p.cat = FT_CAT_GID;
    for ( ; i < e; ++i)
    {
        p.prc.gid = *i;
        FT_CPC
    }

    if (state->check_prexec && p.prc.cmd != NULL)
    {
        p.cat = FT_CAT_PEX; p.prc.cmd = prc->cmd; FT_CPC
    }

    if (ft_get_group(state, path, &gid) != FT_OK) return -1;
    if (ft_get_owner(state, path, &uid) != FT_OK) return -1;

    if (uid == prc->uid)
    {
        p.cat = FT_CAT_OUS; FT_CPC
    }
    else if (gid == prc->gid)
    {
        p.cat = FT_CAT_OGR; FT_CPC
    }
    else
    {
        p.cat = FT_CAT_OTH; FT_CPC
    }

    return (allow & prm) == prm;
}

#define FT_APC \
    if (ft_get_prm(state, path, &p) != FT_OK) { *allow = *deny = 0; return; } \
    *deny  |= p.deny;                                                         \
    *allow |= p.allow;
void ft_get_entry_ad (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t *allow, uint64_t *deny)
{
    ft_prm p;
    const gid_t *i = prc->groups, *e = i + prc->ngroups;
    uid_t uid;
    gid_t gid;
/*
    if (prc->uid == 0 || prc->uid == state->uid)
    {
        *allow = ft_is_dir(state, path) ? FT_PRM_ALLS : FT_PRM_FILE;
        *deny  = 0;
        return;
    }
*/
    *allow = *deny = 0;

    p.cat = FT_CAT_ALL; FT_APC
    if (prc->uid != (uid_t) -1) { p.cat = FT_CAT_UID; p.prc.uid = prc->uid; FT_APC }
    if (prc->gid != (gid_t) -1) { p.cat = FT_CAT_GID; p.prc.gid = prc->gid; FT_APC }

    for ( ; i < e; ++i)
    {
        p.prc.gid = *i;
        FT_APC
    }

    if (state->check_prexec && p.prc.cmd != NULL)
    {
        p.cat = FT_CAT_PEX; p.prc.cmd = prc->cmd; FT_APC
    }

    if (!ft_get_owner(state, path, &uid) && uid == prc->uid)
    {
        p.cat = FT_CAT_OUS; FT_APC
    }
    else if (!ft_get_group(state, path, &gid) && gid  == prc->gid)
    {
        p.cat = FT_CAT_OGR; FT_APC
    }
    else
    {
        p.cat = FT_CAT_OTH; FT_APC
    }
}

int ft_check_inh_prm (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t prm)
{
    int go_fprm, go_dprm, r, q;

    char buffer1 [FT_LIMIT_PATH], buffer2 [FT_LIMIT_PATH];
    ft_path path1 = ft_path_init(buffer1), path2 = ft_path_init(buffer2),
            *curpath = &path1, *oldpath = &path2;

    if (ft_is_dir(state, path))
    {
        go_dprm = ft_check_inh (state, path, FT_INH_INH);
        go_fprm = ft_check_inh (state, path, FT_INH_IFP);
    }
    else
    {
        go_dprm = 0;
        go_fprm = ft_check_inh (state, path, FT_INH_INH);
    }

    if (!ft_str_cpy(ft_get_path(oldpath), FT_PATH, FT_LIMIT_PATH)) return ENOMEM;

    if ((r = ft_check_entry_prm(state, oldpath, prc, prm)) == -1) return 0;

    while (go_dprm && go_fprm && ft_parent(oldpath, curpath))
    {
        if (!go_fprm) prm &= ~FT_PRM_FILE;
        if (!go_dprm) prm &= ~FT_PRM_DIRS;

        if ((q = ft_check_entry_prm(state, oldpath, prc, prm)) == -1) return 0;
        r = r || q;

        go_dprm = go_dprm && ft_check_inh (state, curpath, FT_INH_INH);
        go_fprm = go_fprm && ft_check_inh (state, curpath, FT_INH_IFP);

        swappath(&oldpath, &curpath);
    }


    do
    {
        if (!go_fprm) prm &= ~FT_PRM_FILE;
        if (!go_dprm) prm &= ~FT_PRM_DIRS;

        if ((q = ft_check_entry_prm(state, oldpath, prc, prm)) == -1) return 0;
        r = r || q;

        if (!ft_parent(oldpath, curpath)) break;

        go_dprm = go_dprm && ft_check_inh (state, curpath, FT_INH_INH);
        go_fprm = go_fprm && ft_check_inh (state, curpath, FT_INH_IFP);

        swappath(&oldpath, &curpath);
    } while (go_dprm && go_fprm);

    return r;
}

int ft_check_inh_prms (ft_state *state, ft_path *path, const ft_prc *prc)
{
    int go_fprm, go_dprm;

    char buffer1 [FT_LIMIT_PATH], buffer2 [FT_LIMIT_PATH];
    ft_path path1 = ft_path_init(buffer1), path2 = ft_path_init(buffer2),
            *curpath = &path1, *oldpath = &path2;
    uint64_t allow, deny;

    if (ft_is_dir(state, path))
    {
        go_dprm = ft_check_inh (state, path, FT_INH_INH);
        go_fprm = ft_check_inh (state, path, FT_INH_IFP);
    }
    else
    {
        go_dprm = 0;
        go_fprm = ft_check_inh (state, path, FT_INH_INH);
    }

    if (!ft_str_cpy(ft_get_path(oldpath), FT_PATH, FT_LIMIT_PATH)) return ENOMEM;

    ft_get_entry_ad(state, oldpath, prc, &allow, &deny);

    while (go_dprm && go_fprm && ft_parent(oldpath, curpath))
    {
        uint64_t a, d;
        ft_get_entry_ad(state, curpath, prc, &a, &d);
        if (go_fprm)
        {
            allow |= a & FT_PRM_FILE;
            deny  |= d & FT_PRM_FILE;
        }
        if (go_dprm)
        {
            allow |= a & FT_PRM_DIRS;
            deny  |= d & FT_PRM_DIRS;
        }

        go_dprm = go_dprm && ft_check_inh (state, curpath, FT_INH_INH);
        go_fprm = go_fprm && ft_check_inh (state, curpath, FT_INH_IFP);

        swappath(&oldpath, &curpath);
    }

    return allow & ~deny;
}


int ft_check_dex (ft_state *state, ft_path *path, const ft_prc *prc)
{
    char buffer1 [FT_LIMIT_PATH], buffer2 [FT_LIMIT_PATH];
    ft_path path1 = ft_path_init(buffer1), path2 = ft_path_init(buffer2),
            *oldpath = &path1, *curpath = &path2;

    if (!ft_exists(state, path)) return 0;

    if (!ft_str_cpy(ft_get_path(curpath), FT_PATH, FT_LIMIT_PATH)) return 0;

    do
    {
        if (!ft_check_inh_prm (state, curpath, prc, FT_PRM_DEX)) return 0;

        swappath(&oldpath, &curpath);
    }  while (ft_parent(oldpath, curpath));

    return 1;
}

int ft_check_prm (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t prm)
{
    if (!ft_exists(state, path)) return 0;
    return prc->uid == 0 ||
           prc->uid == state->uid ||
           (ft_check_dex(state, path, prc) && ft_check_inh_prm(state, path, prc, prm));
}

uint64_t ft_check_prms (ft_state *state, ft_path *path, const ft_prc *prc, int susr)
{
    if (!ft_exists(state, path)) return 0;

    if (susr && (prc->uid == 0 || prc->uid == state->uid)) return FT_PRM_ALLS;

    if (!ft_check_dex(state, path, prc)) return 0;

    return ft_check_inh_prms(state, path, prc);
}

typedef enum {OWNER, GROUP, OTHER} ugo_t;

int ft_adjust_mode_inh (ft_state *state, ft_path *path, const ft_prc *prc, ugo_t ugo)
{
    uint64_t rp = ft_is_dir(state, path) ? FT_PRM_DREA : FT_PRM_FREA,
             wp = ft_is_dir(state, path) ? FT_PRM_DWRI : FT_PRM_FWRI,
             xp = ft_is_dir(state, path) ? FT_PRM_DEXE : FT_PRM_FEXE,
             r, w, x,
             prm;

    if (!ft_exists(state, path) || !ft_check_dex(state, path, prc))
    {
        switch (ugo)
        {
            case OWNER:
                path->statbuf.st_mode &= ~S_IRUSR;
                path->statbuf.st_mode &= ~S_IWUSR;
                path->statbuf.st_mode &= ~S_IXUSR;
                break;
            case GROUP:
                path->statbuf.st_mode &= ~S_IRGRP;
                path->statbuf.st_mode &= ~S_IWGRP;
                path->statbuf.st_mode &= ~S_IXGRP;
                break;
            case OTHER:
                path->statbuf.st_mode &= ~S_IROTH;
                path->statbuf.st_mode &= ~S_IWOTH;
                path->statbuf.st_mode &= ~S_IXOTH;
                break;
        }

        return 0;
    }

    prm = ft_check_prms(state, path, prc, 0);

    r = prm & rp;
    w = prm & wp;
    x = prm & xp;

    switch (ugo)
    {
        case OWNER:
             r ? (path->statbuf.st_mode |= S_IRUSR) : (path->statbuf.st_mode &= ~S_IRUSR);
             w ? (path->statbuf.st_mode |= S_IWUSR) : (path->statbuf.st_mode &= ~S_IWUSR);
             x ? (path->statbuf.st_mode |= S_IXUSR) : (path->statbuf.st_mode &= ~S_IXUSR);
             break;
        case GROUP:
             r ? (path->statbuf.st_mode |= S_IRGRP) : (path->statbuf.st_mode &= ~S_IRGRP);
             w ? (path->statbuf.st_mode |= S_IWGRP) : (path->statbuf.st_mode &= ~S_IWGRP);
             x ? (path->statbuf.st_mode |= S_IXGRP) : (path->statbuf.st_mode &= ~S_IXGRP);
             break;
        case OTHER:
             r ? (path->statbuf.st_mode |= S_IROTH) : (path->statbuf.st_mode &= ~S_IROTH);
             w ? (path->statbuf.st_mode |= S_IWOTH) : (path->statbuf.st_mode &= ~S_IWOTH);
             x ? (path->statbuf.st_mode |= S_IXOTH) : (path->statbuf.st_mode &= ~S_IXOTH);
             break;
    }

    return 0;
}

int ft_adjust_mode (ft_state *state, ft_path *path, const ft_prc *prc)
{
    int rc;

    ft_prc uprc, gprc, oprc;

    if (path->statbuf.st_uid == prc->uid)
    {
        uprc.uid = prc->uid;
        uprc.gid = prc->gid;
        uprc.cmd = prc->cmd;
        uprc.ngroups = prc->ngroups;
        memcpy (uprc.groups, prc->groups, sizeof(*uprc.groups) * uprc.ngroups);

        gprc.uid = -1;
        gprc.gid = path->statbuf.st_gid;
        gprc.cmd = NULL;
        gprc.ngroups = 0;

        oprc.uid = -1;
        oprc.gid = -1;
        oprc.cmd = NULL;
        oprc.ngroups = 0;
    }
    else
    {
        uprc.uid = path->statbuf.st_uid;
        uprc.gid = -1;
        uprc.cmd = NULL;
        uprc.ngroups = 0;

        if (path->statbuf.st_gid == prc->gid)
        {
            gprc.uid = prc->uid;
            gprc.gid = prc->gid;
            gprc.cmd = prc->cmd;
            gprc.ngroups = prc->ngroups;
            memcpy (gprc.groups, prc->groups, sizeof(*gprc.groups) * gprc.ngroups);

            oprc.uid = -1;
            oprc.gid = -1;
            oprc.cmd = NULL;
            oprc.ngroups = 0;
        }
        else
        {
            gprc.uid = -1;
            gprc.gid = path->statbuf.st_gid;
            gprc.cmd = NULL;
            gprc.ngroups = 0;

            oprc.uid = prc->uid;
            oprc.gid = prc->gid;
            oprc.cmd = prc->cmd;
            oprc.ngroups = prc->ngroups;
            memcpy (oprc.groups, prc->groups, sizeof(*oprc.groups) * oprc.ngroups);
        }
    }

    if ((rc = ft_adjust_mode_inh (state, path, &uprc, OWNER))) return rc;
    if ((rc = ft_adjust_mode_inh (state, path, &gprc, GROUP))) return rc;
    if ((rc = ft_adjust_mode_inh (state, path, &oprc, OTHER))) return rc;

    if (path->statbuf.st_uid == state->uid)
    {
        path->statbuf.st_mode |= S_IRUSR;
        path->statbuf.st_mode |= S_IWUSR;
        if (S_ISDIR(path->statbuf.st_mode)) path->statbuf.st_mode |= S_IXUSR;
    }
    else if (prc->uid == state->uid)
    {
        if (prc->gid == path->statbuf.st_gid)
        {
            path->statbuf.st_mode |= S_IRGRP;
            path->statbuf.st_mode |= S_IWGRP;
            if (S_ISDIR(path->statbuf.st_mode)) path->statbuf.st_mode |= S_IXGRP;
        }
        else
        {
            path->statbuf.st_mode |= S_IROTH;
            path->statbuf.st_mode |= S_IWOTH;
            if (S_ISDIR(path->statbuf.st_mode)) path->statbuf.st_mode |= S_IXOTH;
        }
    }

    return 0;

}

struct stat * ft_stat (ft_state *state, ft_path *path, const ft_prc *prc)
{
    if (!ft_prestat(state, path)) return NULL;

    if (path->statset != TRUE)
    {
        if (ft_get_owner(state, path, &path->statbuf.st_uid)) {errno = EACCES; return NULL; }
        if (ft_get_group(state, path, &path->statbuf.st_gid)) {errno = EACCES; return NULL; }
        if ((errno = ft_adjust_mode(state, path, prc))) return NULL;

        path->statset = TRUE;
    }

    return &path->statbuf;
}

int ft_check_prm2 (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t fprm, uint64_t dprm)
{
    if (!ft_exists(state, path)) return 0;

    return  ft_check_prm(state, path, prc, ft_is_dir (state, path) ? dprm : fprm);
}

int ft_check_inh (ft_state *state, ft_path *path, uint64_t inh)
{
    uint64_t cinh;
    if (ft_get_inh (state, path, &cinh)) return 0;
    return (cinh & inh) == inh;
}

#define CALL(name,par)                                               \
    int rc;                                                          \
    switch (state->type)                                             \
    {                                                                \
        case FT_XATTR:  rc = xattr_##name(state, path, par);  break; \
        case FT_FXATTR: rc = fxattr_##name(state, path, par); break; \
        case FT_SQLITE: rc = db_##name(state, path, par);     break; \
        default:        rc = FT_ERR_TYPE;                            \
    }

#define RETCALL(name,par)                                       \
    switch (state->type)                                        \
    {                                                           \
        case FT_XATTR:  return xattr_##name(state, path, par);  \
        case FT_FXATTR: return fxattr_##name(state, path, par); \
        case FT_SQLITE: return db_##name(state, path, par);     \
        default:        return FT_ERR_TYPE;                     \
    }

#define GETCALL(name,par)         \
    if (!path->par##set)          \
    {                             \
        CALL(name, &(path->par)); \
        if (rc) return rc;        \
        path->par##set = 1;       \
    }                             \
    *par = path->par;             \
    return FT_OK;                 \

#define SETCALL(name,par) \
    path->par##set = 0;   \
    RETCALL(name, par);


int ft_set_prm (ft_state *state, ft_path *path, const ft_prm *prm)
{
    RETCALL(set_prm, prm);
}

int ft_get_prm (ft_state *state, ft_path *path, ft_prm *prm)
{
    RETCALL(get_prm, prm);
}

int ft_unset_prm (ft_state *state, ft_path *path, ft_prm *prm)
{
    RETCALL(unset_prm, prm);
}

int ft_get_inh (ft_state *state, ft_path *path, uint64_t *inh)
{
    GETCALL(get_inh, inh);
}

int ft_set_inh (ft_state *state, ft_path *path, uint64_t inh)
{
    SETCALL(set_inh, inh);
}

int ft_get_owner (ft_state *state, ft_path *path, uid_t *uid)
{
    GETCALL(get_owner, uid);
}

int ft_set_owner (ft_state *state, ft_path *path, uid_t uid)
{
    path->statset = FALSE;
    SETCALL(set_owner, uid);
}

int ft_get_group (ft_state *state, ft_path *path, gid_t *gid)
{
    GETCALL(get_group, gid);
}

int ft_set_group (ft_state *state, ft_path *path, gid_t gid)
{
    path->statset = FALSE;
    SETCALL(set_group, gid);
}

#define CLEARPATH(path)         \
    *((path)->hostpath) = '\0'; \
    (path)->is_dir    = UNSET;  \
    (path)->exists    = UNSET;  \
    (path)->inhset    = 0;      \
    (path)->uidset    = 0;      \
    (path)->gidset    = 0;      \
    (path)->statset   = UNSET;


int ft_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid)
{
    CLEARPATH(path);
    switch (state->type)
    {
        case FT_XATTR:  return xattr_add(state, path, uid, gid);
        case FT_FXATTR: return fxattr_add(state, path, uid, gid);
        case FT_SQLITE: return db_add(state, path, uid, gid);
        default:        return FT_ERR_TYPE;
    }
}

int ft_delete (ft_state *state, ft_path *path)
{
    CLEARPATH(path);
    switch (state->type)
    {
        case FT_XATTR:  return xattr_delete(state, path);
        case FT_FXATTR: return fxattr_delete(state, path);
        case FT_SQLITE: return db_delete(state, path);
        default:        return FT_ERR_TYPE;
    }
}

int ft_rename (ft_state *state, ft_path *path, const char* newpath)
{
    CLEARPATH(path);
    RETCALL(rename, newpath);
}

ft_prms ft_get_prms (ft_state *state, ft_path *path)
{
    switch (state->type)
    {
        case FT_XATTR:  return xattr_get_prms(state,path);
        case FT_FXATTR: return fxattr_get_prms(state,path);
        case FT_SQLITE: return db_get_prms(state,path);
        default:        return ft_prms_error();
    }
}

ft_prms ft_get_inhprms (ft_state *state, ft_path *path)
{
    ft_prms r = ft_prms_init();

    int go_fprm, go_dprm;

    char buffer1 [FT_LIMIT_PATH], buffer2 [FT_LIMIT_PATH];
    ft_path path1 = ft_path_init(buffer1), path2 = ft_path_init(buffer2),
            *curpath = &path1, *oldpath = &path2;

    if (ft_is_dir(state, path))
    {
        go_dprm = ft_check_inh (state, path, FT_INH_INH);
        go_fprm = ft_check_inh (state, path, FT_INH_IFP);
    }
    else
    {
        go_dprm = 0;
        go_fprm = ft_check_inh (state, path, FT_INH_INH);
    }

    if (!ft_str_cpy(ft_get_path(oldpath), FT_PATH, FT_LIMIT_PATH)) return ft_prms_error();

    do
    {
        size_t i, s;
        ft_prms q;

        if (!ft_parent(oldpath, curpath)) break;

        q = ft_get_prms(state, curpath);
        if (ft_prms_is_error(&q)) return q;

        for (i = 0, s = ft_prms_size(&q); i < s; ++i)
        {
            ft_prm *p = ft_prms_element(&q, i);
            if (!go_fprm) {p->allow &= ~FT_PRM_FILE; p->deny &= ~FT_PRM_FILE;}
            if (!go_dprm) {p->allow &= ~FT_PRM_DIRS; p->deny &= ~FT_PRM_DIRS;}
            if (p->allow || p->deny) ft_prms_add(&r, p);
            if (ft_prms_is_error(&r)) return r;
        }
        ft_prms_free(&q);

        go_dprm = go_dprm && ft_check_inh (state, curpath, FT_INH_INH);
        go_fprm = go_fprm && ft_check_inh (state, curpath, FT_INH_IFP);

        swappath(&oldpath, &curpath);
    } while (go_dprm || go_fprm);

    return r;
}

ft_prms ft_get_allprms (ft_state *state, ft_path *path)
{
    ft_prms oprms = ft_get_prms   (state, path),
            iprms = ft_get_inhprms(state, path);

    ft_prms_join(&oprms, &iprms);

    ft_prms_free(&iprms);

    return oprms;
}

int ft_copy_file_prm (ft_state *state, ft_path *path, ft_path *parent)
{
    size_t i;
    int rc;
    ft_prms prms = ft_get_allprms(state, parent);

    if (ft_prms_is_error(&prms)) return FT_ERR_NOPARENT;

    for (i = 0; i < ft_prms_size(&prms); ++i)
    {
        ft_prm *prm = ft_prms_element(&prms, i);
        prm->allow &= FT_PRM_FILE;
        prm->deny  &= FT_PRM_FILE;
        if ((prm->allow || prm->deny) && (rc = ft_set_prm(state, path, prm))) return rc;
    }
    return FT_OK;
}

int ft_copy_dir_prm (ft_state *state, ft_path *path, ft_path *parent)
{
    size_t i;
    int rc;
    ft_prms prms = ft_get_allprms(state, parent);

    if (ft_prms_is_error(&prms)) return FT_ERR_NOPARENT;

    for (i = 0; i < ft_prms_size(&prms); ++i)
    {
        ft_prm *prm = ft_prms_element(&prms, i);
        prm->allow &= FT_PRM_DIRS;
        prm->deny  &= FT_PRM_DIRS;
        if ((prm->allow || prm->deny) && (rc = ft_set_prm(state, path, prm))) return rc;
    }
    return FT_OK;
}

int ft_copy_prm (ft_state *state, ft_path *path, ft_path *parent)
{
    size_t i;
    int rc;
    ft_prms prms = ft_get_allprms(state, parent);

    if (ft_prms_is_error(&prms)) return FT_ERR_NOPARENT;

    for (i = 0; i < ft_prms_size(&prms); ++i)
    {
        ft_prm *prm = ft_prms_element(&prms, i);
        if ((prm->allow || prm->deny) && (rc = ft_set_prm(state, path, prm))) return rc;
    }
    return FT_OK;
}

int ft_set_mkfile_prm (ft_state *state, ft_path *path)
{
    char pbuffer[FT_LIMIT_PATH];
    ft_path parent = ft_path_init(pbuffer);
    int rc;
    uint64_t inh = 0, pinh = 0;

    if (!ft_parent(path, &parent)) return FT_ERR_NOPARENT;

    if ((rc = ft_get_inh(state, &parent, &pinh))) return rc;

    if (pinh & FT_INH_SPI) inh |= FT_INH_INH;
    if (pinh & FT_INH_SPS) inh |= FT_INH_SET;

    if ((rc = ft_set_inh(state, path, inh))) return rc;

    if (pinh & FT_INH_CPR) return ft_copy_file_prm(state, path, &parent);

    return FT_OK;
}

int ft_set_mkdir_prm (ft_state *state, ft_path *path)
{
    char buffer[FT_LIMIT_PATH];
    ft_path parent = ft_path_init(buffer);
    int rc;
    uint64_t inh = 0, pinh = 0;

    if (!ft_parent(path, &parent)) return FT_ERR_NOPARENT;

    if ((rc = ft_get_inh(state, &parent, &pinh))) return rc;

    if (pinh & FT_INH_SPI) inh |= FT_INH_INH;
    if (pinh & FT_INH_SPS) inh |= FT_INH_SET;

    if (pinh & FT_INH_SFP) inh |= FT_INH_IFP;
    if (pinh & FT_INH_SFS) inh |= FT_INH_IFS;

    if ((rc = ft_set_inh(state, path, inh))) return rc;

    if ((pinh & FT_INH_CPR) && (pinh & FT_INH_CFP))
        return ft_copy_prm (state, path, &parent);
    else if (pinh & FT_INH_CPR)
        return ft_copy_dir_prm (state, path, &parent);
    else if (pinh & FT_INH_CFP)
        return ft_copy_file_prm (state, path, &parent);
    else
        return FT_OK;
}

int ft_unset_inh (ft_state *state, ft_path *path)
{
    uint64_t inh;
    int rc;
    char buffer[FT_LIMIT_PATH];
    ft_path parent = ft_path_init(buffer);

    if ((rc = ft_get_inh(state, path, &inh))) return rc;

    if (!(inh & FT_INH_INH)) return FT_OK;

    inh &= ~FT_INH_INH;

    if ((rc = ft_set_inh(state, path, inh))) return rc;

    if (!(inh & FT_INH_SET)) return FT_OK;

    if (!ft_parent(path, &parent)) return FT_OK;

    if (ft_is_dir(state, path))
        return ft_copy_dir_prm(state, path, &parent);
    else
        return ft_copy_file_prm(state, path, &parent);
}

int ft_unset_ifp (ft_state *state, ft_path *path)
{
    uint64_t inh;
    int rc;
    char buffer[FT_LIMIT_PATH];
    ft_path parent = ft_path_init(buffer);

    if ((rc = ft_get_inh(state, path, &inh))) return rc;

    if (!(inh & FT_INH_IFP)) return FT_OK;

    inh &= ~FT_INH_IFP;

    if ((rc = ft_set_inh(state, path, inh))) return rc;

    if (!(inh & FT_INH_IFS)) return FT_OK;

    if (!ft_parent(path, &parent)) return FT_OK;

    if (ft_is_dir(state, path))
        return ft_copy_file_prm(state, path, &parent);
    else
        return FT_OK;

}

ft_path ft_path_init(char *path)
{
    ft_path r;
    r.path = path;
    CLEARPATH(&r);
    return r;
}

char *ft_get_path (ft_path *path)
{
    return path->path;
}

char *ft_get_hostpath (ft_state *state, ft_path *path)
{
    if (!path->hostpath[0])
        if (!ft_str_cat2 (path->hostpath, state->datadir, path->path, FT_LIMIT_PATH)) return NULL;
    return path->hostpath;
}

int ft_parent(ft_path *path, ft_path *parent)
{
    if (path->path[0] == '/' && !(path->path[1])) return 0;
    ft_str_cpy (parent->path, path->path, FT_LIMIT_PATH);
    dirname(parent->path);
    CLEARPATH(parent);
    return 1;
}

int ft_exists (ft_state *state, ft_path *path)
{
    if (path->exists == UNSET)
    {
        char *host = FT_HOSTPATH;
        struct stat buf;
        path->exists = host && lstat (host, &buf) == 0;
    }
    return path->exists;
}

int ft_is_dir (ft_state *state, ft_path* path)
{
    if (path->is_dir == UNSET)
    {
        struct stat *stat = ft_prestat (state, path);
        if (stat)
            path->is_dir = S_ISDIR(stat->st_mode);
        else
            path->is_dir = FALSE;
    }
    return path->is_dir;
}
