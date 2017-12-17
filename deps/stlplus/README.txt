STLplus
-------

STLplus was originally intended as a library to extend the STL by
providing missing container data structures such as smart-pointers,
matrices, trees and graphs.

On the way it became a collection point for other general-purpose
code. The most significant of these is the data persistance functions
which allow any C++ data structure to be made serialisable.

Since v3.0 the library has been split into a collection of 5
libraries. The idea is that some of these can be used stand-alone. For
example, the extension data structures can be used without the rest of
STLplus by just using the containers library. The libraries can be
combined into one 'monolithic' library if desired.

Download
--------

The source code for the STLplus library collection can be downloaded
from the Sourceforge website:

https://sourceforge.net/projects/stlplus/files/

Download the latest 'stlplus3' project and unpack it.

If you are going to be using gcc make to compile the stlplus3 project,
also download and unpack the latest version of the 'makefiles' project
from the same place.

Subversion
----------

If you want the latest development version (which might occasionally
break because it is a development version), get it via Subversion:

svn checkout svn://svn.code.sf.net/p/stlplus/stlplus3/trunk stlplus3
svn checkout svn://svn.code.sf.net/p/stlplus/makefiles/trunk makefiles

You can also get stable releases this way, these are stored in the
'tags' directory:

To get a listing of the versions available:

svn list svn://svn.code.sf.net/p/stlplus/stlplus3/tags
svn list svn://svn.code.sf.net/p/stlplus/makefiles/tags

In the following example you are checking out stlplus3 v3.15 and makefiles v1.11.

svn checkout svn://svn.code.sf.net/p/stlplus/stlplus3/tags/stlplus3-03-15 stlplus3
svn checkout svn://svn.code.sf.net/p/stlplus/makefiles/tags/makefiles-01-11 makefiles

Changes and Versions
--------------------

For details of the different versions and the changes between them,
see the changes log:

http://stlplus.sourceforge.net/stlplus3/docs/changes.html

Directory Structure
-------------------

The top-level directory is set up for using STLplus as a library collection. Each
library in the colection is be found in a subdirectory:

          containers
          persistence
          portability
          strings
          subsystems

The documentation is in the 'docs' directory and starts with index.html.

Build files for IDEs are found in the 'ide' directory, but the gcc
command-line tools use make and the Makefiles are kept in the library
directories.

Multi-Library Build
-------------------

To build the STLplus3 library collection as 5 separate libraries, use
either gnu make at the top level for gcc, or the project files in the
'ide/' directory for Borland or Microsoft compilers.

The Makefile used by gnu make requires the makefiles project, which is
available from the same SourceForge site that you downloaded the
STLplus library from, as explained in Download above.

The makefiles module should be installed in the same directory as the
stlplus3 module.

Monolithic Build
----------------

The 'source' directory is provided with script files that allow the library
collection to be merged into one large library - termed the monolithic build.

To build the monolithic STLplus3 library, use either make in the
'source' directory for gcc, or the project files ending in '_monolithic'
in the 'ide/' directory for Borland or Microsoft compilers.

You may need to run the make_monolithic script from within the source directory
(i.e. from a command shell). This is run automatically when using gnu
make and Visual Studio, but not necessarily by other IDEs.

More information
----------------

For more details about building the collection, see:

https://stlplus.sourceforge.net/stlplus3/docs/building.html

