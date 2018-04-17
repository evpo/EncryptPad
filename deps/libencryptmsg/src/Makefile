include ../build/config.mak
include ../$(DEPSDIR)/makefiles/platform.mak
# Uncomment to add libraries
LIBRARIES += ../$(DEPSDIR)/state_machine ../$(DEPSDIR)/stlplus/containers
# ../$(DEPSDIR)/botan_1_openpgp_codec
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra

CXXFLAGS += -I./ -I../include/encryptmsg -I../$(DEPSDIR)/plog/include $(BOTAN_CXXFLAGS)
ifneq ($(OVERRIDE_BOTAN_FLAGS),on)
CXXFLAGS += $(shell pkg-config --cflags botan-2)
endif

LDFLAGS += -Wall

ifeq ($(BUILD_SHARED),on)
LDLIBS += -lm -lbotan-2
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

include ../$(DEPSDIR)/makefiles/gcc.mak
