#!/bin/zsh

# cross-compile script
echo
read -q "e?cross compile raylib? (y/n) "
if [[ $e =~ ^[Yy]$ ]]
then
  raylibdir=./dpd/raylib/src
  make -j $(nproc) -C $raylibdir clean
  make -j $(nproc) -C $raylibdir CC=x86_64-w64-mingw32-gcc PLATFORM=PLATFORM_DESKTOP PLATFORM_OS=WINDOWS RAYLIB_LIBTYPE=STATIC
fi

echo
read -q "e?cross compile mupdf? (y/n) "
if [[ $e =~ ^[Yy]$ ]]
then
  mupdfdir=./dpd/mupdf/
  cd "$mupdfdir"

  export CC=x86_64-w64-mingw32-gcc
  export CXX=x86_64-w64-mingw32-g++
  export AR=x86_64-w64-mingw32-ar
  export LD=x86_64-w64-mingw32-ld
  export RANLIB=x86_64-w64-mingw32-ranlib

  make OS=win64 CROSSCOMPILE=x86_64-w64-mingw32- clean
  
  make OS=win64 CROSSCOMPILE=x86_64-w64-mingw32- generate
  
  make -j$(nproc) \
    OS=win64 \
    CROSSCOMPILE=x86_64-w64-mingw32- \
    HAVE_X11=no \
    HAVE_GLUT=no \
    XCFLAGS="-msse4.1" \
    libs
fi
