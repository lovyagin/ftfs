/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

#include "ftpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#define __USE_MISC
#include <grp.h>

// nopar
int ftpc_type (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    char *type;
    (void) argv;
    if (argc > 0) return FTPC_MSG_USAGE;

    if (prc->uid != 0 && (prc->uid != state->uid || prc->gid != state->gid)) return FTPC_ERR_PRM;

    switch (state->type)
    {
        case FT_SQLITE: type = "db";     break;
        case FT_XATTR:  type = "xattr";  break;
        case FT_FXATTR: type = "fxattr"; break;
        default: return FT_ERR_TYPE;
    }

    printf ("%s", type);

    return FT_OK;

}

int ftpc_str_to_int (const char *str, unsigned long *value)
{
    char *end;
    if (str || !*str) return 0;
    *value = strtoul (str, &end, 10);

    return *end ? 1 : 0;
}

int ftpc_mk_path (const char *source, char *buffer, ft_path *path)
{
    size_t l;
    if (source[0] == '/')
        { if (!ft_str_cpy (buffer, source, FT_LIMIT_PATH))       return FT_ERR_PATH; }
    else
        { if (!ft_str_cat2 (buffer, "/", source, FT_LIMIT_PATH)) return FT_ERR_PATH; }
    l = strlen(buffer);
    if (l != 1 && buffer[l-1] == '/') buffer[l-1] = '\0';

    *path = ft_path_init (buffer);
    return FT_OK;
}

// entry
int ftpc_get_uid (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    uid_t uid;
    ft_path path;
    int rc;
    unsigned long v;
    char buffer [FT_LIMIT_PATH];

    if (argc != 1) return FTPC_MSG_USAGE;

    if ((rc = ftpc_mk_path(argv[0], buffer, &path))) return rc;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FRA, FT_PRM_DRA)) return FTPC_ERR_PRM;

    if ((rc = ft_get_owner(state, &path, &uid))) return rc;

    v = uid;
    printf ("%lu", v);

    return FT_OK;
}

// entry
int ftpc_get_gid (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    gid_t gid;
    ft_path path;
    int rc;
    unsigned long v;
    char buffer [FT_LIMIT_PATH];

    if (argc != 1) return FTPC_MSG_USAGE;

    if ((rc = ftpc_mk_path(argv[0], buffer, &path))) return rc;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FRA, FT_PRM_DRA)) return FTPC_ERR_PRM;

    if ((rc = ft_get_group(state, &path, &gid))) return rc;

    v = gid;
    printf ("%lu", v);

    return FT_OK;
}

// flag entry
int ftpc_get_inh (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    uint64_t inh;
    ft_path path;
    int rc;
    char buffer [FT_LIMIT_PATH];

    if (argc != 2) return FTPC_MSG_USAGE;

    if ((rc = ftpc_mk_path(argv[1], buffer, &path))) return rc;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FRP, FT_PRM_DRP)) return FTPC_ERR_PRM;

    if ((rc = ft_get_inh (state, &path, &inh))) return rc;

    if      (!strcmp (argv[0], "INH")) inh &= FT_INH_INH;
    else if (!strcmp (argv[0], "SET")) inh &= FT_INH_SET;
    else if (!strcmp (argv[0], "IFP")) inh &= FT_INH_IFP;
    else if (!strcmp (argv[0], "IFS")) inh &= FT_INH_IFS;
    else if (!strcmp (argv[0], "SPI")) inh &= FT_INH_SPI;
    else if (!strcmp (argv[0], "SPS")) inh &= FT_INH_SPS;
    else if (!strcmp (argv[0], "CPR")) inh &= FT_INH_CPR;
    else if (!strcmp (argv[0], "SFP")) inh &= FT_INH_SFP;
    else if (!strcmp (argv[0], "SFS")) inh &= FT_INH_SFS;
    else if (!strcmp (argv[0], "CFP")) inh &= FT_INH_CFP;
    else    return FTPC_ERR_INH;

    printf (inh ? "1" : "0");
    return FT_OK;

}

