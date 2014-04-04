#!/bin/sh
set -e

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i
