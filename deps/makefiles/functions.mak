# -*- Makefile -*-
################################################################################
#
# Author: Andy Rushton
# Copyright: (c) Andy Rushton, 1999 onwards
# License:   BSD License, see docs/license.html
#
# Part of the generic makefile system - functions for generating names and directory structures
# Generic for use with multiple compilers
#
################################################################################

# function for determining the library name from the directory
# the library name is the containing directory 
# but if the containing directory is "source" (actually if it contains "source"), then the level above is used
library_name = $(if $(findstring source,$(notdir $(1))),$(notdir $(shell dirname $(1))),$(notdir $(1)))
# function for generating the archive name from the library name
archive_name = $(patsubst %,lib%.a,$(1))
# function for generating the subpath of an archive from the library name
archive_subpath = $(addprefix $(SUBDIR)/,$(call archive_name,$(1)))
# function for generating the full path to the archive from the library path
archive_path = $(addprefix $(1)/,$(call archive_subpath,$(call library_name,$(1))))
# function for determining whether a library has an archive or is a header-only library
# this returns the Makefile path if present and an empty string if not - so can be used in an if statement
archive_present = $(wildcard $(addsuffix /Makefile,$(1)))
# function for deciding whether to include a library - returns the library path if true, empty string if not
archive_library = $(if $(call archive_present,$(1)),$(1),)

# get the name of the library containing the code from this directory
LIBNAME := $(call library_name,$(shell pwd))
