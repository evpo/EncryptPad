CRYPT_ROOT = ..
PORTABILITY = -I$(CRYPT_ROOT)/deps/stlplus/portability
ifneq ($(USE_SYSTEM_LIBS),on)
	BOTAN = -I$(CRYPT_ROOT)/deps/botan/build/include
	ZLIB = -I$(CRYPT_ROOT)/deps/zlib
else
	BOTAN = $(shell pkg-config --cflags botan-1.10)
endif
LIBRARIES :=
CXXFLAGS += -ggdb -Wall -Wextra
LDFLAGS += -Wall

CXXFLAGS += $(BOTAN) $(PORTABILITY) $(ZLIB) -std=c++11
include ../deps/makefiles/gcc.mak
