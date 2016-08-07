all:: back_end cli
	$(MAKE) -f Makefile.qt_ui RELEASE=$(RELEASE) USE_SYSTEM_LIBS=$(USE_SYSTEM_LIBS) LOCALIZATION=$(LOCALIZATION)
back_end::
	$(MAKE) -f Makefile.back_end RELEASE=$(RELEASE) USE_SYSTEM_LIBS=$(USE_SYSTEM_LIBS)
cli:
	$(MAKE) -f Makefile.cli RELEASE=$(RELEASE) USE_SYSTEM_LIBS=$(USE_SYSTEM_LIBS)
clean::
	$(MAKE) -f Makefile.qt_ui clean
	$(MAKE) -f Makefile.cli clean RELEASE=$(RELEASE)
	$(MAKE) -f Makefile.back_end clean RELEASE=$(RELEASE)
