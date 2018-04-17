include ../build/config.mak
include ../$(DEPSDIR)/makefiles/platform.mak

LIBRARIES += ../src ../$(DEPSDIR)/state_machine
LDFLAGS += -Wl,--whole-archive
LDLIBS += -Wl,--no-whole-archive

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
LDLIBS += ../$(DEPSDIR)/gtest/googletest/build/libgtest.a $(BOTANLIB) -lpthread -lm
CXXFLAGS += -I../$(DEPSDIR)/gtest/googletest/include -pthread -std=c++11 -Wall -Wextra
CXXFLAGS += -I../include/encryptmsg -I../include/internal -I../$(DEPSDIR)/stlplus/containers -I../$(DEPSDIR)/plog/include $(BOTANCXX)

LDFLAGS += -Wall

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/encryptmsg-tests
include ../$(DEPSDIR)/makefiles/gcc.mak