int ftpc_get_cat (const char *source, ft_cat *cat)
{
    if      (!strcmp (source, "ALL")) *cat = FT_CAT_ALL;
    else if (!strcmp (source, "OUS")) *cat = FT_CAT_OUS;
    else if (!strcmp (source, "OGR")) *cat = FT_CAT_OGR;
    else if (!strcmp (source, "OTH")) *cat = FT_CAT_OTH;
    else if (!strcmp (source, "UID")) *cat = FT_CAT_UID;
    else if (!strcmp (source, "GID")) *cat = FT_CAT_GID;
    else if (!strcmp (source, "PEX")) *cat = FT_CAT_PEX;
    else    return FTPC_ERR_CAT;
    return FT_OK;

}

#define IFPRMGET(arg) if (!strcmp (source, #arg)) *prm = FT_PRM_##arg;
#define EFPRMGET(arg) else IFPRMGET(arg)
int ftpc_get_prmf (const char *source, uint64_t *prm)
{
    IFPRMGET(FRD)
    EFPRMGET(FRA)
    EFPRMGET(FRP)
    EFPRMGET(FXA)
    EFPRMGET(FSL)
    EFPRMGET(FWR)
    EFPRMGET(FCA)
    EFPRMGET(FCP)
    EFPRMGET(FCI)
    EFPRMGET(FCO)
    EFPRMGET(FCG)
    EFPRMGET(FOP)
    EFPRMGET(FRM)
    EFPRMGET(FMV)
    EFPRMGET(FSX)
    EFPRMGET(FEX)
    EFPRMGET(DRD)
    EFPRMGET(DRA)
    EFPRMGET(DRP)
    EFPRMGET(DXA)
    EFPRMGET(DAF)
    EFPRMGET(DAD)
    EFPRMGET(DAL)
    EFPRMGET(DMK)
    EFPRMGET(DMD)
    EFPRMGET(DSL)
    EFPRMGET(DCA)
    EFPRMGET(DCP)
    EFPRMGET(DCI)
    EFPRMGET(DCT)
    EFPRMGET(DCO)
    EFPRMGET(DCG)
    EFPRMGET(DOP)
    EFPRMGET(DRM)
    EFPRMGET(DMX)
    EFPRMGET(DEX)
    EFPRMGET(DCH)
    EFPRMGET(DBL)
    EFPRMGET(DSC)
    EFPRMGET(DFF)
    EFPRMGET(DAC)
    EFPRMGET(DAB)
    EFPRMGET(DAO)
    EFPRMGET(DAP)
    else    return FTPC_ERR_PFL;
    return FT_OK;

}

// cat [id] flag {A|D} entry
int ftpc_get_prm (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    ft_path path;
    int rc;
    long unsigned v;
    char buffer [FT_LIMIT_PATH],
         *flag, *pad, *entry;
    int ad;
    uint64_t perm;
    ft_prm prm;

    if (argc < 1) return FTPC_MSG_USAGE;

    if ((rc = ftpc_get_cat (argv[0], &prm.cat))) return rc;


    if (prm.cat == FT_CAT_UID || prm.cat == FT_CAT_GID || prm.cat == FT_CAT_PEX)
    {
        if (argc != 5) return FTPC_MSG_USAGE;
        flag  = argv[2];
        pad   = argv[3];
        entry = argv[4];

        if (prm.cat == FT_CAT_UID || prm.cat == FT_CAT_GID)
        {
            if (!ftpc_str_to_int(argv[1], &v)) return FTPC_ERR_NUMBER;
            if (prm.cat == FT_CAT_UID) prm.prc.uid = v; else prm.prc.gid = v;
        }
        else
            prm.prc.cmd = argv[1];
    }
    else
    {
        if (argc != 4) return FTPC_MSG_USAGE;
        flag  = argv[1];
        pad   = argv[2];
        entry = argv[3];

    }

    if ((rc = ftpc_mk_path(entry, buffer, &path))) return rc;
    if (!ft_check_prm2(state, &path, prc, FT_PRM_FRP, FT_PRM_DRP)) return FTPC_ERR_PRM;

    if      (!strcmp (pad, "A")     ||
             !strcmp (pad, "a")     ||
             !strcmp (pad, "allow") ||
             !strcmp (pad, "Allow") ||
             !strcmp (pad, "ALLOW")
            ) ad = 0;
    else if (!strcmp (pad, "D")     ||
             !strcmp (pad, "d")     ||
             !strcmp (pad, "deny")  ||
             !strcmp (pad, "Deny")  ||
             !strcmp (pad, "DENY")
            ) ad = 1;
    else return FTPC_ERR_AD;

    if ((rc = ftpc_get_prmf (flag, &perm))) return rc;

    prm.allow = prm.deny = 0;

    if ((rc = ft_get_prm(state, &path, &prm))) return rc;

    printf ((ad ? prm.deny & perm : prm.allow & perm) ? "1" : "0");

    return FT_OK;


}

