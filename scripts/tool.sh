#!/usr/bin/env bash
set -e
set -o pipefail

USAGE="USAGE:\n\
configure.sh <command> [option]\n\n\
COMMANDS:\n\
-r, --run                       run the application\n\
-t, --run-tests                 run the unit tests\n\
-f, --run-func-tests            run functional tests\n\
--update-ts                     update ts files from pro, C++ and ui files (step 1)\n\
--generage-qm                   build qm files from ts files to update culture resources (step 2)\n\
--import-from-fakevim-ts <DIR>  import translation for FakeVim from DIR\n\
--appimage                      build AppImage (requires AppImageKit and binaries)\n\
--docs                          build docs directory from markdown files (requres the markdown utility)\n\
--update-htm                    update htm files (README.htm and CHANGES.htm)\n\
--import-emsg <DIR>             import latest libencryptmsg code from DIR\n\
--icns                          generate icns from iconset
-h, --help                      help\n\n\
OPTIONS:\n\
--debug                         debug configuration. If not specified, the release configuration is used. The unit tests\n\
                                are always built with the debug configuration.\n\
"

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pkg_root=$script_dir/..

TARGET=encryptpad
OSX_APP=EncryptPad.app
TEST_TARGET=encryptpad_test

if [[ $# > 3 ]] || [[ $# < 1 ]]
then
    echo Invalid parameters >&2
    echo -e "$USAGE"
    exit -1
fi

UNAME=`uname -o 2>/dev/null || uname`


COMMAND=$1
shift

CONFIG_DIR=release
case $1 in
    -d|--debug)
        CONFIG_DIR=debug
        ;;
esac

case $COMMAND in
--update-ts)
    lupdate $pkg_root/qt_ui/ -no-ui-lines -locations none -ts $pkg_root/qt_ui/encryptpad_*.ts
    ;;
--generate-qm)
    for TSFILE in $pkg_root/qt_ui/encryptpad_*.ts $pkg_root/qt_ui/fakevim_*.ts $pkg_root/qt_ui/qt_excerpt_*.ts
    do
        file_name=${TSFILE##*/}
        CULTUREFILE=${file_name%.ts}
        if [[ $CULTUREFILE == encryptpad_en_gb || $CULTUREFILE == fakevim_en_gb ]]
        then
            continue
        fi

        echo $CULTUREFILE
        lrelease $TSFILE -qm $pkg_root/qt_ui/${CULTUREFILE}.qm
    done
    ;;
--appimage)
    $pkg_root/linux_deployment/prepare-appimage.sh $pkg_root/build/qt_deployment
    mkdir -p $pkg_root/bin
    mv $pkg_root/build/qt_deployment/encryptpad.AppImage $pkg_root/bin/
    ;;
-r|--run)
    if [[ $UNAME == *Darwin* ]]
    then
        $pkg_root/bin/${CONFIG_DIR}/${OSX_APP}/Contents/MacOS/${TARGET} &
    else
        $pkg_root/bin/${CONFIG_DIR}/${TARGET} &
    fi
    ;;
-f|--run-func-tests)
    pushd $pkg_root/func_tests >/dev/null
    ./run_all_tests.sh ../bin/${CONFIG_DIR}/encryptcli
    result=$?
    popd >/dev/null
    exit $result
    ;;
-t|--run-tests)
    # Unit tests should run from tests directory because they need files the directory contains
    pushd $pkg_root/src/test
    ../../bin/debug/${TEST_TARGET}
    RESULT=$?
    popd >/dev/null
    exit $RESULT
    ;;
--docs)
    mkdir -p $pkg_root/bin
    mkdir -p $pkg_root/bin/docs
    $pkg_root/contrib/markdown2web $pkg_root/docs $pkg_root/bin/docs
    ;;
--update-htm)
    sed 1,/cutline/d $pkg_root/README.md > /tmp/tmp_cut_readme.md
    markdown /tmp/tmp_cut_readme.md > $pkg_root/README.htm
    cp /tmp/tmp_cut_readme.md $pkg_root/docs/en/readme.md
    rm /tmp/tmp_cut_readme.md
    cp $pkg_root/CHANGES.md $pkg_root/docs/en/changes.md
    markdown $pkg_root/CHANGES.md > $pkg_root/CHANGES.htm
    ;;
--import-emsg)
    import_repo="$1"
    rm -rf deps/libencryptmsg
    mkdir deps/libencryptmsg
    git --work-tree=deps/libencryptmsg/ --git-dir=${import_repo}/.git checkout -f
    rm -r deps/libencryptmsg/deps
    result=$?
    exit $result
    ;;
--import-from-fakevim-ts)
    python3 $pkg_root/scripts/import-from-fakevim-ts.py "$1" $pkg_root/qt_ui
    ;;
--icns)
    iconutil -c icns -o images/crypt.icns images/icns.iconset
    exit 0
    ;;
-h|--help)
    echo -e "$USAGE"
    ;;

*)  echo -e "$COMMAND is invalid parameter" >&2
    echo -e "$USAGE"
    exit -1
    ;;
esac
