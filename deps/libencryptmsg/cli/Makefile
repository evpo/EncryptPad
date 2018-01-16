#include ../deps/makefiles/platform.mak

LIBRARIES += ../src ../deps/state_machine ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
CXXFLAGS += -Wall -Wextra -std=c++11 $(BOTANCXX)
CXXFLAGS += -I../include -I../deps/plog/include
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
include ../deps/makefiles/gcc.mak