// cat [id] flag {A|D} [val] entry
int ftpc_set_prm (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    ft_path path;
    int rc;
    long unsigned v;
    char buffer [FT_LIMIT_PATH],
         *flag, *pad, *entry, *pval;
    int ad, val;
    uint64_t perm;
    ft_prm prm;

    if (argc < 1) return FTPC_MSG_USAGE;

    if ((rc = ftpc_get_cat (argv[0], &prm.cat))) return rc;


    if (prm.cat == FT_CAT_UID || prm.cat == FT_CAT_GID || prm.cat == FT_CAT_PEX)
    {
        if (argc == 5)
        {
            pval = NULL;
            entry = argv[4];
        }
        else if (argc == 6)
        {
            pval  = argv[4];
            entry = argv[5];
        }
        else return FTPC_MSG_USAGE;

        flag  = argv[2];
        pad   = argv[3];

        if (prm.cat == FT_CAT_UID || prm.cat == FT_CAT_GID)
        {
            if (!ftpc_str_to_int(argv[1], &v)) return FTPC_ERR_NUMBER;
            if (prm.cat == FT_CAT_UID) prm.prc.uid = v; else prm.prc.gid = v;
        }
        else
            prm.prc.cmd = argv[1];
    }
    else
    {
        if (argc == 4)
        {
            pval = NULL;
            entry = argv[3];
        }
        else if (argc == 5)
        {
            pval  = argv[3];
            entry = argv[4];
        }
        else return FTPC_MSG_USAGE;
        flag  = argv[1];
        pad   = argv[2];
    }

    if      (!pval)               val = 1;
    else if (!strcmp (pval, "1")) val = 1;
    else if (!strcmp (pval, "0")) val = 0;
    else return FTPC_ERR_SET;

    if ((rc = ftpc_mk_path(entry, buffer, &path))) return rc;
    if (!ft_check_prm2(state, &path, prc, FT_PRM_FCP, FT_PRM_DCP)) return FTPC_ERR_PRM;

    if      (!strcmp (pad, "A")     ||
             !strcmp (pad, "a")     ||
             !strcmp (pad, "allow") ||
             !strcmp (pad, "Allow") ||
             !strcmp (pad, "ALLOW")
            ) ad = 0;
    else if (!strcmp (pad, "D")     ||
             !strcmp (pad, "d")     ||
             !strcmp (pad, "deny")  ||
             !strcmp (pad, "Deny")  ||
             !strcmp (pad, "DENY")
            ) ad = 1;
    else return FTPC_ERR_AD;

    if ((rc = ftpc_get_prmf (flag, &perm))) return rc;

    prm.allow = prm.deny = 0;

    if ((rc = ft_get_prm(state, &path, &prm))) return rc;

    if (!ft_is_dir(state, &path) && (perm & FT_PRM_DIRS)) return FTPC_ERR_DFP;

    if (ad)
        if (val) prm.deny  |= perm; else prm.deny  &= ~perm;
    else
        if (val) prm.allow |= perm; else prm.allow &= ~perm;

//    printf ((ad ? prm.deny & perm : prm.allow & perm) ? "1" : "0");

    return ft_set_prm(state, &path, &prm);

//    return FT_OK;

}

