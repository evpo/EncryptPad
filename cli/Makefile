include ../deps/makefiles/platform.mak

LIBRARIES += ../back_end_src ../deps/libencryptmsg/src ../deps/libencryptmsg/state_machine ../deps/libencryptmsg/state_machine_client ../deps/libencryptmsg/deps/botan_1_openpgp_codec ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

ifneq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

ifneq ($(USE_SYSTEM_LIBS),on)
ifneq ($(PLATFORM),MACOS)
LDFLAGS += -static
endif
endif

ifeq ($(PLATFORM),WINDOWS)
LDFLAGS += -lgdi32 -lkernel32 -luser32
endif

CXXFLAGS += -Wall -Wextra
LDFLAGS += -Wall

CXXFLAGS += -I../deps/libencryptmsg/include -I../deps/libencryptmsg/deps/plog/include

ifneq ($(USE_SYSTEM_LIBS),on)
LDLIBS += ../deps/botan/libbotan-1.10.a ../deps/zlib/libz.a -lpthread
CXXFLAGS += -I../deps/botan/build/include -I../deps/zlib -pthread -std=c++11
else
BOTANLIB := $(shell pkg-config --libs botan-2)
BOTANCXX := $(shell pkg-config --cflags botan-2)
LDLIBS += $(BOTANLIB) -lz -lpthread
CXXFLAGS += $(BOTANCXX) -pthread -std=c++11
endif

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

IMAGENAME := $(TARGETDIR)/encryptcli
include ../deps/makefiles/gcc.mak
