include ../deps/makefiles/platform.mak

LIBRARIES += ../back_end_src ../deps/libencryptmsg/src ../deps/libencryptmsg/state_machine ../deps/libencryptmsg/state_machine_client ../deps/libencryptmsg/deps/botan_1_openpgp_codec

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

CXXFLAGS += -I../deps/libencryptmsg/include

LDLIBS += ../deps/gtest/googletest/build/libgtest.a ../deps/stlplus/portability/$(SUBDIR)/libportability.a -lpthread
CXXFLAGS += -I../deps/gtest/googletest/include -pthread -std=c++11 -Wall -Wextra

ifneq ($(USE_SYSTEM_LIBS),on)
LDLIBS += ../deps/botan/libbotan-1.10.a ../deps/zlib/libz.a
CXXFLAGS += -I../deps/botan/build/include -I../deps/zlib
else
BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
LDLIBS += $(BOTANLIB) -lz
CXXFLAGS += $(BOTANCXX)
endif

LDFLAGS += -Wall

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/encrypt_pad_tests
include ../deps/makefiles/gcc.mak
