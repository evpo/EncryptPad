#!/bin/bash
set -e

USAGE="prepare-appimage.sh <work-directory>"

# work-directory
# |
# |-AppDir
# |-AppImage

if [[ $# < 1 ]]
then
    echo $USAGE
    exit -1
fi

if ! which appimagetool > /dev/null
then
    echo "Cannot find appimagetool"
    exit -1
fi

if ! which linuxdeployqt > /dev/null
then
    echo "Cannot find linuxdeployqt"
    exit -1
fi

WORKDIR=$1
mkdir -p $WORKDIR
rm -R $WORKDIR/AppDir || true
rm -R $WORKDIR/encryptpad.AppImage || true
cp -R ./linux_deployment/AppDir $WORKDIR/
cp ./bin/release/encryptpad $WORKDIR/AppDir/
#mkdir -p $WORKDIR/AppDir/plugins/platformthemes
#QT_INSTALL_PLUGINS=$(qmake -query | grep "^QT_INSTALL_PLUGINS:" | sed s/^QT_INSTALL_PLUGINS://g)
#cp $QT_INSTALL_PLUGINS/platformthemes/libqgtk2.so $WORKDIR/AppDir/plugins/platformthemes/

cd $WORKDIR/
linuxdeployqt ./AppDir/encryptpad -bundle-non-qt-libs
appimagetool ./AppDir encryptpad.AppImage
