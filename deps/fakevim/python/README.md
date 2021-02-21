Python Bindings for FakeVim
===========================

Few notes:
 * Bindings build process is handled using
   PyQt's [SIP](http://pyqt.sourceforge.net/Docs/sip4/index.html).
 * Debug build is enabled by default
 * Compile FakeVim library and binndings with same version of Qt.
 * Use same major version of python to build and run.

To build and install run following commands from this directory.

```bash
./configure.py
qmake
make install
```

On Ubuntu 14.04, package `pyqt5-dev` must be installed. Additionally build
script needs to be invoked with following command.

```
python3 ./configure.py
```

