include ../deps/makefiles/platform.mak

LIBRARIES += ../src ../deps/state_machine
LDFLAGS += -Wl,--whole-archive
LDLIBS += -Wl,--no-whole-archive

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
LDLIBS += ../deps/gtest/googletest/build/libgtest.a $(BOTANLIB) -lpthread -lm
CXXFLAGS += -I../deps/gtest/googletest/include -pthread -std=c++11 -Wall -Wextra
CXXFLAGS += -I../include/encryptmsg -I../include/internal -I../deps/stlplus/containers -I../deps/plog/include $(BOTANCXX)

LDFLAGS += -Wall

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/encryptmsg-tests
include ../deps/makefiles/gcc.mak
