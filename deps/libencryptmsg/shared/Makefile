LIBRARIES += ../src ../deps/stlplus/subsystems ../deps/stlplus/containers ../deps/stlplus/portability

LDFLAGS += -Wall
LOADLIBES += -lm -lbotan-2

ifeq ($(RELEASE),on)
TARGETDIR := ../../bin/release
else
TARGETDIR := ../../bin/debug
endif

SHARED := encryptmsg
include ../deps/makefiles/gcc.mak
