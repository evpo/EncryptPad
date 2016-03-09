# -*- Makefile -*-
################################################################################
#
# Author: Daniel Milton
# Copyright: (c) Andy Rushton, Daniel Milton, 1999 onwards
# License:   BSD License, see docs/license.html
#
# Generic makefile for the setup of a bin directory.
# Uses BINFORM, BINDIR and IMAGENAME variable inputs
#
# IMAGENAME must be set for this makefile to do anything and is the simple name of the image (e.g. myprog)
# if IMAGENAME is not set then IMAGE must be present to link an image file
# IMAGE is the name and path of the image file - this can be a simple name or contain variables
#
# BINDIR specifies the base directory in which to place the binaries
#   if undefined, then the current working directory is used
#
# BINFORM specifies how to generate the binary (it's path/name)
#   = simple               - file generated as BINDIR/IMAGENAME with no renaming or subdirectory
#   = rename               - release: BINDIR/IMAGENAME, otherwise BINDIR/IMAGENAME-VARIANT
#   = renameall            - file generated as BINDIR/IMAGENAME-VARIANT
#   = variant              - file generated as BINDIR/VARIANT/IMAGENAME
#   = platform             - file generated as BINDIR/PLATFORM/IMAGENAME
#   =                      - file generated as BINDIR/SUBDIR/IMAGENAME
#
################################################################################

# this file must be called AFTER platform.mak, so that all the build vars are set up

# this must be set for this makefile to do anything
ifdef IMAGENAME

# use cwd if the bin directory isn't specified
ifndef BINDIR
  BINDIR := .
endif

# binary form
ifeq ($(BINFORM),simple)
  # binary is within the BINDIR
  IMAGE := $(BINDIR)/$(IMAGENAME)
endif
ifeq ($(BINFORM),rename)
  # binary is within the BINDIR and renamed for variant (not release variant though)
  ifeq ($(VARIANT),release)
    IMAGE := $(BINDIR)/$(IMAGENAME)
  else
    IMAGE := $(BINDIR)/$(IMAGENAME)-$(VARIANT)
  endif
endif
ifeq ($(BINFORM),renameall)
  # binary is within the BINDIR and renamed for variant
  IMAGE := $(BINDIR)/$(IMAGENAME)-$(VARIANT)
endif
ifeq ($(BINFORM),variant)
  # binary is within the BINDIR in a subdirectory determined by variant
  IMAGE := $(BINDIR)/$(VARIANT)/$(IMAGENAME)
endif
ifeq ($(BINFORM),platform)
  # binary is within the BINDIR in a subdirectory determined by platform
  IMAGE := $(BINDIR)/$(PLATFORM)/$(IMAGENAME)
endif
ifeq ($(IMAGE),)
  # default - binary is within the build subdirectory
  IMAGE := $(BINDIR)/$(SUBDIR)/$(IMAGENAME)
endif

endif

