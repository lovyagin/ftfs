/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

/**
*  @file      ftfs.h
*  @brief     basic include file for ftfs library
*  @authors   Roman Y. Dayneko, <dayneko3000@gmail.com>,
*             Nikita Yu. Lovyagin, <lovyagin@mail.com>
*  @copyright GNU GPLv3.
*/


#ifndef FTFS_H_INCLUDED
#define FTFS_H_INCLUDED

#define _XOPEN_SOURCE 500
#include <sys/stat.h>

#include <sqlite3.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

/*********************  General constants  *************************/
//{
#define FT_LIMIT_PATH   8192
#define FT_LIMIT_XHASH  8192
#define FT_LIMIT_GROUPS 8192

/** type of FTFS permission storing **/
typedef enum
{
    FT_XATTR,   /** safe (ASCII) xattr  **/
    FT_SQLITE,  /** sqlite3 database    **/
    FT_FXATTR   /** fast (binary) xattr **/
} ft_type;

extern const char ft_name[];
extern const char ft_version[];
extern char ft_prg_name[];
extern char ft_prg_version[];

//}

/**********************  FTFS Permissions  *************************/
//{
/** permission inheritance properties **/
#define FT_INH_INH (1uLL <<  0) /** Inherit permission from parent directory **/
#define FT_INH_SET (1uLL <<  1) /** Set (keep) inherited permission when inherit flag removed **/

/** transmitted file permission inheritance properties **/

#define FT_INH_IFP (1uLL << 16) /** Inherit dir->file permission (permission transmitted to file) from parent directory **/
#define FT_INH_IFS (1uLL << 17) /** Set (keep) dir->file permission when inherit flag removed **/

/** permission transmission properties **/

#define FT_INH_SPI (1uLL << 32) /** Set inheritance flag (transmit permissions) **/
#define FT_INH_SPS (1uLL << 33) /** Set "set (keep) permission" flag **/
#define FT_INH_CPR (1uLL << 34) /** Copy permission to child **/

#define FT_INH_SFP (1uLL << 48) /** Set dir->file permission inheritance flag **/
#define FT_INH_SFS (1uLL << 49) /** Set "set (keep) dir->file permission" flag **/
#define FT_INH_CFP (1uLL << 50) /** Copy dir->file permission to child **/

#define FT_INH_FILE (FT_INT_INT | FT_INH_SET)               /** File-assigned inheritance **/
#define FT_INH_DIRS (FT_INH_IFP | FT_INH_IFS |              \
                     FT_INH_SPI | FT_INH_SPS | FT_INH_CPR | \
                     FT_INH_SFP | FT_INH_SFS | FT_INH_CFP)  /** Dir-assigned inheritance **/

#define FT_INH_INHS (FT_INH_INH | FT_INH_SET | \
                     FT_INH_IFP | FT_INH_IFS)               /** Inheritance **/
#define FT_INH_TRMS (FT_INH_SPI | FT_INH_SPS | FT_INH_CPR | \
                     FT_INH_SFP | FT_INH_SFS | FT_INH_CFP)  /** Transmission **/


/** File permissions **/

#define FT_PRM_FRD (1uLL <<  0) /** Read file **/
#define FT_PRM_FRA (1uLL <<  1) /** Stat file permissions (getattr) **/
#define FT_PRM_FRP (1uLL <<  2) /** Read file permissions **/
#define FT_PRM_FXA (1uLL <<  3) /** Read file extended attributes **/
#define FT_PRM_FSL (1uLL <<  4) /** Read (follow) symlink **/

#define FT_PRM_FWR (1uLL <<  8) /** Write to file (modify) **/
#define FT_PRM_FCA (1uLL <<  9) /** Change attributes (utime) **/
#define FT_PRM_FCP (1uLL << 10) /** Chmod (change permission) file **/
#define FT_PRM_FCI (1uLL << 11) /** Change permission inheritance of the file **/
#define FT_PRM_FCO (1uLL << 12) /** Chown file (inside group) **/
#define FT_PRM_FCG (1uLL << 13) /** Change owner group **/
#define FT_PRM_FOP (1uLL << 14) /** Chown and chgroup parent dir owner:group **/
#define FT_PRM_FRM (1uLL << 15) /** Remove file **/
#define FT_PRM_FMV (1uLL << 16) /** Rename file (inside parent directory) **/
#define FT_PRM_FMX (1uLL << 17) /** Modify file extended attributes **/
#define FT_PRM_FSX (1uLL << 17) /** (Un)set file execution for bit for current user **/

