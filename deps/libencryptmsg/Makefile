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
	$(MAKE) -C src RELEASE=$(RELEASE) BOTAN_CXXFLAGS=$(BOTAN_CXXFLAGS)
	$(MAKE) -C deps/state_machine RELEASE=$(RELEASE) TRACE_STATE_MACHINE=$(TRACE_STATE_MACHINE)

clean:
	$(MAKE) -C tests -f Makefile.gtest clean RELEASE=$(RELEASE)
	$(MAKE) -C tests clean RELEASE=$(RELEASE)
	$(MAKE) -C cli clean RELEASE=$(RELEASE)
	$(MAKE) -C src clean RELEASE=$(RELEASE)
	$(MAKE) -C deps/state_machine clean RELEASE=$(RELEASE)
	$(MAKE) -C deps/botan_1_openpgp_codec clean RELEASE=$(RELEASE)
	$(MAKE) -C test_assets clean RELEASE=$(RELEASE)
