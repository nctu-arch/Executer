bin_PROGRAMS = $(top_builddir)/bin/m2s
__top_builddir__bin_m2s_SOURCES = \
	m2sSC.cpp \
	m2sSC.h \
	Main.cpp \
	Main.h \
	Top_module.cpp \
	Top_module.h 

__top_builddir__bin_m2s_LDADD = $(top_builddir)/src/libm2s.a

if COMPILE_M32_FLAG 
__top_builddir__bin_m2s_LDADD += $(SYSTEMCROOT)/lib-linux/libsystemc.a
else
__top_builddir__bin_m2s_LDADD += $(SYSTEMCROOT)/lib-linux64/libsystemc.a
endif

__top_builddir__bin_m2s_LDADD += $(DRAMSim2_LIB)/libdramsim.so
#__top_builddir__bin_m2s_LDADD += $(VERDIROOT)/share/FsdbWriter/LINUX_GNU_32/libnffw.a #IC_S_2013_08_22
#__top_builddir__bin_m2s_LDADD += $(VERDIROOT)/share/PLI/systemc/osci22/lib-linux_gcc3_4x/libfsdbSC.a #IC_S_2013_08_22

AM_LIBTOOLFLAGS = --preserve-dup-deps
__top_builddir__bin_m2s_CXXFLAGS = $(AM_CXXFLAGS)
#__top_builddir__bin_m2s_CXXFLAGS += -DSC_INCLUDE_DYNAMIC_PROCESSES -DPRINT_STATUS

if COMPILE_M32_FLAG 
__top_builddir__bin_m2s_CXXFLAGS += -DSC_INCLUDE_DYNAMIC_PROCESSES -m32 #IC_S_2013_08_22
else
__top_builddir__bin_m2s_CXXFLAGS += -DSC_INCLUDE_DYNAMIC_PROCESSES
endif

INCLUDES = @M2S_INCLUDES@
INCLUDES += @TOP_INCLUDES@
INCLUDES += -I$(SYSTEMCROOT)/include/ -I$(DRAMSim2_LIB) 
#INCLUDES += -I$(VERDIROOT)/share/PLI/systemc/osci22/include #IC_S_2013_08_22

__top_builddir__bin_m2s_LDADD += \
	$(top_builddir)/src/arch/x86/timing/libtiming.a \
	$(top_builddir)/src/arch/x86/emu/libemu.a \
	$(top_builddir)/src/arch/x86/asm/libasm.a \
	\
	$(top_builddir)/src/arch/arm/timing/libtiming.a \
	$(top_builddir)/src/arch/arm/emu/libemu.a \
	$(top_builddir)/src/arch/arm/asm/libasm.a \
	\
	$(top_builddir)/src/arch/evergreen/timing/libtiming.a \
	$(top_builddir)/src/arch/evergreen/emu/libemu.a \
	$(top_builddir)/src/arch/evergreen/asm/libasm.a \
	\
	$(top_builddir)/src/arch/fermi/timing/libtiming.a \
	$(top_builddir)/src/arch/fermi/emu/libemu.a \
	$(top_builddir)/src/arch/fermi/asm/libasm.a \
	\
	$(top_builddir)/src/arch/southern-islands/timing/libtiming.a \
	$(top_builddir)/src/arch/southern-islands/emu/libemu.a \
	$(top_builddir)/src/arch/southern-islands/asm/libasm.a \
	\
	$(top_builddir)/src/arch/common/libcommon.a \
	\
	$(top_builddir)/src/dram/libdram.a \
	$(top_builddir)/src/mem-system/libmemsystem.a \
	$(top_builddir)/src/network/libnetwork.a \
	\
	$(top_builddir)/src/peripheral_systemc/libsystemcif.a \
	\
	$(top_builddir)/src/visual/common/libcommon.a \
	$(top_builddir)/src/visual/evergreen/libevergreen.a \
	$(top_builddir)/src/visual/memory/libmemory.a \
	$(top_builddir)/src/visual/southern-islands/libsouthernislands.a \
	$(top_builddir)/src/visual/x86/libx86.a \
	$(top_builddir)/src/visual/common/libcommon.a \
	\
	$(top_builddir)/src/driver/x86/glut/libglut.a \
	\
	$(top_builddir)/src/lib/mhandle/libmhandle.a \
	$(top_builddir)/src/lib/esim/libesim.a \
	$(top_builddir)/src/lib/util/libutil.a \
	$(top_builddir)/src/lib/Arch_stat/libArch_stat.a \
	\
	$(top_builddir)/src/sc_dramsim2/libscdramsim2.a \
	$(top_builddir)/src/trace_driven/libtracedriven.a \
	$(top_builddir)/src/trace_driven/include_shm/libshm.a

if HAVE_GTK
AM_CFLAGS = @GTK_CFLAGS@
__top_builddir__bin_m2s_LDADD += @GTK_LIBS@
endif

if HAVE_OPENGL
__top_builddir__bin_m2s_LDADD += @OPENGL_LDFLAGS@
endif

__top_builddir__bin_m2s_LDADD += -lpthread -lz -lm