// flag [val] entry
int ftpc_set_inh (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    uint64_t cinh, inh;
    ft_path path;
    int rc;
    char buffer [FT_LIMIT_PATH];
    int val;

    if      (argc == 2)
    {
        val = 1;
        if ((rc = ftpc_mk_path(argv[1], buffer, &path))) return rc;
    }
    else if (argc == 3)
    {
        if      (!strcmp(argv[1], "1")) val = 1;
        else if (!strcmp(argv[1], "0")) val = 0;
        else return FTPC_ERR_SET;
        if ((rc = ftpc_mk_path(argv[2], buffer, &path))) return rc;
    }
    else return FTPC_MSG_USAGE;

    if ((rc = ft_get_inh (state, &path, &cinh))) return rc;

    if      (!strcmp (argv[0], "INH")) inh = FT_INH_INH;
    else if (!strcmp (argv[0], "SET")) inh = FT_INH_SET;
    else if (!strcmp (argv[0], "IFP")) inh = FT_INH_IFP;
    else if (!strcmp (argv[0], "IFS")) inh = FT_INH_IFS;
    else if (!strcmp (argv[0], "SPI")) inh = FT_INH_SPI;
    else if (!strcmp (argv[0], "SPS")) inh = FT_INH_SPS;
    else if (!strcmp (argv[0], "CPR")) inh = FT_INH_CPR;
    else if (!strcmp (argv[0], "SFP")) inh = FT_INH_SFP;
    else if (!strcmp (argv[0], "SFS")) inh = FT_INH_SFS;
    else if (!strcmp (argv[0], "CFP")) inh = FT_INH_CFP;
    else    return FTPC_ERR_INH;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FCI, inh & FT_INH_TRMS ? FT_PRM_DCT : FT_PRM_DCI)) return FTPC_ERR_PRM;

    if (!ft_is_dir (state, &path) && (inh & FT_INH_DIRS)) return FTPC_ERR_DFI;

    if      (!val && inh == FT_INH_INH && (cinh & FT_INH_INH)) return ft_unset_inh (state, &path);
    else if (!val && inh == FT_INH_IFP && (cinh & FT_INH_IFP)) return ft_unset_ifp (state, &path);
    else return ft_set_inh(state, &path, val ? cinh | inh : cinh & ~inh);

}

// uid entry
int ftpc_set_uid (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    uid_t uid;
    ft_path path;
    int rc;
    char buffer [FT_LIMIT_PATH];
    unsigned long v;

    if (argc != 2) return FTPC_MSG_USAGE;
    if (!ftpc_str_to_int(argv[0], &v)) return FTPC_ERR_NUMBER;
    uid = v;

    if ((rc = ftpc_mk_path(argv[1], buffer, &path))) return rc;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FCO, FT_PRM_DCO))
    {
        char pbuffer [FT_LIMIT_PATH];
        uid_t puid;
        ft_path parent = ft_path_init(pbuffer);

        if (!ft_parent(&path, &parent)) return FTPC_ERR_PRM;

        if ((rc == ft_get_owner(state, &parent, &puid))) return rc;
        if (puid != uid || !ft_check_prm2(state, &path, prc, FT_PRM_FOP, FT_PRM_DOP)) return FTPC_ERR_PRM;
    }

    if ((rc = ft_set_owner(state, &path, uid))) return rc;

    return FT_OK;
}

// gid entry
int ftpc_set_gid (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    gid_t gid;
    ft_path path;
    int rc;
    char buffer [FT_LIMIT_PATH];
    unsigned long v;

    if (argc != 2) return FTPC_MSG_USAGE;
    if (!ftpc_str_to_int(argv[0], &v)) return FTPC_ERR_NUMBER;
    gid = v;

    if ((rc = ftpc_mk_path(argv[1], buffer, &path))) return rc;

    if (!ft_check_prm2(state, &path, prc, FT_PRM_FCG, FT_PRM_DCG))
    {
        char pbuffer [FT_LIMIT_PATH];
        gid_t pgid;
        ft_path parent = ft_path_init (pbuffer);

        if (!ft_parent(&path, &parent)) return FTPC_ERR_PRM;

        if ((rc == ft_get_group (state, &parent, &pgid))) return rc;
        if (pgid != gid || !ft_check_prm2(state, &path, prc, FT_PRM_FOP, FT_PRM_DOP)) return FTPC_ERR_PRM;
    }

    if ((rc = ft_set_group(state, &path, gid))) return rc;

    return FT_OK;
}

