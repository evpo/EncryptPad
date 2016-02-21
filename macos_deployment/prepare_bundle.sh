#!/bin/bash

USAGE="USAGE: prepare_bundle.sh <path to the app container>"

if [[ $# < 1 ]] || [[ $# > 1 ]]
then
    echo $USAGE
    exit -1
fi

macdeployqt $1
mkdir -p $1/Contents/Resources
cp -f ./Resources/* $1/Contents/Resources/
cp -f ./Info.plist $1/Contents/
cp -f ../images/crypt.icns $1/Contents/Resources/
