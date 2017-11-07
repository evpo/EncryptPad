#include ../deps/makefiles/platform.mak

LIBRARIES += ../src ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

CXXFLAGS += -Wall -Wextra -std=c++11
CXXFLAGS += -I../include -I../deps/plog/include -I/usr/include/botan-2
LDFLAGS += -Wall
LOADLIBES += -lm -lbotan-2

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