// uid:gid[:cmd] flag entry
int ftpc_test (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    char *puid, *pgid, *pcmd;
    long unsigned iuid, igid;
    ft_prc prc2;
    struct passwd *pwd;
    uint64_t prm;
    int rc;
    ft_path path;
    char buffer [FT_LIMIT_PATH];

    if (prc->uid != 0 && (prc->uid != state->uid || prc->gid != state->gid)) return FTPC_ERR_PRM;

    if (argc != 3) return FTPC_MSG_USAGE;

    puid = argv[0];

    pgid = puid;
    while (*pgid && *pgid != ':') pgid++;
    if (!*pgid) return FTPC_MSG_USAGE;
    *pgid = '\0';
    ++pgid;

    pcmd = pgid;
    while (*pcmd && *pcmd != ':') pcmd++;
    if (!*pcmd)
        pcmd = NULL;
    else
    {
        *pcmd = '\0';
        ++pcmd;
    }


    if (!ftpc_str_to_int(puid, &iuid)) return FTPC_ERR_NUMBER;
    if (!ftpc_str_to_int(pgid, &igid)) return FTPC_ERR_NUMBER;

    prc2.uid = iuid;
    prc2.gid = igid;
    prc2.cmd = pcmd;

    pwd = getpwuid(prc2.uid);

    prc2.ngroups = FT_LIMIT_GROUPS;
    prc2.ngroups = getgrouplist (pwd->pw_name, prc2.gid, prc2.groups, &prc2.ngroups);

    if (prc2.ngroups == -1) return FTPC_ERR_GROUPS;

    if ((rc = ftpc_get_prmf (argv[1], &prm))) return rc;

    if ((rc = ftpc_mk_path(argv[2], buffer, &path))) return rc;

    printf (ft_check_prm(state, &path, &prc2, prm) ? "1" : "0");

    return FT_OK;

}

#define SHOW_INH(INH) printf(inh & FT_INH_##INH ? #INH " " : "    ");

void ftpc_show_inhs (uint64_t inh, int isdir)
{
    printf("INHERITANCE:\t");

    SHOW_INH(INH)
    SHOW_INH(SET)

    if (isdir)
    {
        SHOW_INH(IFP)
        SHOW_INH(IFS)
        printf ("\tTRANSMISSION:\t");

        SHOW_INH(SPI)
        SHOW_INH(SPS)
        SHOW_INH(CPR)
        SHOW_INH(SFP)
        SHOW_INH(SFS)
        SHOW_INH(CFP)
    }
    printf ("\n");
}

#define SHOW_PRM(SRC,PRM) printf(SRC & FT_PRM_##PRM ? #PRM " " : "    ");
#define SHOW_FIL_PRMS(SRC) \
        printf ("R: ");    \
        SHOW_PRM(SRC,FRD)  \
        SHOW_PRM(SRC,FRA)  \
        SHOW_PRM(SRC,FRP)  \
        SHOW_PRM(SRC,FXA)  \
        SHOW_PRM(SRC,FSL)  \
        printf ("W: ");    \
        SHOW_PRM(SRC,FWR)  \
        SHOW_PRM(SRC,FCA)  \
        SHOW_PRM(SRC,FCP)  \
        SHOW_PRM(SRC,FCI)  \
        SHOW_PRM(SRC,FCO)  \
        SHOW_PRM(SRC,FCG)  \
        SHOW_PRM(SRC,FOP)  \
        SHOW_PRM(SRC,FRM)  \
        SHOW_PRM(SRC,FMV)  \
        SHOW_PRM(SRC,FMX)  \
        SHOW_PRM(SRC,FSX)  \
        printf ("X: ");    \
        SHOW_PRM(SRC,FEX)

