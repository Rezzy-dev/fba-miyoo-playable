This source requires the old miyoo toolchain (uclibcgnueabi) to successfully build and run:
https://github.com/steward-fu/miyoo/releases/download/v1.0/toolchain.7z

To build, edit "Makefile" to point CC, CXX, AS, and the directory paths in "incdir" to your installation of the Miyoo toolchain, then go into the source main directory, and run "make".

You may also need to give the Perl scripts in "src/scripts" executive permissions. If they can't execute/run, the build will fail.
