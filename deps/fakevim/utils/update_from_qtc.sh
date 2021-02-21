#!/bin/bash
qtc_home=$1
qtc_fakevim_home=$qtc_home/src/plugins/fakevim/
qtc_utils_home=$qtc_home/src/libs/utils/

die() {
    echo "$1" 1>&2
    exit 1
}

up() {
    local src=$1
    local dst=$2
    rsync "$src" "$dst" || exit
}

set -e

[ -n "$qtc_home" ] ||
    die "Usage: $0 PATH_TO_QT_CREATOR"

[ -d "$qtc_fakevim_home" ] ||
    die "Cannot find FakeVim plugin in \"$qtc_fakevim_home\"!"

( cd fakevim
up {"$qtc_fakevim_home",}fakevimactions.cpp
up {"$qtc_fakevim_home",}fakevimactions.h
up {"$qtc_fakevim_home",}fakevimhandler.cpp
up {"$qtc_fakevim_home",}fakevimhandler.h
up {"$qtc_fakevim_home",}fakevimtr.h
)

#( cd fakevim/utils
#up {"$qtc_utils_home",}hostosinfo.h
#up {"$qtc_utils_home",}qtcassert.cpp
#up {"$qtc_utils_home",}qtcassert.h
#up {"$qtc_utils_home",}utils_global.h
#)

