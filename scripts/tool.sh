#!/usr/bin/env bash
set -e
set -o pipefail

USAGE="USAGE:\n\
configure.sh <command> [option]\n\n\
COMMANDS:\n\
-r, --run            run the application\n\
-t, --run-tests      run the unit tests\n\
-f, --run-func-tests run functional tests\n\
--culture-res        build qm files from ts files to update culture resources\n\
--appimage           build AppImage (requires AppImageKit and binaries)\n\
--docs               build docs directory from markdown files (requres the markdown utility)\n\
--update-htm         update htm files (README.htm and CHANGES.htm)\n\
-h, --help           help\n\n\
OPTIONS:\n\
--debug              debug configuration. If not specified, the release configuration is used. The unit tests\n\
                     are always built with the debug configuration.\n\
"

TARGET=encryptpad
OSX_APP=EncryptPad.app
TEST_TARGET=encrypt_pad_test

if [[ $# > 3 ]] || [[ $# < 1 ]]
then
    echo Invalid parameters >&2
    echo -e "$USAGE"
    exit -1
fi

UNAME=`uname -o 2>/dev/null || uname`

pushd ./build >/dev/null

CONFIG_DIR=release
QT_BIN_SUB=release

while [[ $# > 0 ]]
do
    case $1 in
        -d|--debug)
            QT_BIN_SUB=debug
            CONFIG_DIR=debug
            ;;
        *)
            COMMAND=$1
            ;;
    esac
    shift
done

case $COMMAND in
--culture-res)
    for TSFILE in ../qt_ui/encryptpad_*.ts ../qt_ui/qt_excerpt_*.ts
    do
        CULTUREFILE=$(echo -n "$TSFILE" | sed -n -e "s/..\/qt_ui\///" -e "s/\.ts$//p")
        if [[ $CULTUREFILE == encryptpad_en_gb ]]
        then
            continue
        fi

        echo $CULTUREFILE
        lrelease $TSFILE -qm ../qt_ui/${CULTUREFILE}.qm
    done
    ;;
--appimage)
    USE_SYSTEM_LIBS=on
    build_all
    ../linux_deployment/prepare-appimage.sh ./qt_deployment
    mkdir -p ../bin
    mv ./qt_deployment/encryptpad.AppImage ../bin/
    ;;
-r|--run)
    if [[ $UNAME == *Darwin* ]]
    then
        ../bin/${CONFIG_DIR}/${OSX_APP}/Contents/MacOS/${TARGET} &
    else
        ../bin/${CONFIG_DIR}/${TARGET} &
    fi
    ;;
-f|--run-func-tests)
    pushd ../func_tests >/dev/null
    ./run_all_tests.sh ../bin/${CONFIG_DIR}/encryptcli
    RESULT=$?
    popd >/dev/null
    exit $RESULT
    ;;
-t|--run-tests)
    # Unit tests should run from tests directory because they need files the directory contains
    # pushd ../src/test >/dev/null
    pushd ../src/test
    ../../bin/debug/${TEST_TARGET}
    RESULT=$?
    popd >/dev/null

    if [[ $RESULT != 0 ]] 
    then 
        popd >/dev/null
        exit $RESULT
    fi

    popd >/dev/null
    exit $RESULT
    ;;
--docs)
    mkdir -p ../bin
    mkdir -p ../bin/docs
    ../contrib/markdown2web ../docs ../bin/docs
    ;;
--update-htm)
    sed 1,/cutline/d ../README.md > /tmp/tmp_cut_readme.md
    markdown /tmp/tmp_cut_readme.md > ../README.htm
    cp /tmp/tmp_cut_readme.md ../docs/en/readme.md
    rm /tmp/tmp_cut_readme.md
    cp ../CHANGES.md ../docs/en/changes.md
    markdown ../CHANGES.md > ../CHANGES.htm
    ;;
-h|--help)
    echo -e "$USAGE"
    ;;

*)  echo -e "$COMMAND is invalid parameter" >&2
    echo -e "$USAGE"
    popd >/dev/null
    exit -1
    ;;
esac

RESULT=$?
popd >/dev/null
exit $RESULT
