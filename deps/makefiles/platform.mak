# -*- Makefile -*-
################################################################################
#
# Author: Andy Rushton
# Copyright: (c) Andy Rushton, 1999 onwards
# License:   BSD License, see docs/license.html
#
# Part of the generic makefile system - works out the current compilation platform
# This part is specific to the gcc compiler
# For use in gcc.mak
#
# New platforms should be added as required by adding more ifneq blocks
# The uname command usually gives a string like CYGWIN_NT-5.0 and I convert it to a simpler form like CYGWIN
# The rule is that every OS should map onto a different PLATFORM,
# furthermore, every OS type/version that needs a separate build should map onto a different BUILD,
# but OS versions that are binary-compatible with each other should map onto the same BUILD
#  - PLATFORM is the coarse-grain platform name used as a compiler directive e.g. LINUX
#  - BUILD is the fine-grain name used to differentiate between non-compatible objects = PLATFORM-CPU
#  - VARIANT is the kind of build - release/debug etc
#  - SUBDIR is the unique subdirectory name for object files = BUILD-VARIANT
#
################################################################################

# start by identifying the operating system
OS     := $(shell uname -o 2>/dev/null || uname)

# on most platforms "uname -m" gives the CPU name
# However, see below for situations where this is overridden
# Also, allow the command-line to override it
ifeq ($(CPU),)
CPU    := $(shell uname -m)
endif

# Windows builds

# MinGW build on Windows
# this is a native Windows build
ifneq ($(findstring Msys,$(OS)),)
PLATFORM  := MINGW
WINDOWS := on
endif

# Cygwin build on Windows
# this is a Unix emulation running on Windows so is classed as Unix for building purposes
ifneq ($(findstring Cygwin,$(OS)),)
PLATFORM  := CYGWIN
UNIX := on
endif

# Unix builds

# Build on GNU/Linux
ifneq ($(findstring GNU/Linux,$(OS)),)
PLATFORM  := GNULINUX
UNIX := on
endif

# Build on various flavours of BSD
ifneq ($(findstring FreeBSD,$(OS)),)
PLATFORM  := FREEBSD
UNIX := on
endif
ifneq ($(findstring OpenBSD,$(OS)),)
PLATFORM  := OPENBSD
UNIX := on
endif
ifneq ($(findstring NetBSD,$(OS)),)
PLATFORM  := NETBSD
UNIX := on
endif

# Build on Solaris - which identifies as SunOS
# Note: If I ever need to support SunOS 4 or earlier I'll have to differentiate between them somehow
#       However, this is very unlikely and would cause many other problems since SunOS is not Posix compliant
ifneq ($(findstring SunOS,$(OS)),)
PLATFORM  := SOLARIS
UNIX := on
endif

# Build on MacOS-X which identifies as Darwin
# Should this be identified as Unix?
ifneq ($(findstring Darwin,$(OS)),)
PLATFORM  := MACOS
CPU    := $(shell uname -p)
endif

# test for undefined platform
ifeq ($(PLATFORM),)
$(error you need to configure the make system for platform $(OS))
endif

BUILD := $(PLATFORM)-$(CPU)

################################################################################
# Calculate build variant
# there are four different build variants:
#   debug - for internal development (default)
#   release - for shipping to customers (switched on by environment variable RELEASE=on)
#   gprof - for performance profiling (switched on by environment variable GPROF=on)
#   gcov - for code coverage (switched on by environment variable GCOV=on)

ifeq ($(RELEASE),on)
# release variant
VARIANT  := release
else # RELEASE off
ifeq ($(GPROF),on)
# gprof variant
VARIANT  := gprof
else # GPROF off
ifeq ($(GCOV),on)
# gcov variant
VARIANT  := gcov
else # GCOV off
# debug variant
VARIANT  := debug
endif # GCOV
endif # GPROF
endif # RELEASE

# define the name of the subdirectory so that different builds have different subdirectories
SUBDIR := $(BUILD)-$(VARIANT)

################################################################################
