The makefiles project is a set of universal makefiles for building any project
with Gnu tools (gcc, Gnu make etc.). The idea is that you write a trivial
Makefile which simply includes a universal makefile and it does the rest for
you. This makes it very easy to set up new projects and to ensure consistency
of builds in a multi-library project.

These rules do the following tasks:

 - compile all the .cpp or .c files in the current directory
 - archive the object files together into a single object library
 - build all other projects on which the current project depends
 - link an image
 - (optionally) compile and build a wxWidgets application

Note: These files are designed for ***Gnu make ONLY***.

A make file contains a set of rules. The scope for writing rules is
limited by the makefile syntax. Therefore it has been necessary to
restrict the flexibility of the universal makefile in order for it to
be possible to write it at all. See the included documentation for how
to use it and what the restrictions are.

See http://stlplus.sourceforge.net/makefiles/docs/

