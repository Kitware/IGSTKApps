This file contains information with regard to problems and solutions
associated with building the IGITutorial on various systems.

Linux (Ubuntu):
===============

--Icons not visible--
Icon buttons are not displayed in the programs and a warning is issued
stating that there is a conflict between the libpng used to compile and that
used for running.

This problem occurs when the OpenCV library is compiled with the WITH_PNG
flag set to on and we have a different system libpng. Turning it to off and
recompiling solves the issue (all programs will use the system's libpng).
----

--OpenCV 2.3.1 configuration--
    Built as dynamic libs?:     NO
    Compiler:                   /usr/bin/g++
  GUI: 
    GTK+ 2.x:                   YES
    GThread:                    YES

  Media I/O: 
    ZLib:                       YES
    JPEG:                       build
    PNG:                        TRUE
    TIFF:                       build
    JPEG 2000:                  FALSE
    OpenEXR:                    NO
    OpenNI:                     NO
    OpenNI PrimeSensor Modules: NO
    XIMEA:                      NO

  Video I/O:
    DC1394 1.x:                 NO
    DC1394 2.x:                 NO
    FFMPEG:                     NO
      codec:                    NO
      format:                   NO
      util:                     NO
      swscale:                  NO
      gentoo-style:             NO
    GStreamer:                  NO
    UniCap:                     NO
    PvAPI:                      NO
    V4L/V4L2:                   Using libv4l
    Xine:                       NO

  Other third-party libraries:
    Use IPP:                    NO
    Use TBB:                    NO
    Use ThreadingFramework:     NO
    Use Cuda:                   NO
    Use Eigen:                  NO
----


Windows 7
=========
--Permission issues--
Installing the IGITutorial in the system program folder can cause permission
problems if your windows user account does not have administrator privileges.
In this case either run the programs with administrator privileges (right click on
the application and choose "run as administrator") or install the IGITutorial on
another place where you have permissions to create new folders and files.
----


--Compilation fails with VTK 5.10--
The IGITutorial compiles fine with VTK 5.8.0, the newer version VTK 5.10 
causes following error: 
  ITK'MetaEvent' : is not a member of 'global namespace'
----

--Install error--
Set CMAKE_INSTALL_PREFIX to a directory where you have privileges for creating directory
otherwise the default path can cause following error:

  -- Install configuration: "Release"
  CMake Error at cmake_install.cmake:31 (FILE):
    file cannot create directory: C:/Program Files (x86)/IGITutorial/Programs.
    Maybe need administrative privileges.
----

--Compilation fails with Qt version less then 4.6--
The range slider uses QScopedPointer. This class was introduced in Qt 4.6.
----

Apple Mac OS X
==============
Select "Unix Makefiles" as generator
Set GCC for both C and C++ compilation in CMake


