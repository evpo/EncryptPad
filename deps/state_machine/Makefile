# Uncomment to add libraries
DEPS = ..
LIBRARIES += $(DEPS)/stlplus/containers
CXXFLAGS += -ggdb -std=c++11 -Wall -Wextra
CXXFLAGS += -I$(DEPS)/plog/include
ifeq ($(TRACE_STATE_MACHINE),on)
    CXXFLAGS += -DTRACE_STATE_MACHINE
endif
LDFLAGS += -Wall
include $(DEPS)/makefiles/gcc.mak
