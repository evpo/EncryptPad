#!/bin/bash
set -e

if ! [[ -r "bin/release/encryptpad" ]] || ! [[ -r "bin/release/encryptcli" ]]; then
    echo "the release binary is not found" >&2
    exit -1
fi

if [[ $# != 2 ]]; then
    echo "invalid # of arguments" >&2
    exit -1
fi
prefix="${1}"
bindir="${2}"
ownerarg=""
grouparg=""
if [[ ${UID} == 0 ]]; then
    ownerarg="--owner=0"
    grouparg="--group=0"
fi
install --mode=755 ${ownerarg} ${grouparg} -T bin/release/encryptpad ${bindir}/encryptpad
install --mode=755 ${ownerarg} ${grouparg} -T bin/release/encryptcli ${bindir}/encryptcli
pushd linux_deployment/de_install/usr >/dev/null
for f in $(find -type d)
do
    ( cd ${prefix} ; mkdir -p "${f}" )
done
for f in $(find -type f)
do
    install --mode=744 ${ownerarg} ${grouparg} -T $f "${prefix}/${f}"
done
popd >/dev/null
if which update-icon-caches >/dev/null; then
    echo "running upate-icon-caches ..."
    update-icon-caches ${prefix}/share/icons/*
fi
if which update-mime-database >/dev/null; then
    echo "running update-mime-database ..."
    update-mime-database ${prefix}/share/mime
fi
if which update-desktop-database >/dev/null; then
    echo "running update-desktop-database .."
    update-desktop-database ${prefix}/share/applications
fi
echo "installation finished"
