lib_LIBRARIES = libscdramsim2.a

libscdramsim2_a_SOURCES = \
	\
	dram_ctrl.cpp \
	dram_ctrl.h \
	\
	hier_param.cpp \
	hier_param.h 

AM_LIBTOOLFLAGS = --preserve-dup-deps

if COMPILE_M32_FLAG 
libscdramsim2_a_LIBADD = $(SYSTEMCROOT)/lib-linux/libsystemc.a 
else
libscdramsim2_a_LIBADD = $(SYSTEMCROOT)/lib-linux64/libsystemc.a 
endif

libscdramsim2_a_LIBADD += $(DRAMSim2_LIB)/libdramsim.so
#libscdramsim2_a_LIBADD += $(VERDIROOT)/share/FsdbWriter/LINUX_GNU_32/libnffw.a #IC_S_2013_08_22
#libscdramsim2_a_LIBADD += $(VERDIROOT)/share/PLI/systemc/osci22/lib-linux_gcc3_4x/libfsdbSC.a #IC_S_2013_08_22 

if COMPILE_M32_FLAG 
libscdramsim2_a_CXXFLAGS = -DSC_INCLUDE_DYNAMIC_PROCESSES -pthread -m32 
else
libscdramsim2_a_CXXFLAGS = -DSC_INCLUDE_DYNAMIC_PROCESSES -pthread
endif

#libscdramsim2_a_CXXFLAGS = -DSC_INCLUDE_DYNAMIC_PROCESSES -DPRINT_STATUS -pthread 
libscdramsim2_a_CXXFLAGS += $(AM_CXXFLAGS)

INCLUDES = @M2S_INCLUDES@
INCLUDES += @TOP_INCLUDES@
INCLUDES += -I$(SYSTEMCROOT)/include -I$(DRAMSim2_LIB)
#INCLUDES += -I$(VERDIROOT)/share/PLI/systemc/osci22/include #IC_S_2013_08_22 
