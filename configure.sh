#!/bin/bash

USAGE="USAGE:\n\
configure.sh <command> [option]\n\n\
COMMANDS:\n\
-a, --all\tbuild everything required to run the application including Botan\n\
-c, --clean\tclean the application build files\n\
-r, --run\trun the application\n\
-b, --botan\tbuild Botan\n\
-o, --clean_botan\tclean Botan\n\
-e, --back_end\tbuild the back end with CLI\n\
-u, --tests\tbuild the unit tests\n\
-t, --run_tests\trun the unit tests\n\
-n, --clean_tests\tclean the unit tests\n\
-h, --help\thelp\n\n\
OPTIONS:\n\
--debug\tdebug configuration. If not specified, the release configuration is used. The unit tests are always built with the debug configuration."

TARGET=EncryptPad
TEST_TARGET=encrypt_pad_tests

if [[ $# > 2 ]] || [[ $# < 1 ]]
then
    echo Invalid parameters >&2
    echo -e $USAGE
    exit -1
fi

COMMAND=$1
shift

case $1 in
    -d|--debug) 
        RELEASE=
        QT_BIN_SUB=debug
        ;;
    *) 
        RELEASE=on
        QT_BIN_SUB=release
        ;;
esac

UNAME=`uname`
MAKE=make
if [[ $UNAME == *MINGW* ]]
then
    MAKE=mingw32-make
fi

pushd ./build >/dev/null
SUBDIR=`./get_subdir.sh`

case $COMMAND in
-a|--all) $MAKE -f Makefile.botan
    $MAKE -f Makefile RELEASE=$RELEASE
    if [[ $SUBDIR == *MACOS* ]]
    then
        cd ../macos_deployment && ./prepare_bundle.sh ../build/qt_build/${QT_BIN_SUB}/${TARGET}.app
    fi
    ;;
-c|--clean) 
    $MAKE -f Makefile.qt_ui clean RELEASE=$RELEASE 
    $MAKE -f Makefile clean RELEASE=$RELEASE 

    if [[ $SUBDIR == *MACOS* ]]
    then
        rm -Rf ./qt_build/${QT_BIN_SUB}/${TARGET}.app
    elif [[ $UNAME == *MINGW* ]]
    then
        rm -f ./qt_build/${QT_BIN_SUB}/${TARGET}.exe
    else
        rm -f ./qt_build/${QT_BIN_SUB}/${TARGET}
    fi
    ;;
-r|--run) 
    if [[ $SUBDIR == *MACOS* ]]
    then
        ./qt_build/${QT_BIN_SUB}/${TARGET}.app/Contents/MacOS/${TARGET} &
    else
        ./qt_build/${QT_BIN_SUB}/${TARGET} &
    fi
    ;;
-b|--botan) $MAKE -f Makefile.botan ;;
-o|--clean_botan) $MAKE -f Makefile.botan clean ;;
-e|--back_end) 
    $MAKE -f Makefile.back_end RELEASE=$RELEASE 
    $MAKE -f Makefile.cli RELEASE=$RELEASE 
    ;;
-u|--tests) $MAKE -f Makefile.unit_tests ;;
-n|--clean_tests) $MAKE -f Makefile.unit_tests clean ;;
-t|--run_tests) 
    # Unit tests should run from tests directory because they need files the directory contains
    pushd ../tests >/dev/null
    ./${SUBDIR}/${TEST_TARGET}
    RESULT=$?
    popd >/dev/null
    
    if [[ $RESULT != 0 ]] 
    then 
        popd >/dev/null
        exit $RESULT
    fi

    # Functional tests
    # pushd ../func_tests >/dev/null
    # ./decryption_test.sh ../cli/${SUBDIR}/encrypt_cli gpg_encrypted
    # RESULT=$?
    # popd >/dev/null

    popd >/dev/null
    exit $RESULT
    ;;
-h|--help) echo -e $USAGE ;;
*)  echo -e "$COMMAND is invalid parameter" >&2
    echo -e $USAGE
    popd >/dev/null
    exit -1
    ;;
esac
RESULT=$?
popd >/dev/null
exit $RESULT
