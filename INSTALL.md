1 vfTasks Installation
=======================

This file describes how to build, install & test the vfTasks library
on your local system.


2 Obtaining vfTasks
====================

  1. Download vfTasks archive from github

  2. Unpack the vfTasks archive in a local directory.
     $ tar -zxvf vftasks-<version>.tar.gz
     or use your favourite archive software on windows (7-Zip, WinRaR, ...)


3 Prerequisites
===============

  * To configure the source, CMake is needed (http://www.cmake.org).

  * The CppUnit library is required (http://sourceforge.net/apps/mediawiki/cppunit)

  * To build the (html) documentation from the sources,
    Doxygen is required (http://www.doxygen.org).

3.1 Debian/Ubuntu
-----------------

  * The GNU C compiler (gcc) is required.
    Alternative ANSI C compilers may be used but are not tested.
    $ sudo apt-get install gcc libc-dev libc6-dev

  * To build the source, the GNU Make system is used (http://www.gnu.org/software/make).
    $ sudo apt-get install make cmake

  * For unit testing and doxygen:
    $ sudo apt-get install g++ libcppunit-dev doxygen

3.2 Windows
-----------

  * An ansi C compiler and build environment are needed.
    Microsoft freely offers Visual Studio Express
    (http://www.microsoft.com/express/Downloads/#2010-Visual-CPP)

  * Installing CppUnit on Windows needs to be done from a source build.
    Check out their website for more information
    (http://sourceforge.net/projects/cppunit).

The build has been verified on:
  * Ubuntu 11.10 64 bits:
    - gcc/g++ version 4.6.1
    - CppUnit version 1.12.1
    - Doxygen 1.7.4

  * Ubuntu 10.10 32 bits:
    - gcc/g++ version 4.4.5
    - CppUnit version 1.12.1
    - Doxygen 1.6.3

  * Ubuntu 10.04 64 bits:
    - gcc/g++ version 4.4.3
    - CppUnit version 1.12.1
    - Doxygen 1.6.3

  * Windows 7 Professional 64 bits:
    - Visual Studio Express 10.0
    - CppUnit version 1.12.2
    - Doxygen 1.7.4


4 Building vfTasks
===================

  $ cd vftasks-<version>
  $ mkdir build
  $ cd build

4.1 Debian/Ubuntu
-----------------

  * Build and install the library and headers:
    $ cmake -DCMAKE_BUILD_TYPE=release -DCMAKE_INSTALL_PREFIX=<PATH_TO_INSTALL_DIR> ..
    $ make install
    The vfTasks library is created in <PATH_TO_INSTALL_DIR>/lib/<PLATFORM>
    The include files are located in <PATH_TO_INSTALL_DIR>/include
    When omitting -DCMAKE_INSTALL_PREFIX, vftasks will be installed in /.

  * Build and run the unit tests:
    $ cmake -DCMAKE_BUILD_TYPE=debug ..
    $ make all test

  * Build the html documentation:
    $ make doc
    The documentation is located in vftasks-<version>/build/doc/html/

  * Clean the build directory with:
    $ make clean

  * Some additional example targets are generated:
    - 2dsync: example showcasing the 2-D synchronization API;
    - 2d: original source example for the 2dsync example;
    - partitioned_loop: example showcasing the worker thread API;
    - loop: original source example for the partitioned_loop example;
    - streams: example showcasing the fifo channel API.

  Note: the vftasks library depends on the pthread library, so make sure to link
  against it when using your own Makefiles.

4.1 Windows
-----------------

  * Configure the sources for use in MicroSoft Visual Studio:
    $ cmake -G "Visual Studio <VERSION>" ..

  * Open the generated solution file with the Visual Studio IDE.

  * Depending on the presence of CppUnit and Doxygen, you see a number of projects,
    which can all be built and run using the IDE's interface.

  * Some additional example projects are generated:
    - 2dsync: example showcasing the 2-D synchronization API;
    - 2d: original source example for the 2dsync example;
    - partitioned_loop: example showcasing the worker thread API;
    - loop: original source example for the partitioned_loop example.
