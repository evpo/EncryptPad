#!/bin/bash
set -e

if ! [[ -r "bin/release/encryptpad" ]] || ! [[ -r "bin/release/encryptcli" ]]; then
    echo "the release binary is not found" >&2
    exit -1
fi
install --mode=755 --owner=0 --group=0 -T bin/release/encryptpad /usr/bin/encryptpad
install --mode=755 --owner=0 --group=0 -T bin/release/encryptcli /usr/bin/encryptcli
pushd linux_deployment/de_install >/dev/null
for f in $(find -type d)
do
    ( cd / ; mkdir -p "$f" )
done
for f in $(find -type f)
do
    install --mode=744 --owner=0 --group=0 -T $f "/${f}"
done
popd >/dev/null
if which update-icon-caches >/dev/null; then
    echo "running upate-icon-caches ..."
    update-icon-caches /usr/share/icons/*
fi
if which update-mime-database >/dev/null; then
    echo "running update-mime-database ..."
    update-mime-database /usr/share/mime
fi
if which update-desktop-database >/dev/null; then
    echo "running update-desktop-database .."
    update-desktop-database /usr/share/applications
fi
echo "installation finished"
