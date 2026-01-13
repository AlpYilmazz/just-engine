@echo off
SETLOCAL

set ARG_ENTRY=%1
set ARG_ENTRY_NAME=%~n1
SHIFT

:loop
if NOT "%1"=="" (
    if "%1"=="--out" (
        SET ARG_OUTPUT=%2
        SHIFT
    ) else if "%1"=="--introspect" (
        SET ARG_WITH_INTROSPECT=true
    ) else if "%1"=="--shared" (
        SET IS_SHARED=true
    )
    SHIFT
    goto :loop
)

set SRC_DIR=src
set BUILD_DIR=target

set CC=gcc

set COMPILER_FLAGS=-std=c11

set INCLUDE=^
    -Ijustengine/include/openssl ^
    -Ijustengine/include/curl ^
    -Ijustengine/include/raylib ^
    -Ijustengine/include/clay ^
    -Ijustengine/include/cimgui ^
    -Ijustengine/include
set LIB=^
    -Ljustengine/lib/openssl ^
    -Ljustengine/lib/curl ^
    -Ljustengine/lib/raylib ^
    -Ljustengine/lib/clay ^
    -Ljustengine/lib/cimgui ^
    -Ljustengine/lib

set LINK=^
    -Wl,--start-group^
    -lraylib^
    -ljustengine^
    -lssl -lcrypto^
    -lcurl^
    -lcimgui^
    -lgdi32 -lwinmm^
    -lws2_32^
    -lcrypt32 -luser32 -ladvapi32^
    -lpthread^
    -Wl,--end-group

    @REM -lssl -lcrypto -lcrypt32 -luser32 -ladvapi32 -lws2_32^
    
@REM 
@REM WS2_32.LIB, GDI32.LIB, ADVAPI32.LIB, CRYPT32.LIB and USER32.LIB

set SRC_DIR=.

if defined ARG_OUTPUT (
    set OUTPUT=%ARG_OUTPUT%
) else (
    set OUTPUT=game.exe
)
set COMPILE=^
    %SRC_DIR%/%ARG_ENTRY%

if defined ARG_WITH_INTROSPECT (
    @echo on
    call run "justengine/bin/introspect.exe" %SRC_DIR%/%ARG_ENTRY% introspect_gen__%ARG_ENTRY_NAME%.h %INCLUDE%
    @echo off
)

if defined IS_SHARED (
    @echo on
    %CC% %COMPILER_FLAGS% -fPIC -m64 -c %COMPILE% %INCLUDE% %LIB% %LINK% -o %ARG_ENTRY_NAME%.o
    %CC% %ARG_ENTRY_NAME%.o %INCLUDE% %LIB% %LINK% -shared -m64 -Wl,--subsystem,windows -Wl,--out-implib,lib%ARG_ENTRY_NAME%.dll.a -o %ARG_ENTRY_NAME%.dll
    del %ARG_ENTRY_NAME%.o
    @echo off
) else (
    @echo on
    %CC% %COMPILER_FLAGS% %COMPILE% %INCLUDE% %LIB% -L. %LINK% -lcurl-test -o %OUTPUT%
    @echo off
)