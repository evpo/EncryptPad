#!/bin/bash

MAKE=make
if [[ `uname` == *MINGW* ]]
then
    MAKE=mingw32-make
fi

$MAKE -f Makefile.platform $1| sed -n "s/SUBDIR=\(.*\)/\1/p"
