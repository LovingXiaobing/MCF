#!/bin/bash

set -e

prefix="$(pwd)/debug/mingw32"
mkdir -p "$prefix"

export CPPFLAGS=" -I$prefix/include"
export CFLAGS=" -Og -g"
export CXXFLAGS=" -Og -g"
export LDFLAGS=" -Og -L$prefix/lib"

builddir="$(pwd)/.build_i686_debug"
build=i686-w64-mingw32

mkdir -p "$builddir"

(cd MCFCRT &&
  mkdir -p m4
  autoreconf -i)
(mkdir -p "$builddir/MCFCRT" && cd "$builddir/MCFCRT" &&
  (test -f Makefile || ../../MCFCRT/configure --build="$build" --host="$build" --prefix="$prefix") &&
  make -j7 &&
  make install)

(cd MCF &&
  mkdir -p m4
  autoreconf -i)
(mkdir -p "$builddir/MCF" && cd "$builddir/MCF" &&
  (test -f Makefile || ../../MCF/configure --build="$build" --host="$build" --prefix="$prefix") &&
  make -j7 &&
  make install)
