include ../build/config.mak
include $(DEPSDIR)/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += $(DEPSDIR)/state_machine $(DEPSDIR)/stlplus/containers
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

CXXFLAGS += -I./ -I../include/encryptmsg -I$(DEPSDIR)/plog/include $(BOTAN_CXXFLAGS)

ifeq ($(BOTANDIR),)
CXXFLAGS += $(shell pkg-config --cflags botan-2)
else
CXXFLAGS += -I$(BOTANDIR)/build/include
endif

LDFLAGS += -Wall

ifeq ($(BUILD_SHARED),on)

ifeq ($(BOTANDIR),)
	LDLIBS += $(shell pkg-config --cflags botan-2)
else
	LDLIBS += -L$(BOTANDIR) -lbotan-2 -fstack-protector -lm -lz
endif

ifeq ($(RELEASE),on)
TARGETDIR := ../bin/release
else
TARGETDIR := ../bin/debug
endif

ifeq ($(WINDOWS),on)
SHARED_SO := $(TARGETDIR)/encryptmsg.dll
else
SHARED_SO := $(TARGETDIR)/libencryptmsg.so
endif

endif

include $(DEPSDIR)/makefiles/gcc.mak
