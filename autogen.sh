#!/bin/sh
set -e

for d in $(find . -name configure.ac -exec dirname {} \;)
do
  mkdir -p $d/m4;
done

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i
