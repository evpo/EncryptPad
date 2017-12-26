# Uncomment to add libraries
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra
CXXFLAGS += -I./ $(shell pkg-config --cflags botan-2)
# Enable to see the state machine transition
LDFLAGS += -Wall
include ../makefiles/gcc.mak
