include ../deps/makefiles/platform.mak

LIBRARIES += ../back_end_src ../deps/state_machine ../deps/botan_1_openpgp_codec

ifeq ($(PLATFORM),MACOS)
LDFLAGS += -all_load
else
LDFLAGS += -Wl,--whole-archive
LDLIBS += -Wl,--no-whole-archive
#-dl
endif

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

ifeq ($(WINDOWS),on)
LDFLAGS += -static
endif

CXXFLAGS += -I../deps/libencryptmsg/include
CXXFLAGS += -I../deps/plog/include

LDLIBS += ../deps/gtest/googletest/build/libgtest.a ../deps/stlplus/portability/$(SUBDIR)/libportability.a -lpthread
CXXFLAGS += -I../deps/gtest/googletest/include -pthread -std=c++11 -Wall -Wextra

LDLIBS += ../deps/libencryptmsg/src/$(SUBDIR)/libsrc.a

ifeq ($(WINDOWS),on)
LDLIBS += -lgdi32 -lkernel32 -luser32
BOTANLIBFILE = ../deps/botan/botan.lib -fstack-protector
else
BOTANLIBFILE = ../deps/botan/libbotan-2.a -lm
LIBDL = -ldl
endif

ifneq ($(USE_SYSTEM_LIBS),on)
LDLIBS += $(BOTANLIBFILE) ../deps/zlib/libz.a -lpthread $(LIBDL)
CXXFLAGS += -I../deps/botan/build/include -I../deps/zlib -pthread -std=c++11
else
BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
LDLIBS += $(BOTANLIB) -lz
CXXFLAGS += $(BOTANCXX)
endif

ifeq ($(PLATFORM),MACOS)
LDLIBS += -framework Security
endif

LDFLAGS += -Wall

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/encrypt_pad_tests
include ../deps/makefiles/gcc.mak
