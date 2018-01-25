include ../deps/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += ../deps/state_machine ../deps/stlplus/containers ../deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

CXXFLAGS += -I./ -I../include/encryptmsg -I../deps/plog/include $(BOTAN_CXXFLAGS)
ifneq ($(OVERRIDE_BOTAN_FLAGS),on)
CXXFLAGS += $(shell pkg-config --cflags botan-2)
endif

LDFLAGS += -Wall
include ../deps/makefiles/gcc.mak
