# ftfs
Fine-tuning file system

Based on original works by Roman Dayneko (https://github.com/dayneko3000/Diploma and https://github.com/dayneko3000/prm-control).

tools:
    libftfs	ftfs routines library
    mkftfs	utility to create ftfs filesystem
    ftpc	tool to control ftfs permissions
    ftfs	ftfs fuse driver (mount ftfs filesystem)

req:		fuse, sqlite
buildreq:	fuse-devel, sqlite-devel

information about fine permissions could be found at
    ftpc --help
