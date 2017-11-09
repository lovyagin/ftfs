/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.

  This code is based on bbfs fuse-tutorial code
  Copyright (C) 2012 Joseph J. Pfeiffer, Jr., Ph.D. <pfeiffer@cs.nmsu.edu>
*/
#include <ftfs.h>
#include "fusecfg.h"
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <sys/time.h>
#include <attr/xattr.h>

#define FTFS_INIT                                                                  \
    int rc;                                                                        \
    ft_prc ftprc, *prc = &ftprc;                                                   \
    ft_state *state = ((ft_state *) fuse_get_context()->private_data);             \
    ft_path ftpath = ft_path_init((char *) rpath), *path = &ftpath;                \
    char pex [FT_LIMIT_PATH];                                                      \
    char *hostpath;                                                                \
    (void) hostpath;                                                               \
    (void) rc;                                                                     \
                                                                                   \
    ftprc.uid = fuse_get_context()->uid;                                           \
    ftprc.gid = fuse_get_context()->gid;                                           \
    ftprc.cmd = NULL;                                                              \
                                                                                   \
    if (state->check_prexec)                                                       \
    {                                                                              \
        char exe [FT_LIMIT_PATH];                                                  \
        ssize_t rc;                                                                \
        snprintf (exe, FT_LIMIT_PATH, "/proc/%u/exe", fuse_get_context()->pid);    \
        if ((rc = readlink(exe, pex, FT_LIMIT_PATH) > 0) && rc < FT_LIMIT_PATH -1) \
        {                                                                          \
            pex[rc] = '\0';                                                        \
            ftprc.cmd = pex;                                                       \
        }                                                                          \
    }                                                                              \
                                                                                   \
    ftprc.ngroups = fuse_getgroups(FT_LIMIT_GROUPS, ftprc.groups);                 \
    if (ftprc.ngroups < 0 || ftprc.ngroups > FT_LIMIT_GROUPS) ftprc.ngroups = 0;

#define FTFS_INIT_PARENT                                                           \
    char pbuffer[FT_LIMIT_PATH];                                                   \
    ft_path ftparent = ft_path_init(pbuffer), *parent = &ftparent;                 \
    FTFS_INIT                                                                      \
    if (!ft_parent(path, parent)) parent = NULL;                                   \
    if (!ft_is_dir(state, parent)) return -ENOTDIR;


#define FTFS_EXISTS if (!ft_exists(state, path)) return -ENOENT;
#define FTFS_PARENT_PRM(prm) if (!ft_check_prm(state, parent, prc, FT_PRM_##prm)) return -EACCES;
#define FTFS_PRM(prm) if (!ft_check_prm(state, path, prc, FT_PRM_##prm)) return -EACCES;
#define FTFS_PRM2(prm) if (!ft_check_prm2(state, path, prc, FT_PRM_F##prm, FT_PRM_D##prm)) return -EACCES;

#define FTFS_CALL(call) if ((rc = call) < 0) rc = -errno;

#define FTFS_RETCALL(call) return (call) < 0 ? -errno : 0;

#define FTFS_FAILCALL(call) if ((rc = call) < 0) return -errno;

#define FTFS_HOSTPATH hostpath = FT_HOSTPATH; if (!hostpath) return -EFAULT;

int ftfs_getattr (const char *rpath, struct stat *statbuf)
{
    struct stat *stat;

    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM2(RA)

    stat = ft_stat(state, path, prc);

    if (!stat) return -errno;
    *statbuf = *stat;

    return 0;
}

int ftfs_readlink (const char *rpath, char *buffer, size_t s)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM(FSL)
    FTFS_HOSTPATH

    FTFS_RETCALL(readlink(hostpath, buffer, s - 1));
}


