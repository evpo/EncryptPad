CRYPT_ROOT = ..
ifneq ($(USE_SYSTEM_LIBS),on)
	BOTAN = -I$(CRYPT_ROOT)/deps/botan/build/include
else
	BOTAN = $(shell pkg-config --cflags botan-2)
endif
LIBRARIES := ../deps/libencryptmsg/src ../deps/stlplus/portability ../deps/libencryptmsg/deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -Wall -Wextra
LDFLAGS += -Wall

CXXFLAGS += $(BOTAN) -I../deps/libencryptmsg/include -std=c++11
include ../deps/makefiles/gcc.mak
