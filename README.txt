vfTasks
=======

  vfTasks is a library with a C API containing the following features:
  - Manage worker thread pools
  - Inter-thread streaming communication channels
  - 2-D synchronization for parallelized loops
  It does not depend on any other libraries other than libc and the pthreads library.
  The latter can however be easily replaced with custom threading and memory 
  allocation solution, allowing vfTasks to be ported to an embedded CPU or DSP processor. 

  For more information, visit http://www.vectorfabrics.com/content/products/vftasks.
  vfTasks is developed by Vector Fabrics (http://vectorfabrics.com) and 
  complements Vector Fabrics' vfThreaded-x86 and vfEmbedded products
  that help you parallelize a C application. 


Installing
==========

  Instructions for installing and testing this software are in INSTALL.txt


Documentation and Support
=========================

  Examples on how to use vfTasks APIs are located in example/src.
  See http://www.vectorfabrics.com/content/products/vftasks for the example documentation.


Copyright
=========

  All of the code in this distribution is Copyright (c) 2011
  Vector Fabrics B.V.

  The included LICENSE.txt file describes the license in detail.