#define SHOW_DIR_PRMS(SRC) \
        printf ("R: ");    \
        SHOW_PRM(SRC,DRD)  \
        SHOW_PRM(SRC,DRA)  \
        SHOW_PRM(SRC,DRP)  \
        SHOW_PRM(SRC,DXA)  \
        printf ("W: ");    \
        SHOW_PRM(SRC,DAF)  \
        SHOW_PRM(SRC,DAD)  \
        SHOW_PRM(SRC,DAL)  \
        SHOW_PRM(SRC,DAC)  \
        SHOW_PRM(SRC,DAB)  \
        SHOW_PRM(SRC,DAO)  \
        SHOW_PRM(SRC,DAP)  \
        SHOW_PRM(SRC,DMK)  \
        SHOW_PRM(SRC,DMD)  \
        SHOW_PRM(SRC,DSL)  \
        SHOW_PRM(SRC,DCH)  \
        SHOW_PRM(SRC,DBL)  \
        SHOW_PRM(SRC,DSC)  \
        SHOW_PRM(SRC,DFF)  \
        SHOW_PRM(SRC,DCA)  \
        SHOW_PRM(SRC,DCP)  \
        SHOW_PRM(SRC,DCI)  \
        SHOW_PRM(SRC,DCT)  \
        SHOW_PRM(SRC,DCO)  \
        SHOW_PRM(SRC,DCG)  \
        SHOW_PRM(SRC,DOP)  \
        SHOW_PRM(SRC,DRM)  \
        SHOW_PRM(SRC,DMV)  \
        SHOW_PRM(SRC,DMX)  \
        printf ("X: ");    \
        SHOW_PRM(SRC,DEX)

void ftpc_show_prm (const ft_prm *prm, int isdir)
{
    if (!prm->allow && !prm->deny) return;
    switch (prm->cat)
    {
        case FT_CAT_ALL: printf ("ALL");                    break;
        case FT_CAT_OUS: printf ("OUS (%i)", prm->prc.uid); break;
        case FT_CAT_OGR: printf ("OGR (%i)", prm->prc.gid); break;
        case FT_CAT_OTH: printf ("OTH");                    break;
        case FT_CAT_UID: printf ("UID (%i)", prm->prc.uid); break;
        case FT_CAT_GID: printf ("GID (%i)", prm->prc.gid); break;
        case FT_CAT_PEX: printf ("PEX (%s)", prm->prc.cmd); break;
    }

    if (isdir)
    {
        printf("\n\tDIR ALLOW:\t");  SHOW_DIR_PRMS(prm->allow);
        printf("\n\tDIR DENY:\t");   SHOW_DIR_PRMS(prm->deny);
        printf("\n\tFILE ALLOW:\t"); SHOW_FIL_PRMS(prm->allow);
        printf("\n\tFILE DENY:\t");  SHOW_FIL_PRMS(prm->deny);
        printf("\n");
    }
    else
    {
        printf("\n\tALLOW:\t"); SHOW_FIL_PRMS(prm->allow);
        printf("\n\tDENY:\t");  SHOW_FIL_PRMS(prm->deny);
        printf("\n");
    }

}

