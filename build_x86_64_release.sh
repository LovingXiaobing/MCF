#!/bin/bash -e

prefix="$(pwd)/release/mingw64"
mkdir -p "$prefix"

export CPPFLAGS=" -I$prefix/include -DNDEBUG"
export CFLAGS=" -O2 -ffunction-sections -fdata-sections"
export CXXFLAGS=" -O2 -ffunction-sections -fdata-sections"
export LDFLAGS=" -O2 -Wl,-s,--gc-sections -L$prefix/lib"

builddir="$(pwd)/.build_x86_64_release"
build=x86_64-w64-mingw32

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
