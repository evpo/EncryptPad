.PHONY: src tests cli gtest symlinks clean-symlinks

include ../../deps/makefiles/platform.mak


all: src cli
gtest:
	$(MAKE) -C tests -f Makefile.gtest RELEASE=$(RELEASE)

tests: gtest cli src
	$(MAKE) -C tests RELEASE=$(RELEASE)
	$(MAKE) -C test_assets RELEASE=$(RELEASE)

cli: src
	$(MAKE) -C cli RELEASE=$(RELEASE)

lib: src

src: symlinks 
	$(MAKE) -C src RELEASE=$(RELEASE)
	$(MAKE) -C state_machine RELEASE=$(RELEASE)
	$(MAKE) -C state_machine_client RELEASE=$(RELEASE)

clean: clean-children | clean-symlinks
clean-children: symlinks
	$(MAKE) -C tests -f Makefile.gtest clean RELEASE=$(RELEASE)
	$(MAKE) -C tests clean RELEASE=$(RELEASE)
	$(MAKE) -C cli clean RELEASE=$(RELEASE)
	$(MAKE) -C src clean RELEASE=$(RELEASE)
	$(MAKE) -C state_machine clean RELEASE=$(RELEASE)
	$(MAKE) -C state_machine_client clean RELEASE=$(RELEASE)
	$(MAKE) -C test_assets clean RELEASE=$(RELEASE)

LNDIRNAME=makefiles stlplus gtest botan
LNDIR=$(LNDIRNAME:%=deps/%)
CLEANLNDIR=$(LNDIR:%=clean-%)

.PHONY: $(CLEANLNDIR)

symlinks: $(LNDIR)

clean-symlinks: $(CLEANLNDIR)

$(LNDIR):
ifneq ($(WINDOWS),on)
	ln -s ../../$(@:deps/%=%) deps/
else
	cp -r ../../$(@) deps/
endif

$(CLEANLNDIR):
	rm $(@:clean-%=%)