#define FT_PRM_FEX (1uLL << 24) /** Execute file **/

#define FT_PRM_FREA (FT_PRM_FRD | FT_PRM_FRA | FT_PRM_FRP | FT_PRM_FXA | FT_PRM_FSL)   /** File read permission **/
#define FT_PRM_FWRI (FT_PRM_FWR | FT_PRM_FCA | FT_PRM_FCP | FT_PRM_FCI | FT_PRM_FCO | \
                     FT_PRM_FCG | FT_PRM_FOP | FT_PRM_FRM | FT_PRM_FMV | FT_PRM_FMX | \
                     FT_PRM_FSX)                                                       /** File write permission **/
#define FT_PRM_FEXE (FT_PRM_FEX)                                                       /** File exec permission **/
#define FT_PRM_FILE (FT_PRM_FREA | FT_PRM_FWRI | FT_PRM_FEXE)                          /** File permission **/


/** Directory permissions **/

#define FT_PRM_DRD (1uLL << 32) /** Read directory context **/
#define FT_PRM_DRA (1uLL << 33) /** Stat directory (getattr) **/
#define FT_PRM_DRP (1uLL << 34) /** Read directory permissions **/
#define FT_PRM_DXA (1uLL << 35) /** Read directory extended attributes **/

#define FT_PRM_DAF (1uLL << 37) /** Create (add) files (mknod), same owner as parent directory's one **/
#define FT_PRM_DAD (1uLL << 38) /** Create (add) subdirectory (mkdir), same owner as parent directory's one **/
#define FT_PRM_DAL (1uLL << 39) /** Create (add) symbolic link in the directory, same owner as parent directory's one **/
#define FT_PRM_DMK (1uLL << 40) /** Create own files (mknod) **/
#define FT_PRM_DMD (1uLL << 41) /** Create own subdirectory (mkdir) **/
#define FT_PRM_DSL (1uLL << 42) /** Create own symbolic link in the directory **/
#define FT_PRM_DCA (1uLL << 43) /** Change attributes (utime) **/
#define FT_PRM_DCP (1uLL << 44) /** Chmod (change permission) directory **/
#define FT_PRM_DCI (1uLL << 45) /** Change permission inheritance rules of the directory **/
#define FT_PRM_DCT (1uLL << 46) /** Change permission transmit rules of the directory **/
#define FT_PRM_DCO (1uLL << 47) /** Chown directory (inside group) **/
#define FT_PRM_DCG (1uLL << 48) /** Change owner group **/
#define FT_PRM_DOP (1uLL << 49) /** Chown and chgroup to parent dir owner:group **/
#define FT_PRM_DRM (1uLL << 50) /** Remove directory **/
#define FT_PRM_DMV (1uLL << 51) /** Rename directory (inside parent directory) **/
#define FT_PRM_DMX (1uLL << 52) /** Modify directory extended attributes **/
#define FT_PRM_DCH (1uLL << 53) /** Create own symbolic device **/
#define FT_PRM_DBL (1uLL << 54) /** Create own block device **/
#define FT_PRM_DSC (1uLL << 55) /** Create own socket **/
#define FT_PRM_DFF (1uLL << 56) /** Create own fifo **/
#define FT_PRM_DAC (1uLL << 57) /** Create (add) symbolic device **/
#define FT_PRM_DAB (1uLL << 58) /** Create (add) block device **/
#define FT_PRM_DAO (1uLL << 59) /** Create (add) socket **/
#define FT_PRM_DAP (1uLL << 60) /** Create (add) fifo **/

#define FT_PRM_DEX (1uLL << 63) /** Execute directory (chdir) **/

