CRYPT_ROOT = ..
ifneq ($(USE_SYSTEM_LIBS),on)
	BOTAN = -I$(CRYPT_ROOT)/deps/botan/build/include
else
	BOTAN = $(shell pkg-config --cflags botan-2)
endif
LIBRARIES := ../deps/state_machine ../deps/stlplus/portability ../deps/stlplus/containers ../deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -Wall -Wextra
LDFLAGS += -Wall

CXXFLAGS += $(BOTAN) -I../deps/libencryptmsg/include -I../deps/plog/include -std=c++11
CXXFLAGS += -D_FILE_OFFSET_BITS=64
include ../deps/makefiles/gcc.mak
