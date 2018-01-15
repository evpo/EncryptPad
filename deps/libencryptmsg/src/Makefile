include ../deps/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += ../state_machine ../state_machine_client ../deps/stlplus/containers ../deps/botan_1_openpgp_codec
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

ifneq ($(USE_SYSTEM_LIBS),on)
BOTAN=-I../deps/botan/build/include
else
BOTAN=$(shell pkg-config --cflags botan-2)
endif

CXXFLAGS += -I./ -I../include -I../deps/plog/include $(BOTAN)
# Enable to see the state machine transition
CXXFLAGS += -DTRACE_STATE_MACHINE
LDFLAGS += -Wall
include ../deps/makefiles/gcc.mak
