#!/bin/sh
set -e

mkdir -p m4

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i
