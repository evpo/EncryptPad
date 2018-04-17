.PHONY: all clean
include ../build/config.mak
include ../$(DEPSDIR)/makefiles/platform.mak
include all_obj.mak
CPP_OBJECTS := $(ALL_OBJ)
OBJECTS := $(CPP_OBJECTS)

ifeq ($(RELEASE),on)
	CONF=release
else
	CONF=debug
endif

LIBRARY := ../bin/$(CONF)/libencryptmsg.a

$(LIBRARY): $(OBJECTS)
	@mkdir -p ../bin/$(CONF)
	@echo "$(LIBNAME):$(SUBDIR): Updating library $@"
	@$(AR) crs $(LIBRARY) $(OBJECTS)

all : $(LIBRARY)

clean:
	rm -f $(LIBRARY)


