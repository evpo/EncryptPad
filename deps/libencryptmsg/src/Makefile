include ../deps/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += ../deps/state_machine ../deps/stlplus/containers ../deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

ifndef $(BOTAN_CXXFLAGS)
    BOTAN_CXXFLAGS=$(shell pkg-config --cflags botan-2)
endif

CXXFLAGS += -I./ -I../include -I../deps/plog/include $(BOTAN_CXXFLAGS)
LDFLAGS += -Wall
include ../deps/makefiles/gcc.mak
