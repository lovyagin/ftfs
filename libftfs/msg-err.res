/*
  Copyright (C) 2016 Roman Y. Dayneko, <dayneko3000@gmail.com>
                2017 Nikita Yu. Lovyagin, <lovyagin@mail.com>

  This program can be distributed under the terms of the GNU GPLv3.
  See the file COPYING.
*/

"malloc error, out of memory?",                                     /* FT_ERR_MALLOC    */
"could not open fs.db, file access error or database corrupt",      /* FT_ERR_DBOPEN    */
"could not find xattr or hostdir/data, either filesystem moved to "
    "xattr-unsupported hostdir or its fs.db lost",                  /* FT_ERR_XATTROPEN */
"could not open log file",                                          /* FT_ERR_LOG       */
"could not open hostdir/data directory",                            /* FT_ERR_NODATA    */
"could not create xattr attribute, possible xattr unsupported",     /* FT_ERR_XATTRCRT  */
"bad xattr, ftfs bug or host filesystem altered wrong way",         /* FT_ERR_XATTRFMT  */
"fail creating/accessing host directory not a directory?",          /* FT_ERR_HOSTDIR   */
"host directory not empty",                                         /* FT_ERR_HOSTEMPTY */
"could not access host directory, access restricted or I/O error",  /* FT_ERR_HOSTERROR */
"bad FTFS type, bug in FTFS application",                           /* FT_ERR_TYPE      */
"could not create FTFS database, access error?",                    /* FT_ERR_DBCREATE  */
"could not process DB query, something wrong in fsdb database "
    "or ftfs bug",                                                  /* FT_ERR_DBQUERY   */
"strange permission category, FTFS bug?",                           /* FT_ERR_PRMCAT    */
"XATTR hash limit reached, too many exec permissions",              /* FT_ERR_XHASH     */
"file path length limit reached, too long file path",               /* FT_ERR_PATH      */
"could not find parent directory, probably FTFS bug?",              /* FT_ERR_NOPARENT  */
