#!/bin/bash
set -exu

env_name=$1

git clone https://github.com/randombit/botan $HOME/botan
( cd $HOME/botan && git checkout 2.13.0 )
pushd $HOME/botan
    if [[ "$env_name" == "appveyor" ]]; then
        ./configure.py --cc gcc --cpu x86 --os mingw --amalgamation --disable-shared --with-zlib --with-bzip2 --enable-modules aes,pbkdf2,auto_rng,compression --extra-cxxflags "-I c:\Users\appveyor\zlib-1.2.11 -I c:\Users\appveyor\bzip2-1.0.8"
    elif [[ "$env_name" == "osx" ]]; then
        ./configure.py --enable-modules=aes,pbkdf2,auto_rng,compression,zlib --link-method=symlink --amalgamation --disable-shared --cc-bin clang++
    elif [[ "$env_name" == "linux" ]]; then
        ./configure.py --prefix=/usr --enable-modules=aes,pbkdf2,zlib --link-method=symlink
    fi
popd
