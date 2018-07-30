# Automatically generated by configure - do not modify
# Configured with: './configure' '--target-list=arm-softmmu'
all:
prefix=/usr/local
bindir=${prefix}/bin
libdir=${prefix}/lib
mandir=${prefix}/share/man
datadir=${prefix}/share/qemu
sysconfdir=${prefix}/etc
docdir=${prefix}/share/doc/qemu
confdir=${prefix}/etc/qemu
ARCH=x86_64
STRIP=strip
HOST_LONG_BITS=64
CONFIG_POSIX=y
CONFIG_LINUX=y
CONFIG_SLIRP=y
CONFIG_SMBD_COMMAND="/usr/sbin/smbd"
CONFIG_AC97=y
CONFIG_ES1370=y
CONFIG_SB16=y
CONFIG_HDA=y
CONFIG_AUDIO_DRIVERS=oss
CONFIG_OSS=y
CONFIG_BDRV_WHITELIST=
CONFIG_VNC=y
CONFIG_FNMATCH=y
VERSION=0.15.1
PKGVERSION=
SRC_PATH=/home/cws/perf_sim/qemu_M4
TARGET_DIRS=arm-softmmu
CONFIG_ATFILE=y
CONFIG_UTIMENSAT=y
CONFIG_PIPE2=y
CONFIG_ACCEPT4=y
CONFIG_SPLICE=y
CONFIG_EVENTFD=y
CONFIG_FALLOCATE=y
CONFIG_SYNC_FILE_RANGE=y
CONFIG_FIEMAP=y
CONFIG_DUP3=y
CONFIG_EPOLL=y
CONFIG_EPOLL_CREATE1=y
CONFIG_EPOLL_PWAIT=y
CONFIG_INOTIFY=y
CONFIG_INOTIFY1=y
CONFIG_BYTESWAP_H=y
GLIB_CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include  
INSTALL_BLOBS=yes
CONFIG_IOVEC=y
CONFIG_PREADV=y
CONFIG_FDT=y
CONFIG_SIGNALFD=y
CONFIG_GCC_ATTRIBUTE_WARN_UNUSED_RESULT=y
CONFIG_FDATASYNC=y
CONFIG_MADVISE=y
CONFIG_POSIX_MADVISE=y
CONFIG_SMARTCARD=y
CONFIG_OPENGL=y
CONFIG_UNAME_RELEASE=""
CONFIG_ZERO_MALLOC=y
HOST_USB=linux
TRACE_BACKEND=nop
CONFIG_TRACE_FILE=trace
TOOLS=qemu-ga$(EXESUF) qemu-nbd$(EXESUF) qemu-img$(EXESUF) qemu-io$(EXESUF) 
ROMS=optionrom
MAKE=make
INSTALL=install
INSTALL_DIR=install -d -m 0755
INSTALL_DATA=install -c -m 0644
INSTALL_PROG=install -c -m 0755
PYTHON=python
CC=gcc
CC_I386=$(CC) -m32
HOST_CC=gcc
AR=ar
OBJCOPY=objcopy
LD=ld
WINDRES=windres
LIBTOOL=libtool
CFLAGS=-O2 -g 
QEMU_CFLAGS=-m64 -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wstrict-prototypes -Wredundant-decls -Wall -Wundef -Wwrite-strings -Wmissing-prototypes -fno-strict-aliasing  -fstack-protector-all -Wendif-labels -Wmissing-include-dirs -Wempty-body -Wnested-externs -Wformat-security -Wformat-y2k -Winit-self -Wignored-qualifiers -Wold-style-declaration -Wold-style-definition -Wtype-limits
QEMU_INCLUDES=-I$(SRC_PATH)/slirp -I. -I$(SRC_PATH)
HELPER_CFLAGS=
LDFLAGS=-Wl,--warn-common -m64 -g 
ARLIBS_BEGIN=
ARLIBS_END=
LIBS+=-lrt -pthread 
LIBS_TOOLS+=-lglib-2.0   
EXESUF=
subdir-arm-softmmu: subdir-libhw32
