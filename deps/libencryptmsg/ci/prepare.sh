#!/bin/bash
set -exu

git clone https://github.com/randombit/botan $HOME/botan
pushd $HOME/botan
./configure.py --prefix=/usr --enable-modules=aes,pbkdf2,zlib --link-method=symlink
popd