#define FT_PRM_DREA (FT_PRM_DRD | FT_PRM_DRA | FT_PRM_DRP | FT_PRM_DXA)               /** Directory read permission **/
#define FT_PRM_DWRI (FT_PRM_DAF | FT_PRM_DAD | FT_PRM_DAL | FT_PRM_DMK | FT_PRM_DMD | \
                     FT_PRM_DSL | FT_PRM_DCA | FT_PRM_DCP | FT_PRM_DCI | \
                     FT_PRM_DCT | FT_PRM_DCO | FT_PRM_DCG | FT_PRM_DOP | FT_PRM_DRM | \
                     FT_PRM_DMV | FT_PRM_DMX | FT_PRM_DCH | FT_PRM_DBL | FT_PRM_DSC | \
                     FT_PRM_DFF | FT_PRM_DAC | FT_PRM_DAB | FT_PRM_DAO | FT_PRM_DAP)   /** Directory write permission **/
#define FT_PRM_DEXE (FT_PRM_DEX)                                                       /** Directory exec permission **/
#define FT_PRM_DIRS (FT_PRM_DREA | FT_PRM_DWRI | FT_PRM_DEXE)                          /** Directory permission **/

#define FT_PRM_ALLS FT_PRM_FILE | FT_PRM_DIRS                                          /** All permissions **/
//}

/***********************  FTFS Data types  *************************/
//{

/**
* @struct ft_state
* @brief  handler for ft filesystem operated
* @var    ft_state::hostdir
*         Field 'hostdir' keeps real (absolute) path for host (real) ftfs directory
* @var    ft_state::datadir
*         Field 'datadir' keeps real (absolute) path for data subdirectory of hostdir
*         (where actual files and directories of filesystem is kept in host)
* @var    ft_state::db
*         Field 'db' is database handler (of NULL if xattr version is used)
* @var    ft_state::check_prexec
*         Field 'check_prexec' is 1 if executable process permission checking required, 0 otherwise
* @var    ft_state::type
*         Field 'type' store FTFS permission storing type
* @var    ft_state::uid
*         Field 'uid' store FTFS owner UID
* @var    ft_state::gid
*         Field 'gid' store FTFS owner GID
*/
typedef struct
{
    char    hostdir[FT_LIMIT_PATH];
    char    datadir[FT_LIMIT_PATH];
    sqlite3 *db;
    int      check_prexec;
    ft_type type;
    uid_t   uid;
    gid_t   gid;
} ft_state;

/** permission category **/
typedef enum
{
    FT_CAT_ALL, /**< ALL users                                  **/
    FT_CAT_OUS, /**< Owner USer                                 **/
    FT_CAT_OGR, /**< Owner Group                                **/
    FT_CAT_OTH, /**< OTHer (not owner's group)                  **/
    FT_CAT_UID, /**< User IDentifier                            **/
    FT_CAT_GID, /**< Group IDentifier                           **/
    FT_CAT_PEX  /**< Process EXecutable                         **/
} ft_cat;

/**
* @struct ft_prm
* @brief  represent single ftfs permission record
* @var    ft_prm::cat
*         Field 'cat' keeps permission category
* @var    ft_prm::uid
*         Union field 'srcs.uid' keeps user id, iff cat = FT_CTG_UID
* @var    ft_prm::gid
*         Union field 'srca.gid' keeps group id, iff cat = FT_CTG_GID
* @var    ft_prm::cmd
*         Union field 'srca.cmd' keeps process executable name, iff cat = FT_CTG_PEX
* @var    ft_prm::allow
*         Bitfield 'allow' keeps allowed permissions
* @var    ft_prm::deny
*         Bitfield 'deny' keeps allowed permissions
*/

typedef struct
{
    ft_cat cat;
    union
    {
        uid_t uid;
        gid_t gid;
        char *cmd;
    } prc;
    uint64_t allow;
    uint64_t deny;
} ft_prm;

/**
* @struct ft_prms
* @brief  represent array of permissions (dynamic)
* @var    ft_prms::data
*         Field 'data' is array of permissions
* @var    ft_prm::size
*         Field 'size' keeps number of permissions in array
*/
typedef struct
{
    ft_prm *array;
    size_t length;
} ft_prms;

