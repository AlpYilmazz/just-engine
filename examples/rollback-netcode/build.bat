@echo off
SETLOCAL

set CC=gcc

set COMPILER_FLAGS=-std=c11

set INCLUDE=-Ijustengine/include
set LIB=-Ljustengine/lib

set LINK=-ljustengine -lraylib -lgdi32 -lwinmm -lws2_32

set SRC_DIR=.

set OUTPUT=game.exe
set COMPILE=^
    %SRC_DIR%/main.c
@echo on

%CC% %COMPILER_FLAGS% %COMPILE% %INCLUDE% %LIB% %LINK% -o %OUTPUT%

@echo off
