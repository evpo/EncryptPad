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

LOADLIBES += ../deps/gtest/build/libgtest.a ../deps/stlplus/portability/$(SUBDIR)/libportability.a ../deps/botan/libbotan-1.10.a ../deps/zlib/libz.a -lpthread


CXXFLAGS += -I../deps/gtest/include -I../deps/botan/build/include -pthread -std=c++11 -Wall -Wextra
LDFLAGS += -Wall
IMAGENAME := encrypt_pad_tests
include ../deps/makefiles/gcc.mak
