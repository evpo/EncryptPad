include ../build/config.mak
include $(DEPSDIR)/makefiles/platform.mak

LIBRARIES += ../src $(DEPSDIR)/state_machine $(DEPSDIR)/stlplus/subsystems $(DEPSDIR)/stlplus/containers $(DEPSDIR)/stlplus/portability

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

ifeq ($(BOTANDIR),)
	BOTANCXX := $(shell pkg-config --cflags botan-2)
	BOTANLIB := $(shell pkg-config --libs botan-2)
else
	BOTANCXX := -I$(BOTANDIR)/build/include
	BOTANLIB := -L$(BOTANDIR) -lbotan-2 -fstack-protector -pthread
endif

CXXFLAGS += -Wall -Wextra -std=c++11 $(BOTANCXX)
CXXFLAGS += -I../include/encryptmsg -I$(DEPSDIR)/plog/include
LDFLAGS += -Wall
LDLIBS += -lm $(BOTANLIB)

#ifeq ($(RELEASE),on)
#TARGETDIR := ../../bin/release
#else
#TARGETDIR := ../../bin/debug
#endif

BINDIR := ../bin
BINFORM := variant
IMAGENAME := encryptmsg
#IMAGENAME := $(TARGETDIR)/encryptmsg
include $(DEPSDIR)/makefiles/gcc.mak