/**
* @struct ft_prc
* @brief  ftfs accessing process detail
* @var    ft_prc::uid
*         Field 'uid' keeps user id
* @var    ft_prc::gid
*         Field 'gid' keeps group id
* @var    ft_prc::cmd
*         Field 'cmd' keeps process executable name
*/
typedef struct
{
    uid_t uid;
    gid_t gid;
    char  *cmd;
    gid_t groups[FT_LIMIT_GROUPS];
    int   ngroups;
} ft_prc;


/**
* @struct  ft_path
* @brief   ftfs file or directory path
* @warning incomplete type, not to be used directly
*/
typedef struct ft_path ft_path;

//}

/*********************  FTFS initialization  ***********************/
//{


/**
* @brief      initialize library
* @param[in]  name    program name
* @param[in]  version program version string
*/
void ft_init (const char *name, const char *version);

/**
* @brief      open existing ftfs system for operating
* @param[in]  hostdir path to host directory
* @param[in]  check_prexec should be non-zero if executable process permission checking required, 0 otherwise
* @param[out] state   pointer to pointer to a newly allocated ftfs handler or to NULL on fail
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_open   (const char *hostdir, int check_prexec, ft_state **state);

/**
* @brief         close ftfs
* @param[in,out] state ftfs handler, NULLs on exit
*/
void ft_close (ft_state **state);


/**
* @brief      create ftfs filesystem
* @param[in]  hostdir   host directory on real filesystem
* @param[in]  type      FT_XATTR  for xattr  version
*                       FT_SQLITE for sqlite version
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_create (const char *hostdir, ft_type type);

//}

/************  Permission check and inherit functions  *************/
//{

/**
* @brief      check specific permission
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  prc   accessing process attributes
* @param[in]  prm   permission to check (bitmask)
* @return     non-zero if all operations are permitted, 0 otherwise
* @warning    FT_PRM_DEX for all path is pre-checked before checking entry permission
*/
int ft_check_prm (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t prm);

/**
* @brief      check process permissions
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  prc   accessing process attributes
* @param[in]  susr  set to 1 if superuser and owner should got FT_PRMS_ALL, 0 to get actual permissions
* @return     allowed permission bitmask
* @warning    FT_PRM_DEX for all path is pre-checked before checking entry permission
*/
uint64_t ft_check_prms (ft_state *state, ft_path *path, const ft_prc *prc, int susr);


/**
* @brief      stat ftfs entry
* @param[in]  state   ftfs handler
* @param[in]  path    path to file or directory
* @param[in]  prc     accessing process attributes
* @return     pointer to stat buffer on success or NULL and set errno appropriately fail
*/
struct stat * ft_stat (ft_state *state, ft_path *path, const ft_prc *prc);

/**
* @brief      сheck inheritance or transmit flag
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  inh   inheritance bit to check
* @return     non-zero if flag is set, 0 otherwise
*/
int ft_check_inh (ft_state *state, ft_path *path, uint64_t inh);

/**
* @brief      сheck if entry exists in FTFS
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     non-zero if file exists, 0 otherwise
* @warning    ft_check_prm and ft_check_inh returns 0 on any file/dir access error
*             ft_exists is only way to pre-check file existence and separate no access and no file situation
*/
int ft_exists (ft_state *state, ft_path *path);

/**
* @brief      сheck if entry is directory in FTFS
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     1 if it is a directory, 0 otherwise
* @warning    if the entry doesn't exists, result in unpredictable
*/
int ft_is_dir (ft_state *state, ft_path* path);

/**
* @brief      check if dir or file operation is permitted
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  prc   accessing process attributes
* @param[in]  dprm  permission to check, if entry is a directory
* @return     non-zero if operation permitted, 0 otherwise
* @warning    FT_PRM_DEX for all path is pre-checked before checking entry permission
*/
int ft_check_prm2 (ft_state *state, ft_path *path, const ft_prc *prc, uint64_t fprm, uint64_t dprm);

//}

/*****************  Permission reading functions  ******************/
//{

/**
* @brief      get permissions (no inheritance check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  prm   permission record defining type, category and source of permission
* @param[out] prm   permission record with allow and deny bitfield refined
* @return     FT_OK on success or errorcode on fail
*/
int ft_get_prm (ft_state *state, ft_path *path, ft_prm *prm);

/**
* @brief      get permissions (no inheritance check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     array of permission records
* @warning    return array should be free'd by ft_prms_free()
*/
ft_prms ft_get_prms (ft_state *state, ft_path *path);

