#!/bin/bash

QMAKE=qmake
QMAKE_SPEC=linux-g++
QMAKE_CONFIG="release USE_SYSTEM_LIBS"

mkdir -p build/qt_build
pushd build/qt_build
${QMAKE} -r -spec ${QMAKE_SPEC} "CONFIG+=${QMAKE_CONFIG}" ../../qt_ui/EncryptPad.pro
popd
