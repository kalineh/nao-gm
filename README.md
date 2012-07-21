nao-gm
======

GameMonkey script bindings for NAO robots.

installation for VS2010
=======================

REF: file:///E:/Nao/naoqi-sdk-1.12.5-win32-vs2008/doc/dev/cpp/install_guide.html

* Install Rapid Environment Editor http://www.rapidee.com/download/RapidEE_setup.exe
* Install CMake (http://www.cmake.org/files/v2.8/cmake-2.8.8-win32-x86.exe)
* Install Python 2.7.3 (http://www.python.org/ftp/python/2.7.3/python-2.7.3.msi)
* Install Git for Windows (http://msysgit.googlecode.com/files/Git-1.7.11-preview20120710.exe)

# Python.exe must be in the path, so either add to %PATH% or put in your environment batch file.
# Git installation allows various PATH settings for unix tools - I chose to put it all in the %PATH%, this may affect later steps. 
# qibuild is located in C:\Python27\Scripts after installation, so it needs to exist in the %PATH% or environment batch file.

* Download Naoqi SDK (http://developer.aldebaran-robotics.com/resources/file/1.12.5/naoqi-sdk-1.12.5-win32-vs2008.zip)
* Download Naoqi Docs (http://developer.aldebaran-robotics.com/resources/file/1.12.5/aldebaran-documentation-1.12.5.zip)
* Download Choregraphe (http://developer.aldebaran-robotics.com/resources/file/1.12.5/choregraphe-suite-1.12.5-win32-setup.exe)
* Download qibuild (https://nodeload.github.com/aldebaran/qibuild/zipball/master).

# Note you will need a CD-key for your choregraphe installation. :(
# The docs are not necessary, but might as well grab a local copy now.

% Setup vimrc for git bash, into $HOME/_vimrc (http://phuzz.org/vimrc.html)
% Comment out colorscheme=elflord line in _vimrc, it doesn't seem to exist.

Now lets setup what needs to be in the %PATH% to work correctly. Using the default installed directories:

#!/bin/bash

export PATH=$(PATH);C:\Python27
export PATH=$(PATH);C:\Program Files (x86)\CMake 2.8\bin
export PATH=$(PATH);C:\Program Files (x86)\Git\cmd
export PATH=$(PATH);C:\Program Files (x86)\Git\bin

export NAOQI_SDK_DIR=c:\nao\naoqi-sdk-1.12.5-win32-vs2010
export QIBUILD_DIR=c:\nao\aldebaran-qibuild-34d2153

export PATH=$(PATH);$(QIBUILD_DIR)\python\bin


Now configure the qibuild settings for the local user. TODO: Is there a way to specify a qibuild.xml config file from command-line to avoid system modification?

* qibuild config --wizard

Configuration file will be output to: C:\Users\$USER\.config\qi\qibuild.xml

REF: file:///E:/Nao/naoqi-sdk-1.12.5-win32-vs2008/doc/dev/cpp/tutos/using_qibuild.html#cpp-tutos-using-qibuild

Preparing the workspace:

* export QIBUILD_WORKTREE=C:\nao\worktree
* mkdir $QIBUILD_WORKTREE
* cd $QIBUILD_WORKTREE
* qibuild init

This creates a .qi dir in the directory. This is now the root of a qi build worktree.

* qitoolchain create vs2010 --default

Configuration is output into: C:\Users\$USER\.cache\qi\toolchains\$QIBUILD_TOOLCHAIN

Extract helloworld sample code into worktree:

* unzip $NAOQI_SDK_DIR\doc\_downloads\helloworld.zip -d $QIBUILD_WORKTREE

Create a new project:

* cd $QIBUILD_WORKTREE
* qibuild create testproject
* qibuild configure testproject
* qibuild make testproject

Should be successfully built!

* Open testproject/build-vs2010/testproject.sln.
* Right-click test_testproject in Solution Explorer and Set as StartUp Project.
* F5 and run project.


REF: file:///E:/Nao/naoqi-sdk-1.12.5-win32-vs2008/doc/dev/cpp/tutos/create_module.html

Now modify the CMakeLists.txt to include some naoqi libs for use (or you can just manually edit the file):

* echo qi_use_lib(testproject ALCOMMON) >> $QIBUILD_WORKTREE/testproject/CMakeLists.txt
* echo qi_use_lib(testproject ALAUDIO) >> $QIBUILD_WORKTREE/testproject/CMakeLists.txt
* echo qi_use_lib(testproject ALVISION) >> $QIBUILD_WORKTREE/testproject/CMakeLists.txt

We have configured the build with the default vs2010 config, now use the naoqi-sdk and try it.

* qitoolchain create naoqi-sdk $NAOQI_SDK_DIR/toolchain.xml
* qibuild configure -c naoqi-sdk testproject
* qibuild make -c naoqi-sdk testproject

Now we are successfully running a project built with naoqi sdk. Next step, running some code on NAO.

REF: file:///E:/Nao/naoqi-sdk-1.12.5-win32-vs2010/doc/dev/cpp/examples/core/helloworld/example.html

TODO: running code on nao