/**
* @brief      get inherited permissions (no recorded permissions loaded at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     array of permission records
* @warning    return array should be free'd by ft_prms_free()
*/
ft_prms ft_get_inhprms (ft_state *state, ft_path *path);

/**
* @brief      get joined (own and inherited permissions)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     array of permission records
* @warning    return array should be free'd by ft_prms_free()
*/
ft_prms ft_get_allprms (ft_state *state, ft_path *path);

/**
* @brief      get inheritance flags
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[out] inh   inheritance flags
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_get_inh (ft_state *state, ft_path *path, uint64_t *inh);

//}

/*****************  Permission setting functions  ******************/
//{

/**
* @brief      add, modify or delete (if allow=deny=0) permission record
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  prm   permission record to set
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_set_prm (ft_state *state, ft_path *path, const ft_prm *prm);

/**
* @brief      set file inheritance flags
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  inh   inheritance flags
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_set_inh (ft_state *state, ft_path *path, uint64_t inh);


//}


/*********************  Errors and messages  ***********************/
//{

/** Error codes **/
#define FT_ERR (1 << 13)   /** Error-code bit (message, if not set)                      **/
#define FT_PRG (1 << 14)   /** Program-level bit (this library level if not set)         **/


#define FT_ERR_MALLOC    (FT_ERR |  0)
#define FT_ERR_DBOPEN    (FT_ERR |  1)
#define FT_ERR_XATTROPEN (FT_ERR |  2)
#define FT_ERR_NODATA    (FT_ERR |  4)
#define FT_ERR_XATTRCRT  (FT_ERR |  5)
#define FT_ERR_XATTRFMT  (FT_ERR |  6)
#define FT_ERR_HOSTDIR   (FT_ERR |  7)
#define FT_ERR_HOSTEMPTY (FT_ERR |  8)
#define FT_ERR_HOSTERROR (FT_ERR |  9)
#define FT_ERR_TYPE      (FT_ERR | 10)
#define FT_ERR_DBCREATE  (FT_ERR | 11)
#define FT_ERR_DBQUERY   (FT_ERR | 12)
#define FT_ERR_PRMCAT    (FT_ERR | 13)
#define FT_ERR_XHASH     (FT_ERR | 14)
#define FT_ERR_PATH      (FT_ERR | 15)
#define FT_ERR_NOPARENT  (FT_ERR | 16)

/** Message codes **/

#define FT_OK                     0
#define FT_MSG_VERSION            1



extern const char * const ft_errlist[];
extern const char * const ft_msglist[];

extern const char * const ftprg_errlist[];
extern const char * const ftprg_msglist[];


/**
* @brief      get a message string by index
* @param[in]  idx is an index in ft_errlist or ft_msglist array if FT_PRG bit is unset
*                 is an index in ftprg_errlist or ftprg_msglist array otherwise
* @return     pointer to message (constant data, not needed to be freed
*/
const char * ft_msg (size_t idx);

/**
* @brief      print a formatted message by index
*             prints to stderr, if FT_ERR of idx is set to stdout  otherwise
* @param[in]  idx is an index in ft_errlist or ft_msglist array if FT_PRG bit is unset
*                 is an index in ftprg_errlist or ftprg_msglist array otherwise
* @return     pointer to message (constant data, not needed to be freed
*/
void ft_put_msg (size_t idx, ...);


/**
* @brief      print program and library version
*/
void ft_msg_version ();


//}

/********************  Entry adding and info  **********************/
//{
/**
* @brief      get file owner id (no permission check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[out] uid   owner uid
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_get_owner (ft_state *state, ft_path *path, uid_t *uid);

/**
* @brief      set file owner id (no permission check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  uid   new owner uid
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_set_owner (ft_state *state, ft_path *path, uid_t uid);

/**
* @brief      get file owner group id (no permission check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[out] uid   owner gid
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_get_group (ft_state *state, ft_path *path, gid_t *gid);

/**
* @brief      set file owner group id (no permission check at this point)
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @param[in]  gid   new owner gid
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_set_group (ft_state *state, ft_path *path, gid_t gid);

/**
* @brief      create file or directory (ftfs only)
* @param[in]  state ftfs handler
* @param[in]  path    path to file or directory to create
* @param[in]  uid     owner user id
* @param[in]  oid     owner group id
* @return     FT_OK (0) on success
*             ftfs error code on fail
* @warning    only ftfs synchronization provided, actual host fs operation should be done separately
*/
int ft_add (ft_state *state, ft_path *path, uid_t uid, gid_t gid);

