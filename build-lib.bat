@echo off
SETLOCAL

set SRC_DIR=src
set BUILD_DIR=target
set LIB_DIR=justengine

mkdir %LIB_DIR%
mkdir %LIB_DIR%\include
mkdir %LIB_DIR%\lib

@echo on
mingw32-make -f makefile-lib
@echo off

copy vendor\raylib-5.0\include\*.h %LIB_DIR%\include\
copy vendor\raylib-5.0\lib\libraylib.a %LIB_DIR%\lib\libraylib.a

copy %SRC_DIR%\justengine.h %LIB_DIR%\include\justengine.h
copy %BUILD_DIR%\libjustengine.a %LIB_DIR%\lib\libjustengine.a