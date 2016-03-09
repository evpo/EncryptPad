# -*- Makefile -*-
################################################################################
#
# Author: Andy Rushton, Daniel Milton
# Copyright: (c) Andy Rushton, Daniel Milton, 1999 onwards
# License:   BSD License, see docs/license.html
#
# Generic makefile for building whole projects with the gcc compiler tools
# For usage information see docs/index.html
#
################################################################################

# get this makefile's directory, so we can use it to include the other
# makefiles in the same directory
THIS_MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# include all the makefile components
include $(THIS_MAKEFILE_DIR)/platform.mak
include $(THIS_MAKEFILE_DIR)/functions.mak
include $(THIS_MAKEFILE_DIR)/image.mak
include $(THIS_MAKEFILE_DIR)/build.mak
