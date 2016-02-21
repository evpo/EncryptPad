RELEASE := on
all:: back_end cli
	$(MAKE) -f Makefile.qt_ui
back_end::
	$(MAKE) -f Makefile.back_end RELEASE=$(RELEASE)
cli:
	$(MAKE) -f Makefile.cli RELEASE=$(RELEASE)
clean::
	$(MAKE) -f Makefile.qt_ui clean
	$(MAKE) -f Makefile.cli clean RELEASE=$(RELEASE)
	$(MAKE) -f Makefile.back_end clean RELEASE=$(RELEASE)
