#!/bin/bash

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
