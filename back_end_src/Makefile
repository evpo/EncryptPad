CRYPT_ROOT = ..
BOTAN = $(CRYPT_ROOT)/deps/botan
PORTABILITY = $(CRYPT_ROOT)/deps/stlplus/portability
ZLIB = $(CRYPT_ROOT)/deps/zlib
LIBRARIES :=
CXXFLAGS += -ggdb -Wall -Wextra
LDFLAGS += -Wall

CXXFLAGS += -I$(BOTAN)/build/include -I$(PORTABILITY) -I$(ZLIB) -std=c++11
include ../deps/makefiles/gcc.mak
