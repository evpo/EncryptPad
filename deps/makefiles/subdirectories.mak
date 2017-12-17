# -*- Makefile -*-
################################################################################
#
# Author: Andy Rushton
# Copyright: (c) Andy Rushton, 1999 onwards
# License:   BSD License, see docs/license.html
#
# Generic makefile that simply finds all subdirectories containing Makefiles and recurses on them
# include this in any Makefile where you want this behaviour
# For usage information see readme.txt
#
################################################################################

all build run tidy clean::
	@for m in */Makefile; do $(MAKE) -C `dirname $$m` $@; done

