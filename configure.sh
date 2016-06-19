#!/bin/bash

USAGE="USAGE:\n\
configure.sh <command> [option]\n\n\
COMMANDS:\n\
-a, --all            build everything required to run the application\n\
-c, --clean          clean the application build files\n\
-r, --run            run the application\n\
-b, --botan          build Botan\n\
-o, --clean-botan    clean Botan\n\
-e, --back-end       build the back end with CLI\n\
-u, --tests          build the unit tests\n\
-t, --run-tests      run the unit tests\n\
-f, --run-func-tests run functional tests\n\
-n, --clean-tests    clean the unit tests\n\
-h, --help           help\n\n\
OPTIONS:\n\
--debug              debug configuration. If not specified, the release configuration is used. The unit tests\n\
                     are always built with the debug configuration.\n\
--use-system-libs    use botan, zlib and other shared libraries installed on the system."

TARGET=EncryptPad
TEST_TARGET=encrypt_pad_tests

if [[ $# > 3 ]] || [[ $# < 1 ]]
then
    echo Invalid parameters >&2
    echo -e "$USAGE"
    exit -1
fi

UNAME=`uname`
MAKE=make
if [[ $UNAME == *MINGW* ]]
then
    MAKE=mingw32-make
fi

pushd ./build >/dev/null
SUBDIR=`./get_subdir.sh`

RELEASE=on
QT_BIN_SUB=release

while [[ $# > 0 ]]
do
    case $1 in
        -d|--debug)
            RELEASE=
            QT_BIN_SUB=debug
            ;;
        --use-system-libs)
            if [[ $UNAME == *MINGW* ]]
            then
                echo "--use-system-libs is not supported in MINGW"
                exit -1
            fi
            USE_SYSTEM_LIBS=on
            ;;
        *)
            COMMAND=$1
            ;;
    esac
    shift
done

CONFIG_DIR=release
if [[ ! "$RELEASE" == "on" ]]
then
    CONFIG_DIR=debug
fi

case $COMMAND in
-a|--all)
    if [[ ! "$USE_SYSTEM_LIBS" == "on" ]]
    then
        $MAKE -f Makefile.botan
    fi
    $MAKE -f Makefile RELEASE=$RELEASE USE_SYSTEM_LIBS=$USE_SYSTEM_LIBS
    if [[ $SUBDIR == *MACOS* ]]
    then
        cd ../macos_deployment && ./prepare_bundle.sh ../bin/${CONFIG_DIR}/${TARGET}.app
    fi
    ;;
-c|--clean)
    $MAKE -f Makefile.qt_ui clean RELEASE=$RELEASE 
    $MAKE -f Makefile clean RELEASE=$RELEASE 

    if [[ $SUBDIR == *MACOS* ]]
    then
        rm -Rf ../bin/${CONFIG_DIR}/${TARGET}.app
        rm -f ../bin/${CONFIG_DIR}/encryptcli
    elif [[ $UNAME == *MINGW* ]]
    then
        rm -f ../bin/${CONFIG_DIR}/${TARGET}.exe
        rm -f ../bin/${CONFIG_DIR}/encryptcli.exe
    else
        rm -f ../bin/${CONFIG_DIR}/${TARGET}
        rm -f ../bin/${CONFIG_DIR}/encryptcli
    fi
    ;;
-r|--run)
    if [[ $SUBDIR == *MACOS* ]]
    then
        ../bin/${CONFIG_DIR}/${TARGET}.app/Contents/MacOS/${TARGET} &
    else
        ../bin/${CONFIG_DIR}/${TARGET} &
    fi
    ;;
-b|--botan) $MAKE -f Makefile.botan ;;
-o|--clean-botan) $MAKE -f Makefile.botan clean ;;
-e|--back-end)
    $MAKE -f Makefile.back_end RELEASE=$RELEASE USE_SYSTEM_LIBS=$USE_SYSTEM_LIBS
    $MAKE -f Makefile.cli RELEASE=$RELEASE USE_SYSTEM_LIBS=$USE_SYSTEM_LIBS
    ;;
-u|--tests) $MAKE -f Makefile.unit_tests USE_SYSTEM_LIBS=$USE_SYSTEM_LIBS;;
-n|--clean-tests)
    $MAKE -f Makefile.unit_tests clean 
    rm -f ../bin/debug/${TEST_TARGET}
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
    pushd ../tests >/dev/null
    ../bin/debug/${TEST_TARGET}
    RESULT=$?
    popd >/dev/null

    if [[ $RESULT != 0 ]] 
    then 
        popd >/dev/null
        exit $RESULT
    fi

    # Functional tests
    # pushd ../func_tests >/dev/null
    # ./decryption_test.sh ../bin/${CONFIG_DIR}/encryptcli gpg_encrypted
    # RESULT=$?
    # popd >/dev/null

    popd >/dev/null
    exit $RESULT
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
