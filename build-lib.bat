@echo off
SETLOCAL

set COMPILER_FLAGS=-c

set INCLUDE=-Ivendor/raylib-5.0/include -Isrc
set LIB=

:: -Ivendor/raylib-5.0/include -Lvendor/raylib-5.0/lib -lraylib -lgdi32 -lwinmm

set LINK=

set SRC_DIR=src
set LIB_DIR=justengine

set LIBRARY=libjustengine.a

set COMPILE=^
    %SRC_DIR%/threadpool/threadpool.c^
    %SRC_DIR%/events/events.c^
    %SRC_DIR%/assets/asset.c^
    %SRC_DIR%/assets/assetserver.c^
    %SRC_DIR%/physics/collision.c^
    %SRC_DIR%/lib.c

mkdir %LIB_DIR%
mkdir %LIB_DIR%\include
mkdir %LIB_DIR%\lib

copy %SRC_DIR%\justengine.h %LIB_DIR%\include\justengine.h

copy vendor\raylib-5.0\include\*.h %LIB_DIR%\include\
copy vendor\raylib-5.0\lib\libraylib.a %LIB_DIR%\lib\libraylib.a

@echo on

gcc %COMPILER_FLAGS% %COMPILE% %INCLUDE% %LIB% %LINK%
ar -rcs %LIB_DIR%/lib/%LIBRARY% *.o

@echo off

for %%o in (*.o) do del %%o

:: ar q %LIB_DIR%/libjustengine2.a vendor/raylib-5.0/lib/libraylib.a %LIB_DIR%/%LIBRARY%
:: ranlib %LIB_DIR%/libjustengine2.a