# ftfs
Fine-tuning file system

Based on original works by Roman Dayneko (https://github.com/dayneko3000/Diploma and https://github.com/dayneko3000/prm-control).

tools:
    libftfs	ftfs routines library
    mkftfs	utility to create ftfs filesystem
    ftpc	tool to control ftfs permissions
    ftfs	ftfs fuse driver (mount ftfs filesystem)

make
    clean 	clean built files
    all		compile
    remake	clean and all
    install	compile and install to /usr/local
    reuinstall	clean and install (no uninstall!)'
    uninstall	uninstall from /usr/local
    
req:		fuse, sqlite
buildreq:	fuse-devel, sqlite-devel


make DEBUG=1 builds a debug configuration (default)
make DEBUG=0 builds release module
    
WARNING: make from root directory loops over all tools but building (the 'all' target) requires libftfs to be installed to /usr/local first
         just run 'make DEBUG=0 install' to get full ftfs package build and installed

build and install ftfs: 
    sudo make DEBUG=0 install

information about fine permissions could be found at
    ftpc --help
