@echo off
SETLOCAL

set CC=gcc

set COMPILER_FLAGS=-std=c11

set INCLUDE=-Ijustengine/include
set LIB=-Ljustengine/lib

set LINK=-ljustengine -lraylib -lgdi32 -lwinmm

set SRC_DIR=.
set TARGET_DIR=target

set OUTPUT=main.exe
set COMPILE=^
    %SRC_DIR%/main.c

mkdir %TARGET_DIR%

@echo on

%CC% %COMPILER_FLAGS% %COMPILE% %INCLUDE% %LIB% %LINK% -o ./%TARGET_DIR%/%OUTPUT%

@echo off

set OUTPUT_LOCAL=main.exe
copy %TARGET_DIR%\%OUTPUT% %OUTPUT_LOCAL%
