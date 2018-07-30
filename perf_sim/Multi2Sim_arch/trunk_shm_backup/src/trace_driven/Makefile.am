SUBDIRS = \
	include_shm

lib_LIBRARIES = libtracedriven.a
libtracedriven_a_SOURCES = \
	trace_driven.cpp \
	trace_driven.h 

#libtracedriven_a_LIBADD = /home/hchung/Multi2Sim_arch/trunk_validate/src/mem-system/libmemsystem.a /home/hchung/Multi2Sim_arch/trunk_validate/src/lib/util/libutil.a ./include_shm/libshm.a
libtracedriven_a_LIBADD = ../mem-system/libmemsystem.a ../lib/util/libutil.a ./include_shm/libshm.a
INCLUDES = @M2S_INCLUDES@