/**
* @brief      remove file or directory (ftfs only)
* @param[in]  state ftfs handler
* @param[in]  path    path to file or directory to remove
* @return     FT_OK (0) on success
*             ftfs error code on fail
* @warning    only ftfs synchronization provided, actual host fs operation should be done separately
*/
int ft_delete (ft_state *state, ft_path *path);

/**
* @brief      rename file or directory inside filesystem (ftfs only)
* @param[in]  state ftfs handler
* @param[in]  path    path to file or directory to rename
* @param[in]  newpath new path of the file or directory
* @return     FT_OK (0) on success
*             ftfs error code on fail
* @warning    only ftfs synchronization provided, actual host renaming should be done separately
*/
int ft_rename (ft_state *state, ft_path *path, const char* newpath);

/**
* @brief      set inheritance and permission of newly created file
*             according to parent inheritance
* @param[in]  state ftfs handler
* @param[in]  path  path to file
* @return     FT_OK (0) on success
*             ftfs error code on fail
* @warning    ft_add should be called before and separately
*/
int ft_set_mkfile_prm (ft_state *state, ft_path *path);

/**
* @brief      set inheritance and permission of newly created directory
*             according to parent inheritance
* @param[in]  state ftfs handler
* @param[in]  path  path to directory
* @return     FT_OK (0) on success
*             ftfs error code on fail
* @warning    ft_add should be called before and separately
*/
int ft_set_mkdir_prm (ft_state *state, ft_path *path);

/**
* @brief      unset inheritance flag (FT_INH_INH)
*             and keep (copy) inherited permissions if needed
* @param[in]  state ftfs handler
* @param[in]  path  path to file or directory
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_unset_inh (ft_state *state, ft_path *path);

/**
* @brief      unset dir->file inheritance flag (FT_INH_IFP)
*             and keep (copy) inherited permissions if needed
* @param[in]  state ftfs handler
* @param[in]  path  path to directory
* @return     FT_OK (0) on success
*             ftfs error code on fail
*/
int ft_unset_ifp (ft_state *state, ft_path *path);
//}

/************************  Misc function  **************************/
//{

/**
* @brief      create correct ft_path variable
* @param[in]  path    static or preallocated path buffer containing ftfs-relative path (or initialized later)
* @return     ft_path variable
*/
ft_path ft_path_init(char *path);

/**
* @brief      get relative path from ft_path
* @param[in]  path    ft_path variable
* @return     relative FTFS entry path
*/
char *ft_get_path (ft_path *path);

/**
* @brief      get host (real FS) path from ft_path
* @param[in]  state   ftfs state
* @param[in]  path    ft_path variable
* @return     real (absolute FS) entry path
*/
char *ft_get_hostpath (ft_state *state, ft_path *path);


/**
 * @brief  fast type macro to get FTFS path in usual situation (from `path' variable)
 */
#define FT_PATH \
    ft_get_path(path)

/**
 * @brief  fast type macro to get FTFS hostpath in usual situation (from `state' and `path' variables)
 */
#define FT_HOSTPATH \
    ft_get_hostpath(state, path)

/**
* @brief      find parent ftfs directory
* @param[in]  path     file or directory
* @param[out] parent   parent directory (should be preinitialized properly)
* @return     1 if parent found, 0 if there is no parent, i.e. path is "/"
*/
int ft_parent(ft_path *path, ft_path *parent);

/**
* @brief      safe copy strings
* @param[in]  dest     destination string buffer
* @param[in]  source   source string
* @param[in]  size     buffer size (at most size-1 chars plus '\0' would be copied)
* @return     1 on success, 0 on buffer overflow (safe, dest is zero-length string)
*/
int ft_str_cpy (char *dest, const char *source, size_t size);

