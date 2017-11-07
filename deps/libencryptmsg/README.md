# libEncryptMsg

This is an OpenPGP implementation that works with symmetrically encrypted files following RFC4880 specification.

**This library is currently in Alpha stage. It is not recommended for production**

CLI:

    ./bin/debug/encryptmsg

Unit tests:

    ./bin/debug/encryptmsg-tests

## System requirements

    Linux
    Mingw
    C++11 g++ or clang

## Compile

Build:

    make

Release:

    make RELEASE=on

Verbose:

    make VERBOSE=on

Clean:

    make clean

Tests:

    make tests

## Acknowledgments

1. [**Botan**](http://botan.randombit.net/)
2. [**stlplus**](http://stlplus.sourceforge.net/)
3. [**Makefiles**](http://stlplus.sourceforge.net/makefiles/docs/)
4. [**zlib**](http://zlib.net/)
5. [**gtest**](http://code.google.com/p/googletest/)
6. [**plog**](https://github.com/SergiusTheBest/plog)

## License

libEncryptMsg is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.

libEncryptMsg is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
