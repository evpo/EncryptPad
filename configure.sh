#!/usr/bin/env bash
set -e
set -o pipefail

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
--culture-res        build qm files from ts files to update culture resources\n\
--appimage           build AppImage (requires AppImageKit and binaries)\n\
--docs               build docs directory from markdown files (requres the markdown utility)\n\
--update-htm         update htm files (README.htm and CHANGES.htm)\n\
-h, --help           help\n\n\
OPTIONS:\n\
--debug              debug configuration. If not specified, the release configuration is used. The unit tests\n\
                     are always built with the debug configuration.\n\
--use-system-libs    use botan, zlib and other shared libraries installed on the system."

TARGET=encryptpad
OSX_APP=EncryptPad.app
TEST_TARGET=encrypt_pad_tests

if [[ $# > 3 ]] || [[ $# < 1 ]]
then
    echo Invalid parameters >&2
    echo -e "$USAGE"
    exit -1
fi

UNAME=`uname`
MAKE="${MAKE:-make}"
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

function build_all {
    if [[ ! "$USE_SYSTEM_LIBS" == "on" ]]
    then
        $MAKE -f Makefile.botan
    fi
    $MAKE -f Makefile RELEASE=$RELEASE USE_SYSTEM_LIBS=$USE_SYSTEM_LIBS LOCALIZATION=$LOCALIZATION
    if [[ $SUBDIR == *MACOS* ]]
    then
        pushd ../macos_deployment > /dev/null
        ./prepare_bundle.sh ../bin/${CONFIG_DIR}/${OSX_APP}
        popd
    fi
}

case $COMMAND in
-a|--all)
    mkdir -p qt_build
    build_all
    ;;
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
-c|--clean)
    $MAKE -f Makefile.qt_ui clean RELEASE=$RELEASE 
    $MAKE -f Makefile clean RELEASE=$RELEASE 

    if [[ $SUBDIR == *MACOS* ]]
    then
        rm -Rf ../bin/${CONFIG_DIR}/${OSX_APP}
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
        ../bin/${CONFIG_DIR}/${OSX_APP}/Contents/MacOS/${TARGET} &
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
