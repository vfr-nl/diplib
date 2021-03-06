# DIPlib 3.0

*DIPlib* is a library for quantitative image analysis. It has been in development
at Delft University of Technology in The Netherlands since 1995. The 3.0 release
represents a complete rewrite in modern C++ of the library infrastructure, with most of
the image processing and analysis algorithms ported unmodified from the previous
version.

[Status](src/documentation/workplan.md):
we have built most of the new infrastructure, and are in the process of
porting algorithms over. It's a slow process, but
[you can help](src/documentation/workplan.md)!

See the [documentation](https://diplib.github.io/diplib-docs/) for more information.

## Building the library

To build the library under Unix or MacOS, you will need a C++14 compliant compiler and CMake.
Use the following commands to build:

    mkdir target
    cd target
    cmake /path/to/dip/root/directory
    make -j install

To change which compilers CMake uses (delete any `CMakeCache.txt` file in your build directory first):

    CC=gcc-5 CXX=g++-5 cmake /path/to/dip/root/directory

Available `make` targets:

    <default>    # builds all targets
    DIP          # builds the DIPlib library only
    DIPviewer    # builds the DIPviewer module only (plus the DIPlib library)
    PyDIP        # builds the PyDIP Python module only (plus the DIPlib library)
    DIPimage     # builds the DIPimage MATLAB toolbox only (plus the DIPlib library)
    check        # builds the unit_tests program and runs it
    check_memory # ...and runs it under valgrind
    apidoc       # Doxygen HTML documentation for the library API
    install      # builds and installs all targets
    tests        # deprecated -- old test functions

Important `cmake` command-line arguments:

    -DCMAKE_INSTALL_PREFIX=$HOME/dip        # choose an instal location
    -DCMAKE_BUILD_TYPE=Debug                # by default it is release
    -DDIP_SHARED_LIBRARY=Off                # to build a static library
    -DDIP_EXCEPTIONS_RECORD_STACK_TRACE=Off # to disable stack trace generation on exception
    -DDIP_ENABLE_ASSERT=Off                 # to disable asserts
    -DDIP_ENABLE_DOCTEST=Off                # to disable doctest within DIPlib
    -DDIP_ENABLE_ICS=Off                    # to disable ICS file format support
    -DDIP_ENABLE_TIFF=Off                   # to disable TIFF file format support
    -DDIP_ENABLE_UNICODE=Off                # to disable UFT-8 strings within DIPlib
    -DDIP_ALWAYS_128_PRNG=On                # to use the 128-bit PRNG code where 128-bit
                                            #    integers are not natively supported
    -DDIP_BUILD_DIPVIEWER=Off               # to not build/install the DIPviewer module
    -DDIP_BUILD_PYDIP=Off                   # to not build/install the PyDIP Python module
    -DDIP_BUILD_DIPIMAGE=Off                # to not build/install the DIPimage MATLAB toolbox
    -DPYBIND11_PYTHON_VERSION=3             # to compile PyDIP agains Python 3

Some of these options might not be available on your system. For example, if you don't have
MATLAB installed, the `DIP_BUILD_DIPIMAGE` option will not be defined. Setting it to `Off` will
yield a warning message when running CMake.

Note that on some platforms, the Python module requires the *DIPlib* library to build as a dynamic
load library (`-DDIP_SHARED_LIBRARY=On`, which is the default).

Under Windows you can follow a similar process, but I have never used CMake under
Windows, so I'll let someone else write this bit.

## License

Copyright 2014-2017 Cris Luengo  
Copyright 1995-2014 Delft University of Technology

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this library except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0  
   (or see the LICENSE.txt file in this distribution)

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*DIPlib* includes the following libraries / external code:

- *Eigen 3*  
  Mozilla Public License Version 2.0  
  see dependencies/eigen3/LICENSE.txt  
  (we do not use any of the components with more restrictive licenses)

- *DocTest* (its use can be disabled)  
  Copyright 2016-2017 Viktor Kirilov  
  The MIT License (MIT)  
  see dependencies/doctest/LICENSE.txt

- The DFT algorithm out of *OpenCV* 3.1  
  Copyright 2000 Intel Corporation  
  Intel License Agreement For Open Source Computer Vision Library  
  see src/transform/opencv_dxt.h

- *PCG Random Number Generation for C++*  
  Copyright 2014-2017 Melissa O'Neill and the PCG Project contributors  
  Apache 2.0 License, or The MIT License, at your option  
  see include/diplib/private/pcg_*.hpp

- A fast 2D labeling algorithm out of *YACCLAB*  
  Copyright 2016-2017 Costantino Grana, Federico Bolelli, Lorenzo Baraldi and Roberto Vezzani  
  3-Clause BSD License  
  see include/diplib/regions/labelingGranan2016.h

- *libics* (its use can be disabled)  
  Copyright 2015-2017 Scientific Volume Imaging Holding B.V.  
  Copyright 2000-2013 Cris Luengo and others  
  GNU Lesser General Public License, Version 2.1  
  see dependencies/libics/GNU_LICENSE

- *pybind11* (only used in the Python bindings)  
  Copyright 2016 Wenzel Jakob  
  3-Clause BSD License  
  see dependencies/pybind11/LICENSE

- A few MATLAB scripts from *OME Bio-Formats*  
  Copyright 2012-2017 Open Microscopy Environment  
  GNU General Public License, Version 2  
  see dipimage/private/bf*.m  
  (the full *Bio-Formats* library needs to be installed for these to be useful)

Note that all of these have permissive open-source licenses similar in spirit
to the Apache License, except for *OME Bio-Formats*.

*DIPlib* also optionally links against:

- *LibTIFF* (as installed on your system)  
  Copyright 1988-1997 Sam Leffler  
  Copyright 1991-1997 Silicon Graphics, Inc.  
  MIT-style license

- *ZLib* (as installed on your system)  
  Copyright 1995-2017 Jean-loup Gailly and Mark Adler  
  MIT-style license
