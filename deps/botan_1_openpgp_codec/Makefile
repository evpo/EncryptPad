ifneq ($(USE_SYSTEM_LIBS),on)
	BOTAN = -I../../deps/botan/build/include
else
	BOTAN = $(shell pkg-config --cflags botan-2)
endif

# Uncomment to add libraries
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra
CXXFLAGS += -I./ $(BOTAN)
# Enable to see the state machine transition
LDFLAGS += -Wall
include ../makefiles/gcc.mak