int ftpc_show_prms (ft_prms prms, ft_prc *prc, int isdir)
{
    ft_prms uid = ft_prms_init(), gid = ft_prms_init(), pex = ft_prms_init();
    ft_prm all, ous, ogr, oth; const ft_prm *prm;
    all.allow = all.deny = ous.allow = ous.deny = ogr.allow = ogr.deny = oth.allow = oth.deny = 0;
    all.cat = FT_CAT_ALL;
    ous.cat = FT_CAT_OUS;
    ogr.cat = FT_CAT_OGR;
    oth.cat = FT_CAT_OTH;

    ous.prc.uid = prc->uid;
    ogr.prc.gid = prc->gid;

    size_t i, j, s, l;

    for (i = 0, s = ft_prms_size(&prms); i < s; ++i)
    {
        prm = ft_prms_get(&prms, i);
        switch (prm->cat)
        {
            case FT_CAT_ALL: all.allow |= prm->allow; all.deny |= prm->deny; break;
            case FT_CAT_OUS: ous.allow |= prm->allow; ous.deny |= prm->deny; break;
            case FT_CAT_OGR: ogr.allow |= prm->allow; ogr.deny |= prm->deny; break;
            case FT_CAT_OTH: oth.allow |= prm->allow; oth.deny |= prm->deny; break;
            case FT_CAT_UID:
                ft_prms_add(&uid, prm);
                if (ft_prms_is_error(&uid)) return FT_ERR_MALLOC;
                l = ft_prms_size(&uid);
                if (l > 2)
                {
                    for (j = l - 1; j > 0 && ft_prms_get(&uid, j - 1)->prc.uid > prm->prc.uid; --j)
                        *ft_prms_element(&uid, j) = *ft_prms_get(&uid, j - 1);
                    *ft_prms_element(&uid, j) = *prm;
                }
                break;
            case FT_CAT_GID:
                ft_prms_add(&gid, prm);
                if (ft_prms_is_error(&gid)) return FT_ERR_MALLOC;
                l = ft_prms_size(&gid);
                if (l > 2)
                {
                    for (j = l - 1; j > 0 && ft_prms_get(&gid, j - 1)->prc.gid > prm->prc.gid; --j)
                        *ft_prms_element(&gid, j) = *ft_prms_get(&gid, j - 1);
                    *ft_prms_element(&gid, j) = *prm;
                }
                break;
            case FT_CAT_PEX:
                ft_prms_add(&pex, prm);
                if (ft_prms_is_error(&pex)) return FT_ERR_MALLOC;
                l = ft_prms_size(&pex);
                if (l > 2)
                {
                    for (j = l - 1; j > 0 && strcmp (ft_prms_get(&pex, j - 1)->prc.cmd, prm->prc.cmd) > 0; --j)
                        *ft_prms_element(&pex, j) = *ft_prms_get(&pex, j - 1);
                    *ft_prms_element(&pex, j) = *prm;
                }
                break;
        }
    }

    ftpc_show_prm (&all, isdir);
    ftpc_show_prm (&ous, isdir);
    for (i = 0, s = ft_prms_size(&uid); i < s; ++i) ftpc_show_prm (ft_prms_get(&uid, i), isdir);

    ftpc_show_prm (&ogr, isdir);
    for (i = 0, s = ft_prms_size(&gid); i < s; ++i) ftpc_show_prm (ft_prms_get(&gid, i), isdir);

    for (i = 0, s = ft_prms_size(&pex); i < s; ++i) ftpc_show_prm (ft_prms_get(&pex, i), isdir);
    ftpc_show_prm (&oth, isdir);

    return FT_OK;
}

// entry
int ftpc_show (ft_state *state, ft_prc *prc, int argc, char *argv[])
{
    ft_path path;
    int rc, isdir;
    char buffer [FT_LIMIT_PATH];
    ft_prms oprms, iprms, jprms = ft_prms_init();
    uint64_t inh;
    uid_t uid; gid_t gid;

    if (argc != 1) return FTPC_MSG_USAGE;
    if ((rc = ftpc_mk_path(argv[0], buffer, &path))) return rc;

    if (!ft_check_prm2 (state, &path, prc, FT_PRM_FRP, FT_PRM_DRP)) return FTPC_ERR_PRM;

    oprms = ft_get_prms   (state, &path);
    iprms = ft_get_inhprms(state, &path);

    ft_prms_join(&jprms, &oprms);
    ft_prms_join(&jprms, &iprms);

    if (ft_prms_is_error(&oprms) || ft_prms_is_error(&iprms) || ft_prms_is_error(&jprms)) return FTPC_ERR_PRMGET;

    if ((rc = ft_get_inh   (state, &path, &inh))) return rc;
    if ((rc = ft_get_owner (state, &path, &uid))) return rc;
    if ((rc = ft_get_group (state, &path, &gid))) return rc;

    isdir = ft_is_dir(state, &path);

    printf ("FTFS \"%s\" %s OWNER = %i GROUP = %i\n", path.path, isdir ? "DIR" : "FILE", uid, gid);

    ftpc_show_inhs (inh, isdir);

    printf ("\nJoined permissions:\n");
    if ((rc = ftpc_show_prms (jprms, prc, isdir))) return rc;

    printf ("\nOwn permissions:\n");
    if ((rc = ftpc_show_prms (oprms, prc, isdir))) return rc;

    printf ("\nInherited permissions:\n");
    if ((rc = ftpc_show_prms (iprms, prc, isdir))) return rc;

    return FT_OK;

}
