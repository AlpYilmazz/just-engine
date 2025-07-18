@echo off
SETLOCAL

set ENTRY=%1

set SRC_DIR=src
set BUILD_DIR=target

set CC=gcc

set COMPILER_FLAGS=-std=c11

set INCLUDE=-Ijustengine/include -IC:/dev/vendor/openssl-3.5.0/include
set LIB=-Ljustengine/lib -LC:/dev/vendor/openssl-3.5.0/lib64

set LINK=^
    -lraylib^
    -ljustengine^
    -lssl -lcrypto^
    -lgdi32 -lwinmm^
    -lws2_32^
    -lcrypt32 -luser32 -ladvapi32

    @REM -lssl -lcrypto -lcrypt32 -luser32 -ladvapi32 -lws2_32^
    

@REM 
@REM WS2_32.LIB, GDI32.LIB, ADVAPI32.LIB, CRYPT32.LIB and USER32.LIB

set SRC_DIR=.

set OUTPUT=game.exe
set COMPILE=^
    %SRC_DIR%/%ENTRY%.c
@echo on

%CC% %COMPILER_FLAGS% %COMPILE% %INCLUDE% %LIB% %LINK% -o %OUTPUT%

@echo off
