#!/bin/sh
set -e

#Populate submodules
echo Populating git submodules...
#git submodule update --init --recursive
echo Done

#Autoreconf
echo Running autoreconf...
export AUTOMAKE="automake --foreign -a"
autoreconf -f -i
