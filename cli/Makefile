include ../deps/makefiles/platform.mak

LIBRARIES += ../back_end_src ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

ifneq ($(RELEASE),on)
CXXFLAGS += -ggdb
endif

ifneq ($(PLATFORM),MACOS)
LDFLAGS += -static
endif

ifeq ($(PLATFORM),WINDOWS)
LDFLAGS += -lgdi32 -lkernel32 -luser32
endif

CXXFLAGS += -Wall -Wextra
LDFLAGS += -Wall

LOADLIBES += ../deps/botan/libbotan-1.10.a ../deps/zlib/libz.a -lpthread 
CXXFLAGS += -I../deps/botan/build/include -I../deps/zlib -pthread -std=c++11
IMAGENAME := encrypt_cli
include ../deps/makefiles/gcc.mak
