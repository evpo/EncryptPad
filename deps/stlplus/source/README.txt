This directory is for implementing STLplus as a single, monolithic library.

For Visual Studio 2010 onwards, just build the solution
ide/vs20xx/stlplus3_monolithic.sln. This will automatically copy the
source files into here and build the library.

If you are using Gnu make, then just use make to construct the
monolithic version:

  make

To clean up again:

  make clean

For other compilers, run make_monolithic to import all the source files into
this directory, then build it using your compiler of choice.

Run unmake_monolithic to clear the source files from the directory
again. You may need to "make clean" or use your IDE to clean up object
files etc. as well.