/**
* @brief      safe add string to the chosen position of another string
* @param[in]  dest     destination string buffer
* @param[in]  shift    starting position
* @param[in]  source   added string
* @param[in]  size     buffer size (including zero-char)
* @return     1 on success, 0 on buffer overflow (safe, dest is zero-length string)
*/
int ft_str_add (char *dest, size_t shift, const char *source, size_t size);

/**
* @brief      safe add string to the end of another string (concatenate)
* @param[in]  dest     destination string buffer
* @param[in]  source   added string
* @param[in]  size     buffer size (including zero-char)
* @return     1 on success, 0 on buffer overflow (safe, dest is zero-length string)
*/
int ft_str_cat (char *dest, const char *source, size_t size);


/**
* @brief      safe concatenate two strings
* @param[in]  dest     destination string buffer
* @param[in]  s1       first string
* @param[in]  s2       second string
* @param[in]  size     buffer size (including zero-char)
* @return     1 on success, 0 on buffer overflow (safe, dest is zero-length string)
*/
int ft_str_cat2 (char *dest, const char *s1, const char *s2, size_t size);


/**
* @brief      initialize empty ft_prms array
* @return     empty ft_prms array
*/
ft_prms ft_prms_init ();

/**
* @brief         add element to the end of ft_prms array
* @param[in,out] prms array to be altered
* @param[in]     prm  record to add
* @warning       in case of error (out of memory) prms sets to error-state array
*/
void ft_prms_add (ft_prms *prms, const ft_prm *prm);

/**
* @brief         remove element from ft_prms array
* @param[in,out] prms array to be altered
* @param[in]     i    index of record to remove
* @warning       in case of error (out of memory, index out of bounds) prms sets to error-state array
*/
void ft_prms_remove (ft_prms *prms, size_t i);

/**
* @brief         add element to the end of ft_prms array
* @param[in,out] prms array to be altered
* @param[in]     prm  record to add
* @warning       in case of error (out of memory) prms sets to error-state array
* @warning       if the record of the same type (cat and prmid) already exists,
*                record no added, allow and deny of existing record are joined using XOR
*/
void ft_prms_push (ft_prms *prms, const ft_prm *prm);

/**
* @brief         add all elements from source array to the end of prms array
* @param[in,out] prms   array to be altered
* @param[in]     source array of rerecords to be added
* @warning       in case of error (out of memory) prms sets to error-state array
* @warning       in case of records of duplicate cat and prmid,
*                one record with allow and deny are joined using XOR occurs
*/
void ft_prms_join (ft_prms *prms, const ft_prms *source);

/**
* @brief      free and set to empty prm array
*/
void ft_prms_free (ft_prms *prms);

/**
* @brief      create prms array in error state
* @return     prms array in error state
*/
ft_prms ft_prms_error ();

/**
* @brief      check if prms array is in error state
* @return     1 if prms array is in error state or 0 otherwise
*/
#define ft_prms_is_error(prms) (((prms)->length == -1uLL))

/**
* @brief     get element of non-constant prms array (safe)
* @param[in] prms array of elements
* @param[in] i    index of elements
* @return    pointer to element or NULL if element doesn't exists
*/
ft_prm * ft_prms_element (ft_prms *prms, size_t i);

/**
* @brief     get element of constant prms array (safe)
* @param[in] prms array of elements
* @param[in] i    index of elements
* @return    pointer to element or NULL if element doesn't exists
*/
const ft_prm * ft_prms_get (const ft_prms *prms, size_t i);

/**
* @brief     get size of prms array
* @param[in] prms array of elements
* @return    number of elements
*/
size_t ft_prms_size (const ft_prms *prms);

//}

/** cashed boolean **/
typedef enum
{
    FALSE = 0, /**< known to be false         */
    TRUE  = 1, /**< known to be true          */
    UNSET = -1 /**< unknown, to be calculated */
} ft_cbool;

struct ft_path
{
    char *path;
    char hostpath[FT_LIMIT_PATH];

    ft_cbool is_dir;

    ft_cbool exists;

    int inhset;
    uint64_t inh;

    ft_cbool statset;
    struct stat statbuf;

    int uidset;
    uid_t uid;

    int gidset;
    gid_t gid;
};


#endif // FTFS_H_INCLUDED
