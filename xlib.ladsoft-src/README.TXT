This package is the sources for my DOS C compiler and related tools.

See license.txt and copying for licensing information

See relnotes.doc for release notes

use pkunzip -d to install these files

These compilers emit 32-bit code.  Together all the tools generate DPMI
programs that run under DOS.

These tools do generate debug information; however there is presently
no support for debugging under DOS/DPMI without linking a debug-like
debugger into the program being developed.

Most of the tools in this package were compiled and linked with the
tools in this package.

See license.txt and copying for licensing information

My website is:
http://ladsoft.tripod.com/cc386_compiler.htmml

Companion files to this file which should be found on my website
are:

cwdl375e.zip    - WIN32 version of compiler executables
ccdl375e.zip    - DOS version of compiler executables
cxdl375e.zip    - combined executables, minimal installation
cwdl375l.zip    - rtl sources + DPMI setups

The above files are useful for changing the compiler/library/tool sources
for most of the programs that are included in this file.

This version of the compiler generates either ASM code in the 
TASM or NASM format, or OBJ files in the old MSDOS format.
You may use either the included VALX, or borland TLINK or watcom WLINK to
create images.  I am recommending NOT to use borland tools
unless you are prepared to deal with bugs.

David Lindauer
email: camille@bluegrass.net
web site: http://ladsoft.tripod.com