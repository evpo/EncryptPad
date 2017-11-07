.PHONY: src tests cli gtest
all:: src cli
gtest::
	$(MAKE) -C tests -f Makefile.gtest RELEASE=$(RELEASE)

tests:: gtest src
	$(MAKE) -C tests RELEASE=$(RELEASE)
	$(MAKE) -C test_assets RELEASE=$(RELEASE)

cli:: src
	$(MAKE) -C cli RELEASE=$(RELEASE)

src:: symlinks
	$(MAKE) -C src RELEASE=$(RELEASE)

clean::
	$(MAKE) -C tests -f Makefile.gtest clean RELEASE=$(RELEASE)
	$(MAKE) -C tests clean RELEASE=$(RELEASE)
	$(MAKE) -C cli clean RELEASE=$(RELEASE)
	$(MAKE) -C src clean RELEASE=$(RELEASE)
	$(MAKE) -C test_assets clean RELEASE=$(RELEASE)

symlinks::
	ln -s ../../makefiles deps/
	ln -s ../../stlplus deps/
	ln -s ../../gtest deps/
