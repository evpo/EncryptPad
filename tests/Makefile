include ../deps/makefiles/platform.mak

LIBRARIES += ../back_end_src

ifeq ($(PLATFORM),MACOS)
LDFLAGS += -all_load
else
LDFLAGS += -Wl,--whole-archive
LOADLIBES += -Wl,--no-whole-archive
#-dl
endif

ifeq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

LOADLIBES += ../deps/gtest/build/libgtest.a ../deps/stlplus/portability/$(SUBDIR)/libportability.a -lpthread
CXXFLAGS += -I../deps/gtest/include -pthread -std=c++11 -Wall -Wextra

ifneq ($(USE_SYSTEM_LIBS),on)
LOADLIBES += ../deps/botan/libbotan-1.10.a ../deps/zlib/libz.a
CXXFLAGS += -I../deps/botan/build/include -I../deps/zlib
else
BOTANLIB := $(shell pkg-config --libs botan-1.10)
BOTANCXX := $(shell pkg-config --cflags botan-1.10)
LOADLIBES += $(BOTANLIB) -lz
CXXFLAGS += $(BOTANCXX)
endif

LDFLAGS += -Wall
IMAGENAME := encrypt_pad_tests
include ../deps/makefiles/gcc.mak
