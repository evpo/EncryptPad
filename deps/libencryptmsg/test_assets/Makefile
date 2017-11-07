
ifeq ($(RELEASE),on)
TARGETDIR := ../bin/release
else
TARGETDIR := ../bin/debug
endif

FILES := $(wildcard *.gpg)
FILES += $(wildcard *.txt)

TARGETFILES := $(patsubst %,$(TARGETDIR)/%,$(FILES))

.PHONY: all build clean FORCE

all:: build

build:: $(TARGETFILES)

$(TARGETDIR)/% : %
	cp "$<" "$@"

clean::
	rm -f $(TARGETFILES)
