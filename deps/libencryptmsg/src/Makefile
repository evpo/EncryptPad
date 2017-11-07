# Uncomment to add libraries
LIBRARIES += ../deps/stlplus/containers
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra
CXXFLAGS += -I./ -I../include -I../deps/plog/include -I/usr/include/botan-2
# Enable to see the state machine transition
CXXFLAGS += -DTRACE_STATE_MACHINE
LDFLAGS += -Wall
include ../deps/makefiles/gcc.mak
