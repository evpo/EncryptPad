# -*- Makefile -*-
################################################################################
#
# Author: Andy Rushton
# Copyright: (c) Andy Rushton, 2007 onwards
# License:   BSD License, see docs/license.html
#
# Generic makefile extensions for building wxWidgets projects with the Gnu tools
# include this after the gcc.mak makefile
# Uses the wx-config script to get the specific details for this installation.
# For usage information see docs/index.html
#
################################################################################

ifeq ($(GPROF),on)
# gprof variant
$(error "wxWidgets doesn't support a profiling build")
endif

ifeq ($(GCOV),on)
# gcov variant
$(error "wxWidgets doesn't support a code coverage build")
endif

# default to unicode on, must switch off explicitly by setting the variable UNICODE=off before this is included
# Note: this is really obsolete from wx v2.9 onwards - all builds are now Unicode builds
UNICODE_FLAG := --unicode=yes
ifeq ($(UNICODE),off)
UNICODE_FLAG := --unicode=no
endif

# default to debug build, must switch release build on explicitly
DEBUG_FLAG := --debug=yes
ifeq ($(RELEASE),on)
DEBUG_FLAG := --debug=no
endif

# default to shared build since this is also the wxWidgets default
# However, I recommend a static build because it is the least
# problematic when distributing programs this also removes the
# dependencies on gcc libraries
STATIC_FLAG := --static=no
ifeq ($(WXSTATIC),on)
STATIC_FLAG := --static=yes
endif

# allow a specific version to be selected
VERSION_FLAG :=
ifneq ($(WXVERSION),)
VERSION_FLAG := --version=$(WXVERSION)
endif

# configure the compilation tools for the project to match those required by the wxWidgets library being used
CXX := `wx-config $(VERSION_FLAG) $(DEBUG_FLAG) $(UNICODE_FLAG) $(STATIC_FLAG) --cxx`
CXXFLAGS += `wx-config $(VERSION_FLAG) $(DEBUG_FLAG) $(UNICODE_FLAG) $(STATIC_FLAG) --cxxflags`
LOADLIBES += `wx-config $(VERSION_FLAG) $(DEBUG_FLAG) $(UNICODE_FLAG) $(STATIC_FLAG) --libs`
RC := `wx-config $(VERSION_FLAG) $(DEBUG_FLAG) $(UNICODE_FLAG) $(STATIC_FLAG) --rescomp`

ifeq ($(PLATFORM),MINGW)
# the mingw32 library maps the gcc main onto the Windows WinMain - so seed the link with this file
LDFLAGS += -lmingw32
endif

