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

THIS_MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(THIS_MAKEFILE_DIR)/platform.mak

# default to debug build, must switch release build on explicitly
WX_DEBUG_FLAG := --debug=yes
ifeq ($(RELEASE),on)
WX_DEBUG_FLAG := --debug=no
endif

# default to shared build since this is also the wxWidgets default
# However, I recommend a static build because it is the least
# problematic when distributing programs this also removes the
# dependencies on gcc libraries
WX_STATIC_FLAG := --static=no
ifeq ($(WXSTATIC),on)
WX_STATIC_FLAG := --static=yes
endif

# allow a specific version to be selected
WX_VERSION_FLAG :=
ifneq ($(WXVERSION),)
WX_VERSION_FLAG := --version=$(WXVERSION)
endif

# WX v3.0 headers produce a lot of deprecated warnings, swamping anything from the user's code,
# so for now disable deprecated warnings
# Note that this will also switch off warnings about problems in the user's code
CXXFLAGS += -Wno-deprecated-declarations

# configure the compilation tools for the project to match those required by the wxWidgets library being used
CXX := `wx-config $(WX_VERSION_FLAG) $(WX_DEBUG_FLAG) $(WX_STATIC_FLAG) --cxx`
CXXFLAGS += `wx-config $(WX_VERSION_FLAG) $(WX_DEBUG_FLAG) $(WX_STATIC_FLAG) --cxxflags`
LDLIBS += `wx-config $(WX_VERSION_FLAG) $(WX_DEBUG_FLAG) $(WX_STATIC_FLAG) --libs`
RC := `wx-config $(WX_VERSION_FLAG) $(WX_DEBUG_FLAG) $(WX_STATIC_FLAG) --rescomp`

ifeq ($(PLATFORM),MINGW)
# the mingw32 library maps the gcc main onto the Windows WinMain - so seed the link with this file
LDFLAGS += -lmingw32
endif

