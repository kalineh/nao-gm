#!/bin/bash

echo 'Setting Nao environment variables...'

export ALDEBARAN_DOC_DIR="`pwd`/sdk/aldebaran-documentation-1.12.5"
export NAOQI_SDK_DIR="`pwd`/sdk/naoqi-sdk-1.12.5-win32-vs2010"
export QIBUILD_DIR="`pwd`/sdk/aldebaran-qibuild-v1.12.1-443-g34d2153/aldebaran-qibuild-34d2153"
export QIBUILD_WORKTREE="`pwd`/worktree"

if [ ! -d $ALDEBARAN_DOC_DOR ]; then
	echo 'aldebaran documentation not found at' $ALDEBARAN_DOC_DIR
	exit 1
fi

if [ ! -d $NAOQI_SDK_DIR ]; then
	echo 'naoqi sdk directory not found at' $NAOQI_SDK_DIR
	exit 1
fi

if [ ! -d $QIBUILD_DIR ]; then
	echo 'qibuild directory not found at' $QIBUILD_DIR
	exit 1
fi

if [ ! -d $QIBUILD_WORKTREE ]; then
	echo 'qibuild worktree not found at' $QIBUILD_WORKTREE
	exit 1
fi

echo 'Adding Python to path...'
export PATH="$PATH:/C/Python27"

echo 'Adding CMake to path...'
export PATH="$PATH:/C/Program Files (x86)/CMake 2.8/bin"

echo 'Adding QIBuild to path...'
export PATH="$PATH:$QIBUILD_DIR/python/bin"

echo ''
echo 'Starting NAO development shell...'
echo ''
sh -c "cd `pwd` && sh -l -i"