int ftfs_chown (const char *rpath, uid_t uid, gid_t gid);
#define CHECK_OWN(mk,ad)                   \
    if      (pprm & FT_PRM_D##mk) own = 1; \
    else if (pprm & FT_PRM_D##ad) own = 0; \
    else return -EACCES;

#define ADD(addf,rmf)                            \
    if (own)                                     \
    {                                            \
        uid = prc->uid;                          \
        gid = prc->gid;                          \
    }                                            \
    else                                         \
    {                                            \
        if (ft_get_owner(state, parent, &uid))   \
        {                                        \
            if (!source) rmf (hostpath);         \
            return -EACCES;                      \
        }                                        \
        if (ft_get_group(state, parent, &gid))   \
        {                                        \
            if (!source) rmf (hostpath);         \
            return -EACCES;                      \
        }                                        \
    }                                            \
                                                 \
    if (ft_add (state, path, uid, gid))          \
    {                                            \
        if (!source) rmf (hostpath);             \
        return -EACCES;                          \
    }                                            \
                                                 \
    if (ft_set_##addf##_prm (state, path))       \
    {                                            \
        if (!source) rmf (hostpath);             \
        ft_delete (state, path);                 \
        return -EACCES;                          \
    }                                            \

#define ADD_FILE ADD(mkfile, unlink)
#define ADD_DIR  ADD(mkdir,  rmdir )

#define ADD_ORD_PRM_SINGLE(type,df)                                 \
    prm->allow = prm->deny = 0;                                     \
    if (S_IR##type & stat->st_mode) prm->allow |= FT_PRM_##df##REA; \
    if (S_IW##type & stat->st_mode) prm->allow |= FT_PRM_##df##WRI; \
    if (S_IX##type & stat->st_mode) prm->allow |= FT_PRM_##df##EXE; \
    ft_set_prm(state, path, prm);                                   \


#define ADD_ORD_PRM(df)                                \
    if (!ft_check_inh(state, parent, FT_INH_SPI) &&    \
        !ft_check_inh(state, parent, FT_INH_CPR)       \
       )                                               \
    {                                                  \
        ft_prm ftprm, *prm = &ftprm;                   \
                                                       \
        prm->cat   = FT_CAT_UID;                       \
        prm->prc.uid = prc->uid;                       \
        ADD_ORD_PRM_SINGLE(USR,df)                     \
                                                       \
        prm->cat   = FT_CAT_GID;                       \
        prm->prc.gid = prc->gid;                       \
        ADD_ORD_PRM_SINGLE(GRP,df)                     \
                                                       \
        prm->cat   = FT_CAT_OUS;                       \
        prm->allow = prm->deny = 0;                    \
        ADD_ORD_PRM_SINGLE(USR,df)                     \
                                                       \
        prm->cat   = FT_CAT_OGR;                       \
        ADD_ORD_PRM_SINGLE(GRP,df)                     \
                                                       \
        prm->cat   = FT_CAT_OTH;                       \
        ADD_ORD_PRM_SINGLE(OTH,df)                     \
    }

#define ADD_ORD_PRM_SL                                 \
    if (!ft_check_inh(state, parent, FT_INH_SPI) &&    \
        !ft_check_inh(state, parent, FT_INH_CPR)       \
       )                                               \
    {                                                  \
        ft_prm ftprm, *prm = &ftprm;                   \
                                                       \
        prm->cat   = FT_CAT_UID;                       \
        prm->prc.uid = prc->uid;                       \
        prm->deny = 0;                                 \
        prm->allow = FT_PRM_FILE;                      \
                                                       \
        prm->cat   = FT_CAT_GID;                       \
        prm->prc.gid = prc->gid;                       \
        prm->deny = 0;                                 \
        prm->allow = FT_PRM_FILE;                      \
                                                       \
        prm->cat   = FT_CAT_OUS;                       \
        prm->allow = prm->deny = 0;                    \
        prm->deny = 0;                                 \
        prm->allow = FT_PRM_FILE;                      \
                                                       \
        prm->cat   = FT_CAT_OGR;                       \
        prm->deny = 0;                                 \
        prm->allow = FT_PRM_FILE;                      \
                                                       \
        prm->cat   = FT_CAT_OTH;                       \
        prm->deny = 0;                                 \
        prm->allow = FT_PRM_FILE;                      \
    }

#define CHOWNMOD                                                      \
    if (source)                                                       \
    {                                                                 \
        uint64_t inh, newinh;                                         \
        ft_prms prms = ft_get_prms (state, source);                   \
        ftfs_chown (rpath, uid, gid);                                 \
        if (ft_prms_is_error(&prms)) return -EFAULT;                  \
        if (ft_get_inh (state, path,   &newinh)) return -EFAULT;      \
        if (ft_get_inh (state, source, &inh)) return -EFAULT;         \
                                                                      \
        if (inh != newinh)                                            \
        {                                                             \
            int ci = ft_check_prm2(state, path, prc,                  \
                                   FT_PRM_FCI, FT_PRM_DCI             \
                                  ),                                  \
                ct = ft_is_dir(state, path) &&                        \
                     ft_check_prm(state, path, prc, FT_PRM_DCT);      \
            if (!ci)                                                  \
                inh = (inh & ~FT_INH_INHS) | (newinh & FT_INH_INHS);  \
            if (!ct)                                                  \
                inh = (inh & ~FT_INH_TRMS) | (newinh & FT_INH_TRMS);  \
                                                                      \
            if (inh != newinh)                                        \
            {                                                         \
                ft_set_inh(state, path, inh);                         \
                                                                      \
                if (!(inh & FT_INH_INH) && (newinh & FT_INH_INH))     \
                    ft_unset_inh (state, path);                       \
                if (!(inh & FT_INH_IFP) && (newinh & FT_INH_IFP))     \
                    ft_unset_ifp (state, path);                       \
            }                                                         \
        }                                                             \
        if (ft_check_prm2(state, path, prc, FT_PRM_FCP, FT_PRM_DCP))  \
        {                                                             \
            size_t i, s = ft_prms_size(&prms);                        \
            for (i = 0; i < s; ++i)                                   \
            {                                                         \
                ft_prm prm = *ft_prms_get(&prms, i);                  \
                ft_get_prm(state, path, &prm);                        \
                prm.allow |= ft_prms_get(&prms, i)->allow;            \
                prm.deny  |= ft_prms_get(&prms, i)->deny;             \
                ft_set_prm(state, path, &prm);                        \
            }                                                         \
        }                                                             \
    }

int ftfs_add(const char  *rpath,
             struct stat *stat,
             ft_path     *source,
             const char  *target
            )
{
    int own;
    uint64_t pprm;
    uid_t uid;
    gid_t gid;
    mode_t hostmode = stat->st_mode;
    FTFS_INIT_PARENT
    FTFS_HOSTPATH
    if (!parent) return -EACCES;
    pprm = ft_check_prms(state, parent, prc, 1);

    hostmode &= ~(S_IRWXG | S_IRWXO);
    hostmode |= S_IRWXU;

    if (S_ISREG(stat->st_mode))
    {
        int fd;
        CHECK_OWN(MK,AF)

        if (!source)
        {
            fd = open(hostpath, O_CREAT | O_EXCL | O_WRONLY, hostmode);
            if (fd < 0) return -errno;
            close(fd);
        }

        ADD_FILE
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISCHR(stat->st_mode))
    {
        CHECK_OWN(CH,AC)
        if (!source) FTFS_FAILCALL(mknod(hostpath, hostmode, stat->st_dev))
        ADD_FILE
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISBLK(stat->st_mode))
    {
        CHECK_OWN(BL,AB)
        if (!source) FTFS_FAILCALL(mknod(hostpath, hostmode, stat->st_dev))
        ADD_FILE
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISFIFO(stat->st_mode))
    {
        CHECK_OWN(FF,AP)
        if (!source) FTFS_FAILCALL(mkfifo(hostpath, hostmode))
        ADD_FILE
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISSOCK(stat->st_mode))
    {
        CHECK_OWN(SC,AO)
        if (!source) FTFS_FAILCALL(mknod(hostpath, hostmode, stat->st_dev))
        ADD_FILE
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISDIR(stat->st_mode))
    {
        CHECK_OWN(MD,AD)
        if (!source) FTFS_FAILCALL(mkdir(hostpath, hostmode));
        ADD_DIR
        ADD_ORD_PRM(F)
        CHOWNMOD
    }
    else if (S_ISLNK(stat->st_mode))
    {
        CHECK_OWN(SL,AL)
        if (!source) FTFS_FAILCALL(symlink(target, hostpath))
        ADD_FILE
        ADD_ORD_PRM_SL
        CHOWNMOD
    }
    else return -EACCES;

    return 0;
}

int ftfs_mknod (const char *rpath, mode_t mode, dev_t dev)
{
    struct stat stat;

    stat.st_mode = mode;
    stat.st_dev  = dev;

    return ftfs_add(rpath, &stat, NULL, NULL);
}

int ftfs_mkdir (const char *rpath, mode_t mode)
{
    struct stat stat;

    stat.st_mode = mode | S_IFDIR;

    return ftfs_add(rpath, &stat, NULL, NULL);
}

int ftfs_unlink (const char *rpath)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM(FRM)
    FTFS_HOSTPATH

    FTFS_FAILCALL(unlink(hostpath));
    ft_delete(state, path);

    return 0;
}

int ftfs_rmdir (const char *rpath)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM(DRM)
    FTFS_HOSTPATH

    FTFS_FAILCALL(rmdir(hostpath));
    ft_delete(state, path);

    return 0;
}

int ftfs_symlink (const char *target, const char *rpath)
{
    struct stat stat;

    stat.st_mode = S_IFLNK;
    return ftfs_add(rpath, &stat, NULL, target);
}

int ftfs_rename (const char *rpath, const char *newrpath)
{
    struct stat *stat;
    ft_path ftnewpath = ft_path_init((char *) newrpath), *newpath = &ftnewpath;
    char newpbuffer[FT_LIMIT_PATH];
    ft_path ftnewparent = ft_path_init(newpbuffer), *newparent = &ftnewparent;
    char *newhostpath;
    FTFS_INIT_PARENT
    FTFS_HOSTPATH
    FTFS_EXISTS
    if (!ft_parent(path, newparent)) parent = NULL;
    if (!ft_is_dir(state, newparent)) return -ENOTDIR;
    newhostpath = ft_get_hostpath(state, newpath);

    if (!strcmp(ft_get_hostpath(state,parent), ft_get_hostpath(state,newparent)))
    {
        FTFS_PRM2(MV);

        FTFS_FAILCALL(rename(hostpath, newhostpath))
        if (ft_rename (state, path, newrpath))
        {
            rename(newhostpath, hostpath);
            return -EACCES;
        }

        return 0;
    }

    stat = ft_stat(state, path, prc);

    FTFS_FAILCALL(ftfs_add(newrpath, stat, path, NULL))

    if (rename(hostpath, newhostpath) < 0)
    {
        ft_delete(state, newpath);
        return -EACCES;
    }
    if (ft_rename (state, path, newrpath))
    {
        rename(newhostpath, hostpath);
        ft_delete(state, newpath);
        return -EACCES;
    }

    ft_delete(state, path);
    return 0;
}

#define CHMOD_CATCH(t,T)     \
    if (d##t##T)             \
    {                        \
        if (s##t##T)         \
        {                    \
            prm.allow |=  T; \
            prm.deny  &= ~T; \
        }                    \
        else                 \
        {                    \
            prm.allow &= ~T; \
            prm.deny  |=  T; \
        }                    \
    }

int ftfs_chmod (const char *rpath, mode_t mode)
{
    struct stat *stat;
    ft_prc prc2;
    ft_prm prm;
    uint64_t r, w, x;
    int isdir;
    unsigned sur, suw, sux, sgr, sgw, sgx, sor, sow, sox,
             dur, duw, dux, dgr, dgw, dgx, dor, dow, dox;
    FTFS_INIT
    FTFS_EXISTS

    prc2 = *prc;
    prc2.cmd = NULL;

    stat = ft_stat(state, path, &prc2);
    if (!stat) return -EACCES;

    isdir = ft_is_dir(state, path);

    r = isdir ? FT_PRM_DREA : FT_PRM_FREA;
    w = isdir ? FT_PRM_DWRI : FT_PRM_FWRI;
    x = isdir ? FT_PRM_DEXE : FT_PRM_FEXE;

    sur = mode & S_IRUSR;
    suw = mode & S_IWUSR;
    sux = mode & S_IXUSR;

    sgr = mode & S_IRGRP;
    sgw = mode & S_IWGRP;
    sgx = mode & S_IXGRP;

    sor = mode & S_IROTH;
    sow = mode & S_IWOTH;
    sox = mode & S_IXOTH;


    dur = (stat->st_mode & S_IRUSR) != sur;
    duw = (stat->st_mode & S_IWUSR) != suw;
    dux = (stat->st_mode & S_IXUSR) != sux;

    dgr = (stat->st_mode & S_IRGRP) != sgr;
    dgw = (stat->st_mode & S_IWGRP) != sgw;
    dgx = (stat->st_mode & S_IXGRP) != sgx;

    dor = (stat->st_mode & S_IROTH) != sor;
    dow = (stat->st_mode & S_IWOTH) != sow;
    dox = (stat->st_mode & S_IXOTH) != sox;

    if ((isdir && ft_check_prm(state, path, prc, FT_PRM_DCP)) || (!isdir && ft_check_prm(state, path, prc, FT_PRM_FCP)))
    {
        if (dur || duw || dux)
        {
            prm.cat = FT_CAT_OUS;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(u,r);
            CHMOD_CATCH(u,w);
            CHMOD_CATCH(u,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;

            prm.cat = FT_CAT_UID;
            prm.prc.uid = stat->st_uid;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(u,r);
            CHMOD_CATCH(u,w);
            CHMOD_CATCH(u,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }

        if (dgr || dgw || dgx)
        {
            prm.cat = FT_CAT_OGR;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(g,r);
            CHMOD_CATCH(g,w);
            CHMOD_CATCH(g,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;

            prm.cat = FT_CAT_GID;
            prm.prc.gid = stat->st_gid;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(g,r);
            CHMOD_CATCH(g,w);
            CHMOD_CATCH(g,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }

        if (dor || dow || dox)
        {
            prm.cat = FT_CAT_OTH;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(o,r);
            CHMOD_CATCH(o,w);
            CHMOD_CATCH(o,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }
    }
    else if (!isdir && ft_check_prm(state, path, prc, FT_PRM_FSX))
    {

        if (dur || duw || dgr || dgw || dor || dow) return -EACCES;

        if (dux)
        {
            prm.cat = FT_CAT_OUS;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(u,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;

            prm.cat = FT_CAT_UID;
            prm.prc.uid = stat->st_uid;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(u,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }
        if (dgx)
        {
            prm.cat = FT_CAT_OGR;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(g,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;

            prm.cat = FT_CAT_GID;
            prm.prc.gid = stat->st_gid;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(u,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }
        if (dox)
        {
            prm.cat = FT_CAT_OTH;
            if (ft_get_prm(state, path, &prm)) return -EACCES;
            CHMOD_CATCH(o,x);
            if (ft_set_prm(state, path, &prm)) return -EACCES;
        }
    }
    else return -EACCES;
    return 0;
}

int ftfs_chown (const char *rpath, uid_t uid, gid_t gid)
{
    uid_t ouid;
    gid_t ogid;

    FTFS_INIT_PARENT
    FTFS_EXISTS

    if (ft_get_owner (state, path, &ouid)) return -EACCES;
    if (ft_get_group (state, path, &ogid)) return -EACCES;

    if (ouid != uid)
    {
        if (!ft_check_prm2(state, path, prc, FT_PRM_FCO, FT_PRM_DCO))
        {
            uid_t puid;
            if (!parent) return -EACCES;

            if (ft_get_owner (state, parent, &puid)) return -EACCES;
            if (puid != uid || !ft_check_prm2(state, path, prc, FT_PRM_FOP, FT_PRM_DOP)) return -EACCES;
        }
    }

    if (ogid != gid)
    {
        if (!ft_check_prm2(state, path, prc, FT_PRM_FCG, FT_PRM_DCG))
        {
            gid_t pgid;
            if (!parent) return -EACCES;

            if (ft_get_group (state, parent, &pgid)) return -EACCES;
            if (pgid != gid || !ft_check_prm2(state, path, prc, FT_PRM_FOP, FT_PRM_DOP)) return -EACCES;
        }
    }

    if (ouid != uid) ft_set_owner(state, path, uid);
    if (ogid != gid) ft_set_group(state, path, gid);

    return 0;
}

int ftfs_truncate (const char *rpath, off_t newsize)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM(FWR)
    FTFS_HOSTPATH

    FTFS_RETCALL(truncate(hostpath, newsize))
}

int ftfs_utime (const char *rpath, struct utimbuf *ubuf)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM2(CA)
    FTFS_HOSTPATH

    FTFS_RETCALL(utime(hostpath, ubuf))
}

int ftfs_open (const char *rpath, struct fuse_file_info *fi)
{
    FTFS_INIT
    FTFS_EXISTS

    if (fi->flags & O_RDONLY || fi->flags & O_RDWR) FTFS_PRM(FRD);
    if (fi->flags & O_WRONLY || fi->flags & O_RDWR) FTFS_PRM(FWR);

    FTFS_HOSTPATH

    FTFS_FAILCALL(open(hostpath, fi->flags))

    fi->fh = rc;
    return 0;
}

int ftfs_read (const char *rpath, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void) rpath;
    return pread(fi->fh, buf, size, offset);
}

int ftfs_write (const char *rpath, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    (void) rpath;
    return pwrite(fi->fh, buf, size, offset);
}

int ftfs_statfs (const char *rpath, struct statvfs *statv)
{
    FTFS_INIT
    FTFS_HOSTPATH
    (void) prc;
    FTFS_RETCALL (statvfs(hostpath, statv));
}

int ftfs_flush (const char *rpath, struct fuse_file_info *fi)
{
    (void) rpath;
    (void) fi;
    return 0;
}

int ftfs_release (const char *rpath, struct fuse_file_info *fi)
{
    (void) rpath;
    FTFS_RETCALL(close(fi->fh));
}

int ftfs_fsync (const char *rpath, int datasync, struct fuse_file_info *fi)
{
    (void) rpath;
    (void) datasync;
#ifdef HAVE_FDATASYNC
    if (datasync)
        FTFS_RETCALL(fdatasync(fi->fh))
    else
#endif
	FTFS_RETCALL(fsync(fi->fh))

}

int ftfs_setxattr (const char *rpath, const char *name, const char *value, size_t size, int flags)
{
    FTFS_INIT
    FTFS_EXISTS

    if (!memcmp(name, "user.ftfs.", 10)) return -EACCES;

    FTFS_PRM2(MX);
    FTFS_HOSTPATH

    FTFS_RETCALL(lsetxattr(hostpath, name, value, size, flags))
}

int ftfs_getxattr (const char *rpath, const char *name, char *value, size_t size)
{
    FTFS_INIT
    FTFS_EXISTS

    if (!memcmp(name, "user.ftfs.", 10)) return -ENOATTR;

    FTFS_PRM2(XA);
    FTFS_HOSTPATH

    FTFS_RETCALL(lgetxattr(hostpath, name, value, size))
}

int ftfs_listxattr (const char *rpath, char *list, size_t size)
{
    ssize_t s = size, r = 0;
    char *list2, *i, *e;
    size_t length;

    FTFS_INIT
    FTFS_EXISTS

    FTFS_PRM2(XA);
    FTFS_HOSTPATH

    if (size == 0)
        if ((s = llistxattr(hostpath, list, 0)) < 0) return -errno;

    list2 = malloc(s);
    if (!list2) return -ENOMEM;

    s = llistxattr(hostpath, list2, s);
    if (s < 0) {free(list2); return -errno; }

    for (i = list2, e = list2 + s; i < e; i += length)
    {
        length = strlen (i) + 1;
        if (length < 10 || memcmp(i, "user.ftfs.", 10))
        {
            r += length;
            if (size)
            {
                if (r >= (ssize_t) size) {free (list2); return -ERANGE; }
                memcpy(i, list, length);
                list += length;
            }
        }
    }

    free(list2); return r;
}

int ftfs_removexattr (const char *rpath, const char *name)
{
    FTFS_INIT
    FTFS_EXISTS

    if (!memcmp(name, "user.ftfs.", 10)) return -ENOATTR;

    FTFS_PRM2(MX);
    FTFS_HOSTPATH

    FTFS_RETCALL(lremovexattr(hostpath, name))
}

int ftfs_opendir (const char *rpath, struct fuse_file_info *fi)
{
    FTFS_INIT
    FTFS_EXISTS
    FTFS_PRM(DRD)
    FTFS_HOSTPATH
    FTFS_FAILCALL ((intptr_t) opendir(hostpath));
    fi->fh = rc;
    return 0;
}

int ftfs_readdir (const char *rpath, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    (void)rpath;
    (void)offset;

    DIR *dp = (DIR *) (uintptr_t) fi->fh;
    struct dirent *de;

    errno = 0;
    while ((de = readdir(dp)))
        if (filler(buf, de->d_name, NULL, 0)) return -ENOMEM;

    return -errno;
}

int ftfs_releasedir (const char *rpath, struct fuse_file_info *fi)
{
    (void)rpath;
    closedir((DIR *) (uintptr_t) fi->fh);
    return 0;
}

/*
int ftfs_access (const char *rpath, int mode)
{
    int r = 1;

    FTFS_INIT
    FTFS_HOSTPATH

    if (mode & F_OK) r = access(hostpath, );

    if (mode != F_OK)
    {
        struct stat statbuf;

        int R, W, X;

        if (ft_stat(state, path, prc, &statbuf) < 0) return -errno;

        if      (statbuf.st_uid == prc->uid)
        {
            R = S_IRUSR;
            W = S_IWUSR;
            X = S_IXUSR;
        }
        else if (statbuf.st_gid == prc->gid)
        {
            R = S_IRGRP;
            W = S_IWGRP;
            X = S_IXGRP;
        }
        else
        {
            R = S_IROTH;
            W = S_IWOTH;
            X = S_IXOTH;
        }

        if (mode & R_OK) r = r && statbuf.st_mode & R;
        if (mode & W_OK) r = r && statbuf.st_mode & W;
        if (mode & X_OK) r = r && statbuf.st_mode & X;

    }

    return r ? 0 : -1;
}
*/


int ftfs_ftruncate (const char *rpath, off_t offset, struct fuse_file_info *fi)
{
    (void) rpath;
    FTFS_RETCALL(ftruncate(fi->fh, offset));
}

int ftfs_fgetattr (const char *rpath, struct stat *statbuf, struct fuse_file_info *fi)
{
    (void) fi;
    return ftfs_getattr(rpath, statbuf);
}
